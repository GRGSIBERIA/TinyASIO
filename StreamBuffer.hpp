#pragma once
#include <vector>
#include "SamplePack.hpp"
#include "StreamConverter.hpp"

namespace asio
{
	/**
	* �ǂ������킯���m���^��������ꂽ�ꍇ�̗�O
	*/
	class UnrecognizedTypeException : public std::exception
	{
	public:
		UnrecognizedTypeException(const std::string& message)
			: std::exception(message.c_str()) {}
	};


	/**
	* �o�b�t�@�����O���邽�߂̃X�g���[���N���X
	* @note �q�N���X�͊�{�I��BufferController���猩���Ȃ��Ȃ��Ă���̂ŁC����ȂɌ��J�E����J�͋C�ɂ��Ȃ��Ă������Ǝv��
	*/
	class StreamBuffer
	{
	protected:
		std::vector<TINY_ASIO_BUFFER_TYPE> stream;

		Sample sample;

	protected:
		/**
		* �r�b�O�G���f�B�A���̏���
		*/
		void ReversibleMSB(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Int:
				conv::StreamConverter::FormatBigEndian<int>(buffer, size);
				break;

			case Int24:
				conv::StreamConverter::FormatBigEndian<int>(buffer, size, 3);
				break;

			case Short:
				conv::StreamConverter::FormatBigEndian<short>(buffer, size);
				break;

			case Float:
				conv::StreamConverter::FormatBigEndian<float>(buffer, size);
				break;

			case Double:
				conv::StreamConverter::FormatBigEndian<double>(buffer, size);
				break;

			default:
				throw UnrecognizedTypeException("���p�s�\�ȗʎq���r�b�g�����w�肳��Ă��܂�");
			}
		}


		void RemoveFrontFromSize(const long bufferSize)
		{
			// �擪����bufferSize������������
			unsigned long count;
			switch (sample.type)
			{
			case Short:
				count = bufferSize / sizeof(short);
				break;

			case Int:
				count = bufferSize / sizeof(int);
				break;

			case Int24:
				count = bufferSize / 3;
				break;

			case Float:
				count = bufferSize / sizeof(float);
				break;

			case Double:
				count = bufferSize / sizeof(double);
				break;

			default:
				throw UnrecognizedTypeException("���p�s�\�ȗʎq���r�b�g�����w�肳��Ă��܂�");
			}

			if (count > stream.size())
				count = stream.size();

			stream.erase(stream.begin(), stream.begin() + count);
		}

	public:
		StreamBuffer(Sample& samplePack)
			: sample(samplePack) {}


		inline void Clear()
		{
			stream.clear();
		}

		inline const std::vector<TINY_ASIO_BUFFER_TYPE>& GetStream() const
		{
			return stream;
		}
	};



	/**
	* �f�o�C�X����z�X�g�֗������߂̃X�g���[���N���X
	*/
	class DeviceToHostStream : public StreamBuffer
	{
		/**
		* �ʎq���r�b�g����24bit�̏ꍇ�̓���ȏ���
		*/
		void ConvertTo24Bit(void* buffer, const long size)
		{
			// ������Int24bit����Int32bit�ɕϊ�����
			const long count = size / 3;
			const long resize = count * 4;	// 24bit -> 32bit�̃T�C�Y
			std::vector<int> toInt32List(count);
			for (int i = 0; i < count; ++i)
			{
				BYTE* bytePtr = reinterpret_cast<BYTE*>(buffer);	// �o�C�g�^�̃|�C���^�ňʒu�𒲐�����
				toInt32List[i] = *reinterpret_cast<int*>(bytePtr[i * 3]) & 0xFFFFFF;	// 24�r�b�g�}�X�N��������
			}
			conv::StreamConverter::ConvertToOptionType<int>(stream, reinterpret_cast<void*>(&toInt32List[0]), resize);
		}

		/**
		* �o�b�t�@�ɒǉ�����
		*/
		void StoreBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Int:
				conv::StreamConverter::ConvertToOptionType<int>(stream, buffer, size);
				break;

			case Short:
				conv::StreamConverter::ConvertToOptionType<short>(stream, buffer, size);
				break;

			case Int24:
				ConvertTo24Bit(buffer, size);
				break;

			case Float:
				conv::StreamConverter::ConvertToOptionType<float>(stream, buffer, size);
				break;

			case Double:
				conv::StreamConverter::ConvertToOptionType<double>(stream, buffer, size);
				break;

			default:
				throw UnrecognizedTypeException("���p�s�\�ȗʎq���r�b�g�����w�肳��Ă��܂�");
			}
		}

	public:
		DeviceToHostStream(Sample sample)
			: StreamBuffer(sample) { }

		/**
		* �o�b�t�@�̒��g���X�g���[���֒~�ς���
		*/
		void Store(void* buffer, const long size)
		{
			if (sample.isMSB)
				ReversibleMSB(buffer, size);
			StoreBuffer(buffer, size);
		}
	};



	/**
	* �z�X�g����f�o�C�X�ɑ��邽�߂̃X�g���[���N���X
	*/
	class HostToDeviceStream : public StreamBuffer
	{
		void FetchBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Short:
				conv::StreamConverter::ConvertToVoidBuffer<short>(stream, buffer, sample, size);
				break;

			case Int:
				conv::StreamConverter::ConvertToVoidBuffer<int>(stream, buffer, sample, size);
				break;

			case Int24:
				conv::StreamConverter::ConvertToVoidBuffer<int>(stream, buffer, sample, size);
				break;

			case Float:
				conv::StreamConverter::ConvertToVoidBuffer<float>(stream, buffer, sample, size);
				break;

			case Double:
				conv::StreamConverter::ConvertToVoidBuffer<double>(stream, buffer, sample, size);
				break;

			default:
				throw UnrecognizedTypeException("���p�s�\�ȗʎq���r�b�g�����w�肳��Ă��܂�");
			}
		}

	public:
		HostToDeviceStream(Sample& sample)
			: StreamBuffer(sample) { }

		/**
		* �X�g���[���̒��g���o�b�t�@�֏�������
		*/
		void Fetch(void* buffer, const long size)
		{
			memset(buffer, 0, size);	// �ŏ��Ƀ[������

			FetchBuffer(buffer, size);
			if (sample.isMSB)			// ��ԍŌ�ɃG���f�B�A�����t�]������
				ReversibleMSB(buffer, size);

			RemoveFrontFromSize(size);
		}

		void InsertLast(const std::vector<TINY_ASIO_BUFFER_TYPE>& storeBuffer)
		{
			stream.insert(stream.end(), storeBuffer.begin(), storeBuffer.end());
		}
	};
}