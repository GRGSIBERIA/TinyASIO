#pragma once
#include <Windows.h>
#include <string>

/**
* UNICODEëŒçÙ
*/
#ifdef UNICODE
typedef std::wstring asio_string;
#else
typedef std::string asio_string;
#endif

namespace asio
{
	const auto MUTEX = TEXT("TINY_ASIO_MUTEX");
	const auto hMutex = CreateMutex(NULL, FALSE, MUTEX);
}