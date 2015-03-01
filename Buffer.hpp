/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

TinyASIO is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

TinyASIO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TinyASIO.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

#pragma once
#include <vector>
#include <Windows.h>
#include <algorithm>
#include <array>

#include "Option.hpp"
#include "SDK.hpp"
#include "Driver.hpp"
#include "Channel.hpp"

namespace asio
{
	

	/**
	* �o�b�t�@�p�̃N���X
	*/
	class BufferBase
	{
	protected:
		void *buffers[2];	//!< �o�b�t�@
		long channelNumber;	//!< �`�����l���ԍ�

		StreamPtr stream;		//!< �X�g���[�~���O�p�̕ϐ�
		CRITICAL_SECTION critical;	//!< �N���e�B�J���Z�N�V����

		Channel channelInfo;	//!< �`�����l�����


		template <typename FUNC>
		void Critical(FUNC func)
		{
			EnterCriticalSection(&critical);
			func();
			LeaveCriticalSection(&critical);
		}


	public:
		BufferBase(const ASIOBufferInfo& info, const Channel& channel)
			: channelNumber(info.channelNum), channelInfo(channel)
		{
			buffers[0] = info.buffers[0];
			buffers[1] = info.buffers[1];

			stream = StreamPtr(new Stream());
			InitializeCriticalSection(&critical);
		}

		
		virtual ~BufferBase()
		{
			DeleteCriticalSection(&critical);
		}


		inline const long ChannelNumber() const { return channelNumber; }	//!< �`�����l���ԍ�
		inline void* GetBuffer(const long index) { return buffers[index]; }	//!< index����o�b�t�@���擾����
		inline const Channel& ChannelInfo() const { return channelInfo; }	//!< �`�����l�������擾����
		inline const long StreamLength() const { return stream->size(); }	//!< �X�g���[���̌��݂̒����𓾂�

		/**
		* �o�b�t�@�̒��g�����o��
		* @return �o�b�t�@�̒��g
		*/
		StreamPtr Fetch()
		{
			StreamPtr retval = stream;
			Critical([&](){ stream = StreamPtr(new std::vector<SampleType>()); });
			return retval;
		}


		/**
		* �����炠��o�b�t�@�ɓ]������
		* @param[in,out] buffer �]���������o�b�t�@
		* @param[in] bufferLength �o�b�t�@�̒���
		*/
		void Fetch(void* buffer, const unsigned long bufferLength)
		{
			Critical([&](){
				unsigned long length = bufferLength;
				if (length > stream->size())
					length = stream->size();
				memcpy(buffer, &stream->at(0), length * sizeof(SampleType));
				stream->erase(stream->begin(), stream->begin() + length);
			});
		}


		/**
		* �o�b�t�@�ɒl��~�ς���
		* @param[in] store �~�ς������l
		*/
		void Store(const Stream& store)
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
			SampleType* ptr = reinterpret_cast<SampleType*>(buffer);
			Critical([&](){ stream->insert(stream->end(), ptr, ptr + bufferLength); });
		}
		
		/**
		* �o�b�t�@�̒��g���폜����
		*/
		void Clear()
		{
			Critical([&](){ stream->clear(); });
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

		/**
		* �o�b�t�@�̗̈悪null����Ȃ�������true
		*/
		inline const bool IsEnabledBuffer() const
		{
			return buffers[0] != nullptr && buffers[1] != nullptr;
		}
	};


	/**
	* ���̓o�b�t�@, �M�^�[��}�C�N�Ȃǂ̓��͂�����
	*/
	class InputBuffer : public BufferBase
	{
	public:
		InputBuffer(const ASIOBufferInfo& info, const Channel& channel)
			: BufferBase(info, channel) {}
	};


	/**
	* �o�̓o�b�t�@�C�w�b�h�t�H����X�s�[�J�[�Ȃǂ֏o�͂���
	*/
	class OutputBuffer : public BufferBase
	{
	public:
		OutputBuffer(const ASIOBufferInfo& info, const Channel& channel)
			: BufferBase(info, channel) {}
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

