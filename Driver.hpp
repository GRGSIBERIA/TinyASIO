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

		Interface iasio;
		std::shared_ptr<ChannelManager> channelManager;

	private:
		
		/**
		* @param[in] clsid ���[�h������CLSID
		* @param[in] subkey ���W�X�g���̈ʒu�Ȃ�
		*/
		Driver(const CLSID& clsid, const SubKey& subkey)
			: iasio(clsid, subkey)
		{
			channelManager.reset(new asio::ChannelManager(iasio.IASIO()), [](asio::ChannelManager* p) { delete p; });
		}


	public:
		const std::string& Name() const { return iasio.Name(); }	//!< �h���C�o����Ԃ�
		const long& Version() const { return iasio.Version(); }		//!< �h���C�o�̃o�[�W������Ԃ�

		IASIO* Interface() { return iasio.IASIO(); }				//!< ASIO�̃C���^�[�t�F�[�X��Ԃ�

	public:
		/**
		* �h���C�o�̏�����
		* @param[in] subkey ASIO�h���C�o��Subkey
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
		* @param[in] asioDriverName ASIO�h���C�o�̖��O���������āC����ŏ��������s��
		* @return asioDriverName�ŏ��������ꂽ�h���C�o�̃C���X�^���X
		* @note ������Ȃ��ꍇ�͂��Ԃ񗎂���
		*/
		static Driver& Init(const std::string& asioDriverName) { return InitX(asioDriverName); }
		

		/**
		* �h���C�o�̏�����
		* @param[in] asioDriverName ASIO�h���C�o�̖��O���������āC����ŏ��������s��
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
		~Driver() { }
	};

	std::shared_ptr<Driver> Driver::driver;
}