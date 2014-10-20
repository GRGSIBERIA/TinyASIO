#pragma once
#include <vector>
#include "SamplePack.hpp"
#include "StreamConverter.hpp"

namespace asio
{
	class UnrecognizedTypeException : public std::exception
	{
	public:
		UnrecognizedTypeException(const std::string& message)
			: std::exception(message.c_str()) {}
	};


	/**
	* バッファリングするためのストリームクラス
	*/
	class StreamBuffer
	{
	protected:
		std::vector<TINY_ASIO_BUFFER_TYPE> stream;

		pack::Sample sample;

	protected:
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

			default:
				throw UnrecognizedTypeException("利用不可能な量子化ビット数が指定されています");
			}
		}


		void RemoveFront(const long bufferSize)
		{
			// 先頭からbufferSizeだけ消去する
			long count;
			switch (sample.type)
			{
			case pack::Short:
				count = bufferSize / sizeof(short);
				break;

			case pack::Int:
				count = bufferSize / sizeof(int);
				break;

			case pack::Int24:
				count = bufferSize / 3;
				break;

			case pack::Float:
				count = bufferSize / sizeof(float);
				break;

			case pack::Double:
				count = bufferSize / sizeof(double);
				break;

			default:
				throw UnrecognizedTypeException("利用不可能な量子化ビット数が指定されています");
			}

			if (count > stream.size())
				count = stream.size();

			stream.erase(stream.begin(), stream.begin() + count);
		}

	public:
		StreamBuffer(pack::Sample& samplePack)
			: sample(samplePack) {}
	};


	/**
	* デバイスからホストへ流すためのストリームクラス
	*/
	class DeviceToHostStream : public StreamBuffer
	{
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
					BYTE* bytePtr = reinterpret_cast<BYTE*>(buffer)+i * 3;	// バイト型のポインタで位置を調整する
					toInt32List[i] = *reinterpret_cast<int*>(bytePtr)& 0xFFFFFF;	// 24ビットマスクをかける
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

			default:
				throw UnrecognizedTypeException("利用不可能な量子化ビット数が指定されています");
			}
		}

	public:
		DeviceToHostStream(pack::Sample& sample)
			: StreamBuffer(sample) { }

		/**
		* バッファの中身をストリームへ蓄積する
		*/
		void Store(void* buffer, const long size)
		{
			if (sample.isMSB)
				ReversibleMSB(buffer, size);
			StoreBuffer(buffer, size);
		}

		
	};


	/**
	* ホストからデバイスに送るためのストリームクラス
	*/
	class HostToDeviceStream : public StreamBuffer
	{
		void FetchBuffer(void* buffer, const long size)
		{
			memset(buffer, 0, size);	// 最初にゼロ消去

			switch (sample.type)
			{
			case pack::Short:
				break;

			case pack::Int:
				break;

			case pack::Int24:
				break;

			case pack::Float:
				break;

			case pack::Double:
				break;

			default:
				throw UnrecognizedTypeException("利用不可能な量子化ビット数が指定されています");
			}

			if (sample.isMSB)			// 一番最後にエンディアンを逆転させる
				ReversibleMSB(buffer, size);
		}

	public:
		HostToDeviceStream(pack::Sample& sample)
			: StreamBuffer(sample) { }

		/**
		* ストリームの中身をバッファへ書き込む
		*/
		void Fetch(void* buffer, const long size)
		{

		}
	};
}