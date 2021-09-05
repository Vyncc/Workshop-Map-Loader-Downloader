#include "pch.h"
#include "WorkshopMapLoader.h"
#include "IMGUI/imgui_internal.h"

namespace fs = std::filesystem;


char Pluginx64::MapsFolderPathBuf[200];
bool Pluginx64::FR = false;
int SearchWorkshopDisplayed = 0;


const char* combo_selected_most = "Most Popular";
const char* combo_selected_period = "1 Week";
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
		Tab2SearchWorkshopText = "Search Workshop";
		DlWorkshopByURLText = "Download Workshop By Url";
		Label2Text = "Steam Workshop Url :";
		DownloadButtonText = "Download";
		Label3Text = "Search A Workshop :";
		SearchButtonText = "Search";
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
		Tab2SearchWorkshopText = "Rechercher Un Workshop";
		DlWorkshopByURLText = "Telecharger Un Workshop Avec Une Url";
		Label2Text = "Url Du Workshop Steam :";
		DownloadButtonText = "Telecharger";
		Label3Text = "Rechercher Un Workshop :";
		SearchButtonText = "Rechercher";
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

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}


	if (ImGui::BeginTabBar("TabBar"))
	{
		if (ImGui::BeginTabItem(Tab1MapLoaderText.c_str())) // "Map Loader"
		{
			ImGui::Text(Label1Text.c_str()); // "Put the folder's path of the maps, don't forget to add a  /  at the end."
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


			if (ImGui::Button(SavePathText.c_str())) // "Save Path"
			{
				if (Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\"))
				{
					SaveInCFG(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg", MapsFolderPathBuf, std::to_string(FR), unzipMethod, std::to_string(HasSeeNewUpdateAlert));
				}
			}


			if (ImGui::Button(RefreshMapsButtonText.c_str())) // "Refresh Maps"
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

			ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Exists?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text(DirNotExistText.c_str()); //"The directory you entered does not exist !"
				ImGui::SameLine();
				ImGui::NewLine();
				ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 65.f, ImGui::GetCursorScreenPos().y));
				if (ImGui::Button("OK", ImVec2(100.f, 25.f))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}

			ImGui::Separator();

			renderMaps();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(Tab2SearchWorkshopText.c_str())) //"Search Workshop"
		{
			if (ImGui::CollapsingHeader(DlWorkshopByURLText.c_str())) // "Download Workshop By Url"
			{
				ImGui::Text(Label2Text.c_str()); // "Steam Workshop Url :"
				static char url[200] = "Ex : https://steamcommunity.com/sharedfiles/filedetails/?id=2120912805&searchtext=rings+3";
				ImGui::InputText("##workshopurl", url, IM_ARRAYSIZE(url));
				if (ImGui::Button(DownloadButtonText.c_str())) // "Download"
				{
					std::thread t1(&Pluginx64::DownloadWorkshop, this, url, MapsFolderPathBuf, "", true, 0, false);
					t1.detach();
				}
			}

			ImGui::Separator();

			ImGui::Text(Label3Text.c_str()); // "Search A Workshop :"
			static char keyWord[200] = "";
			ImGui::InputText("##workshopkeyword", keyWord, IM_ARRAYSIZE(keyWord));
			std::string get_full_url = base_url + replace(std::string(keyWord), *" ", *"+");
			//ImGui::Text(get_full_url.c_str());

			if (ImGui::Button(SearchButtonText.c_str()) && isSearching == false) // "Search"
			{
				std::thread t2(&Pluginx64::StartSearchRequest, this, get_full_url);
				t2.detach();
			}


			if (IsRetrievingWorkshopFiles == true)
			{
				ImGui::TextColored(ImVec4(255, 155, 0, 1), RetrievingFilesText.c_str()); // "Retrieving workshop files for download..."
			}

			if (FolderErrorBool)
			{
				ImGui::OpenPopup("FolderError");
			}


			ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("FolderError", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Error : ");
				ImGui::Text(FolderErrorText.c_str()); // error message
				ImGui::NewLine();
				ImGui::NewLine();
				ImGui::Text("if the error message is an acces denied, it means the maps folder need administrator acces,");
				ImGui::Text("so you need to change the location of the maps folder to a place that doesn't need it (ex : desktop)");
				ImGui::NewLine();

				if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
				{
					FolderErrorBool = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}


			if (DownloadFailed)
			{
				ImGui::OpenPopup("DownloadFailed");
			}


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


			if (UserIsChoosingYESorNO)
			{
				ImGui::OpenPopup("Download?");
			}


			ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Download?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text(WantToDawnloadText.c_str()); // "Do you really want to download?\nYou'll not be able to cancel if you start it."
				ImGui::NewLine();
				ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 25.f, ImGui::GetCursorScreenPos().y));

				if (ImGui::Button(YESButtonText.c_str(), ImVec2(100.f, 25.f))) // "YES"
				{
					AcceptTheDownload = true;
					UserIsChoosingYESorNO = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button(NOButtonText.c_str(), ImVec2(100.f, 25.f))) // "NO"
				{
					AcceptTheDownload = false;
					UserIsChoosingYESorNO = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}


			if (IsDownloadingWorkshop == true)
			{
				ImGui::TextColored(ImVec4(0, 255, 0, 1), "%s %s / %s", DownloadingText.c_str(), convertToMB(std::to_string(WorkshopDownload_ProgressString)).c_str(),
					convertToMB(std::to_string(WorkshopDownload_FileSizeString)).c_str()); // "Downloading : 0 MB / 0 MB"
			}

			ImGui::Separator();

			ImGui::Text("%s :", SortByText[0].c_str()); // "Sort By :"
			ImGui::SameLine();

			std::string MostPopular_Url = "https://steamcommunity.com/workshop/browse/?appid=252950&browsesort=trend&section=readytouseitems";

			if (ImGui::BeginCombo(combo_selected_most, combo_selected_most, ImGuiComboFlags_NoPreview))
			{
				if (ImGui::Selectable(SortByText[1].c_str())) // "Most Popular"
				{
					std::thread t6(&Pluginx64::StartSearchRequest, this, MostPopular_Url);
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


			if (MostPopularSelected)
			{
				ImGui::SameLine();
				ImGui::Text("         %s :", PeriodText[7].c_str()); // "Period :"
				ImGui::SameLine();


				if (ImGui::BeginCombo(combo_selected_period, combo_selected_period, ImGuiComboFlags_NoPreview))
				{
					if (ImGui::Selectable(PeriodText[0].c_str())) // "Today"
					{
						std::thread t6(&Pluginx64::StartSearchRequest, this, MostPopular_Url + "&actualsort=trend&p=1&days=1");
						t6.detach();
						combo_selected_period = PeriodText[0].c_str(); // "Today"
					}

					if (ImGui::Selectable(PeriodText[1].c_str())) // "1 Week"
					{
						std::thread t6(&Pluginx64::StartSearchRequest, this, MostPopular_Url + "&actualsort=trend&p=1&days=7");
						t6.detach();
						combo_selected_period = PeriodText[1].c_str(); // "1 Week"
					}

					if (ImGui::Selectable(PeriodText[2].c_str())) // "1 Months"
					{
						std::thread t6(&Pluginx64::StartSearchRequest, this, MostPopular_Url + "&actualsort=trend&p=1&days=30");
						t6.detach();
						combo_selected_period = PeriodText[2].c_str(); // "1 Months"
					}

					if (ImGui::Selectable(PeriodText[3].c_str())) // "3 Months"
					{
						std::thread t6(&Pluginx64::StartSearchRequest, this, MostPopular_Url + "&actualsort=trend&p=1&days=90");
						t6.detach();
						combo_selected_period = PeriodText[3].c_str(); // "3 Months"
					}

					if (ImGui::Selectable(PeriodText[4].c_str())) // "6 Months"
					{
						std::thread t6(&Pluginx64::StartSearchRequest, this, MostPopular_Url + "&actualsort=trend&p=1&days=180");
						t6.detach();
						combo_selected_period = PeriodText[4].c_str(); // "6 Months"
					}

					if (ImGui::Selectable(PeriodText[5].c_str())) // "1 Year"
					{
						std::thread t6(&Pluginx64::StartSearchRequest, this, MostPopular_Url + "&actualsort=trend&p=1&days=365");
						t6.detach();
						combo_selected_period = PeriodText[5].c_str(); // "1 Year"
					}

					if (ImGui::Selectable(PeriodText[6].c_str())) // "Since the begining"
					{
						std::thread t6(&Pluginx64::StartSearchRequest, this, MostPopular_Url + "&actualsort=trend&p=1&days=-1");
						t6.detach();
						combo_selected_period = PeriodText[6].c_str(); // "Since the begining"
					}
					ImGui::EndCombo();
				}
			}

			ImGui::Separator();

			ImGui::Text("%s %d / %d", WorkshopsFoundText.c_str(), SearchWorkshopDisplayed, MapsNamesList.size()); // "Workshops Found : 0 / 0"

			if (OtherPagesList.size() > 0) //if there is other pages of wokshop maps
			{
				for (int i = 0; i < OtherPagesList.size(); i++)
				{
					ImGui::SameLine();

					//std::string PageButtonName = "Page " + OtherPagesList.at(i).substr(OtherPagesList.at(i).length() - 1, 1);
					std::string PageButtonName = "Page " + FindAllSubstringInAString(OtherPagesList.at(i), "&p=", "&days").at(0);

					if (ImGui::Button(PageButtonName.c_str()) && isSearching == false)
					{
						std::thread Page_Thread(&Pluginx64::StartSearchRequest, this, OtherPagesList.at(i));
						Page_Thread.detach();
					}
				}
			}
			ImGui::NewLine();
			ImGui::NewLine();

			renderSearchWorkshopResults(MapsFolderPathBuf);

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
	if (ImGui::BeginChild("#MapsLauncherButtons"))
	{
		for (int i = 0; i < MapList.size(); i++)
		{
			Map curMap = MapList.at(i);

			if (curMap.UdkFile != "NoUdkFound") //needed to avoid problems if a folder is not containing a .udk
			{
				ImGui::PushID(i); //needed to make the button work
				ImGui::BeginGroup();
				{
					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					ImFont* fontA = ImGui::GetDefaultFont();


					if (ImGui::Button("##map", ImVec2(ImGui::GetWindowWidth(), 120)))
					{
						gameWrapper->Execute([&, curMap](GameWrapper* gw)
							{
								cvarManager->executeCommand("load_workshop \"" + curMap.Folder.string() + "/" + curMap.UdkFile.filename().string() + "\"");
							});
						cvarManager->log("Map selected : " + curMap.UdkFile.filename().string());
					}
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
			}
		}
	}
	ImGui::EndChild();
}


void Pluginx64::renderSearchWorkshopResults(static char mapspath[200])
{
	int LinesNb = 0;
	SearchWorkshopDisplayed = 0;


	if (ImGui::BeginChild("#SearchWorkshopMapsResults"))
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();


		for (int i = 0; i < MapResultList.size(); i++)
		{
			if (LinesNb < 4)
			{
				ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();

				RenderAResult(i, draw_list, mapspath);

				ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 270.f, TopCornerLeft.y)); //set the cursor next to the result item, the float is the spacing
				LinesNb++;
			}
			else
			{
				ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();

				RenderAResult(i, draw_list, mapspath);

				ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, TopCornerLeft.y + 280.f)); //set the cursor to a new line, the float is the spacing
				LinesNb = 0;
			}
			
			SearchWorkshopDisplayed++;
		}
	}
	ImGui::EndChild();
}


