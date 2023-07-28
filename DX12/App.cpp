#include "App.h"

App::App()
	: wnd(1600, 900, "DX12")
{
}

void App::Run()
{
	while (!glfwWindowShouldClose(&wnd.Wnd()))
	{
		wnd.Gfx().BeginFrame();
		






		wnd.Gfx().EndFrame();
		glfwPollEvents();

		if (glfwWindowShouldClose(&wnd.Wnd()))
			wnd.Gfx().QueueEmpty();
	}
}
