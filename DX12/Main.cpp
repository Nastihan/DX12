#include <iostream>
#include "App.h"
#include <stdexcept>

void LogException(const char* file, int line) {
	char message[256];
	sprintf_s(message, sizeof(message), "Exception caught in file %s at line %d\n", file, line);
	OutputDebugStringA(message);
}

int main()
{
	try {
		App{}.Run();
	}
	catch (const std::exception& e) {	
		MessageBoxA(nullptr, e.what(), "Error",MB_ICONERROR | MB_SETFOREGROUND);
		LogException(__FILE__, __LINE__);
		OutputDebugStringA(e.what());
	}
	return 0;
}  