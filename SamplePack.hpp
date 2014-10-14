#pragma once
#include <string>
#include <exception>
#include "Interface.hpp"

namespace asio
{
	namespace pack
	{
		/**
		* バッファの1サンプルの大きさが未対応
		*/
		class NotImplementSampleType : std::exception
		{
		public:
			NotImplementSampleType(const std::string& message)
				: std::exception(("サンプルの種類に未対応: " + message).c_str()) {}
		};

		enum Type
		{
			Short = 0,
			Int = 1,
			Float = 2,
			Double = 3
		};

		struct Sample
		{
			Type type;
			bool isMSB;
			Sample() {}
			Sample(const Type type, const bool isMSB) : type(type), isMSB(isMSB) {}
		};

		Sample DetectSampleTypePackStruct(const long sampleType)
		{
			Sample sample;
			switch (sampleType)
			{
			case ASIOSTInt32MSB:
			case ASIOSTInt32MSB16:		// 32 bit data with 16 bit alignment
			case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
				sample = Sample(Type::Int, true);
				break;
			case ASIOSTInt16MSB:
				sample = Sample(Type::Short, true);
				break;
			case ASIOSTFloat32MSB:		// IEEE 754 32 bit float
				sample = Sample(Type::Float, true);
				break;
			case ASIOSTFloat64MSB:		// IEEE 754 64 bit double float
				sample = Sample(Type::Double, true);
				break;
			case ASIOSTInt16LSB:
				sample = Sample(Type::Short, false);
				break;
			case ASIOSTInt32LSB:
			case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
				sample = Sample(Type::Int, false);
				break;
			case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				sample = Sample(Type::Float, false);
				break;
			case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				sample = Sample(Type::Double, false);
				break;

				// Not Implement Formats
			case ASIOSTInt24MSB:		// used for 20 bits as well
				throw NotImplementSampleType("ASIOSTInt24MSB");
			case ASIOSTInt24LSB:	  	// used for 20 bits as well
				throw NotImplementSampleType("ASIOSTInt24LSB");

				//	ASIO DSD format.
			case ASIOSTDSDInt8LSB1:		// DSD 1 bit data, 8 samples per byte. First sample in Least significant bit.
				throw NotImplementSampleType("ASIOSTDSDInt8LSB1");
			case ASIOSTDSDInt8MSB1:		// DSD 1 bit data, 8 samples per byte. First sample in Most significant bit.
				throw NotImplementSampleType("ASIOSTDSDInt8MSB1");
			case ASIOSTDSDInt8NER8:		// DSD 8 bit data, 1 sample per byte. No Endianness required.
				throw NotImplementSampleType("ASIOSTDSDInt8NER8");
			case ASIOSTLastEntry:
				throw NotImplementSampleType("ASIOSTLastEntry");
			}
			return sample;
		}
	}
}