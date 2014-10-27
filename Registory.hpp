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


	const std::wstring ASIO_REGISTORY_PATH = L"SOFTWARE\\ASIO";
	

	/**
	* ���W�X�g���̃p�X�ƃh���C�o�[�����i�[���邽�߂̍\����
	*/
	struct SubKey
	{
	public:
		const std::wstring registoryPath;	/*!< ���W�X�g���̃p�X */
		const std::wstring driverName;		/*!< �h���C�o�� */

		SubKey(const std::wstring& regPath, const std::wstring& driverName)
			: registoryPath(regPath), driverName(driverName) {}
	};

	class Registory;

	typedef std::shared_ptr<std::vector<SubKey>> SubKeyList;
	typedef std::shared_ptr<std::vector<::CLSID>> CLSIDList;


	/**
	* ASIO�h���C�o�̃��W�X�g���̔z����������߂̃��b�p�[�N���X
	*/
	class DriverList
	{
		friend Registory;

		SubKeyList subkeys;

		DriverList(SubKeyList& subkeys)
			: subkeys(subkeys) { }

	public:
		/**
		* ASIO�h���C�o�̃��W�X�g���L�[�̔z����擾����
		* @return ASIO�h���C�o�̃��W�X�g���L�[�̔z��
		*/
		const std::vector<SubKey>& Items() const { return *subkeys; }


		/**
		* �z�񂩂�ASIO�h���C�o�̃��W�X�g����T��
		* @params[in] findName �T������ASIO�h���C�o�̖��O
		* @return ASIO�h���C�o�̃��W�X�g��
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
		* �z�񂩂�ASIO�h���C�o�̃��W�X�g����T��
		* @params[in] findName �T������ASIO�h���C�o�̖��O
		* @return ASIO�h���C�o�̃��W�X�g��
		*/
		const SubKey& Find(const std::string findName)
		{
			std::wstring str(findName.begin(), findName.end());
			return Find(str);
		}
	};


	/**
	* ASIO�֘A�̃��W�X�g����T������
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
			// RegEnumKeyEx�Ɏ��s�������O�𔭐������Ă���
			DWORD max_path_size = 360;
			wchar_t *subkeyBuffer = new wchar_t[max_path_size];
			LONG cr = RegEnumKeyEx(hkey, index, subkeyBuffer, &max_path_size, NULL, NULL, NULL, NULL);
			if (cr != ERROR_SUCCESS)
				throw CantOpenSubKeyIndex(ASIO_REGISTORY_PATH);	// ���̗�O��break�ł���
				
			std::wstring result = subkeyBuffer;
			delete[] subkeyBuffer;		// �����C�����������Ƃ�肽���Ȃ�
			return result;
		}


	public:
		/**
		* SOFTWARE\ASIO�����݂��邩���ׂ�
		*/
		static bool ExistDrivers()
		{
			return Exist(HKEY_LOCAL_MACHINE, ASIO_REGISTORY_PATH);
		}

		/**
		* �o�^����Ă���ASIO�h���C�o�̃��W�X�g���̃p�X��Ԃ�
		* @return ���W�X�g���̃p�X�̔z��
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
					// �J���Ȃ��̂ŋA��
					break;
				}
				
				index++;
			}

			::RegCloseKey(hkey);

			if (subkeys->size() <= 0)
				throw DontFoundASIODrivers(L"ASIO�̃h���C�o�[���ЂƂ����݂��܂���");

			return *subkeys;
		}

		/**
		* ASIO�h���C�o�[��CLSID���擾����
		*/
		static ::CLSID GetCLSID(const std::wstring& regPath)
		{
			HKEY hkey;
			if (WrappedRegOpenKey(HKEY_LOCAL_MACHINE, regPath, hkey) != ERROR_SUCCESS)
				throw CantOpenRegistoryKey(regPath);

			DWORD dataSize = 360 * sizeof(TCHAR);
			wchar_t clsidStrBuffer[360];
			RegQueryValueEx(hkey, TEXT("CLSID"), NULL, NULL, (LPBYTE)clsidStrBuffer, &dataSize);

			// ���W�X�g���G���g���̒l��16�i���̕�����
			std::wstring clsidStr = clsidStrBuffer;
			::CLSID resultCLSID;

			auto check = CLSIDFromString(clsidStr.c_str(), (LPCLSID)&resultCLSID);
			if (check != S_OK)
				throw CantOpenRegistoryKey(L"GUID�����񂪕ϊ��ł��Ȃ���`");

			::RegCloseKey(hkey);

			return resultCLSID;
		}
	};

	Registory::CLSIDList Registory::clsids;
	Registory::SubKeyList Registory::subkeys;
}