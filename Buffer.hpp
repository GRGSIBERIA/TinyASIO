#pragma once
#include <vector>
#include <Windows.h>
#include <algorithm>

#include "SDK.hpp"
#include "Driver.hpp"
#include "Channel.hpp"

namespace asio
{
	typedef std::shared_ptr<std::vector<int>> StreamingBuffer;

	/**
	* �o�b�t�@�p�̃N���X
	*/
	class BufferBase
	{
	protected:
		void *buffers[2];	//!< �o�b�t�@
		long channelNumber;	//!< �`�����l���ԍ�

		StreamingBuffer stream;		//!< �X�g���[�~���O�p�̕ϐ�
		CRITICAL_SECTION critical;	//!< �N���e�B�J���Z�N�V����


		template <typename FUNC>
		void Critical(FUNC func)
		{
			EnterCriticalSection(&critical);
			func();
			LeaveCriticalSection(&critical);
		}


	public:
		BufferBase(const ASIOBufferInfo& info)
			: channelNumber(info.channelNum)
		{
			buffers[0] = info.buffers[0];
			buffers[1] = info.buffers[1];

			stream = StreamingBuffer(new std::vector<int>());
			InitializeCriticalSection(&critical);
		}

		
		virtual ~BufferBase()
		{
			DeleteCriticalSection(&critical);
		}


		inline const long ChannelNumber() const { return channelNumber; }	//!< �`�����l���ԍ�
		inline void* GetBuffer(const long index) { return buffers[index]; }	//!< index����o�b�t�@���擾����


		/**
		* �o�b�t�@�̒��g�����o��
		* @return �o�b�t�@�̒��g
		*/
		StreamingBuffer Fetch()
		{
			StreamingBuffer retval = stream;
			Critical([&](){ stream = StreamingBuffer(new std::vector<int>()); });
			return retval;
		}


		/**
		* �����炠��o�b�t�@�ɓ]������
		* @param[in,out] buffer �]���������o�b�t�@
		* @param[in] bufferLength �o�b�t�@�̒���
		*/
		void Fetch(void* buffer, const long bufferLength)
		{
			Critical([&](){
				long length = bufferLength;
				if (length > stream->size())
					length = stream->size();
				memcpy(buffer, &stream->at(0), length * sizeof(int));
				stream->erase(stream->begin(), stream->begin() + length);
			});
		}


		/**
		* �o�b�t�@�ɒl��~�ς���
		* @param[in] store �~�ς������l
		*/
		void Store(const std::vector<int>& store)
		{
			Critical([&](){stream->insert(stream->end(), store.begin(), store.end()); });
		}


		/**
		* void*����o�b�t�@�ɒ~�ς���
		* @param[in] buffer �ڂ������o�b�t�@
		* @param[in] bufferLength �o�b�t�@�̒���
		*/
		void Store(void* buffer, const long bufferLength)
		{
			int* ptr = reinterpret_cast<int*>(buffer);
			Critical([&](){ stream->insert(stream->end(), ptr, ptr + bufferLength); });
		}

		/**
		* �`�����l���ԍ��Ŕ�r����
		*/
		inline const bool IsChannelNumber(const long channelNumber) const
		{
			return this->channelNumber == channelNumber;
		}

		/**
		* �`�����l���ԍ��Ŕ�r����
		*/
		inline const bool IsChannelNumber(const Channel& channel) const
		{
			return channelNumber == channel.channelNumber;
		}
	};


	/**
	* ���̓o�b�t�@, �M�^�[��}�C�N�Ȃǂ̓��͂�����
	*/
	class InputBuffer : public BufferBase
	{
	public:
		InputBuffer(const ASIOBufferInfo& info)
			: BufferBase(info) {}
	};


	/**
	* �o�̓o�b�t�@�C�w�b�h�t�H����X�s�[�J�[�Ȃǂ֏o�͂���
	*/
	class OutputBuffer : public BufferBase
	{
	public:
		OutputBuffer(const ASIOBufferInfo& info)
			: BufferBase(info) {}
	};


	/**
	* �o�b�t�@�̊Ǘ��N���X
	*/
	class BufferManager
	{
		std::vector<ASIOBufferInfo> bufferInfo;

		std::vector<BufferBase> buffers;
		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

		static std::vector<BufferBase>* buffersPtr;
		static std::vector<InputBuffer>* inputBuffersPtr;
		static std::vector<OutputBuffer>* outputBuffersPtr;

	public:
		BufferManager(const long numChannels, const long bufferLength, ASIOCallbacks* callbacks)
		{
			auto* iasio = Driver::Get().Interface();
			bufferInfo = std::vector<ASIOBufferInfo>(numChannels);
			ErrorCheck(iasio->createBuffers(bufferInfo._Myfirst, numChannels, bufferLength, callbacks));

			for (long i = 0; i < numChannels; ++i)
			{
				if (bufferInfo[i].isInput)
					inputBuffers.emplace_back(bufferInfo[i]);
				else
					outputBuffers.emplace_back(bufferInfo[i]);
			}

			inputBuffersPtr = &inputBuffers;
			outputBuffersPtr = &outputBuffers;
		}


		/**
		* �o�b�t�@����Ώۂ̃`�����l���ԍ���T���Ă���
		*/
		BufferBase& Search(const Channel& channel)
		{
			return *std::find_if(buffers.begin(), buffers.end(), 
				[&](const BufferBase& buffer) -> bool {
				return buffer.ChannelNumber() == channel.channelNumber; 
			});
		}

		InputBuffer& Search(const InputChannel& channel)
		{
			return *std::find_if(inputBuffers.begin(), inputBuffers.end(),
				[&](const BufferBase& buffer) -> bool {
				return buffer.ChannelNumber() == channel.channelNumber;
			});
		}

		OutputBuffer& Search(const OutputChannel& channel)
		{
			return *std::find_if(outputBuffers.begin(), outputBuffers.end(),
				[&](const BufferBase& buffer) -> bool {
				return buffer.ChannelNumber() == channel.channelNumber;
			});
		}

		static std::vector<InputBuffer>* InputBuffer() { return inputBuffersPtr; }		//!< ���J����Ă�����̓o�b�t�@�𓾂�
		static std::vector<OutputBuffer>* OutputBuffer() { return outputBuffersPtr; }	//!< ���J����Ă���o�̓o�b�t�@�𓾂�
	};

	std::vector<BufferBase>* BufferManager::buffersPtr = nullptr;
	std::vector<InputBuffer>* BufferManager::inputBuffersPtr = nullptr;
	std::vector<OutputBuffer>* BufferManager::outputBuffersPtr = nullptr;
}