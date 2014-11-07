#pragma once
#include <vector>

namespace asio
{
	namespace callback
	{
		/**
		* コールバック関数を制御するためのクラス
		* @note このクラスはBufferクラスにfriendされている
		*/
		class CallbackManager
		{
			// Buffer配列のポインタを参照しているだけなので，何度も初期化可能
			static std::vector<InputBuffer>* inputBuffer;
			static std::vector<OutputBuffer>* outputBuffer;

		private:
			
			static void BufferingLoop(const long doubleBufferIndex, const ASIOBool directProcess)
			{
				for (auto& input : *inputBuffer)
				{
					input.StoreC(doubleBufferIndex);
				}
				//for (auto& output : *outputBuffer)
				//{
				//	output.FetchC(doubleBufferIndex);
				//}

				const size_t count = inputBuffer->size() > outputBuffer->size() ? inputBuffer->size() : outputBuffer->size();
				for (size_t i = 0; i < count; ++i)
				{
					Buffer::DirectCopy(doubleBufferIndex, inputBuffer->at(i), outputBuffer->at(i), count);
					//void *source = inputBuffer->at(i).bufferData[doubleBufferIndex];
					//void *dest = outputBuffer->at(i).bufferData[doubleBufferIndex];
					//memcpy(dest, source, inputBuffer->at(i).bufferSize * sizeof(int));
				}
			}

			static void BufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
			{
				BufferingLoop(doubleBufferIndex, directProcess);
			}

			static void SampleRateDidChange(ASIOSampleRate sRate)
			{
				// 自作して
			}

			static long AsioMessage(long selector, long value, void* message, double* opt)
			{
				// これも自作
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