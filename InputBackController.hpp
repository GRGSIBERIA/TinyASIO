#pragma once

#include "ControllerBase.hpp"

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