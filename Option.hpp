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
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>

namespace asio
{
	/**
	* UNICODE�΍�
	*/
#ifdef UNICODE
	typedef std::wstring asio_string;
#else
	typedef std::string asio_string;
#endif

	typedef std::shared_ptr<std::vector<int>> StreamingVector;
}