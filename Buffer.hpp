#pragma once

#include "Interface.hpp"
#include "Structure.hpp"

namespace asio
{
	/**
	* ASIO�o�b�t�@
	* @tparam T �o�b�t�@�̃f�[�^�̌^
	*/
	template <typename T>
	class ASIOBuffer
	{
		IOType ioType;
		long channelNumber;
		long bufferSize;
		T* bufferData;

	public:

	};
}