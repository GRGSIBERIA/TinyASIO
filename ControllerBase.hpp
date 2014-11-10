#pragma once
#include <string>

#include "Driver.hpp"

namespace asio
{
	class ControllerBase
	{
	protected:
		Driver* driver;

		long bufferLength;
		long inputLatency;
		long outputLatency;
		long sampleRate;

	protected:
		ControllerBase()
		{
			driver = &Driver::Get();
			auto *iasio = driver->Interface();

			iasio->getBufferSize(NULL, NULL, &bufferLength, NULL);
			iasio->getLatencies(&inputLatency, &outputLatency);

			double sr;	// double�^�͂Ȃ񂩕s���R�Ȃ̂ŕϊ�����
			iasio->getSampleRate(&sr);
			sampleRate = (long)sr;
		}

	public:
		void Start() { driver->Interface()->start(); }	//!< �^���J�n
		void Stop() { driver->Interface()->stop(); }	//!< �^���I��
		
		inline const long BufferLength() const { return bufferLength; }		//!< �o�b�t�@�̐���Ԃ�
		inline const long InputLatency() const { return inputLatency; }		//!< ���͂̒x����Ԃ�
		inline const long OutputLatency() const { return outputLatency; }	//!< �o�͂̒x����Ԃ�
	};
}