#include <iostream>
#include "App.h"
#include <stdexcept>

int main()
{
	try {
		App{}.Run();
	}
	catch (const std::exception& e) {	
		MessageBoxA(nullptr, e.what(), "Error",MB_ICONERROR | MB_SETFOREGROUND);
	
	}
	return -1;
}  