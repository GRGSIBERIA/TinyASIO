#pragma once
#include <vector>
#include "Interface.hpp"
#include "Channel.hpp"
#include "Buffer.hpp"
#include "Callback.hpp"

namespace asio
{

	/**
	* �o�b�t�@���Ǘ�����N���X
	*/
	class BufferManager
	{
		IASIO* iasio;

		BufferController bufferController;
		callback::CallbackManager callbackManager;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, const ASIOSampleType& sampleType, ASIOCallbacks* callbacks)
		{
			bufferController.Clear();
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				bufferController.Add(info, bufferSize, sampleType);
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
		const std::vector<ASIOBufferInfo>& BufferingChannels() const { return bufferInfos; }

		/**
		* �o�b�t�@�̐���
		* @params[in] bufferSize �o�b�t�@�̐ݒ�
		* @params[in, out] callbacks �o�b�t�@�����O���̒ʒm�̂��߂ɗ��p
		* @note bufferSize�͎��R�ɐ��l�����߂��Ȃ��̂Œ���, (bufferSize % granularity == 0)�ȊO�̐��l�͕ۏ�ł��Ȃ�
		*/
		const BufferController& CreateBuffer(const long& bufferSize, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			ErrorCheck(iasio->createBuffers(&bufferInfos[0], bufferInfos.size(), bufferSize, callbacks));
			InitBuffers(bufferSize, sampleType, callbacks);
			callbackManager.Init(&bufferController.inputBuffers, &bufferController.outputBuffers);
			return bufferController;
		}

		/**
		* �o�b�t�@�̐���
		* @params[in] bufferPreference �o�b�t�@�̐ݒ�
		* @params[in, out] callbacks �o�b�t�@�����O���̒ʒm�̂��߂ɗ��p
		* @note bufferSize�͎��R�ɐ��l�����߂��Ȃ��̂Œ���, (bufferSize % granularity == 0)�ȊO�̐��l�͕ۏ�ł��Ȃ�
		*/
		const BufferController& CreateBuffer(const BufferPreference& bufferPreference, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			CreateBuffer(bufferPreference.preferredSize, sampleType, callbacks);
			return bufferController;
		}
	};
}