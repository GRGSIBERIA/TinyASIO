#define UNICODE

#include <windows.h>
#include <iostream>
using namespace std;

#include "TinyASIO.hpp"



#define TEST_HR(hr) if(!SUCCEEDED(hr)) return -1


int main()
{
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	TEST_HR(hr);

	auto driver = asio::Driver::Init("AudioBox");

	return 0;
}