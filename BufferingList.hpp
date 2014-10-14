#pragma once
#include <vector>

namespace asio
{
	/**
	* バッファリング用のリスト
	* @tparam T バッファの型
	*/
	template <typename T = void*>
	class BufferingList
	{
		std::vector<T> buffer;

	public:
		BufferingList(const void* buffer, const long size)
			: buffer((T*)buffer, (T*)buffer + size)
		{

		}

		void Push(const BufferingList<T>& target)
		{
			buffer.insert(buffer.end(), target.buffer.begin(), target.buffer.end());
		}
	};

	/**
	* Int型のバッファリング用のリスト
	*/
	class IntBufferingList : BufferingList<int>
	{
	public:
		IntBufferingList(const void* buffer, const long size)
			: BufferingList(buffer, size) {}
	};

	/**
	* Short型のバッファリング用のリスト
	*/
	class ShortBufferingList : BufferingList<short>
	{
	public:
		ShortBufferingList(const void* buffer, const long size)
			: BufferingList(buffer, size) {}
	};

	/**
	* Float型のバッファリング用のリスト
	*/
	class FloatBufferingList : BufferingList<float>
	{
	public:
		FloatBufferingList(const void* buffer, const long size)
			: BufferingList(buffer, size) {}
	};

	/**
	* Double型のバッファリング用のリスト
	*/
	class DoubleBufferingList : BufferingList<double>
	{
	public:
		DoubleBufferingList(const void* buffer, const long size)
			: BufferingList(buffer, size) {}
	};
}