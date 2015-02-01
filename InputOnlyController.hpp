/***********************************************************************
Copyright(C) 2014  Eiichi Takebuchi

TinyASIO is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

TinyASIO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TinyASIO.If not, see <http://www.gnu.org/licenses/>
***********************************************************************/

#pragma once

#include "ControllerBase.hpp"

namespace asio
{
	/*
	*���͐M�����o�͂ɂ��̂܂ܕԂ�
	*/
	class InputOnlyController : public ControllerBase
	{
		static InputBuffer* input;

	private:
		static void BufferSwitch(long index, long directProcess)
		{
			void* inBuf = input->GetBuffer(index);

			input->Store(inBuf, bufferLength);	// ���̓X�g���[���ɓ��e��~�ς���
		}

	public:
		/**
		* �w�肵���`�����l������R���g���[���𐶐�����
		* @param[in] inputChannel ���͂��󂯕t����`�����l��
		*/
		InputOnlyController(const std::string& asioDriverName, const InputChannel& inputChannel)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({ inputChannel }, &BufferSwitch);

			input = &bufferManager->Inputs(0);
		}

		/**
		* 0�Ԃ̓��o�̓`�����l������R���g���[���𐶐�����
		* @note 0�Ԃ̓��o�͓��m���Ȃ��̂ŁC�K���ɉ��̏o��`�����l���ɃW���b�N��}���Ă�������
		*/
		InputOnlyController(const std::string& asioDriverName)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({ channelManager->Inputs(0) }, &BufferSwitch);

			input = &bufferManager->Inputs(0);
		}

		/**
		* �`�����l���ԍ�����R���g���[���𐶐�����
		*/
		InputOnlyController(const std::string& asioDriverName, const long inputNum)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({ channelManager->Inputs(inputNum) }, &BufferSwitch);

			input = &bufferManager->Inputs(0);
		}

		/**
		* ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^���擾����
		* @return ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^
		* @note ���̓X�g���[���̓��e�͋�ɂȂ�
		*/
		StreamPtr Fetch()
		{
			return input->Fetch();
		}

		/**
		* �X�g���[���̌��݂̒����𓾂�
		*/
		const long StreamLength() const { return input->StreamLength(); }
	};

	InputBuffer* InputOnlyController::input = nullptr;
}