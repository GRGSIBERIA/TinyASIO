#pragma once
#include <string>

/** �ύX�֎~ */
#define TINY_ASIO_BUFFER_INT	0
#define TINY_ASIO_BUFFER_FLOAT	1
/** �����܂� */


/**
* �����͕ύX�ł���
* TINY_ASIO_BUFFER_OPTION�ɑ΂��āC�C�ӂ�TINY_ASIO_BUFFER_�^�������蓖�Ă邱�ƂŁC
* TINY_ASIO_BUFFER_TYPE�̌^�����߂���悤�ɂ��Ă���
*/
#define TINY_ASIO_BUFFER_OPTION TINY_ASIO_BUFFER_INT



/** �ύX�֎~ */
#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
#define TINY_ASIO_BUFFER_TYPE int
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
#define TINY_ASIO_BUFFER_TYPE float
#endif
/** �����܂� */


/**
* UNICODE�΍�
*/
#ifdef UNICODE
typedef std::wstring asio_string;
#else
typedef std::string asio_string;
#endif