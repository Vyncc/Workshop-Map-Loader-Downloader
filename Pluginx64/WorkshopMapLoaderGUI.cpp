#include "pch.h"
#include "WorkshopMapLoader.h"
#include "IMGUI/imgui_internal.h"

namespace fs = std::filesystem;


char Pluginx64::MapsFolderPathBuf[200];
bool Pluginx64::FR = false;
int RLMAPS_SearchWorkshopDisplayed = 0;




float heightMutators = 35.f;
float heightHostGamePopup = 194.f;

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

	Gamepad controller1 = Gamepad(1);

	controller1.Update();
	if (controller1.Connected())
	{
		float stickX = controller1.LeftStick_X();
		float stickY = controller1.LeftStick_Y();

		POINT point;
		GetCursorPos(&point);

		static bool L1WasPressed = false;
		static bool BWasPressed = false;

		if (controller1.checkButtonPress(XINPUT_GAMEPAD_LEFT_SHOULDER) && !L1WasPressed) {
			cvarManager->log("Button L1 is pressed");
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); //Left click down
			L1WasPressed = true;
		}
		else if (!controller1.checkButtonPress(XINPUT_GAMEPAD_LEFT_SHOULDER) && L1WasPressed)
		{
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); //Left click realease

			cvarManager->log("Button L1 is realeased");
			L1WasPressed = false;
		}


		if (controller1.checkButtonPress(XINPUT_GAMEPAD_B) && !BWasPressed) {
			cvarManager->log("Button B is pressed");
			BWasPressed = true;
		}
		else if (!controller1.checkButtonPress(XINPUT_GAMEPAD_B) && BWasPressed)
		{
			isWindowOpen_ = false;
			cvarManager->log("Button B is realeased");
			BWasPressed = false;
		}


		if (!controller1.LStick_InDeadzone())
		{
			int pixelsX = 0;
			int pixelsY = 0;

			pixelsX = stickX * ControllerSensitivity;
			pixelsY = stickY * ControllerSensitivity;

			//cvarManager->log("pixelX : " + std::to_string(pixelsX));
			//cvarManager->log("pixelY : " + std::to_string(pixelsY));

			SetCursorPos(point.x + pixelsX, point.y - pixelsY);
		}
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


	if (!FR)
	{
		//Menubar
		SettingsText = "Settings";
		MultiplayerText = "Multiplayer";
		LastUpdateText = "Last Update";
		JoinCWGText = "Join Community Workshop Games discord server :";
		OpenCPCCText = "Open CookedPCConsole Directory";
		NoMapsCanBeJoinText = "No maps can be joined";
		MapsJoinableText = "Maps joinable";
		DlTexturesText = "Download Textures";
		LanguageText = "Language";
		ExtractMethodText = "Extract Method";
		WarningText = "Warning :";

		//Controller settings
		ControllerText = "Controller";
		UseControllerText = "Use Controller";
		ControllsText = "Controlls";
		ScrollSensitivityText = "Scroll Sensitivity";
		SensitivityText = "Sensitivity";
		ControllsLitText[0] = "Left Thumb + Right Thumb : open/close the menu";
		ControllsLitText[1] = "DPAD arrows : navigate through the maps";
		ControllsLitText[2] = "Left joystick : move the cursor";
		ControllsLitText[3] = "Right joystick : scroll";
		ControllsLitText[4] = "LB/L1 : click";
		ControllsLitText[5] = "B/O : close the menu";

		//1st Tab
		Tab1MapLoaderText = "Map Loader";
		Label1Text = "Put the path of the maps folder :";
		SelectMapsFolderText = "Select maps folder";
		RefreshMapsButtonText = "Refresh Maps";
		SavePathText = "Save Path";
		MapsPerLineText = "Maps Per Line :";
		//context menu strip
		OpenMapDirText = "Open map directory";
		DeleteMapText = "Delete map";

		//LauchMode Popup
		CancelText = "Cancel";

		//Add Map
		AddMapText = "Add Map";
		NameText = "Name :";
		AuthorText = "Author :";
		MapFilePathText = "Map File Path :";
		ImagePathText = "Image Path :";
		SelectFileText = "Select File";
		FieldEmptyText = "A field is empty !";
		ConfirmLabelText = "Do you really want to add this map ?";
		MapAddedSuccessfullyText = "Map added successfully !";

		//2nd Tab
		DownloadButtonText = "Download";
		Label3Text = "Search A Workshop :";
		SearchButtonText = "Search";
		SearchingText = "Searching...";
		WorkshopsFoundText = "Workshops Found :";
		BrowseMapsText = "Browse Maps";
		Tab3SearchWorkshopText = "Search Workshop (rocketleaguemaps.us)";


		//Search Result
		ResultByText = "By ";
		ResultSizeText = "Size : ";
		DownloadMapButtonText = "Download Map";

		//Warnings
		DirNotExistText = "This directory is not valid !";
		DownloadFailedText = "Download Failed !" + DownloadFailedErrorText;
		WantToDawnloadText = "Do you really want to download?\nYou'll not be able to cancel if you start it.";
		YESButtonText = "YES";
		NOButtonText = "NO";
		IsDownloadDingWarningText = "A download is already running !\nYou cannot download 2 workshops at the same time.";
		PathSavedText = "Path saved successfully !";

		//ExtractMapFiles
		EMFMessageText1 = "The map isn't extracted from ";
		EMFMessageText2 = "\nChoose an extract method (you need to click on refresh maps after extracting) :";
		EMFStillDoesntWorkText = "Still not working";
		//ExtractManually
		EMLabelText = "If both of the extract methods didn't work, you need to extract the files manually of ";

		//Download Texutures
		DLTLabel1Text = "It seems like the workshop textures aren't installed in " + RLCookedPCConsole_Path.string();
		DLTLabel2Text = "You can still play without the workshop textures but some maps will have some white/weird textures.";
		DLTMissingFilesText = "Missing Files";
		DLTTexturesInstalledText = "Workshop textures installed !";
		CloseText = "Close";
		DontAskText = "Don't ask me again";

		//File Explorer
		NewFolderText = "New Folder";
		ConfirmText = "Confirm";
		SelectText = "Select";
	}
	else
	{
		//Menubar
		SettingsText = "Parametres";
		MultiplayerText = "Multijoueur";
		LastUpdateText = "Derniere Maj";
		JoinCWGText = "Rejoins le serveur discord Community Workshop Games :";
		OpenCPCCText = "Ouvrir le dossier CookedPCConsole";
		NoMapsCanBeJoinText = "Aucune map ne peut etre rejoint";
		MapsJoinableText = "Maps rejoignables";
		DlTexturesText = "Telecharger les textures";
		LanguageText = "Langue";
		ExtractMethodText = "Methode d'extraction";
		WarningText = "Attention :";

		//Controller settings
		ControllerText = "Manette";
		UseControllerText = "Activer La Manette";
		ControllsText = "Commandes";
		ScrollSensitivityText = "Sensibilite du defilement";
		SensitivityText = "Sensibilite";
		ControllsLitText[0] = "Pouce Gauche + Pouce Droit : ouvrir/fermer le menu";
		ControllsLitText[1] = "Fleches : naviguer dans les maps";
		ControllsLitText[2] = "Joystick Gauche : bouger la souris";
		ControllsLitText[3] = "Joystick Droit : faire defiler";
		ControllsLitText[4] = "LB/L1 : cliquer";
		ControllsLitText[5] = "B/O : fermer le menu";

		//1st Tab
		Tab1MapLoaderText = "Charger Map";
		Label1Text = "Mets le chemin du dossier des maps :";
		SelectMapsFolderText = "Choisir Dossier Des Maps";
		RefreshMapsButtonText = "Rafraichir Les Maps";
		SavePathText = "Sauvegarder Le Chemin";
		MapsPerLineText = "Maps Par Ligne :";

		//context menu strip
		OpenMapDirText = "Ouvrir le dossier de la map";
		DeleteMapText = "Supprimer la map";

		//LauchMode Popup
		CancelText = "Annuler";

		//Add Map
		AddMapText = "Ajouter Map";
		NameText = "Nom :";
		AuthorText = "Auteur :";
		MapFilePathText = "Fichier De La Map :";
		ImagePathText = "Image :";
		SelectFileText = "Parcourir";
		FieldEmptyText = "Un champ est vide !";
		ConfirmLabelText = "Veux-tu vraiment ajouter cette map ?";
		MapAddedSuccessfullyText = "Map ajoute avec succes !";

		//2nd Tab
		DownloadButtonText = "Telecharger";
		Label3Text = "Rechercher Un Workshop :";
		SearchButtonText = "Rechercher";
		SearchingText = "Recherche en cours...";
		WorkshopsFoundText = "Workshops Trouves :";
		BrowseMapsText = "Parcourir Les Maps";
		Tab3SearchWorkshopText = "Rechercher Workshop (rocketleaguemaps.us)";

		//Search Result
		ResultByText = "Par ";
		ResultSizeText = "Taille : ";
		DownloadMapButtonText = "Telecharger La Map";

		//Warnings
		DirNotExistText = "Ce chemin n'est pas valide !";
		DownloadFailedText = "Le telechargement a echoue !" + DownloadFailedErrorText;
		WantToDawnloadText = "Veux-tu vraiment telecharger?\nTu ne pourras plus l'annuler si tu le commence.";
		YESButtonText = "OUI";
		NOButtonText = "NON";
		IsDownloadDingWarningText = "Un telechargement est deja en cours !\nTu ne peux pas telecharger 2 workshops en meme temps.";
		PathSavedText = "Le chemin a ete sauvegarde !";


		//ExtractMapFiles
		EMFMessageText1 = "La map n'est pas extrait de ";
		EMFMessageText2 = "\nChoisis une methode d'extraction (rafraichis les maps apres l'extraction) :";
		EMFStillDoesntWorkText = "Ne fonctionne pas";
		//ExtractManually
		EMLabelText = "Si les deux methodes d'extraction n'ont pas fonctionne, tu dois extraire les fichiers manuellement de ";


		//Download Texutures
		DLTLabel1Text = "Les textures des workshops ne semblent pas etre installees dans " + RLCookedPCConsole_Path.string();
		DLTLabel2Text = "Tu peux toujours jouer sans mais des maps auront des textures blanches/bizarres.";
		DLTMissingFilesText = "Fichiers Manquants";
		DLTTexturesInstalledText = "Textures des workshops installees!";
		CloseText = "Fermer";
		DontAskText = "Ne plus me demander";

		//File Explorer
		NewFolderText = "Nouv. Dossier";
		ConfirmText = "Confirmer";
		SelectText = "Selectionner";
	}


	if (!HasSeeNewUpdateAlert)
	{
		ImGui::OpenPopup("New Update");
	}
	renderNewUpdatePopup();

	if (AddedMapSccuessfully)
	{
		ImGui::OpenPopup("Add Map Successfull");
		AddedMapSccuessfully = false;
	}
	renderInfoPopup("Add Map Successfull", MapAddedSuccessfullyText.c_str()); //"Map added successfully !"


	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(SettingsText.c_str())) //"Settings"
		{
			if (ImGui::BeginMenu(ExtractMethodText.c_str()))
			{
				if (ImGui::Selectable("Batch File Method"))
				{
					unzipMethod = "Bat";

					if (Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\")) //save changes in cfg
					{
						SaveInCFG();
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
						SaveInCFG();
					}
				}

				if (unzipMethod == "Powershell")
				{
					ImGui::SameLine();
					ImGui::Text(" : Selected");
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(LanguageText.c_str())) //"Language"
			{
				if (ImGui::Selectable("French"))
				{
					FR = true;
					SaveInCFG();
				}

				if (ImGui::Selectable("English"))
				{
					FR = false;
					SaveInCFG();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ControllerText.c_str())) //"Controller"
			{
				if (ImGui::Checkbox(UseControllerText.c_str(), &UseController))
				{
					SaveInCFG();
				}
				

				if (ImGui::BeginMenu(ControllsText.c_str()))
				{
					ImGui::Text(ControllsLitText[0].c_str()); //"Left Thumb + Right Thumb : open/close the menu"
					ImGui::Text(ControllsLitText[1].c_str()); //"DPAD arrows : navigate through the maps"
					ImGui::Text(ControllsLitText[2].c_str()); //"Left joystick : move the cursor"
					ImGui::Text(ControllsLitText[3].c_str()); //"Right joystick : scroll"
					ImGui::Text(ControllsLitText[4].c_str()); //"LB/L1 : click"
					ImGui::Text(ControllsLitText[5].c_str()); //"B/O : close the menu"
					ImGui::EndMenu();
				}

				if(ImGui::SliderInt(SensitivityText.c_str(), &ControllerSensitivity, 1.f, 30.f)) //"Sensitivity"
				{
					SaveInCFG();
				}

				if(ImGui::SliderInt(ScrollSensitivityText.c_str(), &ControllerScrollSensitivity, 1.f, 30.f)) //"ScrollSensitivity"
				{
					SaveInCFG();
				}

				ImGui::EndMenu();
			}

			if (ImGui::Selectable(DlTexturesText.c_str())) //"Download Textures"
			{
				DownloadTexturesBool = true;
			}

			if (ImGui::Checkbox("Enable Freeze Fix", &EnableAntiFreezeFix))
			{
				SaveInCFG();
			}

			ImGui::EndMenu();
		}


		if (ImGui::BeginMenu(MultiplayerText.c_str())) //"Multiplayer"
		{
			if (ImGui::Selectable(OpenCPCCText.c_str())) //"Open CookedPCConsole Directory"
			{
				std::wstring w_modsDir = s2ws(RLCookedPCConsole_Path.string());
				LPCWSTR L_modsDir = w_modsDir.c_str();

				ShellExecute(NULL, L"open", L_modsDir, NULL, NULL, SW_SHOWDEFAULT);
			}

			ImGui::Separator();

			ImGui::Text(JoinCWGText.c_str()); //"Join Community Workshop Games discord server :"
			renderLink("https://discord.com/invite/KVgmf9JFpZ");

			ImGui::EndMenu();
		}

		if (DownloadTexturesBool) //I know this is not good but It works, so I don't care
		{
			ImGui::OpenPopup("DownloadTextures");
		}
		renderDownloadTexturesPopup(CheckExist_TexturesFiles());


		if (ImGui::Selectable(LastUpdateText.c_str(), false, 0, ImGui::CalcTextSize(LastUpdateText.c_str())))
		{
			HasSeeNewUpdateAlert = false;
		}

		if (ImGui::BeginMenu("Support Me"))
		{
			ImGui::Text("Support me on Patreon :");
			renderLink("https://www.patreon.com/WorkshopMapLoader");

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Credits"))
		{
			ImGui::Text("Plugin made by Vync#3866, contact me on discord for custom plugin commissions.");
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
		if (ImGui::BeginTabItem(Tab1MapLoaderText.c_str())) // "Map Loader"
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);

			CenterNexIMGUItItem(ImGui::CalcTextSize(Label1Text.c_str()).x);
			ImGui::Text(Label1Text.c_str()); // "Put the folder's path of the maps, don't forget to add a  /  at the end."

			CenterNexIMGUItItem(628.f);
			ImGui::SetNextItemWidth(628.f);
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

			CenterNexIMGUItItem(628.f); //306.f("Save Path" button) + 8.f(the gap between both buttons) + 306.f("Refresh Mas" button) = 620.f  (cause they are on same line)

			if (ImGui::Button(SelectMapsFolderText.c_str(), ImVec2(151.f, 32.f))) // "Select maps folder"
			{
				ImGui::OpenPopup("Select maps folder");
			}
			renderFileExplorer();

			ImGui::SameLine();

			if (ImGui::Button(SavePathText.c_str(), ImVec2(151.f, 32.f))) // "Save Path"
			{
				if (Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\"))
				{
					SaveInCFG();

					ImGui::OpenPopup("SavePath");
				}
			}
			renderInfoPopup("SavePath", PathSavedText.c_str());


			ImGui::SameLine();

			if (ImGui::Button(AddMapText.c_str(), ImVec2(151.f, 32.f))) // "Add Map"
			{
				ImGui::OpenPopup(AddMapText.c_str()); //"Add Map"
			}
			renderAddMapManuallyPopup();

			ImGui::SameLine();

			std::vector<std::string> missingTexturesFiles = CheckExist_TexturesFiles();

			if (ImGui::Button(RefreshMapsButtonText.c_str(), ImVec2(151.f, 32.f))) // "Refresh Maps"
			{
				if (!Directory_Or_File_Exists(fs::path(MapsFolderPathBuf)))
				{
					ImGui::OpenPopup("Exists?");
				}
				else
				{
					RefreshMapsFunct(MapsFolderPathBuf);
					if (missingTexturesFiles.size() > 0 && dontAsk == 0)
					{
						ImGui::OpenPopup("DownloadTextures");
					}
				}
			}
			renderInfoPopup("Exists?", DirNotExistText.c_str());

			renderDownloadTexturesPopup(missingTexturesFiles);

			ImGui::SameLine();

			ImGui::BeginGroup(); //display mode + maps on line
			{
				AlignRightNexIMGUItItem(95.f, 8.f);
				ImVec2 cursorPos = ImGui::GetCursorPos();
				if (MapsDisplayMode == 1)
				{
					ImGui::SetCursorPos(ImVec2(cursorPos.x + 14.f, cursorPos.y - 48.f));
					ImGui::BeginGroup();
					{
						ImGui::Text(MapsPerLineText.c_str()); //"Maps Per Line : "
						ImGui::SetNextItemWidth(80.f);
						if (ImGui::BeginCombo("##mapsinline", std::to_string(nbTilesPerLine).c_str()))
						{
							for (int i = 2; i <= 14; i++)
							{
								if (ImGui::Selectable(std::to_string(i).c_str()))
								{
									nbTilesPerLine = i;
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
					if (MapsDisplayMode == 0)
					{
						TextureID_DisplayMode1Image = MapsDisplayMode_Logo1_SelectedImage->GetImGuiTex();
					}
					renderImageButton(TextureID_DisplayMode1Image, ImVec2(36.f, 36.f), [this]() {
						MapsDisplayMode = 0;
						cvarManager->log("MapsDisplayMode set to : 0");
						SaveInCFG();
						});

					ImGui::SameLine();

					ImTextureID TextureID_DisplayMode2Image = MapsDisplayMode_Logo2_Image->GetImGuiTex();
					if (MapsDisplayMode == 1)
					{
						TextureID_DisplayMode2Image = MapsDisplayMode_Logo2_SelectedImage->GetImGuiTex();
					}
					renderImageButton(TextureID_DisplayMode2Image, ImVec2(36.f, 36.f), [this]() {
						MapsDisplayMode = 1;
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

				std::string ProgressBar_Label = convertToMB(std::to_string(DownloadTextrures_ProgressDisplayed)) + " / " + convertToMB(std::to_string(46970000));
				renderProgressBar(DownloadTextrures_ProgressDisplayed, 46970000.f, ImGui::GetCursorScreenPos(), ImVec2(1305.f, 24.f),
					ImColor(112, 112, 112, 255), ImColor(33, 65, 103, 255), ProgressBar_Label.c_str());

				ImGui::Separator();
			}

			renderQuickSearch();

			renderMaps(controller1);

			ImGui::EndTabItem();
		}


		if (ImGui::BeginTabItem(Tab3SearchWorkshopText.c_str()))
		{
			static char keyWord[200] = "";
			ImGui::BeginGroup();
			{
				ImGui::Text(Label3Text.c_str()); // "Search A Workshop :"

				ImGui::SetNextItemWidth(308.f);
				ImGui::InputText("##RLMAPSworkshopkeyword", keyWord, IM_ARRAYSIZE(keyWord));


				if (RLMAPS_Searching)
				{
					SearchButtonText = SearchingText;
				}

				if (ImGui::Button(SearchButtonText.c_str(), ImVec2(308.f, 25.f)) && !RLMAPS_Searching && std::string(keyWord) != "") // "Search"
				{
					std::thread t2(&Pluginx64::GetResults, this, std::string(keyWord), 1);
					t2.detach();
				}
				ImGui::EndGroup();
			}

			ImGui::SameLine();

			try
			{
				CenterNexIMGUItItem(63.f);
				ImGui::Image(RLMAPSLogoImage->GetImGuiTex(), ImVec2(63.f, 63.f)); //rocketleaguemaps.us Logo
			}
			catch (const std::exception& ex)
			{
				cvarManager->log(ex.what());
			}

			ImGui::SameLine();

			AlignRightNexIMGUItItem(180.f, 8.f);
			if (ImGui::Button(BrowseMapsText.c_str(), ImVec2(180.f, 65.f)) && !RLMAPS_Searching)//Browse Maps
			{
				strncpy(keyWord, "", IM_ARRAYSIZE(""));
				std::thread t2(&Pluginx64::GetResults, this, "", 1);
				t2.detach();
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
				ImGui::Separator();

				std::string ProgressBar_Label = convertToMB(std::to_string(RLMAPS_WorkshopDownload_Progress)) + " / " + convertToMB(std::to_string(RLMAPS_WorkshopDownload_FileSize));
				renderProgressBar(RLMAPS_WorkshopDownload_Progress, RLMAPS_WorkshopDownload_FileSize, ImGui::GetCursorScreenPos(), ImVec2(1305.f, 24.f),
					ImColor(112, 112, 112, 255), ImColor(33, 65, 103, 255), ProgressBar_Label.c_str());
			}

			ImGui::Separator();

			ImGui::BeginChild("##RLMAPSSearchWorkshopMapsResults");
			{
				ImGui::Text("%s %d / %d", WorkshopsFoundText.c_str(), RLMAPS_SearchWorkshopDisplayed, RLMAPS_NumberOfMapsFound); // "Workshops Found : 0 / 0"

				ImGui::SameLine();


				if (RLMAPS_PageSelected > 1)
				{
					if (ImGui::Button("Previous Page", ImVec2(100.f, 25.f)) && !RLMAPS_Searching)
					{
						std::thread t2(&Pluginx64::GetResults, this, std::string(keyWord), RLMAPS_PageSelected - 1);
						t2.detach();
					}
				}

				ImGui::SameLine();

				for (int i = RLMAPS_PageSelected - 4; i <= RLMAPS_PageSelected + 4; i++)
				{
					if (i <= NumPages && i > 0)
					{
						std::string pageName = "Page " + std::to_string(i);
						if (RLMAPS_PageSelected != i)
						{
							if (ImGui::Button(pageName.c_str(), ImVec2(55.f, 25.f)) && !RLMAPS_Searching)
							{
								std::thread t2(&Pluginx64::GetResults, this, std::string(keyWord), i);
								t2.detach();
							}
							ImGui::SameLine();
						}
						else
						{
							ImGui::Text("Current page : %d", RLMAPS_PageSelected);
							ImGui::SameLine();
						}
					}
					
				}

				if (RLMAPS_PageSelected < NumPages - 1)
				{
					if (ImGui::Button("Next Page", ImVec2(100.f, 25.f)) && !RLMAPS_Searching)
					{
						std::thread t2(&Pluginx64::GetResults, this, std::string(keyWord), RLMAPS_PageSelected + 1);
						t2.detach();
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



void Pluginx64::renderMaps(Gamepad controller)
{
	mapButtonList.clear();

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
	
	MapButtonChild_TopPos = ImGui::GetCursorScreenPos();
	if (ImGui::BeginChild("#MapsLauncherButtons"))
	{
		float windowWidth = ImGui::GetContentRegionAvailWidth();
		float buttonWidth = (windowWidth - ((nbTilesPerLine - 1) * 8)) / nbTilesPerLine;
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

				mapButtonPos buttonMap;
				buttonMap.rectMin = ImGui::GetItemRectMin();
				//cvarManager->log("rectmin : " + std::to_string(buttonMap.rectMin.y));
				buttonMap.rectMax = ImGui::GetItemRectMax();
				//cvarManager->log("rectmax : " + std::to_string(buttonMap.rectMax.y));
				buttonMap.cursorPos = ImVec2(((buttonMap.rectMax.x - buttonMap.rectMin.x) / 2) + buttonMap.rectMin.x, ((buttonMap.rectMax.y - buttonMap.rectMin.y) / 2) + buttonMap.rectMin.y);

				if (buttonMap.cursorPos.y < ImGui::GetWindowDrawList()->GetClipRectMax().y && buttonMap.cursorPos.y > MapButtonChild_TopPos.y)
				{
					//cvarManager->log(map.mapName + " : visible");
					buttonMap.isDisplayed = true;
				}
				else
				{
					//cvarManager->log(map.mapName + " : non visible");
					buttonMap.isDisplayed = false;
				}


				mapButtonList.push_back(buttonMap);

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

		std::vector<bool> isHoveringMapButtonList;

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

			if (MapsDisplayMode == 0)
			{
				renderMaps_DisplayMode_0(curMap);

				/*
				//not working well
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					// Set payload to carry the index of our item (could be anything)
					ImGui::SetDragDropPayload("DND_DEMO_CELL", &ID, sizeof(int));

					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
					{
						IM_ASSERT(payload->DataSize == sizeof(int));
						int payload_n = *(const int*)payload->Data;
						
						//https://stackoverflow.com/questions/45447361/how-to-move-certain-elements-of-stdvector-to-a-new-index-within-the-vector
						if (payload_n > ID)
							std::rotate(MapList.rend() - payload_n - 1, MapList.rend() - payload_n, MapList.rend() - ID);
						else
							std::rotate(MapList.begin() + payload_n, MapList.begin() + payload_n + 1, MapList.begin() + ID + 1);
							
					}
					ImGui::EndDragDropTarget();
				}*/

				ImGui::PopID();
				ID++;
			}
			else
			{
				renderMaps_DisplayMode_1(curMap, buttonWidth);
				nbTilesOnTheCurrentLine++;
				ImGui::PopID();
				ID++;
				if (nbTilesOnTheCurrentLine == nbTilesPerLine)
				{
					nbTilesOnTheCurrentLine = 0;
					ImGui::NewLine();
				}
				else
				{
					ImGui::SameLine();
				}
			}

			if (ImGui::IsItemHovered())
			{
				//cvarManager->log(std::to_string(ID));
				isHoveringMapButtonList.push_back(true);
				selectedButton = ID - 1;
			}
			else
			{
				isHoveringMapButtonList.push_back(false);
			}
		}

		for (int i = 0; i < isHoveringMapButtonList.size(); i++)
		{
			isHoveringMapButton = false;

			if (isHoveringMapButtonList.at(i) == true)
			{
				isHoveringMapButton = true;
				break;
			}
		}


		float rightStickY = controller.RightStick_Y();

		static bool DpadUpWasPressed = false;
		static bool DpadDownWasPressed = false;
		static bool DpadLeftWasPressed = false;
		static bool DpadRightWasPressed = false;

		if (controller.Connected())
		{
			if (controller.checkButtonPress(XINPUT_GAMEPAD_DPAD_UP) && !DpadUpWasPressed) {
				//cvarManager->log("dpad up is pressed");
				DpadUpWasPressed = true;
			}
			else if (!controller.checkButtonPress(XINPUT_GAMEPAD_DPAD_UP) && DpadUpWasPressed)
			{
				if (mapButtonList.size() > 0)
				{
					if (selectedButton != 0)
					{
						if (MapsDisplayMode == 0)
							selectedButton -= 1;
						if (MapsDisplayMode == 1 && (selectedButton - nbTilesPerLine) >= 0)
							selectedButton -= nbTilesPerLine;
					}
					mapButtonPos buttonMap = mapButtonList.at(selectedButton);

					if (buttonMap.isDisplayed)
					{
						SetCursorPos(buttonMap.cursorPos.x, buttonMap.cursorPos.y);
					}
					else
					{
						ImGui::SetScrollY(ImGui::GetScrollY() - (MapButtonChild_TopPos.y - buttonMap.rectMin.y));
						buttonMap.cursorPos.y += (MapButtonChild_TopPos.y - buttonMap.rectMin.y);
						SetCursorPos(buttonMap.cursorPos.x, buttonMap.cursorPos.y);
					}
				}
				

				//cvarManager->log("dpad up is realeased");
				DpadUpWasPressed = false;
			}

			if (controller.checkButtonPress(XINPUT_GAMEPAD_DPAD_DOWN) && !DpadDownWasPressed) {
				//cvarManager->log("dpad down is pressed");
				DpadDownWasPressed = true;
			}
			else if (!controller.checkButtonPress(XINPUT_GAMEPAD_DPAD_DOWN) && DpadDownWasPressed)
			{
				if (mapButtonList.size() > 0)
				{
					if (selectedButton != mapButtonList.size() - 1)
					{
						if (MapsDisplayMode == 0)
							selectedButton += 1;
						if (MapsDisplayMode == 1)
						{
							if ((selectedButton + nbTilesPerLine) > mapButtonList.size() - 1)
							{
								selectedButton = mapButtonList.size() - 1;
							}
							else
							{
								selectedButton += nbTilesPerLine;
							}
						}
					}
					mapButtonPos buttonMap = mapButtonList.at(selectedButton);

					if (buttonMap.isDisplayed)
					{
						SetCursorPos(buttonMap.cursorPos.x, buttonMap.cursorPos.y);
					}
					else
					{
						ImGui::SetScrollY((buttonMap.rectMax.y - ImGui::GetWindowDrawList()->GetClipRectMax().y) + ImGui::GetScrollY());
						buttonMap.cursorPos.y -= (buttonMap.rectMax.y - ImGui::GetWindowDrawList()->GetClipRectMax().y);
						SetCursorPos(buttonMap.cursorPos.x, buttonMap.cursorPos.y);
					}
				}

				//cvarManager->log("dpad down is realeased");
				DpadDownWasPressed = false;
			}

			if (controller.checkButtonPress(XINPUT_GAMEPAD_DPAD_LEFT) && !DpadLeftWasPressed) {
				//cvarManager->log("dpad left is pressed");
				DpadLeftWasPressed = true;
			}
			else if (!controller.checkButtonPress(XINPUT_GAMEPAD_DPAD_LEFT) && DpadLeftWasPressed)
			{
				if (mapButtonList.size() > 0)
				{
					if (MapsDisplayMode == 1 && selectedButton != 0)
						selectedButton -= 1;


					mapButtonPos buttonMap = mapButtonList.at(selectedButton);

					if (buttonMap.isDisplayed)
					{
						SetCursorPos(buttonMap.cursorPos.x, buttonMap.cursorPos.y);
					}
					else
					{
						ImGui::SetScrollY((buttonMap.rectMax.y - ImGui::GetWindowDrawList()->GetClipRectMax().y) + ImGui::GetScrollY());
						buttonMap.cursorPos.y -= (buttonMap.rectMax.y - ImGui::GetWindowDrawList()->GetClipRectMax().y);
						SetCursorPos(buttonMap.cursorPos.x, buttonMap.cursorPos.y);
					}
				}


				//cvarManager->log("dpad left is realeased");
				DpadLeftWasPressed = false;
			}

			if (controller.checkButtonPress(XINPUT_GAMEPAD_DPAD_RIGHT) && !DpadRightWasPressed) {
				//cvarManager->log("dpad right is pressed");
				DpadRightWasPressed = true;
			}
			else if (!controller.checkButtonPress(XINPUT_GAMEPAD_DPAD_RIGHT) && DpadRightWasPressed)
			{
				if (mapButtonList.size() > 0)
				{
					if (MapsDisplayMode == 1 && selectedButton < mapButtonList.size() - 1)
						selectedButton++;

					mapButtonPos buttonMap = mapButtonList.at(selectedButton);

					if (buttonMap.isDisplayed)
					{
						SetCursorPos(buttonMap.cursorPos.x, buttonMap.cursorPos.y);
					}
					else
					{
						ImGui::SetScrollY((buttonMap.rectMax.y - ImGui::GetWindowDrawList()->GetClipRectMax().y) + ImGui::GetScrollY());
						buttonMap.cursorPos.y -= (buttonMap.rectMax.y - ImGui::GetWindowDrawList()->GetClipRectMax().y);
						SetCursorPos(buttonMap.cursorPos.x, buttonMap.cursorPos.y);
					}
				}

				//cvarManager->log("dpad right is realeased");
				DpadRightWasPressed = false;
			}

			if (!controller.RStick_InDeadzone())
			{
				ImGui::SetScrollY(ImGui::GetScrollY() - (rightStickY * ControllerScrollSensitivity));
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
			gameWrapper->Execute([&, map](GameWrapper* gw)
				{
					cvarManager->executeCommand("load_workshop \"" + map.Folder.string() + "/" + map.UpkFile.filename().string() + "\"");
				});
			cvarManager->log("Map selected : " + map.UpkFile.filename().string());
		}



		mapButtonPos buttonMap;
		buttonMap.rectMin = ImGui::GetItemRectMin();
		//cvarManager->log("rectmin : " + std::to_string(buttonMap.rectMin.y));
		buttonMap.rectMax = ImGui::GetItemRectMax();
		//cvarManager->log("rectmax : " + std::to_string(buttonMap.rectMax.y));
		buttonMap.cursorPos = ImVec2(((buttonMap.rectMax.x - buttonMap.rectMin.x) / 2) + buttonMap.rectMin.x, ((buttonMap.rectMax.y - buttonMap.rectMin.y) / 2) + buttonMap.rectMin.y);

		if (buttonMap.cursorPos.y < ImGui::GetWindowDrawList()->GetClipRectMax().y && buttonMap.cursorPos.y > MapButtonChild_TopPos.y)
		{
			//cvarManager->log(map.mapName + " : visible");
			buttonMap.isDisplayed = true;
		}
		else
		{
			//cvarManager->log(map.mapName + " : non visible");
			buttonMap.isDisplayed = false;
		}


		mapButtonList.push_back(buttonMap);

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
			

			//responsive description but it takes too much ressources and causes fps issues for not good PC (like my PC xD)
			/*
			float descriptionWidth = ((windowWidth - 214) * 0.867f);
			std::string mapDescription = GetJSONLocalMapInfos(map.JsonFile).at(1);
			std::vector<std::string> mapDescriptionParts;
			if (ImGui::CalcTextSize(mapDescription.c_str()).x > descriptionWidth)
			{
				mapDescriptionParts.push_back(LimitTextSize(mapDescription, descriptionWidth)); //first line of the description
				mapDescription.erase(mapDescription.find(mapDescriptionParts.at(0)), mapDescriptionParts.at(0).length()); //remove the first line in description
				mapDescriptionParts.push_back(LimitTextSize(mapDescription, descriptionWidth)); //second line of the description

				if (mapDescription.length() > mapDescriptionParts.at(1).length())
				{
					mapDescription = mapDescriptionParts.at(0) + "\n" + mapDescriptionParts.at(1) + "...";
				}
				else
				{
					mapDescription = mapDescriptionParts.at(0) + "\n" + mapDescriptionParts.at(1);
				}
			}
			*/

			draw_list->AddText(fontA, 25.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 2.f), ImColor(255, 255, 255, 255),
				map.mapName.c_str()); //Map title
			draw_list->AddText(fontA, 15.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 40.f), ImColor(200, 200, 200, 255), GoodDescription.c_str()); //Map Description
			draw_list->AddText(fontA, 15.f, ImVec2(ImageMax.x + 4.f, ButtonRectMin.y + 90.f), ImColor(0, 200, 255, 255),
				std::string(ResultByText.c_str() + map.mapAuthor).c_str()); // "By " Map Author
		}

		ImGui::EndGroup();

		if (ImGui::BeginPopupContextItem("Map context menu"))
		{
			if (ImGui::Selectable(OpenMapDirText.c_str())) // "Open map directory"
			{
				std::wstring w_CurrentMapsDir = s2ws(map.Folder.string());
				LPCWSTR L_CurrentMapsDir = w_CurrentMapsDir.c_str();

				ShellExecute(NULL, L"open", L_CurrentMapsDir, NULL, NULL, SW_SHOWDEFAULT);
			}

			if (ImGui::Selectable(DeleteMapText.c_str())) // "Delete Map"
			{
				fs::remove_all(map.Folder);
				RefreshMapsFunct(MapsFolderPathBuf);
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
			gameWrapper->Execute([&, map](GameWrapper* gw)
				{
					cvarManager->executeCommand("load_workshop \"" + map.Folder.string() + "/" + map.UpkFile.filename().string() + "\"");
				});
			cvarManager->log("Map selected : " + map.UpkFile.filename().string());
		}


		mapButtonPos buttonMap;
		buttonMap.rectMin = ImGui::GetItemRectMin();
		//cvarManager->log("rectmin : " + std::to_string(buttonMap.rectMin.y));
		buttonMap.rectMax = ImGui::GetItemRectMax();
		//cvarManager->log("rectmax : " + std::to_string(buttonMap.rectMax.y));
		buttonMap.cursorPos = ImVec2(((buttonMap.rectMax.x - buttonMap.rectMin.x) / 2) + buttonMap.rectMin.x, ((buttonMap.rectMax.y - buttonMap.rectMin.y) / 2) + buttonMap.rectMin.y);

		if (buttonMap.cursorPos.y < ImGui::GetWindowDrawList()->GetClipRectMax().y && buttonMap.cursorPos.y > MapButtonChild_TopPos.y)
		{
			//cvarManager->log(map.mapName + " : visible");
			buttonMap.isDisplayed = true;
		}
		else
		{
			//cvarManager->log(map.mapName + " : non visible");
			buttonMap.isDisplayed = false;
		}


		mapButtonList.push_back(buttonMap);


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
			if (ImGui::Selectable(OpenMapDirText.c_str())) // "Open map directory"
			{
				std::wstring w_CurrentMapsDir = s2ws(map.Folder.string());
				LPCWSTR L_CurrentMapsDir = w_CurrentMapsDir.c_str();

				ShellExecute(NULL, L"open", L_CurrentMapsDir, NULL, NULL, SW_SHOWDEFAULT);
			}

			if (ImGui::Selectable(DeleteMapText.c_str())) // "Delete Map"
			{
				fs::remove_all(map.Folder);
				RefreshMapsFunct(MapsFolderPathBuf);
			}
			ImGui::EndPopup();
		}
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


void Pluginx64::RLMAPS_renderSearchWorkshopResults(static char mapspath[200])
{
	int LinesNb = 0;
	RLMAPS_SearchWorkshopDisplayed = 0;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();


	int nbResultPerLine = 4;

	if (ImGui::GetWindowWidth() > 1560 && ImGui::GetWindowWidth() < 1830)
	{
		nbResultPerLine = 5;
	}
	else if (ImGui::GetWindowWidth() >= 1830)
	{
		nbResultPerLine = 6;
	}

	for (int i = 0; i < RLMAPS_MapResultList.size(); i++)
	{
		if (LinesNb < nbResultPerLine)
		{
			RLMAPS_RenderAResult(i, draw_list, mapspath);

			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 73.f); //the float is the spacing between 2 results (+8 because of sameline())
			LinesNb++;
		}
		else
		{
			RLMAPS_RenderAResult(i, draw_list, mapspath);

			ImGui::NewLine();
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
	//std::string mapSize = mapResult.Size;
	std::string mapDescription = mapResult.Description;
	std::string mapAuthor = mapResult.Author;


	ImGui::BeginChild("##RlmapsResult", ImVec2(190.f, 260.f));
	{
		ImGui::BeginGroup();
		{
			//std::string SizeConverted = ResultSizeText + convertToMB(mapSize);
			//std::string SizeConverted = mapSize;

			ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
			ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + 190.f, TopCornerLeft.y + 260.f);
			ImVec2 ImageP_Min = ImVec2(TopCornerLeft.x + 6.f, TopCornerLeft.y + 6.f);
			ImVec2 ImageP_Max = ImVec2(TopCornerLeft.x + 184.f, TopCornerLeft.y + 179.f);

			drawList->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(44, 75, 113, 255), 5.f, 15); //Blue rectangle
			drawList->AddRect(ImageP_Min, ImageP_Max, ImColor(255, 255, 255, 255), 0, 15, 2.0F); //Image white outline

			if (mapResult.isImageLoaded == true)
			{
				try
				{
					if (mapResult.Image != nullptr)
					{
						if (mapResult.Image->GetImGuiTex())
						{
							drawList->AddImage(mapResult.Image->GetImGuiTex(), ImageP_Min, ImageP_Max); //Map image preview
						}
					}
				}
				catch (const std::exception& ex)
				{
					//cvarManager->log(ex.what());
				}
			}

			/*
			std::string GoodMapName = mapName.substr(0, 29);
			if (mapName.length() > 31)
			{
				GoodMapName.append("...");
			}
			*/
			std::string GoodMapName = mapName;
			if (ImGui::CalcTextSize(GoodMapName.c_str()).x > (186.f * 0.982f))
			{
				GoodMapName = LimitTextSize(GoodMapName, (186.f * 0.982f) - ImGui::CalcTextSize("...").x) + "...";
			}
			drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 185.f), ImColor(255, 255, 255, 255), GoodMapName.c_str()); //Map title
			//drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 200.f), ImColor(255, 255, 255, 255), SizeConverted.c_str()); //Map size
			drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 215.f), ImColor(255, 255, 255, 255),
				std::string(ResultByText.c_str() + mapAuthor).c_str()); // "By : " Map Author
			ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 235.f));
			if (ImGui::Button(DownloadMapButtonText.c_str(), ImVec2(182, 20))) // "Download Map"																								//Map download button
			{
				if (RLMAPS_IsDownloadingWorkshop == false && IsRetrievingWorkshopFiles == false && Directory_Or_File_Exists(fs::path(mapspath)))
				{
					ImGui::OpenPopup("Releases");
				}
			}

			renderReleases(mapResult);

			//Popup if is a downlaod is in progress and user wants to start a new download
			renderInfoPopup("Downloading?", IsDownloadDingWarningText.c_str());

			//Popup if maps directory doesn't exist
			renderInfoPopup("Exists?", DirNotExistText.c_str());

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
		ImGui::EndChild();
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
	std::wstring w_LINK = s2ws(link);
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


void Pluginx64::renderExtractMapFilesPopup(Map curMap)
{
	if (ImGui::BeginPopupModal("ExtractMapFiles", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string message = EMFMessageText1 + curMap.ZipFile.filename().string() + EMFMessageText2; //The map isn't extracted from              \nChoose an extract method (you need to click on refresh maps after extracting) :
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
			CreateUnzipBatchFile(curMap.Folder.string() + "/", curMap.ZipFile.string());
		}
		ImGui::SameLine();
		if (ImGui::Button(EMFStillDoesntWorkText.c_str(), ImVec2(110.f, 25.f)))//"Still doesn't work"
		{
			ImGui::OpenPopup("ExtractManually");
		}

		if (ImGui::BeginPopupModal("ExtractManually", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			std::string txt = EMLabelText + curMap.ZipFile.filename().string(); //"If both of the extract methods didn't work, you need to extract the files manually of "
			ImGui::Text(txt.c_str());
			ImGui::Text("Tutorial : ");
			ImGui::SameLine();
			renderLink("https://youtu.be/mI2PqkissiQ?t=124");
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
			if (ImGui::Button(CancelText.c_str(), ImVec2(100.f, 25.f)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}


		CenterNexIMGUItItem(326.f);
		if (ImGui::Button(CancelText.c_str(), ImVec2(326.f, 25.f)))
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
			if (ImGui::Button(YESButtonText.c_str(), ImVec2(100.f, 25.f)))//YES
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
			if (ImGui::Button(NOButtonText.c_str(), ImVec2(100.f, 25.f)))//NO
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

void Pluginx64::renderReleases(RLMAPS_MapResult mapResult)
{
	if (ImGui::BeginPopupModal("Releases", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		for (int releasesIndex = 0; releasesIndex < mapResult.releases.size(); releasesIndex++)
		{
			RLMAPS_Release release = mapResult.releases[releasesIndex];

			if (ImGui::Button(release.tag_name.c_str(), ImVec2(182, 20))) // "Download Map"																								//Map download button
			{
				if (RLMAPS_IsDownloadingWorkshop == false && IsRetrievingWorkshopFiles == false && Directory_Or_File_Exists(fs::path(MapsFolderPathBuf)))
				{
					std::thread t2(&Pluginx64::RLMAPS_DownloadWorkshop, this, MapsFolderPathBuf, mapResult, release);
					t2.detach();
				}
				else
				{
					if (!Directory_Or_File_Exists(fs::path(MapsFolderPathBuf)))
					{
						ImGui::OpenPopup("Exists?");
					}

					if (RLMAPS_IsDownloadingWorkshop || IsRetrievingWorkshopFiles)
					{
						ImGui::OpenPopup("Downloading?");
					}
				}
			}
		}
		

		//Popup if is a downlaod is in progress and user wants to start a new download
		renderInfoPopup("Downloading?", IsDownloadDingWarningText.c_str());

		//Popup if maps directory doesn't exist
		renderInfoPopup("Exists?", DirNotExistText.c_str());
		
		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button("Close", ImVec2(100.f, 25.f)))
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
			ImGui::Text(DLTLabel1Text.c_str());//"It seems like the workshop textures aren't installed in %s", RLCookedPCConsole_Path.string().c_str()
			ImGui::Text(DLTLabel2Text.c_str());//"You can still play without the workshop textures but some maps will have some white/weird textures."
			if (IsDownloading_WorkshopTextures)
			{
				ImGui::Separator();

				std::string ProgressBar_Label = convertToMB(std::to_string(DownloadTextrures_ProgressDisplayed)) + " / " + convertToMB(std::to_string(46970000));
				renderProgressBar(DownloadTextrures_ProgressDisplayed, 46970000.f, ImGui::GetCursorScreenPos(), ImVec2(1305.f, 24.f),
					ImColor(112, 112, 112, 255), ImColor(33, 65, 103, 255), ProgressBar_Label.c_str());

				ImGui::Separator();
			}

			ImGui::NewLine();

			float height = ((missingTextureFiles.size() - 1) * 21) + 60;

			CenterNexIMGUItItem(250.f);
			ImGui::BeginChild("##MissingFilesTable", ImVec2(250.f, height), true);
			{
				std::string MissingFilesTxt = DLTMissingFilesText + "(" + std::to_string(missingTextureFiles.size()) + ") :";//"Missing Files :"
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

			CenterNexIMGUItItem(383.f);
			if (ImGui::Button(DownloadButtonText.c_str(), ImVec2(100.f, 25.f)))//"Download"
			{
				std::thread t2(&Pluginx64::DownloadWorkshopTextures, this);
				t2.detach();
			}
			ImGui::SameLine();
			if (ImGui::Button(CloseText.c_str(), ImVec2(100.f, 25.f)))//"Close"
			{
				DownloadTexturesBool = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button(DontAskText.c_str(), ImVec2(150.f, 25.f)))//"Don't ask me again"
			{
				dontAsk = 1;
				SaveInCFG();
				DownloadTexturesBool = false;
				ImGui::CloseCurrentPopup();
			}
		}
		else
		{
			ImGui::Text(DLTTexturesInstalledText.c_str());//"Workshop textures installed !"
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
	static bool french = false;
	if (ImGui::BeginPopupModal("New Update", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string AnnouncementText;
		std::string AddedText;
		std::string FixedText;
		std::string FixedText1;
		std::string ChangedText;
		std::string ChangedLabel1Text;
		std::string AnnoucementText1;
		std::string AnnoucementText2;
		std::string AnnoucementText3;
		std::string AnnoucementText4;
		std::string AddMapText;
		std::string MainText1;
		std::string MainText2;
		std::string RemovedText;
		std::string RemovedText1;
		std::string RemovedText2;



		if (!french)
		{
			MainText1 = "Sorry for all of you that I didn't responded to about your issues, I had way too many messages from you.";
			MainText2 = "I think I can't really do anything for the maps that are loading forever or crashing the game. I don't think that the issue comes from my plugin.";

			FixedText = "Fixed :";
			FixedText1 = "- Fixed a random crash happening when searching workshop maps.";

			ChangedText = "Changed :";
			ChangedLabel1Text = "- The fix for the crashing issue when loading maps is no longer enabled by default, to activate it go settings->Enable Freeze Fix";

			AddedText = "Added :";
			AddMapText = "- If you download maps from a website or other than from the plugin, you can now add a map manually (\"Add Map\" button in \"Map Loader\" tab).";

			RemovedText = "Removed :";
			RemovedText1 = "- I removed everything that was related to Multiplayer because it was confusing for a lot of people that didn't know how to use it, also I don't think there are actually people using it.";
			RemovedText2 = " If you really want to play multiplayer workshop maps, just use Rocket Plugin or Rocket Host as it was made for that purpose. There are tons of tutorials on youtube.";

			AnnouncementText = "Announcement";
			AnnoucementText1 = "I've made a new plugin! It's called Looking For Mates (LFM).";
			AnnoucementText2 = "It's a plugin to find people to play with easily and quickly (something like the Overwatch \"Find A Group\" if you know) to play ranked, casual, extra mode, private match, etc...";
			AnnoucementText3 = "Find someone that you are interested in the list of players and click on him to receive a party invitation.";
			AnnoucementText4 = "I'm pretty proud of the result and I think it is a really great tool to share. The more people will use it, the more the plugin will be interesting. So if you are curious or interested, install it here :";
		}
		else
		{
			MainText1 = "Desole a tous ceux que je n'ai pas pu repondre a propos de vos problemes sur le plugin, je recevais vraiment beaucoup de messages.";
			MainText2 = "Je crois que je peux pas faire grand chose pour les maps qui chargent indefiniment ou qui font crasher le jeu. Je pense pas que le probleme vienne de mon plugin.";

			FixedText = "Corrections :";
			FixedText1 = "- Correction d'un crash qui arrivait de temps en temps en recherchant des maps workshops.";

			ChangedText = "Changements :";
			ChangedLabel1Text = "- Le fix pour les problemes de jeu qui crash en chargeant une map est plus active par defaut, pour l'activer settings->Enable Freeze Fix";

			AddedText = "Ajouts :";
			AddMapText = "- Si vous telechargez des maps sur un site web ou autre que depuis le plugin, vous pouvez desormais ajouter une map manuellement (le bouton \"Ajouter Map\" dans l'onglet \"Charger Map\")";

			RemovedText = "Retire :";
			RemovedText1 = "- J'ai retire tout ce qui etait en rapport avec le multijoueur car beaucoup de gens etaient confus et ne savais pas comment l'utiliser, et je pense pas qu'il y ait des gens qui l'utilise en vrai.";
			RemovedText2 = " Si vous voulez vraiment jouer en multijoueur, utilisez Rocket Plugin ou Rocket Host, c'est pour ca qu'ils ont ete cree. Il y a enormement de tutos sur youtube.";

			AnnouncementText = "Annonce";
			AnnoucementText1 = "J'ai creer un nouveau plugin! Ca s'appelle Looking For Mates (LFM)";
			AnnoucementText2 = "C'est un plugin pour trouver des gens avec qui jouer simplement et rapidement (comme \"Trouver Un Groupe\" sur Overwatch si vous connaissez) pour jouer en ranked, occasionnel, mode extra, match prive, etc...";
			AnnoucementText3 = "Trouvez quelqu'un qui vous interesse dans la liste de joueurs et cliquez sur lui pour recevoir une invitation dans son groupe.";
			AnnoucementText4 = "Je suis assez fier du resultat et je pense que c'est vraiment un bon outil a partager. Plus il y aura de gens qui l'utiliseront, plugin le plugin seras interessant, donc si vous etes interesse installez le ici :";
		}


		if (ImGui::BeginTabBar("NewUpdateTabBar"))
		{
			if (ImGui::BeginTabItem("Announcement"))
			{
				ImGui::NewLine();

				ImGui::Checkbox("French", &french);

				ImGui::NewLine();

				CenterNexIMGUItItem(CalcRealTextSize(std::string("/!\\ " + AnnouncementText + " /!\\").c_str(), 18.f).x);
				renderText(std::string("/!\\ " + AnnouncementText + " /!\\").c_str(), 18.f);
				renderUnderLine(ImColor(255, 255, 255, 150));

				ImGui::NewLine();

				ImGui::Text(AnnoucementText1.c_str());
				ImGui::Text(AnnoucementText2.c_str());
				ImGui::Text(AnnoucementText3.c_str());
				ImGui::NewLine();
				ImGui::Text(AnnoucementText4.c_str());
				renderLink("LFM link here");
				ImGui::SameLine();
				renderLink("LFM link here");
				ImGui::SameLine();
				renderLink("LFM link here");
				ImGui::SameLine();
				renderLink("LFM link here");
				ImGui::SameLine();
				renderLink("LFM link here");
				ImGui::SameLine();
				renderLink("LFM link here");
				ImGui::SameLine();
				renderLink("LFM link here");
				ImGui::NewLine();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(std::string("Changelog v" + PluginVersion).c_str())) // "Announcement"
			{
				ImGui::NewLine();

				ImGui::Checkbox("French", &french);

				ImGui::NewLine();

				CenterNexIMGUItItem(ImGui::CalcTextSize(std::string("Changelog v" + PluginVersion).c_str()).x);
				ImGui::Text(std::string("Changelog v" + PluginVersion).c_str());
				renderUnderLine(ImColor(255, 255, 255, 150));
				ImGui::NewLine();
				ImGui::NewLine();


				ImGui::Text(MainText1.c_str());
				ImGui::Text(MainText2.c_str());

				ImGui::NewLine();
				ImGui::NewLine();

				ImGui::Text(AddedText.c_str());
				renderUnderLine(ImColor(255, 255, 255, 150));
				ImGui::NewLine();
				ImGui::Text(AddMapText.c_str());

				ImGui::NewLine();

				ImGui::Text(RemovedText.c_str());
				renderUnderLine(ImColor(255, 255, 255, 150));
				ImGui::NewLine();
				ImGui::Text(RemovedText1.c_str());
				ImGui::Text(RemovedText2.c_str());

				ImGui::NewLine();

				ImGui::Text(FixedText.c_str());
				renderUnderLine(ImColor(255, 255, 255, 150));
				ImGui::NewLine();
				ImGui::Text(FixedText1.c_str());

				ImGui::NewLine();

				ImGui::Text(ChangedText.c_str());
				renderUnderLine(ImColor(255, 255, 255, 150));
				ImGui::NewLine();
				ImGui::Text(ChangedLabel1Text.c_str());

				ImGui::NewLine();

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		

		


		ImGui::NewLine();

		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
		{
			HasSeeNewUpdateAlert = true;
			SaveInCFG();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}


void Pluginx64::renderAddMapManuallyPopup()
{
	//ImGui::SetNextWindowSize(ImVec2(600.f, 429.f));
	if (ImGui::BeginPopupModal(AddMapText.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(NameText.c_str()); //"Name :"
		static char name[128] = "";
		ImGui::SetNextItemWidth(300.f);
		ImGui::InputText("##Name", name, IM_ARRAYSIZE(name));

		ImGui::Text(AuthorText.c_str()); //"Author :"
		static char author[128] = "";
		ImGui::SetNextItemWidth(300.f);
		ImGui::InputText("##Author", author, IM_ARRAYSIZE(author));

		ImGui::Text("Description :");
		static char description[1024 * 16] = "";
		ImGui::InputTextMultiline("##Description", description, IM_ARRAYSIZE(description), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 10));


		ImGui::Text(MapFilePathText.c_str()); //"Map File Path :"
		static char mapfilePath[256] = "";
		ImGui::SetNextItemWidth(400.f);
		ImGui::InputText("##MapFilePath", mapfilePath, IM_ARRAYSIZE(mapfilePath));

		ImGui::SameLine();

		if (ImGui::Button(SelectFileText.c_str())) // "Select File"
		{
			ImGui::OpenPopup("Select map file");
		}
		renderFileExplorerToAddMap(mapfilePath, {".udk", ".upk"});

		ImGui::Text(ImagePathText.c_str()); //"Image Path :"
		static char imagePath[256] = "";
		ImGui::SetNextItemWidth(400.f);
		ImGui::InputText("##ImagePath", imagePath, IM_ARRAYSIZE(imagePath));

		ImGui::SameLine();

		ImGui::PushID(1);
		if (ImGui::Button(SelectFileText.c_str())) // "Select File"
		{
			ImGui::OpenPopup("Select map file");
		}
		renderFileExplorerToAddMap(imagePath, { ".png", ".jpg", ".jpeg", ".jfif" });
		ImGui::PopID();

		ImGui::NewLine();

		if (ImGui::Button(CancelText.c_str(), ImVec2(100.f, 30.f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button(ConfirmText.c_str(), ImVec2(100.f, 30.f))) //"Confirm"
		{
			if (std::string(name) == "" || std::string(author) == "" || std::string(description) == "" || std::string(mapfilePath) == "" || std::string(imagePath) == "") //error
			{
				cvarManager->log("ERORRRRRR");
				ImGui::OpenPopup("Add Map Error");
			}
			else
			{
				ImGui::OpenPopup("Confirm Add Map");
			}
		}
		renderYesNoPopup("Confirm Add Map", ConfirmLabelText.c_str(), [this]() { //"Do you really want to add this map ?"
			AddMapManually(std::string(name), std::string(author), std::string(description), std::string(MapsFolderPathBuf), std::string(mapfilePath), std::string(imagePath));
			ImGui::CloseCurrentPopup();
			}, [this]() {
				ImGui::CloseCurrentPopup();
			});

		renderInfoPopup("Add Map Error", FieldEmptyText.c_str()); //"A field is empty !"

		ImGui::EndPopup();
	}
}


void Pluginx64::renderFileExplorer()
{
	ImGui::SetNextWindowSize(ImVec2(600.f, 429.f));
	if (ImGui::BeginPopupModal("Select maps folder", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char newFolderName[200] = "";

		static char fullPathBuff[256] = "C:/";
		std::filesystem::path currentPath = fullPathBuff;

		ImGui::BeginChild("##fullPath", ImVec2(ImGui::GetContentRegionAvailWidth(), 35.f), true);
		{
			ImGui::Columns(2, 0, true);
			ImGui::SetColumnWidth(0, 40.f);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.f);
			if(ImGui::Selectable("<--"))
			{
				strncpy(fullPathBuff, currentPath.parent_path().string().c_str(), IM_ARRAYSIZE(fullPathBuff));
			}

			ImGui::NextColumn();

			std::vector<std::string> Drives = GetDrives();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 108.f);
			if(ImGui::BeginCombo("", fullPathBuff))
			{
				for (auto drive : Drives)
				{
					drive += ":/";
					if (ImGui::Selectable(drive.c_str()))
					{
						strncpy(fullPathBuff, drive.c_str(), IM_ARRAYSIZE(fullPathBuff));
						currentPath = fullPathBuff;
					}
				}
				ImGui::EndCombo();
			}

			currentPath = fullPathBuff;

			ImGui::SameLine();

			if (ImGui::Button(NewFolderText.c_str(), ImVec2(100.f, 19.f)))
			{
				strncpy(newFolderName, "", IM_ARRAYSIZE(newFolderName));
				ImGui::OpenPopup("Folder Name");
			}
			if (ImGui::BeginPopupModal("Folder Name", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputText("##newFloderNameInputText", newFolderName, IM_ARRAYSIZE(newFolderName));
				if (ImGui::Button(ConfirmText.c_str(), ImVec2(100.f, 25.f)))
				{
					try
					{
						std::filesystem::create_directory(currentPath.string() + "/" + newFolderName);
					}
					catch (const std::exception& ex) //manage errors when trying to create a folder in an administrator folder
					{
						cvarManager->log(ex.what());
					}
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button(CancelText.c_str(), ImVec2(100.f, 25.f)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::EndChild();
		}
		
		ImGui::BeginChild("##directories", ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetWindowHeight() * 0.75f), true);
		{
			try
			{
				for (const auto& dir : fs::directory_iterator(currentPath))
				{
					if (dir.is_directory())
					{
						if (ImGui::Selectable(dir.path().filename().string().c_str()))
						{
							strncpy(fullPathBuff, dir.path().string().c_str(), IM_ARRAYSIZE(fullPathBuff));
						}
					}
				}
			}
			catch (const std::exception& ex)
			{
				cvarManager->log("error : " + std::string(ex.what()));
				strncpy(fullPathBuff, currentPath.parent_path().string().c_str(), IM_ARRAYSIZE(fullPathBuff));
			}

			ImGui::EndChild();
		}

		if (ImGui::Button(CancelText.c_str(), ImVec2(100.f, 30.f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button(SelectText.c_str(), ImVec2(100.f, 30.f))) //"Select"
		{
			strncpy(MapsFolderPathBuf, fullPathBuff, IM_ARRAYSIZE(MapsFolderPathBuf));
			SaveInCFG();
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::EndPopup();
	}
}


void Pluginx64::renderFileExplorerToAddMap(char* filefullPathBuff, std::vector<std::string> extensions)
{
	ImGui::SetNextWindowSize(ImVec2(600.f, 429.f));
	if (ImGui::BeginPopupModal("Select map file", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char newFolderName[200] = "";

		static char fullPathBuff[256] = "C:/";
		std::filesystem::path currentPath = fullPathBuff;

		ImGui::BeginChild("##fullPath", ImVec2(ImGui::GetContentRegionAvailWidth(), 35.f), true);
		{
			ImGui::Columns(2, 0, true);
			ImGui::SetColumnWidth(0, 40.f);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.f);
			if (ImGui::Selectable("<--"))
			{
				strncpy(fullPathBuff, currentPath.parent_path().string().c_str(), IM_ARRAYSIZE(fullPathBuff));
			}

			ImGui::NextColumn();

			std::vector<std::string> Drives = GetDrives();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 108.f);
			if (ImGui::BeginCombo("", fullPathBuff))
			{
				for (auto drive : Drives)
				{
					drive += ":/";
					if (ImGui::Selectable(drive.c_str()))
					{
						strncpy(fullPathBuff, drive.c_str(), IM_ARRAYSIZE(fullPathBuff));
						currentPath = fullPathBuff;
					}
				}
				ImGui::EndCombo();
			}

			currentPath = fullPathBuff;

			ImGui::SameLine();

			if (ImGui::Button(NewFolderText.c_str(), ImVec2(100.f, 19.f)))
			{
				strncpy(newFolderName, "", IM_ARRAYSIZE(newFolderName));
				ImGui::OpenPopup("Folder Name");
			}
			if (ImGui::BeginPopupModal("Folder Name", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputText("##newFloderNameInputText", newFolderName, IM_ARRAYSIZE(newFolderName));
				if (ImGui::Button(ConfirmText.c_str(), ImVec2(100.f, 25.f)))
				{
					try
					{
						std::filesystem::create_directory(currentPath.string() + "/" + newFolderName);
					}
					catch (const std::exception& ex) //manage errors when trying to create a folder in an administrator folder
					{
						cvarManager->log(ex.what());
					}
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button(CancelText.c_str(), ImVec2(100.f, 25.f)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::EndChild();
		}

		ImGui::BeginChild("##directories", ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetWindowHeight() * 0.75f), true);
		{
			try
			{
				for (const auto& dir : fs::directory_iterator(currentPath))
				{
					std::string dirName = dir.path().filename().string();
					std::string dirPath = dir.path().string();

					if (dir.is_directory())
					{
						if (ImGui::Selectable(dirName.c_str()))
						{
							strncpy(fullPathBuff, dirPath.c_str(), IM_ARRAYSIZE(fullPathBuff));
						}
					}
					else
					{
						std::string fileExtension = dir.path().filename().extension().string();
						auto checkExtensions = [&]() {
							for (std::string extension : extensions)
								if (extension == fileExtension)
									return true;
							return false;
						};

						if (checkExtensions())
						{
							bool isSelected = (std::string(filefullPathBuff) == dirPath);
							if (ImGui::Selectable(dirName.c_str(), isSelected))
							{
								if (isSelected)
									strncpy(filefullPathBuff, "", 256); //reset
								else
									strncpy(filefullPathBuff, dirPath.c_str(), 256); //select this file
							}
						}
					}
				}
			}
			catch (const std::exception& ex)
			{
				cvarManager->log("error : " + std::string(ex.what()));
				strncpy(fullPathBuff, currentPath.parent_path().string().c_str(), IM_ARRAYSIZE(fullPathBuff));
			}

			ImGui::EndChild();
		}

		if (ImGui::Button(CancelText.c_str(), ImVec2(100.f, 30.f)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button(SelectText.c_str(), ImVec2(100.f, 30.f))) //"Select"
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
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


	MapsFolderPath = MapsFolderPathBuf;
}
