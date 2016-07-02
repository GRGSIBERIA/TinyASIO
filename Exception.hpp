/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

TinyASIO is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

TinyASIO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TinyASIO.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

#pragma once
#include <string>
#include <exception>

#include "SDK.hpp"

namespace asio
{
	/**
	* TinyASIO�̗�O���N���X
	*/
	class TinyASIOException : public std::exception
	{
		void ShowError(const std::string& message)
		{
			MessageBoxA(NULL, message.c_str(), "�G���[", MB_OK | MB_ICONERROR);
		}

		void ShowError(const std::wstring& message)
		{
			MessageBoxW(NULL, message.c_str(), L"�G���[", MB_OK | MB_ICONERROR);
		}

	public:
		TinyASIOException(const std::string& message)
			: exception(message.c_str()) 
		{
			ShowError(message);
		}

		TinyASIOException(const std::wstring& message)
			: exception(std::string(message.begin(), message.end()).c_str())
		{
			ShowError(message);
		}
	};

	/**
	* �h���C�o�������Ȃ����ɌĂ΂��
	*/
	class CantProcessException : public TinyASIOException
	{
	public:
		CantProcessException(const std::string& message)
			: TinyASIOException(message) {}
	};


	/**
	* ���s���ɃT���v�����O���g�����ύX���ꂽ
	*/
	class SampleRateDidChangeException : public TinyASIOException
	{
	public:
		SampleRateDidChangeException(const std::string& message)
			: TinyASIOException(message) {}
	};


	/**
	* �`�����l����������Ȃ����ɌĂ΂��
	*/
	class DontFoundChannels : public TinyASIOException
	{
	public:
		DontFoundChannels(const std::string& message) 
			: TinyASIOException(message) {}
	};


	/**
	* �h���C�o�̃C���X�^���X�ɐ������s����ƌĂ΂��
	*/
	class CantCreateInstance : public TinyASIOException
	{
	public:
		CantCreateInstance(const std::string& message)
			: TinyASIOException(message) {}
	};


	/**
	* ���ȏ㏉�������ꂽ��ȂǂŌĂяo�����
	*/
	class OverTwiceCallException : public TinyASIOException
	{
	public:
		OverTwiceCallException(const std::string& message)
			: TinyASIOException(message) {}
	};


	/**
	* �h���C�o�̃n���h�����擾�ł��Ȃ�����
	*/
	class CantHandlingASIODriver : public TinyASIOException
	{
	public:
		CantHandlingASIODriver(const std::string& message)
			: TinyASIOException(message) {}
	};


	/**
	* ���W�X�g���[�L�[���J���Ȃ�
	*/
	class CantOpenRegistryKey : public TinyASIOException
	{
	public:
		CantOpenRegistryKey(const std::wstring& regPath) : TinyASIOException(L"���W�X�g�����J���܂���: " + regPath) { }
	};


	/**
	* �T�u�L�[�̃C���f�b�N�X���J���Ȃ��Ȃ��Ă���
	*/
	class CantOpenSubKeyIndex : public TinyASIOException
	{
	public:
		CantOpenSubKeyIndex(const std::wstring& regPath) : TinyASIOException(L"�T�u�L�[�̃C���f�b�N�X���J���܂���:" + regPath) {}
	};


	/**
	* ASIO�̃h���C�o�[���ЂƂ��Ȃ�
	*/
	class DontFoundASIODrivers : public TinyASIOException
	{
	public:
		DontFoundASIODrivers(const std::wstring& message)
			: TinyASIOException(message) {}
	};


	/**
	* ���L�����d�����Ă���
	*/
	class DuplicateOwnershipToken : public TinyASIOException
	{
	public:
		DuplicateOwnershipToken(const std::wstring& message)
			: TinyASIOException(message) {}
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

	/**
	* �܂��J�n���Ă��Ȃ��̂ɁC����̏������s�����Ƃ��̗�O
	*/
	class DontStartException : public TinyASIOException
	{
	public:
		DontStartException(const std::wstring& message)
			: TinyASIOException(message) {}
	};
}