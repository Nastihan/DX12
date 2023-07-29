#include "App.h"

App::App()
	: wnd(1600, 900, "DX12"),
	triangle(wnd.Gfx())
{
}

void App::Run()
{
	while (!glfwWindowShouldClose(&wnd.Wnd()))
	{
		wnd.Gfx().BeginFrame();
		
		// render loop body

		//wnd.Gfx().DrawTriangle();

		// render loop body end


		wnd.Gfx().EndFrame();
		glfwPollEvents();
		if (glfwWindowShouldClose(&wnd.Wnd()))
			wnd.Gfx().QueueEmpty();
	}
}
