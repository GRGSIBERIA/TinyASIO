/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

TinyASIO is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

TinyASIO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TinyASIO.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

#pragma once
#include <vector>
//#include <Windows.h>
#include <algorithm>
#include <array>
#include <mutex>

#include "Option.hpp"
#include "SDK.hpp"
#include "Driver.hpp"
#include "Channel.hpp"

namespace asio
{
	class BufferManager;

	/**
	* バッファ用のクラス
	*/
	class BufferBase
	{
	protected:
		void *buffers[2];	//!< バッファ
		long channelNumber;	//!< チャンネル番号

		StreamPtr stream;		//!< ストリーミング用の変数
		
		static std::mutex critical;	//!< 共有資源を守ってるつもり

		Channel channelInfo;	//!< チャンネル情報
		bool isStart = false;

		template <typename FUNC>
		void Critical(FUNC func)
		{
			critical.lock();
			func();
			critical.unlock();
		}

		friend BufferManager;


	public:
		BufferBase(const ASIOBufferInfo& info, const Channel& channel)
			: channelNumber(info.channelNum), channelInfo(channel)
		{
			buffers[0] = info.buffers[0];
			buffers[1] = info.buffers[1];

			stream = StreamPtr(new Stream());
			//InitializeCriticalSection(&critical);
		}

		
		virtual ~BufferBase()
		{
			//DeleteCriticalSection(&critical);
			stream = nullptr;
		}


		void StartBuffering() { isStart = true; }
		void StopBuffering() { isStart = false; }


		inline const long ChannelNumber() const { return channelNumber; }	//!< チャンネル番号
		inline void* GetBuffer(const long index) { return buffers[index]; }	//!< indexからバッファを取得する
		inline const Channel& ChannelInfo() const { return channelInfo; }	//!< チャンネル情報を取得する
		inline const long StreamLength() const { return stream->size(); }	//!< ストリームの現在の長さを得る

		/**
		* バッファの中身を取り出す
		* @return バッファの中身
		*/
		StreamPtr Fetch()
		{
			StreamPtr retval = stream;
			Critical([&](){ stream = StreamPtr(new std::vector<SampleType>()); });
			return retval;
		}


		/**
		* 元からあるバッファに転送する
		* @param[in,out] buffer 転送したいバッファ
		* @param[in] bufferLength バッファの長さ
		*/
		void Fetch(void* buffer, const unsigned long bufferLength)
		{
			Critical([&](){
				unsigned long length = bufferLength;
				if (length > stream->size())
					length = stream->size();
				memcpy(buffer, &stream->at(0), length * sizeof(SampleType));
				stream->erase(stream->begin(), stream->begin() + length);
			});
		}


		/**
		* バッファに値を蓄積する
		* @param[in] store 蓄積したい値
		*/
		void Store(const Stream& store)
		{
			if (!isStart) return;
			Critical([&](){stream->insert(stream->end(), store.begin(), store.end()); });
		}


		/**
		* void*からバッファに蓄積する
		* @param[in] buffer 移したいバッファ
		* @param[in] bufferLength バッファの長さ
		*/
		void Store(void* buffer, const long bufferLength)
		{
			if (!isStart) return;
			SampleType* ptr = reinterpret_cast<SampleType*>(buffer);
			Critical([&]() { stream->insert(stream->end(), ptr, ptr + bufferLength); });
		}
		
		/**
		* バッファの中身を削除する
		*/
		void Clear()
		{
			Critical([&](){ stream->clear(); });
		}

		/**
		* チャンネル番号で比較する
		*/
		inline const bool IsChannelNumber(const long chNumber) const
		{
			return this->channelNumber == chNumber;
		}

		/**
		* チャンネル番号で比較する
		*/
		inline const bool IsChannelNumber(const Channel& channel) const
		{
			return channelNumber == channel.channelNumber;
		}

		/**
		* バッファの領域がnullじゃなかったらtrue
		*/
		inline const bool IsEnabledBuffer() const
		{
			return buffers[0] != nullptr && buffers[1] != nullptr;
		}
	};

	std::mutex BufferBase::critical;

	/**
	* 入力バッファ, ギターやマイクなどの入力を扱う
	*/
	class InputBuffer : public BufferBase
	{
	public:
		InputBuffer(const ASIOBufferInfo& info, const Channel& channel)
			: BufferBase(info, channel) {}
	};


	/**
	* 出力バッファ，ヘッドフォンやスピーカーなどへ出力する
	*/
	class OutputBuffer : public BufferBase
	{
	public:
		OutputBuffer(const ASIOBufferInfo& info, const Channel& channel)
			: BufferBase(info, channel) {}
	};

	class ControllerBase;

