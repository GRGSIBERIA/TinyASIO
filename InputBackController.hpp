#pragma once

#include "ControllerBase.hpp"
#include "Channel.hpp"

namespace asio
{
	/* ���͐M�������̂܂ܕԂ� */
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