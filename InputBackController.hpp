#pragma once

#include "ControllerBase.hpp"

namespace asio
{
	/* 入力信号をそのまま返す */
	class InputBackController : public ControllerBase
	{
	private:
		static void BufferSwitch(long index, long directProcess)
		{
			
		}

	public:
		InputBackController()
			: ControllerBase() {}
	};
}