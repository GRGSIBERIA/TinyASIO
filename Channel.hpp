#pragma once
#include <vector>
#include "Interface.hpp"
#include "Structure.hpp"


namespace asio
{
	/**
	* デバイスのチャンネル情報
	*/
	struct Channel
	{
		long number;				// チャンネル番号
		IOType ioType;				// 入出力のどちらか
		ASIOBool isActive;				// チャンネルが有効か無効か
		long group;					// チャンネルが所属しているグループ
		std::string name;			// チャンネル名
		ASIOSampleType sampleType;	// サンプリングの種類

	public:
		/**
		* @params[in] info デバイスから得たチャンネル情報
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
	* デバイスで利用可能なチャンネルの管理クラス
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
		* 入力のチャンネル数を得る
		*/
		const long NumberOfInputChannels() const { return numInputChannels; }

		/**
		* 出力のチャンネル数を得る
		*/
		const long NumberOfOutputChannels() const { return numOutputChannels; }

		/**
		* 入力チャンネルの配列を返す
		*/
		const std::vector<Channel>& Inputs() const { return inputChannels; }

		/**
		* 出力チャンネルの配列を返す
		*/
		const std::vector<Channel>& Outputs() const { return outputChannels; }

		/**
		* 添字から入力チャンネルを返す
		*/
		const Channel& Inputs(const long i) const { return inputChannels[i]; }

		/**
		* 添字から出力チャンネルを返す
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