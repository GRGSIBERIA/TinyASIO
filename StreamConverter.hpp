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
				long transferCount = source.size();
				const long bufferCount = size / 3;

				if (transferCount > bufferCount)
					transferCount = bufferCount;

#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT
				// int から int24
				const float diff = 8388608.0f / 2147483647.0f;
#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT
				// float から int24
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
			* 実際に，void*へ転送する実メモリサイズを取得する
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
				//	// ここでtransferSizeが小さいと落ちるので，とりあえず大きくしておく
				//	source.insert(source.begin(), size / sizeof(TO), 0);
				//}

				memset(buffer, 0, size);	// transferSizeがsizeに満たない場合があるため，これで初期化しておく

				if (transferSize == 0)
					return false;

				// 型ごとに決まった処理へ分岐させる
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
					// int24だけ特別な処理を行う
					CovertBit24(source, buffer, size);
					return true;		// ここから先の処理は不要なので強制的に退去させる
				}
#endif
				return false;
			}


			/**
			* 転送するときに値を丸めるのに使う差分値
			*/
			static const float GetDiffFromSample(const Sample& sample)
			{
#if TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_INT

				// int型から
				switch (sample.type)
				{
				case Short:
					return 32767.0f / 2147483647.0f;

				case Float:
				case Double:
					return 1.0f / 2147483647.0f;
				}

#elif TINY_ASIO_BUFFER_OPTION == TINY_ASIO_BUFFER_FLOAT

				// float型から
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
			* @tparam TO 変換先の型
			* @tparam COMP 比較する型
			*/
			template <typename TO>
			static void SwitchingCompositTypeAtEachProcedure(std::vector<TINY_ASIO_BUFFER_TYPE>& source, void* buffer, const Sample& sample, const long size)
			{
				// 固有の処理を行った後，退出するなら退出させる
				if (DoneUniqueRoutineAsIsDone<TO>(source, buffer, sample, size))
					return;

				const float diff = GetDiffFromSample(sample);
				const long destCount = size / sizeof(TO);
				std::vector<TO> toArray(destCount, 0);	// destCountだけメモリを確保し，0で初期化しておく

				long transferCount = source.size();	// 実際に転送する領域が，bufferよりも大きくなるのを防止する
				if (transferCount > destCount)
					transferCount = destCount;

				for (int i = 0; i < transferCount; ++i)
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
					source.insert(source.end(), reinterpret_cast<T*>(buffer), reinterpret_cast<T*>(buffer) + count);
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
			static void ConvertToVoidBuffer(std::vector<TINY_ASIO_BUFFER_TYPE>&source, void* buffer, const Sample& sample, const long size)
			{
				// 各種，型から型への切替を行う
				SwitchingCompositTypeAtEachProcedure<TO>(source, buffer, sample, size);

				if (sample.isMSB)
					FormatBigEndian<TO>(buffer, size);
			}
		};
	}
}