#pragma once
#include "ControllerBase.hpp"

namespace asio
{
	/*
	* エフェクター付きのコントローラ
	* 入力バッファとバッファ長を受け取るラムダ式をコールバック関数内で実行する
	* @tparam EFFECT_FUNC void (*)(void*, long)なラムダ式
	*/
	template <typename EFFECT_FUNC>
	class EffectableInputBackController : public ControllerBase
	{
		static InputBuffer* input;
		static OutputBuffer* output;
		static EFFECT_FUNC effector;

	private:
		static void BufferSwitch(long index, long directProcess)
		{
			void* outBuf = output->GetBuffer(index);
			void* inBuf = input->GetBuffer(index);

			effector(inBuf, bufferLength);	// 任意のラムダ式を実行する

			memcpy(outBuf, inBuf, bufferLength * sizeof(int));	// 入力のバッファを出力へ移す

			input->Store(inBuf, bufferLength);	// 入力ストリームに内容を蓄積する
		}

	public:
		/**
		* 指定したチャンネルからコントローラを生成する
		* @params[in] inputChannel 入力を受け付けるチャンネル
		* @params[in] outputChannel 入力された内容を流し込みたい出力チャンネル
		* @params[in] effectorFunction void (*)(void*, long)なラムダ式, void*は入力バッファ, longはバッファのサンプル数
		*/
		EffectableInputBackController(const InputChannel& inputChannel, const OutputChannel& outputChannel, EFFECT_FUNC effectorFunction)
			: ControllerBase()
		{
			input = &bufferManager->Search(inputChannel);
			output = &bufferManager->Search(outputChannel);
			auto callbacks = CreateCallbacks(&BufferSwitch, NULL, NULL, NULL);
			CreateBuffer(&callbacks);
			effector = effectorFunction;
		}

		/**
		* バッファリング可能なチャンネルからコントローラを生成する
		*/
		EffectableInputBackController(EFFECT_FUNC effectorFunction)
			: ControllerBase()
		{
			input = &bufferManager->SearchBufferableInput();
			output = &bufferManager->SearchBufferableOutput();
			auto callbacks = CreateCallbacks(&BufferSwitch, NULL, NULL, NULL);
			CreateBuffer(&callbacks);
			effector = effectorFunction;
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

	template <typename EFFECT_FUNC>
	InputBuffer* EffectableInputBackController<EFFECT_FUNC>::input = nullptr;

	template <typename EFFECT_FUNC>
	OutputBuffer* EffectableInputBackController<EFFECT_FUNC>::output = nullptr;

	template <typename EFFECT_FUNC>
	EFFECT_FUNC EffectableInputBackController<EFFECT_FUNC>::effector = nullptr;
}