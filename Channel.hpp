/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

TinyASIO is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

TinyASIO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TinyASIO.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

#pragma once
#include <string>

#include "Option.hpp"
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
		const bool isInput;

	public:
		Channel(const ASIOChannelInfo& i)
			:
			isActive(i.isActive > 0),
			name(i.name),
			channelNumber(i.channel),
			sampleType(ASIO_CURRENT_SAMPLE_TYPE), //sampleType(i.type),
			channelGroup(i.channelGroup),
			isInput(i.isInput > 0) { }

		Channel operator=(const Channel& channel)
		{
			return channel;
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
		IASIO* iasio;

		long numberOfChannels;
		long numberOfInput;			//!< ���̓`�����l����
		long numberOfOutput;		//!< �o�̓`�����l����
		std::shared_ptr<ASIOChannelInfo> inPtr;		//!< ���̓`�����l�����
		std::shared_ptr<ASIOChannelInfo> outPtr;	//!< �o�̓`�����l�����


	private:
		//!< ����`�����l���̏�����
		void InitOneChannel(ASIOChannelInfo& info, const long i, const ASIOBool isInput)
		{
			// info�ɗ^����ꂽ�����l�Ŏ擾����`�����l���𔻒f���Ă�
			info.isInput = isInput;
			info.channel = i;
			ErrorCheck(iasio->getChannelInfo(&info));
			info.type = 18;

			if (isInput == 1)
				inputs.emplace_back(info);
			else
				outputs.emplace_back(info);
		}

	public:
		ChannelManager(IASIO* iasio)
			: iasio(iasio)
		{
			ErrorCheck(iasio->getChannels(&numberOfInput, &numberOfOutput));
			
			if (numberOfInput <= 0)
				throw DontFoundChannels("���̓`�����l����������܂���");
			if (numberOfOutput <= 0)
				throw DontFoundChannels("�o�̓`�����l����������܂���");

			numberOfChannels = numberOfInput + numberOfOutput;
			inPtr = std::shared_ptr<ASIOChannelInfo>(new ASIOChannelInfo[numberOfInput]);
			outPtr = std::shared_ptr<ASIOChannelInfo>(new ASIOChannelInfo[numberOfOutput]);

			for (long i = 0; i < numberOfInput; ++i)
				InitOneChannel(inPtr.get()[i], i, 1);

			for (long i = 0; i < numberOfOutput; ++i)
				InitOneChannel(outPtr.get()[i], i, 0);
		}

		~ChannelManager() { }

		const std::vector<InputChannel>& Inputs() const { return inputs; }		//!< ���̓`�����l����Ԃ�
		const std::vector<OutputChannel>& Outputs() const { return outputs; }	//!< �o�̓`�����l����Ԃ�
		const InputChannel& Inputs(const long i) const { return inputs[i]; }	//!< �Y��i�ɑΉ��������̓`�����l����Ԃ�
		const OutputChannel& Outputs(const long i) const { return outputs[i]; }	//!< �Y��i�ɑΉ����ďo�̓`�����l����Ԃ�
		const long NumberOfInputs() const { return numberOfInput; }				//!< ���̓`�����l����
		const long NumberOfOutputs() const { return numberOfOutput; }			//!< �o�̓`�����l����
		const long NumberOfChannels() const { return numberOfChannels; }		//!< �`�����l����
	};
}