#pragma once
#include <vector>
#include "SamplePack.hpp"

namespace asio
{
	/**
	* バッファリングするためのリスト
	*/
	class BufferList
	{
		std::vector<float> floatBuffer;
		std::vector<int> intBuffer;
		std::vector<double> doubleBuffer;
		std::vector<short> shortBuffer;

		pack::Sample sample;

	private:

		template <typename T>
		void Insert(std::vector<T> buffer, void* vbuffer, const long size)
		{
			buffer.insert(buffer.end(), reinterpret_cast<T*>(vbuffer), reinterpret_cast<T*>(vbuffer)+size);
		}

		template <typename T>
		void ReverseEndian(T* p)
		{
			std::reverse(
				reinterpret_cast<BYTE*>(p),
				reinterpret_cast<BYTE*>(p)+sizeof(T));
		}

		template <typename T>
		void FormatBigEndian(void* buffer, const long size)
		{
			T *start = reinterpret_cast<T*>(buffer);
			const size_t num = size / sizeof(T);
			for (size_t i = 0; i < num; ++i)
			{
				ReverseEndian(start + i * sizeof(T));
			}
		}

		/**
		* ビッグエンディアンの処理
		*/
		void ReversibleMSB(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case pack::Int:
				FormatBigEndian<int>(buffer, size);
				break;

			case pack::Short:
				FormatBigEndian<short>(buffer, size);
				break;

			case pack::Float:
				FormatBigEndian<float>(buffer, size);
				break;

			case pack::Double:
				FormatBigEndian<double>(buffer, size);
				break;
			}
		}

		/**
		* バッファに追加する
		*/
		void StoreBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case pack::Int:
				Insert(intBuffer, buffer, size);
				break;

			case pack::Short:
				Insert(shortBuffer, buffer, size);
				break;

			case pack::Float:
				Insert(floatBuffer, buffer, size);
				break;

			case pack::Double:
				Insert(doubleBuffer, buffer, size);
				break;
			}
		}

	public:
		BufferList(pack::Sample& sample)
			: sample(sample) { }

		/**
		* バッファに蓄積する
		*/
		void Store(void* buffer, const long size)
		{
			if (sample.isMSB)
				ReversibleMSB(buffer, size);
			StoreBuffer(buffer, size);
		}
	};
}