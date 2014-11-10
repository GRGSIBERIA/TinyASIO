#pragma once
#include <string>

#include "Driver.hpp"

namespace asio
{
	class ControllerBase
	{
	protected:
		Driver* driver;

	protected:
		ControllerBase(const std::string& driverName)
		{
			driver = &Driver::Init(driverName);
		}

		ControllerBase(const std::wstring& driverName)
		{
			driver = &Driver::Init(driverName);
		}
	};
}