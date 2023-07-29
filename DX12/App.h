#pragma once
#include "Window.h"
#include "GraphicsError.h"
#include "Triangle.h"

class App
{
public:
	App();
	void Run();
private:
	Window wnd;
	Triangle triangle;
};