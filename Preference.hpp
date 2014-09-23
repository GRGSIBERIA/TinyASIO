#pragma once

#include "Interface.hpp"

namespace asio
{
	/**
	* 設定関係のクラス
	*/
	class Preference
	{
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

		/**
		* サンプリング・レートを設定する
		* @params[in] rate サンプリング・レート
		*/
		Preference& SampleRate(double rate)
		{
			ErrorCheck(driver->setSampleRate(rate));
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

	private:
		void InitLatency()
		{
			ErrorCheck(driver->getLatencies(&inputLatency, &outputLatency));
		}

		void InitSampleRate()
		{
			ErrorCheck(driver->getSampleRate(&sampleRate));
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