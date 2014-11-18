#pragma once

#include "ControllerBase.hpp"

namespace asio
{
	/*
	 *���͐M�����o�͂ɂ��̂܂ܕԂ� 
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

			memcpy(outBuf, inBuf, bufferLength * sizeof(int));	// ���͂̃o�b�t�@���o�͂ֈڂ�

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
		* �w�肵���`�����l������R���g���[���𐶐�����
		* @params[in] inputChannel ���͂��󂯕t����`�����l��
		* @params[in] outputChannel ���͂��ꂽ���e�𗬂����݂����o�̓`�����l��
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
		* �o�b�t�@�����O�\�ȃ`�����l������R���g���[���𐶐�����
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