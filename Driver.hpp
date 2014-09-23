#pragma once
#include <Windows.h>
#include <exception>
#include <string>

#include "Interface.hpp"
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
		* ���o�͂̒x����\���\����
		*/
		struct IOLatency
		{
			long input, output;
		};

		/**
		* �`�����l������\���\����
		*/
		struct IOChannels
		{
			long input, output;
		};

		/**
		* �o�b�t�@�̌��݂̐ݒ��\���\����
		*/
		struct BufferPreference
		{
			long maxSize;		// �o�b�t�@�T�C�Y�̍ő�l
			long minSize;		// �o�b�t�@�T�C�Y�̍ŏ��l
			long preferredSize;	// �ݒ蒆�̒l
			long granularity;	// �ݒ�̗��x
		};

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
		* ���͂̃`�����l������Ԃ�
		* @return ���͂̃`�����l����
		*/
		long InputChannels() const
		{
			long i, o;
			ErrorCheck(driver->getChannels(&i, &o));
			return i;
		}

		/**
		* �o�͂̃`�����l������Ԃ�
		* @return �o�͂̃`�����l����
		*/
		long OutputChannels() const
		{
			long i, o;
			ErrorCheck(driver->getChannels(&i, &o));
			return o;
		}

		/**
		* ���o�͂̃`�����l������Ԃ�
		* @return ���o�͂̃`�����l����
		*/
		IOChannels Channels() const
		{
			IOChannels channels;
			ErrorCheck(driver->getChannels(&channels.input, &channels.output));
			return channels;
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
		* @return �`�����l�����
		*/
		Channel& ChannelInfo(const long channelNumber) const
		{
			ASIOChannelInfo info;
			info.channel = channelNumber;
			ErrorCheck(driver->getChannelInfo(&info));
			return Channel(info);
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
			driver->Release();
		}
	};

	// ����
	//virtual void getErrorMessage(char *string) = 0;
	//virtual ASIOError canSampleRate(ASIOSampleRate sampleRate) = 0;
	//virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources) = 0;
	//virtual ASIOError setClockSource(long reference) = 0;
	//virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) = 0;
	//virtual ASIOError getChannelInfo(ASIOChannelInfo *info) = 0;
	//virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
	//	long bufferSize, ASIOCallbacks *callbacks) = 0;
	//virtual ASIOError disposeBuffers() = 0;
	//virtual ASIOError controlPanel() = 0;
	//virtual ASIOError future(long selector, void *opt) = 0;
	//virtual ASIOError outputReady() = 0;
}