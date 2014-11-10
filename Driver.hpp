#pragma once
#include <Windows.h>
#include <exception>
#include <string>
#include <memory>

#include "Exception.hpp"
#include "Registory.hpp"
#include "SDK.hpp"
#include "Interface.hpp"

namespace asio
{
	/**
	* ASIO�h���C�o�̃C���^�[�t�F�[�X�̃��b�p�N���X
	*/
	class Driver
	{
	private:
		static std::shared_ptr<Driver> driver;	// �V���O���g��

		Interface iasio;

	private:
		
		/**
		* @params[in] clsid ���[�h������CLSID
		* @params[in] subkey ���W�X�g���̈ʒu�Ȃ�
		*/
		Driver(const CLSID& clsid, const SubKey& subkey)
			: iasio(clsid, subkey) { }


	public:
		const std::string& Name() const { return iasio.Name(); }	//!< �h���C�o����Ԃ�
		const long& Version() const { return iasio.Version(); }		//!< �h���C�o�̃o�[�W������Ԃ�
		IASIO* Interface() { return iasio.IASIO(); }					//!< ASIO�̃C���^�[�t�F�[�X��Ԃ�

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