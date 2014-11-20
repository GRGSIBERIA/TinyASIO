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

		ASIOCallbacks callbacks;

		static long bufferLength;
		static std::shared_ptr<BufferManager> bufferManager;

	protected:
		ControllerBase()
		{
			driver = &Driver::Get();
			iasio = driver->Interface();

			long buf = 0;
			ErrorCheck(iasio->getBufferSize(&buf, &buf, &bufferLength, &buf));
			ErrorCheck(iasio->getLatencies(&inputLatency, &outputLatency));

			double sr;	// double型はなんか不自然なのでlongに変換する
			ErrorCheck(iasio->getSampleRate(&sr));
			ErrorCheck(iasio->setSampleRate(sr));
			sampleRate = (long)sr;
		}


		/*
		* バッファ生成関数の呼び出しは子クラスに移譲する
		*/
		void CreateBuffer(const std::vector<Channel>& channels, ASIOCallbacks* callbacks)
		{
			const auto& channelManager = Driver::Get().ChannelManager();
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, callbacks));
		}


		/*
		* バッファ生成関数の呼び出しは子クラスに移譲する
		*/
		template <size_t NUM>
		void CreateBuffer(const std::array<Channel, NUM>& channels, ASIOCallbacks* callbacks)
		{
			const auto& channelManager = Driver::Get().ChannelManager();
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, callbacks));
		}


		static void SampleRateDidChange(ASIOSampleRate sRate)
		{
			throw SampleRateDidChangeException("サンプリング周波数が変更されました");
		}
			
		static long AsioMessage(long selector, long value, void* message, double* opt)
		{
			return 0;
		}
		
		static ASIOTime* BufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
		{
			return params;
		}


		/**
		* コールバック関数を生成する
		*/
		ASIOCallbacks CreateCallbacks(ASIOBufferSwitch bufferSwitch)
		{
			auto callbacks = ASIOCallbacks();
			callbacks.asioMessage = &AsioMessage;
			callbacks.bufferSwitch = bufferSwitch;
			callbacks.bufferSwitchTimeInfo = &BufferSwitchTimeInfo;
			callbacks.sampleRateDidChange = &SampleRateDidChange;
			return callbacks;
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
		virtual ~ControllerBase() { }
	};

	std::shared_ptr<BufferManager> ControllerBase::bufferManager;
	long ControllerBase::bufferLength = 0;
}