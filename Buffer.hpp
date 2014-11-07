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
#include "Preference.hpp"

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
		Buffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			:
			ioType((IOType)info.isInput),
			channelNumber(info.channelNum),
			bufferSize(bufferSize),
			sampleType(sampleType)
		{
			bufferData[0] = info.buffers[0];
			bufferData[1] = info.buffers[1];
		}

		static void DirectCopy(const long index, const Buffer& source, Buffer& dest, const size_t size)
		{
			memcpy(dest.bufferData[index], source.bufferData[index], size);
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
			: Buffer(info, bufferSize, sampleType), stream(DetectSampleTypePackStruct(sampleType), info.channelNum) {}

		/**
		* バッファに蓄積されたデータを取得する
		* @return バッファに蓄積されたデータ
		*/
		std::shared_ptr<std::vector<int>> Fetch()
		{
			return stream.CopyAsClear();
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
			: Buffer(info, bufferSize, sampleType), stream(DetectSampleTypePackStruct(sampleType), info.channelNum) {}

		/**
		* バッファにデータを蓄積する
		* @param[in] storeBuffer 蓄積したいデータ
		*/
		inline void Store(const std::vector<int>& storeBuffer) 
		{
			stream.InsertLast(storeBuffer);
		}

		/**
		* バッファにデータを蓄積する
		* @param[in] storeBuffer 蓄積したいデータ
		*/
		inline void Store(const std::shared_ptr<std::vector<int>>& storeBuffer)
		{
			Store(*storeBuffer);
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
		const Preference preference;

	private:
		/**
		* 無闇矢鱈とコピーされていい存在じゃない
		*/
		BufferController(IASIO* iasio, const long& bufferSize)
			: iasio(iasio), preference(iasio, bufferSize) {}

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
		* インスタンスが破棄された時にバッファを解放する
		*/
		~BufferController()
		{
			//if (buffers.size() > 0)
				//ErrorCheck(iasio->disposeBuffers());
		}

		/**
		* ドライバの設定を返す
		*/
		inline const Preference& DriverPreference() const { return preference; }

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
		* @params[in] index 添字
		* @return 入力バッファ
		*/
		inline InputBuffer& InputBuffer(const unsigned int index) { return inputBuffers[index]; }

		/**
		* 出力バッファのインスタンスを得る
		* @params[in] index 添字
		* @return 出力バッファ
		*/
		inline OutputBuffer& OutputBuffer(const unsigned int index) { return outputBuffers[index]; }


		/**
		* 出力バッファの配列を得る
		* @return 出力バッファの配列
		*/
		const std::vector<asio::OutputBuffer>& OutputBuffers() { return outputBuffers; }


		/**
		* 入力バッファの配列を得る
		* @return 入力バッファの配列
		*/
		const std::vector<asio::InputBuffer>& InputBuffers() { return inputBuffers; }


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