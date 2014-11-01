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

	public:
		/**
		* 入力の遅延を得る
		*/
		const long& InputLatency() const { return inputLatency; }

		/**
		* 出力の遅延を得る
		*/
		const long& OutputLatency() const { return outputLatency; }

		/**
		* サンプリング・レートを返す
		* @return サンプリング・レート
		*/
		double SampleRate() const
		{
			return sampleRate;
		}

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
		Preference& CanSampleRate(double rate)
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
	};
}