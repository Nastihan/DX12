#include "App.h"
#include "imgui/imgui.h"



App::App()
	: wnd(2200, 1237.5, "DX12 Engine"),
	light(wnd.Gfx()),
	//cube(wnd.Gfx()),
	test(wnd.Gfx()),
	sphere(wnd.Gfx())
{
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 9.0f/16.0f, 0.5f, 100.f));
	wnd.Gfx().SetCamera(cam.GetMatrix());
	wnd.Gfx().SetLight(light);

}

void App::DoFrame()
{
	wnd.Gfx().BeginFrame();
	wnd.Gfx().SetCamera(cam.GetMatrix());
	// render loop body

	//cube.Draw(wnd.Gfx());
	test.Draw(wnd.Gfx());
	sphere.Draw(wnd.Gfx());

	if (showDemoWindow)
	{
		ImGui::ShowDemoWindow(&showDemoWindow);
	}
	

	// render loop body end
	wnd.Gfx().EndFrame();
}

void App::HandleInput(float dt)
{
	static bool spaceKeyPressedPrev = false;
	bool spaceKeyPressed = glfwGetKey(&wnd.Wnd(), GLFW_KEY_SPACE) == GLFW_PRESS;
	if (spaceKeyPressed && !spaceKeyPressedPrev) {
		// Toggle cursor state
		if (wnd.CursorEnabled()) {
			wnd.DisableCursor();
		}
		else {
			wnd.EnableCursor();
		}
	}
	spaceKeyPressedPrev = spaceKeyPressed;


	if (!wnd.CursorEnabled())
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
	
	{
		double mouseX, mouseY;
		glfwGetCursorPos(&wnd.Wnd(), &mouseX, &mouseY);
		const auto delta = wnd.GetMouseDelta((float)mouseX,(float)mouseY);
		if (!wnd.CursorEnabled())
		{
			cam.Rotate((float)delta.x, (float)delta.y);
		}
	}
}

void App::Run()
{
	while (!glfwWindowShouldClose(&wnd.Wnd()))
	{
		auto dt = timer.Mark() * 4.0f;
		HandleInput(dt);
		DoFrame();

		glfwPollEvents();
		if (glfwWindowShouldClose(&wnd.Wnd()))
			wnd.Gfx().QueueEmpty();
	}
}
