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

void App::Run()
{
	while (!glfwWindowShouldClose(&wnd.Wnd()))
	{
		const auto dt = timer.Mark() * 1.0f;
		cam.Translate({ 0,0,-dt });
		wnd.Gfx().BeginFrame();
		wnd.Gfx().SetCamera(cam.GetMatrix());
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
