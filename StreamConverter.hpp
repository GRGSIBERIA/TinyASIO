#pragma once
#include <vector>
#include <exception>
#include <typeinfo>
#include "Option.hpp"

namespace asio
{
	namespace conv
	{
		/**
		* Option.hpp�Ŏw�肳�ꂽ�^���ς������Ƃ��ɌĂяo�����
		*/
		class UnknownOptionType : public std::exception
		{
		public:
			UnknownOptionType(const std::string& message)
				: exception(message.c_str()) { }
		};

		/**
		* buffer����source�֗������ޏ����̂��߂̃N���X
		* �K�X�C�^�̕ϊ��������s���Ă���
		*/
		class StreamConverter
		{
			static const long ResizeSource(std::vector<TINY_ASIO_BUFFER_TYPE>& source, const long count)
			{
				const long prevSize = source.size();
source.resize(prevSize + count);
return prevSize;
			}

			/**
			* �I�v�V�����̌^��
			*/
			static const float DecideMaxDiffFromType(const type_info& type)
			{
				float MAX_DIFF;

				if (type == typeid(int))
#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
					MAX_DIFF = 1.0f;
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
					MAX_DIFF = 1.0f / 2147483647.0f;
#endif
				else if (type == typeid(short))
#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
					MAX_DIFF = 65538.0f;
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
					MAX_DIFF = 1.0f / 32767.0f;
#endif
				else if (type == typeid(float) || type == typeid(double))
#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
					MAX_DIFF = 2147483647.0f;
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
					MAX_DIFF = 1.0f;
#endif
				else
					throw UnknownOptionType("���ʂł��Ȃ��o�b�t�@�̌^���w�肳��Ă��܂�, see also Option.hpp");

				return MAX_DIFF;
			}

			template <typename T>
			static void PourIntoSource(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const long count)
			{
				const long prevSize = ResizeSource(source, count);
				const float MAX_DIFF = DecideMaxDiffFromType(typeid(T));

				for (long i = 0; i < count; ++i)
					source[prevSize + i] = *(reinterpret_cast<T*>(buffer)+i) * MAX_DIFF;
			}

			/**
			* 1�̒l�̃G���f�B�A���𔽓]������
			*/
			template <typename T>
			static void ReverseEndian(T* p)
			{
				std::reverse(
					reinterpret_cast<BYTE*>(p),
					reinterpret_cast<BYTE*>(p)+sizeof(T));
			}


			static void CovertBit24(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const long size)
			{
				std::vector<BYTE> bit24Array(size, 0);
				long transferCount = source.size();
				const long bufferCount = size / 3;

				if (transferCount > bufferCount)
					transferCount = bufferCount;

#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
				// int ���� int24
				const float diff = 8388608.0f / 2147483647.0f;
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
				// float ���� int24
				const float diff = 8388607.0f;
#endif

				for (int i = 0; i < transferCount; ++i)
				{
					int num = (int)(source[i] * diff);
					BYTE* top = reinterpret_cast<BYTE*>(&num);
					bit24Array[i * 3 + 0] = top[1];
					bit24Array[i * 3 + 1] = top[2];
					bit24Array[i * 3 + 2] = top[3];
				}

				memcpy(buffer, &bit24Array[0], size);
			}


			/**
			* ���ۂɁCvoid*�֓]��������������T�C�Y���擾����
			*/
			template <typename TO>
			static long TransferSize(const long sourceCount, const long size)
			{
				long transferSize = sourceCount * sizeof(TO);
				if (transferSize > size)
					transferSize = size;
				return transferSize;
			}


