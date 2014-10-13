#pragma once
#include <vector>
#include <memory>
#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"
#include "Channel.hpp"


namespace asio
{
	/**
	* バッファの1サンプルの大きさが未対応
	*/
	class NotImplementSampleType : std::exception
	{
	public:
		NotImplementSampleType(const std::string& message)
			: std::exception(("サンプルの種類に未対応: " + message).c_str()) {}
	};

	/**
	* バッファクラス
	*/
	class Buffer
	{
		IOType ioType;
		long channelNumber;
		long bufferSize;
		ASIOSampleType sampleType;
		void* bufferData[2];
		ASIOCallbacks* callbacks;

	private:
		static Buffer* currentBuffer;	// バッファのインスタンスへのポインタ
		static long doubleBufferIndex;

		static void BufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
		{
			Buffer::doubleBufferIndex = doubleBufferIndex;
		}

		static void SampleRateDidChange(ASIOSampleRate sRate)
		{

		}

		static long AsioMessage(long selector, long value, void* message, double* opt)
		{

		}
		
		static ASIOTime* BufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
		{
			Buffer::doubleBufferIndex = doubleBufferIndex;
			return params;
		}

	private:
		enum Type
		{
			Short = 0,
			Int = 1,
			Float = 2,
			Double = 3
		};

		struct Sample
		{
			Type type;
			bool isMSB;
			Sample() {}
			Sample(const Type type, const bool isMSB) : type(type), isMSB(isMSB) {}
		};

		Sample DetectSampleTypePackStruct()
		{
			Sample sample;
			switch (sampleType)
			{
				case ASIOSTInt32MSB:
					sample = Sample(Type::Int, true);
					break;
				case ASIOSTInt16MSB:
					sample = Sample(Type::Short, true);
					break;
				case ASIOSTFloat32MSB:		// IEEE 754 32 bit float
					sample = Sample(Type::Float, true);
					break;
				case ASIOSTFloat64MSB:		// IEEE 754 64 bit double float
					sample = Sample(Type::Double, true);
					break;
				case ASIOSTInt16LSB:
					sample = Sample(Type::Short, false);
					break;
				case ASIOSTInt32LSB:
					sample = Sample(Type::Int, false);
					break;
				case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
					sample = Sample(Type::Float, false);
					break;
				case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
					sample = Sample(Type::Double, false);
					break;
				
				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
					throw NotImplementSampleType("ASIOSTInt32LSB16");
				case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
					throw NotImplementSampleType("ASIOSTInt32LSB18");
				case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
					throw NotImplementSampleType("ASIOSTInt32LSB20");
				case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
					throw NotImplementSampleType("ASIOSTInt32LSB24");

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can be more easily used with these
				case ASIOSTInt32MSB16:		// 32 bit data with 16 bit alignment
					throw NotImplementSampleType("ASIOSTInt32MSB16");
				case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
					throw NotImplementSampleType("ASIOSTInt32MSB18");
				case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
					throw NotImplementSampleType("ASIOSTInt32MSB20");
				case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
					throw NotImplementSampleType("ASIOSTInt32MSB24");
				
				// Not Implement Formats
				case ASIOSTInt24MSB:		// used for 20 bits as well
					throw NotImplementSampleType("ASIOSTInt24MSB");
				case ASIOSTInt24LSB:	  	// used for 20 bits as well
					throw NotImplementSampleType("ASIOSTInt24LSB");

				//	ASIO DSD format.
				case ASIOSTDSDInt8LSB1:		// DSD 1 bit data, 8 samples per byte. First sample in Least significant bit.
					throw NotImplementSampleType("ASIOSTDSDInt8LSB1");
				case ASIOSTDSDInt8MSB1:		// DSD 1 bit data, 8 samples per byte. First sample in Most significant bit.
					throw NotImplementSampleType("ASIOSTDSDInt8MSB1");
				case ASIOSTDSDInt8NER8:		// DSD 8 bit data, 1 sample per byte. No Endianness required.
					throw NotImplementSampleType("ASIOSTDSDInt8NER8");
				case ASIOSTLastEntry:
					throw NotImplementSampleType("ASIOSTLastEntry");
			}
			return sample;
		}

