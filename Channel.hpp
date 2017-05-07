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
	* チャンネル情報
	*/
	class Channel
	{
	public:
		const bool isActive;				//!< チャンネルが有効かどうか
		const std::string name;				//!< チャンネル名
		const long channelNumber;			//!< チャンネル番号
		const ASIOSampleType sampleType;	//!< サンプリングの種類
		const long channelGroup;			//!< チャンネルのグループ
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
	* 入力チャンネル
	*/
	class InputChannel : public Channel
	{
	public:
		InputChannel(const ASIOChannelInfo& info)
			: Channel(info) {}
	};


	/**
	* 出力チャンネル
	*/
	class OutputChannel : public Channel
	{
	public:
		OutputChannel(const ASIOChannelInfo& info)
			: Channel(info) {}
	};


	/**
	* 各種チャンネルの管理クラス
	*/
	class ChannelManager
	{
		std::vector<InputChannel> inputs;
		std::vector<OutputChannel> outputs;
		IASIO* iasio;

		long numberOfChannels;
		long numberOfInput;			//!< 入力チャンネル数
		long numberOfOutput;		//!< 出力チャンネル数
		std::shared_ptr<ASIOChannelInfo> inPtr;		//!< 入力チャンネル情報
		std::shared_ptr<ASIOChannelInfo> outPtr;	//!< 出力チャンネル情報


	private:
		//!< あるチャンネルの初期化
		void InitOneChannel(ASIOChannelInfo& info, const long i, const ASIOBool isInput)
		{
			// infoに与えられた初期値で取得するチャンネルを判断してる
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
				throw DontFoundChannels("入力チャンネルが見つかりません");
			if (numberOfOutput <= 0)
				throw DontFoundChannels("出力チャンネルが見つかりません");

			numberOfChannels = numberOfInput + numberOfOutput;
			inPtr = std::shared_ptr<ASIOChannelInfo>(new ASIOChannelInfo[numberOfInput]);
			outPtr = std::shared_ptr<ASIOChannelInfo>(new ASIOChannelInfo[numberOfOutput]);

			for (long i = 0; i < numberOfInput; ++i)
				InitOneChannel(inPtr.get()[i], i, 1);

			for (long i = 0; i < numberOfOutput; ++i)
				InitOneChannel(outPtr.get()[i], i, 0);
		}

		~ChannelManager() { }

		const std::vector<InputChannel>& Inputs() const { return inputs; }		//!< 入力チャンネルを返す
		const std::vector<OutputChannel>& Outputs() const { return outputs; }	//!< 出力チャンネルを返す
		const InputChannel& Inputs(const long i) const { return inputs[i]; }	//!< 添字iに対応した入力チャンネルを返す
		const OutputChannel& Outputs(const long i) const { return outputs[i]; }	//!< 添字iに対応して出力チャンネルを返す
		const long NumberOfInputs() const { return numberOfInput; }				//!< 入力チャンネル数
		const long NumberOfOutputs() const { return numberOfOutput; }			//!< 出力チャンネル数
		const long NumberOfChannels() const { return numberOfChannels; }		//!< チャンネル数
	};
}