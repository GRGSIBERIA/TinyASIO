#pragma once

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
			static std::vector<Buffer*>* buffers;	// Buffer配列のポインタを参照しているだけなので，何度も初期化可能

		private:
			static void BufferingInputChannel()
			{

			}

			static void BufferingOutputChannel()
			{

			}

			static void BufferingLoop(long doubleBufferIndex, ASIOBool directProcess)
			{
				for (size_t i = 0; i < buffers->size(); ++i)
				{
					if (buffers->at(i)->Type() == IOType::Input)
						BufferingInputChannel();
					else
						BufferingOutputChannel();
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