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
	* �o�b�t�@�N���X
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
	public:
		InputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize, sampleType) {}
	};

	/**
	* �o�̓o�b�t�@�Ǘ��N���X
	*/
	class OutputBuffer : public Buffer
	{
	public:
		OutputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize, sampleType) {}
	};

	class BufferManager;	// �t�����h�ɂ��邽�߂̑O���錾

	/**
	* �R�[���o�b�N�֐��ƃo�b�t�@�����O���܂Ƃ߂邽�߂̃N���X
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
		* �o�b�t�@�����O�J�n
		*/
		inline void Start() const
		{
			ErrorCheck(iasio->start());
		}

		/**
		* �o�b�t�@�����O��~
		*/
		inline void Stop() const
		{
			ErrorCheck(iasio->stop());
		}

		/**
		* ���̓o�b�t�@�̔z����擾����
		* @return ���̓o�b�t�@�̔z��
		*/
		inline const std::vector<InputBuffer> InputBuffers() const { return inputBuffers; }

		/**
		* �o�̓o�b�t�@�̔z����擾����
		* @return �o�̓o�b�t�@�̔z��
		*/
		inline const std::vector<OutputBuffer> OutputBuffers() const { return outputBuffers; }
	};

	std::vector<Buffer*> BufferController::buffers;

	

	
}