		static std::vector<BufferBase>* buffersPtr;			//!< �R�[���o�b�N�֐�����g����悤�ɂ��邽�߂̃|�C���^
		static std::vector<InputBuffer>* inputBuffersPtr;
		static std::vector<OutputBuffer>* outputBuffersPtr;

		bool disposed;

	private:
		template <typename VECTOR_ARRAY>
		void InitBufferInfo(const VECTOR_ARRAY& channels)
		{
			bufferInfo = std::vector<ASIOBufferInfo>(channels.size());

			for (unsigned int i = 0; i < channels.size(); ++i)
			{
				bufferInfo[i].channelNum = channels[i].channelNumber;
				bufferInfo[i].isInput = channels[i].isInput;
			}
		}

		void InitBuffers(const std::vector<Channel>& channels, const long bufferLength, ASIOCallbacks* callbacks)
		{
			auto* iasio = Driver::Get().Interface();
			ErrorCheck(iasio->createBuffers(&bufferInfo[0], bufferInfo.size(), bufferLength, callbacks));

			for (unsigned long i = 0; i < bufferInfo.size(); ++i)
			{
				if (bufferInfo[i].isInput)
					inputBuffers.emplace_back(bufferInfo[i], channels[i]);
				else
					outputBuffers.emplace_back(bufferInfo[i], channels[i]);
			}

			inputBuffersPtr = &inputBuffers;
			outputBuffersPtr = &outputBuffers;
		}

	public:
		void DisposeBuffer()
		{
			if (!disposed)
			{
				Driver::Get().Interface()->disposeBuffers();
				disposed = true;
			}
		}

		virtual ~BufferManager()
		{
			DisposeBuffer();
		}

		BufferManager(const std::vector<Channel>& channels, const long bufferLength, ASIOCallbacks* callbacks)
			: disposed(false)
		{
			InitBufferInfo(channels);
			InitBuffers(channels, bufferLength, callbacks);
		}

		template <size_t NUM>
		BufferManager(const std::array<Channel, NUM>& channels, const long bufferLength, ASIOCallbacks* callbacks)
			: disposed(false)
		{
			InitBufferInfo(channels);
			InitBuffers(channels, bufferLength, callbacks);
		}

		/**
		* �o�b�t�@�����O����Ă�����̓`�����l����T��
		* �����Ƃ��ŏ��Ɍ����������̂��Ԃ����
		* @note �o�b�t�@�ւ�void*��nullptr����Ȃ����̂��擾����
		*/
		InputBuffer& SearchBufferableInput()
		{
			return *std::find_if(inputBuffers.begin(), inputBuffers.end(),
				[](const BufferBase& buffer) -> bool {
				return buffer.IsEnabledBuffer();
			});
		}

		/**
		* �o�b�t�@�����O����Ă���o�̓`�����l����T��
		* �����Ƃ��ŏ��Ɍ����������̂��Ԃ����
		* @note �o�b�t�@�ւ�void*��nullptr����Ȃ����̂��擾����
		*/
		OutputBuffer& SearchBufferableOutput()
		{
			return *std::find_if(outputBuffers.begin(), outputBuffers.end(),
				[](const BufferBase& buffer) -> bool {
				return buffer.IsEnabledBuffer();
			});
		}

		static std::vector<InputBuffer>* Inputs() { return inputBuffersPtr; }		//!< ���J����Ă�����̓o�b�t�@�𓾂�
		static std::vector<OutputBuffer>* Outputs() { return outputBuffersPtr; }	//!< ���J����Ă���o�̓o�b�t�@�𓾂�
		static InputBuffer& Inputs(const size_t i) { return inputBuffersPtr->at(i); }		//!< �Y��������̓o�b�t�@�𓾂�
		static OutputBuffer& Outputs(const size_t i) { return outputBuffersPtr->at(i); }	//!< �Y������o�̓o�b�t�@�𓾂�
	};

	std::vector<BufferBase>* BufferManager::buffersPtr = nullptr;
	std::vector<InputBuffer>* BufferManager::inputBuffersPtr = nullptr;
	std::vector<OutputBuffer>* BufferManager::outputBuffersPtr = nullptr;
}