#pragma once
#include <vector>
#include <Windows.h>
#include "SamplePack.hpp"
#include "Option.hpp"

namespace asio
{
	/**
	* �ǂ������킯���m���^��������ꂽ�ꍇ�̗�O
	*/
	class UnrecognizedTypeException : public std::exception
	{
	public:
		UnrecognizedTypeException(const std::string& message)
			: std::exception(message.c_str()) {}
	};

	/**
	* Option.hpp�Ŏw�肳�ꂽ�^���ς������Ƃ��ɌĂяo�����
	*/
	class UnknownOptionType : public std::exception
	{
	public:
		UnknownOptionType(const std::string& message)
			: exception(message.c_str()) { }
	};


	


	/**
	* �o�b�t�@�����O���邽�߂̃X�g���[���N���X
	* @note �q�N���X�͊�{�I��BufferController���猩���Ȃ��Ȃ��Ă���̂ŁC����ȂɌ��J�E����J�͋C�ɂ��Ȃ��Ă������Ǝv��
	*/
	class StreamBuffer
	{
	protected:
		std::vector<int> stream;

		HANDLE hMutex;
		const std::wstring mutexName;
		Sample sample;

	protected:
		// �X���b�h�Z�[�t�ȏ�����������
		template <typename FUNC>
		void Mutex(FUNC func)
		{
			auto mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexName.c_str());
			WaitForSingleObject(mutex, INFINITE);
			func();
			ReleaseMutex(mutex);
			CloseHandle(mutex);
		}

	public:
		StreamBuffer(const Sample& samplePack, const std::wstring& mutexName)
			: sample(samplePack), mutexName(mutexName) 
		{
			hMutex = CreateMutexW(NULL, FALSE, mutexName.c_str());
		}

		virtual ~StreamBuffer()
		{
			CloseHandle(hMutex);
		}
	};



	/**
	* �f�o�C�X����z�X�g�֗������߂̃X�g���[���N���X
	*/
	class DeviceToHostStream : public StreamBuffer
	{
	private:
		/**
		* �o�b�t�@�ɒǉ�����
		*/
		void StoreBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Int:
			{
				const int* ptr = reinterpret_cast<int*>(buffer);
				Mutex([&]() { stream.insert(stream.end(), ptr, &ptr[size]); });
				break;
			}

			default:
				throw NotImplementSampleType("�T�|�[�g���Ă��Ȃ��ʎq���r�b�g���ł�");
			}
		}

	public:
		DeviceToHostStream(const Sample sample, const long channelNumber)
			: StreamBuffer(sample, std::wstring(L"TINY_ASIO_INPUT_") + std::to_wstring(channelNumber))
		{
			
		}

		/**
		* �o�b�t�@�̒��g���X�g���[���֒~�ς���
		*/
		void Store(void* buffer, const long size)
		{
			StoreBuffer(buffer, size);
		}

		std::shared_ptr<std::vector<int>> CopyAsClear()
		{
			const size_t halfSize = stream.size() >> 1;

			auto retVal = std::shared_ptr<std::vector<int>>(new std::vector<int>());

			Mutex([&]() { 
				retVal->insert(retVal->end(), stream.begin(), stream.end());
				stream.clear(); 
			});
			return retVal;
		}
	};



	/**
	* �z�X�g����f�o�C�X�ɑ��邽�߂̃X�g���[���N���X
	*/
	class HostToDeviceStream : public StreamBuffer
	{
		void FetchBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Int:
			{
				if (stream.size() >= size)					// stream�̒������\���ȂƂ�
					memcpy(buffer, stream.begin()._Ptr, size * sizeof(int));
				else if (stream.size() > 0)					// count�����̏ꍇ
					memcpy(buffer, stream.begin()._Ptr, stream.size() * sizeof(int));
				break;
			}

			default:
				throw NotImplementSampleType("�T�|�[�g���Ă��Ȃ��ʎq���r�b�g���ł�");
			}
		}

		void RemoveFrontFromSize(const long bufferSize)
		{
			// �擪����bufferSize������������
			unsigned long count = bufferSize;
			if (count > stream.size())
				count = stream.size();

			Mutex([&]() { stream.erase(stream.begin(), stream.begin() + count); });
		}

	public:
		HostToDeviceStream(Sample& sample, const long channelNumber)
			: StreamBuffer(sample, std::wstring(L"TINY_ASIO_INPUT_") + std::to_wstring(channelNumber)) { }

		/**
		* �X�g���[���̒��g���o�b�t�@�֏�������
		*/
		void Fetch(void* buffer, const long size)
		{
			memset(buffer, 0, size * sizeof(int));	// �ŏ��Ƀ[������

			FetchBuffer(buffer, size);

			RemoveFrontFromSize(size);
		}

		/**
		* �X�g���[���̍Ō�ɁC�w�肵���z���ǉ�����
		*/
		void InsertLast(const std::vector<int>& storeBuffer)
		{
			Mutex([&]() {stream.insert(stream.end(), storeBuffer.begin(), storeBuffer.end()); });
		}
	};
}