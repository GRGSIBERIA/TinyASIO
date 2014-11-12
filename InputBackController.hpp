#pragma once

#include "ControllerBase.hpp"
#include "Channel.hpp"

namespace asio
{
	/* “ü—ÍM†‚ð‚»‚Ì‚Ü‚Ü•Ô‚· */
	class InputBackController : public ControllerBase
	{
		static InputBuffer* input;
		static OutputBuffer* output;

	private:
		static void BufferSwitch(long index, long directProcess)
		{
			void* outBuf = output->GetBuffer(index);
			void* inBuf = input->GetBuffer(index);

			memcpy(outBuf, inBuf, bufferLength * sizeof(int));

			input->Store(inBuf, bufferLength);
		}

	public:
		InputBackController(const InputChannel& inputChannel, const OutputChannel& outputChannel)
			: ControllerBase() 
		{
			input = &bufferManager->Search(inputChannel);
			output = &bufferManager->Search(outputChannel);
		}

		InputBackController()
			: ControllerBase()
		{
			
		}
	};

	InputBuffer* InputBackController::input = nullptr;
	OutputBuffer* InputBackController::output = nullptr;
}