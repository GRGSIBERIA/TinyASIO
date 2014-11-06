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

		Sample sample;

	protected:
		// �X���b�h�Z�[�t�ȏ�����������
		template <typename FUNC>
		void Mutex(FUNC func)
		{
			auto mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX);
			WaitForSingleObject(mutex, INFINITE);
			func();
			ReleaseMutex(mutex);
			CloseHandle(mutex);
		}

	public:
		StreamBuffer(Sample& samplePack)
			: sample(samplePack) {}
	};



	/**
	* �f�o�C�X����z�X�g�֗������߂̃X�g���[���N���X
	*/
	class DeviceToHostStream : public StreamBuffer
	{
		/**
		* �o�b�t�@�ɒǉ�����
		*/
		void StoreBuffer(void* buffer, const long size)
		{
			switch (sample.type)
			{
			case Int:
			{
				const size_t count = size / sizeof(int);
				const int* ptr = reinterpret_cast<int*>(buffer);
				Mutex([&]() { stream.insert(stream.end(), ptr, &ptr[count]); });
				break;
			}

			default:
				throw NotImplementSampleType("�T�|�[�g���Ă��Ȃ��ʎq���r�b�g���ł�");
			}
		}

	public:
		DeviceToHostStream(Sample sample)
			: StreamBuffer(sample) { }

		/**
		* �o�b�t�@�̒��g���X�g���[���֒~�ς���
		*/
		void Store(void* buffer, const long size)
		{
			if (sample.isMSB)
				throw NotImplementSampleType("�r�b�O�G���f�B�A���̓T�|�[�g���Ă��܂���");
			StoreBuffer(buffer, size);
		}

		std::shared_ptr<std::vector<int>> CopyAsClear()
		{
			auto retVal = std::shared_ptr<std::vector<int>>(new std::vector<int>(stream));
			Mutex([&]() { stream.clear(); });
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
				const size_t count = size / sizeof(int);
				if (stream.size() >= count)					// stream�̒������\���ȂƂ�
					memcpy(buffer, stream.begin()._Ptr, count * sizeof(int));
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
			unsigned long count;
			switch (sample.type)
			{
			case Int:
				count = bufferSize / sizeof(int);
				break;

			default:
				throw UnrecognizedTypeException("���p�s�\�ȗʎq���r�b�g�����w�肳��Ă��܂�");
			}

			if (count > stream.size())
				count = stream.size();

			Mutex([&]() { stream.erase(stream.begin(), stream.begin() + count); });
		}

	public:
		HostToDeviceStream(Sample& sample)
			: StreamBuffer(sample) { }

		/**
		* �X�g���[���̒��g���o�b�t�@�֏�������
		*/
		void Fetch(void* buffer, const long size)
		{
			memset(buffer, 0, size);	// �ŏ��Ƀ[������

			FetchBuffer(buffer, size);
			if (sample.isMSB)			// ��ԍŌ�ɃG���f�B�A�����t�]������
				throw NotImplementSampleType("�r�b�O�G���f�B�A���̓T�|�[�g���Ă��܂���");

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