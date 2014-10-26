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


	namespace callback 
	{
		class CallbackManager;
	}

	/**
	* 入力バッファ管理クラス
	*/
	class InputBuffer : public Buffer
	{
		friend callback::CallbackManager;

		DeviceToHostStream stream;

		/**
		* Streamに入力されたデータを蓄積する
		* @note CallbackManager用の関数
		*/
		inline void StoreC(const long bufferIndex)
		{
			stream.Store(bufferData[bufferIndex], bufferSize);
		}

	public:
		InputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize), stream(pack::DetectSampleTypePackStruct(sampleType)) {}

		/**
		* バッファに蓄積されたデータを取得する
		* @return バッファに蓄積されたデータ
		*/
		std::shared_ptr<std::vector<TINY_ASIO_BUFFER_TYPE>> Fetch()
		{
			auto sharedPtr = std::make_shared < std::vector<TINY_ASIO_BUFFER_TYPE>>(stream.GetStream());
			stream.Clear();
			return sharedPtr;
		}
	};


	/**
	* 出力バッファ管理クラス
	*/
	class OutputBuffer : public Buffer
	{
		friend callback::CallbackManager;

		HostToDeviceStream stream;

		/**
		* Streamに蓄積されたデータを転送する
		* @note CallbackManager用の関数
		*/
		inline void FetchC(const long bufferIndex)
		{
			stream.Fetch(bufferData[bufferIndex], bufferSize);
		}

	public:
		OutputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize), stream(pack::DetectSampleTypePackStruct(sampleType)) {}

		/**
		* バッファにデータを蓄積する
		* @param[in] storeBuffer 蓄積したいデータ
		*/
		void Store(const std::vector<TINY_ASIO_BUFFER_TYPE>& storeBuffer) 
		{
			stream.InsertLast(storeBuffer);
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
		* 入力バッファのインスタンスを得る
		* @return 入力バッファ
		*/
		inline InputBuffer& InputBuffer(const unsigned int index) { return inputBuffers[index]; }

		/**
		* 出力バッファのインスタンスを得る
		* @return 出力バッファ
		*/
		inline OutputBuffer& OutputBuffer(const unsigned int index) { return outputBuffers[index]; }

		/**
		* 入力バッファの数を得る
		* @return 入力バッファの数
		*/
		inline const size_t InputCount() const { return inputBuffers.size(); }

		/**
		* 出力バッファの数を得る
		* @return 出力バッファの数
		*/
		inline const size_t OutputCount() const { return outputBuffers.size(); }
	};
}