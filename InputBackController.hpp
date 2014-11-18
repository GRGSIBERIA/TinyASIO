#pragma once

#include "ControllerBase.hpp"

namespace asio
{
	/*
	 *入力信号を出力にそのまま返す 
	 */
	class InputBackController : public ControllerBase
	{
		static InputBuffer* input;
		static OutputBuffer* output;

	private:
		static void BufferSwitch(long index, long directProcess)
		{
			void* outBuf = output->GetBuffer(index);
			void* inBuf = input->GetBuffer(index);

			memcpy(outBuf, inBuf, bufferLength * sizeof(int));	// 入力のバッファを出力へ移す

			input->Store(inBuf, bufferLength);
		}

		ASIOCallbacks CreateCallbacks()
		{
			auto callbacks = ASIOCallbacks();
			callbacks.asioMessage = NULL;
			callbacks.bufferSwitch = &BufferSwitch;
			callbacks.bufferSwitchTimeInfo = NULL;
			callbacks.sampleRateDidChange = NULL;
			return callbacks;
		}

	public:
		/**
		* 指定したチャンネルからコントローラを生成する
		* @params[in] inputChannel 入力を受け付けるチャンネル
		* @params[in] outputChannel 入力された内容を流し込みたい出力チャンネル
		*/
		InputBackController(const InputChannel& inputChannel, const OutputChannel& outputChannel)
			: ControllerBase() 
		{
			input = &bufferManager->Search(inputChannel);
			output = &bufferManager->Search(outputChannel);
			auto callbacks = CreateCallbacks();
			CreateBuffer(&callbacks);
		}

		/**
		* バッファリング可能なチャンネルからコントローラを生成する
		*/
		InputBackController()
			: ControllerBase()
		{
			input = &bufferManager->SearchBufferableInput();
			output = &bufferManager->SearchBufferableOutput();
			auto callbacks = CreateCallbacks();
			CreateBuffer(&callbacks);
		}

		
	};

	InputBuffer* InputBackController::input = nullptr;
	OutputBuffer* InputBackController::output = nullptr;
}