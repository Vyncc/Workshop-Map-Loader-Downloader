#include "pch.h"
#include "WorkshopMapLoader.h"
#include "FileExplorer.h"
#include "IMGUI/imgui_internal.h"

namespace fs = std::filesystem;


float heightMutators = 35.f;
float heightCustomGamePopup = 194.f;

float windowSizeXBefore = ImGui::GetWindowSize().x;
float windowSizeYBefore = ImGui::GetWindowSize().y;


void Pluginx64::Render()
{
	HANDLE clip; //ClipBoard to copy and paste
	if (OpenClipboard(NULL))
	{
		clip = GetClipboardData(CF_TEXT);
		CloseClipboard();
	}



	ImGui::SetNextWindowSizeConstraints(ImVec2(1326.f, 690.f), ImVec2(1920.f, 1080.f));

	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_MenuBar))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	//Ctrl + F
	static bool CtrlFPressed = false;

	if ((GetKeyState(VK_CONTROL) & 0x8000 && GetKeyState('F') & 0x8000) && !CtrlFPressed)
	{
		//cvarManager->log("ctrl+f pressed");
		CtrlFPressed = true;
	}
	else if(!(GetKeyState(VK_CONTROL) & 0x8000 && GetKeyState('F') & 0x8000) && CtrlFPressed)
	{
		strncpy(QuickSearch_KeyWordBuf, "", IM_ARRAYSIZE(QuickSearch_KeyWordBuf));

		if (!isQuickSearchDisplayed)
		{
			isQuickSearchDisplayed = true;
		}
		else
		{
			isQuickSearchDisplayed = false;
		}

		//cvarManager->log("ctrl+f released");
		CtrlFPressed = false;
	}

	if (!g_pluginConfig.HasSeeNewUpdateAlert)
	{
		ImGui::OpenPopup("New Update");
	}
	renderNewUpdatePopup();

	if (AddedMapSccuessfully)
	{
		ImGui::OpenPopup("Add Map Successfull");
		AddedMapSccuessfully = false;
	}
	renderInfoPopup("Add Map Successfull", g_pluginTexts.MapAddedSuccessfullyText.c_str()); //"Map added successfully !"


	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(g_pluginTexts.SettingsText.c_str())) //"Settings"
		{
			if (ImGui::BeginMenu(g_pluginTexts.ExtractMethodText.c_str()))
			{
				if (ImGui::Selectable("Batch File Method"))
				{
					g_pluginConfig.UnzipMethod = "Bat";

					if (Utils::DirectoryOrFileExists(g_pluginConfig.BakkesmodPath + "data\\WorkshopMapLoader\\")) //save changes in cfg
					{
						SaveInCFG();
					}
				}

				if (g_pluginConfig.UnzipMethod == "Bat")
				{
					ImGui::SameLine();
					ImGui::Text(" : Selected");
				}

				if (ImGui::Selectable("Powershell Method"))
				{
					g_pluginConfig.UnzipMethod = "Powershell";

					if (Utils::DirectoryOrFileExists(g_pluginConfig.BakkesmodPath + "data\\WorkshopMapLoader\\")) //save changes in cfg
					{
						SaveInCFG();
					}
				}

				if (g_pluginConfig.UnzipMethod == "Powershell")
				{
					ImGui::SameLine();
					ImGui::Text(" : Selected");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(g_pluginTexts.LanguageText.c_str())) //"Language"
			{
				if (ImGui::Selectable("French"))
				{
					g_pluginConfig.Language = true;
					g_pluginTexts.SetLanguage(g_pluginConfig.Language);
					SaveInCFG();
				}

				if (ImGui::Selectable("English"))
				{
					g_pluginConfig.Language = false;
					g_pluginTexts.SetLanguage(g_pluginConfig.Language);
					SaveInCFG();
				}
				ImGui::EndMenu();
			}

			if (ImGui::Selectable(g_pluginTexts.DlTexturesText.c_str())) //"Download Textures"
			{
				DownloadTexturesBool = true;
			}

			if (ImGui::Checkbox("Enable Freeze Fix", &g_pluginConfig.EnableAntiFreezeFix))
			{
				SaveInCFG();
			}

			ImGui::EndMenu();
		}


		if (ImGui::BeginMenu(g_pluginTexts.MultiplayerText.c_str())) //"Multiplayer"
		{
			if (ImGui::Selectable(g_pluginTexts.OpenCPCCText.c_str())) //"Open CookedPCConsole Directory"
			{
				std::wstring w_modsDir = Utils::StringToWideString(RLCookedPCConsole_Path.string());
				LPCWSTR L_modsDir = w_modsDir.c_str();

				ShellExecute(NULL, L"open", L_modsDir, NULL, NULL, SW_SHOWDEFAULT);
			}

			ImGui::Separator();

			ImGui::Text(g_pluginTexts.JoinCWGText.c_str()); //"Join Community Workshop Games discord server :"
			renderLink("https://discord.com/invite/KVgmf9JFpZ");

			ImGui::EndMenu();
		}

		if (DownloadTexturesBool) //I know this is not good but It works, so I don't care
		{
			ImGui::OpenPopup("DownloadTextures");
		}
		renderDownloadTexturesPopup(CheckExist_TexturesFiles());


		if (ImGui::Selectable(g_pluginTexts.LastUpdateText.c_str(), false, 0, ImGui::CalcTextSize(g_pluginTexts.LastUpdateText.c_str())))
		{
			g_pluginConfig.HasSeeNewUpdateAlert = false;
		}

		if (ImGui::BeginMenu("Support Me"))
		{
			ImGui::Text("Support me on Patreon :");
			renderLink("https://www.patreon.com/WorkshopMapLoader");

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Credits"))
		{
			ImGui::Text("Plugin made by Vync#3866, contact me on discord or twitter for custom plugin commissions.");
			renderLink("https://twitter.com/Vync5");
			renderLink("https://www.youtube.com/@vync8978");
			ImGui::NewLine();
			ImGui::Text("Thanks to PatteAuSucre for testing and Teyq for his help ;)");
			ImGui::Text("Thanks to JetFox for his contribution on the plugin");

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (ImGui::BeginTabBar("TabBar"))
	{
		if (ImGui::BeginTabItem(g_pluginTexts.Tab1MapLoaderText.c_str())) // "Map Loader"
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);

			CenterNexIMGUItItem(ImGui::CalcTextSize(g_pluginTexts.Label1Text.c_str()).x);
			ImGui::Text(g_pluginTexts.Label1Text.c_str()); // "Put the folder's path of the maps, don't forget to add a  /  at the end."

			CenterNexIMGUItItem(628.f);
			ImGui::SetNextItemWidth(628.f);
			ImGui::InputText("##workshopurl123", g_pluginConfig.MapsFolderPathBuffer, IM_ARRAYSIZE(g_pluginConfig.MapsFolderPathBuffer));
			ImGui::SameLine();
			if (!Utils::DirectoryOrFileExists(fs::path(g_pluginConfig.MapsFolderPathBuffer)))
			{
				ImGui::TextColored(ImVec4(255, 0, 0, 1), g_pluginTexts.DirNotExistText.c_str());
			}
			else
			{
				ImGui::Text("");
			}

			CenterNexIMGUItItem(628.f); //306.f("Save Path" button) + 8.f(the gap between both buttons) + 306.f("Refresh Mas" button) = 620.f  (cause they are on same line)

			static FileExplorer mapsFolderExplorer;
			if (ImGui::Button(g_pluginTexts.SelectMapsFolderText.c_str(), ImVec2(151.f, 32.f))) // "Select maps folder"
			{
				FileExplorer::Config config;
				config.showPreview = false;
				config.fileFilters = { ".udk", ".upk" };
				mapsFolderExplorer.SetConfig(config);
				mapsFolderExplorer.SetOnPathSelected([](const std::string& path, const std::string& selectedEntryPath) {
					std::string selectedMapsPath = selectedEntryPath.empty()
						? path
						: fs::path(selectedEntryPath).parent_path().string();

					strncpy(g_pluginConfig.MapsFolderPathBuffer, selectedMapsPath.c_str(), IM_ARRAYSIZE(g_pluginConfig.MapsFolderPathBuffer) - 1);
					g_pluginConfig.MapsFolderPathBuffer[IM_ARRAYSIZE(g_pluginConfig.MapsFolderPathBuffer) - 1] = '\0';
				});
				mapsFolderExplorer.Open(g_pluginConfig.MapsFolderPathBuffer);
			}
			mapsFolderExplorer.Render();

			ImGui::SameLine();

			if (ImGui::Button(g_pluginTexts.SavePathText.c_str(), ImVec2(151.f, 32.f))) // "Save Path"
			{
				if (Utils::DirectoryOrFileExists(g_pluginConfig.BakkesmodPath + "data\\WorkshopMapLoader\\"))
				{
					SaveInCFG();

					ImGui::OpenPopup("SavePath");
				}
			}
			renderInfoPopup("SavePath", g_pluginTexts.PathSavedText.c_str());


			ImGui::SameLine();

			if (ImGui::Button(g_pluginTexts.AddMapText.c_str(), ImVec2(151.f, 32.f))) // "Add Map"
			{
				ImGui::OpenPopup(g_pluginTexts.AddMapText.c_str()); //"Add Map"
			}
			renderAddMapManuallyPopup();

			ImGui::SameLine();

			std::vector<std::string> missingTexturesFiles = CheckExist_TexturesFiles();

			if (ImGui::Button(g_pluginTexts.RefreshMapsButtonText.c_str(), ImVec2(151.f, 32.f))) // "Refresh Maps"
			{
				if (!Utils::DirectoryOrFileExists(fs::path(g_pluginConfig.MapsFolderPathBuffer)))
				{
					ImGui::OpenPopup("Exists?");
				}
				else
				{
					RefreshMapsFunct(g_pluginConfig.MapsFolderPathBuffer);
					if (missingTexturesFiles.size() > 0 && g_pluginConfig.DontAsk == 0)
					{
						ImGui::OpenPopup("DownloadTextures");
					}
				}
			}
			renderInfoPopup("Exists?", g_pluginTexts.DirNotExistText.c_str());

			renderDownloadTexturesPopup(missingTexturesFiles);

			ImGui::SameLine();

			ImGui::BeginGroup(); //display mode + maps on line
			{
				AlignRightNexIMGUItItem(95.f, 8.f);
				ImVec2 cursorPos = ImGui::GetCursorPos();
				if (g_pluginConfig.MapsDisplayMode == 1)
				{
					ImGui::SetCursorPos(ImVec2(cursorPos.x + 14.f, cursorPos.y - 48.f));
					ImGui::BeginGroup();
					{
						ImGui::Text(g_pluginTexts.MapsPerLineText.c_str()); //"Maps Per Line : "
						ImGui::SetNextItemWidth(80.f);
						if (ImGui::BeginCombo("##mapsinline", std::to_string(g_pluginConfig.NbTilesPerLine).c_str()))
						{
							for (int i = 2; i <= 14; i++)
							{
								if (ImGui::Selectable(std::to_string(i).c_str()))
								{
									g_pluginConfig.NbTilesPerLine = i;
									SaveInCFG();
								}
							}
							ImGui::EndCombo();
						}

						ImGui::EndGroup();
					}
				}
				ImGui::SetCursorPos(ImVec2(cursorPos.x + 14.f, cursorPos.y - 4.f));
				ImGui::BeginGroup();
				{
					ImTextureID TextureID_DisplayMode1Image = MapsDisplayMode_Logo1_Image->GetImGuiTex();
					if (g_pluginConfig.MapsDisplayMode == 0)
					{
						TextureID_DisplayMode1Image = MapsDisplayMode_Logo1_SelectedImage->GetImGuiTex();
					}
					renderImageButton(TextureID_DisplayMode1Image, ImVec2(36.f, 36.f), [this]() {
						g_pluginConfig.MapsDisplayMode = 0;
						cvarManager->log("MapsDisplayMode set to : 0");
						SaveInCFG();
						});

					ImGui::SameLine();

					ImTextureID TextureID_DisplayMode2Image = MapsDisplayMode_Logo2_Image->GetImGuiTex();
					if (g_pluginConfig.MapsDisplayMode == 1)
					{
						TextureID_DisplayMode2Image = MapsDisplayMode_Logo2_SelectedImage->GetImGuiTex();
					}
					renderImageButton(TextureID_DisplayMode2Image, ImVec2(36.f, 36.f), [this]() {
						g_pluginConfig.MapsDisplayMode = 1;
						cvarManager->log("MapsDisplayMode set to : 1");
						SaveInCFG();
						});

					ImGui::EndGroup();
				}
				ImGui::EndGroup();
			}
			
			//ImGui::Separator();

			if (IsDownloading_WorkshopTextures)
			{
				ImGui::Separator();

				std::string ProgressBar_Label = Utils::ConvertToMB(std::to_string(DownloadTextrures_ProgressDisplayed)) + " / " + Utils::ConvertToMB(std::to_string(46970000));
				Utils::RenderProgressBar(DownloadTextrures_ProgressDisplayed, 46970000.f, ImGui::GetCursorScreenPos(), ImVec2(1305.f, 24.f),
					ImColor(112, 112, 112, 255), ImColor(33, 65, 103, 255), ProgressBar_Label.c_str());

				ImGui::Separator();
			}

			renderQuickSearch();

			renderMaps();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(g_pluginTexts.InstallMapTab.c_str()))
		{
			if(m_bakkesPluginsMapInstaller.ConsumeRefreshRequested() && Utils::DirectoryOrFileExists(g_pluginConfig.MapsFolderPathBuffer))
				RefreshMapsFunct(g_pluginConfig.MapsFolderPathBuffer);

			m_bakkesPluginsMapInstaller.Render();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}




void Pluginx64::renderMaps()
{
	std::vector<Map> NoUpk_MapList;
	std::vector<Map> Good_MapList;

	int ID = 0;

	/*
	ImGui::Text("window width : %f", ImGui::GetWindowWidth());
	ImGui::Separator();

	ImGui::SliderInt("width", &widthTest, -100, 1920);
	ImGui::SliderInt("height", &heightTest, -100, 500);
	ImGui::SliderFloat("fontsize", &fontSizeTest, 0.f, 1.f);
	*/
	
	if (ImGui::BeginChild("#MapsLauncherButtons"))
	{
		float windowWidth = ImGui::GetContentRegionAvailWidth();
		float buttonWidth = (windowWidth - ((g_pluginConfig.NbTilesPerLine - 1) * 8)) / g_pluginConfig.NbTilesPerLine;
		int nbTilesOnTheCurrentLine = 0;

		for (auto map : MapList)
		{
			if (map.UpkFile == "NoUpkFound" && map.ZipFile != "EmptyFolder" && map.ZipFile != "NoZipFound")
			{
				NoUpk_MapList.push_back(map);
			}

			if (map.UpkFile != "NoUpkFound" && map.UpkFile != "EmptyFolder")
			{
				Good_MapList.push_back(map);
			}
		}

		for (auto curMap : NoUpk_MapList)
		{
			ImGui::PushID(ID); //needed to make the button work
			ImGui::BeginGroup();
			{
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				ImFont* fontA = ImGui::GetDefaultFont();


				if (ImGui::Button("##map", ImVec2(ImGui::GetWindowWidth(), 120)))
				{
					ImGui::OpenPopup("ExtractMapFiles");
				}
				renderExtractMapFilesPopup(curMap);

				ImVec2 ButtonRectMin = ImGui::GetItemRectMin();
				ImVec2 ButtonRectMax = ImGui::GetItemRectMax();
				ImVec2 ImageMin = ImVec2(ButtonRectMin.x + 5.f, ButtonRectMin.y + 5.f);
				ImVec2 ImageMax = ImVec2(ImageMin.x + 190.f, ButtonRectMax.y - 5.f);


				draw_list->AddRect(ImageMin, ImageMax, ImColor(255, 255, 255, 255), 0, 15, 2.0F);
				if (curMap.isPreviewImageLoaded == true)
				{
					try
					{
						if (curMap.PreviewImage != nullptr)
						{
							if (curMap.PreviewImage->GetImGuiTex())
							{
								draw_list->AddImage(curMap.PreviewImage->GetImGuiTex(), ImageMin, ImageMax); //Map image preview
							}
						}
					}
					catch (const std::exception& ex)
					{
						cvarManager->log(ex.what());
					}
				}

				std::string mapName;
				if (curMap.JsonFile == "NoInfos")
				{
					mapName = replace(curMap.Folder.filename().string(), *"_", *" ");
				}
				else
				{
					mapName = curMap.mapName;
				}


				draw_list->AddText(fontA, 25.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 2.f), ImColor(255, 255, 255, 255),
					mapName.c_str());

				draw_list->AddText(fontA, 25.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 40.f), ImColor(255, 0, 0, 255),
					"This map wont work because the map isn't extracted, click to fix.");

				ImGui::EndGroup();

				if (ImGui::BeginPopupContextItem("Map context menu"))
				{
					if (ImGui::Selectable(g_pluginTexts.OpenMapDirText.c_str())) // "Open map directory"
					{
						std::wstring w_CurrentMapsDir = Utils::StringToWideString(curMap.Folder.string());
						LPCWSTR L_CurrentMapsDir = w_CurrentMapsDir.c_str();

						ShellExecute(NULL, L"open", L_CurrentMapsDir, NULL, NULL, SW_SHOWDEFAULT);
					}

					if (ImGui::Selectable(g_pluginTexts.DeleteMapText.c_str())) // "Delete Map"
					{
						MapList.clear();
						fs::remove_all(curMap.Folder);
						RefreshMapsFunct(g_pluginConfig.MapsFolderPathBuffer);
					}
					ImGui::EndPopup();
				}
			}
			ImGui::PopID();
			ID++;
		}

		if (std::string(QuickSearch_KeyWordBuf) != "") //if there is something to find
		{
			Good_MapList = QuickSearch_GetMapList(std::string(QuickSearch_KeyWordBuf));
			QuickSearch_Searching = true;
		}
		else
		{
			QuickSearch_Searching = false;
		}

		for (auto curMap : Good_MapList)
		{
			ImGui::PushID(ID); //needed to make the button work

			if (g_pluginConfig.MapsDisplayMode == 0)
			{
				renderMaps_DisplayMode_0(curMap);

				ImGui::PopID();
				ID++;
			}
			else
			{
				renderMaps_DisplayMode_1(curMap, buttonWidth);
				nbTilesOnTheCurrentLine++;
				ImGui::PopID();
				ID++;
				if (nbTilesOnTheCurrentLine == g_pluginConfig.NbTilesPerLine)
				{
					nbTilesOnTheCurrentLine = 0;
					ImGui::NewLine();
				}
				else
				{
					ImGui::SameLine();
				}
			}
		}
	}
	ImGui::EndChild();
}

