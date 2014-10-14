#pragma once
#include <vector>

namespace asio
{
	/**
	* �o�b�t�@�����O�p�̃��X�g
	* @tparam T �o�b�t�@�̌^
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
	* Int�^�̃o�b�t�@�����O�p�̃��X�g
	*/
	class IntBufferingList : BufferingList<int>
	{
	public:
		IntBufferingList(const void* buffer, const long size)
			: BufferingList(buffer, size) {}
	};

	/**
	* Short�^�̃o�b�t�@�����O�p�̃��X�g
	*/
	class ShortBufferingList : BufferingList<short>
	{
	public:
		ShortBufferingList(const void* buffer, const long size)
			: BufferingList(buffer, size) {}
	};

	/**
	* Float�^�̃o�b�t�@�����O�p�̃��X�g
	*/
	class FloatBufferingList : BufferingList<float>
	{
	public:
		FloatBufferingList(const void* buffer, const long size)
			: BufferingList(buffer, size) {}
	};

	/**
	* Double�^�̃o�b�t�@�����O�p�̃��X�g
	*/
	class DoubleBufferingList : BufferingList<double>
	{
	public:
		DoubleBufferingList(const void* buffer, const long size)
			: BufferingList(buffer, size) {}
	};
}