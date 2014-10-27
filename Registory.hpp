#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <algorithm>

#include <cstdlib>
#include <stdlib.h>

namespace asio
{
	/**
	* レジストリーキーが開けない
	*/
	class CantOpenRegistoryKey : public std::exception
	{
	public:
		CantOpenRegistoryKey(const std::wstring& regPath) : std::exception(("レジストリを開けません: " + std::string(regPath.begin(), regPath.end())).c_str()) { }
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


	const std::wstring ASIO_REGISTORY_PATH = L"SOFTWARE\\ASIO";
	

	/**
	* レジストリのパスとドライバー名を格納するための構造体
	*/
	struct SubKey
	{
	public:
		const std::wstring registoryPath;	/*!< レジストリのパス */
		const std::wstring driverName;		/*!< ドライバ名 */

		SubKey(const std::wstring& regPath, const std::wstring& driverName)
			: registoryPath(regPath), driverName(driverName) {}
	};

	class Registory;

	typedef std::shared_ptr<std::vector<SubKey>> SubKeyList;
	typedef std::shared_ptr<std::vector<::CLSID>> CLSIDList;


	/**
	* ASIOドライバのレジストリの配列を扱うためのラッパークラス
	*/
	class DriverList
	{
		friend Registory;

		SubKeyList subkeys;

		DriverList(SubKeyList& subkeys)
			: subkeys(subkeys) { }

	public:
		/**
		* ASIOドライバのレジストリキーの配列を取得する
		* @return ASIOドライバのレジストリキーの配列
		*/
		const std::vector<SubKey>& Items() const { return *subkeys; }


		/**
		* 配列からASIOドライバのレジストリを探す
		* @params[in] findName 探したいASIOドライバの名前
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
		* @params[in] findName 探したいASIOドライバの名前
		* @return ASIOドライバのレジストリ
		*/
		const SubKey& Find(const std::string findName)
		{
			std::wstring str(findName.begin(), findName.end());
			return Find(str);
		}
	};


	/**
	* ASIO関連のレジストリを探す処理
	*/
	class Registory
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

		static const std::wstring GetSubKey(HKEY& hkey, const DWORD index)
		{
			// RegEnumKeyExに失敗したら例外を発生させておく
			DWORD max_path_size = 360;
			wchar_t *subkeyBuffer = new wchar_t[max_path_size];
			LONG cr = RegEnumKeyEx(hkey, index, subkeyBuffer, &max_path_size, NULL, NULL, NULL, NULL);
			if (cr != ERROR_SUCCESS)
				throw CantOpenSubKeyIndex(ASIO_REGISTORY_PATH);	// この例外でbreakできる
				
			std::wstring result = subkeyBuffer;
			delete[] subkeyBuffer;		// 正直，こういうことやりたくない
			return result;
		}


	public:
		/**
		* SOFTWARE\ASIOが存在するか調べる
		*/
		static bool ExistDrivers()
		{
			return Exist(HKEY_LOCAL_MACHINE, ASIO_REGISTORY_PATH);
		}

		/**
		* 登録されているASIOドライバのレジストリのパスを返す
		* @return レジストリのパスの配列
		*/
		static std::vector<SubKey> GetAsioDriverPathes()
		{
			subkeys = SubKeyList(new std::vector<SubKey>());

			HKEY hkey;
			LONG cr = WrappedRegOpenKey(HKEY_LOCAL_MACHINE, ASIO_REGISTORY_PATH, hkey);
			if (cr != ERROR_SUCCESS)
				return *subkeys;

			DWORD index = 0;
			while (true)
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
				
				index++;
			}

			::RegCloseKey(hkey);

			if (subkeys->size() <= 0)
				throw DontFoundASIODrivers(L"ASIOのドライバーがひとつも存在しません");

			return *subkeys;
		}

		/**
		* ASIOドライバーのCLSIDを取得する
		*/
		static ::CLSID GetCLSID(const std::wstring& regPath)
		{
			HKEY hkey;
			if (WrappedRegOpenKey(HKEY_LOCAL_MACHINE, regPath, hkey) != ERROR_SUCCESS)
				throw CantOpenRegistoryKey(regPath);

			DWORD dataSize = 360 * sizeof(TCHAR);
			wchar_t clsidStrBuffer[360];
			RegQueryValueEx(hkey, TEXT("CLSID"), NULL, NULL, (LPBYTE)clsidStrBuffer, &dataSize);

			// レジストリエントリの値が16進数の文字列
			std::wstring clsidStr = clsidStrBuffer;
			::CLSID resultCLSID;

			auto check = CLSIDFromString(clsidStr.c_str(), (LPCLSID)&resultCLSID);
			if (check != S_OK)
				throw CantOpenRegistoryKey(L"GUID文字列が変換できないよ～");

			::RegCloseKey(hkey);

			return resultCLSID;
		}
	};

	Registory::CLSIDList Registory::clsids;
	Registory::SubKeyList Registory::subkeys;
}