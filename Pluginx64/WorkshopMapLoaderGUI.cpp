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
		SupportMeText = "Support Me & Patrons";
		SupportMePopupText = "You can donate or subscribe to my patreon if you want to support me and if you think my work deserves. Thank you.";
		DonateText = "Donate :";
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
		HostText = "Host Multiplayer Server";
		JoinServerText = "Join server";
		CancelText = "Cancel";

		//2nd Tab
		Tab2SearchWorkshopText = "Search Workshop(Steam)";
		DlWorkshopByURLText = "Download Workshop By Url(not working)";
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

		BrowseMapsText = "Browse Maps";

		//3rd Tab
		Tab3SearchWorkshopText = "Search Workshop(rocketleaguemaps.us)";

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
		PathSavedText = "Path saved successfully !";

		//LaunchMode
		LMLabel1Text = "There is no \"mods\" folder in " + RLCookedPCConsole_Path.string() + "\nDo you want to create it ?";
		LMLabel2Text = " isn't in CookedPCConsole. Paste the map to CookedPCConsole/mods ?";
		LMLabel3Text = "You need to restart Rocket Legaue first to be able to join a server on this map!";

		//HostGame
		GameSettingsText = "Game Settings :";
		GameModeText = "Game Mode :";
		NBOfPlayersText = "Number Of Players :";
		HostGameText = "Host Game";

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

		//MapsUnavailable
		MapsUnavailableText = "This map is unvailable to download for few seconds, wait.";

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
		SupportMeText = "Me Soutenir & Patrons";
		SupportMePopupText = "Vous pouvez me faire un don ou vous abonner a mon patreon si vous voulez me soutenir et si vous pensez que mon travail merite. Merci.";
		DonateText = "Faire un don :";
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
		SelectMapsFolderText = "Choisir Le Dossier Des Maps";
		RefreshMapsButtonText = "Rafraichir Les Maps";
		SavePathText = "Sauvegarder Le Chemin";
		MapsPerLineText = "Maps Par Ligne :";

		//context menu strip
		OpenMapDirText = "Ouvrir le dossier de la map";
		DeleteMapText = "Supprimer la map";

		//LauchMode Popup
		HostText = "Heberger Serveur Multijoueur";
		JoinServerText = "Rejoindre Serveur";
		CancelText = "Annuler";

		//2nd Tab
		Tab2SearchWorkshopText = "Rechercher Un Workshop(Steam)";
		DlWorkshopByURLText = "Telecharger Un Workshop Avec Une Url(not working)";
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

		BrowseMapsText = "Parcourir Les Maps";

		//3rd Tab
		Tab3SearchWorkshopText = "Rechercher Un Workshop(rocketleaguemaps.us)";

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
		PathSavedText = "Le chemin a ete sauvegarde !";


		//LaunchMode
		LMLabel1Text = "Il n'y a pas le de dossier \"mods\" dans " + RLCookedPCConsole_Path.string() + "\nCreer ce dossier ?";
		LMLabel2Text = " n'est pas dans CookedPCConsole. Copier la map dans CookedPCConsole/mods ?";
		LMLabel3Text = "Tu dois d'abord relancer Rocket League pour pouvoir rejoindre un serveur sur cette map!";

		//HostGame
		GameSettingsText = "Parametres :";
		GameModeText = "Mode De Jeu :";
		NBOfPlayersText = "Nombre De Joueurs :";
		HostGameText = "Heberger";

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

		//MapsUnavailable
		MapsUnavailableText = "Cette map est indisponible pendant quelques secondes, attends.";

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

	if (OpenSupportMePopup)
	{
		ImGui::OpenPopup("Support Me & Patrons");
	}
	renderSupportMePopup();


	if (!HasSeenIssuesEncountered && HasSeeNewUpdateAlert)
	{
		ImGui::OpenPopup("Issues Encountered");
	}
	if (ImGui::BeginPopupModal("Issues Encountered", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		CenterNexIMGUItItem(ImGui::CalcTextSize(WarningText.c_str()).x);
		ImGui::Text(WarningText.c_str()); //"Warning"
		ImGui::NewLine();

		for (auto issue : IssuesEncountered)
		{
			ImGui::Text(issue.c_str());
		}
		ImGui::NewLine();

		CenterNexIMGUItItem(100.f);
		if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
		{
			HasSeenIssuesEncountered = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	

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

			ImGui::EndMenu();
		}


		if (ImGui::BeginMenu(MultiplayerText.c_str())) //"Multiplayer"
		{
			if (ImGui::BeginMenu(MapsJoinableText.c_str())) //"Maps Joinable"
			{
				if (MapsAlreadyInCPCC.size() != 0)
				{
					for (auto mapInCPCC : MapsAlreadyInCPCC)
					{
						ImGui::Text(mapInCPCC.filename().string().c_str());
					}
				}
				else
				{
					ImGui::Text(NoMapsCanBeJoinText.c_str()); //"No maps can be joined"
				}
				ImGui::EndMenu();
			}

			if (ImGui::Selectable(JoinServerText.c_str())) //"Join Server"
			{
				JoinServerBool = true;
			}


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

		if (JoinServerBool) //I know this is not good but It works, so I don't care
		{
			ImGui::OpenPopup("JoinServer");
		}
		renderJoinServerPopup();

		if (DownloadTexturesBool) //I know this is not good but It works, so I don't care
		{
			ImGui::OpenPopup("DownloadTextures");
		}
		renderDownloadTexturesPopup(CheckExist_TexturesFiles());


		if (ImGui::Selectable(LastUpdateText.c_str(), false, 0, ImGui::CalcTextSize(LastUpdateText.c_str())))
		{
			HasSeeNewUpdateAlert = false;
		}

		if (ImGui::Selectable(SupportMeText.c_str(), false, 0, ImGui::CalcTextSize(SupportMeText.c_str())))
		{
			OpenSupportMePopup = true;
		}

		if (ImGui::BeginMenu("Credits"))
		{
			ImGui::Text("Plugin made by Vync#3866, contact me on discord for custom plugin commissions.");
			ImGui::NewLine();
			ImGui::Text("Thanks to PatteAuSucre for testing and Teyq for his help ;)");
			ImGui::Text("Thanks to JetFox for his help");

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

			if (ImGui::Button(SelectMapsFolderText.c_str(), ImVec2(204.f, 32.f))) // "Select maps folder"
			{
				ImGui::OpenPopup("Select maps folder");
			}
			renderFileExplorer();

			ImGui::SameLine();

			if (ImGui::Button(SavePathText.c_str(), ImVec2(204.f, 32.f))) // "Save Path"
			{
				if (Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\"))
				{
					SaveInCFG();

					ImGui::OpenPopup("SavePath");
				}
			}
			renderInfoPopup("SavePath", PathSavedText.c_str());


			ImGui::SameLine();

			std::vector<std::string> missingTexturesFiles = CheckExist_TexturesFiles();

			if (ImGui::Button(RefreshMapsButtonText.c_str(), ImVec2(204.f, 32.f))) // "Refresh Maps"
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

		if (ImGui::BeginTabItem(Tab2SearchWorkshopText.c_str())) //"Search Workshop"
		{
			if (ImGui::CollapsingHeader(DlWorkshopByURLText.c_str())) // "Download Workshop By Url"
			{
				ImGui::Text(Label2Text.c_str()); // "Steam Workshop Url :"
				static char url[200] = "put the url of a workshop";
				ImGui::InputText("##STEAMworkshopurl", url, IM_ARRAYSIZE(url));
				
				
				if (ImGui::Button(DownloadButtonText.c_str())) // "Download"
				{
					/*
					std::thread t1(&Pluginx64::STEAM_DownloadWorkshop, this, url, MapsFolderPathBuf, NULL, 0, false);
					t1.detach();
					*/
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

			try
			{
				CenterNexIMGUItItem(63.f);
				ImGui::Image(SteamLogoImage->GetImGuiTex(), ImVec2(63.f, 63.f)); //Steam Logo
			}
			catch (const std::exception& ex)
			{
				cvarManager->log(ex.what());
			}

			ImGui::SameLine();

			ImGui::BeginGroup();
			{
				std::string MostPopular_Url = "https://steamcommunity.com/workshop/browse/?appid=252950&browsesort=trend&section=readytouseitems";

				renderSortByCombos(MostPopular_Url);
				
				ImGui::SameLine();

				if (ImGui::Button(BrowseMapsText.c_str(), ImVec2(180.f, 65.f)) && !STEAM_Searching)//Browse Maps
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
				ImGui::Separator();
				
				std::string ProgressBar_Label = convertToMB(std::to_string(STEAM_WorkshopDownload_Progress)) + " / " + convertToMB(std::to_string(STEAM_WorkshopDownload_FileSize));
				renderProgressBar(STEAM_WorkshopDownload_Progress, STEAM_WorkshopDownload_FileSize, ImGui::GetCursorScreenPos(), ImVec2(1305.f, 24.f),
								  ImColor(112, 112, 112, 255), ImColor(33, 65, 103, 255), ProgressBar_Label.c_str());
			}

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

		if (ImGui::BeginTabItem(Tab3SearchWorkshopText.c_str()))
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

				if (ImGui::Button(SearchButtonText.c_str(), ImVec2(308.f, 25.f)) && !RLMAPS_Searching && std::string(keyWord) != "") // "Search"
				{
					std::thread t2(&Pluginx64::GetResults, this, std::string(combo_selected_searchingType), std::string(keyWord));
					t2.detach();

					RLMAPS_browsing = false;
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
				CurrentPage = 0;
				std::thread t2(&Pluginx64::GetResultsBrowseMaps, this, CurrentPage * 30);
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

				if (RLMAPS_browsing)
				{
					ImGui::BeginGroup();
					{
						if (CurrentPage > 0)
						{
							if (ImGui::Button("Previous Page", ImVec2(100.f, 25.f)) && !RLMAPS_Searching)
							{
								CurrentPage -= 1;
								std::thread t2(&Pluginx64::GetResultsBrowseMaps, this, CurrentPage * 30);
								t2.detach();
							}
						}

						ImGui::SameLine();

						for (auto page : listBrowsePages())
						{
							std::string pageName = "Page " + std::to_string(page + 1);
							if (page != CurrentPage)
							{
								if (ImGui::Button(pageName.c_str(), ImVec2(55.f, 25.f)) && !RLMAPS_Searching)
								{
									CurrentPage = page;
									std::thread t2(&Pluginx64::GetResultsBrowseMaps, this, CurrentPage * 30);
									t2.detach();
								}
								ImGui::SameLine();
							}
							else
							{
								ImGui::Text("Current page : %d", CurrentPage + 1);
								ImGui::SameLine();
							}
						}

						if (CurrentPage < (NBOfMapsOnSite / 30)) //(NBOfMapsOnSite / 30) is the number of pages
						{
							if (ImGui::Button("Next Page", ImVec2(100.f, 25.f)) && !RLMAPS_Searching)
							{
								CurrentPage++;
								std::thread t2(&Pluginx64::GetResultsBrowseMaps, this, CurrentPage * 30);
								t2.detach();
							}
						}
						ImGui::EndGroup();
					}
				}
				else
				{
					for (int i = 0; i < RLMAPS_Pages.size(); i++)
					{
						std::string pageName = "Page " + std::to_string(i + 1);
						if (RLMAPS_PageSelected != i)
						{
							if (ImGui::Button(pageName.c_str(), ImVec2(55.f, 25.f)))
							{
								RLMAPS_PageSelected = i;
							}
							ImGui::SameLine();
						}
						else
						{
							ImGui::Text("Current page : %d", RLMAPS_PageSelected + 1);
							ImGui::SameLine();
						}
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
			ImGui::OpenPopup("LaunchMode");
		}
		renderLaunchModePopup(map);



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
				draw_list->AddImage(map.PreviewImage->GetImGuiTex(), ImageMin, ImageMax); //Map image preview
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
			ImGui::OpenPopup("LaunchMode");
		}
		renderLaunchModePopup(map);


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
				draw_list->AddImage(map.PreviewImage->GetImGuiTex(), ImageMin, ImageMax); //Map image preview
			}
			catch (const std::exception& ex)
			{
				cvarManager->log(ex.what());
			}
		}

		ImFont* fontA = ImGui::GetDefaultFont();

		
		std::string mapTitle = map.mapName;
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


void Pluginx64::Steam_renderSearchWorkshopResults(static char mapspath[200])
{
	int LinesNb = 0;
	Steam_SearchWorkshopDisplayed = 0;

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

	for (int i = 0; i < Steam_MapResultList.size(); i++)
	{
		if (LinesNb < nbResultPerLine)
		{
			Steam_RenderAResult(i, draw_list, mapspath);

			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 73.f); //the float is the spacing between 2 results (+8 because of sameline())
			LinesNb++;
		}
		else
		{
			Steam_RenderAResult(i, draw_list, mapspath);

			ImGui::NewLine();
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



	ImGui::BeginChild("##SteamResult", ImVec2(190.f, 260.f));
	{
		ImGui::BeginGroup();
		{
			std::string SizeConverted;
			//cvarManager->log("mapSize : " + mapSize);

			if (mapSize.find_first_not_of("0123456789") == std::string::npos)
			{
				SizeConverted = ResultSizeText + convertToMB(mapSize);
			}
			else
			{
				SizeConverted = ResultSizeText + mapSize;
			}

			ImVec2 TopCornerLeft = ImGui::GetCursorScreenPos();
			ImVec2 RectFilled_p_max = ImVec2(TopCornerLeft.x + 190.f, TopCornerLeft.y + 260.f);
			ImVec2 ImageP_Min = ImVec2(TopCornerLeft.x + 6.f, TopCornerLeft.y + 6.f);
			ImVec2 ImageP_Max = ImVec2(TopCornerLeft.x + 184.f, TopCornerLeft.y + 179.f);

			drawList->AddRectFilled(TopCornerLeft, RectFilled_p_max, ImColor(44, 75, 113, 255), 5.f, 15); //Blue rectangle result
			drawList->AddRect(ImageP_Min, ImageP_Max, ImColor(255, 255, 255, 255), 0, 15, 2.0F); //Image white outline

			if (mapResult.isImageLoaded == true)
			{
				try
				{
					drawList->AddImage(mapResult.Image->GetImGuiTex(), ImageP_Min, ImageP_Max); //Map image preview
				}
				catch (const std::exception& ex)
				{
					cvarManager->log(ex.what());
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
			drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 200.f), ImColor(255, 255, 255, 255), SizeConverted.c_str()); //Map size
			drawList->AddText(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 215.f), ImColor(255, 255, 255, 255),
				std::string(ResultByText.c_str() + mapAuthor).c_str()); // "By : " Map Author
			ImGui::SetCursorScreenPos(ImVec2(TopCornerLeft.x + 4.f, TopCornerLeft.y + 235.f));
			if (ImGui::Button(DownloadMapButtonText.c_str(), ImVec2(182, 20))) // "Download Map"
			{
				if (mapResult.IsMapAvailable)
				{
					if (STEAM_IsDownloadingWorkshop == false && IsRetrievingWorkshopFiles == false && Directory_Or_File_Exists(fs::path(mapspath)))
					{
						std::thread t2(&Pluginx64::STEAM_DownloadWorkshop, this, "", mapspath, mapResult, true);
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
				else
				{
					ImGui::OpenPopup("Map Unavaiable");
				}
			}
			renderMapUnavaiablePopup();

			//Popup if maps directory doesn't exist
			renderInfoPopup("Exists?", DirNotExistText.c_str());

			//Popup if is a downlaod is in progress and user wants to start a new download
			renderInfoPopup("Downloading?", IsDownloadDingWarningText.c_str());

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
		ImGui::EndChild();
	}
	
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

	if (RLMAPS_Pages.size() != 0)
	{
		for (int i = 0; i < RLMAPS_Pages.at(RLMAPS_PageSelected).size(); i++)
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
	
}

void Pluginx64::RLMAPS_RenderAResult(int i, ImDrawList* drawList, static char mapspath[200])
{
	ImGui::PushID(i);

	RLMAPS_MapResult mapResult = RLMAPS_Pages.at(RLMAPS_PageSelected).at(i);
	std::string mapName = mapResult.Name;
	std::string mapSize = mapResult.Size;
	std::string mapDescription = mapResult.Description;
	std::string mapAuthor = mapResult.Author;


	ImGui::BeginChild("##RlmapsResult", ImVec2(190.f, 260.f));
	{
		ImGui::BeginGroup();
		{
			std::string SizeConverted = ResultSizeText + convertToMB(mapSize);

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
					drawList->AddImage(mapResult.Image->GetImGuiTex(), ImageP_Min, ImageP_Max); //Map image preview
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

void Pluginx64::renderLaunchModePopup(Map curMap)
{
	if (ImGui::BeginPopupModal("LaunchMode", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (isHoveringMapButton)
		{
			SetCursorPos(857, 522);
		}

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

		if (ImGui::Button(HostText.c_str(), ImVec2(200.f, 50.f))) //Host Multiplayer Server
		{
			ImGui::OpenPopup("HostGame");
		}

		renderHostGamePopup(curMap);


		if (ImGui::Button(JoinServerText.c_str(), ImVec2(200.f, 50.f)))//Join Server
		{
			ImGui::OpenPopup("JoinServer");
		}


		std::string modsDirPath = RLCookedPCConsole_Path.string() + "\\mods";
		if (!Directory_Or_File_Exists(modsDirPath))
		{
			renderYesNoPopup("JoinServer", LMLabel1Text.c_str(), [this, modsDirPath]() { //"There is no \"mods\" folder in " + RLCookedPCConsole_Path.string() + "\nDo you want to create it ?";
				fs::create_directory(modsDirPath);
				}, [this]() {ImGui::CloseCurrentPopup(); });
		}
		else if (!FileIsInDirectoryRecursive(RLCookedPCConsole_Path.string(), curMap.UpkFile.filename().string()))
		{
			renderYesNoPopup("JoinServer", std::string(curMap.UpkFile.filename().string() + LMLabel2Text).c_str(), [this, modsDirPath, curMap]() { //" isn't in CookedPCConsole. Paste the map to CookedPCConsole/mods ?";
				fs::copy(curMap.UpkFile, modsDirPath);
				}, [this]() {ImGui::CloseCurrentPopup(); });
		}
		else if (!MapWasAlreadyInCPCC(curMap))
		{
			renderInfoPopup("JoinServer", LMLabel3Text.c_str());//"You need to restart Rocket Legaue first to be able to join a server on this map!";
		}
		else
		{
			renderJoinServerPopup();
		}


		ImGui::SameLine();

		if (ImGui::Button(CancelText.c_str(), ImVec2(200.f, 50.f)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}


void Pluginx64::renderHostGamePopup(Map curMap)
{
	if (ImGui::BeginPopupModal("HostGame", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::BeginChild("##gameSettings", ImVec2(500.f, heightHostGamePopup), true);
		{
			CenterNexIMGUItItem(ImGui::CalcTextSize(GameSettingsText.c_str()).x); //Game Settings :
			ImGui::Text(GameSettingsText.c_str()); //Game Settings :
			ImGui::NewLine();
			ImGui::Text(GameModeText.c_str()); //Game Mode :

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

			ImGui::Text(NBOfPlayersText.c_str());
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
					heightHostGamePopup = 201.f + heightMutators;

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
				heightHostGamePopup = 197.f;
			}

			ImGui::Separator();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.f);

			if (ImGui::Button(HostGameText.c_str(), ImVec2(100.f, 30.f))) //Host Game
			{
				gameWrapper->Execute([&, curMap](GameWrapper* gw)
					{
						gameWrapper->ExecuteUnrealCommand("start " + curMap.Folder.string() + "/" + curMap.UpkFile.filename().string() + "?game=" + gameModes->GetSelectedValue() + "?GameTags=BotsNone," + getMutatorsCommandString() + "?NumPublicConnections=" + std::to_string(nbPlayers) + "?NumOpenPublicConnections=" + std::to_string(nbPlayers) + "?Lan?Listen");
					});

				cvarManager->log("Command : start " + curMap.Folder.string() + "/" + curMap.UpkFile.filename().string() + "?game=" + gameModes->GetSelectedValue() + "?GameTags=BotsNone," + getMutatorsCommandString() + "?NumPublicConnections=" + std::to_string(nbPlayers) + "?NumOpenPublicConnections=" + std::to_string(nbPlayers) + "?Lan?Listen");

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			AlignRightNexIMGUItItem(100.f, 8.f);
			if (ImGui::Button(CancelText.c_str(), ImVec2(100.f, 30.f)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndChild();
		}
		ImGui::EndPopup();
	}
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

void Pluginx64::renderJoinServerPopup()
{
	if (ImGui::BeginPopupModal("JoinServer", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.f);
		ImGui::Text("IP :");
		ImGui::SameLine();
		static char IP[200] = "";
		ImGui::InputText("##inputIP", IP, IM_ARRAYSIZE(IP));

		ImGui::Text("PORT :");
		ImGui::SameLine();
		static char PORT[200] = "7777";
		ImGui::InputText("##inputPORT", PORT, IM_ARRAYSIZE(PORT));

		std::string str_IP = std::string(IP);
		std::string str_PORT = std::string(PORT);


		CenterNexIMGUItItem(208.f);
		ImGui::BeginGroup();
		{
			if (ImGui::Button(JoinServerText.c_str(), ImVec2(100.f, 25.f)))//Join Server
			{
				JoinServerBool = false;

				gameWrapper->Execute([&, str_IP, str_PORT](GameWrapper* gw)
					{
						cvarManager->log("IP : " + str_IP);
						cvarManager->log("PORT : " + str_PORT);
						gameWrapper->ExecuteUnrealCommand("start " + str_IP + ":" + str_PORT + "/?Lan?Password=password");
					});

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button(CancelText.c_str(), ImVec2(100.f, 25.f)))//Cancel
			{
				JoinServerBool = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndGroup();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderNewUpdatePopup()
{
	static bool french = false;
	if (ImGui::BeginPopupModal("New Update", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string AddedText;
		std::string CtrlFText;
		std::string DonateText;
		std::string SupportMeText2;
		std::string SupportMeText3;
		std::string CWGDiscordText;
		if (!french)
		{
			AddedText = "Added :";
			CtrlFText = "-Shortcut Ctrl + F : Search maps by name in \"Map Loader\" tab";
			DonateText = "Donate :";
			SupportMeText2 = "Support Me :";
			SupportMeText3 = "You can donate or subscribe to my patreon if you want to support me and if you think my work deserves. Thank you.";
			CWGDiscordText = "Join Community Workshop Games discord server to play multiplayer workshop maps with the community, and participate in events.";
		}
		else
		{
			AddedText = "Ajouts :";
			CtrlFText = "-Raccourcis Ctrl + F : Recherche de maps par nom dans l'onglet \"Charger Map\"";
			DonateText = "Faire un don :";
			SupportMeText2 = "Me Soutenir :";
			SupportMeText3 = "Vous pouvez me faire un don ou vous abonner a mon patreon si vous voulez me soutenir et si vous pensez que mon travail merite. Merci.";
			CWGDiscordText = "Rejoignez le serveur discord Community Workshop Games pour jouer aux maps workshop en multijoueur avec la communaute, et participez aux evenements.";
		}

		ImGui::NewLine();
		CenterNexIMGUItItem(ImGui::CalcTextSize("Changelog v1.15.1").x);
		ImGui::Text("Changelog v1.15.1");
		ImGui::NewLine();
		ImGui::Checkbox("French", &french);
		ImGui::NewLine();
		ImGui::Text(AddedText.c_str());
		renderUnderLine(ImColor(255, 255, 255, 150));
		ImGui::NewLine();
		ImGui::Text(CtrlFText.c_str());

		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::Text(SupportMeText2.c_str());
		renderUnderLine(ImColor(255, 255, 255, 150));
		ImGui::Text(SupportMeText3.c_str());

		ImGui::Text("Patreon :");
		ImGui::SameLine();
		renderLink("https://www.patreon.com/WorkshopMapLoader");

		ImGui::Text(DonateText.c_str()); //"Donate :"
		ImGui::SameLine();
		renderLink("https://www.paypal.com/donate/?hosted_button_id=35N9JXTJ9PL6Q");

		ImGui::NewLine();
		ImGui::NewLine();

		ImGui::Text("Community Workshop Games :");
		renderUnderLine(ImColor(255, 255, 255, 150));
		ImGui::Text(CWGDiscordText.c_str());
		renderLink("https://discord.com/invite/KVgmf9JFpZ");

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

void Pluginx64::renderMapUnavaiablePopup()
{
	if (ImGui::BeginPopupModal("Map Unavaiable", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(MapsUnavailableText.c_str()); //"This map is unvailable to download for few seconds, wait"

		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button("OK", ImVec2(100.f, 25.f)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Pluginx64::renderSupportMePopup()
{
	//ImGui::SetNextWindowSizeConstraints(ImVec2{ 10.f, 10.f }, ImVec2{ 800.f, 500.f });
	if (ImGui::BeginPopupModal("Support Me & Patrons", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::NewLine();

		CenterNexIMGUItItem(ImGui::CalcTextSize(SupportMePopupText.c_str()).x);
		ImGui::Text(SupportMePopupText.c_str());

		ImGui::NewLine();

		CenterNexIMGUItItem(ImGui::CalcTextSize("Patreon :").x + ImGui::CalcTextSize(" https://www.patreon.com/WorkshopMapLoader").x);
		ImGui::Text("Patreon :");
		ImGui::SameLine();
		renderLink("https://www.patreon.com/WorkshopMapLoader");

		CenterNexIMGUItItem(ImGui::CalcTextSize(DonateText.c_str()).x + ImGui::CalcTextSize(" https://www.paypal.com/donate/?hosted_button_id=35N9JXTJ9PL6Q").x);
		ImGui::Text(DonateText.c_str()); //"Donate"
		ImGui::SameLine();
		renderLink("https://www.paypal.com/donate/?hosted_button_id=35N9JXTJ9PL6Q");

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		CenterNexIMGUItItem(ImGui::CalcTextSize("Thanks very much to all the patrons.").x);
		ImGui::Text("Thanks very much to all the patrons.");
		renderUnderLine(ImColor(255, 255, 255, 150));

		/*need to remove
		ImGui::SameLine();
		static int patronInt = 0;
		if (ImGui::Button("add"))
		{
			PatronsList.push_back("patron " + std::to_string(patronInt));
			patronInt++;
		}*/

		ImGui::NewLine();

		float LinesCount = float(PatronsList.size()) / 3.f;
		if (LinesCount - int(LinesCount) != 0.f)
		{
			LinesCount++;
		}
		if (LinesCount >= 30)
		{
			LinesCount = 30;
		}
		float height = int(LinesCount) * 17 + 12;

		//cvarManager->log("LinesCount : " + std::to_string(int(LinesCount)));

		ImGui::BeginChild("##PatronsListChild", ImVec2{800.f, height}, true);

		ImGui::Columns(3, 0, true);

		if (PatronsList.size() > 0)
		{
			for (int i = 0; i < PatronsList.size(); i++)
			{
				std::string patron = PatronsList[i];

				//center patron in column
				auto windowWidth = ImGui::GetContentRegionAvailWidth();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (windowWidth - ImGui::CalcTextSize(patron.c_str()).x) * 0.5f);

				ImGui::Text(patron.c_str());

				ImGui::NextColumn();
			}
		}

		ImGui::EndChild();

		AlignRightNexIMGUItItem(100.f, 8.f);
		if (ImGui::Button(CloseText.c_str(), ImVec2(100.f, 25.f)))
		{
			OpenSupportMePopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
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

}

// Called when window is closed
void Pluginx64::OnClose()
{
	isWindowOpen_ = false;


	MapsFolderPath = MapsFolderPathBuf;
}
