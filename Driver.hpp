#pragma once
#include <Windows.h>
#include <exception>
#include <string>

#include "Interface.hpp"
#include "Structure.hpp"

namespace asio
{
	/**
	* ドライバのインスタンスに生成失敗すると呼ばれる
	*/
	class CantCreateInstance : std::exception
	{
	public:
		CantCreateInstance(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* ドライバが動かない時に呼ばれる
	*/
	class CantProcessException : std::exception
	{
	public:
		CantProcessException(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* ASIOドライバのインターフェースのラッパクラス
	*/
	class ASIODriver
	{
	private:
		IASIO *driver;			// インターフェースへのポインタ
		void *systemHandle;		// 謎のシステムハンドル

		std::string driverName;
		long driverVersion;

	private:
		void ErrorCheck(const ASIOError& error) const
		{
			switch (error)
			{
			case ASE_NotPresent:
				throw CantProcessException("hardware input or output is not present or available");
			case ASE_HWMalfunction:
				throw CantProcessException("hardware is malfunctioning (can be returned by any ASIO function)");
			case ASE_InvalidParameter:
				throw CantProcessException("input parameter invalid");
			case ASE_InvalidMode:
				throw CantProcessException("hardware is in a bad mode or used in a bad mode");
			case ASE_SPNotAdvancing:
				throw CantProcessException("hardware is not running when sample position is inquired");
			case ASE_NoClock:
				throw CantProcessException("sample clock or rate cannot be determined or is not present");
			case ASE_NoMemory:
				throw CantProcessException("not enough memory for completing the request");
			}
		}

	public:
		

		/**
		* ドライバ名を返す
		*/
		const std::string& Name() const { return driverName; }

		/**
		* ドライバのバージョンを返す
		*/
		const long& Version() const { return driverVersion; }

		/**
		* ドライバのインターフェースを返す
		*/
		const IASIO& Interface() const { return *driver; }

		/**
		* 入力の遅延を返す
		* @return 入力の遅延
		*/
		long InputLatency() const 
		{
			long i, o;
			ErrorCheck(driver->getLatencies(&i, &o));
			return i;
		}

		/**
		* 出力の遅延を返す
		* @return 出力の遅延
		*/
		long OutputLatency() const 
		{
			long i, o;
			ErrorCheck(driver->getLatencies(&i, &o));
			return o;
		}

		/**
		* 入力と出力の遅延を返す
		* @return 入出力の遅延
		*/
		IOLatency Latencies() const
		{
			IOLatency latency;
			ErrorCheck(driver->getLatencies(&latency.input, &latency.output));
			return latency;
		}

		/**
		* 入力のチャンネル数を返す
		* @return 入力のチャンネル数
		*/
		long InputChannels() const
		{
			long i, o;
			ErrorCheck(driver->getChannels(&i, &o));
			return i;
		}

		/**
		* 出力のチャンネル数を返す
		* @return 出力のチャンネル数
		*/
		long OutputChannels() const
		{
			long i, o;
			ErrorCheck(driver->getChannels(&i, &o));
			return o;
		}

		/**
		* 入出力のチャンネル数を返す
		* @return 入出力のチャンネル数
		*/
		IOChannels Channels() const
		{
			IOChannels channels;
			ErrorCheck(driver->getChannels(&channels.input, &channels.output));
			return channels;
		}

		/**
		* サンプリング・レートを返す
		* @return サンプリング・レート
		*/
		double SampleRate() const
		{
			double rate;
			ErrorCheck(driver->getSampleRate(&rate));
			return rate;
		}

		/**
		* サンプリング・レートを設定する
		* @params[in] rate サンプリング・レート
		*/
		void SampleRate(double rate)
		{
			ErrorCheck(driver->setSampleRate(rate));
		}

		/**
		* サンプリング・レートを設定できるか試すらしい
		* @params[in] rate サンプリング・レート
		* @note よくわからないので非推奨関数
		*/
		void CanSampleRate(double rate)
		{
			ErrorCheck(driver->canSampleRate(rate));
		}

		/**
		* バッファの設定を取得
		* @return バッファの現在の設定
		* @note 必ずしも信用できる値を取得できるとは限らない
		*/
		BufferPreference GetBufferPreference() const
		{
			BufferPreference buf;
			ErrorCheck(driver->getBufferSize(&buf.minSize, &buf.maxSize, &buf.preferredSize, &buf.granularity));
			return buf;
		}

		/**
		* チャンネルの情報を取得
		* @params[in] channelNumber 取得したいチャンネルの番号
		* @return チャンネル情報
		*/
		Channel& ChannelInfo(const long channelNumber) const
		{
			ASIOChannelInfo info;
			info.channel = channelNumber;
			ErrorCheck(driver->getChannelInfo(&info));
			return Channel(info);
		}

		/**
		* オーディオ機器のクロック周波数を取得
		*/
		long ClockSource() const
		{

		}

	public:
		/**
		* バッファリング開始
		*/
		void Start()
		{
			ErrorCheck(driver->start());
		}

		/**
		* バッファリング終了
		*/
		void Stop()
		{
			ErrorCheck(driver->stop());
		}

		void OutputReady()
		{

		}

		/**
		* @params[in] clsid ロードしたいCLSID
		*/
		ASIODriver(const CLSID& clsid)
		{
			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&driver);
			if (FAILED(hr))
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");

			driver->init(systemHandle);

			// 名前とドライバのバージョンだけ取得
			char buffer[360];
			driver->getDriverName(buffer);
			driverName = buffer;
			driverVersion = driver->getDriverVersion();
		}

		/**
		* ドライバの解放など
		*/
		virtual ~ASIODriver()
		{
			driver->disposeBuffers();
			driver->Release();
		}
	};

	// メモ
	//virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) = 0;
	//virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
	//	long bufferSize, ASIOCallbacks *callbacks) = 0;
	//virtual ASIOError disposeBuffers() = 0;
	//virtual ASIOError controlPanel() = 0;
	//virtual ASIOError future(long selector, void *opt) = 0;
	//virtual ASIOError outputReady() = 0;

	// 必要なのかどうかわからないので保留
	//virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources) = 0;
	//virtual ASIOError setClockSource(long reference) = 0;
	//virtual void getErrorMessage(char *string) = 0;
}