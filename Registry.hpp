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
#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <algorithm>

#include <cstdlib>
#include <stdlib.h>

#include "Exception.hpp"

namespace asio
{
	const std::wstring ASIO_REGISTORY_PATH = L"SOFTWARE\\ASIO";					//!< ASIOドライバのレジストリを格納しているところ
	const std::wstring ASIO_CLSID_PATH = L"SOFTWARE\\Classes\\CLSID";			//!< CLSIDを格納しているところ
	const std::wstring ASIO_CLSID_PATH_WOW6432NODE = L"SOFTWARE\\Classes\\CLSID\\Wow6432Node";

	/**
	* レジストリのパスとドライバー名を格納するための構造体
	*/
	struct SubKey
	{
	public:
		std::wstring registryPath;	/*!< レジストリのパス */
		std::wstring driverName;	/*!< ドライバ名 */

		SubKey(const std::wstring& regPath, const std::wstring& driverName)
			: registryPath(regPath), driverName(driverName) {}
	};

	class Registry;

	typedef std::shared_ptr<std::vector<SubKey>> SubKeyList;
	typedef std::shared_ptr<std::vector<::CLSID>> CLSIDList;


	/**
	* ASIOドライバのレジストリの配列を扱うためのラッパークラス
	*/
	class DriverList
	{
		friend Registry;

		SubKeyList subkeys;

		DriverList(SubKeyList& subkeys)
			: subkeys(subkeys) { }

	public:
		/**
		* ASIOドライバのレジストリキーの配列を取得する
		* @return ASIOドライバのレジストリキーの配列
		*/
		const std::vector<SubKey>& Items() const { return *subkeys; }
		const SubKey& Items(const long i) const { return subkeys->at(i); }
		const size_t Count() const { return subkeys->size(); }


		/**
		* 配列からASIOドライバのレジストリを探す
		* @param[in] findName 探したいASIOドライバの名前
		* @return ASIOドライバのレジストリ
		*/
		const SubKey& Find(const std::wstring findName)
		{
			return *std::find_if(subkeys->begin(), subkeys->end(),
				[findName](asio::SubKey& subkey)
			{
				return subkey.driverName.find(findName) != std::wstring::npos;
			});
		}


		/**
		* 配列からASIOドライバのレジストリを探す
		* @param[in] findName 探したいASIOドライバの名前
		* @return ASIOドライバのレジストリ
		*/
		const SubKey& Find(const std::string findName)
		{
			std::wstring str(findName.begin(), findName.end());
			return Find(str);
		}
		
		/*
		* 中身が空のドライバリスト
		*/
		DriverList() {}
	};


	enum ThreadingModel
	{
		Apartment,	//!< STA，シングルスレッドで動作
		Both,		//!< 両方，E_NOINTERFACEでCoCreateInstanceでコケる場合に
		Free		//!< MTA，マルチスレッドで動作
	};


	/**
	* ASIO関連のレジストリを探す処理
	*/
	class Registry
	{
	public:
		static SubKeyList subkeys;
		static CLSIDList clsids;
		
	private:
		static LONG WrappedRegOpenKey(HKEY mainKey, const std::wstring& regPath, HKEY& hkey)
		{
			return RegOpenKeyEx(mainKey, (LPCTSTR)regPath.c_str(), 0, KEY_ALL_ACCESS, &hkey);
		}


		static bool Exist(HKEY mainKey, const std::wstring& regPath)
		{
			HKEY hkey;

			LONG cr = WrappedRegOpenKey(mainKey, regPath.c_str(), hkey);
			if (cr != ERROR_SUCCESS)
				return false;

			::RegCloseKey(hkey);

			return true;
		}

		static inline void OutputMessage(DWORD)
		{
			LPTSTR lpBuffer = NULL;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, GetLastError(), LANG_USER_DEFAULT,
				(LPTSTR)&lpBuffer, 0, NULL);
			MessageBox(NULL, lpBuffer, TEXT("Last Error Message"), MB_ICONHAND | MB_OK);
			LocalFree(lpBuffer);
		}

		static const std::wstring GetSubKey(HKEY& hkey, const DWORD index)
		{
			// RegEnumKeyExに失敗したら例外を発生させておく
			DWORD max_path_size = 360;
			wchar_t *subkeyBuffer = new wchar_t[max_path_size];
			LONG cr = RegEnumKeyExW(hkey, index, subkeyBuffer, &max_path_size, NULL, NULL, NULL, NULL);
			if (cr != ERROR_SUCCESS) {
				throw CantOpenSubKeyIndex(ASIO_REGISTORY_PATH);	// この例外でbreakできる
			}
				
			std::wstring result = subkeyBuffer;
			delete[] subkeyBuffer;		// 正直，こういうことやりたくない
			return result;
		}


