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
	private:
		static void BufferSwitch(long index, long)
		{
			void* inBuf = GetInputMemory(0, index);

			Input(0).Store(inBuf, bufferLength);	// ���̓X�g���[���ɓ��e��~�ς���
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
		}

		/**
		* 0�Ԃ̓��o�̓`�����l������R���g���[���𐶐�����
		* @note 0�Ԃ̓��o�͓��m���Ȃ��̂ŁC�K���ɉ��̏o��`�����l���ɃW���b�N��}���Ă�������
		*/
		InputOnlyController(const std::string& asioDriverName)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({ channelManager->Inputs(0) }, &BufferSwitch);
		}

		/**
		* �`�����l���ԍ�����R���g���[���𐶐�����
		*/
		InputOnlyController(const std::string& asioDriverName, const long inputNum)
			: ControllerBase(asioDriverName)
		{
			CreateBuffer({ channelManager->Inputs(inputNum) }, &BufferSwitch);
		}

		/**
		* ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^���擾����
		* @return ���̓X�g���[���ɒ~�ς��ꂽ�f�[�^
		* @note ���̓X�g���[���̓��e�͋�ɂȂ�
		*/
		StreamPtr Fetch()
		{
			return Input(0).Fetch();
		}

		/**
		* �X�g���[���̌��݂̒����𓾂�
		*/
		const long StreamLength() const { return Input(0).StreamLength(); }
	};
}