#pragma once
#include <Windows.h>
#include <exception>
#include <string>

#include "Interface.hpp"
#include "Structure.hpp"
#include "Buffer.hpp"
#include "Channel.hpp"

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
		* �o�b�t�@�̐ݒ���擾
		* @return �o�b�t�@�̌��݂̐ݒ�
		* @note �K�������M�p�ł���l���擾�ł���Ƃ͌���Ȃ�
		*/
		BufferPreference GetBufferPreference() const
		{
			BufferPreference buf;
			ErrorCheck(driver->getBufferSize(&buf.minSize, &buf.maxSize, &buf.preferredSize, &buf.granularity));
			return buf;
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

		/**
		* �h���C�o�̉���Ȃ�
		*/
		virtual ~ASIODriver()
		{
			driver->disposeBuffers();
			driver->Release();
		}
	};

	// ����
	//virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) = 0;
	//virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
	//	long bufferSize, ASIOCallbacks *callbacks) = 0;
	//virtual ASIOError disposeBuffers() = 0;
	//virtual ASIOError controlPanel() = 0;
	//virtual ASIOError future(long selector, void *opt) = 0;
	//virtual ASIOError outputReady() = 0;

	// �K�v�Ȃ̂��ǂ����킩��Ȃ��̂ŕۗ�
	//virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources) = 0;
	//virtual ASIOError setClockSource(long reference) = 0;
	//virtual void getErrorMessage(char *string) = 0;
}