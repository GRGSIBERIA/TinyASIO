#pragma once
#include <vector>
#include "Interface.hpp"
#include "Channel.hpp"
#include "Buffer.hpp"
#include "Callback.hpp"
#include "Preference.hpp"

namespace asio
{
	/**
	* �`�����l�����o�^����Ă��Ȃ��ꍇ�ɑ��o������O
	*/
	class DontEntryAnyChannels : std::exception
	{
	public:
		DontEntryAnyChannels(const std::string& message) : exception(message.c_str()) {}
	};

	/**
	* �o�b�t�@���Ǘ�����N���X
	*/
	class BufferManager
	{
		IASIO* iasio;

		std::shared_ptr<BufferController> bufferController;
		callback::CallbackManager callbackManager;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, const ASIOSampleType& sampleType)
		{
			bufferController = std::shared_ptr<BufferController>(new BufferController(iasio, bufferSize));
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				bufferController->Add(info, bufferSize, sampleType);
			}
		}

	public:
		BufferManager(IASIO* iasio)
			: iasio(iasio), bufferController(iasio), callbackManager()
		{

		}

		~BufferManager()
		{
			if (bufferInfos.size() > 0)
				ErrorCheck(iasio->disposeBuffers());
		}

		/**
		* �o�b�t�@�����O�������`�����l����ǉ�
		*/
		inline void AddChannel(const IOType& ioType, const long& channelNumber)
		{
			ASIOBufferInfo info;
			info.channelNum = channelNumber;
			info.isInput = ioType;
			info.buffers[0] = 0;
			info.buffers[1] = 0;
			bufferInfos.push_back(info);
		}

		/**
		* �o�b�t�@�����O�������`�����l����ǉ�
		*/
		inline void AddChannel(const Channel& channel)
		{
			AddChannel(channel.ioType, channel.ioType);
		}

		/**
		* �o�b�t�@�����O�������`�����l��������ςȂ��ɂ���
		*/
		inline void ClearChannel()
		{
			bufferInfos.clear();
		}

		/**
		* �o�b�t�@�����O����`�����l����Ԃ�
		*/
		std::vector<ASIOBufferInfo>& BufferingChannels() { return bufferInfos; }


		/**
		* �o�b�t�@�̐���
		* @params[in] bufferSize �o�b�t�@�̐ݒ�
		* @params[in, out] callbacks �o�b�t�@�����O���̒ʒm�̂��߂ɗ��p
		* @note 
		* bufferSize�͎��R�ɐ��l�����߂��Ȃ��̂Œ���, (bufferSize % granularity == 0)�ȊO�̐��l�͕ۏ�ł��Ȃ��D
		* Sampling Rate�Ȃǂ̐ݒ���C�h���C�o���̐ݒ�Ɉˑ�����悤�ɂȂ��Ă���̂Œ���
		*/
		const BufferController& CreateBuffer(const long& bufferSize, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			Preference pref(iasio);
			pref.SetSampleRate();	// �f�t�H���g�̃T���v�����O���[�g��ݒ肷��

			ErrorCheck(iasio->createBuffers(&bufferInfos[0], bufferInfos.size(), bufferSize, callbacks));
			InitBuffers(bufferSize, sampleType);
			callbackManager.Init(&bufferController->inputBuffers, &bufferController->outputBuffers);
			return *bufferController;
		}

		/**
		* �o�b�t�@�̐���
		* @params[in] bufferPreference �o�b�t�@�̐ݒ�
		* @params[in, out] callbacks �o�b�t�@�����O���̒ʒm�̂��߂ɗ��p
		* @note bufferSize�͎��R�ɐ��l�����߂��Ȃ��̂Œ���, (bufferSize % granularity == 0)�ȊO�̐��l�͕ۏ�ł��Ȃ�
		* Sampling Rate�Ȃǂ̐ݒ���C�h���C�o���̐ݒ�Ɉˑ�����悤�ɂȂ��Ă���̂Œ���
		*/
		const BufferController& CreateBuffer(const BufferPreference& bufferPreference, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			CreateBuffer(bufferPreference.preferredSize, sampleType, callbacks);
			return *bufferController;
		}


		void EraseDisuseBuffer(const bool activeChannelOnly)
		{
			if (activeChannelOnly)
			{
				auto& buffers = bufferInfos;
				for (auto itr = buffers.begin(); itr != buffers.end(); ++itr)
				{
					// �s�v�ȃ`�����l�����폜����
					if ((*itr).buffers[0] == nullptr || (*itr).buffers[0] == nullptr)
						itr = buffers.erase(itr);
				}
			}
		}
	};
}