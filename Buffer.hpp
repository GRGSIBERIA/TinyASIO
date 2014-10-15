#pragma once
#include <vector>
#include <memory>
#include <algorithm>

#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"
#include "Channel.hpp"
#include "SamplePack.hpp"
#include "BufferList.hpp"

namespace asio
{
	class BufferController;

	/**
	* バッファクラス
	*/
	class Buffer
	{
		friend BufferController;

		IOType ioType;
		long channelNumber;
		long bufferSize;
		const ASIOSampleType sampleType;
		void* bufferData[2];

		BufferList bufferList;

	public:
		/**
		* IOの種類を返す
		*/
		inline const IOType Type() const { return ioType; }

	public:
		Buffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			:
			ioType((IOType)info.isInput),
			channelNumber(info.channelNum),
			bufferSize(bufferSize),
			sampleType(sampleType),
			bufferList(pack::DetectSampleTypePackStruct(sampleType))
		{
			bufferData[0] = info.buffers[0];
			bufferData[1] = info.buffers[1];
		}

		void Store(const long index, const long size)
		{
			bufferList.Store(bufferData[index], size);
		}
	};

	/**
	* 入力バッファ管理クラス
	*/
	class InputBuffer : public Buffer
	{
	public:
		InputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize, sampleType) {}
	};

	/**
	* 出力バッファ管理クラス
	*/
	class OutputBuffer : public Buffer
	{
	public:
		OutputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize, sampleType) {}
	};

	class BufferManager;	// フレンドにするための前方宣言

	/**
	* コールバック関数とバッファリングをまとめるためのクラス
	*/
	class BufferController
	{
		friend BufferManager;

		static std::vector<Buffer*> buffers;

		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

		IASIO* iasio;

	private:
		BufferController(IASIO* iasio)
			: iasio(iasio) {}

	private:
		void Add(const ASIOBufferInfo& info, const long& bufferSize, const ASIOSampleType& sampleType)
		{
			Buffer* ptr = nullptr;

			if (info.isInput)
			{
				inputBuffers.emplace_back(info, bufferSize, sampleType);
				ptr = &inputBuffers[inputBuffers.size() - 1];
			}
			else
			{
				outputBuffers.emplace_back(info, bufferSize, sampleType);
				ptr = &outputBuffers[outputBuffers.size() - 1];
			}
			buffers.push_back(ptr);
		}

		void Clear()
		{
			buffers.clear();
			inputBuffers.clear();
			outputBuffers.clear();
		}

	public:

		/**
		* バッファリング開始
		*/
		inline void Start() const
		{
			ErrorCheck(iasio->start());
		}

		/**
		* バッファリング停止
		*/
		inline void Stop() const
		{
			ErrorCheck(iasio->stop());
		}

		/**
		* 入力バッファの配列を取得する
		* @return 入力バッファの配列
		*/
		inline const std::vector<InputBuffer> InputBuffers() const { return inputBuffers; }

		/**
		* 出力バッファの配列を取得する
		* @return 出力バッファの配列
		*/
		inline const std::vector<OutputBuffer> OutputBuffers() const { return outputBuffers; }
	};

	std::vector<Buffer*> BufferController::buffers;

	

	
}