void Pluginx64::RenderAResult(int i, ImDrawList* drawList, static char mapspath[200])
{
	ImGui::PushID(i);

	MapResult mapResult = MapResultList.at(i);
	std::string mapName = mapResult.Name;
	std::string mapID = mapResult.ID;
	std::string mapDescription = mapResult.Description;
	std::string mapSize = mapResult.Size;
	std::string mapAuthor = mapResult.Author;

	//Popup if is a downlaod is in progress and user wants to start a new download
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Downloading?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(IsDownloadDingWarningText.c_str()); // "A download is already running !\nYou cannot download 2 workshops at the same time."
		ImGui::SameLine();
		ImGui::NewLine();

		ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 65.f, ImGui::GetCursorScreenPos().y));
		if (ImGui::Button("OK", ImVec2(100.f, 25.f))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}


	//Popup if maps directory doesn't exist
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() / 2, ImGui::GetWindowHeight() / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Exists?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(DirNotExistText.c_str()); // "The directory you entered does not exist !"
		ImGui::SameLine();
		ImGui::NewLine();

		ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 65.f, ImGui::GetCursorScreenPos().y));
		if (ImGui::Button("OK", ImVec2(100.f, 25.f))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}


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
			if (IsDownloadingWorkshop == false && IsRetrievingWorkshopFiles == false && Directory_Or_File_Exists(fs::path(mapspath)))
			{
				std::thread t2(&Pluginx64::DownloadWorkshop, this, "", mapspath, mapResult.ID, false, i, true);
				t2.detach();
			}
			else
			{
				if (!Directory_Or_File_Exists(fs::path(mapspath)))
				{
					ImGui::OpenPopup("Exists?");
				}

				if (IsDownloadingWorkshop || IsRetrievingWorkshopFiles)
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

		int nbFilesInDirectory = 0;
		//get the numbers of files in the current map directory
		for (const auto& file : fs::directory_iterator(CurrentMapDirectory))
		{
			nbFilesInDirectory++;
		}

		if (nbFilesInDirectory > 0)
		{
			int nbFiles = 0;
			bool hasFoundUDK = false;
			bool hasFoundPreview = false;
			bool hasFoundJSON = false;
			for (const auto& file : fs::directory_iterator(CurrentMapDirectory))
			{
				std::string fileExtension = file.path().filename().extension().string();
				nbFiles++;

				if (!hasFoundUDK && fileExtension == ".udk")
				{
					map.UdkFile = file.path();
					hasFoundUDK = true;
				}
				else
				{
					if (nbFiles == nbFilesInDirectory && hasFoundUDK == false)
					{
						map.UdkFile = "NoUdkFound";
						cvarManager->log("No udk found in this folder : " + CurrentMapDirectory.string());
						cvarManager->log("You have to extract the files manually of this .zip : " + CurrentMapDirectory.string() + "/" + CurrentMapDirectory.filename().string() + ".zip");
					}
				}

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

				if ((!hasFoundPreview && fileExtension == ".png") || (!hasFoundPreview && fileExtension == ".jpg")) // || fileExtension == ".jpg"
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
			}
		}
		else       //if the folder is empty
		{
			cvarManager->log("Empty folder !");
			map.UdkFile = "NoUdkFound";
			map.JsonFile = "NoInfos";
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
