#pragma once
#include <string>

#include "Driver.hpp"
#include "Channel.hpp"
#include "Buffer.hpp"

namespace asio
{
	/**
	* �R���g���[���̌��ɂȂ�N���X
	*/
	class ControllerBase
	{
	protected:
		Driver* driver;
		IASIO* iasio;

		long bufferLength;
		long inputLatency;
		long outputLatency;
		long sampleRate;

		ChannelManager* channelManager;
		BufferManager* bufferManager;

	protected:
		ControllerBase()
		{
			driver = &Driver::Get();
			iasio = driver->Interface();

			iasio->getBufferSize(NULL, NULL, &bufferLength, NULL);
			iasio->getLatencies(&inputLatency, &outputLatency);

			double sr;	// double�^�͂Ȃ񂩕s���R�Ȃ̂�long�ɕϊ�����
			iasio->getSampleRate(&sr);
			sampleRate = (long)sr;

			channelManager = new ChannelManager();
		}

		void CreateBuffer(ASIOCallbacks* callbacks)
		{
			bufferManager = new BufferManager(channelManager->NumberOfChannels(), bufferLength, callbacks);
		}

	public:
		void Start() { driver->Interface()->start(); }	//!< �^���J�n
		void Stop() { driver->Interface()->stop(); }	//!< �^���I��
		
		inline const long BufferSize() const { return bufferLength * sizeof(int); }		//!< �o�b�t�@�̗e�ʁi�o�C�g�j��Ԃ�
		inline const long BufferLength() const { return bufferLength; }		//!< �o�b�t�@�̒�����Ԃ�
		inline const long InputLatency() const { return inputLatency; }		//!< ���͂̒x����Ԃ�
		inline const long OutputLatency() const { return outputLatency; }	//!< �o�͂̒x����Ԃ�
		inline const long SampleRate() const { return sampleRate; }			//!< �T���v�����O���g����Ԃ�

	public:
		virtual ~ControllerBase()
		{
			delete channelManager;
			delete bufferManager;
		}
	};
}