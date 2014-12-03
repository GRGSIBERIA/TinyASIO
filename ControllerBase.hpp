/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

TinyASIO is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

TinyASIO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TinyASIO.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

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
		const ChannelManager* channelManager;

		long inputLatency;
		long outputLatency;

		ASIOCallbacks callbacks;
	
		static long sampleRate;
		static long bufferLength;		//!< バッファの長さ
		static std::shared_ptr<BufferManager> bufferManager;

		static bool ownershipToken;		//!< 所有権

	protected:
		ControllerBase()
		{
			if (ownershipToken)
				throw DuplicateOwnershipToken(L"複数のコントローラが生成されています．片方を削除してください．");

			ownershipToken = true;

			driver = &Driver::Get();
			iasio = driver->Interface();
			channelManager = &driver->ChannelManager();

			long buf = 0;
			ErrorCheck(iasio->getBufferSize(&buf, &buf, &bufferLength, &buf));
			ErrorCheck(iasio->getLatencies(&inputLatency, &outputLatency));

			double sr;	// double型はなんか不自然なのでlongに変換する
			ErrorCheck(iasio->getSampleRate(&sr));
			ErrorCheck(iasio->setSampleRate(sr));
			sampleRate = (long)sr;
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
		void InitCallbacks(ASIOBufferSwitch bufferSwitch)
		{
			callbacks.asioMessage = &AsioMessage;
			callbacks.bufferSwitch = bufferSwitch;
			callbacks.bufferSwitchTimeInfo = &BufferSwitchTimeInfo;
			callbacks.sampleRateDidChange = &SampleRateDidChange;
		}

		/*
		* バッファ生成関数の呼び出しは子クラスに移譲する
		*/
		void CreateBuffer(const std::vector<Channel>& channels, ASIOCallbacks* callbacks)
		{
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, callbacks));
		}

		/*
		* バッファ生成関数の呼び出しは子クラスに移譲する
		*/
		void CreateBuffer(const std::vector<Channel>& channels, ASIOBufferSwitch bufferSwitch)
		{
			InitCallbacks(bufferSwitch);
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, &callbacks));
		}

		/*
		* バッファ生成関数の呼び出しは子クラスに移譲する
		*/
		template <size_t NUM>
		void CreateBuffer(const std::array<Channel, NUM>& channels, ASIOCallbacks* callbacks)
		{
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, callbacks));
		}

		/*
		* バッファ生成関数の呼び出しは子クラスに移譲する
		*/
		template <size_t NUM>
		void CreateBuffer(const std::array<Channel, NUM>& channels, ASIOBufferSwitch bufferSwitch)
		{
			InitCallbacks(bufferSwitch);
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, &callbacks));
		}

	public:
		inline void Start() { driver->Interface()->start(); }	//!< バッファリング開始
		inline void Stop() { driver->Interface()->stop(); }	//!< バッファリング終了
		
		inline const long BufferSize() const { return bufferLength * sizeof(int); }		//!< バッファの容量（バイト）を返す
		inline const long BufferLength() const { return bufferLength; }		//!< バッファの長さを返す
		inline const long InputLatency() const { return inputLatency; }		//!< 入力の遅延を返す
		inline const long OutputLatency() const { return outputLatency; }	//!< 出力の遅延を返す
		inline const long SampleRate() const { return sampleRate; }			//!< サンプリング周波数を返す

	public:
		virtual ~ControllerBase() 
		{
			ownershipToken = false;
		}
	};

	std::shared_ptr<BufferManager> ControllerBase::bufferManager;
	long ControllerBase::bufferLength = 0;
	long ControllerBase::sampleRate = 0;
	bool ControllerBase::ownershipToken = false;
}