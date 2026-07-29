#include "pch.h"
#include "WorkshopMapLoader.h"


BAKKESMOD_PLUGIN(Pluginx64, "Workshop Map Loader & Downloader", "1.15.3", 0)

std::shared_ptr<GameWrapper> _globalGameWrapper;


void Pluginx64::onLoad()
{
	_globalGameWrapper = gameWrapper;

	BakkesmodPath = gameWrapper->GetBakkesModPath().string() + "\\";
	IfNoPreviewImagePath = BakkesmodPath + "data\\WorkshopMapLoader\\Search\\NoPreview.jpg";

	std::string RLWin64_Path = std::filesystem::current_path().string();
	RLCookedPCConsole_Path = RLWin64_Path.substr(0, RLWin64_Path.length() - 14) + "TAGame\\CookedPCConsole";

	std::string Data_WorkshopMapLoader_Path = BakkesmodPath + "data\\WorkshopMapLoader\\";

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
		PluginVersion = "1.15.2";
		EnableAntiFreezeFix = false;

		strncpy(MapsFolderPathBuf, MapsFolderPath.c_str(), IM_ARRAYSIZE(MapsFolderPathBuf)); //Make  MapsFolderPathBuf = MapsFolderPath
		SaveInCFG();
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
	std::string mapDestinationFolder = "";
	std::string workshopSafeMapName = replace(mapName, *" ", *"_"); //replace spaces by underscores
	std::string specials[] = { "/", "\\", "?", ":", "*", "\"", "<", ">", "|", "-", "#" };
	for (auto special : specials)
	{
		eraseAll(workshopSafeMapName, special);
	}

	if (mapsDirectoryPath.string().back() == '/' || mapsDirectoryPath.string().back() == '\\')
	{
		mapDestinationFolder = mapsDirectoryPath.string() + workshopSafeMapName;
	}
	else  //if last character != /
	{
		mapDestinationFolder = mapsDirectoryPath.string() + "/" + workshopSafeMapName;
		//cvarManager->log("A slash has been added a the end of the path");
	}


	try
	{
		fs::create_directory(mapDestinationFolder); //create directory for the map downloaded
		cvarManager->log("Directory Created : " + mapDestinationFolder);
	}
	catch (const std::exception& ex) //manage errors when trying to create a folder in an administrator folder
	{
		cvarManager->log(ex.what());
		FolderErrorText = ex.what();
		FolderErrorBool = true;
		return;
	}


	std::string newZipFilePath = mapDestinationFolder + "/" + mapFilePath.filename().string();

	if (Directory_Or_File_Exists(mapFilePath))
	{
		fs::copy(mapFilePath, newZipFilePath); //copy map zip file to map directory
		cvarManager->log("Map pasted : " + newZipFilePath);
	}
	else
	{
		cvarManager->log("Couldn't find map file to paste");
		return;
	}

	CreateJSONLocalWorkshopInfos(workshopSafeMapName, mapDestinationFolder + "/", mapName, mapAuthor, mapDescription, "");
	cvarManager->log("JSON Created : " + mapDestinationFolder + "/" + workshopSafeMapName + ".json");

	if (Directory_Or_File_Exists(imagePath))
	{
		fs::copy(imagePath, mapDestinationFolder + "/" + imagePath.filename().string()); //copy preview to map directory
		cvarManager->log("Preview pasted : " + mapDestinationFolder + "/" + imagePath.filename().string());
	}
	else
	{
		cvarManager->log("Couldn't find preview to paste");
	}

	if (unzipMethod == "Bat")
	{
		CreateUnzipBatchFile(mapDestinationFolder, newZipFilePath);
	}
	else
	{
		std::string extractCommand = "powershell.exe Expand-Archive -LiteralPath '" + newZipFilePath + "' -DestinationPath '" + mapDestinationFolder + "'";
		system(extractCommand.c_str());
	}

	int checkTime = 0; //this isn't good but I don't care
	while (UdkInDirectory(mapDestinationFolder) == "Null")
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

	cvarManager->log(mapName + " added successfully!");
	AddedMapSccuessfully = true;

	gameWrapper->SetTimeout([this, mapDestinationFolder](GameWrapper* gw) {
		renameFileToUPK(mapDestinationFolder);
		}, 1.f);
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
	for (std::string texture : WorkshopTexturesFilesList)
	{
		if (filePath.filename().string() == texture)
			return;
	}

	std::string UDKPath = UdkInDirectory(filePath.string());
	if (UDKPath == "Null")
	{
		cvarManager->log("[ERROR] File \"" + filePath.string() + "\" not found!");
		return;
	}

	cvarManager->log("udk file : " + UDKPath);

	std::filesystem::path oldPath = UDKPath;
	std::filesystem::path newPath = oldPath;
	newPath.replace_extension(".upk");
	rename(oldPath.string().c_str(), newPath.string().c_str());

	cvarManager->log("file renamed : " + newPath.string());

	if (EnableAntiFreezeFix)
	{
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
	CFGFile << "PluginVersion = \"" + PluginVersion + "\"\n";
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