#pragma once
#include <Windows.h>
#include <exception>
#include <string>

#include "Interface.hpp"
#include "Structure.hpp"
#include "Channel.hpp"
#include "BufferManager.hpp"

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
	* ���ȏ㏉�������ꂽ��ȂǂŌĂяo�����
	*/
	class OverTwiceCallException : std::exception
	{
	public:
		OverTwiceCallException(const std::string& message)
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
			bufferManager = nullptr;
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
		const IASIO& Interface() const { return *iasio; }


	public:		// �`�����l������

		/**
		* ���̓`�����l���̔z���Ԃ�
		* @return ���̓`�����l���̔z��
		*/
		inline const std::vector<InputChannel>& InputChannels() const { return channelManager->Inputs(); }
		
		/**
		* �o�̓`�����l���̔z���Ԃ�
		* @return �o�̓`�����l���̔z��
		*/
		inline const std::vector<OutputChannel>& OutputChannels() const { return channelManager->Outputs(); }

		/**
		* �`�����l����ǉ�
		*/
		inline const void AddChannel(const Channel& channel) { bufferManager->AddChannel(channel); }

		/**
		* �o�^�����`�����l�����폜
		*/
		inline const void ClearChannels() { bufferManager->ClearChannel(); }


	public:		// �o�b�t�@����

		/**
		* ASIO�̃o�b�t�@�̐ݒ���擾
		* @return �o�b�t�@�̌��݂̐ݒ�
		* @note BufferPreference�̒l��ύX����CreateBuffer�֓n��
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
		* @params[in] channel �o�b�t�@�𐶐��������`�����l��
		* @params[in] bufferPref �o�b�t�@�̐ݒ�
		*/
		const BufferController& CreateBuffer(const Channel& channel, const BufferPreference& bufferPref)
		{
			ASIOCallbacks callback = callback::CallbackManager::CreateCallbacks();

			if (bufferManager != nullptr)	// �o�b�t�@���d�����ė��p�����Ȃ����
				delete bufferManager;
			bufferManager = new BufferManager(iasio);

			auto& bufferCtrl = bufferManager->CreateBuffer(bufferPref, channel.sampleType, &callback);
			
			return bufferCtrl;
		}

		/**
		* �o�b�t�@�̐���
		* @params[in] channel �o�b�t�@�𐶐��������`�����l��
		* @note ���̊֐����g���ƃh���C�o���Őݒ肳��Ă���o�b�t�@�T�C�Y�𗘗p���܂�
		*/
		const BufferController& CreateBuffer(const Channel& channel)
		{
			return CreateBuffer(channel, GetBufferPreference());
		}

	public:
		/**
		* �h���C�o�̏�����
		* @params[in] clsid ASIO�h���C�o��CLSID
		* @note �ȑO�ɐ������ꂽ�h���C�o�͔j�������
		*/
		static void Init(const CLSID& clsid)
		{
			Driver::driver.reset(new Driver(clsid), [](Driver *p) { delete p; });
		}

		/**
		* �h���C�o�C���X�^���X�̎擾
		* @return �h���C�o�C���X�^���X
		*/
		static Driver& Get()
		{
			return *Driver::driver;
		}

		/**
		* �h���C�o�̉���Ȃ�
		*/
		virtual ~Driver()
		{
			delete channelManager;
			delete bufferManager;

			iasio->Release();
		}
	};

	std::shared_ptr<Driver> Driver::driver;

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