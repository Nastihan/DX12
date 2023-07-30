#include "App.h"

App::App()
	: wnd(1600, 900, "DX12"),
	triangle(wnd.Gfx())
{
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1600.0f, 900.0f, 0.5f, 20.f));
	DirectX::XMMATRIX view;
	{
		// setup view (camera) matrix
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

		triangle.Draw(wnd.Gfx());

		//wnd.Gfx().DrawTriangle();

		// render loop body end


		wnd.Gfx().EndFrame();
		glfwPollEvents();
		if (glfwWindowShouldClose(&wnd.Wnd()))
			wnd.Gfx().QueueEmpty();
	}
}
