#pragma once
#include <string>

/**
* UNICODE‘Îô
*/
#ifdef UNICODE
typedef std::wstring asio_string;
#else
typedef std::string asio_string;
#endif