#pragma once
#include <vector>
#include <memory>
#include <algorithm>

#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"
#include "Channel.hpp"
#include "SamplePack.hpp"
#include "BufferList.hpp"

namespace asio
{
	class BufferController;

	/**
	* バッファクラス
	*/
	class Buffer
	{
		friend BufferController;

		IOType ioType;
		long channelNumber;
		long bufferSize;
		const ASIOSampleType sampleType;
		void* bufferData[2];

		BufferList bufferList;

	public:
		/**
		* IOの種類を返す
		*/
		inline const IOType Type() const { return ioType; }

	public:
		Buffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			:
			ioType((IOType)info.isInput),
			channelNumber(info.channelNum),
			bufferSize(bufferSize),
			sampleType(sampleType),
			bufferList(pack::DetectSampleTypePackStruct(sampleType))
		{
			bufferData[0] = info.buffers[0];
			bufferData[1] = info.buffers[1];
		}

		void Store(const long index, const long size)
		{
			bufferList.Store(bufferData[index], size);
		}
	};

	/**
	* 入力バッファ管理クラス
	*/
	class InputBuffer : public Buffer
	{
	public:
		InputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize, sampleType) {}
	};

	/**
	* 出力バッファ管理クラス
	*/
	class OutputBuffer : public Buffer
	{
	public:
		OutputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize, sampleType) {}
	};

	class BufferManager;	// フレンドにするための前方宣言

	/**
	* コールバック関数とバッファリングをまとめるためのクラス
	*/
	class BufferController
	{
		friend BufferManager;

		static std::vector<Buffer*> buffers;

		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

		IASIO* iasio;

	private:
		BufferController(IASIO* iasio)
			: iasio(iasio) {}

	private:
		void Add(const ASIOBufferInfo& info, const long& bufferSize, const ASIOSampleType& sampleType)
		{
			Buffer* ptr = nullptr;

			if (info.isInput)
			{
				inputBuffers.emplace_back(info, bufferSize, sampleType);
				ptr = &inputBuffers[inputBuffers.size() - 1];
			}
			else
			{
				outputBuffers.emplace_back(info, bufferSize, sampleType);
				ptr = &outputBuffers[outputBuffers.size() - 1];
			}
			buffers.push_back(ptr);
		}

		void Clear()
		{
			buffers.clear();
			inputBuffers.clear();
			outputBuffers.clear();
		}

		static ASIOCallbacks CreateCallbacks()
		{
			ASIOCallbacks callback;
			callback.bufferSwitch = &BufferController::BufferSwitch;
			callback.sampleRateDidChange = &BufferController::SampleRateDidChange;
			callback.asioMessage = &BufferController::AsioMessage;
			callback.bufferSwitchTimeInfo = &BufferController::BufferSwitchTimeInfo;
			return callback;
		}

	public:

		/**
		* バッファリング開始
		*/
		inline void Start() const
		{
			ErrorCheck(iasio->start());
		}

		/**
		* バッファリング停止
		*/
		inline void Stop() const
		{
			ErrorCheck(iasio->stop());
		}

		/**
		* 入力バッファの配列を取得する
		* @return 入力バッファの配列
		*/
		inline const std::vector<InputBuffer> InputBuffers() const { return inputBuffers; }

		/**
		* 出力バッファの配列を取得する
		* @return 出力バッファの配列
		*/
		inline const std::vector<OutputBuffer> OutputBuffers() const { return outputBuffers; }
	};

	std::vector<Buffer*> BufferController::buffers;

	namespace callback
	{
		/**
		* コールバック関数を制御するためのクラス
		*/
		class CallbackManager
		{
			static std::vector<Buffer*>* buffers;

		private:
			static void BufferingInputChannel()
			{

			}

			static void BufferingOutputChannel()
			{

			}

			static void BufferingLoop(long doubleBufferIndex, ASIOBool directProcess)
			{
				for (size_t i = 0; i < buffers->size(); ++i)
				{
					if (buffers->at(i)->Type() == IOType::Input)
						BufferingInputChannel();
					else
						BufferingOutputChannel();
				}
			}

			static void BufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
			{
				BufferingLoop(doubleBufferIndex, directProcess);
			}

			static void SampleRateDidChange(ASIOSampleRate sRate)
			{

			}

			static long AsioMessage(long selector, long value, void* message, double* opt)
			{
				return 0;
			}

			static ASIOTime* BufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
			{
				BufferingLoop(doubleBufferIndex, directProcess);
				return params;
			}
		};

		std::vector<Buffer*>* buffers;
	}

	/**
	* バッファを管理するクラス
	*/
	class BufferManager
	{
		IASIO* iasio;

		BufferController bufferController;
		callback::CallbackManager callbackManager;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, const ASIOSampleType& sampleType, ASIOCallbacks* callbacks)
		{
			bufferController.Clear();
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				bufferController.Add(info, bufferSize, sampleType);
			}
		}

	public:
		BufferManager(IASIO* iasio)
			: iasio(iasio), bufferController(iasio)
		{

		}

		~BufferManager()
		{
			if (bufferInfos.size() > 0)
				ErrorCheck(iasio->disposeBuffers());
		}

		/**
		* バッファリングしたいチャンネルを追加
		*/
		inline void AddChannel(const IOType& ioType, const long& channelNumber)
		{
			ASIOBufferInfo info;
			info.channelNum = channelNumber;
			info.isInput = ioType;
			bufferInfos.push_back(info);
		}

		/**
		* バッファリングしたいチャンネルを追加
		*/
		inline void AddChannel(const Channel& channel)
		{
			AddChannel(channel.ioType, channel.ioType);
		}

		/**
		* バッファリングしたいチャンネルをやっぱなしにする
		*/
		inline void ClearChannel()
		{
			bufferInfos.clear();
		}

		/**
		* バッファリングするチャンネルを返す
		*/
		const std::vector<ASIOBufferInfo>& BufferingChannels() const { return bufferInfos; }

		/**
		* バッファの生成
		* @params[in] bufferSize バッファの設定
		* @params[in, out] callbacks バッファリング等の通知のために利用
		* @note bufferSizeは自由に数値を決められないので注意, (bufferSize % granularity == 0)以外の数値は保障できない
		*/
		const BufferController& CreateBuffer(const long& bufferSize, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			ErrorCheck(iasio->createBuffers(&bufferInfos[0], bufferInfos.size(), bufferSize, callbacks));
			InitBuffers(bufferSize, sampleType, callbacks);
			return bufferController;
		}

		/**
		* バッファの生成
		* @params[in] bufferPreference バッファの設定
		* @params[in, out] callbacks バッファリング等の通知のために利用
		* @note bufferSizeは自由に数値を決められないので注意, (bufferSize % granularity == 0)以外の数値は保障できない
		*/
		const BufferController& CreateBuffer(const BufferPreference& bufferPreference, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			CreateBuffer(bufferPreference.preferredSize, sampleType, callbacks);
			return bufferController;
		}
	};
}