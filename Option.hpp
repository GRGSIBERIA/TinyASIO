#pragma once
#include <string>

/**
* UNICODE�΍�
*/
#ifdef UNICODE
typedef std::wstring asio_string;
#else
typedef std::string asio_string;
#endif