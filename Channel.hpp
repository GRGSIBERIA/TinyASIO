#pragma once
#include <string>
#include "Interface.hpp"

namespace asio
{
	struct Channel
	{
		long number;			// チャンネル番号
		IOType ioType;			// 入出力のどちらか
		bool isActive;			// チャンネルが有効か無効か
		long group;				// チャンネルが所属しているグループ
		std::string name;		// チャンネル名
		SampleType sampleType;	// サンプリング方法の種類

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