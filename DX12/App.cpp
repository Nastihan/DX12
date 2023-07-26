#include "App.h"

App::App()
	: wnd(1600, 900, "DX12")
{
}

void App::Run()
{
	while (!glfwWindowShouldClose(&wnd.Wnd()))
	{
		


		glfwPollEvents();
	}
}
