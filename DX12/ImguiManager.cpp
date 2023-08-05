#include "ImguiManager.h"
#include "imgui/imgui.h"

ImguiManager::ImguiManager()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScaleAllSizes(1.5f);
	ImGui::GetIO().FontGlobalScale = 1.4f;
}

ImguiManager::~ImguiManager()
{
	ImGui::DestroyContext();
}
