#pragma once
#include <vector>

namespace asio
{
	namespace callback
	{
		/**
		* �R�[���o�b�N�֐��𐧌䂷�邽�߂̃N���X
		* @note ���̃N���X��Buffer�N���X��friend����Ă���
		*/
		class CallbackManager
		{
			static std::vector<Buffer*>* buffers;	// Buffer�z��̃|�C���^���Q�Ƃ��Ă��邾���Ȃ̂ŁC���x���������\

		private:
			
			static void BufferingInputChannel(long doubleBufferIndex)
			{

			}

			static void BufferingOutputChannel(long doubleBufferIndex)
			{

			}

			static void BufferingLoop(const long doubleBufferIndex, const ASIOBool directProcess)
			{
				for (size_t i = 0; i < buffers->size(); ++i)
				{
					if (buffers->at(i)->Type() == IOType::Input)
						BufferingInputChannel(doubleBufferIndex);
					else
						BufferingOutputChannel(doubleBufferIndex);
				}
			}

			static void BufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
			{
				BufferingLoop(doubleBufferIndex, directProcess);
			}

			static void SampleRateDidChange(ASIOSampleRate sRate)
			{

			}

			static long AsioMessage(long selector, long value, void* message, double* opt)
			{
				return 0;
			}

			static ASIOTime* BufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)
			{
				BufferingLoop(doubleBufferIndex, directProcess);
				return params;
			}

		public:
			static ASIOCallbacks CreateCallbacks()
			{
				ASIOCallbacks callback;
				callback.bufferSwitch = &CallbackManager::BufferSwitch;
				callback.sampleRateDidChange = &CallbackManager::SampleRateDidChange;
				callback.asioMessage = &CallbackManager::AsioMessage;
				callback.bufferSwitchTimeInfo = &CallbackManager::BufferSwitchTimeInfo;
				return callback;
			}

			void Init(std::vector<Buffer*>* buf)
			{
				buffers = buf;
			}
		};

		std::vector<Buffer*>* buffers;
	}
}