#pragma once

#include "ControllerBase.hpp"
#include "Channel.hpp"

namespace asio
{
	/* “ü—ÍM†‚ğ‚»‚Ì‚Ü‚Ü•Ô‚· */
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