#pragma once
#include <string>

/** 変更禁止 */
#define TINY_ASIO_BUFFER_INT	0
#define TINY_ASIO_BUFFER_FLOAT	1
/** ここまで */


/**
* ここは変更できる
* TINY_ASIO_BUFFER_OPTIONに対して，任意のTINY_ASIO_BUFFER_型名を割り当てることで，
* TINY_ASIO_BUFFER_TYPEの型を決められるようにしている
*/
#define TINY_ASIO_BUFFER_OPTION TINY_ASIO_BUFFER_INT



/** 変更禁止 */
#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
#define TINY_ASIO_BUFFER_TYPE int
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
#define TINY_ASIO_BUFFER_TYPE float
#endif
/** ここまで */


/**
* UNICODE対策
*/
#ifdef UNICODE
typedef std::wstring asio_string;
#else
typedef std::string asio_string;
#endif