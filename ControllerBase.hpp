#pragma once
#include <string>

#include "Driver.hpp"
#include "Channel.hpp"
#include "Buffer.hpp"

namespace asio
{
	/**
	* コントローラの元になるクラス
	* @warning 複数作成すると挙動がメチャクチャになるので注意
	*/
	class ControllerBase
	{
	protected:
		Driver* driver;
		IASIO* iasio;

		long inputLatency;
		long outputLatency;
		long sampleRate;

		BufferManager* bufferManager;

		static long bufferLength;
		static BufferManager* bufferManagerPtr;

	protected:
		ControllerBase()
		{
			driver = &Driver::Get();
			iasio = driver->Interface();

			ErrorCheck(iasio->getBufferSize(NULL, NULL, &bufferLength, NULL));
			ErrorCheck(iasio->getLatencies(&inputLatency, &outputLatency));

			double sr;	// double型はなんか不自然なのでlongに変換する
			ErrorCheck(iasio->getSampleRate(&sr));
			ErrorCheck(iasio->setSampleRate(sr));
			sampleRate = (long)sr;
		}

		/*
		* バッファ生成関数の呼び出しは子クラスに移譲する
		*/
		void CreateBuffer(ASIOCallbacks* callbacks)
		{
			const auto& channelManager = Driver::Get().ChannelManager();
			bufferManager = new BufferManager(channelManager.NumberOfChannels(), bufferLength, callbacks);
			bufferManagerPtr = bufferManager;
		}


	public:
		void Start() { driver->Interface()->start(); }	//!< バッファリング開始
		void Stop() { driver->Interface()->stop(); }	//!< バッファリング終了
		
		inline const long BufferSize() const { return bufferLength * sizeof(int); }		//!< バッファの容量（バイト）を返す
		inline const long BufferLength() const { return bufferLength; }		//!< バッファの長さを返す
		inline const long InputLatency() const { return inputLatency; }		//!< 入力の遅延を返す
		inline const long OutputLatency() const { return outputLatency; }	//!< 出力の遅延を返す
		inline const long SampleRate() const { return sampleRate; }			//!< サンプリング周波数を返す

	public:
		virtual ~ControllerBase()
		{
			delete bufferManager;
		}
	};

	BufferManager* ControllerBase::bufferManagerPtr = nullptr;
	long ControllerBase::bufferLength = 0;
}