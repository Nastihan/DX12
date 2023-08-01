#include "App.h"
#include <DirectXTex.h>

App::App()
	: wnd(1600, 900, "DX12"),
	triangle(wnd.Gfx()),
	cube(wnd.Gfx())
{
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 9.0f/16.0f, 0.5f, 100.f));
	DirectX::XMMATRIX view;
	// setup view (camera) matrix
	{
		const auto eyePosition = DirectX::XMVectorSet(0, 0, -5, 1);
		const auto focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
		const auto upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
		view = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	}
	wnd.Gfx().SetCamera(view);

}

void App::Run()
{
	while (!glfwWindowShouldClose(&wnd.Wnd()))
	{
		wnd.Gfx().BeginFrame();
		// render loop body

		//triangle.Draw(wnd.Gfx());
		cube.Draw(wnd.Gfx());


		// render loop body end
		wnd.Gfx().EndFrame();
		glfwPollEvents();
		if (glfwWindowShouldClose(&wnd.Wnd()))
			wnd.Gfx().QueueEmpty();
	}
}
