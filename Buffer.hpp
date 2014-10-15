#pragma once
#include <vector>
#include <memory>
#include <algorithm>

#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"
#include "Channel.hpp"
#include "SamplePack.hpp"

namespace asio
{
	class BufferList
	{
		std::vector<float> floatBuffer;
		std::vector<int> intBuffer;
		std::vector<double> doubleBuffer;
		std::vector<short> shortBuffer;

		pack::Sample sample;

	private:

		template <typename T>
		void Insert(std::vector<T> buffer, void* vbuffer, const long size)
		{
			buffer.insert(buffer.end(), reinterpret_cast<T*>(vbuffer), reinterpret_cast<T*>(vbuffer) + size);
		}

		template <typename T>
		void ReverseEndian(T* p)
		{
			std::reverse(
				reinterpret_cast<BYTE*>(p),
				reinterpret_cast<BYTE*>(p) + sizeof(T));
		}

		template <typename T>
		void FormatBigEndian(void* buffer, const long size)
		{
			T *start = reinterpret_cast<T*>(buffer);
			const size_t num = size / sizeof(T);
			for (size_t i = 0; i < num; ++i)
			{
				ReverseEndian(start + i * sizeof(T));
			}
		}

		/**
		* �r�b�O�G���f�B�A���̏���
		*/
		void ReversibleMSB(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case pack::Int:
				FormatBigEndian<int>(buffer, size);
				break;

			case pack::Short:
				FormatBigEndian<short>(buffer, size);
				break;

			case pack::Float:
				FormatBigEndian<float>(buffer, size);
				break;

			case pack::Double:
				FormatBigEndian<double>(buffer, size);
				break;
			}
		}

		/**
		* �o�b�t�@�ɒǉ�����
		*/
		void StoreBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case pack::Int:
				Insert(intBuffer, buffer, size);
				break;

			case pack::Short:
				Insert(shortBuffer, buffer, size);
				break;

			case pack::Float:
				Insert(floatBuffer, buffer, size);
				break;

