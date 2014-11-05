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
			

		public:
			/**
			* bufferからsourceへ流し込む処理
			*/
			template <typename T>
			static void ConvertToOptionType(std::vector<int>& source, void* buffer, const long size)
			{
				
			}


			/**
			* std::vectorからvoid*へ変換する
			* @tparam TO 変換先の型
			*/
			template <typename TO>
			static void ConvertToVoidBuffer(std::vector<int>&source, void* buffer, const Sample& sample, const long size)
			{
				// 各種，型から型への切替を行う
				SwitchingCompositTypeAtEachProcedure<TO>(source, buffer, sample, size);

			}
		};
	}
}