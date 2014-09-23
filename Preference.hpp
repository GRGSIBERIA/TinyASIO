#pragma once

#include "Interface.hpp"

namespace asio
{
	/**
	* �ݒ�֌W�̃N���X
	*/
	class Preference
	{
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

		/**
		* �T���v�����O�E���[�g��ݒ肷��
		* @params[in] rate �T���v�����O�E���[�g
		*/
		Preference& SampleRate(double rate)
		{
			ErrorCheck(driver->setSampleRate(rate));
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

	private:
		void InitLatency()
		{
			ErrorCheck(driver->getLatencies(&inputLatency, &outputLatency));
		}

		void InitSampleRate()
		{
			ErrorCheck(driver->getSampleRate(&sampleRate));
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