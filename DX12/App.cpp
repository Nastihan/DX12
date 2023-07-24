#include "App.h"

App::App()
	: window(1600, 900, "Engine Window")

{
}

void App::Run()
{
	while (!glfwWindowShouldClose(&window.Wnd()))
	{
		glfwPollEvents();
	}
}
