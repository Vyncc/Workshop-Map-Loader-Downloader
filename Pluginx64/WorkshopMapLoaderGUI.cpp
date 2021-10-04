#include "pch.h"
#include "WorkshopMapLoader.h"
#include "IMGUI/imgui_internal.h"

namespace fs = std::filesystem;


char Pluginx64::MapsFolderPathBuf[200];
bool Pluginx64::FR = false;
int Steam_SearchWorkshopDisplayed = 0;
int RLMAPS_SearchWorkshopDisplayed = 0;


const char* combo_selected_most = "Most Popular";
const char* combo_selected_period = "1 Week";
const char* combo_selected_searchingType = "Maps";
bool MostPopularSelected = false;


void Pluginx64::Render()
{
	HANDLE clip; //ClipBoard to copy and paste
	if (OpenClipboard(NULL))
	{
		clip = GetClipboardData(CF_TEXT);
		CloseClipboard();
	}



	ImGui::SetNextWindowSize(ImVec2(1320.f, 690.f));

	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_MenuBar))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	if (!FR)
	{
		//1st Tab
		Tab1MapLoaderText = "Map Loader";
		Label1Text = "Put the path of the maps folder :";
		RefreshMapsButtonText = "Refresh Maps";
		SavePathText = "Save Path";
		//context menu strip
		OpenMapDirText = "Open map directory";
		DeleteMapText = "Delete map";

		//2nd Tab
		Tab2SearchWorkshopText = "Search Workshop(Steam)";
		DlWorkshopByURLText = "Download Workshop By Url";
		Label2Text = "Steam Workshop Url :";
		DownloadButtonText = "Download";
		Label3Text = "Search A Workshop :";
		SearchButtonText = "Search";
		SearchingText = "Searching...";
		RetrievingFilesText = "Retrieving workshop files for download...";
		DownloadingText = "Downloading  :";
		WorkshopsFoundText = "Workshops Found :";

		SortByText[0] = "Sort By";
		SortByText[1] = "Most Popular";
		SortByText[2] = "Most Recent";
		SortByText[3] = "Most Subscribers";

		PeriodText[7] = "Period";
		PeriodText[0] = "Today";
		PeriodText[1] = "1 Week";
		PeriodText[2] = "1 Month";
		PeriodText[3] = "3 Months";
		PeriodText[4] = "6 Months";
		PeriodText[5] = "1 Year";
		PeriodText[6] = "Since The Begining";

		//Search Result
		ResultByText = "By ";
		ResultSizeText = "Size : ";
		DownloadMapButtonText = "Download Map";
		//context menu strip
		GoToUrlSelectableText = "Go To Url";

		//Warnings
		DirNotExistText = "The directory you entered does not exist !";
		DownloadFailedText = "Download Failed !" + DownloadFailedErrorText;
		WantToDawnloadText = "Do you really want to download?\nYou'll not be able to cancel if you start it.";
		YESButtonText = "YES";
		NOButtonText = "NO";
		IsDownloadDingWarningText = "A download is already running !\nYou cannot download 2 workshops at the same time.";
	}
	else
	{
		//1st Tab
		Tab1MapLoaderText = "Charger Map";
		Label1Text = "Mets le chemin du dossier ou se trouvent les maps :";
		RefreshMapsButtonText = "Rafraichir Les Maps";
		SavePathText = "Sauvegarder Le Chemin";

		//context menu strip
		OpenMapDirText = "Ouvrir le dossier de la map";
		DeleteMapText = "Supprimer la map";

		//2nd Tab
		Tab2SearchWorkshopText = "Rechercher Un Workshop(Steam)";
		DlWorkshopByURLText = "Telecharger Un Workshop Avec Une Url";
		Label2Text = "Url Du Workshop Steam :";
		DownloadButtonText = "Telecharger";
		Label3Text = "Rechercher Un Workshop :";
		SearchButtonText = "Rechercher";
		SearchingText = "Recherche en cours...";
		RetrievingFilesText = "Recuperation des fichiers du workshop pour le telechargement...";
		DownloadingText = "Telechargement :";
		WorkshopsFoundText = "Workshops Trouves :";

		SortByText[0] = "Trier Par";
		SortByText[1] = "Les Plus Populaires";
		SortByText[2] = "Les Plus Recents";
		SortByText[3] = "Les Plus Abonnes";

		PeriodText[7] = "Periode";
		PeriodText[0] = "Aujourd'hui";
		PeriodText[1] = "1 Semaine";
		PeriodText[2] = "1 Mois";
		PeriodText[3] = "3 Mois";
		PeriodText[4] = "6 Mois";
		PeriodText[5] = "1 An";
		PeriodText[6] = "Depuis Le Debut";

		//Search Result
		ResultByText = "Par ";
		ResultSizeText = "Taille : ";
		DownloadMapButtonText = "Telecharger La Map";
		//context menu strip
		GoToUrlSelectableText = "Aller A L'Url";

		//Warnings
		DirNotExistText = "Le chemin du dossier n'existe pas !";
		DownloadFailedText = "Le telechargement a echoue !" + DownloadFailedErrorText;
		WantToDawnloadText = "Veux-tu vraiment telecharger?\nTu ne pourras plus l'annuler si tu le commence.";
		YESButtonText = "OUI";
		NOButtonText = "NON";
		IsDownloadDingWarningText = "Un telechargement est deja en cours !\nTu ne peux pas telecharger 2 workshops en meme temps.";
	}


	if (!HasSeeNewUpdateAlert)
	{
		ImGui::OpenPopup("New Update");
	}
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("New Update", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Changelog : v1.13 :");
		ImGui::NewLine();
		ImGui::Text("-Added 2 methods to extract the files of a map just downloaded : Powershell Method, Batch File Method");
		ImGui::TextColored(ImVec4{255, 0, 0, 255}, "IMPORTANT : If when you were clicking on \"Refresh Maps\" nothing happened, now you don't need to extract the files manually anymore to make the maps appear,");
		ImGui::TextColored(ImVec4{ 255, 0, 0, 255 }, "you have to choose settings->Extract Method->Powershell Method");
		ImGui::NewLine();
		ImGui::Text("-Fixed previews that didn't correspond to the maps");
		ImGui::NewLine();
		ImGui::Text("-Fixed : the previews images don't appear when you are searching a workshop, and you can't download a map");
		ImGui::NewLine();
		ImGui::TextColored(ImVec4{ 255, 0, 0, 255 }, "If one of the fixes still doesn't work for you, let me know on discord(check Credits)");
		ImGui::NewLine();
		if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
		{
			HasSeeNewUpdateAlert = true;
			SaveInCFG(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg", MapsFolderPathBuf, std::to_string(FR), unzipMethod, std::to_string(HasSeeNewUpdateAlert));
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}


	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Settings"))
		{
			if (ImGui::BeginMenu("Extract Method"))
			{
				if (ImGui::Selectable("Batch File Method"))
				{
					unzipMethod = "Bat";

					if (Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\")) //save changes in cfg
					{
						SaveInCFG(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg", MapsFolderPathBuf, std::to_string(FR), unzipMethod, std::to_string(HasSeeNewUpdateAlert));
					}
				}

				if (unzipMethod == "Bat")
				{
					ImGui::SameLine();
					ImGui::Text(" : Selected");
				}

				if (ImGui::Selectable("Powershell Method"))
				{
					unzipMethod = "Powershell";

					if (Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\")) //save changes in cfg
					{
						SaveInCFG(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg", MapsFolderPathBuf, std::to_string(FR), unzipMethod, std::to_string(HasSeeNewUpdateAlert));
					}
				}

				if (unzipMethod == "Powershell")
				{
					ImGui::SameLine();
					ImGui::Text(" : Selected");
				}


				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Language"))
			{
				if (ImGui::Selectable("French"))
				{
					FR = true;
				}

				if (ImGui::Selectable("English"))
				{
					FR = false;
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::Selectable("LastUpdate", false, 0, ImVec2{63, 14}))
		{
			HasSeeNewUpdateAlert = false;
		}

		if (ImGui::BeginMenu("Credits"))
		{
			ImGui::Text("Plugin made by Vync#3866, contact me on discord for custom plugin commissions.");
			ImGui::NewLine();
			ImGui::Text("Thanks to PatteAuSucre for testing ;)");
			ImGui::Text("Thanks to JetFox for help");

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}


	if (ImGui::BeginTabBar("TabBar"))
	{
		if (ImGui::BeginTabItem(Tab1MapLoaderText.c_str())) // "Map Loader"
		{
			if (ImGui::Button("Test", ImVec2(100.f, 25.f)))
			{
				ImGui::OpenPopup("DownloadTextures");
			}

			if (ImGui::BeginPopupModal("DownloadTextures", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("t as pas les texture frero, telecharges les");
				if (IsDownloading_WorkshopTextures)
				{
					ImGui::Text("Progress : %d", DownloadTextrures_ProgressDisplayed);
				}
				
				std::vector<std::string> missingFiles = CheckExist_TexturesFiles();
				if (missingFiles.size() > 0)
				{
					for (auto missingFile : missingFiles)
					{
						ImGui::Text(missingFile.c_str()); //je fais ca actuellement
					}
				}

				if (ImGui::Button("Download"))
				{
					std::thread t2(&Pluginx64::DownloadWorkshopTextures, this);
					t2.detach();
				}
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);

			CenterNexIMGUItItem(ImGui::CalcTextSize(Label1Text.c_str()).x);
			ImGui::Text(Label1Text.c_str()); // "Put the folder's path of the maps, don't forget to add a  /  at the end."


			CenterNexIMGUItItem(620.f);
			ImGui::SetNextItemWidth(620.f);
			ImGui::InputText("##workshopurl123", MapsFolderPathBuf, IM_ARRAYSIZE(MapsFolderPathBuf));
			ImGui::SameLine();
			if (!Directory_Or_File_Exists(fs::path(MapsFolderPathBuf)))
			{
				ImGui::TextColored(ImVec4(255, 0, 0, 1), DirNotExistText.c_str());
			}
			else
			{
				ImGui::Text("");
			}

			CenterNexIMGUItItem(620.f); //306.f("Save Path" button) + 8.f(the gap between both buttons) + 306.f("Refresh Mas" button) = 620.f  (cause they are on same line)

			if (ImGui::Button(SavePathText.c_str(), ImVec2(306.f, 32.f))) // "Save Path"
			{
				if (Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\"))
				{
					SaveInCFG(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg", MapsFolderPathBuf, std::to_string(FR), unzipMethod, std::to_string(HasSeeNewUpdateAlert));
				}
			}
			ImGui::CalcItemWidth();

			ImGui::SameLine();

			if (ImGui::Button(RefreshMapsButtonText.c_str(), ImVec2(306.f, 32.f))) // "Refresh Maps"
			{
				if (!Directory_Or_File_Exists(fs::path(MapsFolderPathBuf)))
				{
					ImGui::OpenPopup("Exists?");
				}
				else
				{
					RefreshMapsFunct(MapsFolderPathBuf);
				}
			}
			renderInfoPopup("Exists?", DirNotExistText.c_str());


			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);

			renderMaps();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(Tab2SearchWorkshopText.c_str())) //"Search Workshop"
		{
			if (ImGui::CollapsingHeader(DlWorkshopByURLText.c_str())) // "Download Workshop By Url"
			{
				ImGui::Text(Label2Text.c_str()); // "Steam Workshop Url :"
				static char url[200] = "Ex : https://steamcommunity.com/sharedfiles/filedetails/?id=2120912805&searchtext=rings+3";
				ImGui::InputText("##STEAMworkshopurl", url, IM_ARRAYSIZE(url));
				if (ImGui::Button(DownloadButtonText.c_str())) // "Download"
				{
					std::thread t1(&Pluginx64::STEAM_DownloadWorkshop, this, url, MapsFolderPathBuf, "", true, 0, false);
					t1.detach();
				}
			}

			ImGui::Separator();



			ImGui::BeginGroup();
			{
				ImGui::Text(Label3Text.c_str()); // "Search A Workshop :"

				ImGui::SetNextItemWidth(300.f);
				static char keyWord[200] = "";
				ImGui::InputText("##STEAMworkshopkeyword", keyWord, IM_ARRAYSIZE(keyWord));
				std::string get_full_url = steam_base_url + replace(std::string(keyWord), *" ", *"+");
				//ImGui::Text(get_full_url.c_str());

				if (STEAM_Searching)
				{
					SearchButtonText = SearchingText;
				}

				if (ImGui::Button(SearchButtonText.c_str(), ImVec2(300.f, 25.f)) && !STEAM_Searching) // "Search"
				{
					std::thread t2(&Pluginx64::StartSearchRequest, this, get_full_url);
					t2.detach();

					STEAM_browsing = false;
					MostPopularSelected = false;
				}

				ImGui::EndGroup();
			}

			ImGui::SameLine();


			CenterNexIMGUItItem(63.f);
			ImGui::Image(SteamLogoImage->GetImGuiTex(), ImVec2(63.f, 63.f)); //Steam Logo

			ImGui::SameLine();

			ImGui::BeginGroup();
			{
				std::string MostPopular_Url = "https://steamcommunity.com/workshop/browse/?appid=252950&browsesort=trend&section=readytouseitems";

				renderSortByCombos(MostPopular_Url);
				
				ImGui::SameLine();

				if (ImGui::Button("Browse Maps", ImVec2(180.f, 65.f)) && !STEAM_Searching)
				{
					std::thread t2(&Pluginx64::StartSearchRequest, this, MostPopular_Url);
					t2.detach();

					combo_selected_most = SortByText[1].c_str(); //"Most Popular"
					MostPopularSelected = true;
					STEAM_browsing = true;
				}

				ImGui::EndGroup();
			}
			



			if (FolderErrorBool)
			{
				ImGui::OpenPopup("FolderError");
			}
			renderFolderErrorPopup();


			if (DownloadFailed)
			{
				ImGui::OpenPopup("DownloadFailed");
			}
			renderDownloadFailedPopup();


			if (IsRetrievingWorkshopFiles == true)
			{
				ImGui::Separator();
				ImGui::TextColored(ImVec4(255, 155, 0, 1), RetrievingFilesText.c_str()); // "Retrieving workshop files for download..."
			}

			if (UserIsChoosingYESorNO)
			{
				ImGui::OpenPopup("Download?");
			}
			renderAcceptDownload();



			if (STEAM_IsDownloadingWorkshop == true)
			{
				/*
				ImGui::TextColored(ImVec4(0, 255, 0, 1), "%s %s / %s", DownloadingText.c_str(), convertToMB(std::to_string(STEAM_WorkshopDownload_ProgressString)).c_str(),
					convertToMB(std::to_string(STEAM_WorkshopDownload_FileSizeString)).c_str()); // "Downloading : 0 MB / 0 MB"
				*/

				ImGui::Separator();

				std::string ProgressBar_Label = convertToMB(std::to_string(STEAM_WorkshopDownload_Progress)) + " / " + convertToMB(std::to_string(STEAM_WorkshopDownload_FileSize));
				renderProgressBar(STEAM_WorkshopDownload_Progress, STEAM_WorkshopDownload_FileSize, ImGui::GetCursorScreenPos(), ImVec2(1305.f, 24.f),
								  ImColor(112, 112, 112, 255), ImColor(33, 65, 103, 255), ProgressBar_Label.c_str());
			}


			/*
			float ProgressPercent = (100.f * STEAM_WorkshopDownload_ProgressString) / STEAM_WorkshopDownload_FileSizeString;
			ImGui::Text("percent : %f", ProgressPercent);
			*/

			/*
			ImGui::Separator();
			
			ImGui::SliderInt("width", &widthTest, -1920, 1920);
			ImGui::SliderInt("height", &heightTest, -300, 300);
			*/
			

			ImGui::Separator();

			ImGui::BeginChild("##SteamSearchWorkshopMapsResults");
			{
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.f);
				ImGui::Text("%s %d / %d", WorkshopsFoundText.c_str(), Steam_SearchWorkshopDisplayed, STEAM_NumberOfMapsFound); // "Workshops Found : 0 / 0"

				ImGui::SameLine();

				AlignRightNexIMGUItItem((55.f + 8.f) * OtherPagesList.size(), 27.f); //55.f(width of one buttonPage)
				ImGui::BeginGroup();
				{
					for (int i = 0; i < OtherPagesList.size(); i++)
					{
						std::string PageButtonName = "Page " + FindAllSubstringInAString(OtherPagesList.at(i), "&p=", "&days").at(0);

						if (ImGui::Button(PageButtonName.c_str(), ImVec2(55.f, 25.f)) && STEAM_Searching == false)
						{
							std::thread Page_Thread(&Pluginx64::StartSearchRequest, this, OtherPagesList.at(i));
							Page_Thread.detach();
						}
						ImGui::SameLine();
					}
					ImGui::EndGroup();
				}

				ImGui::NewLine();
				ImGui::NewLine();

				Steam_renderSearchWorkshopResults(MapsFolderPathBuf);

				ImGui::EndChild();
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Search Workshop(rocketleaguemaps.us)"))
		{
			ImGui::BeginGroup();
			{
				ImGui::Text(Label3Text.c_str()); // "Search A Workshop :"

				ImGui::SetNextItemWidth(200.f);
				static char keyWord[200] = "";
				ImGui::InputText("##RLMAPSworkshopkeyword", keyWord, IM_ARRAYSIZE(keyWord));


				ImGui::SameLine();
				ImGui::SetNextItemWidth(100.f);
				if (ImGui::BeginCombo("##searchingType", combo_selected_searchingType))
				{
					if (ImGui::Selectable("Creator"))
					{
						combo_selected_searchingType = "Creator";
					}

					if (ImGui::Selectable("Maps"))
					{
						combo_selected_searchingType = "Maps";
					}
					ImGui::EndCombo();
				}

				if (RLMAPS_Searching)
				{
					SearchButtonText = SearchingText;
				}

				if (ImGui::Button(SearchButtonText.c_str(), ImVec2(308.f, 25.f)) && !RLMAPS_Searching) // "Search"
				{
					std::thread t2(&Pluginx64::GetResults, this, std::string(combo_selected_searchingType), std::string(keyWord));
					t2.detach();

					RLMAPS_browsing = false;
				}
				ImGui::EndGroup();
			}

			ImGui::SameLine();

			CenterNexIMGUItItem(63.f);
			ImGui::Image(RLMAPSLogoImage->GetImGuiTex(), ImVec2(63.f, 63.f)); //rocketleaguemaps.us Logo

			ImGui::SameLine();

			AlignRightNexIMGUItItem(180.f, 8.f);
			if (ImGui::Button("Browse Maps", ImVec2(180.f, 65.f)) && !RLMAPS_Searching)
			{
				CurrentPage = 0;
				std::thread t2(&Pluginx64::GetResultsBrowseMaps, this, CurrentPage * 20);
				t2.detach();

				RLMAPS_browsing = true;
			}


			if (FolderErrorBool)
			{
				ImGui::OpenPopup("FolderError");
			}
			renderFolderErrorPopup();


			if (DownloadFailed)
			{
				ImGui::OpenPopup("DownloadFailed");
			}
			renderDownloadFailedPopup();


			if (UserIsChoosingYESorNO)
			{
				ImGui::OpenPopup("Download?");
			}
			renderAcceptDownload();


			if (RLMAPS_IsDownloadingWorkshop == true)
			{
				/*
				ImGui::TextColored(ImVec4(0, 255, 0, 1), "%s %s / %s", DownloadingText.c_str(), convertToMB(std::to_string(RLMAPS_WorkshopDownload_ProgressString)).c_str(),
					convertToMB(std::to_string(RLMAPS_WorkshopDownload_FileSizeString)).c_str()); // "Downloading : 0 MB / 0 MB"
				*/

				ImGui::Separator();

				std::string ProgressBar_Label = convertToMB(std::to_string(RLMAPS_WorkshopDownload_Progress)) + " / " + convertToMB(std::to_string(RLMAPS_WorkshopDownload_FileSize));
				renderProgressBar(RLMAPS_WorkshopDownload_Progress, RLMAPS_WorkshopDownload_FileSize, ImGui::GetCursorScreenPos(), ImVec2(1305.f, 24.f),
					ImColor(112, 112, 112, 255), ImColor(33, 65, 103, 255), ProgressBar_Label.c_str());
			}

			ImGui::Separator();

			ImGui::BeginChild("##RLMAPSSearchWorkshopMapsResults");
			{
				//ImGui::Text("Page : %d", CurrentPage);


				ImGui::Text("%s %d / %d", WorkshopsFoundText.c_str(), RLMAPS_SearchWorkshopDisplayed, RLMAPS_NumberOfMapsFound); // "Workshops Found : 0 / 0"

				ImGui::SameLine();

				if (RLMAPS_browsing)
				{
					ImGui::BeginGroup();
					{
						if (CurrentPage > 0)
						{
							if (ImGui::Button("Previous Page", ImVec2(100.f, 25.f)) && !RLMAPS_Searching)
							{
								CurrentPage -= 1;
								std::thread t2(&Pluginx64::GetResultsBrowseMaps, this, CurrentPage * 20);
								t2.detach();
							}
						}

						ImGui::SameLine();

						//faudra que je rajoute un if qui verifie si CurrentPage < nbTotalDePage
						if (ImGui::Button("Next Page", ImVec2(100.f, 25.f)) && !RLMAPS_Searching)
						{
							CurrentPage++;
							std::thread t2(&Pluginx64::GetResultsBrowseMaps, this, CurrentPage * 20);
							t2.detach();
						}

						ImGui::EndGroup();
					}
				}
				ImGui::NewLine();
				ImGui::NewLine();
				RLMAPS_renderSearchWorkshopResults(MapsFolderPathBuf);

				ImGui::EndChild();
			}
			
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


void Pluginx64::renderProgressBar(float value, float maxValue, ImVec2 pos, ImVec2 size, ImColor colorBackground, ImColor colorProgress, const char* label)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	float windowWidth = ImGui::GetWindowWidth();
	auto textWidth = ImGui::CalcTextSize(label).x;
	float percent = ((value * 100) / maxValue) / 100.f;

	ImGui::BeginChild("##ProgessBar", size);
	{
		draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), colorBackground, 15.f, 15.f);
		if (value != 0)
		{
			draw_list->AddRectFilled(pos, ImVec2(pos.x + (percent * size.x), pos.y + size.y), colorProgress, 15.f, 15.f);
		}
		draw_list->AddText(ImVec2(ImGui::GetCursorScreenPos().x + (windowWidth - textWidth) * 0.5f, ImGui::GetCursorScreenPos().y + 5.f), ImColor(255, 255, 255, 255), label);

		ImGui::EndChild();
	}
}



void Pluginx64::renderMaps()
{
	std::vector<Map> NoUpk_MapList;
	std::vector<Map> Good_MapList;

	int ID = 0;

	if (ImGui::BeginChild("#MapsLauncherButtons"))
	{
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
						draw_list->AddImage(curMap.PreviewImage->GetImGuiTex(), ImageMin, ImageMax); //Map image preview
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
					mapName = GetJSONLocalMapInfos(curMap.JsonFile).at(0);
				}


				draw_list->AddText(fontA, 25.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 2.f), ImColor(255, 255, 255, 255),
					mapName.c_str());

				draw_list->AddText(fontA, 25.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 40.f), ImColor(255, 0, 0, 255),
					"This map wont work because the map isn't extracted, click to fix.");

				ImGui::EndGroup();

				if (ImGui::BeginPopupContextItem("Map context menu")) //faudrait que je change le nom (que je mette le nom de la map ou jsp
				{
					if (ImGui::Selectable(OpenMapDirText.c_str())) // "Open map directory"
					{
						std::wstring w_CurrentMapsDir = s2ws(curMap.Folder.string());
						LPCWSTR L_CurrentMapsDir = w_CurrentMapsDir.c_str();

						ShellExecute(NULL, L"open", L_CurrentMapsDir, NULL, NULL, SW_SHOWDEFAULT);
					}

					if (ImGui::Selectable(DeleteMapText.c_str())) // "Delete Map"
					{
						MapList.clear();
						fs::remove_all(curMap.Folder);
						RefreshMapsFunct(MapsFolderPathBuf);
					}
					ImGui::EndPopup();
				}
			}

			ImGui::PopID();
			ID++;
		}


		for (auto curMap : Good_MapList)
		{
			ImGui::PushID(ID); //needed to make the button work
			ImGui::BeginGroup();
			{
				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				ImFont* fontA = ImGui::GetDefaultFont();


				if (ImGui::Button("##map", ImVec2(ImGui::GetWindowWidth(), 120)))
				{
					ImGui::OpenPopup("LaunchMode");
				}
				renderLaunchModePopup(curMap);


				ImVec2 ButtonRectMin = ImGui::GetItemRectMin();
				ImVec2 ButtonRectMax = ImGui::GetItemRectMax();
				ImVec2 ImageMin = ImVec2(ButtonRectMin.x + 5.f, ButtonRectMin.y + 5.f);
				ImVec2 ImageMax = ImVec2(ImageMin.x + 190.f, ButtonRectMax.y - 5.f);


				draw_list->AddRect(ImageMin, ImageMax, ImColor(255, 255, 255, 255), 0, 15, 2.0F);
				if (curMap.isPreviewImageLoaded == true)
				{
					try
					{
						draw_list->AddImage(curMap.PreviewImage->GetImGuiTex(), ImageMin, ImageMax); //Map image preview
					}
					catch (const std::exception& ex)
					{
						cvarManager->log(ex.what());
					}
				}

				if (curMap.JsonFile == "NoInfos")
				{
					draw_list->AddText(fontA, 25.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 10.f), ImColor(255, 255, 255, 255),
						replace(curMap.Folder.filename().string(), *"_", *" ").c_str());
				}
				else
				{
					std::string GoodDescription = GetJSONLocalMapInfos(curMap.JsonFile).at(1);
					if (GetJSONLocalMapInfos(curMap.JsonFile).at(1).length() > 150)
					{
						GoodDescription.insert(145, "\n");

						if (GetJSONLocalMapInfos(curMap.JsonFile).at(1).length() > 280)
						{
							GoodDescription.erase(280);
							GoodDescription.append("...");
						}
					}

					draw_list->AddText(fontA, 25.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 2.f), ImColor(255, 255, 255, 255),
						GetJSONLocalMapInfos(curMap.JsonFile).at(0).c_str()); //Map title
					draw_list->AddText(fontA, 15.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 40.f), ImColor(200, 200, 200, 255), GoodDescription.c_str()); //Map Description
					draw_list->AddText(fontA, 15.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 90.f), ImColor(0, 200, 255, 255),
						std::string(ResultByText.c_str() + GetJSONLocalMapInfos(curMap.JsonFile).at(2)).c_str()); // "By " Map Author
				}

				ImGui::EndGroup();

				if (ImGui::BeginPopupContextItem("Map context menu")) //faudrait que je change le nom (que je mette le nom de la map ou jsp
				{
					if (ImGui::Selectable(OpenMapDirText.c_str())) // "Open map directory"
					{
						std::wstring w_CurrentMapsDir = s2ws(curMap.Folder.string());
						LPCWSTR L_CurrentMapsDir = w_CurrentMapsDir.c_str();

						ShellExecute(NULL, L"open", L_CurrentMapsDir, NULL, NULL, SW_SHOWDEFAULT);
					}

					if (ImGui::Selectable(DeleteMapText.c_str())) // "Delete Map"
					{
						fs::remove_all(curMap.Folder);
						RefreshMapsFunct(MapsFolderPathBuf);
					}
					ImGui::EndPopup();
				}
			}

			ImGui::PopID();
			ID++;
		}
	}
	ImGui::EndChild();
}


//https://stackoverflow.com/questions/64653747/how-to-center-align-text-horizontally
void Pluginx64::TextCenter(std::string text) {
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	ImGui::Text(text.c_str());
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


void Pluginx64::renderSortByCombos(std::string mostPopular_url)
{
	AlignRightNexIMGUItItem(widthBrowseGroup, 8.f);
	ImGui::BeginGroup();
	{
		if (STEAM_browsing)
		{
			widthBrowseGroup = 385.f; //180(browse button) + 8(gap between 2 imgui component) + 142(combo) + 41(Sort By :) + 8(gap between item and right window border)

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.f);
			ImGui::Text("%s :", SortByText[0].c_str()); // "Sort By :"
			//ImGui::Text("width of sort by : %f", ImGui::CalcTextSize(SortByText[0].c_str()).x);
			ImGui::SameLine();

			ImGui::SetNextItemWidth(142.f);
			if (ImGui::BeginCombo("##comboMost", combo_selected_most))
			{
				if (ImGui::Selectable(SortByText[1].c_str())) // "Most Popular"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, mostPopular_url);
					t6.detach();
					combo_selected_most = SortByText[1].c_str(); // "Most Popular"
					MostPopularSelected = true;
				}

				if (ImGui::Selectable(SortByText[2].c_str())) // "Most Recent"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, "https://steamcommunity.com/workshop/browse/?appid=252950&browsesort=mostrecent&section=readytouseitems&actualsort=mostrecent&p=1");
					t6.detach();
					combo_selected_most = SortByText[2].c_str(); // "Most Recent"
					MostPopularSelected = false;
				}

				if (ImGui::Selectable(SortByText[3].c_str())) // "Most Subscribers"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, "https://steamcommunity.com/workshop/browse/?appid=252950&browsesort=totaluniquesubscribers&section=readytouseitems&actualsort=totaluniquesubscribers&p=1");
					t6.detach();
					combo_selected_most = SortByText[3].c_str(); // "Most Subscribers"
					MostPopularSelected = false;
				}
				ImGui::EndCombo();
			}
		}
		else
		{
			widthBrowseGroup = 186.f;
		}



		if (MostPopularSelected)
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.f);
			ImGui::Text("%s : ", PeriodText[7].c_str()); // "Period :"
			ImGui::SameLine();


			ImGui::SetNextItemWidth(142.f);
			if (ImGui::BeginCombo("##comboPeriod", combo_selected_period))
			{
				if (ImGui::Selectable(PeriodText[0].c_str())) // "Today"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, mostPopular_url + "&actualsort=trend&p=1&days=1");
					t6.detach();
					combo_selected_period = PeriodText[0].c_str(); // "Today"
				}

				if (ImGui::Selectable(PeriodText[1].c_str())) // "1 Week"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, mostPopular_url + "&actualsort=trend&p=1&days=7");
					t6.detach();
					combo_selected_period = PeriodText[1].c_str(); // "1 Week"
				}

				if (ImGui::Selectable(PeriodText[2].c_str())) // "1 Months"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, mostPopular_url + "&actualsort=trend&p=1&days=30");
					t6.detach();
					combo_selected_period = PeriodText[2].c_str(); // "1 Months"
				}

				if (ImGui::Selectable(PeriodText[3].c_str())) // "3 Months"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, mostPopular_url + "&actualsort=trend&p=1&days=90");
					t6.detach();
					combo_selected_period = PeriodText[3].c_str(); // "3 Months"
				}

				if (ImGui::Selectable(PeriodText[4].c_str())) // "6 Months"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, mostPopular_url + "&actualsort=trend&p=1&days=180");
					t6.detach();
					combo_selected_period = PeriodText[4].c_str(); // "6 Months"
				}

				if (ImGui::Selectable(PeriodText[5].c_str())) // "1 Year"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, mostPopular_url + "&actualsort=trend&p=1&days=365");
					t6.detach();
					combo_selected_period = PeriodText[5].c_str(); // "1 Year"
				}

				if (ImGui::Selectable(PeriodText[6].c_str())) // "Since the begining"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, mostPopular_url + "&actualsort=trend&p=1&days=-1");
					t6.detach();
					combo_selected_period = PeriodText[6].c_str(); // "Since the begining"
				}
				ImGui::EndCombo();
			}
		}
		ImGui::EndGroup();
	}
}

