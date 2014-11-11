#pragma once
#include <string>

#include "Exception.hpp"
#include "SDK.hpp"
#include "Driver.hpp"

namespace asio
{
	/**
	* �`�����l�����
	*/
	class Channel
	{
	public:
		const bool isActive;				//!< �`�����l�����L�����ǂ���
		const std::string name;				//!< �`�����l����
		const long channelNumber;			//!< �`�����l���ԍ�
		const ASIOSampleType sampleType;	//!< �T���v�����O�̎��
		const long channelGroup;			//!< �`�����l���̃O���[�v

	public:
		Channel(const ASIOChannelInfo& i) 
			: 
			isActive(i.isActive),
			name(i.name),
			channelNumber(i.channel),
			sampleType(i.type),
			channelGroup(i.channelGroup)
		{ 
			
		}
	};


	/**
	* ���̓`�����l��
	*/
	class InputChannel : public Channel
	{
	public:
		InputChannel(const ASIOChannelInfo& info)
			: Channel(info) {}
	};


	/**
	* �o�̓`�����l��
	*/
	class OutputChannel : public Channel
	{
	public:
		OutputChannel(const ASIOChannelInfo& info)
			: Channel(info) {}
	};


	/**
	* �e��`�����l���̊Ǘ��N���X
	*/
	class ChannelManager
	{
		std::vector<InputChannel> inputs;
		std::vector<OutputChannel> outputs;

		long numberOfChannels;
		long numberOfInput;			//!< ���̓`�����l����
		long numberOfOutput;		//!< �o�̓`�����l����
		ASIOChannelInfo* infoPtr;	//!< �`�����l�����̐擪�|�C���^

	public:
		ChannelManager()
		{
			auto* iasio = Driver::Get().Interface();
			
			ErrorCheck(iasio->getChannels(&numberOfInput, &numberOfOutput));
			ErrorCheck(iasio->getChannelInfo(infoPtr));
			
			numberOfChannels = numberOfInput + numberOfOutput;
			for (size_t i = 0; i < numberOfChannels; ++i)
			{
				const auto& info = infoPtr[i];
				if (info.isInput)
					inputs.emplace_back(info);
				else
					outputs.emplace_back(info);
			}
		}

		const std::vector<InputChannel>& Inputs() const { return inputs; }		//!< ���̓`�����l����Ԃ�
		const std::vector<OutputChannel>& Outputs() const { return outputs; }	//!< �o�̓`�����l����Ԃ�
		const long NumberOfInputs() const { return numberOfInput; }				//!< ���̓`�����l����
		const long NumberOfOutputs() const { return numberOfOutput; }			//!< �o�̓`�����l����
		const long NumberOfChannels() const { return numberOfChannels; }		//!< �`�����l����
	};
}