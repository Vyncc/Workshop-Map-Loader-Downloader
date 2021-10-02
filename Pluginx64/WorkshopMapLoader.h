#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"


struct Map
{
	std::filesystem::path Folder; //Map folder
	std::filesystem::path UpkFile; //Map(.upk) in the map directory
	std::string JsonFile; //.json file found in your map directory
	std::shared_ptr<ImageWrapper> PreviewImage;
	bool isPreviewImageLoaded;
};

struct Steam_MapResult
{
	std::string ID;
	std::string Name;
	std::string Size;
	std::string Description;
	std::string PreviewUrl;
	std::string Author;
	std::filesystem::path ImagePath; //Stored in bakkesmodFolder/data/WorkshopMapLoader/Search/img/
	std::shared_ptr<ImageWrapper> Image;
	bool isImageLoaded;
};

struct RLMAPS_MapResult
{
	std::string ID;
	std::string Name;
	std::string ZipName;
	std::string Size;
	std::string ShortDescription;
	std::string Description;
	std::string PreviewUrl;
	std::string Author;
	std::filesystem::path ImagePath; //Stored in bakkesmodFolder/data/WorkshopMapLoader/Search/img/
	std::shared_ptr<ImageWrapper> Image;
	bool isImageLoaded;
};



class Pluginx64 : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow, public BakkesMod::Plugin::PluginSettingsWindow
{
public:
	virtual void onLoad();
	virtual void onUnload();

	int widthTest;
	int heightTest;



	void TextCenter(std::string text);
	void CenterNexIMGUItItem(float itemWidth);
	void AlignRightNexIMGUItItem(float itemWidth, float borderGap);

	std::shared_ptr<ImageWrapper> SteamLogoImage;
	std::shared_ptr<ImageWrapper> RLMAPSLogoImage;

	std::string UdkInDirectory(std::string dirPath);

	bool STEAM_browsing;
	bool RLMAPS_browsing;

	bool RLMAPS_Searching;

	float widthBrowseGroup;

	int STEAM_NumberOfMapsFound;
	int RLMAPS_NumberOfMapsFound;

	//Variables
	std::string BakkesmodPath;
	std::string MapsFolderPath;
	static bool FR;
	std::string unzipMethod = "Bat";
	bool HasSeeNewUpdateAlert;
	std::string IfNoPreviewImagePath;
	std::string steam_base_url = "https://steamcommunity.com/workshop/browse/?appid=252950&searchtext=";
	std::string rlmaps_url = "http://rocketleaguemaps.us/api/getmultimap.php";
	std::string rlmaps_offset_url = "http://usa2.rocketleaguemaps.tk/api/getmultimapoffset.php?offset=";
	static char MapsFolderPathBuf[200];


	void SaveInCFG(std::string cfgFilePath, std::string mapsfolderpathvariable, std::string languageVariable, std::string unzipMethodVariable, std::string hasSeenNewUpdateAlert);
	std::vector<std::string> GetMapsFolderPathInCfg(std::string cfgFilePath);


	//Local Maps
	std::vector<Map> MapList;
	std::vector<std::string> GetJSONLocalMapInfos(std::string jsonFilePath);
	void RefreshMapsFunct(std::string mapsfolders);


	//Search Workshop
	//Steam
	std::vector<Steam_MapResult> Steam_MapResultList;
	std::vector<std::string> OtherPagesList;
	void StartSearchRequest(std::string fullurl);
	void DownloadPreviewImage(std::string downloadUrl, std::string filePath);
	void CreateJSONSearchWorkshopInfos(std::string jsonFileName, std::string workshopMapPath, std::string mapSize, std::string mapDescription);
	std::vector<std::string> GetJSONSearchMapInfos(std::string jsonFilePath);
	bool STEAM_Searching = false;
	//rocketleaguemaps.us
	std::vector<RLMAPS_MapResult> RLMAPS_MapResultList;
	void GetResults(std::string searchType, std::string keyWord);
	void GetResultsBrowseMaps(int offset);
	int CurrentPage;


