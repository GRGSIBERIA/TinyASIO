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
#include <string>

#include "Driver.hpp"
#include "Channel.hpp"
#include "Buffer.hpp"

namespace asio
{
	/**
	* �R���g���[���̌��ɂȂ�N���X
	* @warning �����쐬����Ƌ��������`���N�`���ɂȂ�̂Œ���
	*/
	class ControllerBase
	{
	protected:
		Driver* driver;
		IASIO* iasio;
		const ChannelManager* channelManager;

		long inputLatency;
		long outputLatency;

		ASIOCallbacks callbacks;
	
		static long sampleRate;
		static long bufferLength;		//!< �o�b�t�@�̒���
		static std::shared_ptr<BufferManager> bufferManager;

		static bool ownershipToken;		//!< ���L��

	private:
		/**
		* �R���g���[���̏�����
		*/
		void InitController()
		{
			if (ownershipToken)
				throw DuplicateOwnershipToken(L"�����̃R���g���[������������Ă��܂��D�Е����폜���Ă��������D");

			ownershipToken = true;

			driver = &Driver::Get();
			iasio = driver->Interface();
			channelManager = &driver->ChannelManager();

			long buf = 0;
			ErrorCheck(iasio->getBufferSize(&buf, &buf, &bufferLength, &buf));
			ErrorCheck(iasio->getLatencies(&inputLatency, &outputLatency));

			double sr;	// double�^�͂Ȃ񂩕s���R�Ȃ̂�long�ɕϊ�����
			ErrorCheck(iasio->getSampleRate(&sr));
			ErrorCheck(iasio->setSampleRate(sr));
			sampleRate = (long)sr;
		}

	protected:
		ControllerBase(const std::string& asioDriverName)
		{
			Driver::Init(asioDriverName);
			InitController();
		}

		ControllerBase(const std::wstring& asioDriverName)
		{
			Driver::Init(asioDriverName);
			InitController();
		}

		static void SampleRateDidChange(ASIOSampleRate)
		{
			throw SampleRateDidChangeException("�T���v�����O���g���̕ύX�����m���܂����B\n�ύX���Ȃ��ł�������");
		}

		static long AsioMessage(long, long, void*, double*)
		{
			return 0;
		}

		static ASIOTime* BufferSwitchTimeInfo(ASIOTime* params, long, ASIOBool)
		{
			return params;
		}

		static InputBuffer& Input(const size_t index) { return bufferManager->Input(index); }

		static OutputBuffer& Output(const size_t index) { return bufferManager->Output(index); }

		/**
		* �R�[���o�b�N�֐��𐶐�����
		*/
		void InitCallbacks(ASIOBufferSwitch bufferSwitch)
		{
			callbacks.asioMessage = &AsioMessage;
			callbacks.bufferSwitch = bufferSwitch;
			callbacks.bufferSwitchTimeInfo = &BufferSwitchTimeInfo;
			callbacks.sampleRateDidChange = &SampleRateDidChange;
		}

		/*
		* �o�b�t�@�����֐��̌Ăяo���͎q�N���X�Ɉڏ�����
		*/
		void CreateBuffer(const std::vector<Channel>& channels, ASIOCallbacks* callbackMethod)
		{
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, callbackMethod));
		}

		/*
		* �o�b�t�@�����֐��̌Ăяo���͎q�N���X�Ɉڏ�����
		*/
		void CreateBuffer(const std::vector<Channel>& channels, ASIOBufferSwitch bufferSwitch)
		{
			InitCallbacks(bufferSwitch);
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, &callbacks));
		}

	public:
		//!< �o�b�t�@�����O�J�n
		virtual void Start() 
		{ 
			bufferManager->StartBuffering();
			driver->Interface()->start(); 
		}

		//!< �o�b�t�@�����O�I��
		virtual void Stop() 
		{ 
			bufferManager->StopBuffering();
			driver->Interface()->stop(); 
		}	
		
		static const size_t BufferSize() { return bufferLength * sizeof(asio::SampleType); }		//!< �o�b�t�@�̗e�ʁi�o�C�g�j��Ԃ�
		static const long BufferLength() { return bufferLength; }		//!< �o�b�t�@�̒�����Ԃ�

		inline const long InputLatency() const { return inputLatency; }		//!< ���͂̒x����Ԃ�
		inline const long OutputLatency() const { return outputLatency; }	//!< �o�͂̒x����Ԃ�
		inline const long SampleRate() const { return sampleRate; }			//!< �T���v�����O���g����Ԃ�

		/*
		* �o�b�t�@�𖾎��I�ɊJ������
		*/
		void DisposeBuffer()
		{
			Stop();
			bufferManager->DisposeBuffer();
		}

		virtual ~ControllerBase() 
		{
			DisposeBuffer();
		}

		/**
		* ���͂���o�͂ɓ]���������ƁC�o�b�t�@�ɃX�g�A����
		*/
		static void TransferMemoryAsStored(InputBuffer& inBuffer, void* inPtr, void* outPtr)
		{
			memcpy(outPtr, inPtr, bufferLength * sizeof(asio::SampleType));
			inBuffer.Store(inPtr, bufferLength);
		}

		/**
		* ���̓o�b�t�@�̃������A�h���X�𓾂�
		* @param channelIndex �`�����l��ID
		* @param bufferIndex �_�u���o�b�t�@ID
		*/
		static void* GetInputMemory(const size_t channelIndex, const long bufferIndex)
		{
			return bufferManager->Input(channelIndex).GetBuffer(bufferIndex);
		}

		/**
		* �o�̓o�b�t�@�̃������A�h���X�𓾂�
		* @param channelIndex �`�����l��ID
		* @param bufferIndex �_�u���o�b�t�@ID
		*/
		static void* GetOutputMemory(const size_t channelIndex, const long bufferIndex)
		{
			return bufferManager->Output(channelIndex).GetBuffer(bufferIndex);
		}
	};

	std::shared_ptr<BufferManager> ControllerBase::bufferManager;
	long ControllerBase::bufferLength = 0;
	long ControllerBase::sampleRate = 0;
	bool ControllerBase::ownershipToken = false;
}