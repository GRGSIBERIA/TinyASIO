#pragma once
#include <vector>
#include "Interface.hpp"
#include "Channel.hpp"
#include "Buffer.hpp"
#include "Callback.hpp"
#include "Preference.hpp"

namespace asio
{
	/**
	* チャンネルが登録されていない場合に送出される例外
	*/
	class DontEntryAnyChannels : std::exception
	{
	public:
		DontEntryAnyChannels(const std::string& message) : exception(message.c_str()) {}
	};

	/**
	* バッファを管理するクラス
	*/
	class BufferManager
	{
		IASIO* iasio;

		std::shared_ptr<BufferController> bufferController;
		callback::CallbackManager callbackManager;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, const ASIOSampleType& sampleType)
		{
			bufferController = std::shared_ptr<BufferController>(new BufferController(iasio, bufferSize));
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				bufferController->Add(info, bufferSize, sampleType);
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
			info.buffers[0] = 0;
			info.buffers[1] = 0;
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
		std::vector<ASIOBufferInfo>& BufferingChannels() { return bufferInfos; }


		/**
		* バッファの生成
		* @params[in] bufferSize バッファの設定
		* @params[in, out] callbacks バッファリング等の通知のために利用
		* @note 
		* bufferSizeは自由に数値を決められないので注意, (bufferSize % granularity == 0)以外の数値は保障できない．
		* Sampling Rateなどの設定も，ドライバ側の設定に依存するようになっているので注意
		*/
		const BufferController& CreateBuffer(const long& bufferSize, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			Preference pref(iasio);
			pref.SetSampleRate();	// デフォルトのサンプリングレートを設定する

			ErrorCheck(iasio->createBuffers(&bufferInfos[0], bufferInfos.size(), bufferSize, callbacks));
			InitBuffers(bufferSize, sampleType);
			callbackManager.Init(&bufferController->inputBuffers, &bufferController->outputBuffers);
			return *bufferController;
		}

		/**
		* バッファの生成
		* @params[in] bufferPreference バッファの設定
		* @params[in, out] callbacks バッファリング等の通知のために利用
		* @note bufferSizeは自由に数値を決められないので注意, (bufferSize % granularity == 0)以外の数値は保障できない
		* Sampling Rateなどの設定も，ドライバ側の設定に依存するようになっているので注意
		*/
		const BufferController& CreateBuffer(const BufferPreference& bufferPreference, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			CreateBuffer(bufferPreference.preferredSize, sampleType, callbacks);
			return *bufferController;
		}


		void EraseDisuseBuffer(const bool activeChannelOnly)
		{
			if (activeChannelOnly)
			{
				auto& buffers = bufferInfos;
				for (auto itr = buffers.begin(); itr != buffers.end(); ++itr)
				{
					// 不要なチャンネルを削除する
					if ((*itr).buffers[0] == nullptr || (*itr).buffers[0] == nullptr)
						itr = buffers.erase(itr);
				}
			}
		}
	};
}