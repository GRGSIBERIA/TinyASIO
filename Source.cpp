#define UNICODE

#include <windows.h>
#include <iostream>
using namespace std;

#include "TinyASIO.hpp"


int main()
{
	if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) return -1;

	asio::InputBackController controller("AudioBox");

	controller.Start();

	while (true)
	{
		auto buffer = controller.Fetch();
	}

	controller.Stop();

	return 0;
}