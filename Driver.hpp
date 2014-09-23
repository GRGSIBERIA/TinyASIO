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
		* ���͂̒x����Ԃ�
		* @return ���͂̒x��
		*/
		long InputLatency() const 
		{
			long i, o;
			ErrorCheck(driver->getLatencies(&i, &o));
			return i;
		}

		/**
		* �o�͂̒x����Ԃ�
		* @return �o�͂̒x��
		*/
		long OutputLatency() const 
		{
			long i, o;
			ErrorCheck(driver->getLatencies(&i, &o));
			return o;
		}

		/**
		* ���͂Əo�͂̒x����Ԃ�
		* @return ���o�͂̒x��
		*/
		IOLatency Latencies() const
		{
			IOLatency latency;
			ErrorCheck(driver->getLatencies(&latency.input, &latency.output));
			return latency;
		}

		/**
		* �T���v�����O�E���[�g��Ԃ�
		* @return �T���v�����O�E���[�g
		*/
		double SampleRate() const
		{
			double rate;
			ErrorCheck(driver->getSampleRate(&rate));
			return rate;
		}

		/**
		* �T���v�����O�E���[�g��ݒ肷��
		* @params[in] rate �T���v�����O�E���[�g
		*/
		void SampleRate(double rate)
		{
			ErrorCheck(driver->setSampleRate(rate));
		}

		/**
		* �T���v�����O�E���[�g��ݒ�ł��邩�����炵��
		* @params[in] rate �T���v�����O�E���[�g
		* @note �悭�킩��Ȃ��̂Ŕ񐄏��֐�
		*/
		void CanSampleRate(double rate)
		{
			ErrorCheck(driver->canSampleRate(rate));
		}

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

		/**
		* �`�����l���̏����擾
		* @params[in] channelNumber �擾�������`�����l���̔ԍ�
		* @return �`�����l�����
		*/
		Channel ChannelInfo(const long channelNumber) const
		{
			ASIOChannelInfo info;
			info.channel = channelNumber;
			ErrorCheck(driver->getChannelInfo(&info));
			return Channel(info);
		}

		

		/**
		* �o�b�t�@���擾
		*/
		ASIOBuffer GetBuffer(const long channelNumber, const long bufferSize) const
		{

		}

		/**
		* �o�b�t�@���擾
		*/
		ASIOBuffer GetBuffer(const Channel& channel, const long bufferSize) const
		{

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