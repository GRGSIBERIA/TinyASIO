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

		ChannelManager* channelManager;
		BufferManager* bufferManager;

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

		/**
		* @params[in] clsid ���[�h������CLSID
		*/
		Driver(const CLSID& clsid)
		{
			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");

			iasio->init(systemHandle);

			// ���O�ƃh���C�o�̃o�[�W���������擾
			char buffer[360];
			iasio->getDriverName(buffer);
			driverName = buffer;
			driverVersion = iasio->getDriverVersion();

			channelManager = new ChannelManager(iasio);
			bufferManager = new BufferManager(iasio);
		}

		/**
		* �R�s�[�֎~
		*/
		Driver(const Driver& hoge) {}

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
		const IASIO& Interface() const { return *iasio; }


	public:		// �`�����l������

		/**
		* ���̓`�����l���̔z���Ԃ�
		* @return ���̓`�����l���̔z��
		*/
		inline const std::vector<Channel>& InputChannels() const { return channelManager->Inputs(); }
		
		/**
		* �o�̓`�����l���̔z���Ԃ�
		* @return �o�̓`�����l���̔z��
		*/
		inline const std::vector<Channel>& OutputChannels() const { return channelManager->Outputs(); }

		/**
		* �Y��������̓`�����l����Ԃ�
		* @return ���̓`�����l��
		*/
		inline const Channel& InputChannel(const long i) const { return channelManager->Inputs(i); }

		/**
		* �Y������o�̓`�����l����Ԃ�
		* @return �o�̓`�����l��
		*/
		inline const Channel& OutputChannel(const long i) const { return channelManager->Outputs(i); }


	public:		// �o�b�t�@����

		/**
		* ASIO�̃o�b�t�@�̐ݒ���擾
		* @return �o�b�t�@�̌��݂̐ݒ�
		* @note �K�������M�p�ł���l���擾�ł���Ƃ͌���Ȃ�
		*/
		BufferPreference GetBufferPreference() const
		{
			BufferPreference buf;
			ErrorCheck(iasio->getBufferSize(&buf.minSize, &buf.maxSize, &buf.preferredSize, &buf.granularity));
			return buf;
		}

		/**
		* �o�b�t�@�̐���
		* @note ���̊֐����g���ƃh���C�o���Őݒ肳��Ă���o�b�t�@�T�C�Y�𗘗p���܂�
		*/
		const BufferArray& CreateBuffer(const Channel& channel)
		{
			ASIOCallbacks callback = Buffer::CreateCallbacks();

			bufferManager->AddChannel(channel);
			auto retval = bufferManager->CreateBuffer(GetBufferPreference(), &callback);
			bufferManager->ClearChannel();
			
			return retval;
		}

		/**
		* �o�b�t�@�����O�J�n
		*/
		const void Start() const
		{
			iasio->start();
		}

		/**
		* �o�b�t�@�����O�ꎞ��~
		*/
		const void Stop() const
		{
			iasio->stop();
		}

	public:
		/**
		* �V���O���g�����\�b�h
		*/
		static Driver& Get(const CLSID& clsid)
		{
			if (Driver::driver == nullptr)
				Driver::driver = std::make_shared<Driver>(clsid);
		}

		/**
		* �h���C�o�̉���Ȃ�
		*/
		virtual ~Driver()
		{
			iasio->disposeBuffers();
			iasio->Release();

			delete channelManager;
			delete bufferManager;
		}
	};

	std::shared_ptr<Driver> Driver::driver = std::make_shared<Driver>(nullptr);

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