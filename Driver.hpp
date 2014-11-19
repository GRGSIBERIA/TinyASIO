#pragma once
#include <Windows.h>
#include <exception>
#include <string>
#include <memory>

#include "Exception.hpp"
#include "Registory.hpp"
#include "SDK.hpp"
#include "Interface.hpp"
#include "Channel.hpp"

namespace asio
{
	/**
	* ASIO�h���C�o�̃C���^�[�t�F�[�X�̃��b�p�N���X
	*/
	class Driver
	{
	private:
		static std::shared_ptr<Driver> driver;	// �V���O���g��

		//Interface iasio;
		IASIO *iasio;			//!< �C���^�[�t�F�[�X�ւ̃|�C���^
		void *systemHandle;		//!< �V�X�e���n���h��
		SubKey subkey;			//!< ���W�X�g���̏ꏊ

		std::string driverName;	//!< �h���C�o��
		long driverVersion;		//!< �h���C�o�̃o�[�W����

		std::shared_ptr<ChannelManager> channelManager;

	private:
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

			channelManager.reset(new asio::ChannelManager(iasio), [](asio::ChannelManager* p) { delete p; });
		}


	public:
		const std::string& Name() const { return driverName; }	//!< �h���C�o����Ԃ�
		const long& Version() const { return driverVersion; }	//!< �h���C�o�̃o�[�W������Ԃ�

		IASIO* Interface() { return iasio; }					//!< ASIO�̃C���^�[�t�F�[�X��Ԃ�

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

		const ChannelManager& ChannelManager() const { return *channelManager; }


	private:
		// ���Ԃ̓s����C�����ɂ���
		template <typename T>
		static Driver& InitX(const T& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}


	public:
		/**
		* �h���C�o�̏�����
		* @params[in] asioDriverName ASIO�h���C�o�̖��O���������āC����ŏ��������s��
		* @return asioDriverName�ŏ��������ꂽ�h���C�o�̃C���X�^���X
		* @note ������Ȃ��ꍇ�͂��Ԃ񗎂���
		*/
		static Driver& Init(const std::string& asioDriverName) { return InitX(asioDriverName); }
		

		/**
		* �h���C�o�̏�����
		* @params[in] asioDriverName ASIO�h���C�o�̖��O���������āC����ŏ��������s��
		* @return asioDriverName�ŏ��������ꂽ�h���C�o�̃C���X�^���X
		* @note ������Ȃ��ꍇ�͂��Ԃ񗎂���
		*/
		static Driver& Init(const std::wstring& asioDriverName)	{ return InitX(asioDriverName);	}


		/**
		* �h���C�o�̎擾
		*/
		static Driver& Get() { return *driver; }


		/**
		* �h���C�o�̉���Ȃ�
		*/
		~Driver() {	}
	};

	std::shared_ptr<Driver> Driver::driver;
}