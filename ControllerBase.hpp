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

		long inputLatency;
		long outputLatency;
		long sampleRate;

		BufferManager* bufferManager;
		ASIOCallbacks callbacks;

		static long bufferLength;
		static BufferManager* bufferManagerPtr;

	protected:
		ControllerBase()
		{
			driver = &Driver::Get();
			iasio = driver->Interface();

			long buf = 0;
			ErrorCheck(iasio->getBufferSize(&buf, &buf, &bufferLength, &buf));
			ErrorCheck(iasio->getLatencies(&inputLatency, &outputLatency));

			double sr;	// double�^�͂Ȃ񂩕s���R�Ȃ̂�long�ɕϊ�����
			ErrorCheck(iasio->getSampleRate(&sr));
			ErrorCheck(iasio->setSampleRate(sr));
			sampleRate = (long)sr;
		}

		/*
		* �o�b�t�@�����֐��̌Ăяo���͎q�N���X�Ɉڏ�����
		*/
		void CreateBuffer(ASIOCallbacks* callbacks)
		{
			const auto& channelManager = Driver::Get().ChannelManager();
			bufferManager = new BufferManager(channelManager.NumberOfChannels(), bufferLength, callbacks);
			bufferManagerPtr = bufferManager;
		}

		/**
		* �R�[���o�b�N�֐��𐶐�����
		*/
		ASIOCallbacks CreateCallbacks(
			ASIOBufferSwitch bufferSwitch, ASIOSampleRateDidChange sampleRateDidChange,
			ASIOAsioMessage asioMessage, ASIOBufferSwitchTimeInfo bufferSwitchTimeInfo)
		{
			auto callbacks = ASIOCallbacks();
			callbacks.asioMessage = asioMessage;
			callbacks.bufferSwitch = bufferSwitch;
			callbacks.bufferSwitchTimeInfo = bufferSwitchTimeInfo;
			callbacks.sampleRateDidChange = sampleRateDidChange;
			return callbacks;
		}


	public:
		void Start() { driver->Interface()->start(); }	//!< �o�b�t�@�����O�J�n
		void Stop() { driver->Interface()->stop(); }	//!< �o�b�t�@�����O�I��
		
		inline const long BufferSize() const { return bufferLength * sizeof(int); }		//!< �o�b�t�@�̗e�ʁi�o�C�g�j��Ԃ�
		inline const long BufferLength() const { return bufferLength; }		//!< �o�b�t�@�̒�����Ԃ�
		inline const long InputLatency() const { return inputLatency; }		//!< ���͂̒x����Ԃ�
		inline const long OutputLatency() const { return outputLatency; }	//!< �o�͂̒x����Ԃ�
		inline const long SampleRate() const { return sampleRate; }			//!< �T���v�����O���g����Ԃ�

	public:
		virtual ~ControllerBase()
		{
			delete bufferManager;
		}
	};

	BufferManager* ControllerBase::bufferManagerPtr = nullptr;
	long ControllerBase::bufferLength = 0;
}