	public:
		Buffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
			: ioType((IOType)info.isInput), channelNumber(info.channelNum), bufferSize(bufferSize), callbacks(callbacks), sampleType(sampleType)
		{
			Buffer::currentBuffer = this;

			bufferData[0] = info.buffers[0];
			bufferData[1] = info.buffers[1];
		}

		static ASIOCallbacks CreateCallbacks()
		{
			ASIOCallbacks callback;
			callback.bufferSwitch = &Buffer::BufferSwitch;
			callback.sampleRateDidChange = &Buffer::SampleRateDidChange;
			callback.asioMessage = &Buffer::AsioMessage;
			callback.bufferSwitchTimeInfo = &Buffer::BufferSwitchTimeInfo;
			return callback;
		}
	};


	// 静的領域の初期化
	Buffer* Buffer::currentBuffer;
	long Buffer::doubleBufferIndex;


	/**
	* バッファを管理するクラス
	*/
	class BufferManager
	{
		IASIO* iasio;

		std::vector<Buffer> buffers;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			buffers.clear();
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				buffers.emplace_back(info, bufferSize, sampleType, callbacks);
			}
		}

	public:
		BufferManager(IASIO* iasio)
			: iasio(iasio)
		{

		}

		~BufferManager()
		{
			if (bufferInfos.size() > 0)
				ErrorCheck(iasio->disposeBuffers());
		}

		/**
		* バッファリング開始
		*/
		void Start()
		{
			ErrorCheck(iasio->start());
		}

		/**
		* バッファリング終了
		*/
		void Stop()
		{
			ErrorCheck(iasio->stop());
		}

		/**
		* バッファリングしたいチャンネルを追加
		*/
		void AddChannel(const IOType& ioType, const long& channelNumber)
		{
			ASIOBufferInfo info;
			info.channelNum = channelNumber;
			info.isInput = ioType;
			bufferInfos.push_back(info);
		}

		/**
		* バッファリングしたいチャンネルを追加
		*/
		void AddChannel(const Channel& channel)
		{
			AddChannel(channel.ioType, channel.ioType);
		}

		/**
		* バッファリングしたいチャンネルをやっぱなしにする
		*/
		void ClearChannel()
		{
			bufferInfos.clear();
		}

		/**
		* バッファリングするチャンネルを返す
		*/
		const std::vector<ASIOBufferInfo>& BufferingChannels() const { return bufferInfos; }

		/**
		* バッファの生成
		* @params[in] bufferSize バッファの設定
		* @params[in, out] callbacks バッファリング等の通知のために利用
		* @note bufferSizeは自由に数値を決められないので注意, (bufferSize % granularity == 0)以外の数値は保障できない
		*/
		const std::vector<Buffer>& CreateBuffer(const long& bufferSize, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			asio::ASIOBufferInfo* infos = &bufferInfos.at(0);
			auto result = iasio->createBuffers(infos, bufferInfos.size(), bufferSize, callbacks);
			ErrorCheck(result);
			InitBuffers(bufferSize, sampleType, callbacks);
			return buffers;
		}

		/**
		* バッファの生成
		* @params[in] bufferPreference バッファの設定
		* @params[in, out] callbacks バッファリング等の通知のために利用
		* @note bufferSizeは自由に数値を決められないので注意, (bufferSize % granularity == 0)以外の数値は保障できない
		*/
		const std::vector<Buffer>& CreateBuffer(const BufferPreference& bufferPreference, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			CreateBuffer(bufferPreference.preferredSize, sampleType, callbacks);
			return buffers;
		}

		/**
		* 明示的にバッファを解放
		*/
		void DisposeBuffers() const
		{
			ErrorCheck(iasio->disposeBuffers());
		}
		
	};
}