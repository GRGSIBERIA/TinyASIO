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

/**
* このファイルで定義されている内容はASIO SDKから拝借
*/

#pragma once

#include <windows.h>

namespace asio
{
	typedef long ASIOBool;
	enum {
		ASIOFalse = 0,
		ASIOTrue = 1
	};


	typedef long ASIOError;
	enum {
		ASE_OK = 0,					//!< This value will be returned whenever the call succeeded
		ASE_SUCCESS = 0x3f4847a0,	//!< unique success return value for ASIOFuture calls
		ASE_NotPresent = -1000,		//!< hardware input or output is not present or available
		ASE_HWMalfunction,			//!< hardware is malfunctioning (can be returned by any ASIO function)
		ASE_InvalidParameter,		//!< input parameter invalid
		ASE_InvalidMode,			//!< hardware is in a bad mode or used in a bad mode
		ASE_SPNotAdvancing,			//!< hardware is not running when sample position is inquired
		ASE_NoClock,				//!< sample clock or rate cannot be determined or is not present
		ASE_NoMemory				//!< not enough memory for completing the request
	};


	typedef double ASIOSampleRate;


	struct ASIOClockSource
	{
		long index;					//!< as used for ASIOSetClockSource()
		long associatedChannel;		//!< for instance, S/PDIF or AES/EBU
		long associatedGroup;		//!< see channel groups (ASIOGetChannelInfo())
		ASIOBool isCurrentSource;	//!< ASIOTrue if this is the current clock source
		char name[32];				//!< for user selection
	};


	struct ASIOSamples {
		unsigned long hi;
		unsigned long lo;
	};


	struct ASIOTimeStamp {
		unsigned long hi;
		unsigned long lo;
	};


	typedef long ASIOSampleType;
	enum {
		ASIOSTInt16MSB = 0,
		ASIOSTInt24MSB = 1,			//!< used for 20 bits as well
		ASIOSTInt32MSB = 2,
		ASIOSTFloat32MSB = 3,		//!< IEEE 754 32 bit float
		ASIOSTFloat64MSB = 4,		//!< IEEE 754 64 bit double float

		// these are used for 32 bit data buffer, with different alignment of the data inside
		// 32 bit PCI bus systems can be more easily used with these
		ASIOSTInt32MSB16 = 8,		//!< 32 bit data with 16 bit alignment
		ASIOSTInt32MSB18 = 9,		//!< 32 bit data with 18 bit alignment
		ASIOSTInt32MSB20 = 10,		//!< 32 bit data with 20 bit alignment
		ASIOSTInt32MSB24 = 11,		//!< 32 bit data with 24 bit alignment

		ASIOSTInt16LSB = 16,
		ASIOSTInt24LSB = 17,		//!< used for 20 bits as well
		ASIOSTInt32LSB = 18,
		ASIOSTFloat32LSB = 19,		//!< IEEE 754 32 bit float, as found on Intel x86 architecture
		ASIOSTFloat64LSB = 20, 		//!< IEEE 754 64 bit double float, as found on Intel x86 architecture

		// these are used for 32 bit data buffer, with different alignment of the data inside
		// 32 bit PCI bus systems can more easily used with these
		ASIOSTInt32LSB16 = 24,		//!< 32 bit data with 18 bit alignment
		ASIOSTInt32LSB18 = 25,		//!< 32 bit data with 18 bit alignment
		ASIOSTInt32LSB20 = 26,		//!< 32 bit data with 20 bit alignment
		ASIOSTInt32LSB24 = 27,		//!< 32 bit data with 24 bit alignment

		//	ASIO DSD format.
		ASIOSTDSDInt8LSB1 = 32,		//!< DSD 1 bit data, 8 samples per byte. First sample in Least significant bit.
		ASIOSTDSDInt8MSB1 = 33,		//!< DSD 1 bit data, 8 samples per byte. First sample in Most significant bit.
		ASIOSTDSDInt8NER8 = 40,		//!< DSD 8 bit data, 1 sample per byte. No Endianness required.

		ASIOSTLastEntry
	};

	struct ASIOChannelInfo
	{
		long channel;			//!< on input, channel index
		ASIOBool isInput;		//!< on input
		ASIOBool isActive;		//!< on exit
		long channelGroup;		//!< dto
		ASIOSampleType type;	//!< dto
		char name[32];			//!< dto
	};


	struct ASIOBufferInfo
	{
		ASIOBool isInput;			//!< on input:  ASIOTrue: input, else output
		long channelNum;			//!< on input:  channel index
		void *buffers[2];			//!< on output: double buffer addresses
	};


