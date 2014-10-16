#pragma once
#include <vector>
#include "SamplePack.hpp"
#include "Option.hpp"

namespace asio
{
	/**
	* バッファ用のストリーム
	*/
	class StreamBuffer
	{
		std::vector<TINY_ASIO_BUFFER_TYPE> stream;

		pack::Sample sample;

	private:

		template <typename T>
		void Insert(void* vbuffer, const long size)
		{
			stream.insert(buffer.end(), reinterpret_cast<T*>(vbuffer), reinterpret_cast<T*>(vbuffer)+size);
		}

		template <typename T>
		void ReverseEndian(T* p)
		{
			std::reverse(
				reinterpret_cast<BYTE*>(p),
				reinterpret_cast<BYTE*>(p)+sizeof(T));
		}

		template <typename T>
		void FormatBigEndian(void* buffer, const long size, const long typeSize = sizeof(T))
		{
			T *start = reinterpret_cast<T*>(buffer);
			const size_t num = size / sizeof(T);
			for (size_t i = 0; i < num; ++i)
			{
				ReverseEndian(start + i * typeSize);
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

			case pack::Int24:
				FormatBigEndian<int>(buffer, size, 3);
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
				
				break;

			case pack::Short:
				
				break;

			case pack::Int24:
				{
					// ここでInt24bitからInt32bitに変換する
					const long count = size / 3;
					const long resize = count * 4;	// 24bit -> 32bitのサイズ
					std::vector<int> toInt32List(count);
					for (int i = 0; i < count; ++i)
					{
						BYTE* bytePtr = reinterpret_cast<BYTE*>(buffer) + i * 3;	// バイト型のポインタで位置を調整する
						toInt32List[i] = *reinterpret_cast<int*>(bytePtr) & 0xFFFFFF;	// 24ビットマスクをかける
					}
					
				}
				break;

			case pack::Float:
				
				break;

			case pack::Double:
				
				break;
			}
		}

	public:
		StreamBuffer(pack::Sample& sample)
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