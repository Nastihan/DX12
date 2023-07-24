#pragma once
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <Windows.h>
#include "Graphics.h"

class Window
{
public:
	Window(uint16_t width, uint16_t height, std::string title);
	~Window();
	GLFWwindow& Wnd();
	HWND& Hwnd();

private:

	std::unique_ptr<Graphics> gfx;
	GLFWwindow* pWindow;
	uint16_t width;
	uint16_t height;
};