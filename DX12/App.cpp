#include "App.h"

App::App()
	: wnd(1600, 900, "DX12"),
	//triangle(wnd.Gfx()),
	cube(wnd.Gfx())
{

	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 9.0f/16.0f, 0.5f, 100.f));
	cam.Translate({ 0,0, +1 });
	DirectX::XMMATRIX view = cam.GetMatrix();
	wnd.Gfx().SetCamera(cam.GetMatrix());

}

void App::DoFrame()
{
	wnd.Gfx().BeginFrame();
	wnd.Gfx().SetCamera(cam.GetMatrix());
	// render loop body

	//triangle.Draw(wnd.Gfx());
	cube.Draw(wnd.Gfx());
	       

	// render loop body end
	wnd.Gfx().EndFrame();
}

void App::Run()
{
	while (!glfwWindowShouldClose(&wnd.Wnd()))
	{
		auto dt = timer.Mark() * 1.0f;
		if (glfwGetKey(&wnd.Wnd(),GLFW_KEY_SPACE) == GLFW_PRESS)
		{ 
			dt = 0;
		}
		cam.Translate({ 0,0,-dt });

		DoFrame();

		glfwPollEvents();
		if (glfwWindowShouldClose(&wnd.Wnd()))
			wnd.Gfx().QueueEmpty();
	}
}
