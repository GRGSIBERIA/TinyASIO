#pragma once
#include <memory>
#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"

namespace asio
{
	/**
	* ASIOバッファ
	* @tparam T バッファのデータの型
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