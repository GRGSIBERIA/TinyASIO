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
//#include <Windows.h>
#include <algorithm>
#include <array>
#include <mutex>

#include "Option.hpp"
#include "SDK.hpp"
#include "Driver.hpp"
#include "Channel.hpp"

namespace asio
{
	class BufferManager;

	/**
	* �o�b�t�@�p�̃N���X
	*/
	class BufferBase
	{
	protected:
		void *buffers[2];	//!< �o�b�t�@
		long channelNumber;	//!< �`�����l���ԍ�

		StreamPtr stream;		//!< �X�g���[�~���O�p�̕ϐ�
		
		static std::mutex critical;	//!< ���L����������Ă����

		Channel channelInfo;	//!< �`�����l�����
		bool isStart = false;

		template <typename FUNC>
		void Critical(FUNC func)
		{
			critical.lock();
			func();
			critical.unlock();
		}

		friend BufferManager;


	public:
		BufferBase(const ASIOBufferInfo& info, const Channel& channel)
			: channelNumber(info.channelNum), channelInfo(channel)
		{
			buffers[0] = info.buffers[0];
			buffers[1] = info.buffers[1];

			stream = StreamPtr(new Stream());
			//InitializeCriticalSection(&critical);
		}

		
		virtual ~BufferBase()
		{
			//DeleteCriticalSection(&critical);
			stream = nullptr;
		}


		void StartBuffering() { isStart = true; }
		void StopBuffering() { isStart = false; }


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
			if (!isStart) return;
			Critical([&](){stream->insert(stream->end(), store.begin(), store.end()); });
		}


		/**
		* void*����o�b�t�@�ɒ~�ς���
		* @param[in] buffer �ڂ������o�b�t�@
		* @param[in] bufferLength �o�b�t�@�̒���
		*/
		void Store(void* buffer, const long bufferLength)
		{
			if (!isStart) return;
			SampleType* ptr = reinterpret_cast<SampleType*>(buffer);
			Critical([&]() { stream->insert(stream->end(), ptr, ptr + bufferLength); });
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
		inline const bool IsChannelNumber(const long chNumber) const
		{
			return this->channelNumber == chNumber;
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

	std::mutex BufferBase::critical;

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

	class ControllerBase;

	/**
	* �o�b�t�@�̊Ǘ��N���X
	*/
	class BufferManager
	{
		std::vector<ASIOBufferInfo> bufferInfo;

		std::vector<BufferBase*> buffers;
		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

		static std::vector<InputBuffer>* inputPtr;
		static std::vector<OutputBuffer>* outputPtr;

		bool disposed;

		friend ControllerBase;

		

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
				BufferBase* ptr;
				if (bufferInfo[i].isInput)
				{
					inputBuffers.emplace_back(bufferInfo[i], channels[i]);
					ptr = &inputBuffers.back();
				}
				else
				{
					outputBuffers.emplace_back(bufferInfo[i], channels[i]);
					ptr = &outputBuffers.back();
				}
				buffers.push_back(ptr);
			}

			inputPtr = &inputBuffers;
			outputPtr = &outputBuffers;
		}

	public:
		void DisposeBuffer()
		{
			if (this != nullptr)	// null�Ȃ̂�DisposeBuffer���Ă΂�邱�Ƃ�����Ǝv���̂ŉ������
			{
				if (!disposed)
				{
					Driver::Get().Interface()->disposeBuffers();
					disposed = true;
					inputPtr = nullptr;
					outputPtr = nullptr;
				}
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

		void StartBuffering()
		{
			for (auto& in : inputBuffers)
				in.StartBuffering();
			for (auto& out : outputBuffers)
				out.StartBuffering();
		}

		void StopBuffering()
		{
			for (auto& in : inputBuffers)
				in.StopBuffering();
			for (auto& out : outputBuffers)
				out.StopBuffering();
		}

		/**
		* BufferSwitch������̓o�b�t�@�𓾂邽�߂̊֐�
		*/
		static InputBuffer& Input(const size_t index) { return inputPtr->at(index); }

		/**
		* ���̓o�b�t�@�̑���
		*/
		static const size_t InputSize() { return inputPtr->size(); }

		/**
		* BufferSwitch����o�̓o�b�t�@�𓾂邽�߂̊֐�
		*/
		static OutputBuffer& Output(const size_t index) { return outputPtr->at(index); }
		
		/**
		* �o�̓o�b�t�@�̑���
		*/
		static const size_t OutputSize() { return outputPtr->size(); }

		static std::vector<InputBuffer>& Inputs() { return *inputPtr; }		// ���̓o�b�t�@�z��𓾂�
		static std::vector<OutputBuffer>& Outputs() { return *outputPtr; }	// �o�̓o�b�t�@�z��𓾂�
	};

	std::vector<InputBuffer>* BufferManager::inputPtr = nullptr;
	std::vector<OutputBuffer>* BufferManager::outputPtr = nullptr;
}