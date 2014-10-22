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
		* Option.hppで指定された型が変だったときに呼び出される
		*/
		class UnknownOptionType : public std::exception
		{
		public:
			UnknownOptionType(const std::string& message)
				: exception(message.c_str()) { }
		};

		/**
		* bufferからsourceへ流し込む処理のためのクラス
		* 適宜，型の変換処理も行っている
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
			* オプションの型と
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
					throw UnknownOptionType("判別できないバッファの型が指定されています, see also Option.hpp");

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
			* 1つの値のエンディアンを反転させる
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

				// 型ごとに決まった処理へ分岐させる
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
					// int24だけ特別な処理を行う
					CovertBit24(source, buffer, size);
					return true;		// ここから先の処理は不要なので強制的に退去させる
				}
#endif
				return false;
			}


			/**
			* @tparam TO 変換先の型
			* @tparam COMP 比較する型
			*/
			template <typename TO>
			static void SwitchingCompositTypeAtEachProcedure(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const pack::Sample& sample, const long size)
			{
				const long destCount = size / sizeof(TO);

				// 固有の処理を行った後，退出するなら退出させる
				if (DoneUniqueRoutineAsIsDone(source, buffer, sample, size))
					return;

				std::vector<TO> toArray(destCount, 0);
				float diff;

#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT

				// int型から
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

				// float型から
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
			* bufferからsourceへ流し込む処理
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
			* 受け取ったバッファのエンディアンを反転させる
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
			* std::vectorからvoid*へ変換する
			* @tparam TO 変換先の型
			*/
			template <typename TO>
			static void ConvertToVoidBuffer(std::vector<TINY_ASIO_BUFFER_TYPE>&source, void* buffer, const pack::Sample& sample, const long size)
			{
				// 各種，型から型への切替を行う
				SwitchingCompositTypeAtEachProcedure<TO>(source, buffer, sample, size);

				if (sample.isMSB)
					FormatBigEndian<TO>(buffer, size);
			}
		};
	}
}