			template <typename TO>
			static bool DoneUniqueRoutineAsIsDone(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const Sample& sample, const long size)
			{
				const long transferSize = TransferSize<TO>(source.size(), size);
				//if (transferSize < size)
				//{
				//	// ������transferSize���������Ɨ�����̂ŁC�Ƃ肠�����傫�����Ă���
				//	source.insert(source.begin(), size / sizeof(TO), 0);
				//}

				memset(buffer, 0, size);	// transferSize��size�ɖ����Ȃ��ꍇ�����邽�߁C����ŏ��������Ă���

				if (transferSize == 0)
					return false;

				// �^���ƂɌ��܂��������֕��򂳂���
#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
				if (sample.type == Int)
				{
					memcpy(buffer, &source[0], transferSize);
					return true;
				}
				if (sample.type == Int24)
				{
					CovertBit24(source, buffer, size);
					return true;
				}
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
				if (sample.type == Float)
				{
					memcpy(buffer, &source[0], transferSize);
					return true;
				}
				if (sample.type == Int24)
				{
					// int24�������ʂȏ������s��
					CovertBit24(source, buffer, size);
					return true;		// ���������̏����͕s�v�Ȃ̂ŋ����I�ɑދ�������
				}
#endif
				return false;
			}


			/**
			* �]������Ƃ��ɒl���ۂ߂�̂Ɏg�������l
			*/
			static const float GetDiffFromSample(const Sample& sample)
			{
#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT

				// int�^����
				switch (sample.type)
				{
				case Short:
					return 32767.0f / 2147483647.0f;

				case Float:
				case Double:
					return 1.0f / 2147483647.0f;
				}

#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT

				// float�^����
				switch (sample.type)
				{
				case Short:
					return 32767.0f;

				case Int:
					return 2147483647.0f;

				case Double:
					return 1.0f;
				}
#endif
				return 0.0f;
			}


			/**
			* @tparam TO �ϊ���̌^
			* @tparam COMP ��r����^
			*/
			template <typename TO>
			static void SwitchingCompositTypeAtEachProcedure(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const Sample& sample, const long size)
			{
				// �ŗL�̏������s������C�ޏo����Ȃ�ޏo������
				if (DoneUniqueRoutineAsIsDone<TO>(source, buffer, sample, size))
					return;

				const float diff = GetDiffFromSample(sample);
				const long destCount = size / sizeof(TO);
				std::vector<TO> toArray(destCount, 0);	// destCount�������������m�ۂ��C0�ŏ��������Ă���

				long transferCount = source.size();	// ���ۂɓ]������̈悪�Cbuffer�����傫���Ȃ�̂�h�~����
				if (transferCount > destCount)
					transferCount = destCount;

				for (int i = 0; i < transferCount; ++i)
					toArray[i] = (TO)(source[i] * diff);

				memcpy(buffer, &toArray[0], size);
			}


		public:
			/**
			* buffer����source�֗������ޏ���
			*/
			template <typename T>
			static void ConvertToOptionType(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const long size)
			{
				const long count = size / sizeof(T);

				if (typeid(TINY_ASIO_BUFFER_TYPE) == typeid(int) || typeid(TINY_ASIO_BUFFER_TYPE) == typeid(float))
				{
					source.insert(source.end(), reinterpret_cast<T*>(buffer), reinterpret_cast<T*>(buffer) + count);
				}
				else
				{
					PourIntoSource<T>(source, buffer, count);
				}
			}

			/**
			* �󂯎�����o�b�t�@�̃G���f�B�A���𔽓]������
			*/
			template <typename T>
			static void FormatBigEndian(void* buffer, const long size, const long typeSize = sizeof(T))
			{
				T *start = reinterpret_cast<T*>(buffer);
				const size_t num = size / sizeof(T);
				for (size_t i = 0; i < num; ++i)
				{
					ReverseEndian(start + i * typeSize);
				}
			}


			/**
			* std::vector����void*�֕ϊ�����
			* @tparam TO �ϊ���̌^
			*/
			template <typename TO>
			static void ConvertToVoidBuffer(std::vector<TINY_ASIO_BUFFER_TYPE>&source, void* buffer, const Sample& sample, const long size)
			{
				// �e��C�^����^�ւ̐ؑւ��s��
				SwitchingCompositTypeAtEachProcedure<TO>(source, buffer, sample, size);

				if (sample.isMSB)
					FormatBigEndian<TO>(buffer, size);
			}
		};
	}
}