void Pluginx64::renderDownloadFailedPopup()
{
	if (ImGui::BeginPopupModal("DownloadFailed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(DownloadFailedText.c_str());
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
	renderYesNoPopup("Download?", WantToDawnloadText.c_str(), [this]() {
		AcceptTheDownload = true;
		UserIsChoosingYESorNO = false;
		ImGui::CloseCurrentPopup();
		}, [this]() {
			AcceptTheDownload = false;
			UserIsChoosingYESorNO = false;
			ImGui::CloseCurrentPopup();
		});
}

void Pluginx64::renderLaunchModePopup(Map curMap)
{
	if (ImGui::BeginPopupModal("LaunchMode", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("Solo", ImVec2(200.f, 50.f)))
		{
			gameWrapper->Execute([&, curMap](GameWrapper* gw)
				{
					cvarManager->executeCommand("load_workshop \"" + curMap.Folder.string() + "/" + curMap.UpkFile.filename().string() + "\"");
				});
			cvarManager->log("Map selected : " + curMap.UpkFile.filename().string());

			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Host Multiplayer Server", ImVec2(200.f, 50.f)))
		{
			gameWrapper->Execute([&, curMap](GameWrapper* gw)
				{
					//cvarManager->executeCommand("unreal_command \"start C:\\Users\\snipj\\AppData\\Roaming\\bakkesmod\\bakkesmod\\maps\\dribble_2_overhaul\\DribbleChallenge2Overhaul.upk?game=TAGame.GameInfo_Soccar_TA?GameTag=FiveMinutes,BotsNone,UnlimitedBoost,PlayerCount8?NumPublicConnections=10?NumOpenPublicConnections=10?Lan?Listen\"");
					gameWrapper->ExecuteUnrealCommand("start " + curMap.Folder.string() + "/" + curMap.UpkFile.filename().string() + "?game=TAGame.GameInfo_Soccar_TA?GameTags=FiveMinutes,BotsNone,UnlimitedBoost,PlayerCount8?NumPublicConnections=10?NumOpenPublicConnections=10?Lan?Listen");
				});
			ImGui::CloseCurrentPopup();
		}


		if (ImGui::Button("Join Server", ImVec2(200.f, 50.f)))
		{
			ImGui::OpenPopup("JoinServerPopup");
		}


		std::string modsDirPath = RLCookedPCConsole_Path.string() + "\\mods";
		if (!Directory_Or_File_Exists(modsDirPath))
		{
			renderYesNoPopup("JoinServerPopup", "The directory \"mods\" doesn't exist. Create it?", [this, modsDirPath]() {
				fs::create_directory(modsDirPath);
				}, [this]() {ImGui::CloseCurrentPopup(); });
		}
		else if (!Directory_Or_File_Exists(modsDirPath + "\\" + curMap.UpkFile.filename().string()))
		{
			renderYesNoPopup("JoinServerPopup", std::string(curMap.UpkFile.filename().string() + " isn't in mods/. Paste the map to mods/ ?\n/!\\You need to restart the game begore to be able to join the server !").c_str(), [this, modsDirPath, curMap]() {
				fs::copy(curMap.UpkFile, modsDirPath);
				}, [this]() {ImGui::CloseCurrentPopup(); });
		}
		else
		{
			if (ImGui::BeginPopupModal("JoinServerPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);
				ImGui::Text("IP :");
				ImGui::SameLine();
				static char IP[200] = "";
				ImGui::InputText("##inputIP", IP, IM_ARRAYSIZE(IP));

				ImGui::Text("PORT :");
				ImGui::SameLine();
				static char PORT[200] = "";
				ImGui::InputText("##inputPORT", PORT, IM_ARRAYSIZE(PORT));

				std::string str_IP = std::string(IP);
				std::string str_PORT = std::string(PORT);


				CenterNexIMGUItItem(208.f);
				ImGui::BeginGroup();
				{
					if (ImGui::Button("Join Server", ImVec2(100.f, 25.f)))
					{
						gameWrapper->Execute([&, str_IP, str_PORT](GameWrapper* gw)
							{
								cvarManager->log("IP : " + str_IP);
								cvarManager->log("PORT : " + str_PORT);
								gameWrapper->ExecuteUnrealCommand("start " + str_IP + ":" + str_PORT + "/?Lan?Password=password");
							});

						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();

					if (ImGui::Button("Cancel", ImVec2(100.f, 25.f)))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndGroup();
				}
				ImGui::EndPopup();
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(200.f, 50.f)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderExtractMapFilesPopup(Map curMap)
{
	if (ImGui::BeginPopupModal("ExtractMapFiles", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string message = "The map isn't extracted from " + curMap.ZipFile.filename().string() + "\nChoose an extract method (you need to click on refresh maps after extracting) :";
		ImGui::Text(message.c_str());
		ImGui::NewLine();


		CenterNexIMGUItItem(326.f);
		if (ImGui::Button("Powershell", ImVec2(100.f, 25.f)))
		{
			ImGui::CloseCurrentPopup();
			std::string extractCommand = "powershell.exe Expand-Archive -LiteralPath " + curMap.ZipFile.string() + " -DestinationPath " + curMap.Folder.string() + "/";
			system(extractCommand.c_str());
		}
		ImGui::SameLine();
		if (ImGui::Button("Batch File", ImVec2(100.f, 25.f)))
		{
			ImGui::CloseCurrentPopup();
			CreateUnzipBatchFile(curMap.Folder.string() + "/", curMap.ZipFile.string());
		}
		ImGui::SameLine();
		if (ImGui::Button("Still doesn't work", ImVec2(110.f, 25.f)))
		{
			ImGui::OpenPopup("ExtractManually");
		}

		if (ImGui::BeginPopupModal("ExtractManually", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			std::string txt = "If both of the extract methods didn't work, you need to extract the files manually of %s" + curMap.ZipFile.filename().string();
			ImGui::Text(txt.c_str());
			ImGui::Text("Tutorial : ");
			ImGui::SameLine();
			ImGui::TextColored(ImColor(3, 94, 252, 255), "https://youtu.be/mI2PqkissiQ?t=124");
			renderUnderLine(ImColor(3, 94, 252, 255));
			if (ImGui::IsItemHovered())
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				if (ImGui::IsMouseClicked(0))
				{
					ShellExecute(0, 0, L"https://youtu.be/mI2PqkissiQ?t=124", 0, 0, SW_SHOW); //open the video tutorial in browser
				}
				renderUnderLine(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
			}
			ImGui::NewLine();
			float buttonWidth = ImGui::CalcTextSize(OpenMapDirText.c_str()).x + 8.f;
			CenterNexIMGUItItem(buttonWidth + 100.f);
			if (ImGui::Button(OpenMapDirText.c_str(), ImVec2(buttonWidth, 25.f)))
			{
				std::wstring w_CurrentMapsDir = s2ws(curMap.Folder.string());
				LPCWSTR L_CurrentMapsDir = w_CurrentMapsDir.c_str();
				ShellExecute(NULL, L"open", L_CurrentMapsDir, NULL, NULL, SW_SHOWDEFAULT); //open the map directory in file explorer
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(100.f, 25.f)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}


		CenterNexIMGUItItem(326.f);
		if (ImGui::Button("Cancel", ImVec2(326.f, 25.f)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderUnderLine(ImColor col_)
{
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	min.y = max.y;
	ImGui::GetWindowDrawList()->AddLine(min, max, col_, 1.0f);
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
			if (ImGui::Button("YES", ImVec2(100.f, 25.f)))
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
			if (ImGui::Button("NO", ImVec2(100.f, 25.f)))
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
		ImGui::Text(FolderErrorText.c_str()); // error message
		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::Text("if the error message is an acces denied, it means the maps folder need administrator acces,");
		ImGui::Text("so you need to change the location of the maps folder to a place that doesn't need it (ex : desktop)");
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


void Pluginx64::Steam_renderSearchWorkshopResults(static char mapspath[200])
{
	int LinesNb = 0;
	Steam_SearchWorkshopDisplayed = 0;


	ImDrawList* draw_list = ImGui::GetWindowDrawList();


	for (int i = 0; i < Steam_MapResultList.size(); i++)
	{
		if (LinesNb < 4)
		{
			ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();

			Steam_RenderAResult(i, draw_list, mapspath);

			ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 270.f, TopCornerLeft.y)); //set the cursor next to the result item, the float is the spacing
			LinesNb++;
		}
		else
		{
			ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();

			Steam_RenderAResult(i, draw_list, mapspath);

			ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, TopCornerLeft.y + 280.f)); //set the cursor to a new line, the float is the spacing
			LinesNb = 0;
		}
			
		Steam_SearchWorkshopDisplayed++;
	}
}


void Pluginx64::Steam_RenderAResult(int i, ImDrawList* drawList, static char mapspath[200])
{
	ImGui::PushID(i);

	Steam_MapResult mapResult = Steam_MapResultList.at(i);
	std::string mapName = mapResult.Name;
	std::string mapID = mapResult.ID;
	std::string mapDescription = mapResult.Description;
	std::string mapSize = mapResult.Size;
	std::string mapAuthor = mapResult.Author;

	//Popup if is a downlaod is in progress and user wants to start a new download
	renderInfoPopup("Downloading?", IsDownloadDingWarningText.c_str());

	//Popup if maps directory doesn't exist
	renderInfoPopup("Exists?", DirNotExistText.c_str());


	ImGui::BeginGroup();
	{
		std::string SizeConverted = ResultSizeText + convertToMB(mapSize);

		ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
		ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + 190.f, TopCornerLeft.y + 260.f);
		ImVec2 ImageP_Min = ImVec2(TopCornerLeft.x + 6.f, TopCornerLeft.y + 6.f);
		ImVec2 ImageP_Max = ImVec2(TopCornerLeft.x + 184.f, TopCornerLeft.y + 179.f);

		drawList->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(44, 75, 113, 255), 5.f, 15); //Blue rectangle result
		drawList->AddRect(ImageP_Min, ImageP_Max, ImColor(255, 255, 255, 255), 0, 15, 2.0F); //Image white outline

		if (mapResult.isImageLoaded == true)
		{
			drawList->AddImage(mapResult.Image->GetImGuiTex(), ImageP_Min, ImageP_Max); //Map image preview
		}

		std::string GoodMapName = mapName.substr(0, 29);
		if (mapName.length() > 31)
		{
			GoodMapName.append("...");
		}
		drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 185.f), ImColor(255, 255, 255, 255), GoodMapName.c_str()); //Map title
		drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 200.f), ImColor(255, 255, 255, 255), SizeConverted.c_str()); //Map size
		drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 215.f), ImColor(255, 255, 255, 255),
			std::string(ResultByText.c_str() + mapAuthor).c_str()); // "By : " Map Author
		ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 235.f));
		if (ImGui::Button(DownloadMapButtonText.c_str(), ImVec2(182, 20))) // "Download Map"
		{
			if (STEAM_IsDownloadingWorkshop == false && IsRetrievingWorkshopFiles == false && Directory_Or_File_Exists(fs::path(mapspath)))
			{
				std::thread t2(&Pluginx64::STEAM_DownloadWorkshop, this, "", mapspath, mapResult.ID, false, i, true);
				t2.detach();
			}
			else
			{
				if (!Directory_Or_File_Exists(fs::path(mapspath)))
				{
					ImGui::OpenPopup("Exists?");
				}

				if (STEAM_IsDownloadingWorkshop || IsRetrievingWorkshopFiles)
				{
					ImGui::OpenPopup("Downloading?");
				}
			}
		}
		ImGui::EndGroup();

		if (ImGui::IsItemHovered())
		{
			std::string GoodDescription = mapDescription;

			if (mapDescription.length() > 150)
			{
				GoodDescription.insert(150, "\n");

				if (mapDescription.length() > 280)
				{
					GoodDescription.erase(280);
					GoodDescription.append("...");
				}
			}


			ImGui::BeginTooltip();
			ImGui::Text("Title : %s", mapName.c_str());
			ImGui::Text(SizeConverted.c_str());
			ImGui::Text("By : %s", mapAuthor.c_str());
			ImGui::Text("Description : \n%s", GoodDescription.c_str());
			ImGui::EndTooltip();
		}
		if (ImGui::BeginPopupContextItem("SearchResult context menu"))
		{
			if (ImGui::Selectable(GoToUrlSelectableText.c_str())) // "go to url"
			{
				std::wstring w_Workshop_URL = s2ws("https://steamcommunity.com/sharedfiles/filedetails/?id=" + mapID);
				LPCWSTR L_URL = w_Workshop_URL.c_str();
				ShellExecute(NULL, L"open", L_URL, NULL, NULL, SW_SHOWNORMAL); //open web browser at workshop url
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
}






void Pluginx64::RLMAPS_renderSearchWorkshopResults(static char mapspath[200])
{
	int LinesNb = 0;
	RLMAPS_SearchWorkshopDisplayed = 0;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();


	for (int i = 0; i < RLMAPS_MapResultList.size(); i++)
	{
		if (LinesNb < 4)
		{
			ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();

			RLMAPS_RenderAResult(i, draw_list, mapspath);

			ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 270.f, TopCornerLeft.y)); //set the cursor next to the result item, the float is the spacing
			LinesNb++;
		}
		else
		{
			ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();

			RLMAPS_RenderAResult(i, draw_list, mapspath);

			ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, TopCornerLeft.y + 280.f)); //set the cursor to a new line, the float is the spacing
			LinesNb = 0;
		}

		RLMAPS_SearchWorkshopDisplayed++;
	}
}

void Pluginx64::RLMAPS_RenderAResult(int i, ImDrawList* drawList, static char mapspath[200])
{
	ImGui::PushID(i);

	RLMAPS_MapResult mapResult = RLMAPS_MapResultList.at(i);
	std::string mapName = mapResult.Name;
	std::string mapSize = mapResult.Size;
	std::string mapDescription = mapResult.Description;
	std::string mapAuthor = mapResult.Author;

	//Popup if is a downlaod is in progress and user wants to start a new download
	renderInfoPopup("Downloading?", IsDownloadDingWarningText.c_str());

	//Popup if maps directory doesn't exist
	renderInfoPopup("Exists?", DirNotExistText.c_str());


	ImGui::BeginGroup();
	{
		std::string SizeConverted = ResultSizeText + convertToMB(mapSize);

		ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
		ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + 190.f, TopCornerLeft.y + 260.f);
		ImVec2 ImageP_Min = ImVec2(TopCornerLeft.x + 6.f, TopCornerLeft.y + 6.f);
		ImVec2 ImageP_Max = ImVec2(TopCornerLeft.x + 184.f, TopCornerLeft.y + 179.f);

		drawList->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(44, 75, 113, 255), 5.f, 15); //Blue rectangle result
		drawList->AddRect(ImageP_Min, ImageP_Max, ImColor(255, 255, 255, 255), 0, 15, 2.0F); //Image white outline

		if (mapResult.isImageLoaded == true)
		{
			drawList->AddImage(mapResult.Image->GetImGuiTex(), ImageP_Min, ImageP_Max); //Map image preview
		}

		std::string GoodMapName = mapName.substr(0, 29);
		if (mapName.length() > 31)
		{
			GoodMapName.append("...");
		}
		drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 185.f), ImColor(255, 255, 255, 255), GoodMapName.c_str()); //Map title
		drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 200.f), ImColor(255, 255, 255, 255), SizeConverted.c_str()); //Map size
		drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 215.f), ImColor(255, 255, 255, 255),
			std::string(ResultByText.c_str() + mapAuthor).c_str()); // "By : " Map Author
		ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 235.f));
		if (ImGui::Button(DownloadMapButtonText.c_str(), ImVec2(182, 20))) // "Download Map"																								//Map download button
		{
			if (RLMAPS_IsDownloadingWorkshop == false && IsRetrievingWorkshopFiles == false && Directory_Or_File_Exists(fs::path(mapspath)))
			{
				std::thread t2(&Pluginx64::RLMAPS_DownloadWorkshop, this, mapspath, mapResult);
				t2.detach();
			}
			else
			{
				if (!Directory_Or_File_Exists(fs::path(mapspath)))
				{
					ImGui::OpenPopup("Exists?");
				}

				if (RLMAPS_IsDownloadingWorkshop || IsRetrievingWorkshopFiles)
				{
					ImGui::OpenPopup("Downloading?");
				}
			}
		}
		ImGui::EndGroup();

		if (ImGui::IsItemHovered())
		{
			std::string GoodDescription = mapDescription;

			if (mapDescription.length() > 150)
			{
				GoodDescription.insert(150, "\n");

				if (mapDescription.length() > 280)
				{
					GoodDescription.erase(280);
					GoodDescription.append("...");
				}
			}


			ImGui::BeginTooltip();
			ImGui::Text("Title : %s", mapName.c_str());
			ImGui::Text("By : %s", mapAuthor.c_str());
			ImGui::Text("Description : \n%s", GoodDescription.c_str());
			ImGui::EndTooltip();
		}
		ImGui::PopID();
	}
}