void Pluginx64::renderMaps_DisplayMode_0(Map map)
{
	ImGui::BeginGroup();
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImFont* fontA = ImGui::GetDefaultFont();

		float windowWidth = ImGui::GetContentRegionAvailWidth();
		
		if (ImGui::Button("##map", ImVec2(ImGui::GetWindowWidth(), 120)))
		{
			ImGui::OpenPopup("LaunchMode");
		}
		renderLaunchModePopup(map);

		ImVec2 ButtonRectMin = ImGui::GetItemRectMin();
		ImVec2 ButtonRectMax = ImGui::GetItemRectMax();
		ImVec2 ImageMin = ImVec2(ButtonRectMin.x + 5.f, ButtonRectMin.y + 5.f);
		ImVec2 ImageMax = ImVec2(ImageMin.x + 190.f, ButtonRectMax.y - 5.f);

		draw_list->AddRect(ImageMin, ImageMax, ImColor(255, 255, 255, 255), 0, 15, 2.0F);
		if (map.isPreviewImageLoaded == true)
		{
			try
			{
				if (map.PreviewImage != nullptr)
				{
					if (map.PreviewImage->GetImGuiTex())
					{
						draw_list->AddImage(map.PreviewImage->GetImGuiTex(), ImageMin, ImageMax); //Map image preview
					}
				}
			}
			catch (const std::exception& ex)
			{
				cvarManager->log(ex.what());
			}
		}

		if (map.JsonFile == "NoInfos")
		{
			draw_list->AddText(fontA, 25.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 10.f), ImColor(255, 255, 255, 255),
				replace(map.Folder.filename().string(), *"_", *" ").c_str());
		}
		else
		{
			
			std::string GoodDescription = map.mapDescription;
			if (map.mapDescription.length() > 150)
			{
				GoodDescription.insert(145, "\n");

				if (map.mapDescription.length() > 280)
				{
					GoodDescription.erase(280);
					GoodDescription.append("...");
				}
			}

			draw_list->AddText(fontA, 25.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 2.f), ImColor(255, 255, 255, 255),
				map.mapName.c_str()); //Map title
			draw_list->AddText(fontA, 15.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 40.f), ImColor(200, 200, 200, 255), GoodDescription.c_str()); //Map Description
			draw_list->AddText(fontA, 15.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 90.f), ImColor(0, 200, 255, 255),
				std::string(g_pluginTexts.ResultByText.c_str() + map.mapAuthor).c_str()); // "By " Map Author
		}

		ImGui::EndGroup();

		if (ImGui::BeginPopupContextItem("Map context menu"))
		{
			if (ImGui::Selectable(g_pluginTexts.OpenMapDirText.c_str())) // "Open map directory"
			{
				std::wstring w_CurrentMapsDir = Utils::StringToWideString(map.Folder.string());
				LPCWSTR L_CurrentMapsDir = w_CurrentMapsDir.c_str();

				ShellExecute(NULL, L"open", L_CurrentMapsDir, NULL, NULL, SW_SHOWDEFAULT);
			}

			if (ImGui::Selectable(g_pluginTexts.DeleteMapText.c_str())) // "Delete Map"
			{
				fs::remove_all(map.Folder);
				RefreshMapsFunct(g_pluginConfig.MapsFolderPathBuffer);
			}
			ImGui::EndPopup();
		}
	}
}

