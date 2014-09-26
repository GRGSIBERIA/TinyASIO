#pragma once
#include <string>
#include "Interface.hpp"

namespace asio
{
	enum IOType
	{
		Input = 1,
		Output = 0
	};

	/**
	* �o�b�t�@�̌��݂̐ݒ��\���\����
	*/
	struct BufferPreference
	{
		long maxSize;		// �o�b�t�@�T�C�Y�̍ő�l
		long minSize;		// �o�b�t�@�T�C�Y�̍ŏ��l
		long preferredSize;	// �œK�Ȓl
		long granularity;	// �ݒ�̗��x
	};
}