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

		HANDLE hMutex;
		const std::wstring mutexName;
		Sample sample;

	protected:
		// スレッドセーフな処理をしたい
		template <typename FUNC>
		void Mutex(FUNC func)
		{
			auto mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexName.c_str());
			WaitForSingleObject(mutex, INFINITE);
			func();
			ReleaseMutex(mutex);
			CloseHandle(mutex);
		}

	public:
		StreamBuffer(const Sample& samplePack, const std::wstring& mutexName)
			: sample(samplePack), mutexName(mutexName) 
		{
			hMutex = CreateMutexW(NULL, FALSE, mutexName.c_str());
		}

		virtual ~StreamBuffer()
		{
			CloseHandle(hMutex);
		}
	};



	/**
	* デバイスからホストへ流すためのストリームクラス
	*/
	class DeviceToHostStream : public StreamBuffer
	{
	private:
		/**
		* バッファに追加する
		*/
		void StoreBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Int:
			{
				const int* ptr = reinterpret_cast<int*>(buffer);
				Mutex([&]() { stream.insert(stream.end(), ptr, &ptr[size]); });
				break;
			}

			default:
				throw NotImplementSampleType("サポートしていない量子化ビット数です");
			}
		}

	public:
		DeviceToHostStream(const Sample sample, const long channelNumber)
			: StreamBuffer(sample, std::wstring(L"TINY_ASIO_INPUT_") + std::to_wstring(channelNumber))
		{
			
		}

		/**
		* バッファの中身をストリームへ蓄積する
		*/
		void Store(void* buffer, const long size)
		{
			StoreBuffer(buffer, size);
		}

		std::shared_ptr<std::vector<int>> CopyAsClear()
		{
			const size_t halfSize = stream.size() >> 1;

			auto retVal = std::shared_ptr<std::vector<int>>(new std::vector<int>());

			Mutex([&]() { 
				retVal->insert(retVal->end(), stream.begin(), stream.end());
				stream.clear(); 
			});
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
				if (stream.size() >= size)					// streamの長さが十分なとき
					memcpy(buffer, stream.begin()._Ptr, size * sizeof(int));
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
			unsigned long count = bufferSize;
			if (count > stream.size())
				count = stream.size();

			Mutex([&]() { stream.erase(stream.begin(), stream.begin() + count); });
		}

	public:
		HostToDeviceStream(Sample& sample, const long channelNumber)
			: StreamBuffer(sample, std::wstring(L"TINY_ASIO_INPUT_") + std::to_wstring(channelNumber)) { }

		/**
		* ストリームの中身をバッファへ書き込む
		*/
		void Fetch(void* buffer, const long size)
		{
			memset(buffer, 0, size * sizeof(int));	// 最初にゼロ消去

			FetchBuffer(buffer, size);

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