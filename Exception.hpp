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
	* ドライバが動かない時に呼ばれる
	*/
	class CantProcessException : public std::exception
	{
	public:
		CantProcessException(const std::string& message)
			: exception(message.c_str()) {}
	};


	/**
	* 実行中にサンプリング周波数が変更された
	*/
	class SampleRateDidChangeException : public std::exception
	{
	public:
		SampleRateDidChangeException(const std::string& message)
			: exception(message.c_str()) {}
	};


	/**
	* チャンネルが見つからない時に呼ばれる
	*/
	class DontFoundChannels : public std::exception
	{
	public:
		DontFoundChannels(const std::string& message) 
			: exception(message.c_str()) {}
	};


	/**
	* ドライバのインスタンスに生成失敗すると呼ばれる
	*/
	class CantCreateInstance : public std::exception
	{
	public:
		CantCreateInstance(const std::string& message)
			: exception(message.c_str()) {}
	};


	/**
	* 二回以上初期化されたりなどで呼び出される
	*/
	class OverTwiceCallException : public std::exception
	{
	public:
		OverTwiceCallException(const std::string& message)
			: exception(message.c_str()) {}
	};


	/**
	* ドライバのハンドルを取得できなかった
	*/
	class CantHandlingASIODriver : public std::exception
	{
	public:
		CantHandlingASIODriver(const std::string& message)
			: exception(message.c_str()) {}
	};


	/**
	* レジストリーキーが開けない
	*/
	class CantOpenRegistryKey : public std::exception
	{
	public:
		CantOpenRegistryKey(const std::wstring& regPath) : std::exception(("レジストリを開けません: " + std::string(regPath.begin(), regPath.end())).c_str()) { }
	};


	/**
	* サブキーのインデックスが開けなくなっている
	*/
	class CantOpenSubKeyIndex : public std::exception
	{
	public:
		CantOpenSubKeyIndex(const std::wstring& regPath) : std::exception(("サブキーのインデックスが開けません:" + std::string(regPath.begin(), regPath.end())).c_str()) {}
	};


	/**
	* ASIOのドライバーがひとつもない
	*/
	class DontFoundASIODrivers : public std::exception
	{
	public:
		DontFoundASIODrivers(const std::wstring& message)
			: exception(std::string(message.begin(), message.end()).c_str()) {}
	};


	/**
	* 所有権が重複している
	*/
	class DuplicateOwnershipToken : public std::exception
	{
	public:
		DuplicateOwnershipToken(const std::wstring& message)
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