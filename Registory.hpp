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
	/**
	* レジストリーキーが開けない
	*/
	class CantOpenRegistoryKey : public std::exception
	{
	public:
		CantOpenRegistoryKey(const std::string& regPath) : std::exception(("レジストリを開けません: " + regPath).c_str()) { }
	};


	/**
	* サブキーのインデックスが開けなくなっている
	*/
	class CantOpenSubKeyIndex : public std::exception
	{
	public:
		CantOpenSubKeyIndex(const std::string& regPath) : std::exception(("サブキーのインデックスが開けません:" + regPath).c_str()) {}
	};


	/**
	* ASIOのドライバーがひとつもない
	*/
	class DontFoundASIODrivers : public std::exception
	{
	public:
		DontFoundASIODrivers(const std::string& message)
			: exception(message.c_str()) {}
	};


	const std::string ASIO_REGISTORY_PATH = "SOFTWARE\\ASIO";
	

	/**
	* レジストリのパスとドライバー名を格納するための構造体
	*/
	struct SubKey
	{
	public:
		const std::string registoryPath;	/*!< レジストリのパス */
		const std::string driverName;		/*!< ドライバ名 */

		SubKey(const std::string& regPath, const std::string& driverName)
			: registoryPath(regPath), driverName(driverName) {}
	};



	/**
	* ASIO関連のレジストリを探す処理
	*/
	class Registory
	{
	public:
		typedef std::shared_ptr<std::vector<SubKey>> SubKeyList;
		typedef std::shared_ptr<std::vector<::CLSID>> CLSIDList;

		static SubKeyList subkeys;
		static CLSIDList clsids;
		
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
					subkeys->emplace_back(ASIO_REGISTORY_PATH + "\\" + subkey, subkey);
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
				throw DontFoundASIODrivers("ASIOのドライバーがひとつも存在しません");

			return *subkeys;
		}

		/**
		* ASIOドライバーのCLSIDを取得する
		*/
		static ::CLSID GetCLSID(const std::string& regPath)
		{
			HKEY hkey;
			if (WrappedRegOpenKey(HKEY_LOCAL_MACHINE, regPath, hkey) != ERROR_SUCCESS)
				throw CantOpenRegistoryKey(regPath);

			DWORD dataSize = 360 * sizeof(TCHAR);
			TCHAR clsidStrBuffer[360];
			RegQueryValueEx(hkey, TEXT("CLSID"), NULL, NULL, (LPBYTE)clsidStrBuffer, &dataSize);

			// レジストリエントリの値が16進数の文字列
			std::wstring clsidStr = ToWide(clsidStrBuffer);
			::CLSID resultCLSID;

			auto check = CLSIDFromString(clsidStr.c_str(), (LPCLSID)&resultCLSID);
			if (check != S_OK)
				throw CantOpenRegistoryKey("GUID文字列が変換できないよ～");

			::RegCloseKey(hkey);

			return resultCLSID;
		}
	};

	Registory::CLSIDList Registory::clsids;
	Registory::SubKeyList Registory::subkeys;
}