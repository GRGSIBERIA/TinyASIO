#include <windows.h>
#include <shobjidl.h> 
#include <iostream>
using namespace std;

#include "TinyASIO.hpp"


#define TEST_HR(hr) if(!SUCCEEDED(hr)) return -1


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	
	TEST_HR(hr);

	auto list = asio::Registory::GetAsioDriverPathes();

	auto driver = asio::Driver::Init(list[0]);

	const auto& inputs = driver.InputChannels();
	const auto& outputs = driver.OutputChannels();

	if (inputs.size() > 0)
	{
		driver.AddChannel(inputs[0]);
	}

	if (outputs.size() > 0)
	{
		driver.AddChannel(outputs[0]);
	}

	const auto& controller = driver.CreateBuffer(asio::pack::Sample(asio::pack::Int, false));

	controller.Start();

	while (1)
	{
		
	}

	controller.Stop();

	return 0;
}