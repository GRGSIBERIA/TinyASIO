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

#include "SDK.hpp"

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

#define ASIO_CURRENT_SAMPLE_TYPE ASIOSTInt32LSB	//!< ���݁C���C�u�����I�ɋ������Ă���T���v���^

	typedef int SampleType;	//!< �T���v�����O�������̌^

	typedef std::vector<SampleType> Stream;
	typedef std::shared_ptr<Stream> StreamPtr;	//!< �o�b�t�@�����O�������Ɏ擾����^
}