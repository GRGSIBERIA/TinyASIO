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

		static long bufferLength;
		static BufferManager* bufferManagerPtr;

	protected:
		ControllerBase()
		{
			driver = &Driver::Get();
			iasio = driver->Interface();

			ErrorCheck(iasio->getBufferSize(NULL, NULL, &bufferLength, NULL));
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