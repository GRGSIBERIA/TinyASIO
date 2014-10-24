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
	asio::Driver::Init(list[0]);

	auto driver = asio::Driver::Get();

	auto inputs = driver.InputChannels();
	auto outputs = driver.OutputChannels();

	if (inputs.size() <= 0)
		return 0;

	if (outputs.size() <= 0)
		return 0;

	

	return 0;
}