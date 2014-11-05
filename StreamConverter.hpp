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
			

		public:
			/**
			* buffer����source�֗������ޏ���
			*/
			static void DeviceToHost(std::vector<int>& source, void* buffer, const long size)
			{
				
			}


			/**
			* std::vector����void*�֕ϊ�����
			* @tparam TO �ϊ���̌^
			*/
			static void HostToDevice(std::vector<int>&source, void* buffer, const Sample& sample, const long size)
			{
				
			}
		};
	}
}