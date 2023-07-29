#include "pch.h"
#include "WorkshopMapLoader.h"


BAKKESMOD_PLUGIN(Pluginx64, "Workshop Map Loader & Downloader", "1.15.3", 0)


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
	RLMAPSLogoImage = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logos/rlmapslogo.png", false, true);

	//Load display mode images
	MapsDisplayMode_Logo1_Image = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logos/logo1.png", false, true);
	MapsDisplayMode_Logo2_Image = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logos/logo2.png", false, true);
	MapsDisplayMode_Logo1_SelectedImage = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logos/logo1_selected.png", false, true);
	MapsDisplayMode_Logo2_SelectedImage = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logos/logo2_selected.png", false, true);


	

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

		if (CFGVariablesList.size() >= 11)
		{
			UseController = (CFGVariablesList.at(10) == "1");
		}


		FR = (CFGVariablesList.at(1) == "1");
		HasSeeNewUpdateAlert = (CFGVariablesList.at(3) == "1");


		if (CFGVariablesList.size() >= 12)
		{
			EnableAntiFreezeFix = (CFGVariablesList.at(11) == "1");
		}

		if (CFGVariablesList.size() >= 11) //the user has the new version
		{
			HasSeeNewUpdateAlert = (PluginVersion == CFGVariablesList.at(9));
		}
		else  //the user has the old version
		{
			HasSeeNewUpdateAlert = false;
		}


		strncpy(MapsFolderPathBuf, MapsFolderPath.c_str(), IM_ARRAYSIZE(MapsFolderPathBuf)); //Make  MapsFolderPathBuf = MapsFolderPath
	}
	else
	{
		cvarManager->log(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg : doesn't exist");

		if (!Directory_Or_File_Exists(RLCookedPCConsole_Path.string() + "\\mods"))
		{
			try
			{
				fs::create_directory(RLCookedPCConsole_Path.string() + "\\mods");
			}
			catch (const std::exception& ex) //manage errors when trying to create a folder in an administrator folder
			{
				cvarManager->log(ex.what());
			}
		}
		MapsFolderPath = RLCookedPCConsole_Path.string() + "\\mods";

		FR = false;
		unzipMethod = "Powershell";
		HasSeeNewUpdateAlert = false;
		dontAsk = 0;
		MapsDisplayMode = 0;
		nbTilesPerLine = 6;
		ControllerSensitivity = 10;
		ControllerScrollSensitivity = 10;
		PluginVersion = "1.15.2";
		EnableAntiFreezeFix = false;

		strncpy(MapsFolderPathBuf, MapsFolderPath.c_str(), IM_ARRAYSIZE(MapsFolderPathBuf)); //Make  MapsFolderPathBuf = MapsFolderPath
		SaveInCFG();
	}


}



