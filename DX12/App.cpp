#include "App.h"

App::App()
	: wnd(1600, 900, "DX12"),
	//triangle(wnd.Gfx()),
	cube(wnd.Gfx())
{

	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 9.0f/16.0f, 0.5f, 100.f));
	
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

void App::HandleInput(float dt)
{
	if (glfwGetKey(&wnd.Wnd(), GLFW_KEY_W) == GLFW_PRESS)
	{
		cam.Translate({ 0.0f,0.0f,dt });
	}
	if (glfwGetKey(&wnd.Wnd(), GLFW_KEY_S) == GLFW_PRESS)
	{
		cam.Translate({ 0.0f,0.0f,-dt });
	}
	if (glfwGetKey(&wnd.Wnd(), GLFW_KEY_A) == GLFW_PRESS)
	{
		cam.Translate({ -dt,0.0f,0.0f });
	}
	if (glfwGetKey(&wnd.Wnd(), GLFW_KEY_D) == GLFW_PRESS)
	{
		cam.Translate({ dt,0.0f,0.0f });
	}
	if (glfwGetKey(&wnd.Wnd(), GLFW_KEY_R) == GLFW_PRESS)
	{
		cam.Translate({ 0.0f,dt,0.0f });
	}
	if (glfwGetKey(&wnd.Wnd(), GLFW_KEY_F) == GLFW_PRESS)
	{
		cam.Translate({ 0.0f,-dt,0.0f });
	}
}

void App::Run()
{
	while (!glfwWindowShouldClose(&wnd.Wnd()))
	{
		auto dt = timer.Mark() * 2.0f;
		if (glfwGetKey(&wnd.Wnd(), GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			dt = 0;
		}
		HandleInput(dt);
		DoFrame();

		glfwPollEvents();
		if (glfwWindowShouldClose(&wnd.Wnd()))
			wnd.Gfx().QueueEmpty();
	}
}
