#pragma once
#include <vector>
#include <memory>
#include <algorithm>

#include "Interface.hpp"
#include "Structure.hpp"
#include "Driver.hpp"
#include "Channel.hpp"
#include "SamplePack.hpp"
#include "BufferList.hpp"

namespace asio
{
	class BufferController;

	/**
	* �o�b�t�@�N���X
	*/
	class Buffer
	{
		friend BufferController;

		IOType ioType;
		long channelNumber;
		long bufferSize;
		const ASIOSampleType sampleType;
		void* bufferData[2];

		BufferList bufferList;

	public:
		/**
		* IO�̎�ނ�Ԃ�
		*/
		inline const IOType Type() const { return ioType; }

	public:
		Buffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			:
			ioType((IOType)info.isInput),
			channelNumber(info.channelNum),
			bufferSize(bufferSize),
			sampleType(sampleType),
			bufferList(pack::DetectSampleTypePackStruct(sampleType))
		{
			bufferData[0] = info.buffers[0];
			bufferData[1] = info.buffers[1];
		}

		void Store(const long index, const long size)
		{
			bufferList.Store(bufferData[index], size);
		}
	};

	/**
	* ���̓o�b�t�@�Ǘ��N���X
	*/
	class InputBuffer : public Buffer
	{
	public:
		InputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize, sampleType) {}
	};

	/**
	* �o�̓o�b�t�@�Ǘ��N���X
	*/
	class OutputBuffer : public Buffer
	{
	public:
		OutputBuffer(const ASIOBufferInfo& info, const long bufferSize, const ASIOSampleType sampleType)
			: Buffer(info, bufferSize, sampleType) {}
	};

	class BufferManager;	// �t�����h�ɂ��邽�߂̑O���錾

	/**
	* �R�[���o�b�N�֐��ƃo�b�t�@�����O���܂Ƃ߂邽�߂̃N���X
	*/
	class BufferController
	{
		friend BufferManager;

		static std::vector<Buffer*> buffers;

		std::vector<InputBuffer> inputBuffers;
		std::vector<OutputBuffer> outputBuffers;

		IASIO* iasio;

	private:
		BufferController(IASIO* iasio)
			: iasio(iasio) {}

	private:
		void Add(const ASIOBufferInfo& info, const long& bufferSize, const ASIOSampleType& sampleType)
		{
			Buffer* ptr = nullptr;

			if (info.isInput)
			{
				inputBuffers.emplace_back(info, bufferSize, sampleType);
				ptr = &inputBuffers[inputBuffers.size() - 1];
			}
			else
			{
				outputBuffers.emplace_back(info, bufferSize, sampleType);
				ptr = &outputBuffers[outputBuffers.size() - 1];
			}
			buffers.push_back(ptr);
		}

		void Clear()
		{
			buffers.clear();
			inputBuffers.clear();
			outputBuffers.clear();
		}

		static ASIOCallbacks CreateCallbacks()
		{
			ASIOCallbacks callback;
			callback.bufferSwitch = &BufferController::BufferSwitch;
			callback.sampleRateDidChange = &BufferController::SampleRateDidChange;
			callback.asioMessage = &BufferController::AsioMessage;
			callback.bufferSwitchTimeInfo = &BufferController::BufferSwitchTimeInfo;
			return callback;
		}

	public:

		/**
		* �o�b�t�@�����O�J�n
		*/
		inline void Start() const
		{
			ErrorCheck(iasio->start());
		}

		/**
		* �o�b�t�@�����O��~
		*/
		inline void Stop() const
		{
			ErrorCheck(iasio->stop());
		}

		/**
		* ���̓o�b�t�@�̔z����擾����
		* @return ���̓o�b�t�@�̔z��
		*/
		inline const std::vector<InputBuffer> InputBuffers() const { return inputBuffers; }

		/**
		* �o�̓o�b�t�@�̔z����擾����
		* @return �o�̓o�b�t�@�̔z��
		*/
		inline const std::vector<OutputBuffer> OutputBuffers() const { return outputBuffers; }
	};

	std::vector<Buffer*> BufferController::buffers;

	namespace callback
	{
		/**
		* �R�[���o�b�N�֐��𐧌䂷�邽�߂̃N���X
		*/
		class CallbackManager
		{
			static std::vector<Buffer*>* buffers;

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
		};

		std::vector<Buffer*>* buffers;
	}

	/**
	* �o�b�t�@���Ǘ�����N���X
	*/
	class BufferManager
	{
		IASIO* iasio;

		BufferController bufferController;
		callback::CallbackManager callbackManager;
		std::vector<ASIOBufferInfo> bufferInfos;

	private:
		void InitBuffers(const long& bufferSize, const ASIOSampleType& sampleType, ASIOCallbacks* callbacks)
		{
			bufferController.Clear();
			for (unsigned i = 0; i < bufferInfos.size(); ++i)
			{
				const auto& info = bufferInfos[i];
				bufferController.Add(info, bufferSize, sampleType);
			}
		}

	public:
		BufferManager(IASIO* iasio)
			: iasio(iasio), bufferController(iasio)
		{

		}

		~BufferManager()
		{
			if (bufferInfos.size() > 0)
				ErrorCheck(iasio->disposeBuffers());
		}

		/**
		* �o�b�t�@�����O�������`�����l����ǉ�
		*/
		inline void AddChannel(const IOType& ioType, const long& channelNumber)
		{
			ASIOBufferInfo info;
			info.channelNum = channelNumber;
			info.isInput = ioType;
			bufferInfos.push_back(info);
		}

		/**
		* �o�b�t�@�����O�������`�����l����ǉ�
		*/
		inline void AddChannel(const Channel& channel)
		{
			AddChannel(channel.ioType, channel.ioType);
		}

		/**
		* �o�b�t�@�����O�������`�����l��������ςȂ��ɂ���
		*/
		inline void ClearChannel()
		{
			bufferInfos.clear();
		}

		/**
		* �o�b�t�@�����O����`�����l����Ԃ�
		*/
		const std::vector<ASIOBufferInfo>& BufferingChannels() const { return bufferInfos; }

		/**
		* �o�b�t�@�̐���
		* @params[in] bufferSize �o�b�t�@�̐ݒ�
		* @params[in, out] callbacks �o�b�t�@�����O���̒ʒm�̂��߂ɗ��p
		* @note bufferSize�͎��R�ɐ��l�����߂��Ȃ��̂Œ���, (bufferSize % granularity == 0)�ȊO�̐��l�͕ۏ�ł��Ȃ�
		*/
		const BufferController& CreateBuffer(const long& bufferSize, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			ErrorCheck(iasio->createBuffers(&bufferInfos[0], bufferInfos.size(), bufferSize, callbacks));
			InitBuffers(bufferSize, sampleType, callbacks);
			return bufferController;
		}

		/**
		* �o�b�t�@�̐���
		* @params[in] bufferPreference �o�b�t�@�̐ݒ�
		* @params[in, out] callbacks �o�b�t�@�����O���̒ʒm�̂��߂ɗ��p
		* @note bufferSize�͎��R�ɐ��l�����߂��Ȃ��̂Œ���, (bufferSize % granularity == 0)�ȊO�̐��l�͕ۏ�ł��Ȃ�
		*/
		const BufferController& CreateBuffer(const BufferPreference& bufferPreference, const ASIOSampleType sampleType, ASIOCallbacks* callbacks)
		{
			CreateBuffer(bufferPreference.preferredSize, sampleType, callbacks);
			return bufferController;
		}
	};
}