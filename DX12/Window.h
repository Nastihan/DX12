#pragma once
#include <initguid.h> 
#include <glfw/include/GLFW/glfw3.h>
#include <memory>
#include <string>
#include <Windows.h>
#include "Graphics.h"

class Window
{
public:
	Window(uint16_t width, uint16_t height, std::string title);
	~Window();
	Graphics& Gfx();
	GLFWwindow& Wnd();
	HWND& Hwnd();
	void EnableCursor();
	void DisableCursor();
	bool CursorEnabled();
	DirectX::XMFLOAT2 GetMouseDelta(float x,float y);

private:
	bool cursorEnabled = true;
	float lastMouseX;
	float lastMouseY;
	std::unique_ptr<Graphics> pGfx;
	GLFWwindow* pWindow;
	uint16_t width;
	uint16_t height;
};