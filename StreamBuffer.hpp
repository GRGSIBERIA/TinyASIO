#pragma once
#include <vector>
#include <Windows.h>
#include "SamplePack.hpp"
#include "Option.hpp"

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
	* Option.hppで指定された型が変だったときに呼び出される
	*/
	class UnknownOptionType : public std::exception
	{
	public:
		UnknownOptionType(const std::string& message)
			: exception(message.c_str()) { }
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
		// スレッドセーフな処理をしたい
		template <typename FUNC>
		void Mutex(FUNC func)
		{
			auto mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX);
			WaitForSingleObject(mutex, INFINITE);
			func();
			ReleaseMutex(mutex);
			CloseHandle(mutex);
		}

	public:
		StreamBuffer(Sample& samplePack)
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
			case Int:
			{
				const size_t count = size / sizeof(int);
				const int* ptr = reinterpret_cast<int*>(buffer);
				Mutex([&]() { stream.insert(stream.end(), ptr, &ptr[count]); });
				break;
			}

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
				throw NotImplementSampleType("ビッグエンディアンはサポートしていません");
			StoreBuffer(buffer, size);
		}

		std::shared_ptr<std::vector<int>> CopyAsClear()
		{
			auto retVal = std::shared_ptr<std::vector<int>>(new std::vector<int>(stream));
			Mutex([&]() { stream.clear(); });
			return retVal;
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
			{
				const size_t count = size / sizeof(int);
				if (stream.size() >= count)					// streamの長さが十分なとき
					memcpy(buffer, stream.begin()._Ptr, count * sizeof(int));
				else if (stream.size() > 0)					// count未満の場合
					memcpy(buffer, stream.begin()._Ptr, stream.size() * sizeof(int));
				break;
			}

			default:
				throw NotImplementSampleType("サポートしていない量子化ビット数です");
			}
		}

		void RemoveFrontFromSize(const long bufferSize)
		{
			// 先頭からbufferSizeだけ消去する
			unsigned long count;
			switch (sample.type)
			{
			case Int:
				count = bufferSize / sizeof(int);
				break;

			default:
				throw UnrecognizedTypeException("利用不可能な量子化ビット数が指定されています");
			}

			if (count > stream.size())
				count = stream.size();

			Mutex([&]() { stream.erase(stream.begin(), stream.begin() + count); });
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
				throw NotImplementSampleType("ビッグエンディアンはサポートしていません");

			RemoveFrontFromSize(size);
		}

		/**
		* ストリームの最後に，指定した配列を追加する
		*/
		void InsertLast(const std::vector<int>& storeBuffer)
		{
			Mutex([&]() {stream.insert(stream.end(), storeBuffer.begin(), storeBuffer.end()); });
		}
	};
}