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
	* デバイスのチャンネル情報
	*/
	struct Channel
	{
		long number;			// チャンネル番号
		IOType ioType;			// 入出力のどちらか
		bool isActive;			// チャンネルが有効か無効か
		long group;				// チャンネルが所属しているグループ
		std::string name;		// チャンネル名
		ASIOSampleType sampleType;	// サンプリング方法の種類

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
	* 入出力の遅延を表す構造体
	*/
	struct IOLatency
	{
		long input, output;
	};

	/**
	* チャンネル数を表す構造体
	*/
	struct IOChannels
	{
		long input, output;
	};

	/**
	* バッファの現在の設定を表す構造体
	*/
	struct BufferPreference
	{
		long maxSize;		// バッファサイズの最大値
		long minSize;		// バッファサイズの最小値
		long preferredSize;	// 設定中の値
		long granularity;	// 設定の粒度
	};
}