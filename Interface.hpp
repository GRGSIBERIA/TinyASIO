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
#include "SDK.hpp"
#include "Registry.hpp"

namespace asio
{
	class Interface
	{
		IASIO *iasio;			//!< �C���^�[�t�F�[�X�ւ̃|�C���^
		void *systemHandle;		//!< �V�X�e���n���h��
		SubKey subkey;			//!< ���W�X�g���̏ꏊ

		std::string driverName;	//!< �h���C�o��
		long driverVersion;		//!< �h���C�o�̃o�[�W����

	private:
		void RetryCreateInstance(const CLSID& clsid, const SubKey& subkeyData)
		{
			// �f�t�H���g����ThreadingModel��STA�Ȃ̂ŁCSTA/MTA�iBoth�j�ɕύX���čĎ��s����
			if (Registry::ChangeTheadingModel(subkeyData) != ERROR_SUCCESS)
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");

			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");
		}

	public:
		/**
		* @param clsid ���[�h������CLSID
		* @param subkey ���W�X�g���̈ʒu�Ȃ�
		*/
		Interface(const CLSID& clsid, const SubKey& subkey)
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

		/**
		* �h���C�o����Ԃ�
		* @return �h���C�o��
		*/
		const std::string& Name() const { return driverName; }

		/**
		* �h���C�o�̃o�[�W������Ԃ�
		* @return �h���C�o�̃o�[�W����
		*/
		const long& Version() const { return driverVersion; }

		/**
		* �C���^�[�t�F�[�X��Ԃ�
		* @return �h���C�o��COM�C���^�[�t�F�[�X
		*/
		IASIO* IASIO() { return iasio; }

		/**
		* �C���^�[�t�F�[�X�����
		*/
		void Release() { iasio->Release(); }
	};
}