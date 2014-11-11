#pragma once
#include <string>

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

		long numberOfChannels;
		long numberOfInput;			//!< 入力チャンネル数
		long numberOfOutput;		//!< 出力チャンネル数
		ASIOChannelInfo* infoPtr;	//!< チャンネル情報の先頭ポインタ

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

		const std::vector<InputChannel>& Inputs() const { return inputs; }		//!< 入力チャンネルを返す
		const std::vector<OutputChannel>& Outputs() const { return outputs; }	//!< 出力チャンネルを返す
		const long NumberOfInputs() const { return numberOfInput; }				//!< 入力チャンネル数
		const long NumberOfOutputs() const { return numberOfOutput; }			//!< 出力チャンネル数
		const long NumberOfChannels() const { return numberOfChannels; }		//!< チャンネル数
	};
}