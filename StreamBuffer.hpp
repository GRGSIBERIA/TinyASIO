#pragma once
#include <vector>
#include "SamplePack.hpp"
#include "StreamConverter.hpp"

namespace asio
{
	/**
	* �o�b�t�@�p�̃X�g���[��
	*/
	class StreamBuffer
	{
		std::vector<TINY_ASIO_BUFFER_TYPE> stream;

		pack::Sample sample;

	private:
		/**
		* �r�b�O�G���f�B�A���̏���
		*/
		void ReversibleMSB(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case pack::Int:
				conv::StreamConverter::FormatBigEndian<int>(buffer, size);
				break;

			case pack::Int24:
				conv::StreamConverter::FormatBigEndian<int>(buffer, size, 3);
				break;

			case pack::Short:
				conv::StreamConverter::FormatBigEndian<short>(buffer, size);
				break;

			case pack::Float:
				conv::StreamConverter::FormatBigEndian<float>(buffer, size);
				break;

			case pack::Double:
				conv::StreamConverter::FormatBigEndian<double>(buffer, size);
				break;
			}
		}

		/**
		* �o�b�t�@�ɒǉ�����
		*/
		void StoreBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case pack::Int:
				conv::StreamConverter::ConvertToOptionType<int>(stream, buffer, size);
				break;

			case pack::Short:
				conv::StreamConverter::ConvertToOptionType<short>(stream, buffer, size);
				break;

			case pack::Int24:
				{
					// ������Int24bit����Int32bit�ɕϊ�����
					const long count = size / 3;
					const long resize = count * 4;	// 24bit -> 32bit�̃T�C�Y
					std::vector<int> toInt32List(count);
					for (int i = 0; i < count; ++i)
					{
						BYTE* bytePtr = reinterpret_cast<BYTE*>(buffer) + i * 3;	// �o�C�g�^�̃|�C���^�ňʒu�𒲐�����
						toInt32List[i] = *reinterpret_cast<int*>(bytePtr) & 0xFFFFFF;	// 24�r�b�g�}�X�N��������
					}
					conv::StreamConverter::ConvertToOptionType<int>(stream, reinterpret_cast<void*>(&toInt32List[0]), size);
				}
				break;

			case pack::Float:
				conv::StreamConverter::ConvertToOptionType<float>(stream, buffer, size);
				break;

			case pack::Double:
				conv::StreamConverter::ConvertToOptionType<double>(stream, buffer, size);
				break;
			}
		}

	public:
		StreamBuffer(pack::Sample& sample)
			: sample(sample) { }

		/**
		* �o�b�t�@�ɒ~�ς���
		*/
		void Store(void* buffer, const long size)
		{
			if (sample.isMSB)
				ReversibleMSB(buffer, size);
			StoreBuffer(buffer, size);
		}
	};
}