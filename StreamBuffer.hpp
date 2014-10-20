#pragma once
#include <vector>
#include "SamplePack.hpp"
#include "StreamConverter.hpp"

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
		/**
		* ビッグエンディアンの処理
		*/
		void ReversibleMSB(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case pack::Int:
				conv::StreamConverter::FormatBigEndian<int>(buffer, size);
				break;

			case pack::Int24:
				conv::StreamConverter::FormatBigEndian<int>(buffer, size, 3);
				break;

			case pack::Short:
				conv::StreamConverter::FormatBigEndian<short>(buffer, size);
				break;

			case pack::Float:
				conv::StreamConverter::FormatBigEndian<float>(buffer, size);
				break;

			case pack::Double:
				conv::StreamConverter::FormatBigEndian<double>(buffer, size);
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
				conv::StreamConverter::ConvertToOptionType<int>(stream, buffer, size);
				break;

			case pack::Short:
				conv::StreamConverter::ConvertToOptionType<short>(stream, buffer, size);
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
					conv::StreamConverter::ConvertToOptionType<int>(stream, reinterpret_cast<void*>(&toInt32List[0]), size);
				}
				break;

			case pack::Float:
				conv::StreamConverter::ConvertToOptionType<float>(stream, buffer, size);
				break;

			case pack::Double:
				conv::StreamConverter::ConvertToOptionType<double>(stream, buffer, size);
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