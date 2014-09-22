#pragma once
#include <Windows.h>
#include <exception>
#include <string>

#include "Interface.hpp"

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
		*/
		long InputLatency() const 
		{
			long i, o;
			ErrorCheck(driver->getLatencies(&i, &o));
			return i;
		}

		/**
		* 出力の遅延を返す
		*/
		long OutputLatency() const 
		{
			long i, o;
			ErrorCheck(driver->getLatencies(&i, &o));
			return o;
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

		virtual ~ASIODriver()
		{
			driver->Release();
		}
	};
}