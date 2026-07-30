#include "pch.h"
#include "WorkshopMapLoader.h"


BAKKESMOD_PLUGIN(Pluginx64, "Workshop Map Loader & Downloader", "1.16", 0)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
std::shared_ptr<GameWrapper> _globalGameWrapper;

namespace
{
	std::string TrimConfigValue(const std::string& p_value)
	{
		const size_t firstCharacter = p_value.find_first_not_of(" \t\r\n");
		if(firstCharacter == std::string::npos)
			return "";

		const size_t lastCharacter = p_value.find_last_not_of(" \t\r\n");
		return p_value.substr(firstCharacter, lastCharacter - firstCharacter + 1);
	}

	std::string ToLowerConfigValue(std::string p_value)
	{
		std::transform(p_value.begin(), p_value.end(), p_value.begin(), [](unsigned char p_character)
			{
				return static_cast<char>(std::tolower(p_character));
			});
		return p_value;
	}

	bool ParseConfigBool(const std::string& p_value, bool p_defaultValue)
	{
		const std::string value = ToLowerConfigValue(TrimConfigValue(p_value));
		if(value == "1" || value == "true" || value == "yes" || value == "on")
			return true;
		if(value == "0" || value == "false" || value == "no" || value == "off")
			return false;

		return p_defaultValue;
	}

	int ParseConfigInt(const std::string& p_value, int p_defaultValue, int p_minimum, int p_maximum)
	{
		try
		{
			size_t parsedCharacterCount = 0;
			const int value = std::stoi(TrimConfigValue(p_value), &parsedCharacterCount);
			if(parsedCharacterCount == TrimConfigValue(p_value).size() && value >= p_minimum && value <= p_maximum)
				return value;
		}
		catch(const std::exception&)
		{
		}

		return p_defaultValue;
	}
}

