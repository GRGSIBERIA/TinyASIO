#pragma once
#include <vector>
#include <Windows.h>

#include "SDK.hpp"
#include "Driver.hpp"

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
			EnterCriticalSection(&critical);
			stream = StreamingBuffer(new std::vector<int>());
			LeaveCriticalSection(&critical);
			return retval;
		}


		/**
		* �o�b�t�@�ɒl��~�ς���
		* @param[in] store �~�ς������l
		*/
		void Store(const std::vector<int>& store)
		{
			EnterCriticalSection(&critical);
			stream->insert(stream->end(), store.begin(), store.end());
			LeaveCriticalSection(&critical);
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

		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

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
		}
	};
}