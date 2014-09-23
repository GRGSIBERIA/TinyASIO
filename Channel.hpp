#pragma once
#include <string>
#include "Interface.hpp"

namespace asio
{
	struct Channel
	{
		long number;			// �`�����l���ԍ�
		IOType ioType;			// ���o�͂̂ǂ��炩
		bool isActive;			// �`�����l�����L����������
		long group;				// �`�����l�����������Ă���O���[�v
		std::string name;		// �`�����l����
		SampleType sampleType;	// �T���v�����O���@�̎��

	public:
		Channel(const ASIOChannelInfo& info)
		{
			number = info.channel;
			ioType = info.isInput ? IOType::Input : IOType::Output;
			isActive = info.isActive;
			group = info.channelGroup;
			name = info.name;
			sampleType = (SampleType)info.type;
		}
	};
}