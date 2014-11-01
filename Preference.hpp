#pragma once

#include "Interface.hpp"

namespace asio
{
	class BufferManager;

	/**
	* 設定関係のクラス
	*/
	class Preference
	{
		friend BufferManager;	// 設計ミスったらしい

		IASIO* driver;

		long inputLatency;
		long outputLatency;
		double sampleRate;
		long bufferSize;

	public:
		/**
		* 入力の遅延を得る
		*/
		inline const long& InputLatency() const { return inputLatency; }

		/**
		* 出力の遅延を得る
		*/
		inline const long& OutputLatency() const { return outputLatency; }

		/**
		* サンプリング・レートを返す
		* @return サンプリング・レート
		*/
		inline const double SampleRate() const { return sampleRate; }

		/**
		* バッファの大きさを返す
		* @return バッファの大きさ
		*/
		inline const long BufferSize() const { return bufferSize; }

	private:
		inline void InitLatency()
		{
			ErrorCheck(driver->getLatencies(&inputLatency, &outputLatency));
		}

		inline void InitSampleRate()
		{
			ErrorCheck(driver->getSampleRate(&sampleRate));
		}

		/**
		* サンプリング・レートを設定する
		* @params[in] rate サンプリング・レート
		*/
		inline Preference& SetSampleRate(double rate)
		{
			ErrorCheck(driver->setSampleRate(rate));
			return *this;
		}

		inline Preference& SetSampleRate()
		{
			ErrorCheck(driver->setSampleRate(sampleRate));
			return *this;
		}

		/**
		* サンプリング・レートを設定できるか試すらしい
		* @params[in] rate サンプリング・レート
		* @note よくわからないので非推奨関数
		*/
		inline Preference& CanSampleRate(double rate)
		{
			ErrorCheck(driver->canSampleRate(rate));
		}

	public:
		Preference(IASIO* iasio)
			: driver(iasio)
		{
			InitLatency();
			InitSampleRate();
		}

		Preference(IASIO* iasio, const long bufferSize)
			: driver(iasio), bufferSize(bufferSize)
		{
			InitLatency();
			InitSampleRate();
		}
	};
}