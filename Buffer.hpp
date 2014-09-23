#pragma once
#include <memory>
#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"

namespace asio
{
	/**
	* ASIO�o�b�t�@
	* @tparam T �o�b�t�@�̃f�[�^�̌^
	*/
	class ASIOBuffer
	{
		IOType ioType;
		long channelNumber;
		long bufferSize;
		void* bufferData;

	public:
		ASIOBuffer(ASIOBufferInfo* info)
		{

		}
	};
}