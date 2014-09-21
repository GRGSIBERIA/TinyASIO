#pragma once
#include <Windows.h>
#include <exception>
#include <string>

#include "Interface.hpp"

namespace asio
{
	class CantCreateInstance : std::exception
	{
	public:
		CantCreateInstance(const std::string message)
			: exception(message.c_str()) {}
	};

	class ASIODriver
	{
	private:
		IASIO *driver;

	public:
		ASIODriver(const CLSID& clsid)
		{
			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&driver);
			if (FAILED(hr))
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");
		}

		virtual ~ASIODriver()
		{
			driver->Release();
		}
	};
}