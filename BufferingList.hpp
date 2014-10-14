#pragma once
#include <vector>

namespace asio
{
	/**
	* �o�b�t�@�����O�p�̃��X�g
	* @tparam T �o�b�t�@�̌^
	*/
	template <typename T>
	class BufferingList
	{
		std::vector<T> buffer;

	public:
		BufferingList(const void* buffer, const long size)
			: buffer((T*)buffer, (T*)buffer + size)
		{

		}

		void Store(const BufferingList<T>& target)
		{
			buffer.insert(buffer.end(), target.buffer.begin(), target.buffer.end());
		}

		std::vector<T> Spend()
		{
			auto buf = std::vector<T>(buffer.begin(), buffer.end());
			buffer.clear();
			return buf;
		}
	};
}