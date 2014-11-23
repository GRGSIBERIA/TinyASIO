/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

#pragma once
#include "ControllerBase.hpp"

namespace asio
{
	/*
	* �G�t�F�N�^�[�t���̃R���g���[��
	* ���̓o�b�t�@�ƃo�b�t�@�����󂯎�郉���_�����R�[���o�b�N�֐����Ŏ��s����
	* �������ǂ����͂킩��Ȃ�
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
			callbacks = CreateCallbacks(&BufferSwitch);
			CreateBuffer({ inputChannel, outputChannel }, &callbacks);

			input = &bufferManager->SearchBufferableInput();
			output = &bufferManager->SearchBufferableOutput();
		}

		/**
		* �o�b�t�@�����O�\�ȃ`�����l������R���g���[���𐶐�����
		*/
		EffectableInputBackController(EFFECT_FUNC effectorFunction)
			: ControllerBase()
		{
			callbacks = CreateCallbacks(&BufferSwitch);
			auto& channelMng = Driver::Get().ChannelManager();

			CreateBuffer({ channelMng.Inputs(0), channelMng.Outputs(0) }, &callbacks);

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

	template <typename EFFECT_FUNC>
	InputBuffer* EffectableInputBackController<EFFECT_FUNC>::input = nullptr;

	template <typename EFFECT_FUNC>
	OutputBuffer* EffectableInputBackController<EFFECT_FUNC>::output = nullptr;

	template <typename EFFECT_FUNC>
	EFFECT_FUNC EffectableInputBackController<EFFECT_FUNC>::effector = nullptr;
}