#pragma once
#include <Windows.h>
#include <exception>
#include <string>
#include <memory>

#include "Registory.hpp"
#include "Interface.hpp"
#include "Structure.hpp"
#include "Channel.hpp"
#include "BufferManager.hpp"

namespace asio
{
	/**
	* �h���C�o�̃C���X�^���X�ɐ������s����ƌĂ΂��
	*/
	class CantCreateInstance : public std::exception
	{
	public:
		CantCreateInstance(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* ���ȏ㏉�������ꂽ��ȂǂŌĂяo�����
	*/
	class OverTwiceCallException : public std::exception
	{
	public:
		OverTwiceCallException(const std::string& message)
			: exception(message.c_str()) {}
	};

	/**
	* �h���C�o�̃n���h�����擾�ł��Ȃ�����
	*/
	class CantHandlingASIODriver : public std::exception
	{
	public:
		CantHandlingASIODriver(const std::string& message)
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
		SubKey subkey;

		std::shared_ptr<ChannelManager> channelManager;
		std::shared_ptr<BufferManager> bufferManager;

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


		void RetryCreateInstance(const CLSID& clsid, const SubKey& subkey)
		{
			// �f�t�H���g����ThreadingModel��STA�Ȃ̂ŁCSTA/MTA�iBoth�j�ɕύX���čĎ��s����
			if (Registory::ChangeTheadingModel(subkey) != ERROR_SUCCESS)
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");

			HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (LPVOID*)&iasio);
			if (FAILED(hr))
				throw CantCreateInstance("�h���C�o�̃C���X�^���X�����Ɏ��s���܂���");
		}


		/**
		* @params[in] clsid ���[�h������CLSID
		* @params[in] subkey ���W�X�g���̈ʒu�Ȃ�
		*/
		Driver(const CLSID& clsid, const SubKey& subkey)
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

			channelManager = std::shared_ptr<ChannelManager>(new ChannelManager(iasio));
			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(iasio));
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
		* �`�����l���̔z���ǉ�����
		* @params[in] channels �`�����l���̔z��
		* @params[in] isActiveChannelOnly ���̃t���O�������Ă���ƁC�L���ȃ`�����l���̂ݓo�^����
		* @tparam CHANNEL InputChannel��������OutputChannel
		*/
		template <typename CHANNEL>
		void AddChannels(const std::vector<CHANNEL>& channels, const bool isActiveChannelOnly = true)
		{
			for (const auto& channel : channels)
			{
				if (isActiveChannelOnly)	// isActiveOnly�t���O�������Ă���΁CisActive�̃`�F�b�N���s��
				{
					if (channel.isActive)
						bufferManager->AddChannel(channel);
				}
				else
				{
					// ���Ƀt���O�������ĂȂ���΁C���ʂɒʂ�
					bufferManager->AddChannel(channel);
				}
			}
		}


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
		* @params[in] sample �T���v�����O���@
		* @params[in] bufferPref �o�b�t�@�̐ݒ�
		* @return �o�b�t�@�̃R���g���[��
		*/
		const BufferController& CreateBuffer(const pack::Sample& sample, const BufferPreference& bufferPref)
		{
			ASIOCallbacks callback = callback::CallbackManager::CreateCallbacks();

			//if (bufferManager != nullptr)		// �o�b�t�@���d�����ė��p�����Ȃ����
			//	delete bufferManager;
			//bufferManager = new BufferManager(iasio);

			auto& bufferCtrl = bufferManager->CreateBuffer(bufferPref, sample.ToSampleType(), &callback);

			bufferManager = std::shared_ptr<BufferManager>(new BufferManager(iasio));
			
			return bufferCtrl;
		}

		/**
		* �o�b�t�@�𐶐�����
		* @params[in] sample �T���v�����O���@
		* @return �o�b�t�@�̃R���g���[��
		* @note ���̊֐����g���ƃh���C�o���Őݒ肳��Ă���o�b�t�@�T�C�Y�𗘗p���܂�
		*/
		const BufferController& CreateBuffer(const pack::Sample& sample)
		{
			return CreateBuffer(sample, GetBufferPreference());
		}


		/**
		* ���݂��Ă���S�Ẵ`�����l������o�b�t�@�𐶐�����
		* @params[in] sample �T���v�����O���@
		* @params[in] activeChannelOnly �L���ȃ`�����l���̂ݐ�������
		* @return �o�b�t�@�̃R���g���[��
		*/
		const BufferController& CreateBufferAll(const pack::Sample& sample, const bool activeChannelOnly = false)
		{
			bufferManager->ClearChannel();	// ���O�ɃN���A���Ă���

			AddChannels(channelManager->Inputs(), activeChannelOnly);
			AddChannels(channelManager->Outputs(), activeChannelOnly);

			if (bufferManager->BufferingChannels().size() <= 0)
				throw DontEntryAnyChannels("����`�����l�����o�^����Ă��܂���");

			return CreateBuffer(sample);
		}


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


		/**
		* �h���C�o�̏�����
		* @params[in] asioDriverName ASIO�h���C�o�̖��O���������āC����ŏ��������s��
		* @return asioDriverName�ŏ��������ꂽ�h���C�o�̃C���X�^���X
		* @note ������Ȃ��ꍇ�͂��Ԃ񗎂���
		*/
		static Driver& Init(const std::string& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}
		

		/**
		* �h���C�o�̏�����
		* @params[in] asioDriverName ASIO�h���C�o�̖��O���������āC����ŏ��������s��
		* @return asioDriverName�ŏ��������ꂽ�h���C�o�̃C���X�^���X
		* @note ������Ȃ��ꍇ�͂��Ԃ񗎂���
		*/
		static Driver& Init(const std::wstring& asioDriverName)
		{
			auto asioList = asio::Registory::GetAsioDriverPathes();
			auto asioRegistory = asioList.Find(asioDriverName);
			return Init(asioRegistory);
		}


		/**
		* �h���C�o�̉���Ȃ�
		*/
		virtual ~Driver()
		{
			iasio->Release();
		}
	};

	std::shared_ptr<Driver> Driver::driver;
}