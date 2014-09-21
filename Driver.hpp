#pragma once
#include <Windows.h>

#include "Interface.hpp"

namespace asio
{
	class ASIODriver
	{
	private:
		IASIO *driver;

	public:
		ASIODriver(const CLSID& clsid)
		{
			CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&driver);
		}

		virtual ~ASIODriver()
		{
			driver->Release();
		}
	};
}