void Pluginx64::renderMaps_DisplayMode_1(Map map, float buttonWidth)
{
	ImGui::BeginGroup();
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		if (ImGui::Button("##map", ImVec2(buttonWidth, (buttonWidth * 0.75f))))
		{
			ImGui::OpenPopup("LaunchMode");
		}
		renderLaunchModePopup(map);

		ImVec2 ButtonRectMin = ImGui::GetItemRectMin();
		ImVec2 ButtonRectMax = ImGui::GetItemRectMax();
		ImVec2 ImageMin = ImVec2(ButtonRectMin.x + 5.f, ButtonRectMin.y + 29.f);
		ImVec2 ImageMax = ImVec2(ButtonRectMax.x - 5.f, ButtonRectMax.y - 5.f);

		draw_list->AddRect(ImageMin, ImageMax, ImColor(255, 255, 255, 255), 0, 15, 2.0F);
		if (map.isPreviewImageLoaded == true)
		{
			try
			{
				if (map.PreviewImage != nullptr)
				{
					if (map.PreviewImage->GetImGuiTex())
					{
						draw_list->AddImage(map.PreviewImage->GetImGuiTex(), ImageMin, ImageMax); //Map image preview
					}
				}
			}
			catch (const std::exception& ex)
			{
				cvarManager->log(ex.what());
			}
		}

		ImFont* fontA = ImGui::GetDefaultFont();

		std::string mapTitle;
		if (map.JsonFile == "NoInfos")
		{
			mapTitle = replace(map.Folder.filename().string(), *"_", *" ");
		}
		else
		{
			mapTitle = map.mapName;
		}

		if (ImGui::CalcTextSize(mapTitle.c_str()).x > (buttonWidth * 0.808f))
		{
			mapTitle = LimitTextSize(mapTitle, (buttonWidth * 0.808f) - ImGui::CalcTextSize("...").x) + "...";
		}

		draw_list->AddText(fontA, 15.5f, ImVec2(ButtonRectMin.x + 5.f, ButtonRectMin.y + 6.f), ImColor(255, 255, 255, 255), mapTitle.c_str()); //Map title

		ImGui::EndGroup();

		if (ImGui::BeginPopupContextItem("Map context menu"))
		{
			if (ImGui::Selectable(g_pluginTexts.OpenMapDirText.c_str())) // "Open map directory"
			{
				std::wstring w_CurrentMapsDir = Utils::StringToWideString(map.Folder.string());
				LPCWSTR L_CurrentMapsDir = w_CurrentMapsDir.c_str();

				ShellExecute(NULL, L"open", L_CurrentMapsDir, NULL, NULL, SW_SHOWDEFAULT);
			}

			if (ImGui::Selectable(g_pluginTexts.DeleteMapText.c_str())) // "Delete Map"
			{
				fs::remove_all(map.Folder);
				RefreshMapsFunct(g_pluginConfig.MapsFolderPathBuffer);
			}
			ImGui::EndPopup();
		}
	}
}

