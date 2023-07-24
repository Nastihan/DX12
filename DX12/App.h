#pragma once
#include "Window.h"
#include <d3d12.h>
#include <wrl.h>
#include "GraphicsError.h"

class App
{
public:
	App()
		:
		window(1600, 900, "Engine Window")
	{
		Microsoft::WRL::ComPtr<ID3D12Device> device;
		D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device)) >> chk;
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