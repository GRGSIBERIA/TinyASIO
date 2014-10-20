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

			template <typename T>
			static void ConvertToVoidBuffer(std::vector<TINY_ASIO_BUFFER_TYPE>&source, void* buffer, pack::Sample& sample, const long size)
			{
				
			}
		};
	}
}