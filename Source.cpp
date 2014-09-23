#include <windows.h>
#include <shobjidl.h> 
#include <iostream>

#include "Registory.hpp"
#include "Driver.hpp"


#define TEST_HR(hr) if(!SUCCEEDED(hr)) return -1


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	
	TEST_HR(hr);

	auto list = asio::ASIORegistory::GetAsioDriverPathes();
	auto clsids = asio::ASIORegistory::GetCLSIDs(list);

	auto a = asio::ASIODriver((*clsids).at(0));
	auto ic = a.GetBufferPreference();

	return 0;
}