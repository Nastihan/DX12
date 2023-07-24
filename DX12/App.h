#pragma once
#include "Window.h"
#include <d3d12.h>
#include <wrl.h>
#include "GraphicsError.h"

class App
{
public:
	App();
	void Run();
private:
	Window window;

};