void Pluginx64::renderLaunchModePopup(Map curMap)
{
	if (ImGui::BeginPopupModal("LaunchMode", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button(g_pluginTexts.StartText.c_str(), ImVec2(200.f, 50.f))) //Start
		{
			gameWrapper->Execute([&, curMap](GameWrapper* gw)
				{
					cvarManager->executeCommand("load_workshop \"" + curMap.Folder.string() + "/" + curMap.UpkFile.filename().string() + "\"");
				});

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button(g_pluginTexts.CustomGameText.c_str(), ImVec2(200.f, 50.f))) //Custom Game
		{
			ImGui::OpenPopup("CustomGame");
		}
		renderCustomGamePopup(curMap);

		CenterNexIMGUItItem(200.f);
		if (ImGui::Button(g_pluginTexts.CancelText.c_str(), ImVec2(200.f, 50.f)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderCustomGamePopup(Map curMap)
{
	if (ImGui::BeginPopupModal("CustomGame", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::BeginChild("##gameSettings", ImVec2(500.f, heightCustomGamePopup), true);
		{
			CenterNexIMGUItItem(ImGui::CalcTextSize(g_pluginTexts.GameSettingsText.c_str()).x); //Game Settings :
			ImGui::Text(g_pluginTexts.GameSettingsText.c_str()); //Game Settings :
			ImGui::NewLine();
			ImGui::Text(g_pluginTexts.GameModeText.c_str()); //Game Mode :

			if (ImGui::BeginCombo("##gameModes", gameModes->DisplayValuesNames.at(gameModes->selectedValue).c_str()))
			{
				for (auto& displayName : gameModes->DisplayValuesNames)
				{
					if (ImGui::Selectable(displayName.c_str()))
					{
						int index = std::find(gameModes->DisplayValuesNames.begin(), gameModes->DisplayValuesNames.end(), displayName) - gameModes->DisplayValuesNames.begin();
						//cvarManager->log("index : " + std::to_string(index));
						gameModes->selectedValue = index;
						cvarManager->log(gameModes->Name + " | Value : " + gameModes->GetSelectedValue());
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Text(g_pluginTexts.NBOfPlayersText.c_str());
			if (ImGui::SliderInt("##nbPlayers", &nbPlayers, 2, 8, "%d players") && nbPlayers < 2)
			{
				nbPlayers = 6;
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Mutators"))
			{
				ImGui::BeginChild("##mutators", ImVec2(ImGui::GetWindowWidth() - 10, heightMutators));
				{
					heightMutators = 410.f;
					heightCustomGamePopup = 201.f + heightMutators;

					for (auto& mutator : mutators)
					{
						//cvarManager->log(std::to_string(mutator->selectedValue));

						if (ImGui::BeginCombo(mutator->Name.c_str(), mutator->DisplayValuesNames.at(mutator->selectedValue).c_str()))
						{
							for (auto& displayName : mutator->DisplayValuesNames)
							{
								if (ImGui::Selectable(displayName.c_str()))
								{
									int index = std::find(mutator->DisplayValuesNames.begin(), mutator->DisplayValuesNames.end(), displayName) - mutator->DisplayValuesNames.begin();
									//cvarManager->log("index : " + std::to_string(index));
									mutator->selectedValue = index;
									cvarManager->log(mutator->Name + " | Value : " + mutator->GetSelectedValue());
								}
							}
							ImGui::EndCombo();
						}
					}

					if (ImGui::Button("Reset Mutators"))
					{
						for (auto& mutator : mutators)
						{
							mutator->selectedValue = 0;
						}
					}
					ImGui::EndChild();
				}
			}
			else
			{
				heightMutators = 35.f;
				heightCustomGamePopup = 197.f;
			}

			ImGui::Separator();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);

			if (ImGui::Button(g_pluginTexts.StartText.c_str(), ImVec2(100.f, 30.f))) //Start
			{
				gameWrapper->Execute([&, curMap](GameWrapper* gw)
					{
						gameWrapper->ExecuteUnrealCommand("start " + curMap.Folder.string() + "/" + curMap.UpkFile.filename().string() + "?game=" + gameModes->GetSelectedValue() + "?GameTags=BotsNone," + getMutatorsCommandString() + "?NumPublicConnections=" + std::to_string(nbPlayers) + "?NumOpenPublicConnections=" + std::to_string(nbPlayers) + "?Lan?Listen");
					});

				//cvarManager->log("Command : start " + curMap.Folder.string() + "/" + curMap.UpkFile.filename().string() + "?game=" + gameModes->GetSelectedValue() + "?GameTags=BotsNone," + getMutatorsCommandString() + "?NumPublicConnections=" + std::to_string(nbPlayers) + "?NumOpenPublicConnections=" + std::to_string(nbPlayers) + "?Lan?Listen");

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			AlignRightNexIMGUItItem(100.f, 8.f);
			if (ImGui::Button(g_pluginTexts.CancelText.c_str(), ImVec2(100.f, 30.f)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndChild();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderQuickSearch()
{
	static bool QuickSearch_FirstDisaplayed = true;
	if (isQuickSearchDisplayed)
	{
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 3.f, ImGui::GetCursorPosY() - 26.f));
		ImGui::BeginGroup();
		{
			ImGui::Text("Search :");
			ImGui::SameLine();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3.f);
			ImGui::SetNextItemWidth(230.f);
			if (QuickSearch_FirstDisaplayed)
			{
				ImGui::SetKeyboardFocusHere(0);
				QuickSearch_FirstDisaplayed = false;
			}
			ImGui::InputText("##QuickSearch", QuickSearch_KeyWordBuf, IM_ARRAYSIZE(QuickSearch_KeyWordBuf));
			ImGui::SameLine();
			if (ImGui::Selectable("X", false, 0, ImGui::CalcTextSize("X")))
			{
				strncpy(QuickSearch_KeyWordBuf, "", IM_ARRAYSIZE(QuickSearch_KeyWordBuf));
				isQuickSearchDisplayed = false;
			}

			ImGui::EndGroup();
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 22.f);
	}
	else
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16.f);
		QuickSearch_FirstDisaplayed = true;
	}
}

void Pluginx64::CenterNexIMGUItItem(float itemWidth)
{
	auto windowWidth = ImGui::GetWindowSize().x;
	ImGui::SetCursorPosX((windowWidth - itemWidth) * 0.5f);
}

void Pluginx64::AlignRightNexIMGUItItem(float itemWidth, float borderGap)
{
	auto windowWidth = ImGui::GetWindowSize().x;
	float totalWidth = itemWidth + borderGap;
	ImGui::SetCursorPosX(windowWidth - totalWidth);
}


std::string Pluginx64::LimitTextSize(std::string str, float maxTextSize)
{
	while (ImGui::CalcTextSize(str.c_str()).x > maxTextSize)
	{
		str = str.substr(0, str.size() - 1);
	}
	return str;
}

//https://gist.github.com/dougbinks/ef0962ef6ebe2cadae76c4e9f0586c69
void Pluginx64::renderUnderLine(ImColor col_)
{
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	min.y = max.y;
	ImGui::GetWindowDrawList()->AddLine(min, max, col_, 1.0f);
}

void Pluginx64::renderLink(std::string link)
{
	std::wstring w_LINK = Utils::StringToWideString(link);
	LPCWSTR L_LINK = w_LINK.c_str();

	ImGui::TextColored(ImColor(3, 94, 252, 255), link.c_str());
	renderUnderLine(ImColor(3, 94, 252, 255));
	if (ImGui::IsItemHovered())
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		if (ImGui::IsMouseClicked(0))
		{
			ShellExecute(0, 0, L_LINK, 0, 0, SW_SHOW); //open link in web browser
		}
		renderUnderLine(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
	}
}

void Pluginx64::renderImageButton(ImTextureID user_texture_id, ImVec2 size, std::function<void()> function)
{
	ImGui::BeginGroup();
	{
		ImGui::Image(user_texture_id, size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));

		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			if (ImGui::IsMouseClicked(0))
			{
				function();
			}
		}

		ImGui::EndGroup();
	}
}

void Pluginx64::renderDownloadFailedPopup()
{
	if (ImGui::BeginPopupModal("DownloadFailed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(g_pluginTexts.DownloadFailedText.c_str());
		ImGui::NewLine();
		if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
		{
			DownloadFailed = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderAcceptDownload()
{
	renderYesNoPopup("Download?", g_pluginTexts.WantToDawnloadText.c_str(), [this]() {
		AcceptTheDownload = true;
		UserIsChoosingYESorNO = false;
		ImGui::CloseCurrentPopup();
		}, [this]() {
			AcceptTheDownload = false;
			UserIsChoosingYESorNO = false;
			ImGui::CloseCurrentPopup();
		});
}


void Pluginx64::renderExtractMapFilesPopup(Map curMap)
{
	if (ImGui::BeginPopupModal("ExtractMapFiles", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string message = g_pluginTexts.EMFMessageText1 + curMap.ZipFile.filename().string() + g_pluginTexts.EMFMessageText2; //The map isn't extracted from              \nChoose an extract method (you need to click on refresh maps after extracting) :
		ImGui::Text(message.c_str());
		ImGui::NewLine();


		CenterNexIMGUItItem(326.f);
		if (ImGui::Button("Powershell", ImVec2(100.f, 25.f)))
		{
			ImGui::CloseCurrentPopup();
			std::string extractCommand = "powershell.exe Expand-Archive -LiteralPath '" + curMap.ZipFile.string() + "' -DestinationPath '" + curMap.Folder.string() + "/'";
			system(extractCommand.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button("Batch File", ImVec2(100.f, 25.f)))
		{
			ImGui::CloseCurrentPopup();
			Utils::CreateUnzipBatchFile(g_pluginConfig.BakkesmodPath, curMap.Folder.string() + "/", curMap.ZipFile.string());
		}
		ImGui::SameLine();
		if (ImGui::Button(g_pluginTexts.EMFStillDoesntWorkText.c_str(), ImVec2(110.f, 25.f)))//"Still doesn't work"
		{
			ImGui::OpenPopup("ExtractManually");
		}

		if (ImGui::BeginPopupModal("ExtractManually", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			std::string txt = g_pluginTexts.EMLabelText + curMap.ZipFile.filename().string(); //"If both of the extract methods didn't work, you need to extract the files manually of "
			ImGui::Text(txt.c_str());
			ImGui::Text("Tutorial : ");
			ImGui::SameLine();
			renderLink("https://youtu.be/mI2PqkissiQ?t=124");
			ImGui::NewLine();
			float buttonWidth = ImGui::CalcTextSize(g_pluginTexts.OpenMapDirText.c_str()).x + 8.f;
			CenterNexIMGUItItem(buttonWidth + 100.f);
			if (ImGui::Button(g_pluginTexts.OpenMapDirText.c_str(), ImVec2(buttonWidth, 25.f)))
			{
				std::wstring w_CurrentMapsDir = Utils::StringToWideString(curMap.Folder.string());
				LPCWSTR L_CurrentMapsDir = w_CurrentMapsDir.c_str();
				ShellExecute(NULL, L"open", L_CurrentMapsDir, NULL, NULL, SW_SHOWDEFAULT); //open the map directory in file explorer
			}
			ImGui::SameLine();
			if (ImGui::Button(g_pluginTexts.CancelText.c_str(), ImVec2(100.f, 25.f)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}


		CenterNexIMGUItItem(326.f);
		if (ImGui::Button(g_pluginTexts.CancelText.c_str(), ImVec2(326.f, 25.f)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderYesNoPopup(const char* popupName, const char* label, std::function<void()> yesFunc, std::function<void()> noFunc)
{
	if (ImGui::BeginPopupModal(popupName, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(label);
		ImGui::NewLine();

		CenterNexIMGUItItem(208.f);
		ImGui::BeginGroup();
		{
			if (ImGui::Button(g_pluginTexts.YESButtonText.c_str(), ImVec2(100.f, 25.f)))//YES
			{
				try
				{
					yesFunc();
				}
				catch (const std::exception& ex)
				{
					cvarManager->log(ex.what());
					ImGui::OpenPopup("PasteMap");
					renderInfoPopup("PasteMap", ex.what());
				}

			}
			ImGui::SameLine();
			if (ImGui::Button(g_pluginTexts.NOButtonText.c_str(), ImVec2(100.f, 25.f)))//NO
			{
				noFunc();
			}

			ImGui::EndGroup();
		}

		ImGui::EndPopup();
	}
}

void Pluginx64::renderFolderErrorPopup()
{
	if (ImGui::BeginPopupModal("FolderError", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Error : ");
		ImGui::Text(g_pluginTexts.FolderErrorText.c_str()); // error message
		ImGui::NewLine();

		CenterNexIMGUItItem(100.f);
		if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
		{
			FolderErrorBool = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void Pluginx64::renderInfoPopup(const char* popupName, const char* label)
{
	if (ImGui::BeginPopupModal(popupName, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(label);
		ImGui::NewLine();
		CenterNexIMGUItItem(100.f);
		if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderDownloadTexturesPopup(std::vector<std::string> missingTextureFiles)
{
	if (ImGui::BeginPopupModal("DownloadTextures", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (missingTextureFiles.size())
		{
			const std::string texturesDirectoryText = g_pluginTexts.DLTLabel1Text + RLCookedPCConsole_Path.string();
			ImGui::Text(texturesDirectoryText.c_str());
			ImGui::Text(g_pluginTexts.DLTLabel2Text.c_str());//"You can still play without the workshop textures but some maps will have some white/weird textures."
			if (IsDownloading_WorkshopTextures)
			{
				ImGui::Separator();

				std::string ProgressBar_Label = Utils::ConvertToMB(std::to_string(DownloadTextrures_ProgressDisplayed)) + " / " + Utils::ConvertToMB(std::to_string(46970000));
				Utils::RenderProgressBar(DownloadTextrures_ProgressDisplayed, 46970000.f, ImGui::GetCursorScreenPos(), ImVec2(1305.f, 24.f),
					ImColor(112, 112, 112, 255), ImColor(33, 65, 103, 255), ProgressBar_Label.c_str());

				ImGui::Separator();
			}

			ImGui::NewLine();

			float height = ((missingTextureFiles.size() - 1) * 21) + 60;

			CenterNexIMGUItItem(250.f);
			ImGui::BeginChild("##MissingFilesTable", ImVec2(250.f, height), true);
			{
				std::string MissingFilesTxt = g_pluginTexts.DLTMissingFilesText + "(" + std::to_string(missingTextureFiles.size()) + ") :";//"Missing Files :"
				CenterNexIMGUItItem(ImGui::CalcTextSize(MissingFilesTxt.c_str()).x);//"Missing Files :"
				ImGui::Text(MissingFilesTxt.c_str());//"Missing Files :"
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
				ImGui::Separator();

				if (missingTextureFiles.size() > 0)
				{
					for (auto missingFile : missingTextureFiles)
					{
						CenterNexIMGUItItem(ImGui::CalcTextSize(missingFile.c_str()).x);
						ImGui::Text(missingFile.c_str());
						ImGui::Separator();
					}
				}

				ImGui::EndChild();
			}

			ImGui::NewLine();

			ImGui::Text("In case it doesn't work when downloading, here is a tutorial to download and install them :");
			ImGui::SameLine();
			renderLink("https://youtu.be/X34TMcA0Jok?t=8");

			ImGui::NewLine();

			CenterNexIMGUItItem(258.f);
			if (ImGui::Button(g_pluginTexts.DownloadButtonText.c_str(), ImVec2(100.f, 25.f)))//"Download"
			{
				std::thread t2(&Pluginx64::DownloadWorkshopTextures, this);
				t2.detach();
			}
			ImGui::SameLine();
			if (ImGui::Button(g_pluginTexts.CloseText.c_str(), ImVec2(100.f, 25.f)))//"Close"<
			{
				DownloadTexturesBool = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button(g_pluginTexts.DontAskText.c_str(), ImVec2(150.f, 25.f)))//"Don't ask me again"
			{
				g_pluginConfig.DontAsk = 1;
				SaveInCFG();
				DownloadTexturesBool = false;
				ImGui::CloseCurrentPopup();
			}
		}
		else
		{
			ImGui::Text(g_pluginTexts.DLTTexturesInstalledText.c_str());//"Workshop textures installed !"
			ImGui::NewLine();
			CenterNexIMGUItItem(100.f);
			if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
			{
				DownloadTexturesBool = false;
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}

void Pluginx64::renderNewUpdatePopup()
{
	if (ImGui::BeginPopupModal("New Update", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static bool french = false;

		std::string Explanations1;
		std::string Explanations2;
		std::string Explanations3;
		std::string Explanations4;
		std::string Explanations5;
		std::string Explanations6;
		std::string Explanations7;

		std::string AddedText;
		std::string AddedText1;
		std::string AddedText4;

		if (!french)
		{
			Explanations1 = "Hello everyone, sorry for taking so long to make this update. Better late than never!";
			Explanations2 = "The plugin previously allowed you to browse Workshop maps through https://rocketleaguemaps.us, but the website closed and moved its database to the BakkesMod website: https://bakkesplugins.com/";
			Explanations3 = "Unfortunately, I could not bring this browsing feature back into the plugin with the new website because its owner told me he would prefer the traffic to go directly through the website for financial reasons.";
			Explanations4 = "This is completely understandable: hosting a service that is entirely free for its users has a cost, and the advertisements on the website help cover it.";
			Explanations5 = "Thanks to him for continuing to support the Rocket League modding community! And thanks to JetFox for maintaining https://rocketleaguemaps.us over the past few years.";
			Explanations6 = "This solution is not as convenient as the previous one, but I think it is the best way to keep maps easy to install.";
			Explanations7 = "You can also use the \"" + g_pluginTexts.AddMapText + "\" button to install a map manually from its .zip file.";

			AddedText = "Added :";
			AddedText1 = "- Added a new \"" + g_pluginTexts.InstallMapTab + "\" tab. You can paste a BakkesPlugins map URL to download and install it.";
			AddedText4 = "- Added back the possibility to launch a workshop map with custom settings.";
		}
		else
		{
			Explanations1 = u8"Bonjour à tous, désolé d'avoir pris tout ce temps pour faire cette mise à jour. Mieux vaut tard que jamais!";
			Explanations2 = u8"Avant, le plugin permettait de parcourir les maps workshop grace au site https://rocketleaguemaps.us, mais il a fermé et migré sa base de données vers le site de bakkesmod : https://bakkesplugins.com/.";
			Explanations3 = u8"Malheureusement, je ne pouvais pas réintégré cette fonctionnalité de parcourir les maps sur le plugin avec ce nouveau site car le détenteur m'a indiqué qu'il préfèrerait que le trafic passe directement par le site pour des raisons financières.";
			Explanations4 = u8"Ce qui est totalement compréhensible, héberger un service entièrement gratuit pour ses utilisateurs a un coût, et les publicités sur le site permettent de l'amortir.";
			Explanations5 = u8"Merci à lui de continuer à faire vivre la communauté du modding sur Rocket League! Et merci à JetFox d'avoir maintenu https://rocketleaguemaps.us pendant ces dernières années.";
			Explanations6 = u8"Cette solution n'est pas aussi pratique que celle d'avant mais je pense que c'est la meilleure qui permette de toujours installer facilement une map.";
			Explanations7 = u8"Vous pouvez aussi utiliser le bouton \"" + g_pluginTexts.AddMapText + "\" pour installer une map manuellement avec son fichier .zip.";

			AddedText = "Ajouts :";
			AddedText1 = u8"- Ajout d'un nouvel onglet \"" + g_pluginTexts.InstallMapTab + "\". Vous pouvez coller l'URL d'une map BakkesPlugins pour la télécharger et l'installer.";
			AddedText4 = u8"- Remis la possibilité de lancer une map workshop avec des settings custom.";
		}

		ImGui::NewLine();

		ImGui::Checkbox("French", &french);

		ImGui::NewLine();

		ImGui::Text(Explanations1.c_str());
		ImGui::Text(Explanations2.c_str());
		ImGui::Text(Explanations3.c_str());
		ImGui::Text(Explanations4.c_str());
		ImGui::Text(Explanations5.c_str());
		ImGui::NewLine();
		ImGui::Text(Explanations6.c_str());
		ImGui::Text(Explanations7.c_str());

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		CenterNexIMGUItItem(ImGui::CalcTextSize(std::string("Changelog v" + PluginVersion).c_str()).x);
		ImGui::Text(std::string("Changelog v" + PluginVersion).c_str());
		renderUnderLine(ImColor(255, 255, 255, 150));
		ImGui::NewLine();
		ImGui::NewLine();

		ImGui::Text(AddedText.c_str());
		renderUnderLine(ImColor(255, 255, 255, 150));
		ImGui::NewLine();
		ImGui::Text(AddedText1.c_str());
		ImGui::Text(AddedText4.c_str());

		ImGui::NewLine();

		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
		{
			g_pluginConfig.HasSeeNewUpdateAlert = true;
			SaveInCFG();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderAddMapManuallyPopup()
{
	//ImGui::SetNextWindowSize(ImVec2(600.f, 429.f));
	if (ImGui::BeginPopupModal(g_pluginTexts.AddMapText.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(g_pluginTexts.NameText.c_str()); //"Name :"
		static char name[128] = "";
		ImGui::SetNextItemWidth(300.f);
		ImGui::InputText("##Name", name, IM_ARRAYSIZE(name));

		ImGui::Text(g_pluginTexts.MapFilePathText.c_str()); //"Map File Path :"
		ImGui::SameLine();
		renderHelpMarker(g_pluginTexts.SelectZIPMapFileText.c_str()); // "Select the map file (.zip) to add manually"
		static char mapfilePath[256] = "";
		static FileExplorer addMapFileExplorer;
		ImGui::SetNextItemWidth(600.f);
		ImGui::InputText("##MapFilePath", mapfilePath, IM_ARRAYSIZE(mapfilePath));

		ImGui::SameLine();

		if (ImGui::Button(g_pluginTexts.SelectFileText.c_str())) // "Select File"
		{
			FileExplorer::Config config;
			config.showPreview = false;
			config.showNewFolderButton = false;
			config.fileFilters = { ".zip" };
			addMapFileExplorer.SetConfig(config);
			addMapFileExplorer.SetOnPathSelected([](const std::string& path, const std::string& selectedEntryPath) {
				std::string selectedMapPath = selectedEntryPath.empty() ? path : selectedEntryPath;
				strncpy(mapfilePath, selectedMapPath.c_str(), IM_ARRAYSIZE(mapfilePath) - 1);
				mapfilePath[IM_ARRAYSIZE(mapfilePath) - 1] = '\0';
			});
			addMapFileExplorer.Open();
		}
		addMapFileExplorer.Render();

		ImGui::NewLine();

		if (ImGui::Button(g_pluginTexts.CancelText.c_str(), ImVec2(100.f, 30.f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button(g_pluginTexts.ConfirmText.c_str(), ImVec2(100.f, 30.f))) //"Confirm"
		{
			if (std::string(name) == "" || !Utils::DirectoryOrFileExists(std::string(mapfilePath))) //error
			{
				cvarManager->log("ERORRRRRR");
				ImGui::OpenPopup("Add Map Error");
			}
			else
			{
				ImGui::OpenPopup("Confirm Add Map");
			}
		}
		renderYesNoPopup("Confirm Add Map", g_pluginTexts.ConfirmLabelText.c_str(), [this]() { //"Do you really want to add this map ?"
			AddMapManually(std::string(name), "", "", std::string(g_pluginConfig.MapsFolderPathBuffer), std::string(mapfilePath), "");
			ImGui::CloseCurrentPopup();
			}, [this]() {
				ImGui::CloseCurrentPopup();
			});

		renderInfoPopup("Add Map Error", g_pluginTexts.FieldEmptyText.c_str()); //"A field is empty !"

		ImGui::EndPopup();
	}
}

void Pluginx64::renderHelpMarker(const char* label)
{
	ImGui::Text("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%s", label);
		ImGui::EndTooltip();
	}
}

ImVec2 Pluginx64::CalcRealTextSize(const char* text, float fontSize)
{
	float TextSizeMultiplier = fontSize / 13.f; //13 is imgui default font size
	ImVec2 TextSize = ImGui::CalcTextSize(text);
	ImVec2 RealTextSize(TextSize.x * TextSizeMultiplier, TextSize.y * TextSizeMultiplier);
	return RealTextSize;
}

void Pluginx64::renderText(const char* text, float fontSize)
{
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImGui::BeginChild(text, CalcRealTextSize(text, fontSize));
	drawList->AddText(ImGui::GetDefaultFont(), fontSize, cursorPos, ImColor(255, 255, 255, 255), text);
	ImGui::EndChild();
}

// Name of the menu that is used to toggle the window.
std::string Pluginx64::GetMenuName()
{
	return "WorkshopMapLoaderMenu";
}

// Title to give the menu
std::string Pluginx64::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context

void Pluginx64::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}


// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool Pluginx64::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool Pluginx64::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void Pluginx64::OnOpen()
{
	isWindowOpen_ = true;

}

// Called when window is closed
void Pluginx64::OnClose()
{
	isWindowOpen_ = false;


	g_pluginConfig.MapsFolderPath = g_pluginConfig.MapsFolderPathBuffer;
}
