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
			CreateBuffer({inputChannel, outputChannel}, &BufferSwitch);

			input = &bufferManager->Inputs(0);
			output = &bufferManager->Outputs(0);
		}

		/**
		* 0�Ԃ̓��o�̓`�����l������R���g���[���𐶐�����
		* @note 0�Ԃ̓��o�͓��m���Ȃ��̂ŁC�K���ɉ��̏o��`�����l���ɃW���b�N��}���Ă�������
		*/
		InputBackController()
			: ControllerBase()
		{
			CreateBuffer({channelManager->Inputs(0), channelManager->Outputs(0)}, &BufferSwitch);

			input = &bufferManager->Inputs(0);
			output = &bufferManager->Outputs(0);
		}

		/**
		* �`�����l���ԍ�����R���g���[���𐶐�����
		*/
		InputBackController(const long inputNum, const long outputNum)
		{
			CreateBuffer({ channelManager->Inputs(inputNum), channelManager->Outputs(outputNum) }, &BufferSwitch);

			input = &bufferManager->Inputs(0);
			output = &bufferManager->Outputs(0);
		}

		/**
		* ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^���擾����
		* @return ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^
		* @note ���̓X�g���[���̓��e�͋�ɂȂ�
		*/
		StreamingVector Fetch()
		{
			return input->Fetch();
		}
	};

	InputBuffer* InputBackController::input = nullptr;
	OutputBuffer* InputBackController::output = nullptr;
}