#pragma once
#include <vector>
#include "Option.hpp"

namespace asio
{
	namespace conv
	{
		void IntToOptionType(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const long size)
		{
			const long count = size / sizeof(int);

#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
			source.insert(source.end(), reinterpret_cast<int*>(buffer), reinterpret_cast<int*>(buffer) + count);
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
			
			const long prevSize = source.size();
			source.resize(prevSize + count);

			const float MAX_DIFF = 1.0 / 2147483647.0;
			for (long i = 0; i < count; ++i)
			{
				source[prevSize + i] = *(reinterpret_cast<int*>(buffer)+i) * MAX_DIFF;
			}
#endif
		}

		void FloatToOptionType(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const long size)
		{
			const long count = size / sizeof(float);
		}
	}
}