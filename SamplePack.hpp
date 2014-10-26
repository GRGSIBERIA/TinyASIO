#pragma once
#include <string>
#include <exception>
#include "Interface.hpp"

namespace asio
{
	namespace pack
	{
		/**
		* �o�b�t�@��1�T���v���̑傫�������Ή�
		*/
		class NotImplementSampleType : std::exception
		{
		public:
			NotImplementSampleType(const std::string& message)
				: std::exception(("�T���v���̎�ނɖ��Ή�: " + message).c_str()) {}
		};


		/**
		* �^�̎�ނ�\��
		*/
		enum Type
		{
			Short = 0,
			Int = 1,
			Int24 = 2,
			Float = 3,
			Double = 4
		};


		/**
		* 1�T���v���̏���\��
		*/
		struct Sample
		{
			Type type;		//!< �^
			bool isMSB;		//!< �r�b�O�G���f�B�A�����ǂ���
			bool isAligned;

			Sample() {}

			/**
			* 1�T���v���̏����i�[����\����
			* @param[in] type �^�̎��
			* @param[in] isMSB true�̓r�b�O�G���f�B�A���Cfalse�̓��g���G���f�B�A��
			* @param[in] isAligned 32�r�b�g�ɃA���C�������g���邩�ǂ���
			* @note 20�r�b�g��18�r�b�g�͖�������
			*/
			Sample(const Type type, const bool isMSB, const bool isAligned = true) : type(type), isMSB(isMSB), isAligned(isAligned) {}
			
			/**
			* ASIOSampleType���擾����
			* @return ASIOSampleType�ɕϊ����ꂽ����
			*/
			const ASIOSampleType ToSampleType() const
			{
				switch (type)
				{
				case Short:
					if (isMSB)
					{
						if (isAligned)
							return ASIOSTInt32MSB16;
						return ASIOSTInt16MSB;
					}
					else
					{
						if (isAligned)
							return ASIOSTInt32LSB16;
						return ASIOSTInt16LSB;
					}
				case Int:
					if (isMSB)
						return ASIOSTInt32MSB;
					else
						return ASIOSTInt32LSB;
				case Int24:
					if (isMSB)
					{
						if (isAligned)
							return ASIOSTInt32MSB24;
						return ASIOSTInt24MSB;
					}
					else
					{
						if (isAligned)
							return ASIOSTInt32LSB24;
						return ASIOSTInt24LSB;
					}
				case Float:
					if (isMSB)
						return ASIOSTFloat32MSB;
					else
						return ASIOSTFloat32LSB;
				case Double:
					if (isMSB)
						return ASIOSTFloat64MSB;
					else
						return ASIOSTFloat64LSB;
				}
				throw NotImplementSampleType("���p�s�ȃT���v���̎��");
			}
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
				sample = Sample(Type::Int24, true);
			case ASIOSTInt24LSB:	  	// used for 20 bits as well
				sample = Sample(Type::Int24, false);

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