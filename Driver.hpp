#pragma once
#include <Windows.h>
#include <exception>
#include <string>
#include <memory>

#include "Exception.hpp"
#include "Registory.hpp"
#include "SDK.hpp"

namespace asio
{
	/**
	* ASIO�h���C�o�̃C���^�[�t�F�[�X�̃��b�p�N���X
	*/
	class Driver
	{
	private:
		static std::shared_ptr<Driver> driver;	// �V���O���g���ϐ�

		IASIO *iasio;			// �C���^�[�t�F�[�X�ւ̃|�C���^
		void *systemHandle;		// ��̃V�X�e���n���h��

		std::string driverName;
		long driverVersion;
		SubKey subkey;
		ASIOCallbacks callback;


	private:
		ASIOCallbacks InitNullCallbacks()
		{
			ASIOCallbacks callback;
			callback.asioMessage = NULL;
			callback.bufferSwitch = NULL;
			callback.bufferSwitchTimeInfo = NULL;
			callback.sampleRateDidChange = NULL;
			return callback;
		}


		void RetryCreateInstance(const CLSID& clsid, const SubKey& subkey)
		{
			// �f�t�H���g����ThreadingModel��STA�Ȃ̂ŁCSTA/MTA�iBoth�j�ɕύX���čĎ��s����
			if (Registory::ChangeTheadingModel(subkey) != ERROR_SUCCESS)
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");

			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");
		}


		/**
		* @params[in] clsid ���[�h������CLSID
		* @params[in] subkey ���W�X�g���̈ʒu�Ȃ�
		*/
		Driver(const CLSID& clsid, const SubKey& subkey)
			: subkey(subkey)
		{
			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				RetryCreateInstance(clsid, subkey);

			try
			{
				iasio->init(systemHandle);
			}
			catch (...)
			{
				throw CantHandlingASIODriver("�h���C�o�̃n���h���̏������Ɏ��s���܂���");
			}

			// ���O�ƃh���C�o�̃o�[�W���������擾
			char buffer[360];
			iasio->getDriverName(buffer);
			driverName = buffer;
			driverVersion = iasio->getDriverVersion();
		}

		

	public:

		/**
		* �h���C�o����Ԃ�
		*/
		const std::string& Name() const { return driverName; }

		/**
		* �h���C�o�̃o�[�W������Ԃ�
		*/
		const long& Version() const { return driverVersion; }


	public:
		/**
		* �h���C�o�̏�����
		* @params[in] subkey ASIO�h���C�o��Subkey
		* @return subkey�ŏ��������ꂽ�h���C�o�̃C���X�^���X
		* @note �ȑO�ɐ������ꂽ�h���C�o�͔j�������
		*/
		static Driver& Init(const SubKey& subkey)
		{
			auto clsid = Registory::GetCLSID(subkey.registoryPath);
			Driver::driver.reset(new Driver(clsid, subkey), [](Driver *p) { delete p; });
			return *Driver::driver;
		}


		/**
		* �h���C�o�̏�����
		* @params[in] asioDriverName ASIO�h���C�o�̖��O���������āC����ŏ��������s��
		* @return asioDriverName�ŏ��������ꂽ�h���C�o�̃C���X�^���X
		* @note ������Ȃ��ꍇ�͂��Ԃ񗎂���
		*/
		static Driver& Init(const std::string& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}
		

		/**
		* �h���C�o�̏�����
		* @params[in] asioDriverName ASIO�h���C�o�̖��O���������āC����ŏ��������s��
		* @return asioDriverName�ŏ��������ꂽ�h���C�o�̃C���X�^���X
		* @note ������Ȃ��ꍇ�͂��Ԃ񗎂���
		*/
		static Driver& Init(const std::wstring& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}


		/**
		* �h���C�o�̉���Ȃ�
		*/
		virtual ~Driver()
		{
			// �h���C�o��������ꂽ��Ԃ炵���C���܂�Ӗ��𐬂��Ȃ�
			//ErrorCheck(iasio->disposeBuffers());
			//ErrorCheck(iasio->Release());
			//CloseHandle(hMutex);	// �~���[�e�b�N�X�̃n���h�����J������
		}
	};

	std::shared_ptr<Driver> Driver::driver;
}