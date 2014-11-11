#pragma once
#include <vector>
#include <Windows.h>

#include "SDK.hpp"
#include "Driver.hpp"

namespace asio
{
	typedef std::shared_ptr<std::vector<int>> StreamingBuffer;

	/**
	* バッファ用のクラス
	*/
	class BufferBase
	{
	protected:
		void *buffers[2];	//!< バッファ
		long channelNumber;	//!< チャンネル番号

		StreamingBuffer stream;		//!< ストリーミング用の変数
		CRITICAL_SECTION critical;	//!< クリティカルセクション


	public:
		BufferBase(const ASIOBufferInfo& info)
			: channelNumber(info.channelNum)
		{
			buffers[0] = info.buffers[0];
			buffers[1] = info.buffers[1];

			stream = StreamingBuffer(new std::vector<int>());
			InitializeCriticalSection(&critical);
		}

		
		virtual ~BufferBase()
		{
			DeleteCriticalSection(&critical);
		}


		inline const long ChannelNumber() const { return channelNumber; }	//!< チャンネル番号
		inline void* GetBuffer(const long index) { return buffers[index]; }	//!< indexからバッファを取得する


		/**
		* バッファの中身を取り出す
		* @return バッファの中身
		*/
		StreamingBuffer Fetch()
		{
			StreamingBuffer retval = stream;
			EnterCriticalSection(&critical);
			stream = StreamingBuffer(new std::vector<int>());
			LeaveCriticalSection(&critical);
			return retval;
		}


		/**
		* バッファに値を蓄積する
		* @param[in] store 蓄積したい値
		*/
		void Store(const std::vector<int>& store)
		{
			EnterCriticalSection(&critical);
			stream->insert(stream->end(), store.begin(), store.end());
			LeaveCriticalSection(&critical);
		}
	};


	/**
	* 入力バッファ, ギターやマイクなどの入力を扱う
	*/
	class InputBuffer : public BufferBase
	{
	public:
		InputBuffer(const ASIOBufferInfo& info)
			: BufferBase(info) {}
	};


	/**
	* 出力バッファ，ヘッドフォンやスピーカーなどへ出力する
	*/
	class OutputBuffer : public BufferBase
	{
	public:
		OutputBuffer(const ASIOBufferInfo& info)
			: BufferBase(info) {}
	};


	/**
	* バッファの管理クラス
	*/
	class BufferManager
	{
		std::vector<ASIOBufferInfo> bufferInfo;

		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

	public:
		BufferManager(const long numChannels, const long bufferLength, ASIOCallbacks* callbacks)
		{
			auto* iasio = Driver::Get().Interface();
			bufferInfo = std::vector<ASIOBufferInfo>(numChannels);
			ErrorCheck(iasio->createBuffers(bufferInfo._Myfirst, numChannels, bufferLength, callbacks));

			for (long i = 0; i < numChannels; ++i)
			{
				if (bufferInfo[i].isInput)
					inputBuffers.emplace_back(bufferInfo[i]);
				else
					outputBuffers.emplace_back(bufferInfo[i]);
			}
		}
	};
}