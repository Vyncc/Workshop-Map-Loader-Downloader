#include "pch.h"
#include "WorkshopMapLoader.h"


BAKKESMOD_PLUGIN(Pluginx64, "Workshop Map Loader & Downloader", "1.13", 0)


namespace
{
	char dummyChar;
}


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


void Pluginx64::onLoad()
{
	BakkesmodPath = gameWrapper->GetBakkesModPath().string() + "\\";
	IfNoPreviewImagePath = BakkesmodPath + "data\\WorkshopMapLoader\\Search\\NoPreview.jpg";


	if (Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg"))
	{
		cvarManager->log("Workshop Maps Folder : " + GetMapsFolderPathInCfg(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg").at(0));
		MapsFolderPath = GetMapsFolderPathInCfg(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg").at(0);
		unzipMethod = GetMapsFolderPathInCfg(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg").at(2);

		if (GetMapsFolderPathInCfg(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg").at(1) == "0")
		{
			FR = false;
		}
		else
		{
			FR = true;
		}

		if (GetMapsFolderPathInCfg(BakkesmodPath + "data\\WorkshopMapLoader\\workshopmaploader.cfg").at(3) == "0")
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
		MapsFolderPath = "Ex : D:/Workshop Maps/";
		FR = false;
		unzipMethod = "Powershell";
		HasSeeNewUpdateAlert = false;
	}

	strncpy(MapsFolderPathBuf, MapsFolderPath.c_str(), IM_ARRAYSIZE(MapsFolderPathBuf)); //Make  MapsFolderPathBuf = MapsFolderPath

	//cvarManager->executeCommand("cl_settings_refreshplugins");
	cvarManager->log("The bakkesmod path : " + BakkesmodPath);
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

	std::string request_url = "https://backend-02-prd.steamworkshopdownloader.io/api/download/request";
	cpr::Response request_response = cpr::Post(cpr::Url{request_url}, cpr::Body{"{\"publishedFileId\":" + Workshop_id + "}"});
	cvarManager->log("Response : " + request_response.text);


	//get the Workshop uuid
	std::string Workshop_uuid = request_response.text.substr(9, request_response.text.length() - 11);

	//get the state (if prepared)
	std::string status_url = "https://backend-02-prd.steamworkshopdownloader.io/api/download/status";
	cpr::Response state = cpr::Post(cpr::Url{status_url}, cpr::Body{"{\"uuids\":[\"" + Workshop_uuid + "\"]}:"});
	cvarManager->log("State : " + state.text);


	int NbMaxOfRequest = 0;

	//while prepared is not in the state response, we send the request
	while (state.text.find("prepared") != 59 && NbMaxOfRequest < 8)
	{
		//59 is the index "prepared" in the state response

		cvarManager->log("Resending request...");

		state = cpr::Post(cpr::Url{status_url}, cpr::Body{"{\"uuids\":[\"" + Workshop_uuid + "\"]}:"});
		cvarManager->log("State : " + state.text);

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
			return;
		}
		Sleep(2000);
	}

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

	if (state.text.find("prepared") != std::string::npos)
	{
		cvarManager->log("Found !");

		//Get the workshop map infos, name and size
		std::string Workshop_request_url = "https://backend-02-prd.steamworkshopdownloader.io/api/details/file";
		cpr::Response Workshop_request_response = cpr::Post(cpr::Url{Workshop_request_url}, cpr::Body{"[" + Workshop_id + "]:"});

		int WorkshopNameLenght = Workshop_request_response.text.find("\",\"file_description\"") - Workshop_request_response.text.find("\"title_disk_safe\":\"");
		int WorkshopSizeLenght = Workshop_request_response.text.find("\",\"preview_file_size\"") - Workshop_request_response.text.find("\"file_size\":\"");

		std::string Workshop_filename = Workshop_request_response.text.substr(Workshop_request_response.text.find("\"title_disk_safe\":\"") + 19, WorkshopNameLenght - 19);
		cvarManager->log(Workshop_filename);

		std::string Workshop_filesize = Workshop_request_response.text.substr(Workshop_request_response.text.find("\"file_size\":\"") + 13, WorkshopSizeLenght - 13);
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

		std::string download_url = "https://backend-02-prd.steamworkshopdownloader.io/api/download/transmit?uuid=" + Workshop_uuid;
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

			STEAM_WorkshopDownload_ProgressString = STEAM_Download_Progress;
			STEAM_WorkshopDownload_FileSizeString = std::stoi(Workshop_filesize);
			Sleep(500);
		}
		
		if (unzipMethod == "Bat")
		{
			CreateUnzipBatchFile(Workshop_Dl_Path, Folder_Path);
		}
		else
		{
			std::string extractCommand = "powershell.exe Expand-Archive -LiteralPath " + Folder_Path + " -DestinationPath " + Workshop_Dl_Path;
			system(extractCommand.c_str());
		}

		while (UdkInDirectory(Workshop_Dl_Path) == "Null")
		{
			cvarManager->log("Extracting udk file");
			Sleep(10);
		}


		cvarManager->log("File Extracted");

		std::string UDKPath = UdkInDirectory(Workshop_Dl_Path);
		std::string UPKPath = UDKPath.substr(0, UDKPath.length() - 3) + "upk";
		cvarManager->log("upk file : " + UPKPath);

		if (rename(UDKPath.c_str(), UPKPath.c_str()) != 0)
			cvarManager->log("Error renaming file");
		else
			cvarManager->log("File renamed successfully");
	}
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



void Pluginx64::StartSearchRequest(std::string fullurl)
{
	isSearching = true;
	Steam_MapResultList.clear();

	cpr::Response request_response = cpr::Get(cpr::Url{fullurl});
	
	MapsNamesList = FindAllSubstringInAString(request_response.text, "workshopItemTitle ellipsis\">", "</div>"); //get how many maps there is on the page

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


		std::string file_infos_request_url = "https://backend-03-prd.steamworkshopdownloader.io/api/details/file";

		if (!Directory_Or_File_Exists(BakkesmodPath + "data\\WorkshopMapLoader\\Search\\json\\" + resultMapID + ".json")) //if json map infos doesn't exist
		{
			cpr::Response file_infos_request_response = cpr::Post(cpr::Url{ file_infos_request_url }, cpr::Body{ "[" + resultMapID + "]:" });

			int WorkshopMapSizeLenght = file_infos_request_response.text.find("\",\"preview_file_size\"") - file_infos_request_response.text.find("\"file_size\":\"");
			std::string Workshop_filesize = file_infos_request_response.text.substr(file_infos_request_response.text.find("\"file_size\":\"") + 13, WorkshopMapSizeLenght - 13);

			int WorkshopMapDescriptionLenght = file_infos_request_response.text.find("\",\"time_created\":") - file_infos_request_response.text.find("\"file_description\":\"");
			std::string WorkshopMapDescription = file_infos_request_response.text.substr(file_infos_request_response.text.find("\"file_description\":\"") + 20,
				WorkshopMapDescriptionLenght - 20);

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
	isSearching = false;
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


std::vector<std::string> Pluginx64::GetJSONLocalMapInfos(std::string jsonFilePath)
{
	std::vector<std::string> Infos;
	std::string line;
	std::ifstream myfile(jsonFilePath);

	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{
			//std::string MapTitle = line.substr(line.find("\"Title\":\"") + 9, line.find("\",\"Author\":\"") - 9);
			std::string MapTitle = FindAllSubstringInAString(line, "\"Title\":\"", "\",\"").at(0);
			std::string MapDescription = FindAllSubstringInAString(line, "\"Description\":\"", "\",\"").at(0);
			std::string MapAuthor = FindAllSubstringInAString(line, "\"Author\":\"", "\",\"").at(0);
			Infos.push_back(MapTitle);
			Infos.push_back(MapDescription);
			Infos.push_back(MapAuthor);
		}
		myfile.close();
	}

	//{\"Title\":\"" + Title + "\",\"Author\":\"" + Author + "\",\"Description\":\"" + Description + "\",\"PreviewUrl\":\"" + PreviewUrl + "\"}

	return Infos;
}


std::vector<std::string> Pluginx64::GetJSONSearchMapInfos(std::string jsonFilePath)
{
	std::vector<std::string> Infos;
	std::string line;
	std::ifstream myfile(jsonFilePath);

	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{
			std::string MapSize = FindAllSubstringInAString(line, "\"Size\":\"", "\",\"").at(0);
			std::string MapDescription = FindAllSubstringInAString(line, "\"Description\":\"", "\",\"").at(0);
			Infos.push_back(MapSize);
			Infos.push_back(MapDescription);
		}
		myfile.close();
	}

	//"{\"Size\":\"" + Size + "\",\"Description\":\"" + Description + "\"}"
	return Infos;
}


std::vector<std::string> Pluginx64::GetMapsFolderPathInCfg(std::string cfgFilePath)
{
	std::vector<std::string> CfgInfosList;
	std::string line;
	std::ifstream myfile(cfgFilePath);

	if (myfile.is_open())
	{
		int i = 0;
		while (std::getline(myfile, line))
		{
			if (i == 0)
			{
				CfgInfosList.push_back(line.substr(18, line.length() - 19)); // pushback folderPath
			}
			if (i == 1)
			{
				CfgInfosList.push_back(line.substr(12, line.length() - 13)); // pushback language
			}
			if (i == 2)
			{
				CfgInfosList.push_back(line.substr(15, line.length() - 16)); //pushback UnzipMethod
			}
			if (i == 3)
			{
				CfgInfosList.push_back(line.substr(24, line.length() - 25)); //pushback HasSeeNewUpdateAlert
			}
			i++;
		}
		myfile.close();
	}

	return CfgInfosList;
}

void Pluginx64::SaveInCFG(std::string cfgFilePath, std::string mapsfolderpathvariable, std::string languageVariable, std::string unzipMethodVariable, std::string hasSeenNewUpdateAlert)
{
	std::string filename = cfgFilePath;
	std::ofstream CFGFile(filename);

	CFGFile << "MapsFolderPath = \"" + mapsfolderpathvariable + "\"\n";
	CFGFile << "Language = \"" + languageVariable + "\"\n";
	CFGFile << "UnzipMethod = \"" + unzipMethodVariable + "\"\n";
	CFGFile << "HasSeeNewUpdateAlert = \"" + hasSeenNewUpdateAlert + "\"";

	CFGFile.close();
}


void Pluginx64::GetResults(std::string searchType, std::string keyWord)
{
	RLMAPS_MapResultList.clear();

	std::string request_url = rlmaps_url;
	cpr::Response request_response = cpr::Get(cpr::Url{ request_url });


	//Parse response json
	Json::Value actualJson;
	Json::Reader reader;
	
	reader.parse(request_response.text, actualJson);

	const Json::Value maps = actualJson["body"];

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

			RLMAPS_MapResultList.push_back(result);
		}
	}
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

	fs::copy(mapResult.ImagePath, Workshop_Dl_Path + "/" + Workshop_filename + ".jfif"); //copy preview to map directory
	cvarManager->log("Preview pasted : " + Workshop_Dl_Path + "/" + Workshop_filename + ".jfif");

	
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

		RLMAPS_WorkshopDownload_ProgressString = RLMAPS_Download_Progress;
		RLMAPS_WorkshopDownload_FileSizeString = std::stoi(mapResult.Size);
		Sleep(500);
	}

	if (unzipMethod == "Bat")
	{
		CreateUnzipBatchFile(Workshop_Dl_Path, Folder_Path);
	}
	else
	{
		std::string extractCommand = "powershell.exe Expand-Archive -LiteralPath " + Folder_Path + " -DestinationPath " + Workshop_Dl_Path;
		system(extractCommand.c_str());
	}



	while(UdkInDirectory(Workshop_Dl_Path) == "Null")
	{
		cvarManager->log("Extracting udk file");
		Sleep(10);
	}


	cvarManager->log("File Extracted");

	std::string UDKPath = UdkInDirectory(Workshop_Dl_Path);
	std::string UPKPath = UDKPath.substr(0, UDKPath.length() - 3) + "upk";
	cvarManager->log("upk file : " + UPKPath);
	
	if (rename(UDKPath.c_str(), UPKPath.c_str()) != 0)
		cvarManager->log("Error renaming file");
	else
		cvarManager->log("File renamed successfully");
}

std::string Pluginx64::UdkInDirectory(std::string dirPath)
{
	for (const auto& file : fs::directory_iterator(dirPath))
	{
		if (file.path().extension().string() == ".udk")
		{
			cvarManager->log(file.path().string());
			return file.path().string();
		}
	}
	return "Null";
}


void Pluginx64::onUnload() {}