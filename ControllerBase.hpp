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

		ASIOCallbacks callbacks;

		static long bufferLength;
		static std::shared_ptr<BufferManager> bufferManager;

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
		void CreateBuffer(const std::vector<Channel>& channels, ASIOCallbacks* callbacks)
		{
			const auto& channelManager = Driver::Get().ChannelManager();
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, callbacks));
		}


		/*
		* �o�b�t�@�����֐��̌Ăяo���͎q�N���X�Ɉڏ�����
		*/
		template <size_t NUM>
		void CreateBuffer(const std::array<Channel, NUM>& channels, ASIOCallbacks* callbacks)
		{
			const auto& channelManager = Driver::Get().ChannelManager();
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(channels, bufferLength, callbacks));
		}


		static void SampleRateDidChange(ASIOSampleRate sRate)
		{
			throw SampleRateDidChangeException("�T���v�����O���g�����ύX����܂���");
		}
			
		static long AsioMessage(long selector, long value, void* message, double* opt)
		{
			return 0;
		}
		
		static ASIOTime* BufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
		{
			return params;
		}


		/**
		* �R�[���o�b�N�֐��𐶐�����
		*/
		ASIOCallbacks CreateCallbacks(ASIOBufferSwitch bufferSwitch)
		{
			auto callbacks = ASIOCallbacks();
			callbacks.asioMessage = &AsioMessage;
			callbacks.bufferSwitch = bufferSwitch;
			callbacks.bufferSwitchTimeInfo = &BufferSwitchTimeInfo;
			callbacks.sampleRateDidChange = &SampleRateDidChange;
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
		virtual ~ControllerBase() { }
	};

	std::shared_ptr<BufferManager> ControllerBase::bufferManager;
	long ControllerBase::bufferLength = 0;
}