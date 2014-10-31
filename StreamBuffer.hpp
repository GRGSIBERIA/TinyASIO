#pragma once
#include <vector>
#include "SamplePack.hpp"
#include "StreamConverter.hpp"

namespace asio
{
	/**
	* どういうわけか知らん型が投げられた場合の例外
	*/
	class UnrecognizedTypeException : public std::exception
	{
	public:
		UnrecognizedTypeException(const std::string& message)
			: std::exception(message.c_str()) {}
	};


	/**
	* バッファリングするためのストリームクラス
	* @note 子クラスは基本的にBufferControllerから見えなくなっているので，そんなに公開・非公開は気にしなくてもいいと思う
	*/
	class StreamBuffer
	{
	protected:
		std::vector<TINY_ASIO_BUFFER_TYPE> stream;

		Sample sample;

	protected:
		/**
		* ビッグエンディアンの処理
		*/
		void ReversibleMSB(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Int:
				conv::StreamConverter::FormatBigEndian<int>(buffer, size);
				break;

			case Int24:
				conv::StreamConverter::FormatBigEndian<int>(buffer, size, 3);
				break;

			case Short:
				conv::StreamConverter::FormatBigEndian<short>(buffer, size);
				break;

			case Float:
				conv::StreamConverter::FormatBigEndian<float>(buffer, size);
				break;

			case Double:
				conv::StreamConverter::FormatBigEndian<double>(buffer, size);
				break;

			default:
				throw UnrecognizedTypeException("利用不可能な量子化ビット数が指定されています");
			}
		}


		void RemoveFrontFromSize(const long bufferSize)
		{
			// 先頭からbufferSizeだけ消去する
			unsigned long count;
			switch (sample.type)
			{
			case Short:
				count = bufferSize / sizeof(short);
				break;

			case Int:
				count = bufferSize / sizeof(int);
				break;

			case Int24:
				count = bufferSize / 3;
				break;

			case Float:
				count = bufferSize / sizeof(float);
				break;

			case Double:
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
		StreamBuffer(Sample& samplePack)
			: sample(samplePack) {}


		inline void Clear()
		{
			stream.clear();
		}

		inline const std::vector<TINY_ASIO_BUFFER_TYPE>& GetStream() const
		{
			return stream;
		}
	};



	/**
	* デバイスからホストへ流すためのストリームクラス
	*/
	class DeviceToHostStream : public StreamBuffer
	{
		/**
		* 量子化ビット数が24bitの場合の特殊な処理
		*/
		void ConvertTo24Bit(void* buffer, const long size)
		{
			// ここでInt24bitからInt32bitに変換する
			const long count = size / 3;
			const long resize = count * 4;	// 24bit -> 32bitのサイズ
			std::vector<int> toInt32List(count);
			for (int i = 0; i < count; ++i)
			{
				BYTE* bytePtr = reinterpret_cast<BYTE*>(buffer);	// バイト型のポインタで位置を調整する
				toInt32List[i] = *reinterpret_cast<int*>(bytePtr[i * 3]) & 0xFFFFFF;	// 24ビットマスクをかける
			}
			conv::StreamConverter::ConvertToOptionType<int>(stream, reinterpret_cast<void*>(&toInt32List[0]), resize);
		}

		/**
		* バッファに追加する
		*/
		void StoreBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Int:
				conv::StreamConverter::ConvertToOptionType<int>(stream, buffer, size);
				break;

			case Short:
				conv::StreamConverter::ConvertToOptionType<short>(stream, buffer, size);
				break;

			case Int24:
				ConvertTo24Bit(buffer, size);
				break;

			case Float:
				conv::StreamConverter::ConvertToOptionType<float>(stream, buffer, size);
				break;

			case Double:
				conv::StreamConverter::ConvertToOptionType<double>(stream, buffer, size);
				break;

			default:
				throw UnrecognizedTypeException("利用不可能な量子化ビット数が指定されています");
			}
		}

	public:
		DeviceToHostStream(Sample sample)
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
			switch (sample.type)
			{
			case Short:
				conv::StreamConverter::ConvertToVoidBuffer<short>(stream, buffer, sample, size);
				break;

			case Int:
				conv::StreamConverter::ConvertToVoidBuffer<int>(stream, buffer, sample, size);
				break;

			case Int24:
				conv::StreamConverter::ConvertToVoidBuffer<int>(stream, buffer, sample, size);
				break;

			case Float:
				conv::StreamConverter::ConvertToVoidBuffer<float>(stream, buffer, sample, size);
				break;

			case Double:
				conv::StreamConverter::ConvertToVoidBuffer<double>(stream, buffer, sample, size);
				break;

			default:
				throw UnrecognizedTypeException("利用不可能な量子化ビット数が指定されています");
			}
		}

	public:
		HostToDeviceStream(Sample& sample)
			: StreamBuffer(sample) { }

		/**
		* ストリームの中身をバッファへ書き込む
		*/
		void Fetch(void* buffer, const long size)
		{
			memset(buffer, 0, size);	// 最初にゼロ消去

			FetchBuffer(buffer, size);
			if (sample.isMSB)			// 一番最後にエンディアンを逆転させる
				ReversibleMSB(buffer, size);

			RemoveFrontFromSize(size);
		}

		void InsertLast(const std::vector<TINY_ASIO_BUFFER_TYPE>& storeBuffer)
		{
			stream.insert(stream.end(), storeBuffer.begin(), storeBuffer.end());
		}
	};
}