void Pluginx64::RefreshMapsFunct(std::string mapsfolders)
{
	MapList.clear();

	std::vector<std::filesystem::path> MapsDirectories;

	for (const auto& dir : fs::directory_iterator(mapsfolders))
	{
		if (dir.is_directory())
		{
			MapsDirectories.push_back(dir.path());
		}
	}

	if (MapsDirectories.size() == 0) { cvarManager->log("No maps detected"); return; }

	for (int i = 0; i < MapsDirectories.size(); i++)
	{
		cvarManager->log("Checking files in : " + MapsDirectories.at(i).string() + "/");

		std::filesystem::path CurrentMapDirectory = MapsDirectories.at(i);
		Map map;
		map.Folder = MapsDirectories.at(i).string();


		renameFileToUPK(CurrentMapDirectory);		//rename the map udk to upk, for people that already had the plugin


		int nbFilesInDirectory = 0;
		//get the numbers of files in the current map directory
		for (const auto& file : fs::directory_iterator(CurrentMapDirectory))
		{
			nbFilesInDirectory++;
		}

		if (nbFilesInDirectory > 0)
		{
			int nbFiles = 0;
			bool hasFoundUPK = false;
			bool hasFoundPreview = false;
			bool hasFoundJSON = false;
			bool hasFoundZIP = false;
			for (const auto& file : fs::directory_iterator(CurrentMapDirectory))
			{
				std::string fileExtension = file.path().filename().extension().string();
				nbFiles++;



				if (!hasFoundJSON && fileExtension == ".json")
				{
					if (file.path().filename().string().substr(0, file.path().filename().string().length() - 5) == CurrentMapDirectory.filename().string())//if JSON file name == the map folder's name
					{
						map.JsonFile = file.path().string();
						hasFoundJSON = true;

						//cvarManager->log("Folder name  : " + CurrentMapDirectory.filename().string());
						cvarManager->log("JSON name  : " + file.path().filename().string());
					}
				}
				else
				{
					if (nbFiles == nbFilesInDirectory && hasFoundJSON == false)
					{
						map.JsonFile = "NoInfos";
					}
				}

				if ((!hasFoundPreview && fileExtension == ".png") || (!hasFoundPreview && fileExtension == ".jpg") || (!hasFoundPreview && fileExtension == ".jfif"))
				{
					map.PreviewImage = std::make_shared<ImageWrapper>(file.path(), false, true);
					map.isPreviewImageLoaded = true;
					cvarManager->log("Preview Loaded : " + file.path().string());
					hasFoundPreview = true;
				}
				else
				{
					if (nbFiles == nbFilesInDirectory && hasFoundPreview == false)
					{
						map.PreviewImage = std::make_shared<ImageWrapper>(IfNoPreviewImagePath, false, true); //if there is no preview image, it will load a red cross image
						map.isPreviewImageLoaded = true;
						cvarManager->log("No preview found in this folder");
					}
				}

				if (!hasFoundZIP && fileExtension == ".zip")
				{
					map.ZipFile = file.path();
					hasFoundZIP = true;
				}
				else
				{
					if (nbFiles == nbFilesInDirectory && hasFoundZIP == false)
					{
						map.ZipFile = "NoZipFound";
						cvarManager->log("No .zip file found in this folder : " + CurrentMapDirectory.string());
					}
				}


				if (!hasFoundUPK && fileExtension == ".upk")
				{
					map.UpkFile = file.path();
					hasFoundUPK = true;
				}
				else
				{
					if (nbFiles == nbFilesInDirectory && hasFoundUPK == false)
					{
						map.UpkFile = "NoUpkFound";
						cvarManager->log("No upk found in this folder : " + CurrentMapDirectory.string());
						cvarManager->log("You have to extract the files manually of this .zip : " + CurrentMapDirectory.string() + "/" + CurrentMapDirectory.filename().string() + ".zip");
					}
				}
			}
		}
		else       //if the folder is empty
		{
			cvarManager->log("Empty folder !");
			map.UpkFile = "EmptyFolder";
			map.ZipFile = "EmptyFolder";
			map.JsonFile = "EmptyFolder";
			map.PreviewImage = std::make_shared<ImageWrapper>(IfNoPreviewImagePath, false, true); //if there is no preview image, it will load a red cross image
			map.isPreviewImageLoaded = true;
		}

		MapList.push_back(map);
		cvarManager->log("");
	}
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


	//RefreshMapsFunct();
}

// Called when window is closed
void Pluginx64::OnClose()
{
	isWindowOpen_ = false;


	MapsFolderPath = MapsFolderPathBuf;
}
