#include "pch.h"
#include "WorkshopMapLoader.h"


BAKKESMOD_PLUGIN(Pluginx64, "Workshop Map Loader & Downloader", "1.14", 0)


namespace
{
	char dummyChar;
}



std::string GameSetting::GetSelectedValue()
{
	return Values[selectedValue];
}



void Pluginx64::onLoad()
{
	gameWrapper->RegisterDrawable(std::bind(&Pluginx64::checkOpenMenuWithController, this, std::placeholders::_1));

	BakkesmodPath = gameWrapper->GetBakkesModPath().string() + "\\";
	IfNoPreviewImagePath = BakkesmodPath + "data\\WorkshopMapLoader\\Search\\NoPreview.jpg";

	std::string RLWin64_Path = std::filesystem::current_path().string();
	RLCookedPCConsole_Path = RLWin64_Path.substr(0, RLWin64_Path.length() - 14) + "TAGame\\CookedPCConsole";

	std::string Data_WorkshopMapLoader_Path = BakkesmodPath + "data\\WorkshopMapLoader\\";

	//Load logos
	SteamLogoImage = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "steamlogo.png", false, true);
	RLMAPSLogoImage = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "rlmapslogo.png", false, true);

	//Load display mode images
	MapsDisplayMode_Logo1_Image = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logo1.png", false, true);
	MapsDisplayMode_Logo2_Image = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logo2.png", false, true);
	MapsDisplayMode_Logo1_SelectedImage = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logo1_white.png", false, true);
	MapsDisplayMode_Logo2_SelectedImage = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logo2_white.png", false, true);

	STEAM_browsing = false;
	RLMAPS_browsing = false;
	RLMAPS_Searching = false;
	CurrentPage = 0; //starts at 0
	NBOfMapsOnSite = 0;

	RLMAPS_PageSelected = 0;


	JoinServerBool = false;
	DownloadTexturesBool = false;


	std::thread t1(&Pluginx64::CheckIssuesEncountered, this);
	t1.detach();


	try
	{
		for (const auto& dir : fs::directory_iterator(RLCookedPCConsole_Path.string()))
		{
			if (dir.is_directory())
			{
				for (const auto& file : fs::recursive_directory_iterator(dir.path()))
					if (!file.is_directory())
					{
						MapsAlreadyInCPCC.push_back(file.path());
						cvarManager->log("Map already in cpcc : " + file.path().filename().string());
					}
			}
		}
	}
	catch (const std::exception& ex)
	{
		cvarManager->log("error : verifying MapsAlreadyInCPCC : " + std::string(ex.what()));
	}
	

	if (Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg"))
	{
		std::vector<std::string> CFGVariablesList = GetMapsFolderPathInCfg(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg");
		cvarManager->log("Workshop Maps Folder : " + CFGVariablesList.at(0));
		MapsFolderPath = CFGVariablesList.at(0);
		unzipMethod = CFGVariablesList.at(2);
		dontAsk = std::stoi(CFGVariablesList.at(4));
		MapsDisplayMode = std::stoi(CFGVariablesList.at(5));
		nbTilesPerLine = std::stoi(CFGVariablesList.at(6));
		ControllerSensitivity = std::stoi(CFGVariablesList.at(7));
		ControllerScrollSensitivity = std::stoi(CFGVariablesList.at(8));




		if (CFGVariablesList.at(1) == "0")
		{
			FR = false;
		}
		else
		{
			FR = true;
		}

		if (CFGVariablesList.at(3) == "0")
		{
			HasSeeNewUpdateAlert = false;
		}
		else
		{
			HasSeeNewUpdateAlert = true;
		}
	}
	else
	{
		cvarManager->log(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg : doesn't exist");
		MapsFolderPath = "The path of the maps folder";
		FR = false;
		unzipMethod = "Powershell";
		HasSeeNewUpdateAlert = false;
	}

	strncpy(MapsFolderPathBuf, MapsFolderPath.c_str(), IM_ARRAYSIZE(MapsFolderPathBuf)); //Make  MapsFolderPathBuf = MapsFolderPath

	cvarManager->log("The bakkesmod path : " + BakkesmodPath);
}



void Pluginx64::checkOpenMenuWithController(CanvasWrapper canvas)
{
	Gamepad ds4 = Gamepad(1);

	ds4.Update();

	if (gameWrapper->IsInOnlineGame())
		return;

	if (ds4.Connected())
	{
		static bool ButtonsWasPressed = false;

		if (ds4.checkButtonPress(XINPUT_GAMEPAD_LEFT_THUMB) && ds4.checkButtonPress(XINPUT_GAMEPAD_RIGHT_THUMB) && !ButtonsWasPressed)
		{
			ButtonsWasPressed = true;
		}
		else if (!ds4.checkButtonPress(XINPUT_GAMEPAD_LEFT_THUMB) && !ds4.checkButtonPress(XINPUT_GAMEPAD_RIGHT_THUMB) && ButtonsWasPressed)
		{
			gameWrapper->Execute([&](GameWrapper* gw)
				{
					cvarManager->executeCommand("togglemenu " + GetMenuName());
				});
			ButtonsWasPressed = false;
		}
	}
	
}




//Host multiplayer game

std::string Pluginx64::getMutatorsCommandString()
{
	std::string mutatorsCommandString = "";

	for (auto mutator : mutators)
	{
		if (mutator->GetSelectedValue() != "")
		{
			mutatorsCommandString += mutator->GetSelectedValue() + ",";
		}
	}

	if (mutatorsCommandString.back() == ',')
	{
		mutatorsCommandString = mutatorsCommandString.substr(0, mutatorsCommandString.size() - 1);
	}

	return mutatorsCommandString;
}







//Local Maps

std::vector<std::string> Pluginx64::GetJSONLocalMapInfos(std::string jsonFilePath)
{
	std::vector<std::string> Infos;
	std::string Text;
	std::string line;
	std::ifstream myfile(jsonFilePath);

	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{
			Text += line;
		}
		myfile.close();
	}

	//Parse response json
	Json::Value actualJson;
	Json::Reader reader;

	reader.parse(Text, actualJson);

	std::string MapTitle = actualJson["Title"].asString();
	std::string MapDescription = actualJson["Description"].asString();
	std::string MapAuthor = actualJson["Author"].asString();

	MapTitle.erase(std::remove(MapTitle.begin(), MapTitle.end(), '\n'), MapTitle.end()); //remove newlines
	MapDescription.erase(std::remove(MapDescription.begin(), MapDescription.end(), '\n'), MapDescription.end()); //remove newlines
	MapAuthor.erase(std::remove(MapAuthor.begin(), MapAuthor.end(), '\n'), MapAuthor.end()); //remove newlines

	Infos.push_back(MapTitle);
	Infos.push_back(MapDescription);
	Infos.push_back(MapAuthor);

	//{\"Title\":\"" + Title + "\",\"Author\":\"" + Author + "\",\"Description\":\"" + Description + "\",\"PreviewUrl\":\"" + PreviewUrl + "\"}

	return Infos;
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






//Download Utils

void Pluginx64::CreateJSONSearchWorkshopInfos(std::string jsonFileName, std::string filePath, std::string mapSize, std::string mapDescription)
{
	std::string filename = filePath + jsonFileName + ".json";
	//cvarManager->log(filename);
	std::ofstream JSONFile(filename);

	std::string Size = mapSize;
	std::string Description = mapDescription;

	JSONFile << "{\"Size\":\"" + Size + "\",\"Description\":\"" + Description + "\"}";
	JSONFile.close();
}

void Pluginx64::CreateJSONLocalWorkshopInfos(std::string jsonFileName, std::string workshopMapPath, std::string mapTitle, std::string mapAuthor, std::string mapDescription,
	std::string mapPreviewUrl)
{
	std::string filename = workshopMapPath + jsonFileName + ".json";
	//cvarManager->log(filename);
	std::ofstream JSONFile(filename);

	std::string Title = mapTitle;
	std::string Author = mapAuthor;
	std::string Description = mapDescription;
	std::string PreviewUrl = mapPreviewUrl;

	JSONFile << "{\"Title\":\"" + Title + "\",\"Author\":\"" + Author + "\",\"Description\":\"" + Description + "\",\"PreviewUrl\":\"" + PreviewUrl + "\"}";
	JSONFile.close();
}

void Pluginx64::CreateUnzipBatchFile(std::string destinationPath, std::string zipFilePath)
{
	std::string filename = BakkesmodPath + "data\\WorkshopMapLoader\\temp\\unzip.bat";
	std::ofstream BatFile(filename);

	std::string Destination_Path = destinationPath; //where it unzip (just the folder)
	std::string ZIPFile_Path = zipFilePath; //where the .zip is (complete path + .zip)

	//just replace the slashes by backslashes for both pathes
	std::string Correct_File_Path = replace(Destination_Path, *"/", *"\\");
	std::string Correct_ZIPFile_Path = replace(ZIPFile_Path, *"/", *"\\");


	// Comment added by Martinn
	// https://stackoverflow.com/questions/21704041/creating-batch-script-to-unzip-a-file-without-additional-zip-tools
	BatFile << "@echo off\n";
	BatFile << "setlocal\n";
	BatFile << "cd /d %~dp0\n";
	BatFile << "Call :UnZipFile \"" + Correct_File_Path + "\" \"" + Correct_ZIPFile_Path + "\"\n";
	BatFile << "exit /b\n";
	BatFile << "\n";
	BatFile << ":UnZipFile <ExtractTo> <newzipfile>\n";
	BatFile << "set vbs=\" % temp % \_.vbs\"\n";
	BatFile << "if exist %vbs% del /f /q %vbs%\n";
	BatFile << ">%vbs%  echo Set fso = CreateObject(\"Scripting.FileSystemObject\")\n";
	BatFile << ">>%vbs% echo If NOT fso.FolderExists(%1) Then\n";
	BatFile << ">>%vbs% echo fso.CreateFolder(%1)\n";
	BatFile << ">>%vbs% echo End If\n";
	BatFile << ">>%vbs% echo set objShell = CreateObject(\"Shell.Application\")\n";
	BatFile << ">>%vbs% echo set FilesInZip=objShell.NameSpace(%2).items\n";
	BatFile << ">>%vbs% echo objShell.NameSpace(%1).CopyHere(FilesInZip)\n";
	BatFile << ">>%vbs% echo Set fso = Nothing\n";
	BatFile << ">>%vbs% echo Set objShell = Nothing\n";
	BatFile << "cscript //nologo %vbs%\n";
	BatFile << "if exist %vbs% del /f /q %vbs%\n";

	BatFile.close();

	system(filename.c_str());
}

std::string Pluginx64::GetWorkshopIDByUrl(std::string workshopurl)
{
	std::string Workshop_url = workshopurl;

	//get the workshop ID
	std::string Workshop_id = Workshop_url.substr(Workshop_url.find("id=") + 3, 10);

	return Workshop_id;
}



//Steam Download & Search

void Pluginx64::STEAM_DownloadWorkshop(std::string workshopURL, std::string Dfolderpath, Steam_MapResult mapResult, bool createJsonFile)
{
	std::string Workshop_url = workshopURL;
	std::string Workshop_id;

	if (workshopURL != "")
	{
		//get the workshop ID
		Workshop_id = GetWorkshopIDByUrl(Workshop_url);
	}
	else
	{
		Workshop_id = mapResult.ID;
	}
	cvarManager->log("Workshop id : " + Workshop_id);


	UserIsChoosingYESorNO = true;

	while (UserIsChoosingYESorNO)
	{
		//while he is choosing
		Sleep(100);
	}

	if (!AcceptTheDownload) //if user canceled the download
	{
		return;
	}




	std::string Workshop_Dl_Path = "";
	std::string workshopSafeMapName = replace(mapResult.Name, *" ", *"_");
	std::string specials[] = { "/", "\\", "?", ":", "*", "\"", "<", ">", "|", "-"};
	for (auto special : specials)
	{
		eraseAll(workshopSafeMapName, special);
	}

	if (Dfolderpath.back() == '/' || Dfolderpath.back() == '\\') //if last character != /
	{
		Workshop_Dl_Path = Dfolderpath + workshopSafeMapName;
	}
	else
	{
		Workshop_Dl_Path = Dfolderpath + "/" + workshopSafeMapName;
		cvarManager->log("A slash has been added a the end of the path");
	}


	try
	{
		fs::create_directory(Workshop_Dl_Path); //create directory for the map downloaded
		cvarManager->log("Directory Created : " + Workshop_Dl_Path);

	}
	catch (const std::exception& ex) //manage errors when trying to create a folder in an administrator folder
	{
		cvarManager->log(ex.what());
		FolderErrorText = ex.what();
		FolderErrorBool = true;
		IsRetrievingWorkshopFiles = false;
		return;
	}


	if (createJsonFile)
	{
		CreateJSONLocalWorkshopInfos(workshopSafeMapName, Workshop_Dl_Path + "/", mapResult.Name, mapResult.Author, mapResult.Description, mapResult.PreviewUrl);
		cvarManager->log("JSON Created : " + Workshop_Dl_Path + "/" + workshopSafeMapName + ".json");
	}


	std::string download_url = "https://steamworkshop.jetfox.ovh/query.php?workshopid=" + Workshop_id;
	cvarManager->log("Download URL : " + download_url);
	std::string Folder_Path = Workshop_Dl_Path + "/" + workshopSafeMapName + ".zip";


	IsRetrievingWorkshopFiles = false;
	STEAM_IsDownloadingWorkshop = true;

	cvarManager->log("Download Starting...");

	//download test
	CurlRequest req;
	req.url = download_url;
	req.progress_function = [this](double file_size, double downloaded, ...)
	{
		//cvarManager->log("Download progress : " + std::to_string(downloaded));
		STEAM_Download_Progress = downloaded;
	};

	HttpWrapper::SendCurlRequest(req, [this, Folder_Path, Workshop_Dl_Path](int code, char* data, size_t size)
		{
			std::ofstream out_file{ Folder_Path, std::ios_base::binary };
			if (out_file)
			{
				out_file.write(data, size);

				cvarManager->log("Workshop Downloaded in : " + Workshop_Dl_Path);
				STEAM_IsDownloadingWorkshop = false;
			}
		});


	while (STEAM_IsDownloadingWorkshop == true)
	{
		cvarManager->log("downloading...............");

		STEAM_WorkshopDownload_Progress = STEAM_Download_Progress;
		STEAM_WorkshopDownload_FileSize = std::stoi(mapResult.Size);
		Sleep(500);
	}

	if (unzipMethod == "Bat")
	{
		CreateUnzipBatchFile(Workshop_Dl_Path, Folder_Path);
	}
	else
	{
		std::string extractCommand = "powershell.exe Expand-Archive -LiteralPath '" + Folder_Path + "' -DestinationPath '" + Workshop_Dl_Path + "'";
		system(extractCommand.c_str());
	}


	int checkTime = 0; //this isn't good but I don't care
	while (UdkInDirectory(Workshop_Dl_Path) == "Null")
	{
		cvarManager->log("Extracting zip file");
		if (checkTime > 10)
		{
			cvarManager->log("Failed extracting the map zip file");
			return;
		}
		Sleep(1000);
		checkTime++;
	}

	cvarManager->log("File Extracted");

	renameFileToUPK(Workshop_Dl_Path);

}

/*
void Pluginx64::STEAM_DownloadWorkshop(std::string workshopURL, std::string Dfolderpath, std::string workshopid, bool WorkshopIDByUrl, int index, bool createJsonFile)
{
	std::string Workshop_url = workshopURL;
	std::string Workshop_id;

	if (WorkshopIDByUrl == true)
	{
		//get the workshop ID
		Workshop_id = GetWorkshopIDByUrl(Workshop_url);
	}
	else
	{
		Workshop_id = workshopid;
	}
	cvarManager->log("Workshop id : " + Workshop_id);

	IsRetrievingWorkshopFiles = true;

	std::string request_url = "https://node04.steamworkshopdownloader.io/prod/api/download/request";
	cpr::Response request_response = cpr::Post(cpr::Url{request_url}, cpr::Body{"{\"publishedFileId\":" + Workshop_id + ",\"downloadFormat\":\"raw\"}"});

	if (request_response.status_code != 200)
	{
		cvarManager->log("request_response.status_code : " + std::to_string(request_response.status_code));
		DownloadFailedErrorText = " error " + std::to_string(request_response.status_code);
		DownloadFailed = true;
		IsRetrievingWorkshopFiles = false;
		return;
	}

	cvarManager->log("Response : " + request_response.text);


	//get the Workshop uuid
	std::string Workshop_uuid = request_response.text.substr(9, request_response.text.length() - 11);

	//get the state (if prepared)
	std::string status_url = "https://node04.steamworkshopdownloader.io/prod/api/download/status";
	cpr::Response state = cpr::Post(cpr::Url{status_url}, cpr::Body{"{\"uuids\":[\"" + Workshop_uuid + "\"]}:"});

	if (state.status_code != 200)
	{
		cvarManager->log("state.status_code : " + std::to_string(state.status_code));
		DownloadFailedErrorText = "state : error " + std::to_string(state.status_code);
		DownloadFailed = true;
		IsRetrievingWorkshopFiles = false;
		return;
	}

	cvarManager->log("State : " + state.text);


	//Parse state response json
	Json::Value stateJson;
	Json::Reader stateReader;

	stateReader.parse(state.text, stateJson);


	int NbMaxOfRequest = 0;

	std::string testtt = stateJson[Workshop_uuid]["status"].asString();
	cvarManager->log("testtt : " + testtt);

	//while prepared is not in the state response, we send the request
	while (stateJson[Workshop_uuid]["status"].asString() != "prepared" && NbMaxOfRequest < 8)
	{
		cvarManager->log("Resending request...");

		state = cpr::Post(cpr::Url{status_url}, cpr::Body{"{\"uuids\":[\"" + Workshop_uuid + "\"]}:"});
		cvarManager->log("State : " + state.text);

		stateReader.parse(state.text, stateJson);

		NbMaxOfRequest++;
		if (NbMaxOfRequest >= 8) //stopping the requests 
		{
			IsRetrievingWorkshopFiles = false;

			if (FindAllSubstringInAString(state.text, "<head><title>", "</title></head>").size() > 0)
			{
				cvarManager->log("Download Failed : 404 Not Found !");
				DownloadFailedErrorText = " : 404 Not Found !";
			}
			else
			{
				cvarManager->log("Download Failed : too many tries !");
				DownloadFailedErrorText = "";
			}

			DownloadFailed = true;
			IsRetrievingWorkshopFiles = false;
			return;
		}
		Sleep(2000);
	}

	cvarManager->log("Found !");


	std::string storageNode = stateJson[Workshop_uuid]["storageNode"].asString();
	std::string storagePath = stateJson[Workshop_uuid]["storagePath"].asString();
	cvarManager->log("storageNode : " + storageNode);
	cvarManager->log("storagePath : " + storagePath);

	UserIsChoosingYESorNO = true;

	while (UserIsChoosingYESorNO)
	{
		//while he is choosing
		Sleep(100);
	}

	if (!AcceptTheDownload) //if user canceled the download
	{
		IsRetrievingWorkshopFiles = false;
		return;
	}



	//Get the workshop map infos, name and size
	std::string Workshop_request_url = "https://node04.steamworkshopdownloader.io/prod/api/details/file";
	cpr::Response Workshop_request_response = cpr::Post(cpr::Url{Workshop_request_url}, cpr::Body{"[" + Workshop_id + "]:"});

	if (Workshop_request_response.status_code != 200)
	{
		cvarManager->log("Workshop_request_response.status_code : " + std::to_string(Workshop_request_response.status_code));
		DownloadFailedErrorText = "Workshop_request_response : error " + std::to_string(Workshop_request_response.status_code);
		DownloadFailed = true;
		IsRetrievingWorkshopFiles = false;
		return;
	}

	//Parse response json
	Json::Value fileJson;
	Json::Reader fileReader;

	fileReader.parse(Workshop_request_response.text, fileJson);

	std::string Workshop_filename = fileJson[0]["title_disk_safe"].asString();
	std::string Workshop_filesize = fileJson[0]["file_size"].asString();
	cvarManager->log("Map name  : " + Workshop_filename);
	cvarManager->log("File size : " + Workshop_filesize + "Bytes");


	std::string Workshop_Dl_Path = "";

	if (Dfolderpath.back() == '/' || Dfolderpath.back() == '\\') //if last character != /
	{
		Workshop_Dl_Path = Dfolderpath + Workshop_filename;
	}
	else
	{
		Workshop_Dl_Path = Dfolderpath + "/" + Workshop_filename;
		cvarManager->log("A slash has been added a the end of the path");
	}


	try 
	{
		fs::create_directory(Workshop_Dl_Path); //create directory for the map downloaded
		cvarManager->log("Directory Created : " + Workshop_Dl_Path);
			
	}
	catch (const std::exception& ex) //manage errors when trying to create a folder in an administrator folder
	{
		cvarManager->log(ex.what());
		FolderErrorText = ex.what();
		FolderErrorBool = true;
		IsRetrievingWorkshopFiles = false;
		return;
	}
		

	if (createJsonFile)
	{
		CreateJSONLocalWorkshopInfos(Workshop_filename, Workshop_Dl_Path + "/", Steam_MapResultList.at(index).Name, Steam_MapResultList.at(index).Author, Steam_MapResultList.at(index).Description, Steam_MapResultList.at(index).PreviewUrl);
		cvarManager->log("JSON Created : " + Workshop_Dl_Path + "/" + Workshop_filename + ".json");
	}


	std::string download_url = "https://" + storageNode + "/prod//storage/" + storagePath + "?uuid=" + Workshop_uuid;
	cvarManager->log("Download URL : " + download_url);
	std::string Folder_Path = Workshop_Dl_Path + "/" + Workshop_filename + ".zip";


	IsRetrievingWorkshopFiles = false;
	STEAM_IsDownloadingWorkshop = true;

	cvarManager->log("Download Starting...");
		
	//download test
	CurlRequest req;
	req.url = download_url;
	req.progress_function = [this](double file_size, double downloaded, ...)
	{
		//cvarManager->log("Download progress : " + std::to_string(downloaded));
		STEAM_Download_Progress = downloaded;
	};

	HttpWrapper::SendCurlRequest(req, [this, Folder_Path, Workshop_Dl_Path](int code, char* data, size_t size)
		{
			std::ofstream out_file{ Folder_Path, std::ios_base::binary };
			if (out_file)
			{
				out_file.write(data, size);

				cvarManager->log("Workshop Downloaded in : " + Workshop_Dl_Path);
				STEAM_IsDownloadingWorkshop = false;
			}
		});


	while (STEAM_IsDownloadingWorkshop == true)
	{
		cvarManager->log("downloading...............");

		STEAM_WorkshopDownload_Progress = STEAM_Download_Progress;
		STEAM_WorkshopDownload_FileSize = std::stoi(Workshop_filesize);
		Sleep(500);
	}
		
	if (unzipMethod == "Bat")
	{
		CreateUnzipBatchFile(Workshop_Dl_Path, Folder_Path);
	}
	else
	{
		std::string extractCommand = "powershell.exe Expand-Archive -LiteralPath '" + Folder_Path + "' -DestinationPath '" + Workshop_Dl_Path + "'";
		system(extractCommand.c_str());
	}


	int checkTime = 0; //this isn't good but I don't care
	while (UdkInDirectory(Workshop_Dl_Path) == "Null")
	{
		cvarManager->log("Extracting zip file");
		if (checkTime > 10)
		{
			cvarManager->log("Failed extracting the map zip file");
			return;
		}
		Sleep(1000);
		checkTime++;
	}

	cvarManager->log("File Extracted");

	renameFileToUPK(Workshop_Dl_Path);
	
}*/

void Pluginx64::StartSearchRequest(std::string fullurl)
{
	STEAM_Searching = true;
	Steam_MapResultList.clear();

	cpr::Response request_response = cpr::Get(cpr::Url{fullurl});
	
	STEAM_NumberOfMapsFound = FindAllSubstringInAString(request_response.text, "workshopItemTitle ellipsis\">", "</div>").size(); //get number of maps there is on the page

	for (int i = 0; i < FindAllSubstringInAString(request_response.text, "workshopItemTitle ellipsis\">", "</div>").size(); i++)
	{
		std::string resultMapName = FindAllSubstringInAString(request_response.text, "workshopItemTitle ellipsis\">", "</div>").at(i);
		std::string resultMapID = FindAllSubstringInAString(request_response.text, "<div id=\"sharedfile_", "\" class=\"workshopItemPreviewHolder \"").at(i);
		std::string resultMapPreviewUrl = FindAllSubstringInAString(request_response.text, "class=\"workshopItemPreviewImage \" src=\"", "\">").at(i);
		std::string resultMapAuthor = FindAllSubstringInAString(request_response.text, "myworkshopfiles/?appid=252950\">", "</a>").at(i);
		std::string resultSize;
		std::string resultDescription;
		std::filesystem::path resultImagePath;
		std::shared_ptr<ImageWrapper> resultImage;
		bool resultisImageLoaded;


		std::string file_infos_request_url = "https://steamworkshop.jetfox.ovh/query.php?filesize=" + resultMapID;

		if (!Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\Search\\json\\" + resultMapID + ".json")) //if json map infos doesn't exist
		{
			cpr::Response file_size_request_response = cpr::Get(cpr::Url{ file_infos_request_url });

			//cvarManager->log("status code : " + std::to_string(file_infos_request_response.status_code));

			std::string Workshop_filesize;
			std::string WorkshopMapDescription;

			if (file_size_request_response.status_code == 200)
			{
				//Parse state response json
				Json::Value fileSizeJson;
				Json::Reader fileSizeReader;

				fileSizeReader.parse(file_size_request_response.text, fileSizeJson);

				Workshop_filesize = fileSizeJson["Filesize"].asString();


				cpr::Response workshop_steam_request_response = cpr::Get(cpr::Url{ "https://steamcommunity.com/sharedfiles/filedetails/?id=" + resultMapID });
				WorkshopMapDescription = FindAllSubstringInAString(workshop_steam_request_response.text, "<div class=\"workshopItemDescription\" id=\"highlightContent\">", "<script>").at(0);
				cvarManager->log("result : " + WorkshopMapDescription);
				CleanHTML(WorkshopMapDescription);
				cvarManager->log("parser : " + WorkshopMapDescription);
			}
			else
			{
				Workshop_filesize = "n/a";
				WorkshopMapDescription = "n/a";
			}


			resultSize = Workshop_filesize;
			resultDescription = WorkshopMapDescription;

			CreateJSONSearchWorkshopInfos(resultMapID, BakkesmodPath + "data\\WorkshopMapLoader\\Search\\json\\", Workshop_filesize, WorkshopMapDescription);
		}
		else
		{
			resultSize = GetJSONSearchMapInfos(BakkesmodPath + "data\\WorkshopMapLoader\\Search\\json\\" + resultMapID + ".json").at(0);
			resultDescription = GetJSONSearchMapInfos(BakkesmodPath + "data\\WorkshopMapLoader\\Search\\json\\" + resultMapID + ".json").at(1);
			cvarManager->log("Optimisation : JSON exists so no download");
		}

		if (!Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\Search\\img\\" + resultMapID + ".jfif")) //if preview image doesn't exist
		{
			IsDownloadingPreview = true;
			DownloadPreviewImage(resultMapPreviewUrl, BakkesmodPath + "data\\WorkshopMapLoader\\Search\\img\\" + resultMapID + ".jfif");
		}

		while (IsDownloadingPreview == true)
		{
			Sleep(10);
		}

		resultImagePath = BakkesmodPath + "data\\WorkshopMapLoader\\Search\\img\\" + resultMapID + ".jfif";
		resultImage = std::make_shared<ImageWrapper>(resultImagePath, false, true);
		resultisImageLoaded = true;


		Steam_MapResult result;
		result.Name = resultMapName;
		result.ID = resultMapID;
		result.PreviewUrl = resultMapPreviewUrl;
		result.Author = resultMapAuthor;
		result.Size = resultSize;
		result.Description = resultDescription;
		result.ImagePath = resultImagePath;
		result.Image = resultImage;
		result.isImageLoaded = resultisImageLoaded;

		Steam_MapResultList.push_back(result);
	}

	//get the href(url) of other pages if there are
	OtherPagesList = FindAllSubstringInAString(request_response.text, "pagelink\" href=\"", "\">");

	/*
	for (int i = 0; i < Steam_MapResultList.size(); i++)
	{
		cvarManager->log("Map Name : " + Steam_MapResultList.at(i).Name);
		cvarManager->log("Map ID : " + Steam_MapResultList.at(i).ID);
		cvarManager->log("Map Size : " + Steam_MapResultList.at(i).Size);
		cvarManager->log("Map Description : " + Steam_MapResultList.at(i).Description);
		cvarManager->log("Map Preview Url : " + Steam_MapResultList.at(i).PreviewUrl);
		cvarManager->log("Map Author : " + Steam_MapResultList.at(i).Author);
		cvarManager->log("");
	}
	*/
	STEAM_Searching = false;
}

std::vector<std::string> Pluginx64::GetJSONSearchMapInfos(std::string jsonFilePath)
{
	std::vector<std::string> Infos;
	std::string Text;
	std::string line;
	std::ifstream myfile(jsonFilePath);

	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{
			Text += line;
		}
		myfile.close();
	}

	//Parse response json
	Json::Value actualJson;
	Json::Reader reader;

	reader.parse(Text, actualJson);

	std::string MapSize = actualJson["Size"].asString();
	std::string MapDescription = actualJson["Description"].asString();

	MapSize.erase(std::remove(MapSize.begin(), MapSize.end(), '\n'), MapSize.end()); //remove newlines
	MapDescription.erase(std::remove(MapDescription.begin(), MapDescription.end(), '\n'), MapDescription.end()); //remove newlines

	Infos.push_back(MapSize);
	Infos.push_back(MapDescription);

	//"{\"Size\":\"" + Size + "\",\"Description\":\"" + Description + "\"}"

	return Infos;
}







//rocketleaguemaps.us

void Pluginx64::GetResults(std::string searchType, std::string keyWord)
{
	RLMAPS_Searching = true;
	RLMAPS_Pages.clear();
	RLMAPS_MapResultList.clear();
	RLMAPS_PageSelected = 0;

	std::string request_url = rlmaps_url;
	cpr::Response request_response = cpr::Get(cpr::Url{ request_url });


	//Parse response json
	Json::Value actualJson;
	Json::Reader reader;
	
	reader.parse(request_response.text, actualJson);

	const Json::Value maps = actualJson["body"];

	//get the number of maps found
	int nbMapsFound = 0;
	for (int index = 0; index < maps.size(); ++index)
	{
		std::string searchingTypeValueToLower;
		if (searchType == "Maps")
		{
			searchingTypeValueToLower = maps[index]["mapName"].asString();
		}
		else
		{
			searchingTypeValueToLower = maps[index]["creator"].asString();
		}
		std::transform(searchingTypeValueToLower.begin(), searchingTypeValueToLower.end(), searchingTypeValueToLower.begin(), ::tolower); //transform a string to lowercase
		std::transform(keyWord.begin(), keyWord.end(), keyWord.begin(), ::tolower); //transform a string to lowercase

		if (searchingTypeValueToLower.find(keyWord) != std::string::npos) //if keyWord is in the mapNameToLower
		{
			nbMapsFound++;
		}
	}
	RLMAPS_NumberOfMapsFound = nbMapsFound;

	int nbResults = 0;
	int pageIndex = 0;

	for (int index = 0; index < maps.size(); ++index)
	{
		std::string resultMapID = maps[index]["mapid"].asString();
		std::string resultMapName = maps[index]["mapName"].asString();
		std::string resultMapPreviewUrl = maps[index]["mapPicture"].asString();


		std::filesystem::path resultImagePath = BakkesmodPath + "data\\WorkshopMapLoader\\Search\\img\\RLMAPS\\" + resultMapID + ".jfif";
		std::shared_ptr<ImageWrapper> resultImage;
		bool resultisImageLoaded;



		std::string searchingTypeValueToLower;
		if (searchType == "Maps")
		{
			searchingTypeValueToLower = maps[index]["mapName"].asString();
		}
		else
		{
			searchingTypeValueToLower = maps[index]["creator"].asString();
		}


		std::transform(searchingTypeValueToLower.begin(), searchingTypeValueToLower.end(), searchingTypeValueToLower.begin(), ::tolower); //transform a string to lowercase
		std::transform(keyWord.begin(), keyWord.end(), keyWord.begin(), ::tolower); //transform a string to lowercase


		if (searchingTypeValueToLower.find(keyWord) != std::string::npos) //if keyWord is in the mapNameToLower
		{
			cvarManager->log("Search on RLMAPS : " + resultMapName);

			if (!Directory_Or_File_Exists(resultImagePath)) //if preview image doesn't exist
			{
				IsDownloadingPreview = true;
				DownloadPreviewImage(resultMapPreviewUrl, resultImagePath.string());
			}

			while (IsDownloadingPreview == true)
			{
				Sleep(10);
			}


			resultImage = std::make_shared<ImageWrapper>(resultImagePath, false, true);
			resultisImageLoaded = true;

			RLMAPS_MapResult result;
			result.ID = resultMapID;
			result.Name = resultMapName;
			result.ZipName = maps[index]["mapZipName"].asString();
			result.Size = maps[index]["filesize"].asString();
			result.Author = maps[index]["creator"].asString();
			result.ShortDescription = maps[index]["mapShortDescription"].asString();
			result.Description = maps[index]["mapDescription"].asString();
			result.PreviewUrl = resultMapPreviewUrl;
			result.ImagePath = resultImagePath;
			result.Image = resultImage;
			result.isImageLoaded = resultisImageLoaded;


			if (nbResults == 0)
			{
				RLMAPS_Pages.push_back({ result });
			}
			else
			{
				RLMAPS_Pages.at(pageIndex).push_back(result);
			}


			nbResults++;

			if (nbResults == 30)
			{
				nbResults = 0;
				pageIndex++;
			}
		}
	}

	RLMAPS_Searching = false;
}

void Pluginx64::GetResultsBrowseMaps(int offset)
{
	RLMAPS_Searching = true;
	RLMAPS_MapResultList.clear();
	RLMAPS_Pages.clear();
	RLMAPS_PageSelected = 0;

	NBOfMapsOnSite = GetNBOfMapsOnSite();

	std::string request_url = rlmaps_offset_url + std::to_string(offset);
	cpr::Response request_response = cpr::Get(cpr::Url{ request_url });


	//Parse response json
	Json::Value actualJson;
	Json::Reader reader;

	reader.parse(request_response.text, actualJson);

	const Json::Value maps = actualJson["body"];

	RLMAPS_NumberOfMapsFound = maps.size();



	int nbResults = 0;
	int pageIndex = 0;

	for (int index = 0; index < maps.size(); ++index)
	{
		std::string resultMapID = maps[index]["mapid"].asString();
		std::string resultMapName = maps[index]["mapName"].asString();
		std::string resultMapPreviewUrl = maps[index]["mapPicture"].asString();


		std::filesystem::path resultImagePath = BakkesmodPath + "data\\WorkshopMapLoader\\Search\\img\\RLMAPS\\" + resultMapID + ".jfif";
		std::shared_ptr<ImageWrapper> resultImage;
		bool resultisImageLoaded;



		cvarManager->log(resultMapName);

		if (!Directory_Or_File_Exists(resultImagePath)) //if preview image doesn't exist
		{
			IsDownloadingPreview = true;
			DownloadPreviewImage(resultMapPreviewUrl, resultImagePath.string());
		}

		while (IsDownloadingPreview == true)
		{
			Sleep(10);
		}


		resultImage = std::make_shared<ImageWrapper>(resultImagePath, false, true);
		resultisImageLoaded = true;

		RLMAPS_MapResult result;
		result.ID = resultMapID;
		result.Name = resultMapName;
		result.ZipName = maps[index]["mapZipName"].asString();
		result.Size = maps[index]["filesize"].asString();
		result.Author = maps[index]["creator"].asString();
		result.ShortDescription = maps[index]["mapShortDescription"].asString();
		result.Description = maps[index]["mapDescription"].asString();
		result.PreviewUrl = resultMapPreviewUrl;
		result.ImagePath = resultImagePath;
		result.Image = resultImage;
		result.isImageLoaded = resultisImageLoaded;


		if (nbResults == 0)
		{
			RLMAPS_Pages.push_back({ result });
		}
		else
		{
			RLMAPS_Pages.at(pageIndex).push_back(result);
		}
		

		nbResults++;

		if (nbResults == 30)
		{
			nbResults = 0;
			pageIndex++;
		}
	}

	RLMAPS_Searching = false;
}

int Pluginx64::GetNBOfMapsOnSite()
{
	std::string request_url = rlmaps_url;
	cpr::Response request_response = cpr::Get(cpr::Url{ request_url });

	//Parse response json
	Json::Value actualJson;
	Json::Reader reader;

	reader.parse(request_response.text, actualJson);
	const Json::Value maps = actualJson["body"];

	cvarManager->log("total maps on site : " + std::to_string(maps.size()));
	return maps.size();
}

std::vector<int> Pluginx64::listBrowsePages()
{
	std::vector<int> listPages;
	int nbPages = (NBOfMapsOnSite / 30);

	for (int i = 0; i <= nbPages; i++)
	{
		if (i >= (CurrentPage - 2) && i <= (CurrentPage + 2))
		{
			listPages.push_back(i);
		}
	}
	return listPages;
}

void Pluginx64::RLMAPS_DownloadWorkshop(std::string folderpath, RLMAPS_MapResult mapResult)
{

	UserIsChoosingYESorNO = true;

	while (UserIsChoosingYESorNO)
	{
		//while he is choosing
		Sleep(100);
	}

	if (!AcceptTheDownload) //if user canceled the download
	{
		return;
	}

	std::string Workshop_Dl_Path = "";
	std::string Workshop_filename = replace(mapResult.Name, *" ", *"_"); //replace spaces by underscores


	if (folderpath.back() == '/' || folderpath.back() == '\\')
	{
		Workshop_Dl_Path = folderpath + Workshop_filename;
	}
	else  //if last character != /
	{
		Workshop_Dl_Path = folderpath + "/" + Workshop_filename;
		cvarManager->log("A slash has been added a the end of the path");
	}


	try
	{
		fs::create_directory(Workshop_Dl_Path); //create directory for the map downloaded
		cvarManager->log("Directory Created : " + Workshop_Dl_Path);
	}
	catch (const std::exception& ex) //manage errors when trying to create a folder in an administrator folder
	{
		cvarManager->log(ex.what());
		FolderErrorText = ex.what();
		FolderErrorBool = true;
		return;
	}


	CreateJSONLocalWorkshopInfos(Workshop_filename, Workshop_Dl_Path + "/", mapResult.Name, mapResult.Author, mapResult.Description, mapResult.PreviewUrl);
	cvarManager->log("JSON Created : " + Workshop_Dl_Path + "/" + Workshop_filename + ".json");

	if (Directory_Or_File_Exists(mapResult.ImagePath))
	{
		fs::copy(mapResult.ImagePath, Workshop_Dl_Path + "/" + Workshop_filename + ".jfif"); //copy preview to map directory
		cvarManager->log("Preview pasted : " + Workshop_Dl_Path + "/" + Workshop_filename + ".jfif");
	}
	else
	{
		cvarManager->log("Couldn't find preview to paste");
	}

	
	std::string download_url = "http://rocketleaguemaps.b-cdn.net/" + mapResult.Author + "/Maps/" + mapResult.ZipName;
	cvarManager->log("Download URL : " + download_url);
	std::string Folder_Path = Workshop_Dl_Path + "/" + mapResult.ZipName;


	RLMAPS_IsDownloadingWorkshop = true;

	cvarManager->log("Download Starting...");

	//download
	CurlRequest req;
	req.url = download_url;
	req.progress_function = [this](double file_size, double downloaded, ...)
	{
		//cvarManager->log("Download progress : " + std::to_string(downloaded));
		RLMAPS_Download_Progress = downloaded;
	};

	HttpWrapper::SendCurlRequest(req, [this, Folder_Path, Workshop_Dl_Path](int code, char* data, size_t size)
		{
			std::ofstream out_file{ Folder_Path, std::ios_base::binary };
			if (out_file)
			{
				out_file.write(data, size);

				cvarManager->log("Workshop Downloaded in : " + Workshop_Dl_Path);
				RLMAPS_IsDownloadingWorkshop = false;
			}
		});


	while (RLMAPS_IsDownloadingWorkshop == true)
	{
		cvarManager->log("downloading...............");

		RLMAPS_WorkshopDownload_Progress = RLMAPS_Download_Progress;
		RLMAPS_WorkshopDownload_FileSize = std::stoi(mapResult.Size);
		Sleep(500);
	}

	if (unzipMethod == "Bat")
	{
		CreateUnzipBatchFile(Workshop_Dl_Path, Folder_Path);
	}
	else
	{
		std::string extractCommand = "powershell.exe Expand-Archive -LiteralPath '" + Folder_Path + "' -DestinationPath '" + Workshop_Dl_Path + "'";
		system(extractCommand.c_str());
	}


	int checkTime = 0; //this isn't good but I don't care
	while(UdkInDirectory(Workshop_Dl_Path) == "Null")
	{
		cvarManager->log("Extracting zip file");
		if (checkTime > 10)
		{
			cvarManager->log("Failed extracting the map zip file");
			return;
		}
		Sleep(1000);
		checkTime++;
	}


	cvarManager->log("File Extracted");

	renameFileToUPK(Workshop_Dl_Path);
}






//Textures

void Pluginx64::DownloadWorkshopTextures()
{
	std::string ZipFilePath = BakkesmodPath + "data\\WorkshopMapLoader\\Textures.zip";
	
	if (!Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\Textures.zip")) //le if peut causer des problemes
	{
		IsDownloading_WorkshopTextures = true;

		cvarManager->log("Starting download : Workshop Textures");

		//download
		CurlRequest req;
		req.url = "http://rocketleaguemaps.b-cdn.net/Textures/Textures.zip";
		req.progress_function = [this](double file_size, double downloaded, ...)
		{
			//cvarManager->log("Download progress : " + std::to_string(downloaded));
			Download_Textrures_Progress = downloaded;
		};

		HttpWrapper::SendCurlRequest(req, [this, ZipFilePath](int code, char* data, size_t size)
			{
				std::ofstream out_file{ ZipFilePath, std::ios_base::binary };
				if (out_file)
				{
					out_file.write(data, size);

					cvarManager->log("Textures downloaded : " + ZipFilePath);
					IsDownloading_WorkshopTextures = false;
				}
			});


		while (IsDownloading_WorkshopTextures)
		{
			cvarManager->log("downloading textures.......");

			DownloadTextrures_ProgressDisplayed = Download_Textrures_Progress;
			Sleep(500);
		}
	}

	if (unzipMethod == "Bat")
	{
		CreateUnzipBatchFile(RLCookedPCConsole_Path.string(), ZipFilePath);
	}
	else
	{
		std::string extractCommand = "powershell.exe Expand-Archive -LiteralPath '" + ZipFilePath + "' -DestinationPath '" + RLCookedPCConsole_Path.string() + "'";
		system(extractCommand.c_str());
	}

	cvarManager->log("File Extracted");
}

std::vector<std::string> Pluginx64::CheckExist_TexturesFiles()
{
	std::vector<std::string> missingFiles;
	for (auto textureFile : WorkshopTexturesFilesList)
	{
		if (!Directory_Or_File_Exists(RLCookedPCConsole_Path.string() + "\\" + textureFile))
		{
			missingFiles.push_back(textureFile);
		}
	}

	return missingFiles;
}

bool Pluginx64::MapWasAlreadyInCPCC(Map map)
{
	for (auto item : MapsAlreadyInCPCC)
	{
		if (map.UpkFile.filename().string() == item.filename().string())
		{
			//cvarManager->log(map.UpkFile.filename().string() + " : was already in mods");
			return true;
		}
	}

	//cvarManager->log(map.UpkFile.filename().string() + " : wasn't in mods");
	return false;

}






//Utils

std::wstring Pluginx64::s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::string Pluginx64::replace(std::string s, char c1, char c2)
{
	int l = s.length();

	// loop to traverse in the string 
	for (int i = 0; i < l; i++)
	{
		// check for c1 and replace 
		if (s[i] == c1)
			s[i] = c2;
	}
	return s;
}

std::string Pluginx64::convertToMB(std::string numberToConvert)
{
	if (numberToConvert.length() > 9 && numberToConvert.length() < 13) //If it's GigaBytes
	{
		std::string result = numberToConvert.insert(numberToConvert.length() - 6, ","); //insert a ","
		result = numberToConvert.erase(numberToConvert.length() - 6) + " GB"; //remove the last 4 numbers
		return result;
	}

	if (numberToConvert.length() > 6 && numberToConvert.length() < 10) //If it's MegaBytes
	{
		std::string result = numberToConvert.insert(numberToConvert.length() - 6, ","); //insert a ","
		result = numberToConvert.erase(numberToConvert.length() - 4) + " MB"; //remove the last 4 numbers
		return result;
	}

	if (numberToConvert.length() > 3 && numberToConvert.length() < 7) //If it's KiloBytes
	{
		std::string result = numberToConvert.insert(numberToConvert.length() - 3, ","); //insert a ","
		result = numberToConvert.erase(numberToConvert.length() - 2) + " kB"; //remove the last 2 numbers
		return result;
	}

	if (numberToConvert.length() > 0 && numberToConvert.length() < 4) //If it's Bytes
	{
		std::string result = numberToConvert + " Bytes"; //insert a ","
		return result;
	}
}

bool Pluginx64::Directory_Or_File_Exists(const fs::path& p, fs::file_status s)
{
	bool DirectoryExists;
	if (fs::status_known(s) ? fs::exists(s) : fs::exists(p))
		DirectoryExists = true;
	else
		DirectoryExists = false;

	return DirectoryExists;
}

std::vector<std::string> Pluginx64::FindAllSubstringInAString(std::string texte, std::string beginString, std::string endString)
{
	std::vector<std::string> List;
	std::vector<std::size_t> IndexPos;

	int occurrences = 0;
	std::string::size_type posBegin = 0;
	std::string s = texte;
	std::string BeginTarget = beginString;

	while ((posBegin = s.find(BeginTarget, posBegin)) != std::string::npos)
	{
		++occurrences;
		IndexPos.push_back(posBegin);
		posBegin += BeginTarget.length();
	}

	for (int i = 0; i < IndexPos.size(); i++)
	{
		s = texte;
		s.erase(0, IndexPos.at(i));

		std::string resultString = s.substr(beginString.length(), s.find(endString) - beginString.length());
		//std::cout << "Resultat : " + resultString << std::endl;
		List.push_back(resultString);
		//std::cout << std::endl;
	}
	return List;
}

std::string Pluginx64::UdkInDirectory(std::string dirPath)
{
	for (const auto& file : fs::directory_iterator(dirPath))
	{
		if (file.path().extension().string() == ".udk")
		{
			//cvarManager->log(file.path().string());
			return file.path().string();
		}
	}
	return "Null";
}

void Pluginx64::renameFileToUPK(std::filesystem::path filePath)
{
	std::string UDKPath = UdkInDirectory(filePath.string());
	if (UDKPath == "Null") { return; }
	std::string UPKPath = UDKPath.substr(0, UDKPath.length() - 3) + "upk";
	//cvarManager->log("upk file : " + UPKPath);

	if (rename(UDKPath.c_str(), UPKPath.c_str()) != 0)
		cvarManager->log("Error renaming file");
	else
		cvarManager->log("File renamed successfully");
}

void Pluginx64::SaveInCFG()
{
	std::string filename = BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg";
	std::ofstream CFGFile(filename);

	CFGFile << "MapsFolderPath = \"" + std::string(MapsFolderPathBuf) + "\"\n";
	CFGFile << "Language = \"" + std::to_string(FR) + "\"\n";
	CFGFile << "UnzipMethod = \"" + unzipMethod + "\"\n";
	CFGFile << "HasSeeNewUpdateAlert = \"" + std::to_string(HasSeeNewUpdateAlert) + "\"\n";
	CFGFile << "dontask = \"" + std::to_string(dontAsk) + "\"\n";
	CFGFile << "MapsDisplayMode = \"" + std::to_string(MapsDisplayMode) + "\"\n";
	CFGFile << "nbTilesPerLine = \"" + std::to_string(nbTilesPerLine) + "\"\n";
	CFGFile << "ControllerSensitivity = \"" + std::to_string(ControllerSensitivity) + "\"\n";
	CFGFile << "ControllerScrollSensitivity = \"" + std::to_string(ControllerScrollSensitivity) + "\"";

	CFGFile.close();

	cvarManager->log("Saved in cfg");
}

std::vector<std::string> Pluginx64::GetMapsFolderPathInCfg(std::string cfgFilePath)
{
	std::vector<std::string> CfgInfosList;
	std::string line;
	std::ifstream myfile(cfgFilePath);

	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{
			std::string value = FindAllSubstringInAString(line, "\"", "\"").at(0);
			CfgInfosList.push_back(value.substr(0, value.find("\"")));
		}
		myfile.close();
	}

	return CfgInfosList;
}

void Pluginx64::DownloadPreviewImage(std::string downloadUrl, std::string filePath)
{
	std::string download_url = downloadUrl;
	std::string File_Path = filePath;
	//converting download_url string to LPCWSTR
	std::wstring w_URL = s2ws(download_url);
	LPCWSTR L_URL = w_URL.c_str();
	//converting Folder_Path string to LPCWSTR
	std::wstring w_PATH = s2ws(File_Path);
	LPCWSTR L_PATH = w_PATH.c_str();


	CurlRequest req;
	req.url = download_url;

	HttpWrapper::SendCurlRequest(req, L_PATH, [this, File_Path](int code, std::wstring out_path)
		{
			cvarManager->log("IMAGE PREVIEW DONWLOADED : " + File_Path);
			IsDownloadingPreview = false;
		});

}

bool Pluginx64::FileIsInDirectoryRecursive(std::string dirPath, std::string filename)
{
	for (const auto& dir : fs::directory_iterator(dirPath))
	{
		if (dir.is_directory())
		{
			for (const auto& file : fs::recursive_directory_iterator(dir.path()))
				if (!file.is_directory() && file.path().filename().string() == filename)
				{
					return true;
				}
		}
	}
	return false;
}

float Pluginx64::DoRatio(float x, float y)
{
	float result = x / y;
	return result;
}

//https://www.geeksforgeeks.org/html-parser-in-c-cpp/
void Pluginx64::CleanHTML(std::string& S)
{
	// Store the length of the
	// input string
	int n = S.length();
	int start = 0, end = 0;

	while (end != n - 1)
	{
		n = S.length();
		// Traverse the string
		for (int i = 0; i < n; i++) {
			// If S[i] is '>', update
			// start to i+1 and break
			if (S[i] == '<') {
				start = i;
				break;
			}
			if (i == n - 1)
			{
				return;
			}
		}

		// Remove the blank space
		while (S[start] == ' ') {
			start++;
		}

		// Traverse the string
		for (int i = start; i < n; i++) {
			// If S[i] is '<', update
			// end to i-1 and break
			if (S[i] == '>') {
				end = i;
				break;
			}
		}

		std::string result;
		// Print the characters in the
		// range [start, end]
		for (int j = start; j <= end; j++) {
			result += S[j];
		}

		//cvarManager->log("parser result : " + result);
		if (result == "<br>")
		{
			S.replace(start, (end + 1) - start, "\n");
		}
		else
		{
			S.erase(start, (end + 1) - start);
		}

	}

}

//https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
void Pluginx64::replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

void Pluginx64::eraseAll(std::string& str, const std::string& from) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.erase(start_pos, from.length());
	}
}


//Issues Encountered
void Pluginx64::CheckIssuesEncountered()
{
	IssuesEncountered.clear();

	cpr::Response request_response = cpr::Get(cpr::Url{ "https://bakkesplugins.com/plugins/view/223" });
	if (request_response.status_code != 200)
	{
		cvarManager->log("CheckIssuesEncountered : error " + std::to_string(request_response.status_code));
		return;
	}

	std::vector<std::string> issuesEncountered = FindAllSubstringInAString(request_response.text, "<h2>Issues Encountered</h2>", "<p>This plugin allows you :</p>");

	if (issuesEncountered.size() == 0) {cvarManager->log("No Issues Encountered"); return;}

	cvarManager->log("Issues Encountered :");
	for (auto item : FindAllSubstringInAString(issuesEncountered.at(0), "<li>", "</li>"))
	{
		cvarManager->log("- " + item);
		IssuesEncountered.push_back(item);
	}

	HasSeenIssuesEncountered = false;
}



void Pluginx64::onUnload() {}