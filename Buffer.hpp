#pragma once
#include <vector>
#include <Windows.h>
#include <algorithm>

#include "SDK.hpp"
#include "Driver.hpp"
#include "Channel.hpp"

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


		template <typename FUNC>
		void Critical(FUNC func)
		{
			EnterCriticalSection(&critical);
			func();
			LeaveCriticalSection(&critical);
		}


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
			Critical([&](){ stream = StreamingBuffer(new std::vector<int>()); });
			return retval;
		}


		/**
		* 元からあるバッファに転送する
		* @param[in,out] buffer 転送したいバッファ
		* @param[in] bufferLength バッファの長さ
		*/
		void Fetch(void* buffer, const long bufferLength)
		{
			Critical([&](){
				long length = bufferLength;
				if (length > stream->size())
					length = stream->size();
				memcpy(buffer, &stream->at(0), length * sizeof(int));
				stream->erase(stream->begin(), stream->begin() + length);
			});
		}


		/**
		* バッファに値を蓄積する
		* @param[in] store 蓄積したい値
		*/
		void Store(const std::vector<int>& store)
		{
			Critical([&](){stream->insert(stream->end(), store.begin(), store.end()); });
		}


		/**
		* void*からバッファに蓄積する
		* @param[in] buffer 移したいバッファ
		* @param[in] bufferLength バッファの長さ
		*/
		void Store(void* buffer, const long bufferLength)
		{
			int* ptr = reinterpret_cast<int*>(buffer);
			Critical([&](){ stream->insert(stream->end(), ptr, ptr + bufferLength); });
		}

		/**
		* チャンネル番号で比較する
		*/
		inline const bool IsChannelNumber(const long channelNumber) const
		{
			return this->channelNumber == channelNumber;
		}

		/**
		* チャンネル番号で比較する
		*/
		inline const bool IsChannelNumber(const Channel& channel) const
		{
			return channelNumber == channel.channelNumber;
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

		std::vector<BufferBase> buffers;
		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

		static std::vector<BufferBase>* buffersPtr;
		static std::vector<InputBuffer>* inputBuffersPtr;
		static std::vector<OutputBuffer>* outputBuffersPtr;

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

			inputBuffersPtr = &inputBuffers;
			outputBuffersPtr = &outputBuffers;
		}


		/**
		* バッファから対象のチャンネル番号を探してくる
		*/
		BufferBase& Search(const Channel& channel)
		{
			return *std::find_if(buffers.begin(), buffers.end(), 
				[&](const BufferBase& buffer) -> bool {
				return buffer.ChannelNumber() == channel.channelNumber; 
			});
		}

		InputBuffer& Search(const InputChannel& channel)
		{
			return *std::find_if(inputBuffers.begin(), inputBuffers.end(),
				[&](const BufferBase& buffer) -> bool {
				return buffer.ChannelNumber() == channel.channelNumber;
			});
		}

		OutputBuffer& Search(const OutputChannel& channel)
		{
			return *std::find_if(outputBuffers.begin(), outputBuffers.end(),
				[&](const BufferBase& buffer) -> bool {
				return buffer.ChannelNumber() == channel.channelNumber;
			});
		}

		static std::vector<InputBuffer>* InputBuffer() { return inputBuffersPtr; }		//!< 公開されている入力バッファを得る
		static std::vector<OutputBuffer>* OutputBuffer() { return outputBuffersPtr; }	//!< 公開されている出力バッファを得る
	};

	std::vector<BufferBase>* BufferManager::buffersPtr = nullptr;
	std::vector<InputBuffer>* BufferManager::inputBuffersPtr = nullptr;
	std::vector<OutputBuffer>* BufferManager::outputBuffersPtr = nullptr;
}