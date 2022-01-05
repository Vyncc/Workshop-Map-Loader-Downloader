#include "pch.h"
#include "WorkshopMapLoader.h"
#include "IMGUI/imgui_internal.h"


void Pluginx64::RenderSettings()
{
	if(ImGui::Button("Open Plugin Window"))
	{
		gameWrapper->Execute([&](GameWrapper* gw) {
			cvarManager->executeCommand("togglemenu WorkshopMapLoaderMenu");
			});
	}

	ImGui::Separator();

	ImGui::Text("Set A Bind To Open The Window : (you can find the bind in the \"bindings\" tab of bakkesmod)");
	static char KeyBindInputBuf[200] = "F3";
	ImGui::InputText("##KeyBindInput", KeyBindInputBuf, IM_ARRAYSIZE(KeyBindInputBuf));

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("BindSet", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Open plugin window : %s", std::string(KeyBindInputBuf));
		ImGui::NewLine();

		if (ImGui::Button("OK", ImVec2(100.f, 25.f))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}


	if (ImGui::Button("Set Bind"))
	{
		gameWrapper->Execute([&](GameWrapper* gw) {
			cvarManager->setBind(KeyBindInputBuf, "togglemenu WorkshopMapLoaderMenu");
			cvarManager->log("New key bind set : " + std::string(KeyBindInputBuf) + " -> togglemenu WorkshopMapLoaderMenu");
			});

		ImGui::OpenPopup("BindSet");
	}

	ImGui::Separator();
	ImGui::Text("Plugin made by Vync#3866, contact me on discord for custom plugin commissions.");
}

std::string Pluginx64::GetPluginName()
{
	return "WorkshopMapLoader";
}