	/**
	* バッファの管理クラス
	*/
	class BufferManager
	{
		std::vector<ASIOBufferInfo> bufferInfo;

		std::vector<BufferBase*> buffers;
		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

		static std::vector<InputBuffer>* inputPtr;
		static std::vector<OutputBuffer>* outputPtr;

		bool disposed;

		friend ControllerBase;

		

	private:
		template <typename VECTOR_ARRAY>
		void InitBufferInfo(const VECTOR_ARRAY& channels)
		{
			bufferInfo = std::vector<ASIOBufferInfo>(channels.size());

			for (unsigned int i = 0; i < channels.size(); ++i)
			{
				bufferInfo[i].channelNum = channels[i].channelNumber;
				bufferInfo[i].isInput = channels[i].isInput;
			}
		}

		void InitBuffers(const std::vector<Channel>& channels, const long bufferLength, ASIOCallbacks* callbacks)
		{
			auto* iasio = Driver::Get().Interface();
			ErrorCheck(iasio->createBuffers(&bufferInfo[0], bufferInfo.size(), bufferLength, callbacks));

			for (unsigned long i = 0; i < bufferInfo.size(); ++i)
			{
				BufferBase* ptr;
				if (bufferInfo[i].isInput)
				{
					inputBuffers.emplace_back(bufferInfo[i], channels[i]);
					ptr = &inputBuffers.back();
				}
				else
				{
					outputBuffers.emplace_back(bufferInfo[i], channels[i]);
					ptr = &outputBuffers.back();
				}
				buffers.push_back(ptr);
			}

			inputPtr = &inputBuffers;
			outputPtr = &outputBuffers;
		}

	public:
		void DisposeBuffer()
		{
			if (this != nullptr)	// nullなのにDisposeBufferが呼ばれることもあると思うので回避する
			{
				if (!disposed)
				{
					Driver::Get().Interface()->disposeBuffers();
					disposed = true;
					inputPtr = nullptr;
					outputPtr = nullptr;
				}
			}
		}

		virtual ~BufferManager()
		{
			DisposeBuffer();
		}

		BufferManager(const std::vector<Channel>& channels, const long bufferLength, ASIOCallbacks* callbacks)
			: disposed(false)
		{
			InitBufferInfo(channels);
			InitBuffers(channels, bufferLength, callbacks);
		}

		template <size_t NUM>
		BufferManager(const std::array<Channel, NUM>& channels, const long bufferLength, ASIOCallbacks* callbacks)
			: disposed(false)
		{
			InitBufferInfo(channels);
			InitBuffers(channels, bufferLength, callbacks);
		}

		/**
		* バッファリングされている入力チャンネルを探す
		* もっとも最初に見つかったものが返される
		* @note バッファへのvoid*がnullptrじゃないものを取得する
		*/
		InputBuffer& SearchBufferableInput()
		{
			return *std::find_if(inputBuffers.begin(), inputBuffers.end(),
				[](const BufferBase& buffer) -> bool {
				return buffer.IsEnabledBuffer();
			});
		}

		/**
		* バッファリングされている出力チャンネルを探す
		* もっとも最初に見つかったものが返される
		* @note バッファへのvoid*がnullptrじゃないものを取得する
		*/
		OutputBuffer& SearchBufferableOutput()
		{
			return *std::find_if(outputBuffers.begin(), outputBuffers.end(),
				[](const BufferBase& buffer) -> bool {
				return buffer.IsEnabledBuffer();
			});
		}

		void StartBuffering()
		{
			for (auto& in : inputBuffers)
				in.StartBuffering();
			for (auto& out : outputBuffers)
				out.StartBuffering();
		}

		void StopBuffering()
		{
			for (auto& in : inputBuffers)
				in.StopBuffering();
			for (auto& out : outputBuffers)
				out.StopBuffering();
		}

		/**
		* BufferSwitchから入力バッファを得るための関数
		*/
		static InputBuffer& Input(const size_t index) { return inputPtr->at(index); }

		/**
		* 入力バッファの総数
		*/
		static const size_t InputSize() { return inputPtr->size(); }

		/**
		* BufferSwitchから出力バッファを得るための関数
		*/
		static OutputBuffer& Output(const size_t index) { return outputPtr->at(index); }
		
		/**
		* 出力バッファの総数
		*/
		static const size_t OutputSize() { return outputPtr->size(); }

		static std::vector<InputBuffer>& Inputs() { return *inputPtr; }		// 入力バッファ配列を得る
		static std::vector<OutputBuffer>& Outputs() { return *outputPtr; }	// 出力バッファ配列を得る
	};

	std::vector<InputBuffer>* BufferManager::inputPtr = nullptr;
	std::vector<OutputBuffer>* BufferManager::outputPtr = nullptr;
}