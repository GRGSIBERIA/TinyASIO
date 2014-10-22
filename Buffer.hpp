#pragma once
#include <vector>
#include <memory>
#include <algorithm>

#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"
#include "Channel.hpp"
#include "SamplePack.hpp"
#include "StreamBuffer.hpp"

namespace asio
{
	class CallbackManager;

	/**
	* バッファクラス
	*/
	class Buffer
	{
		friend CallbackManager;

	protected:
		IOType ioType;
		long channelNumber;
		long bufferSize;
		const ASIOSampleType sampleType;
		void* bufferData[2];

	public:
		/**
		* IOの種類を返す
		*/
		inline const IOType Type() const { return ioType; }

	public:
		Buffer(const ASIOBufferInfo& info, const long bufferSize)
			:
			ioType((IOType)info.isInput),
			channelNumber(info.channelNum),
			bufferSize(bufferSize),
			sampleType(sampleType)
		{
			bufferData[0] = info.buffers[0];
			bufferData[1] = info.buffers[1];
		}
	};

	/**
	* 入力バッファ管理クラス
	*/
	class InputBuffer : public Buffer
	{
		DeviceToHostStream stream;

	public:
		InputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize), stream(pack::DetectSampleTypePackStruct(sampleType)) {}

		void Read() {}

		/**
		* Streamに入力されたデータを蓄積する
		*/
		inline void Store(const long bufferIndex) 
		{
			stream.Store(bufferData[bufferIndex], bufferSize);
		}
	};

	/**
	* 出力バッファ管理クラス
	*/
	class OutputBuffer : public Buffer
	{
		HostToDeviceStream stream;

	public:
		OutputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize), stream(pack::DetectSampleTypePackStruct(sampleType)) {}

		void Write() {}

		/**
		* Streamに蓄積されたデータを転送する
		*/
		inline void Fetch(const long bufferIndex)
		{
			stream.Fetch(bufferData[bufferIndex], bufferSize);
		}
	};



	class BufferManager;	// コピー不可にするための仕組み

	/**
	* コールバック関数とバッファリングをまとめるためのクラス
	*/
	class BufferController
	{
		friend BufferManager;

		std::vector<Buffer*> buffers;	// callback::CallbackManagerにポインタを渡してる

		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

		IASIO* iasio;

	private:
		/**
		* 無闇矢鱈とコピーされていい存在じゃない
		*/
		BufferController(IASIO* iasio)
			: iasio(iasio) {}

		BufferController(const BufferController& c) {}

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
}