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
			input = &bufferManager->Search(inputChannel);
			output = &bufferManager->Search(outputChannel);
			auto callbacks = CreateCallbacks(&BufferSwitch, NULL, NULL, NULL);
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
			auto callbacks = CreateCallbacks(&BufferSwitch, NULL, NULL, NULL);
			CreateBuffer(&callbacks);
		}

		/**
		* 入力ストリームに蓄積されたデータを取得する
		* @return 入力ストリームに蓄積されたデータ
		* @note 入力ストリームの内容は空になる
		*/
		std::shared_ptr<std::vector<int>> Fetch()
		{
			return input->Fetch();
		}
	};

	InputBuffer* InputBackController::input = nullptr;
	OutputBuffer* InputBackController::output = nullptr;
}