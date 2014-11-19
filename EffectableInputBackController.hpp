#pragma once
#include "ControllerBase.hpp"

namespace asio
{
	/*
	* �G�t�F�N�^�[�t���̃R���g���[��
	* ���̓o�b�t�@�ƃo�b�t�@�����󂯎�郉���_�����R�[���o�b�N�֐����Ŏ��s����
	* @tparam EFFECT_FUNC void (*)(void*, long)�ȃ����_��
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

			effector(inBuf, bufferLength);	// �C�ӂ̃����_�������s����

			memcpy(outBuf, inBuf, bufferLength * sizeof(int));	// ���͂̃o�b�t�@���o�͂ֈڂ�

			input->Store(inBuf, bufferLength);	// ���̓X�g���[���ɓ��e��~�ς���
		}

	public:
		/**
		* �w�肵���`�����l������R���g���[���𐶐�����
		* @params[in] inputChannel ���͂��󂯕t����`�����l��
		* @params[in] outputChannel ���͂��ꂽ���e�𗬂����݂����o�̓`�����l��
		* @params[in] effectorFunction void (*)(void*, long)�ȃ����_��, void*�͓��̓o�b�t�@, long�̓o�b�t�@�̃T���v����
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
		* �o�b�t�@�����O�\�ȃ`�����l������R���g���[���𐶐�����
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
		* ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^���擾����
		* @return ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^
		* @note ���̓X�g���[���̓��e�͋�ɂȂ�
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