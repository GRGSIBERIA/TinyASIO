#pragma once
#include <string>
#include <exception>

#include "SDK.hpp"

namespace asio
{
	/**
	* �h���C�o�������Ȃ����ɌĂ΂��
	*/
	class CantProcessException : public std::exception
	{
	public:
		CantProcessException(const std::string& message)
			: exception(message.c_str()) {}
	};


	/**
	* �`�����l����������Ȃ����ɌĂ΂��
	*/
	class DontFoundChannels : public std::exception
	{
	public:
		DontFoundChannels(const std::string& message) 
			: exception(message.c_str()) {}
	};


	/**
	* �h���C�o�̃C���X�^���X�ɐ������s����ƌĂ΂��
	*/
	class CantCreateInstance : public std::exception
	{
	public:
		CantCreateInstance(const std::string& message)
			: exception(message.c_str()) {}
	};


	/**
	* ���ȏ㏉�������ꂽ��ȂǂŌĂяo�����
	*/
	class OverTwiceCallException : public std::exception
	{
	public:
		OverTwiceCallException(const std::string& message)
			: exception(message.c_str()) {}
	};


	/**
	* �h���C�o�̃n���h�����擾�ł��Ȃ�����
	*/
	class CantHandlingASIODriver : public std::exception
	{
	public:
		CantHandlingASIODriver(const std::string& message)
			: exception(message.c_str()) {}
	};


	/**
	* ���W�X�g���[�L�[���J���Ȃ�
	*/
	class CantOpenRegistoryKey : public std::exception
	{
	public:
		CantOpenRegistoryKey(const std::wstring& regPath) : std::exception(("���W�X�g�����J���܂���: " + std::string(regPath.begin(), regPath.end())).c_str()) { }
	};


	/**
	* �T�u�L�[�̃C���f�b�N�X���J���Ȃ��Ȃ��Ă���
	*/
	class CantOpenSubKeyIndex : public std::exception
	{
	public:
		CantOpenSubKeyIndex(const std::wstring& regPath) : std::exception(("�T�u�L�[�̃C���f�b�N�X���J���܂���:" + std::string(regPath.begin(), regPath.end())).c_str()) {}
	};


	/**
	* ASIO�̃h���C�o�[���ЂƂ��Ȃ�
	*/
	class DontFoundASIODrivers : public std::exception
	{
	public:
		DontFoundASIODrivers(const std::wstring& message)
			: exception(std::string(message.begin(), message.end()).c_str()) {}
	};


	void ErrorCheck(const long& error)
	{
		switch (error)
		{
		case ASE_NotPresent:
			throw CantProcessException("hardware input or output is not present or available");
		case ASE_HWMalfunction:
			throw CantProcessException("hardware is malfunctioning (can be returned by any ASIO function)");
		case ASE_InvalidParameter:
			throw CantProcessException("input parameter invalid");
		case ASE_InvalidMode:
			throw CantProcessException("hardware is in a bad mode or used in a bad mode");
		case ASE_SPNotAdvancing:
			throw CantProcessException("hardware is not running when sample position is inquired");
		case ASE_NoClock:
			throw CantProcessException("sample clock or rate cannot be determined or is not present");
		case ASE_NoMemory:
			throw CantProcessException("not enough memory for completing the request");
		}
	}
}