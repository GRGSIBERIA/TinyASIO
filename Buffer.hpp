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
	* �o�b�t�@�N���X
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
		* IO�̎�ނ�Ԃ�
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
	* ���̓o�b�t�@�Ǘ��N���X
	*/
	class InputBuffer : public Buffer
	{
		DeviceToHostStream stream;

	public:
		InputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize), stream(pack::DetectSampleTypePackStruct(sampleType)) {}

		void Read() {}

		/**
		* Stream�ɓ��͂��ꂽ�f�[�^��~�ς���
		*/
		inline void Store(const long bufferIndex) 
		{
			stream.Store(bufferData[bufferIndex], bufferSize);
		}
	};

	/**
	* �o�̓o�b�t�@�Ǘ��N���X
	*/
	class OutputBuffer : public Buffer
	{
		HostToDeviceStream stream;

	public:
		OutputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize), stream(pack::DetectSampleTypePackStruct(sampleType)) {}

		void Write() {}

		/**
		* Stream�ɒ~�ς��ꂽ�f�[�^��]������
		*/
		inline void Fetch(const long bufferIndex)
		{
			stream.Fetch(bufferData[bufferIndex], bufferSize);
		}
	};



	class BufferManager;	// �R�s�[�s�ɂ��邽�߂̎d�g��

	/**
	* �R�[���o�b�N�֐��ƃo�b�t�@�����O���܂Ƃ߂邽�߂̃N���X
	*/
	class BufferController
	{
		friend BufferManager;

		std::vector<Buffer*> buffers;	// callback::CallbackManager�Ƀ|�C���^��n���Ă�

		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

		IASIO* iasio;

	private:
		/**
		* ���Ŗ�L�ƃR�s�[����Ă������݂���Ȃ�
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
}