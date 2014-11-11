#pragma once
#include <string>

#include "Driver.hpp"
#include "Channel.hpp"
#include "Buffer.hpp"

namespace asio
{
	/**
	* コントローラの元になるクラス
	*/
	class ControllerBase
	{
	protected:
		Driver* driver;
		IASIO* iasio;

		long bufferLength;
		long inputLatency;
		long outputLatency;
		long sampleRate;

		ChannelManager* channelManager;
		BufferManager* bufferManager;

	protected:
		ControllerBase()
		{
			driver = &Driver::Get();
			iasio = driver->Interface();

			iasio->getBufferSize(NULL, NULL, &bufferLength, NULL);
			iasio->getLatencies(&inputLatency, &outputLatency);

			double sr;	// double型はなんか不自然なのでlongに変換する
			iasio->getSampleRate(&sr);
			sampleRate = (long)sr;

			channelManager = new ChannelManager();
		}

		void CreateBuffer(ASIOCallbacks* callbacks)
		{
			bufferManager = new BufferManager(channelManager->NumberOfChannels(), bufferLength, callbacks);
		}

	public:
		void Start() { driver->Interface()->start(); }	//!< 録音開始
		void Stop() { driver->Interface()->stop(); }	//!< 録音終了
		
		inline const long BufferSize() const { return bufferLength * sizeof(int); }		//!< バッファの容量（バイト）を返す
		inline const long BufferLength() const { return bufferLength; }		//!< バッファの長さを返す
		inline const long InputLatency() const { return inputLatency; }		//!< 入力の遅延を返す
		inline const long OutputLatency() const { return outputLatency; }	//!< 出力の遅延を返す
		inline const long SampleRate() const { return sampleRate; }			//!< サンプリング周波数を返す

	public:
		virtual ~ControllerBase()
		{
			delete channelManager;
			delete bufferManager;
		}
	};
}