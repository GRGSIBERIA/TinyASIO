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

			input->Store(inBuf, bufferLength);	// 入力ストリームに内容を蓄積する
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
			CreateBuffer({inputChannel, outputChannel}, &BufferSwitch);

			input = &bufferManager->InputBuffers(0);
			output = &bufferManager->OutputBuffers(0);
		}

		/**
		* バッファリング可能なチャンネルからコントローラを生成する
		* @note 0番の入出力同士をつなぐので，適当に音の出るチャンネルにジャックを挿してください
		*/
		InputBackController()
			: ControllerBase()
		{
			CreateBuffer({channelManager->Inputs(0), channelManager->Outputs(0)}, &BufferSwitch);

			input = &bufferManager->InputBuffers(0);
			output = &bufferManager->OutputBuffers(0);
		}

		/**
		* 入力ストリームに蓄積されたデータを取得する
		* @return 入力ストリームに蓄積されたデータ
		* @note 入力ストリームの内容は空になる
		*/
		StreamingVector Fetch()
		{
			return input->Fetch();
		}
	};

	InputBuffer* InputBackController::input = nullptr;
	OutputBuffer* InputBackController::output = nullptr;
}