			case pack::Double:
				Insert(doubleBuffer, buffer, size);
				break;
			}
		}

	public:
		BufferList(pack::Sample& sample)
			: sample(sample) { }

		/**
		* �o�b�t�@�ɒ~�ς���
		*/
		void Store(void* buffer, const long size)
		{
			if (sample.isMSB)
				ReversibleMSB(buffer, size);
			StoreBuffer(buffer, size);
		}
	};

	class BufferController;

	/**
	* �o�b�t�@�N���X
	*/
	class Buffer
	{
		IOType ioType;
		long channelNumber;
		long bufferSize;
		const ASIOSampleType sampleType;
		void* bufferData[2];

		BufferList bufferList;

		friend BufferController;

	public:
		/**
		* IO�̎�ނ�Ԃ�
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
	* ���̓o�b�t�@�Ǘ��N���X
	*/
	class InputBuffer : public Buffer
	{

	};

	/**
	* �o�̓o�b�t�@�Ǘ��N���X
	*/
	class OutputBuffer : public Buffer
	{

	};

	class BufferManager;	// �t�����h�ɂ��邽�߂̑O���錾

	/**
	* �R�[���o�b�N�֐��ƃo�b�t�@�����O���܂Ƃ߂邽�߂̃N���X
	*/
	class BufferController
	{
		friend BufferManager;

		static std::vector<Buffer> buffers;

		std::vector<Buffer> inputBuffers;
		std::vector<Buffer> outputBuffers;

	private:

		static void BufferingInputChannel()
		{

		}

		static void BufferingOutputChannel()
		{

		}

		static void BufferingLoop(long doubleBufferIndex, ASIOBool directProcess)
		{
			for (size_t i = 0; i < buffers.size(); ++i)
			{
				if (buffers[i].Type() == IOType::Input)
					BufferingInputChannel();
				else
					BufferingOutputChannel();
			}
		}

		static void BufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
		{
			BufferingLoop(doubleBufferIndex, directProcess);
		}

		static void SampleRateDidChange(ASIOSampleRate sRate)
		{

		}

		static long AsioMessage(long selector, long value, void* message, double* opt)
		{
			return 0;
		}

		static ASIOTime* BufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
		{
			BufferingLoop(doubleBufferIndex, directProcess);
			return params;
		}

	private:
		void Add(const ASIOBufferInfo& info, const long& bufferSize, const ASIOSampleType& sampleType, ASIOCallbacks* callbacks)
		{
			buffers.emplace_back(info, bufferSize, sampleType);

			auto& buf = buffers[buffers.size()];
			if (info.isInput)
				inputBuffers.push_back(buf);
			else
				outputBuffers.push_back(buf);
		}

		void Clear()
		{
			buffers.clear();
			inputBuffers.clear();
			outputBuffers.clear();
		}

	public:
		static ASIOCallbacks CreateCallbacks()
		{
			ASIOCallbacks callback;
			callback.bufferSwitch = &BufferController::BufferSwitch;
			callback.sampleRateDidChange = &BufferController::SampleRateDidChange;
			callback.asioMessage = &BufferController::AsioMessage;
			callback.bufferSwitchTimeInfo = &BufferController::BufferSwitchTimeInfo;
			return callback;
		}
	};

	std::vector<Buffer> BufferController::buffers;

	/**
	* �o�b�t�@���Ǘ�����N���X
	*/
	class BufferManager
	{
		IASIO* iasio;

		BufferController bufferController;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, const ASIOSampleType& sampleType, ASIOCallbacks* callbacks)
		{
			bufferController.Clear();
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				bufferController.Add(info, bufferSize, sampleType, callbacks);
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
		* �o�b�t�@�����O�J�n
		*/
		inline void Start()
		{
			ErrorCheck(iasio->start());
		}

		/**
		* �o�b�t�@�����O�I��
		*/
		inline void Stop()
		{
			ErrorCheck(iasio->stop());
		}

		/**
		* �o�b�t�@�����O�������`�����l����ǉ�
		*/
		inline void AddChannel(const IOType& ioType, const long& channelNumber)
		{
			ASIOBufferInfo info;
			info.channelNum = channelNumber;
			info.isInput = ioType;
			bufferInfos.push_back(info);
		}

		/**
		* �o�b�t�@�����O�������`�����l����ǉ�
		*/
		inline void AddChannel(const Channel& channel)
		{
			AddChannel(channel.ioType, channel.ioType);
		}

		/**
		* �o�b�t�@�����O�������`�����l��������ςȂ��ɂ���
		*/
		inline void ClearChannel()
		{
			bufferInfos.clear();
		}

		/**
		* �o�b�t�@�����O����`�����l����Ԃ�
		*/
		const std::vector<ASIOBufferInfo>& BufferingChannels() const { return bufferInfos; }

		/**
		* �o�b�t�@�̐���
		* @params[in] bufferSize �o�b�t�@�̐ݒ�
		* @params[in, out] callbacks �o�b�t�@�����O���̒ʒm�̂��߂ɗ��p
		* @note bufferSize�͎��R�ɐ��l�����߂��Ȃ��̂Œ���, (bufferSize % granularity == 0)�ȊO�̐��l�͕ۏ�ł��Ȃ�
		*/
		const BufferController& CreateBuffer(const long& bufferSize, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			ErrorCheck(iasio->createBuffers(&bufferInfos[0], bufferInfos.size(), bufferSize, callbacks));
			InitBuffers(bufferSize, sampleType, callbacks);
			return bufferController;
		}

		/**
		* �o�b�t�@�̐���
		* @params[in] bufferPreference �o�b�t�@�̐ݒ�
		* @params[in, out] callbacks �o�b�t�@�����O���̒ʒm�̂��߂ɗ��p
		* @note bufferSize�͎��R�ɐ��l�����߂��Ȃ��̂Œ���, (bufferSize % granularity == 0)�ȊO�̐��l�͕ۏ�ł��Ȃ�
		*/
		const BufferController& CreateBuffer(const BufferPreference& bufferPreference, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			CreateBuffer(bufferPreference.preferredSize, sampleType, callbacks);
			return bufferController;
		}
	};
}