void Pluginx64::onLoad()
{
	_globalCvarManager = cvarManager;
	_globalGameWrapper = gameWrapper;

	m_bakkesPluginsMapInstaller.Initialize(cvarManager);
	static std::once_flag curlInitFlag;
	std::call_once(curlInitFlag, []() {
		curl_global_init(CURL_GLOBAL_DEFAULT);
		});

	g_pluginConfig.BakkesmodPath = gameWrapper->GetBakkesModPath().string() + "\\";
	IfNoPreviewImagePath = g_pluginConfig.BakkesmodPath + "data\\WorkshopMapLoader\\Search\\NoPreview.jpg";

	std::string RLWin64_Path = std::filesystem::current_path().string();
	RLCookedPCConsole_Path = RLWin64_Path.substr(0, RLWin64_Path.length() - 14) + "TAGame\\CookedPCConsole";

	std::string Data_WorkshopMapLoader_Path = g_pluginConfig.BakkesmodPath + "data\\WorkshopMapLoader\\";

	//Load display mode images
	MapsDisplayMode_Logo1_Image = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logos/logo1.png", false, true);
	MapsDisplayMode_Logo2_Image = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logos/logo2.png", false, true);
	MapsDisplayMode_Logo1_SelectedImage = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logos/logo1_selected.png", false, true);
	MapsDisplayMode_Logo2_SelectedImage = std::make_shared<ImageWrapper>(Data_WorkshopMapLoader_Path + "logos/logo2_selected.png", false, true);

	LoadConfig();
}




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
		g_pluginTexts.FolderErrorText = ex.what();
		FolderErrorBool = true;
		return;
	}


	std::string newZipFilePath = mapDestinationFolder + "/" + mapFilePath.filename().string();

	if (Utils::DirectoryOrFileExists(mapFilePath))
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

	if (Utils::DirectoryOrFileExists(imagePath))
	{
		fs::copy(imagePath, mapDestinationFolder + "/" + imagePath.filename().string()); //copy preview to map directory
		cvarManager->log("Preview pasted : " + mapDestinationFolder + "/" + imagePath.filename().string());
	}
	else
	{
		cvarManager->log("Couldn't find preview to paste");
	}

	if (g_pluginConfig.UnzipMethod == "Bat")
	{
		Utils::CreateUnzipBatchFile(g_pluginConfig.BakkesmodPath, mapDestinationFolder, newZipFilePath);
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

void Pluginx64::CreateJSONLocalWorkshopInfos(std::string jsonFileName, std::string workshopMapPath, std::string mapTitle, std::string mapAuthor,
	std::string mapDescription, std::string mapPreviewUrl)
{
	std::string filename = workshopMapPath + jsonFileName + ".json";
	std::ofstream JSONFile(filename);

	std::string Title = mapTitle;
	std::string Author = mapAuthor;
	std::string Description = mapDescription;
	std::string PreviewUrl = mapPreviewUrl;

	JSONFile << "{\"Title\":\"" + Title + "\",\"Author\":\"" + Author + "\",\"Description\":\"" + Description + "\",\"PreviewUrl\":\"" + PreviewUrl + "\"}";
	JSONFile.close();
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
	std::string ZipFilePath = g_pluginConfig.BakkesmodPath + "data\\WorkshopMapLoader\\Textures.zip";
	
	if (!Utils::DirectoryOrFileExists(g_pluginConfig.BakkesmodPath + "data\\WorkshopMapLoader\\Textures.zip")) //le if peut causer des problemes
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

	if (g_pluginConfig.UnzipMethod == "Bat")
	{
		Utils::CreateUnzipBatchFile(g_pluginConfig.BakkesmodPath, RLCookedPCConsole_Path.string(), ZipFilePath);
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
		if (!Utils::DirectoryOrFileExists(RLCookedPCConsole_Path.string() + "\\" + textureFile))
		{
			missingFiles.push_back(textureFile);
		}
	}

	return missingFiles;
}



//Utils

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

	if (g_pluginConfig.EnableAntiFreezeFix)
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
	std::string filename = g_pluginConfig.BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg";
	std::ofstream CFGFile(filename);

	CFGFile << "MapsFolderPath = \"" + std::string(g_pluginConfig.MapsFolderPathBuffer) + "\"\n";
	CFGFile << "Language = \"" + std::to_string(g_pluginConfig.Language) + "\"\n";
	CFGFile << "UnzipMethod = \"" + g_pluginConfig.UnzipMethod + "\"\n";
	CFGFile << "HasSeeNewUpdateAlert = \"" + std::to_string(g_pluginConfig.HasSeeNewUpdateAlert) + "\"\n";
	CFGFile << "dontask = \"" + std::to_string(g_pluginConfig.DontAsk) + "\"\n";
	CFGFile << "MapsDisplayMode = \"" + std::to_string(g_pluginConfig.MapsDisplayMode) + "\"\n";
	CFGFile << "nbTilesPerLine = \"" + std::to_string(g_pluginConfig.NbTilesPerLine) + "\"\n";
	CFGFile << "PluginVersion = \"" + PluginVersion + "\"\n";
	CFGFile << "EnableAntiFreezeFix = \"" + std::to_string(g_pluginConfig.EnableAntiFreezeFix) + "\"";

	CFGFile.close();
	cvarManager->log("Saved in cfg");
}

