#pragma once
#include "SDK.hpp"
#include "Registory.hpp"

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
		void RetryCreateInstance(const CLSID& clsid, const SubKey& subkey)
		{
			// �f�t�H���g����ThreadingModel��STA�Ȃ̂ŁCSTA/MTA�iBoth�j�ɕύX���čĎ��s����
			if (Registory::ChangeTheadingModel(subkey) != ERROR_SUCCESS)
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");

			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");
		}

	public:
		/**
		* @params[in] clsid ���[�h������CLSID
		* @params[in] subkey ���W�X�g���̈ʒu�Ȃ�
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
		*/
		const std::string& Name() const { return driverName; }

		/**
		* �h���C�o�̃o�[�W������Ԃ�
		*/
		const long& Version() const { return driverVersion; }

		/**
		* �C���^�[�t�F�[�X��Ԃ�
		*/
		IASIO* IASIO() { return iasio; }
	};
}