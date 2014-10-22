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
				const long sourceCount = source.size();

#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
				const float diff = 8388608.0f / 2147483647.0f;
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
				const float diff = 8388607.0f;
#endif

				for (int i = 0; i < sourceCount; ++i)
				{
					int num = (int)(source[i] * diff);
					BYTE* top = reinterpret_cast<BYTE*>(&num);
					bit24Array[i * 3 + 0] = top[1];
					bit24Array[i * 3 + 1] = top[2];
					bit24Array[i * 3 + 2] = top[3];
				}

				memcpy(buffer, &bit24Array[0], size);
			}


			static bool DoneUniqueRoutineAsIsDone(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const pack::Sample& sample, const long size)
			{
				const long sourceCount = source.size();
				const long sourceSize = sizeof(TINY_ASIO_BUFFER_TYPE) * sourceCount;

				// �^���ƂɌ��܂��������֕��򂳂���
#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
				if (sample.type == pack::Int)
				{
					memcpy(buffer, &source[0], sourceSize);
					return true;
				}
				if (sample.type == pack::Int24)
				{
					CovertBit24(source, buffer, size);
					return true;
				}
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
				if (sample.type == pack::Float)
				{
					memcpy(buffer, &source[0], sourceSize);
					return true;
				}
				if (sample.type == pack::Int24)
				{
					// int24�������ʂȏ������s��
					CovertBit24(source, buffer, size);
					return true;		// ���������̏����͕s�v�Ȃ̂ŋ����I�ɑދ�������
				}
#endif
				return false;
			}


			/**
			* @tparam TO �ϊ���̌^
			* @tparam COMP ��r����^
			*/
			template <typename TO>
			static void SwitchingCompositTypeAtEachProcedure(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const pack::Sample& sample, const long size)
			{
				const long destCount = size / sizeof(TO);

				// �ŗL�̏������s������C�ޏo����Ȃ�ޏo������
				if (DoneUniqueRoutineAsIsDone(source, buffer, sample, size))
					return;

				std::vector<TO> toArray(destCount, 0);
				float diff;

#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT

				// int�^����
				switch (sample.type)
				{
				case pack::Short:
					diff = 32767.0f / 2147483647.0f;
					break;

				case pack::Float:
				case pack::Double:
					diff = 1.0f / 2147483647.0f;
					break;
				}

#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT

				// float�^����
				switch (sample.type)
				{
				case pack::Short:
					diff = 32767.0f;
					break;

				case pack::Int:
					diff = 2147483647.0f;
					break;

				case pack::Double:
					diff = 1.0f;
					break;
				}
#endif
				const long sourceCount = source.size();
				for (int i = 0; i < sourceCount; ++i)
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
					source.insert(source.end(), reinterpret_cast<T*>(buffer), reinterpret_cast<T*>(buffer)+count);
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
			static void ConvertToVoidBuffer(std::vector<TINY_ASIO_BUFFER_TYPE>&source, void* buffer, const pack::Sample& sample, const long size)
			{
				// �e��C�^����^�ւ̐ؑւ��s��
				SwitchingCompositTypeAtEachProcedure<TO>(source, buffer, sample, size);

				if (sample.isMSB)
					FormatBigEndian<TO>(buffer, size);
			}
		};
	}
}