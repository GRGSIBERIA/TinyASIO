#pragma once
#include <vector>

#include "SDK.hpp"
#include "Driver.hpp"

namespace asio
{
	/**
	* バッファ用のクラス
	*/
	class BufferBase
	{
		void *buffers[2];
		long channelNumber;

	public:
		BufferBase(const ASIOBufferInfo& info)
			: channelNumber(info.channelNum)
		{
			buffers[0] = info.buffers[0];
			buffers[1] = info.buffers[1];
		}
	};


	class InputBuffer : public BufferBase
	{
	public:
		InputBuffer(const ASIOBufferInfo& info)
			: BufferBase(info) {}
	};


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