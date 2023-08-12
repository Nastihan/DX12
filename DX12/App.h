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
#include "SpherePBR.h"
#include "TriangleRT.h"

class App
{
public:
	App();
	void Run();
private:
	void DoFrame();
	void HandleInput(float dt);
	void ShowFPSWindow();
private:
	ImguiManager imgui;
	Window wnd;
	NastihanTimer timer;
	PointLight light{ wnd.Gfx() };
	Camera cam;
	SpherePBR spherePBR{ wnd.Gfx() };
	AssimpTest test{ wnd.Gfx() };
	TriangleRT triangleRT{ wnd.Gfx() };
	bool showDemoWindow = true;
};