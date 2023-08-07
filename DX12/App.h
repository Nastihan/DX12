#pragma once
#include "Window.h"
#include "GraphicsError.h"
#include "Triangle.h"
#include "Cube.h"
#include "NastihanTimer.h"
#include "Camera.h"
#include "ImguiManager.h"
#include "AssimpTest.h"
#include "Drawable.h"
#include "Sphere.h"
#include "PointLight.h"

class App
{
public:
	App();
	void Run();
private:
	void DoFrame();
	void HandleInput(float dt);
private:
	ImguiManager imgui;
	Window wnd;
	NastihanTimer timer;
	PointLight light;
	Camera cam;
	//Cube cube;
	AssimpTest test;
	bool showDemoWindow = true;
};