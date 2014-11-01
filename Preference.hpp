#pragma once

#include "Interface.hpp"

namespace asio
{
	class BufferManager;

	/**
	* �ݒ�֌W�̃N���X
	*/
	class Preference
	{
		friend BufferManager;	// �݌v�~�X�����炵��

		IASIO* driver;

		long inputLatency;
		long outputLatency;
		double sampleRate;

	public:
		/**
		* ���͂̒x���𓾂�
		*/
		const long& InputLatency() const { return inputLatency; }

		/**
		* �o�͂̒x���𓾂�
		*/
		const long& OutputLatency() const { return outputLatency; }

		/**
		* �T���v�����O�E���[�g��Ԃ�
		* @return �T���v�����O�E���[�g
		*/
		double SampleRate() const
		{
			return sampleRate;
		}

	private:
		inline void InitLatency()
		{
			ErrorCheck(driver->getLatencies(&inputLatency, &outputLatency));
		}

		inline void InitSampleRate()
		{
			ErrorCheck(driver->getSampleRate(&sampleRate));
		}

		/**
		* �T���v�����O�E���[�g��ݒ肷��
		* @params[in] rate �T���v�����O�E���[�g
		*/
		inline Preference& SetSampleRate(double rate)
		{
			ErrorCheck(driver->setSampleRate(rate));
			return *this;
		}

		inline Preference& SetSampleRate()
		{
			ErrorCheck(driver->setSampleRate(sampleRate));
			return *this;
		}

		/**
		* �T���v�����O�E���[�g��ݒ�ł��邩�����炵��
		* @params[in] rate �T���v�����O�E���[�g
		* @note �悭�킩��Ȃ��̂Ŕ񐄏��֐�
		*/
		Preference& CanSampleRate(double rate)
		{
			ErrorCheck(driver->canSampleRate(rate));
		}

	public:
		Preference(IASIO* iasio)
			: driver(iasio)
		{
			InitLatency();
			InitSampleRate();
		}
	};
}