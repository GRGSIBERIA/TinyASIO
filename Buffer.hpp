#pragma once

#include "Interface.hpp"
#include "Structure.hpp"

namespace asio
{
	/**
	* ASIOバッファ
	* @tparam T バッファのデータの型
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