void Pluginx64::LoadConfig()
{
	const std::string configFilePath = g_pluginConfig.BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg";
	const std::string defaultMapsFolderPath = (RLCookedPCConsole_Path / "mods").string();
	std::unordered_map<std::string, std::string> configValues;
	std::string line;
	std::ifstream configFile(configFilePath);
	const bool configExists = configFile.is_open();

	g_pluginConfig.MapsFolderPath = defaultMapsFolderPath;
	g_pluginConfig.Language = false;
	g_pluginConfig.UnzipMethod = "Powershell";
	g_pluginConfig.HasSeeNewUpdateAlert = false;
	g_pluginConfig.DontAsk = 0;
	g_pluginConfig.MapsDisplayMode = 0;
	g_pluginConfig.NbTilesPerLine = 6;
	g_pluginConfig.EnableAntiFreezeFix = false;

	if(configExists)
	{
		while(std::getline(configFile, line))
		{
			line = TrimConfigValue(line);
			if(line.empty() || line[0] == '#' || line[0] == ';' || line.rfind("//", 0) == 0)
				continue;

			const size_t separatorPosition = line.find('=');
			if(separatorPosition == std::string::npos)
				continue;

			const std::string key = ToLowerConfigValue(TrimConfigValue(line.substr(0, separatorPosition)));
			std::string value = TrimConfigValue(line.substr(separatorPosition + 1));
			if(value.size() >= 2 && ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\'')))
				value = value.substr(1, value.size() - 2);

			if(!key.empty())
				configValues[key] = value;
		}

		configFile.close();

		if(configValues.count("mapsfolderpath") && !TrimConfigValue(configValues["mapsfolderpath"]).empty())
			g_pluginConfig.MapsFolderPath = TrimConfigValue(configValues["mapsfolderpath"]);

		if(configValues.count("language"))
			g_pluginConfig.Language = ParseConfigBool(configValues["language"], g_pluginConfig.Language);
		if(configValues.count("hasseenewupdatealert"))
			g_pluginConfig.HasSeeNewUpdateAlert = ParseConfigBool(configValues["hasseenewupdatealert"], g_pluginConfig.HasSeeNewUpdateAlert);
		if(configValues.count("dontask"))
			g_pluginConfig.DontAsk = ParseConfigInt(configValues["dontask"], g_pluginConfig.DontAsk, 0, 1);
		if(configValues.count("mapsdisplaymode"))
			g_pluginConfig.MapsDisplayMode = ParseConfigInt(configValues["mapsdisplaymode"], g_pluginConfig.MapsDisplayMode, 0, 1);
		if(configValues.count("nbtilesperline"))
			g_pluginConfig.NbTilesPerLine = ParseConfigInt(configValues["nbtilesperline"], g_pluginConfig.NbTilesPerLine, 2, 14);
		if(configValues.count("enableantifreezefix"))
			g_pluginConfig.EnableAntiFreezeFix = ParseConfigBool(configValues["enableantifreezefix"], g_pluginConfig.EnableAntiFreezeFix);

		if(configValues.count("unzipmethod"))
		{
			const std::string method = ToLowerConfigValue(TrimConfigValue(configValues["unzipmethod"]));
			if(method == "bat" || method == "batch" || method == "1")
				g_pluginConfig.UnzipMethod = "Bat";
			else if(method == "powershell" || method == "0")
				g_pluginConfig.UnzipMethod = "Powershell";
		}

		if(!configValues.count("pluginversion") || configValues["pluginversion"] != PluginVersion)
			g_pluginConfig.HasSeeNewUpdateAlert = false;
	}
	else
		cvarManager->log(configFilePath + " doesn't exist");

	if(g_pluginConfig.MapsFolderPath == defaultMapsFolderPath && !Utils::DirectoryOrFileExists(defaultMapsFolderPath))
	{
		try
		{
			fs::create_directories(defaultMapsFolderPath);
		}
		catch(const std::exception& p_exception)
		{
			cvarManager->log(p_exception.what());
		}
	}

	strncpy(g_pluginConfig.MapsFolderPathBuffer, g_pluginConfig.MapsFolderPath.c_str(), IM_ARRAYSIZE(g_pluginConfig.MapsFolderPathBuffer) - 1);
	g_pluginConfig.MapsFolderPathBuffer[IM_ARRAYSIZE(g_pluginConfig.MapsFolderPathBuffer) - 1] = '\0';
	cvarManager->log("Workshop Maps Folder : " + g_pluginConfig.MapsFolderPath);

	if(!configExists)
		SaveInCFG();

	g_pluginTexts.SetLanguage(g_pluginConfig.Language);
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





void Pluginx64::onUnload()
{
	m_bakkesPluginsMapInstaller.Shutdown();
}
