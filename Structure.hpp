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
	* バッファの現在の設定を表す構造体
	*/
	struct BufferPreference
	{
		long maxSize;		// バッファサイズの最大値
		long minSize;		// バッファサイズの最小値
		long preferredSize;	// 最適な値
		long granularity;	// 設定の粒度
	};
}