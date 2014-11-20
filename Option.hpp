#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>

/**
* UNICODE‘Îô
*/
#ifdef UNICODE
typedef std::wstring asio_string;
#else
typedef std::string asio_string;
#endif

typedef std::shared_ptr<std::vector<int>> StreamingVector;