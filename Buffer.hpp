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
			: Buffer(info, bufferSize, sampleType), stream(DetectSampleTypePackStruct(sampleType), info.channelNum) {}

		/**
		* �o�b�t�@�ɒ~�ς��ꂽ�f�[�^���擾����
		* @return �o�b�t�@�ɒ~�ς��ꂽ�f�[�^
		*/
		std::shared_ptr<std::vector<int>> Fetch()
		{
			return stream.CopyAsClear();
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
			: Buffer(info, bufferSize, sampleType), stream(DetectSampleTypePackStruct(sampleType), info.channelNum) {}

		/**
		* �o�b�t�@�Ƀf�[�^��~�ς���
		* @param[in] storeBuffer �~�ς������f�[�^
		*/
		inline void Store(const std::vector<int>& storeBuffer) 
		{
			stream.InsertLast(storeBuffer);
		}

		/**
		* �o�b�t�@�Ƀf�[�^��~�ς���
		* @param[in] storeBuffer �~�ς������f�[�^
		*/
		inline void Store(const std::shared_ptr<std::vector<int>>& storeBuffer)
		{
			Store(*storeBuffer);
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
		const Preference preference;

	private:
		/**
		* ���Ŗ�L�ƃR�s�[����Ă������݂���Ȃ�
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
		* �C���X�^���X���j�����ꂽ���Ƀo�b�t�@���������
		*/
		~BufferController()
		{
			//if (buffers.size() > 0)
				//ErrorCheck(iasio->disposeBuffers());
		}

		/**
		* �h���C�o�̐ݒ��Ԃ�
		*/
		inline const Preference& DriverPreference() const { return preference; }

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
		* @params[in] index �Y��
		* @return ���̓o�b�t�@
		*/
		inline InputBuffer& InputBuffer(const unsigned int index) { return inputBuffers[index]; }

		/**
		* �o�̓o�b�t�@�̃C���X�^���X�𓾂�
		* @params[in] index �Y��
		* @return �o�̓o�b�t�@
		*/
		inline OutputBuffer& OutputBuffer(const unsigned int index) { return outputBuffers[index]; }


		/**
		* �o�̓o�b�t�@�̔z��𓾂�
		* @return �o�̓o�b�t�@�̔z��
		*/
		const std::vector<asio::OutputBuffer>& OutputBuffers() { return outputBuffers; }


		/**
		* ���̓o�b�t�@�̔z��𓾂�
		* @return ���̓o�b�t�@�̔z��
		*/
		const std::vector<asio::InputBuffer>& InputBuffers() { return inputBuffers; }


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