#include "App.h"

App::App()
	: window(1600, 900, "DX12")
{
}

void App::Run()
{
	while (!glfwWindowShouldClose(&window.Wnd()))
	{
		glfwPollEvents();
	}
}
