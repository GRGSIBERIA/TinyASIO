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
			// Buffer�z��̃|�C���^���Q�Ƃ��Ă��邾���Ȃ̂ŁC���x���������\
			static std::vector<InputBuffer>* inputBuffer;
			static std::vector<OutputBuffer>* outputBuffer;

		private:
			
			static void BufferingLoop(const long doubleBufferIndex, const ASIOBool directProcess)
			{
				for (auto& input : *inputBuffer)
				{
					input.StoreC(doubleBufferIndex);
				}
				for (auto& output : *outputBuffer)
				{
					output.FetchC(doubleBufferIndex);
				}
			}

			static void BufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
			{
				BufferingLoop(doubleBufferIndex, directProcess);
			}

			static void SampleRateDidChange(ASIOSampleRate sRate)
			{
				// ���삵��
			}

			static long AsioMessage(long selector, long value, void* message, double* opt)
			{
				// ���������
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

			void Init(std::vector<InputBuffer>* inb, std::vector<OutputBuffer>* outb)
			{
				inputBuffer = inb;
				outputBuffer = outb;
			}
		};

		std::vector<InputBuffer>* CallbackManager::inputBuffer = nullptr;
		std::vector<OutputBuffer>* CallbackManager::outputBuffer = nullptr;
	}
}