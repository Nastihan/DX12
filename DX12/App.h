#pragma once
#include "Window.h"
#include "GraphicsError.h"
#include "Triangle.h"
#include "Cube.h"
#include "NastihanTimer.h"
#include "Camera.h"

class App
{
public:
	App();
	void Run();
private:
	Window wnd;
	NastihanTimer timer;
	Camera cam;
	//Triangle triangle;
	Cube cube;
};