	//Related to download
	//Steam
	void STEAM_DownloadWorkshop(std::string workshopURL, std::string Dfolderpath, std::string workshopid, bool WorkshopIDByUrl, int index, bool createJsonFile);
	std::string GetWorkshopIDByUrl(std::string workshopurl);
	void CreateUnzipBatchFile(std::string destinationPath, std::string zipFilePath);
	void CreateJSONLocalWorkshopInfos(std::string jsonFileName, std::string workshopMapPath, std::string mapTitle, std::string mapAuthor, std::string mapDescription, std::string mapPreviewUrl);
	bool IsRetrievingWorkshopFiles = false;
	bool DownloadFailed = false;
	bool UserIsChoosingYESorNO = false;
	bool FolderErrorBool = false;
	bool STEAM_IsDownloadingWorkshop = false;
	bool AcceptTheDownload = false;
	int STEAM_Download_Progress;
	int STEAM_WorkshopDownload_ProgressString;
	int STEAM_WorkshopDownload_FileSizeString;
	bool IsDownloadingPreview;

	//rocketleaguemaps.us
	void RLMAPS_DownloadWorkshop(std::string folderpath, RLMAPS_MapResult mapResult);
	int RLMAPS_Download_Progress;
	int RLMAPS_WorkshopDownload_ProgressString;
	int RLMAPS_WorkshopDownload_FileSizeString;
	bool RLMAPS_IsDownloadingWorkshop = false;



	//utils
	std::wstring s2ws(const std::string& s);
	std::string replace(std::string s, char c1, char c2);
	std::string convertToMB(std::string numberToConvert);
	std::vector<std::string> FindAllSubstringInAString(std::string texte, std::string beginString, std::string endString);
	bool Directory_Or_File_Exists(const fs::path& p, fs::file_status s = fs::file_status{});
	void renameFileToUPK(std::filesystem::path filePath);


	//ImGui SettingsWindow Functions
	void RenderSettings() override;
	std::string GetPluginName() override;


	//ImGui Functions
	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "Workshop Map Loader & Downloader v1.13";

	void renderProgressBar(float value, float maxValue, ImVec2 pos, ImVec2 size, ImColor colorBackground, ImColor colorProgress, const char* label);

	void renderInfoPopup(const char* popupName, const char* label);
	void renderYesNoPopup(const char* popupName, const char* label, std::function<void()> yesFunc, std::function<void()> noFunc);

	void renderFolderErrorPopup();
	void renderLaunchModePopup(Map curMap);
	void renderAcceptDownload();
	void renderDownloadFailedPopup();

	void renderSortByCombos(std::string mostPopular_url);

	void Render() override;
	void renderMaps();
	void Steam_RenderAResult(int i, ImDrawList* drawList, static char mapspath[200]);
	void Steam_renderSearchWorkshopResults(static char mapspath[200]);
	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;



	void RLMAPS_RenderAResult(int i, ImDrawList* drawList, static char mapspath[200]);
	void RLMAPS_renderSearchWorkshopResults(static char mapspath[200]);

	//////////////////////////////////////////////////////////////////////////////////////////////////////  Text Variables

	//1st Tab
	std::string Tab1MapLoaderText;
	std::string Label1Text; //Put the folder's path of the maps, don't forget to add a  /  at the end.
	std::string RefreshMapsButtonText;
	std::string SavePathText;
	//context menu strip
	std::string OpenMapDirText;
	std::string DeleteMapText;


	//2nd Tab
	std::string Tab2SearchWorkshopText;
	std::string DlWorkshopByURLText;
	std::string Label2Text; //"Steam Workshop Url :"
	std::string DownloadButtonText;
	std::string Label3Text;
	std::string SearchButtonText;
	std::string SearchingText;
	std::string RetrievingFilesText;
	std::string DownloadingText;
	std::string WorkshopsFoundText;

	std::string PeriodText[8];
	std::string SortByText[4];


	//Search Result
	std::string ResultByText;
	std::string ResultSizeText;
	std::string DownloadMapButtonText;
	//context menu strip
	std::string GoToUrlSelectableText;


	//Warnings
	std::string DirNotExistText;
	std::string DownloadFailedText; std::string DownloadFailedErrorText;
	std::string FolderErrorText;
	std::string WantToDawnloadText;
	std::string YESButtonText;
	std::string NOButtonText;
	std::string IsDownloadDingWarningText;
};
