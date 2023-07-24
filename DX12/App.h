#pragma once
#include "Window.h"

class App
{
public:
	App()
		:
		window(1600, 900, "Engine Window")
	{

	}
	void Run()
	{
		while (!glfwWindowShouldClose(&window.Wnd()))
		{
			glfwPollEvents();
		}

	}
private:
	Window window;

};