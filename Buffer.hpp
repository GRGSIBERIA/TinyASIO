#pragma once
#include <vector>
#include <memory>
#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"
#include "Channel.hpp"


namespace asio
{
	/**
	* バッファクラス
	*/
	class Buffer
	{
		IOType ioType;
		long channelNumber;
		long bufferSize;
		void* bufferData[2];
		ASIOCallbacks* callbacks;

	private:
		static Buffer* currentBuffer;

		static void BufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
		{

		}

		static void SampleRateDidChange(ASIOSampleRate sRate)
		{

		}

		static long AsioMessage(long selector, long value, void* message, double* opt)
		{

		}
		
		static ASIOTime* BufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
		{
			return params;
		}

	public:


		Buffer(const ASIOBufferInfo& info, const long bufferSize, ASIOCallbacks* callbacks)
			: ioType((IOType)info.isInput), channelNumber(info.channelNum), bufferSize(bufferSize), callbacks(callbacks)
		{
			bufferData[0] = info.buffers[0];
			bufferData[1] = info.buffers[1];
		}

		static ASIOCallbacks CreateCallbacks()
		{
			ASIOCallbacks callback;
			callback.bufferSwitch = &Buffer::BufferSwitch;
			callback.sampleRateDidChange = &Buffer::SampleRateDidChange;
			callback.asioMessage = &Buffer::AsioMessage;
			callback.bufferSwitchTimeInfo = &Buffer::BufferSwitchTimeInfo;
			return callback;
		}
	};

	Buffer* Buffer::currentBuffer = nullptr;

	/**
	* バッファを管理するクラス
	*/
	class BufferManager
	{
		IASIO* iasio;

		std::vector<Buffer> buffers;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, ASIOCallbacks* callbacks)
		{
			buffers.clear();
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				buffers.emplace_back(info, bufferSize, callbacks);
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
		void Start()
		{
			ErrorCheck(iasio->start());
		}

		/**
		* バッファリング終了
		*/
		void Stop()
		{
			ErrorCheck(iasio->stop());
		}

		/**
		* バッファリングしたいチャンネルを追加
		*/
		void AddChannel(const IOType& ioType, const long& channelNumber)
		{
			ASIOBufferInfo info;
			info.channelNum = channelNumber;
			info.isInput = ioType;
			bufferInfos.push_back(info);
		}

		/**
		* バッファリングしたいチャンネルを追加
		*/
		void AddChannel(const Channel& channel)
		{
			AddChannel(channel.ioType, channel.ioType);
		}

		/**
		* バッファリングしたいチャンネルをやっぱなしにする
		*/
		void ClearChannel()
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
		const std::vector<Buffer>& CreateBuffer(const long& bufferSize, ASIOCallbacks* callbacks)
		{
			asio::ASIOBufferInfo* infos = &bufferInfos.at(0);
			auto result = iasio->createBuffers(infos, bufferInfos.size(), bufferSize, callbacks);
			ErrorCheck(result);
			InitBuffers(bufferSize, callbacks);
			return buffers;
		}

		/**
		* バッファの生成
		* @params[in] bufferPreference バッファの設定
		* @params[in, out] callbacks バッファリング等の通知のために利用
		* @note bufferSizeは自由に数値を決められないので注意, (bufferSize % granularity == 0)以外の数値は保障できない
		*/
		const std::vector<Buffer>& CreateBuffer(const BufferPreference& bufferPreference, ASIOCallbacks* callbacks)
		{
			CreateBuffer(bufferPreference.preferredSize, callbacks);
			return buffers;
		}

		/**
		* 明示的にバッファを解放
		*/
		void DisposeBuffers() const
		{
			ErrorCheck(iasio->disposeBuffers());
		}
		
	};
}