void Pluginx64::checkOpenMenuWithController(CanvasWrapper canvas)
{
	if (!UseController)
		return;

	if (gameWrapper->IsInOnlineGame())
		return;

	Gamepad ds4 = Gamepad(1);

	ds4.Update();

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
	selectedButton = 0;

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

						std::vector<std::string> MapInfosList = GetJSONLocalMapInfos(map.JsonFile);
						map.mapName = MapInfosList.at(0);
						map.mapDescription = MapInfosList.at(1);
						map.mapAuthor = MapInfosList.at(2);

						//cvarManager->log("Folder name  : " + CurrentMapDirectory.filename().string());
						cvarManager->log("JSON name  : " + file.path().filename().string());
					}
				}
				else
				{
					if (nbFiles == nbFilesInDirectory && hasFoundJSON == false)
					{
						map.JsonFile = "NoInfos";
						cvarManager->log("No json found in this folder");
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


void Pluginx64::AddMapManually(std::string mapName, std::string mapAuthor, std::string mapDescription, std::filesystem::path mapsDirectoryPath, std::filesystem::path mapFilePath, std::filesystem::path imagePath)
{
	std::string Workshop_Dl_Path = "";
	std::string workshopSafeMapName = replace(mapName, *" ", *"_"); //replace spaces by underscores
	std::string specials[] = { "/", "\\", "?", ":", "*", "\"", "<", ">", "|", "-", "#" };
	for (auto special : specials)
	{
		eraseAll(workshopSafeMapName, special);
	}

	if (mapsDirectoryPath.string().back() == '/' || mapsDirectoryPath.string().back() == '\\')
	{
		Workshop_Dl_Path = mapsDirectoryPath.string() + workshopSafeMapName;
	}
	else  //if last character != /
	{
		Workshop_Dl_Path = mapsDirectoryPath.string() + "/" + workshopSafeMapName;
		//cvarManager->log("A slash has been added a the end of the path");
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


	if (Directory_Or_File_Exists(mapFilePath))
	{
		fs::copy(mapFilePath, Workshop_Dl_Path + "/" + mapFilePath.filename().string()); //copy map file to map directory
		cvarManager->log("Map pasted : " + Workshop_Dl_Path + "/" + mapFilePath.filename().string());
	}
	else
	{
		cvarManager->log("Couldn't find map file to paste");
		return;
	}

	CreateJSONLocalWorkshopInfos(workshopSafeMapName, Workshop_Dl_Path + "/", mapName, mapAuthor, mapDescription, "");
	cvarManager->log("JSON Created : " + Workshop_Dl_Path + "/" + workshopSafeMapName + ".json");

	if (Directory_Or_File_Exists(imagePath))
	{
		fs::copy(imagePath, Workshop_Dl_Path + "/" + imagePath.filename().string()); //copy preview to map directory
		cvarManager->log("Preview pasted : " + Workshop_Dl_Path + "/" + imagePath.filename().string());
	}
	else
	{
		cvarManager->log("Couldn't find preview to paste");
	}

	cvarManager->log(mapName + " added successfully!");
	AddedMapSccuessfully = true;
}





//Download Utils

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


//rocketleaguemaps.us

void Pluginx64::GetResults(std::string keyWord, int IndexPage)
{
	RLMAPS_Searching = true;
	RLMAPS_MapResultList.clear();
	RLMAPS_PageSelected = IndexPage;
	if (IndexPage == 1)
	{
		NumPages = 0;
		std::thread t2(&Pluginx64::GetNumpPages, this, keyWord);
		t2.detach();
	}


	cpr::Response Request_MapInfos = cpr::Get(cpr::Url{ rlmaps_url + keyWord + "&page=" + std::to_string(IndexPage) });
	
	//Parse response json
	Json::Value actualJson;
	Json::Reader reader;

	reader.parse(Request_MapInfos.text, actualJson);

	const Json::Value maps = actualJson;

	RLMAPS_NumberOfMapsFound = maps.size();


	for (int index = 0; index < maps.size(); ++index)
	{
		//GetMapResult(maps, index);
		std::thread t2(&Pluginx64::GetMapResult, this, maps, index);
		t2.detach();


		Sleep(100);
	}

	while (RLMAPS_MapResultList.size() != maps.size())
	{
		Sleep(10);
	}

	RLMAPS_Searching = false;
}


void Pluginx64::GetMapResult(Json::Value maps, int index)
{
	RLMAPS_MapResult result;
	result.ID = maps[index]["id"].asString();
	result.Name = maps[index]["name"].asString();
	result.Description = maps[index]["description"].asString();

	cpr::Response Request_MapDownloadLinks = cpr::Get(cpr::Url{ "https://celab.jetfox.ovh/api/v4/projects/" + result.ID + "/releases" });
	//Parse response json
	Json::Value actualJson2;
	Json::Reader reader2;
	reader2.parse(Request_MapDownloadLinks.text, actualJson2);
	const Json::Value maps2 = actualJson2;


	std::vector<RLMAPS_Release> releases;
	for (int release_index = 0; release_index < maps2.size(); ++release_index)
	{
		RLMAPS_Release release;
		release.name = maps2[release_index]["name"].asString();
		release.tag_name = maps2[release_index]["tag_name"].asString();
		release.description = maps2[release_index]["description"].asString();
		release.pictureLink = maps2[release_index]["assets"]["links"][0]["url"].asString();
		release.downloadLink = maps2[release_index]["assets"]["links"][1]["url"].asString();
		release.zipName = maps2[release_index]["assets"]["links"][1]["name"].asString();


		releases.push_back(release);
	}

	result.releases = releases;
	result.Size = "10000000";
	result.Author = maps[index]["namespace"]["path"].asString();
	result.PreviewUrl = releases[0].pictureLink;

	//cvarManager->log("download url : " + releases[0].downloadLink);
	//GetMapSize(releases[0].downloadLink);


	cvarManager->log("Map : " + result.Name);


	std::filesystem::path resultImagePath = BakkesmodPath + "data\\WorkshopMapLoader\\Search\\img\\RLMAPS\\" + result.ID + ".jfif";



	if (!Directory_Or_File_Exists(resultImagePath)) //if preview image doesn't exist
	{
		//IsDownloadingPreview = true;
		/*RLMAPS_MapResultList[index].IsDownloadingPreview = true;*/
		result.IsDownloadingPreview = true;

		RLMAPS_MapResultList.push_back(result);
		DownloadPreviewImage(result.PreviewUrl, resultImagePath.string(), index);
	}
	else
	{
		std::shared_ptr<ImageWrapper> resultImage;
		bool resultisImageLoaded;

		resultImage = std::make_shared<ImageWrapper>(resultImagePath, false, true);
		resultisImageLoaded = true;

		result.ImagePath = resultImagePath;
		result.Image = resultImage;
		result.isImageLoaded = resultisImageLoaded;


		RLMAPS_MapResultList.push_back(result);
	}


}

void Pluginx64::GetNumpPages(std::string keyWord)
{
	int ResultsSize = 20;
	NumPages = 0;
	while (ResultsSize == 20)
	{
		NumPages++;
		cpr::Response Request_Page = cpr::Get(cpr::Url{ rlmaps_url + keyWord + "&page=" + std::to_string(NumPages) });

		//Parse response json
		Json::Value actualJson;
		Json::Reader reader;

		reader.parse(Request_Page.text, actualJson);
		const Json::Value maps = actualJson;

		ResultsSize = maps.size();
		cvarManager->log("ResultsSize : " + std::to_string(ResultsSize));
	}
}

void Pluginx64::GetMapSize(std::string donwloadUrl)
{
	cpr::Response Request_Page = cpr::Head(cpr::Url{ donwloadUrl });

	/*cvarManager->log("HEADERS : " + Request_Page.raw_header);
	cvarManager->log("Content-Length HEADER LETS GO : " + Request_Page.header.at("Content-Length"));*/

	std::string locationstr = Request_Page.raw_header.substr(Request_Page.raw_header.find("Location: ") + 10, Request_Page.raw_header.find("Vary:") - Request_Page.raw_header.find("Location: ") - 10);
	cvarManager->log("location found : " + locationstr);


	cpr::Response Request_size = cpr::Get(cpr::Url{ locationstr });
	cvarManager->log("HEADERS : " + Request_size.raw_header);

}


//Quick search ctrl+f
std::vector<Map> Pluginx64::QuickSearch_GetMapList(std::string keyWord)
{
	std::vector<Map> List;
	for (auto map : MapList)
	{
		std::string mapName = map.mapName;

		std::transform(mapName.begin(), mapName.end(), mapName.begin(), ::tolower); //transform a string to lowercase
		std::transform(keyWord.begin(), keyWord.end(), keyWord.begin(), ::tolower); //transform a string to lowercase

		if (mapName.find(keyWord) != std::string::npos) //if keyWord is in the mapNameToLower
		{
			List.push_back(map);
		}
	}
	return List;
}



void Pluginx64::RLMAPS_DownloadWorkshop(std::string folderpath, RLMAPS_MapResult mapResult, RLMAPS_Release release)
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
	std::string workshopSafeMapName = replace(mapResult.Name, *" ", *"_"); //replace spaces by underscores
	std::string specials[] = { "/", "\\", "?", ":", "*", "\"", "<", ">", "|", "-", "#" };
	for (auto special : specials)
	{
		eraseAll(workshopSafeMapName, special);
	}


	if (folderpath.back() == '/' || folderpath.back() == '\\')
	{
		Workshop_Dl_Path = folderpath + workshopSafeMapName;
	}
	else  //if last character != /
	{
		Workshop_Dl_Path = folderpath + "/" + workshopSafeMapName;
		//cvarManager->log("A slash has been added a the end of the path");
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


	CreateJSONLocalWorkshopInfos(workshopSafeMapName, Workshop_Dl_Path + "/", mapResult.Name, mapResult.Author, mapResult.Description, mapResult.PreviewUrl);
	cvarManager->log("JSON Created : " + Workshop_Dl_Path + "/" + workshopSafeMapName + ".json");

	if (Directory_Or_File_Exists(mapResult.ImagePath))
	{
		fs::copy(mapResult.ImagePath, Workshop_Dl_Path + "/" + workshopSafeMapName + ".jfif"); //copy preview to map directory
		cvarManager->log("Preview pasted : " + Workshop_Dl_Path + "/" + workshopSafeMapName + ".jfif");
	}
	else
	{
		cvarManager->log("Couldn't find preview to paste");
	}

	
	std::string download_url = release.downloadLink;
	cvarManager->log("Download URL : " + download_url);
	std::string Folder_Path = Workshop_Dl_Path + "/" + release.zipName;

	RLMAPS_WorkshopDownload_Progress = 0;
	RLMAPS_Download_Progress = 0;
	RLMAPS_IsDownloadingWorkshop = true;

	cvarManager->log("Download Starting...");

	//download
	CurlRequest req;
	req.url = download_url;
	req.progress_function = [this](double file_size, double downloaded, ...)
	{
		//cvarManager->log("Download progress : " + std::to_string(downloaded));
		RLMAPS_Download_Progress = downloaded;
		RLMAPS_WorkshopDownload_FileSize = file_size;
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
		req.url = "https://cdn.discordapp.com/attachments/1062156148054179850/1062156149257932821/Workshop-textures.zip";
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
		if (file.path().extension().string() == ".udk" || file.path().extension().string() == ".upk")
		{
			//cvarManager->log(file.path().string());
			return file.path().string();
		}
	}
	return "Null";
}

void Pluginx64::renameFileToUPK(std::filesystem::path filePath)
{
	if (!EnableAntiFreezeFix)
		return;

	for (std::string texture : WorkshopTexturesFilesList)
	{
		if (filePath.filename().string() == texture)
			return;
	}

	std::string UDKPath = UdkInDirectory(filePath.string());
	if (UDKPath == "Null") { return; }
	//cvarManager->log("upk file : " + UPKPath);


	//This is to fix the crashing issues when loading maps
	//What it does ? it only renames the map, that's what fixes the crashing issue actually, wtf Psyonix?????
	if (UDKPath.find("_antifreeze") != std::string::npos)
	{
		std::string oldUDKPath = UDKPath;
		eraseAll(UDKPath, "_antifreeze");

		/*cvarManager->log("oldUDKPath : " + oldUDKPath);
		cvarManager->log("UDKPath : " + UDKPath);*/

		if (rename(oldUDKPath.c_str(), UDKPath.c_str()) != 0)
			cvarManager->log("Error renaming file");
		else
			cvarManager->log("File renamed successfully");
	}
	else
	{
		std::string UPKPath_antifreeze = UDKPath.substr(0, UDKPath.length() - 4) + "_antifreeze.upk";

		/*cvarManager->log("UPKPath_antifreeze : " + UPKPath_antifreeze);
		cvarManager->log("UDKPath : " + UDKPath);*/

		if (rename(UDKPath.c_str(), UPKPath_antifreeze.c_str()) != 0)
			cvarManager->log("Error renaming file for antifreeze");
		else
			cvarManager->log("File renamed successfully for antifreeze");
	}

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
	CFGFile << "ControllerScrollSensitivity = \"" + std::to_string(ControllerScrollSensitivity) + "\"\n";
	CFGFile << "PluginVersion = \"" + PluginVersion + "\"\n";
	CFGFile << "UseController = \"" + std::to_string(UseController) + "\"\n";
	CFGFile << "EnableAntiFreezeFix = \"" + std::to_string(EnableAntiFreezeFix) + "\"";

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

void Pluginx64::DownloadPreviewImage(std::string downloadUrl, std::string filePath, int mapResultIndex)
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

	HttpWrapper::SendCurlRequest(req, L_PATH, [this, File_Path, mapResultIndex, filePath](int code, std::wstring out_path)
		{
			cvarManager->log("PREVIEW DONWLOADED : " + File_Path);

			Sleep(100);

			RLMAPS_MapResultList[mapResultIndex].IsDownloadingPreview = false;


			std::shared_ptr<ImageWrapper> resultImage;
			bool resultisImageLoaded;

			resultImage = std::make_shared<ImageWrapper>(filePath, false, true);
			resultisImageLoaded = true;

			RLMAPS_MapResultList[mapResultIndex].ImagePath = filePath;
			RLMAPS_MapResultList[mapResultIndex].Image = resultImage;
			RLMAPS_MapResultList[mapResultIndex].isImageLoaded = resultisImageLoaded;

			cvarManager->log("Downloaded/loaded preview for " + RLMAPS_MapResultList[mapResultIndex].Name);
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
	if (S == "")
		return;
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

std::vector<std::string> Pluginx64::GetDrives()
{
	std::vector<std::string> Drives;
	int iCounter = 0;
	int iASCIILetter = (int)'a';

	DWORD dwDrivesMask = GetLogicalDrives();

	if (dwDrivesMask == 0) {
		printf("Failed to acquire mask of drives.\n");
		exit(EXIT_FAILURE);
	}

	printf("Drives available:\n");
	//extract the drives from the mask using 
	//logical and-ing with & and bitshift operator <<
	while (iCounter < 24) {
		if (dwDrivesMask & (1 << iCounter)) {
			printf("%c:\\ \n", iASCIILetter + iCounter);
			char driveLetter = iASCIILetter + iCounter;
			std::string str(1, driveLetter);
			Drives.push_back(str);
		}
		iCounter++;
	}
	return Drives;
}





void Pluginx64::onUnload() {}