#include <windows.h>
#include <shobjidl.h> 
#include <iostream>

#include "Registory.hpp"
#include "Driver.hpp"


#define TEST_HR(hr) if(!SUCCEEDED(hr)) return -1


class CallbackTest
{
	static asio::Driver* driver;
	static asio::BufferArray bufferArray;

public:
	static void Init(asio::Driver* driver)
	{
		CallbackTest::driver = driver;
		auto callbacks = asio::ASIOCallbacks();
		callbacks.bufferSwitch = &CallbackTest::switched;
		callbacks.asioMessage = &CallbackTest::message;
		callbacks.bufferSwitchTimeInfo = &CallbackTest::switchedWithTimeInfo;
		callbacks.sampleRateDidChange = &CallbackTest::changeSampleRate;

		auto& in = driver->Channel().Inputs(0);
		driver->Buffer().AddChannel(in);

		driver->CreateBuffer(callbacks);
	}

	static void switched(long doubleBufferIndex, asio::ASIOBool directProcess)
	{
		
	}

	static void changeSampleRate(asio::ASIOSampleRate sRate)
	{

	}

	static long message(long selector, long value, void* message, double* opt)
	{
		return 0;
	}

	static asio::ASIOTime* switchedWithTimeInfo(asio::ASIOTime* params, long doubleBufferIndex, asio::ASIOBool directProcess)
	{
		return params;
	}
};

asio::Driver* CallbackTest::driver = nullptr;
asio::BufferArray CallbackTest::bufferArray;


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	
	TEST_HR(hr);

	auto list = asio::ASIORegistory::GetAsioDriverPathes();
	auto clsids = asio::ASIORegistory::GetCLSIDs(list);

	auto driver = asio::Driver((*clsids).at(0));


	CallbackTest::Init(&driver);
	

	return 0;
}