	struct AsioTimeInfo
	{
		double          speed;                  //!< absolute speed (1. = nominal)
		ASIOTimeStamp   systemTime;             //!< system time related to samplePosition, in nanoseconds
		// on mac, must be derived from Microseconds() (not UpTime()!)
		// on windows, must be derived from timeGetTime()
		ASIOSamples     samplePosition;
		ASIOSampleRate  sampleRate;             //!< current rate
		unsigned long flags;                    //!< (see below)
		char reserved[12];
	};


	struct ASIOTimeCode
	{
		double          speed;                  //!< speed relation (fraction of nominal speed)
												//!< optional; set to 0. or 1. if not supported
		ASIOSamples     timeCodeSamples;        // time in samples
		unsigned long   flags;                  // some information flags (see below)
		char future[64];
	};


	struct ASIOTime                     // both input/output
	{
		long reserved[4];                //!< must be 0
		AsioTimeInfo     timeInfo;       //!< required
		ASIOTimeCode     timeCode;       //!< optional, evaluated if (timeCode.flags & kTcValid)
	};

	typedef void(*ASIOBufferSwitch) (long doubleBufferIndex, ASIOBool directProcess);
	typedef void(*ASIOSampleRateDidChange) (ASIOSampleRate sRate);
	typedef long(*ASIOAsioMessage) (long selector, long value, void* message, double* opt);
	typedef ASIOTime* (*ASIOBufferSwitchTimeInfo) (ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);

	struct ASIOCallbacks
	{
		ASIOBufferSwitch bufferSwitch;
		// bufferSwitch indicates that both input and output are to be processed.
		// the current buffer half index (0 for A, 1 for B) determines
		// - the output buffer that the host should start to fill. the other buffer
		//   will be passed to output hardware regardless of whether it got filled
		//   in time or not.
		// - the input buffer that is now filled with incoming data. Note that
		//   because of the synchronicity of i/o, the input always has at
		//   least one buffer latency in relation to the output.
		// directProcess suggests to the host whether it should immedeately
		// start processing (directProcess == ASIOTrue), or whether its process
		// should be deferred because the call comes from a very low level
		// (for instance, a high level priority interrupt), and direct processing
		// would cause timing instabilities for the rest of the system. If in doubt,
		// directProcess should be set to ASIOFalse.
		// Note: bufferSwitch may be called at interrupt time for highest efficiency.

		ASIOSampleRateDidChange sampleRateDidChange;
		// gets called when the AudioStreamIO detects a sample rate change
		// If sample rate is unknown, 0 is passed (for instance, clock loss
		// when externally synchronized).

		ASIOAsioMessage asioMessage;
		// generic callback for various purposes, see selectors below.
		// note this is only present if the asio version is 2 or higher

		ASIOBufferSwitchTimeInfo bufferSwitchTimeInfo;
		// new callback with time info. makes ASIOGetSamplePosition() and various
		// calls to ASIOGetSampleRate obsolete,
		// and allows for timecode sync etc. to be preferred; will be used if
		// the driver calls asioMessage with selector kAsioSupportsTimeInfo.
	};


	typedef interface IASIO IASIO;
	interface IASIO : public IUnknown
	{

		virtual ASIOBool init(void *sysHandle) = 0;
		virtual void getDriverName(char *name) = 0;
		virtual long getDriverVersion() = 0;
		virtual void getErrorMessage(char *string) = 0;
		virtual ASIOError start() = 0;
		virtual ASIOError stop() = 0;
		virtual ASIOError getChannels(long *numInputChannels, long *numOutputChannels) = 0;
		virtual ASIOError getLatencies(long *inputLatency, long *outputLatency) = 0;
		virtual ASIOError getBufferSize(long *minSize, long *maxSize,
			long *preferredSize, long *granularity) = 0;
		virtual ASIOError canSampleRate(ASIOSampleRate sampleRate) = 0;
		virtual ASIOError getSampleRate(ASIOSampleRate *sampleRate) = 0;
		virtual ASIOError setSampleRate(ASIOSampleRate sampleRate) = 0;
		virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources) = 0;
		virtual ASIOError setClockSource(long reference) = 0;
		virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) = 0;
		virtual ASIOError getChannelInfo(ASIOChannelInfo *info) = 0;
		virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
			long bufferSize, ASIOCallbacks *callbacks) = 0;
		virtual ASIOError disposeBuffers() = 0;
		virtual ASIOError controlPanel() = 0;
		virtual ASIOError future(long selector, void *opt) = 0;
		virtual ASIOError outputReady() = 0;
	};
}