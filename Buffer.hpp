#pragma once
#include <vector>
#include <memory>
#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"
#include "Channel.hpp"


namespace asio
{
	/**
	* �o�b�t�@�N���X
	*/
	class Buffer
	{
		IOType ioType;
		long channelNumber;
		long bufferSize;
		void* bufferData[2];
		ASIOCallbacks* callbacks;

	public:
		Buffer(const ASIOBufferInfo& info, const long bufferSize, ASIOCallbacks* callbacks)
			: ioType((IOType)info.isInput), channelNumber(info.channelNum), bufferSize(bufferSize), callbacks(callbacks)
		{
			bufferData[0] = info.buffers[0];
			bufferData[1] = info.buffers[1];
		}
	};

	typedef std::vector<Buffer> BufferArray;

	/**
	* �o�b�t�@���Ǘ�����N���X
	*/
	class BufferManager
	{
		IASIO* iasio;

		BufferArray buffers;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, ASIOCallbacks* callbacks)
		{
			buffers.clear();
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				buffers.emplace_back(info, bufferSize, callbacks);
			}
		}

	public:
		BufferManager(IASIO* iasio)
			: iasio(iasio)
		{

		}

		~BufferManager()
		{
			if (bufferInfos.size() > 0)
				ErrorCheck(iasio->disposeBuffers());
		}

		/**
		* �o�b�t�@�����O�J�n
		*/
		void Start()
		{
			ErrorCheck(iasio->start());
		}

		/**
		* �o�b�t�@�����O�I��
		*/
		void Stop()
		{
			ErrorCheck(iasio->stop());
		}

		/**
		* �o�b�t�@�����O�������`�����l����ǉ�
		*/
		void AddChannel(const IOType& ioType, const long& channelNumber)
		{
			ASIOBufferInfo info;
			info.channelNum = channelNumber;
			info.isInput = ioType;
			bufferInfos.push_back(info);
		}

		/**
		* �o�b�t�@�����O�������`�����l����ǉ�
		*/
		void AddChannel(const Channel& channel)
		{
			AddChannel(channel.ioType, channel.ioType);
		}

		/**
		* �o�b�t�@�����O�������`�����l��������ςȂ��ɂ���
		*/
		void ClearChannel()
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
		const BufferArray& CreateBuffer(const long& bufferSize, ASIOCallbacks* callbacks)
		{
			asio::ASIOBufferInfo* infos = &bufferInfos.at(0);
			auto result = iasio->createBuffers(infos, bufferInfos.size(), bufferSize, callbacks);
			ErrorCheck(result);
			InitBuffers(bufferSize, callbacks);
			return buffers;
		}

		/**
		* �o�b�t�@�̐���
		* @params[in] bufferPreference �o�b�t�@�̐ݒ�
		* @params[in, out] callbacks �o�b�t�@�����O���̒ʒm�̂��߂ɗ��p
		* @note bufferSize�͎��R�ɐ��l�����߂��Ȃ��̂Œ���, (bufferSize % granularity == 0)�ȊO�̐��l�͕ۏ�ł��Ȃ�
		*/
		const BufferArray& CreateBuffer(const BufferPreference& bufferPreference, ASIOCallbacks* callbacks)
		{
			CreateBuffer(bufferPreference.preferredSize, callbacks);
			return buffers;
		}

		/**
		* �����I�Ƀo�b�t�@�����
		*/
		void DisposeBuffers() const
		{
			ErrorCheck(iasio->disposeBuffers());
		}
		
	};
}