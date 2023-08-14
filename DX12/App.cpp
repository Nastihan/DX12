#include "App.h"
#include "imgui/imgui.h"



App::App()
	: wnd(2200, 1237, "DX12 Engine")
{
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 9.0f/16.0f, 0.5f, 100.f));
	wnd.Gfx().SetCamera(cam.GetMatrix());
	wnd.Gfx().SetLight(light);

}

void App::DoFrame()
{
	wnd.Gfx().BeginFrame();
	if (!wnd.Gfx().RTEnabled())
	{
		wnd.Gfx().SetCamera(cam.GetMatrix());

		// update buffers
		light.Update(wnd.Gfx(), cam.GetMatrix());
		spherePBR.Update(wnd.Gfx());

		// render loop body begin
		// reset command list
		wnd.Gfx().ResetCmd();

		// Draw Calls
		light.Draw(wnd.Gfx());
		//cube.Draw(wnd.Gfx());
		//test.Draw(wnd.Gfx());
		spherePBR.Draw(wnd.Gfx());

		// execute
		wnd.Gfx().Execute();
		wnd.Gfx().Sync();

		// Imgui Calls
		light.SpawnControlWindow();
		cam.SpawnControlWindow();
		spherePBR.SpawnControlWindow();
		ShowFPSWindow();

		wnd.Gfx().EndFrame();
	}
	else 
	{

		wnd.Gfx().ResetCmd();


		triangleRT.Draw(wnd.Gfx());

		wnd.Gfx().Execute();
		wnd.Gfx().Sync();

		wnd.Gfx().EndFrameRT(triangleRT.GetOutputBuffer());
	}


	// render loop body end
}

void App::HandleInput(float dt)
{
	static bool xKeyPressedPrev = false;
	bool xKeyPressed = glfwGetKey(&wnd.Wnd(), GLFW_KEY_X) == GLFW_PRESS;
	if (xKeyPressed && !xKeyPressedPrev) {
		// Toggle cursor state
		if (wnd.Gfx().RTEnabled()) 
		{			
			wnd.Gfx().QueueEmpty();
			wnd.Gfx().DisableRT();
		}
		else 
		{
			wnd.Gfx().QueueEmpty();
			wnd.Gfx().EnableRT();
		}
	}
	xKeyPressedPrev = xKeyPressed;
	
	if (!wnd.Gfx().RTEnabled())
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
			const auto delta = wnd.GetMouseDelta((float)mouseX, (float)mouseY);
			if (!wnd.CursorEnabled())
			{
				cam.Rotate((float)delta.x, (float)delta.y);
			}
		}
	}
}

void App::ShowFPSWindow()
{
	if (ImGui::Begin("FPS"))
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
	ImGui::End();
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
		{
			wnd.Gfx().QueueEmpty();
		}
	}
}
