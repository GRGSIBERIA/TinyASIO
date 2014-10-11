#pragma once
#include <vector>
#include "Interface.hpp"
#include "Structure.hpp"


namespace asio
{
	/**
	* �f�o�C�X�̃`�����l�����
	*/
	struct Channel
	{
		long number;				// �`�����l���ԍ�
		IOType ioType;				// ���o�͂̂ǂ��炩
		ASIOBool isActive;				// �`�����l�����L����������
		long group;					// �`�����l�����������Ă���O���[�v
		std::string name;			// �`�����l����
		ASIOSampleType sampleType;	// �T���v�����O�̎��

	public:
		/**
		* @params[in] info �f�o�C�X���瓾���`�����l�����
		*/
		Channel(const ASIOChannelInfo& info)
		{
			number = info.channel;
			ioType = info.isInput ? IOType::Input : IOType::Output;
			isActive = info.isActive;
			group = info.channelGroup;
			name = info.name;
			sampleType = info.type;
		}
	};


	/**
	* �f�o�C�X�ŗ��p�\�ȃ`�����l���̊Ǘ��N���X
	*/
	class ChannelManager
	{
		IASIO* driver;

		long numInputChannels;
		long numOutputChannels;

		std::vector<Channel> inputChannels;
		std::vector<Channel> outputChannels;

	public:
		/**
		* ���͂̃`�����l�����𓾂�
		*/
		const long NumberOfInputChannels() const { return numInputChannels; }

		/**
		* �o�͂̃`�����l�����𓾂�
		*/
		const long NumberOfOutputChannels() const { return numOutputChannels; }

		/**
		* ���̓`�����l���̔z���Ԃ�
		*/
		const std::vector<Channel>& Inputs() const { return inputChannels; }

		/**
		* �o�̓`�����l���̔z���Ԃ�
		*/
		const std::vector<Channel>& Outputs() const { return outputChannels; }

		/**
		* �Y��������̓`�����l����Ԃ�
		*/
		const Channel& Inputs(const long i) const { return inputChannels[i]; }

		/**
		* �Y������o�̓`�����l����Ԃ�
		*/
		const Channel& Outputs(const long i) const { return outputChannels[i]; }

	private:
		void InitNumberOfChannels()
		{
			ErrorCheck(driver->getChannels(&numInputChannels, &numOutputChannels));
		}

		void GetChannel(const long index, const ASIOBool isInput)
		{
			ASIOChannelInfo info;
			info.channel = index;
			info.isInput = isInput;
			ErrorCheck(driver->getChannelInfo(&info));
			
			if (isInput == 1)
				inputChannels.emplace_back(info);
			else
				outputChannels.emplace_back(info);
		}

		void InitChannels()
		{
			for (int i = 0; i < numInputChannels; ++i)
				GetChannel(i, 1);

			for (int i = 0; i < numOutputChannels; ++i)
				GetChannel(i, 0);
		}

	public:
		ChannelManager(IASIO* iasio)
			: driver(iasio)
		{
			InitNumberOfChannels();
			InitChannels();
		}
	};
}