		static std::wstring GetCLSIDString(const std::wstring& regPath, HKEY& hkey)
		{
			if (WrappedRegOpenKey(HKEY_LOCAL_MACHINE, regPath, hkey) != ERROR_SUCCESS)
				throw CantOpenRegistryKey(regPath);

			DWORD dataSize = 360 * sizeof(TCHAR);
			wchar_t clsidStrBuffer[360];
			RegQueryValueEx(hkey, TEXT("CLSID"), NULL, NULL, (LPBYTE)clsidStrBuffer, &dataSize);

			// レジストリエントリの値が16進数の文字列
			return clsidStrBuffer;
		}


		static const std::wstring GetCLSIDString(const SubKey& subkey)
		{
			HKEY hkey;
			LONG cr = WrappedRegOpenKey(HKEY_LOCAL_MACHINE, ASIO_REGISTORY_PATH, hkey);
			if (cr != ERROR_SUCCESS)
				throw CantOpenRegistryKey(subkey.registryPath);

			std::wstring clsidStr = GetCLSIDString(subkey.registryPath, hkey);

			::RegCloseKey(hkey);

			return clsidStr;
		}


		static std::wstring ToStr(const ThreadingModel& model)
		{
			std::wstring modelStr(L"");
			switch (model)
			{
			case ThreadingModel::Apartment:
				modelStr = L"Apartment";
				break;
			case ThreadingModel::Both:
				modelStr = L"Both";
				break;
			case ThreadingModel::Free:
				modelStr = L"Free";
				break;
			default:
				throw std::exception("おかしなものが投げられてるnyo");
			}
			return modelStr;
		}


	public:
		/**
		* SOFTWARE/ASIOが存在するか調べる
		*/
		static bool ExistDrivers()
		{
			return Exist(HKEY_LOCAL_MACHINE, ASIO_REGISTORY_PATH);
		}


		/**
		* ThreadingModelを変更する
		* @param[in] subkey 対象のドライバのサブキー
		* @param[in] model ThreadingModeの種類，デフォルト推奨
		* @return ERROR_SUCCES以外はエラー
		* @warning CLSIDのレジストリ値を書き換えるので，利用には注意
		*/
		static LONG ChangeTheadingModel(const SubKey& subkey, const ThreadingModel model = ThreadingModel::Both)
		{
			const auto clsidStr = GetCLSIDString(subkey);

			HKEY hkey;
			std::wstring regPath = ASIO_CLSID_PATH + L"\\" + clsidStr + L"\\InprocServer32";	// ここにThreadingModeがありますよー
			LONG cr = WrappedRegOpenKey(HKEY_LOCAL_MACHINE, regPath, hkey);
			if (cr != ERROR_SUCCESS)
			{
				// さっきのパスには入っていなかったので，互換性のあるパスを調査してみる
				regPath = ASIO_CLSID_PATH_WOW6432NODE + L"\\" + clsidStr + L"\\InprocServer32";
				cr = WrappedRegOpenKey(HKEY_LOCAL_MACHINE, regPath, hkey);
				if (cr != ERROR_SUCCESS)
					throw CantOpenRegistryKey(regPath);
			}

			const std::wstring modelStr = ToStr(model);
			cr = RegSetValueEx(hkey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)modelStr.c_str(), sizeof(wchar_t) * modelStr.size());

			::RegCloseKey(hkey);

			return cr;
		}

		/**
		* 登録されているASIOドライバのレジストリのパスを返す
		* @return レジストリのパスの配列
		*/
		static DriverList GetAsioDriverPathes()
		{
			subkeys = SubKeyList(new std::vector<SubKey>());

			HKEY hkey;
			LONG cr = WrappedRegOpenKey(HKEY_LOCAL_MACHINE, ASIO_REGISTORY_PATH, hkey);

			if (cr != ERROR_SUCCESS)
				return DriverList(subkeys);		// 何らかの理由でレジストリを開くのに失敗した

			DWORD max_index = 0;

			RegQueryInfoKey(hkey, NULL, NULL, NULL, &max_index, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

			for (DWORD index = 0; index < max_index; ++index)
			{
				try
				{
					const auto subkey = GetSubKey(hkey, index);
					subkeys->emplace_back(ASIO_REGISTORY_PATH + L"\\" + subkey, subkey);
				}
				catch (CantOpenSubKeyIndex)
				{
					// 開けないので帰る
					break;
				}
			}

			::RegCloseKey(hkey);

			if (subkeys->size() <= 0)
				throw DontFoundASIODrivers(L"ASIOのドライバーがひとつも存在しません");

			return DriverList(subkeys);
		}


		/**
		* ASIOドライバーのCLSIDを取得する
		*/
		static ::CLSID GetCLSID(const std::wstring& regPath)
		{
			HKEY hkey;
			std::wstring clsidStr = GetCLSIDString(regPath, hkey);

			::CLSID resultCLSID;

			auto check = CLSIDFromString(clsidStr.c_str(), (LPCLSID)&resultCLSID);
			if (check != S_OK)
				throw CantOpenRegistryKey(L"GUID文字列が変換できないよ～");

			::RegCloseKey(hkey);

			return resultCLSID;
		}
	};

	CLSIDList Registry::clsids;
	SubKeyList Registry::subkeys;
}