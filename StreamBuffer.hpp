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
		std::vector<int> stream;

		Sample sample;

	protected:

		void RemoveFrontFromSize(const long bufferSize)
		{
			// 先頭からbufferSizeだけ消去する
			unsigned long count;
			switch (sample.type)
			{
			case Int:
				count = bufferSize / sizeof(int);
				break;

			case Float:
				count = bufferSize / sizeof(float);
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

		inline const std::vector<int>& GetStream() const
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
		* バッファに追加する
		*/
		void StoreBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Int:
				conv::StreamConverter::ConvertToOptionType<int>(stream, buffer, size);
				break;

			case Float:
				conv::StreamConverter::ConvertToOptionType<float>(stream, buffer, size);
				break;

			default:
				throw NotImplementSampleType("サポートしていない量子化ビット数です");
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
				throw NotImplementSampleType("サポートしていない量子化ビット数です");
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
			case Int:
				conv::StreamConverter::ConvertToVoidBuffer<int>(stream, buffer, sample, size);
				break;

			case Float:
				conv::StreamConverter::ConvertToVoidBuffer<float>(stream, buffer, sample, size);
				break;

			default:
				throw NotImplementSampleType("サポートしていない量子化ビット数です");
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
				throw NotImplementSampleType("サポートしていない量子化ビット数です");

			RemoveFrontFromSize(size);
		}

		void InsertLast(const std::vector<int>& storeBuffer)
		{
			stream.insert(stream.end(), storeBuffer.begin(), storeBuffer.end());
		}
	};
}