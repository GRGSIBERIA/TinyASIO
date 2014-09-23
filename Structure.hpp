#pragma once
#include <string>
#include "Interface.hpp"

namespace asio
{
	enum IOType
	{
		Input = 1,
		Output = 0
	};

	/**
	* �f�o�C�X�̃`�����l�����
	*/
	struct Channel
	{
		long number;			// �`�����l���ԍ�
		IOType ioType;			// ���o�͂̂ǂ��炩
		bool isActive;			// �`�����l�����L����������
		long group;				// �`�����l�����������Ă���O���[�v
		std::string name;		// �`�����l����
		ASIOSampleType sampleType;	// �T���v�����O���@�̎��

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
}