#pragma once
#include <vector>
#include "Interface.hpp"
#include "Channel.hpp"
#include "Buffer.hpp"
#include "Callback.hpp"

namespace asio
{

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
			: iasio(iasio), bufferController(iasio), callbackManager()
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
			callbackManager.Init(&bufferController.inputBuffers, &bufferController.outputBuffers);
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