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

			input->Store(inBuf, bufferLength);	// ���̓X�g���[���ɓ��e��~�ς���
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
			callbacks = CreateCallbacks(&BufferSwitch);
			CreateBuffer({inputChannel, outputChannel}, &callbacks);

			input = &bufferManager->SearchBufferableInput();
			output = &bufferManager->SearchBufferableOutput();
		}

		/**
		* �o�b�t�@�����O�\�ȃ`�����l������R���g���[���𐶐�����
		* @note 0�Ԃ̓��o�͓��m���Ȃ��̂ŁC�K���ɉ��̏o��`�����l���ɃW���b�N��}�����蔲�����肵�Ă�������
		*/
		InputBackController()
			: ControllerBase()
		{
			callbacks = CreateCallbacks(&BufferSwitch);
			auto& channelMng = Driver::Get().ChannelManager();

			CreateBuffer({channelMng.Inputs(0), channelMng.Outputs(0)}, &callbacks);

			input = &bufferManager->SearchBufferableInput();
			output = &bufferManager->SearchBufferableOutput();
		}

		/**
		* ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^���擾����
		* @return ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^
		* @note ���̓X�g���[���̓��e�͋�ɂȂ�
		*/
		std::shared_ptr<std::vector<int>> Fetch()
		{
			return input->Fetch();
		}
	};

	InputBuffer* InputBackController::input = nullptr;
	OutputBuffer* InputBackController::output = nullptr;
}