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
		CantCreateInstance(const std::string message)
			: exception(message.c_str()) {}
	};

	class ASIODriver
	{
	private:
		IASIO *driver;			// インターフェースへのポインタ
		void *systemHandle;		// 謎のシステムハンドル

		std::string driverName;
		long driverVersion;

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

	public:
		ASIODriver(const CLSID& clsid)
		{
			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&driver);
			if (FAILED(hr))
				throw CantCreateInstance("ドライバのインスタンス生成に失敗しました");

			driver->init(systemHandle);

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