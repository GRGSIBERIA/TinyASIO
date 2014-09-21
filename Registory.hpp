#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <exception>

#include <cstdlib>
#include <stdlib.h>

namespace asio
{
	class CantOpenRegistoryKey : public std::exception
	{
	public:
		CantOpenRegistoryKey(const std::string& regPath) : std::exception(("レジストリを開けません: " + regPath).c_str()) { }
	};


	class CantOpenSubKeyIndex : public std::exception
	{
	public:
		CantOpenSubKeyIndex(const std::string& regPath) : std::exception(("サブキーのインデックスが開けません:" + regPath).c_str()) {}
	};


	const std::string ASIO_REGISTORY_PATH = "SOFTWARE\\ASIO";
	

	/**
	* サブキーとドライバー名を格納するための構造体
	*/
	struct SubKey
	{
	public:
		const std::string registoryPath;
		const std::string driverName;

		SubKey(const std::string& regPath, const std::string& driverName)
			: registoryPath(regPath), driverName(driverName) {}
	};



	/**
	* ASIO関連のレジストリを探す処理
	*/
	class ASIORegistory
	{
	public:
		typedef std::shared_ptr<std::vector<SubKey>> SubKeyList;
		typedef std::shared_ptr<std::vector<CLSID>> CLSIDList;
		
	private:
		static LONG WrappedRegOpenKey(HKEY mainKey, const std::string& regPath, HKEY& hkey)
		{
			return RegOpenKeyEx(mainKey, (LPCTSTR)regPath.c_str(), 0, KEY_ALL_ACCESS, &hkey);
		}


		static bool Exist(HKEY mainKey, const std::string& regPath)
		{
			HKEY hkey;

			LONG cr = WrappedRegOpenKey(mainKey, regPath.c_str(), hkey);
			if (cr != ERROR_SUCCESS)
				return false;

			::RegCloseKey(hkey);

			return true;
		}

		static const std::string GetSubKey(HKEY& hkey, const DWORD index)
		{
			// RegEnumKeyExに失敗したら例外を発生させておく
			DWORD max_path_size = 360;
			char *subkeyBuffer = new char[max_path_size];
			LONG cr = RegEnumKeyEx(hkey, index, subkeyBuffer, &max_path_size, NULL, NULL, NULL, NULL);
			if (cr != ERROR_SUCCESS)
				throw CantOpenSubKeyIndex(ASIO_REGISTORY_PATH);	// この例外でbreakできる
				
			std::string result = subkeyBuffer;
			delete[] subkeyBuffer;		// 正直，こういうことやりたくない
			return result;
		}

		static const std::wstring ToWide(std::string src)
		{
			std::wstring strWide;
			size_t size = src.length() + 1;
			wchar_t* converter = new wchar_t[size];

#if defined(_MSC_VER) && _MSC_VER >= 1400 
#pragma warning(push) 
#pragma warning(disable:4996) 
#endif 
			// デバッグモードでのエラーを潰す
			std::mbstowcs(converter, src.c_str(), size);
#if defined(_MSC_VER) && _MSC_VER >= 1400 
#pragma warning(pop) 
#endif 
			strWide = converter;
			delete[] converter;
			return strWide;
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
		*/
		static SubKeyList GetAsioDriverPathes()
		{
			SubKeyList resultSubKeys(new std::vector<SubKey>());

			HKEY hkey;
			LONG cr = WrappedRegOpenKey(HKEY_LOCAL_MACHINE, ASIO_REGISTORY_PATH, hkey);
			if (cr != ERROR_SUCCESS)
				return resultSubKeys;

			DWORD index = 0;
			while (true)
			{
				try
				{
					const auto subkey = GetSubKey(hkey, index);
					resultSubKeys->emplace_back(ASIO_REGISTORY_PATH + "\\" + subkey, subkey);
				}
				catch (CantOpenSubKeyIndex)
				{
					// 開けないので帰る
					break;
				}
				
				index++;
			}

			::RegCloseKey(hkey);

			return resultSubKeys;
		}

		/**
		* ASIOドライバーのCLSIDを取得する
		*/
		static CLSID GetCLSID(const std::string& regPath)
		{
			HKEY hkey;
			if (WrappedRegOpenKey(HKEY_LOCAL_MACHINE, regPath, hkey) != ERROR_SUCCESS)
				throw CantOpenRegistoryKey(regPath);

			DWORD dataSize = 360 * sizeof(TCHAR);
			TCHAR clsidStrBuffer[360];
			RegQueryValueEx(hkey, TEXT("CLSID"), NULL, NULL, (LPBYTE)clsidStrBuffer, &dataSize);

			// レジストリエントリの値が16進数の文字列
			std::wstring clsidStr = ToWide(clsidStrBuffer);
			CLSID resultCLSID;

			auto check = CLSIDFromString(clsidStr.c_str(), (LPCLSID)&resultCLSID);
			if (check != S_OK)
				throw CantOpenRegistoryKey("GUID文字列が変換できないよ～");

			::RegCloseKey(hkey);

			return resultCLSID;
		}

		/**
		* SubKeyListに存在する全てのパスからCLSIDを取得する
		*/
		static CLSIDList GetCLSIDs(const SubKeyList& subkeyList)
		{
			CLSIDList clsids(new std::vector<CLSID>());

			for (const auto& subkey : *subkeyList)
			{
				CLSID clsid = GetCLSID(subkey.registoryPath);
				clsids->push_back(clsid);
			}

			return clsids;
		}
	};
}