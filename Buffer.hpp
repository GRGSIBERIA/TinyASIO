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


	namespace callback 
	{
		class CallbackManager;
	}

	/**
	* ���̓o�b�t�@�Ǘ��N���X
	*/
	class InputBuffer : public Buffer
	{
		friend callback::CallbackManager;

		DeviceToHostStream stream;

		/**
		* Stream�ɓ��͂��ꂽ�f�[�^��~�ς���
		* @note CallbackManager�p�̊֐�
		*/
		inline void StoreC(const long bufferIndex)
		{
			stream.Store(bufferData[bufferIndex], bufferSize);
		}

	public:
		InputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize), stream(pack::DetectSampleTypePackStruct(sampleType)) {}

		/**
		* �o�b�t�@�ɒ~�ς��ꂽ�f�[�^���擾����
		* @return �o�b�t�@�ɒ~�ς��ꂽ�f�[�^
		*/
		std::shared_ptr<std::vector<TINY_ASIO_BUFFER_TYPE>> Fetch()
		{
			auto sharedPtr = std::make_shared < std::vector<TINY_ASIO_BUFFER_TYPE>>(stream.GetStream());
			stream.Clear();
			return sharedPtr;
		}
	};


	/**
	* �o�̓o�b�t�@�Ǘ��N���X
	*/
	class OutputBuffer : public Buffer
	{
		friend callback::CallbackManager;

		HostToDeviceStream stream;

		/**
		* Stream�ɒ~�ς��ꂽ�f�[�^��]������
		* @note CallbackManager�p�̊֐�
		*/
		inline void FetchC(const long bufferIndex)
		{
			stream.Fetch(bufferData[bufferIndex], bufferSize);
		}

	public:
		OutputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize), stream(pack::DetectSampleTypePackStruct(sampleType)) {}

		/**
		* �o�b�t�@�Ƀf�[�^��~�ς���
		* @param[in] storeBuffer �~�ς������f�[�^
		*/
		void Store(const std::vector<TINY_ASIO_BUFFER_TYPE>& storeBuffer) 
		{
			stream.InsertLast(storeBuffer);
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
		* ���̓o�b�t�@�̃C���X�^���X�𓾂�
		* @return ���̓o�b�t�@
		*/
		inline InputBuffer& InputBuffer(const unsigned int index) { return inputBuffers[index]; }

		/**
		* �o�̓o�b�t�@�̃C���X�^���X�𓾂�
		* @return �o�̓o�b�t�@
		*/
		inline OutputBuffer& OutputBuffer(const unsigned int index) { return outputBuffers[index]; }

		/**
		* ���̓o�b�t�@�̐��𓾂�
		* @return ���̓o�b�t�@�̐�
		*/
		inline const size_t InputCount() const { return inputBuffers.size(); }

		/**
		* �o�̓o�b�t�@�̐��𓾂�
		* @return �o�̓o�b�t�@�̐�
		*/
		inline const size_t OutputCount() const { return outputBuffers.size(); }
	};
}