#pragma once
#include <vector>
#include <memory>
#include <algorithm>

#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"
#include "Channel.hpp"
#include "SamplePack.hpp"

namespace asio
{
	class BufferList
	{
		std::vector<float> floatBuffer;
		std::vector<int> intBuffer;
		std::vector<double> doubleBuffer;
		std::vector<short> shortBuffer;

		pack::Sample sample;

	private:

		template <typename T>
		void Insert(std::vector<T> buffer, const void* buffer, const long size)
		{
			buffer.insert(buffer.end(), buffer, buffer + size);
		}

		template <typename T>
		void ReverseEndian(T* p)
		{
			std::reverse(
				reinterpret_cast<BYTE*>(p),
				reinterpret_cast<BYTE*>(p) + sizeof(T));
		}

		template <typename T>
		void FormatBigEndian(const void* buffer, const long size)
		{
			T *start = reinterpret_cast<T*>(buffer);
			const size_t num = size / sizeof(T);
			for (size_t i = 0; i < num; ++i)
			{
				ReverseEndian(start + i * sizeof(T));
			}
		}

		/**
		* ビッグエンディアンの処理
		*/
		void ReversibleMSB(const void* buffer, const long size)
		{
			switch (sample.isMSB)
			{
			case pack::Int:
				FormatBigEndian<int>(buffer, size);
				break;

			case pack::Short:
				FormatBigEndian<short>(buffer, size);
				break;

			case pack::Float:
				FormatBigEndian<float>(buffer, size);
				break;

			case pack::Double:
				FormatBigEndian<double>(buffer, size);
				break;
			}
		}

		/**
		* バッファに追加する
		*/
		void StoreBuffer(const void* buffer, const long size)
		{
			switch (sample.type)
			{
			case pack::Int:
				Insert(intBuffer, buffer, size);
				break;

			case pack::Short:
				Insert(shortBuffer, buffer, size);
				break;

			case pack::Float:
				Insert(floatBuffer, buffer, size);
				break;

			case pack::Double:
				Insert(doubleBuffer, buffer, size);
				break;
			}
		}

	public:
		BufferList(pack::Sample& sample)
			: sample(sample) { }

		/**
		* バッファに蓄積する
		*/
		void Store(const void* buffer, const long size)
		{
			if (sample.isMSB)
				ReversibleMSB(buffer, size);
			StoreBuffer(buffer, size);
		}
	};

	/**
	* バッファクラス
	*/
	class Buffer
	{
		IOType ioType;
		long channelNumber;
		long bufferSize;
		const ASIOSampleType sampleType;
		void* bufferData[2];

		BufferList bufferList;

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

	class BufferManager;	// フレンドにするための前方宣言

	/**
	* コールバック関数とバッファリングをまとめるためのクラス
	*/
	class BufferController
	{
		friend BufferManager;

	private:
		static std::vector<Buffer> buffers;

		static void BufferingLoop(long doubleBufferIndex, ASIOBool directProcess)
		{
			for (int i = 0; i < buffers.size(); ++i)
			{
				
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

	private:
		inline void Add(const ASIOBufferInfo& info, const long& bufferSize, const ASIOSampleType& sampleType, ASIOCallbacks* callbacks)
		{
			buffers.emplace_back(info, bufferSize, sampleType, callbacks);
		}

		inline void Clear()
		{
			buffers.clear();
		}

	public:
		static ASIOCallbacks CreateCallbacks()
		{
			ASIOCallbacks callback;
			callback.bufferSwitch = &BufferController::BufferSwitch;
			callback.sampleRateDidChange = &BufferController::SampleRateDidChange;
			callback.asioMessage = &BufferController::AsioMessage;
			callback.bufferSwitchTimeInfo = &BufferController::BufferSwitchTimeInfo;
			return callback;
		}

		/**
		* バッファの配列を取得
		*/
		const std::vector<Buffer>& Buffers() const { return buffers; }
	};

	std::vector<Buffer> BufferController::buffers;

	/**
	* バッファを管理するクラス
	*/
	class BufferManager
	{
		IASIO* iasio;

		BufferController bufferController;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, const ASIOSampleType& sampleType, ASIOCallbacks* callbacks)
		{
			bufferController.Clear();
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				bufferController.Add(info, bufferSize, sampleType, callbacks);
			}
		}

	public:
		BufferManager(IASIO* iasio)
			: iasio(iasio)
		{

		}

		~BufferManager()
		{
			if (bufferInfos.size() > 0)
				ErrorCheck(iasio->disposeBuffers());
		}

		/**
		* バッファリング開始
		*/
		inline void Start()
		{
			ErrorCheck(iasio->start());
		}

		/**
		* バッファリング終了
		*/
		inline void Stop()
		{
			ErrorCheck(iasio->stop());
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