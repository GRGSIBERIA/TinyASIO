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

			double sr;	// doubleŒ^‚Í‚È‚ñ‚©•sŽ©‘R‚È‚Ì‚Å•ÏŠ·‚·‚é
			iasio->getSampleRate(&sr);
			sampleRate = (long)sr;
		}

	public:
		void Start() { driver->Interface()->start(); }	//!< ˜^‰¹ŠJŽn
		void Stop() { driver->Interface()->stop(); }	//!< ˜^‰¹I—¹
		
		inline const long BufferLength() const { return bufferLength; }		//!< ƒoƒbƒtƒ@‚Ì”‚ð•Ô‚·
		inline const long InputLatency() const { return inputLatency; }		//!< “ü—Í‚Ì’x‰„‚ð•Ô‚·
		inline const long OutputLatency() const { return outputLatency; }	//!< o—Í‚Ì’x‰„‚ð•Ô‚·
	};
}