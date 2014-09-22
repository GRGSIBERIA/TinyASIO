#pragma once
#include <Windows.h>
#include <exception>
#include <string>

#include "Interface.hpp"

namespace asio
{
	/**
	* �h���C�o�̃C���X�^���X�ɐ������s����ƌĂ΂��
	*/
	class CantCreateInstance : std::exception
	{
	public:
		CantCreateInstance(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* �h���C�o�������Ȃ����ɌĂ΂��
	*/
	class CantProcessException : std::exception
	{
	public:
		CantProcessException(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* ASIO�h���C�o�̃C���^�[�t�F�[�X�̃��b�p�N���X
	*/
	class ASIODriver
	{
	private:
		IASIO *driver;			// �C���^�[�t�F�[�X�ւ̃|�C���^
		void *systemHandle;		// ��̃V�X�e���n���h��

		std::string driverName;
		long driverVersion;

	private:
		void ErrorCheck(const ASIOError& error) const
		{
			switch (error)
			{
			case ASE_NotPresent:
				throw CantProcessException("hardware input or output is not present or available");
			case ASE_HWMalfunction:
				throw CantProcessException("hardware is malfunctioning (can be returned by any ASIO function)");
			case ASE_InvalidParameter:
				throw CantProcessException("input parameter invalid");
			case ASE_InvalidMode:
				throw CantProcessException("hardware is in a bad mode or used in a bad mode");
			case ASE_SPNotAdvancing:
				throw CantProcessException("hardware is not running when sample position is inquired");
			case ASE_NoClock:
				throw CantProcessException("sample clock or rate cannot be determined or is not present");
			case ASE_NoMemory:
				throw CantProcessException("not enough memory for completing the request");
			}
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

		/**
		* �h���C�o�̃C���^�[�t�F�[�X��Ԃ�
		*/
		const IASIO& Interface() const { return *driver; }

		/**
		* ���͂̒x����Ԃ�
		*/
		long InputLatency() const 
		{
			long i, o;
			ErrorCheck(driver->getLatencies(&i, &o));
			return i;
		}

		/**
		* �o�͂̒x����Ԃ�
		*/
		long OutputLatency() const 
		{
			long i, o;
			ErrorCheck(driver->getLatencies(&i, &o));
			return o;
		}

	public:
		/**
		* �o�b�t�@�����O�J�n
		*/
		void Start()
		{
			ErrorCheck(driver->start());
		}

		/**
		* �o�b�t�@�����O�I��
		*/
		void Stop()
		{
			ErrorCheck(driver->stop());
		}

		/**
		* @params[in] clsid ���[�h������CLSID
		*/
		ASIODriver(const CLSID& clsid)
		{
			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&driver);
			if (FAILED(hr))
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");

			driver->init(systemHandle);

			// ���O�ƃh���C�o�̃o�[�W���������擾
			char buffer[360];
			driver->getDriverName(buffer);
			driverName = buffer;
			driverVersion = driver->getDriverVersion();
		}

		virtual ~ASIODriver()
		{
			driver->Release();
		}
	};
}