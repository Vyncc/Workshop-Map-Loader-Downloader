#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "Gamepad.h"

//#include <WinUser.h>


struct Map
{
	std::string mapName;
	std::string mapDescription;
	std::string mapAuthor;

	std::filesystem::path Folder; //Map folder
	std::filesystem::path UpkFile; //Map(.upk) in the map directory
	std::filesystem::path ZipFile; //Map(.zip) in the map directory
	std::string JsonFile; //.json file found in the map directory
	std::shared_ptr<ImageWrapper> PreviewImage;
	bool isPreviewImageLoaded;
};

struct RLMAPS_Release
{
	std::string name;
	std::string tag_name;
	std::string description;
	std::string zipName;
	std::string downloadLink;
	std::string pictureLink;
};

struct RLMAPS_MapResult
{
	std::string ID;
	std::string Name;
	std::string Size;
	std::string Description;
	std::string PreviewUrl;
	std::string Author;
	std::vector<RLMAPS_Release> releases;
	std::filesystem::path ImagePath; //Stored in bakkesmodFolder/data/WorkshopMapLoader/Search/img/
	std::shared_ptr<ImageWrapper> Image;
	bool isImageLoaded;
	bool IsDownloadingPreview;
};

struct GameSetting
{
	std::string Name;
	int selectedValue = 0;
	std::vector<std::string> DisplayValuesNames;
	std::vector<std::string> Values;

	std::string GetSelectedValue();
};

struct mapButtonPos
{
	ImVec2 rectMin;
	ImVec2 rectMax;
	ImVec2 cursorPos;
	bool isDisplayed;
};


//Hey Jerry or Martinn, have a good time reviewing my awful code ;)

class Pluginx64 : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow, public BakkesMod::Plugin::PluginSettingsWindow
{
public:
	virtual void onLoad();
	virtual void onUnload();

	/*
	int widthTest;
	int heightTest;
	float fontSizeTest = 0.f;
	*/

	bool EnableAntiFreezeFix = false;

	int SearchRequestCounter = 0;
	bool isHoveringMapButton;

	ImVec2 MapButtonChild_TopPos;
	std::vector<mapButtonPos> mapButtonList;
	int selectedButton = 0;

	bool UseController = false;
	void checkOpenMenuWithController(CanvasWrapper canvas);
	int ControllerSensitivity = 10;
	int ControllerScrollSensitivity = 10;



	//Textures
	std::vector<std::string> WorkshopTexturesFilesList =
	{
		"EditorLandscapeResources.upk", "EditorMaterials.upk", "EditorMeshes.upk", "EditorResources.upk", "Engine_MI_Shaders.upk", "EngineBuildings.upk", "EngineDebugMaterials.upk",
		"EngineMaterials.upk", "EngineResources.upk", "EngineVolumetrics.upk", "MapTemplateIndex.upk", "MapTemplates.upk", "mods.upk", "NodeBuddies.upk"
	};
	bool IsDownloading_WorkshopTextures;
	std::vector<std::string> CheckExist_TexturesFiles();
	void DownloadWorkshopTextures();
	int Download_Textrures_Progress;
	int DownloadTextrures_ProgressDisplayed;


	

	//Variables
	std::string BakkesmodPath;
	std::string MapsFolderPath;
	std::string PluginVersion = "1.15.3";
	static bool FR;
	std::string unzipMethod = "Bat";
	bool HasSeeNewUpdateAlert;
	int dontAsk = 0;
	bool DownloadTexturesBool = false;
	std::string IfNoPreviewImagePath;
	std::string steam_base_url = "https://steamcommunity.com/workshop/browse/?appid=252950&searchtext=";
	std::string rlmaps_url = "https://celab.jetfox.ovh/api/v4/projects/?search=";
	std::string rlmaps_offset_url = "http://www.rocketleaguemaps.us/api/getmultimapoffset.php?offset=";
	static char MapsFolderPathBuf[200];
	std::filesystem::path RLCookedPCConsole_Path;


	//Local Maps
	std::vector<Map> MapList;
	std::vector<std::string> GetJSONLocalMapInfos(std::string jsonFilePath);
	void RefreshMapsFunct(std::string mapsfolders);
	void AddMapManually(std::string mapName, std::string mapAuthor, std::string mapDescription, std::filesystem::path mapsDirectoryPath, std::filesystem::path mapFilePath, std::filesystem::path imagePath);

	//Display mode
	int nbTilesPerLine = 5;
	float maxTextSize = 160.f;
	int MapsDisplayMode = 0;
	std::shared_ptr<ImageWrapper> MapsDisplayMode_Logo1_Image;
	std::shared_ptr<ImageWrapper> MapsDisplayMode_Logo1_SelectedImage;
	std::shared_ptr<ImageWrapper> MapsDisplayMode_Logo2_Image;
	std::shared_ptr<ImageWrapper> MapsDisplayMode_Logo2_SelectedImage;


	//Search Workshop
	//rocketleaguemaps.us
	std::vector<RLMAPS_MapResult> RLMAPS_MapResultList;
	int RLMAPS_PageSelected = 0;
	void GetResults(std::string keyWord, int IndexPage);
	void GetMapResult(Json::Value maps, int index);
	void GetNumpPages(std::string keyWord);
	void GetMapSize(std::string url);
	int NumPages = 0;
	bool RLMAPS_Searching = false;
	int RLMAPS_NumberOfMapsFound;
	std::shared_ptr<ImageWrapper> RLMAPSLogoImage;



	//Related to download
	void CreateUnzipBatchFile(std::string destinationPath, std::string zipFilePath);
	void CreateJSONLocalWorkshopInfos(std::string jsonFileName, std::string workshopMapPath, std::string mapTitle, std::string mapAuthor, std::string mapDescription, std::string mapPreviewUrl);
	bool IsRetrievingWorkshopFiles = false;
	bool DownloadFailed = false;
	bool UserIsChoosingYESorNO = false;
	bool FolderErrorBool = false;
	bool AcceptTheDownload = false;

	//rocketleaguemaps.us
	void RLMAPS_DownloadWorkshop(std::string folderpath, RLMAPS_MapResult mapResult, RLMAPS_Release release);
	int RLMAPS_Download_Progress;
	int RLMAPS_WorkshopDownload_Progress;
	int RLMAPS_WorkshopDownload_FileSize;
	bool RLMAPS_IsDownloadingWorkshop = false;



	//utils
	std::wstring s2ws(const std::string& s);
	std::string replace(std::string s, char c1, char c2);
	std::string convertToMB(std::string numberToConvert);
	std::vector<std::string> FindAllSubstringInAString(std::string texte, std::string beginString, std::string endString);
	bool Directory_Or_File_Exists(const fs::path& p, fs::file_status s = fs::file_status{});
	void renameFileToUPK(std::filesystem::path filePath);
	std::string UdkInDirectory(std::string dirPath);
	void SaveInCFG();
	std::vector<std::string> GetMapsFolderPathInCfg(std::string cfgFilePath);
	void DownloadPreviewImage(std::string downloadUrl, std::string filePath, int mapResultIndex);
	bool FileIsInDirectoryRecursive(std::string dirPath, std::string filename);
	float DoRatio(float x, float y);
	void CleanHTML(std::string& S);
	void replaceAll(std::string& str, const std::string& from, const std::string& to);
	void eraseAll(std::string& str, const std::string& from);
	std::vector<std::string> GetDrives();


	//ImGui SettingsWindow Functions
	void RenderSettings() override;
	std::string GetPluginName() override;


	//ImGui
	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "Workshop Map Loader & Downloader v" + PluginVersion + " | Made By Vync";

	//Quick search ctrl+f
	void renderQuickSearch();
	bool isQuickSearchDisplayed = false;
	bool QuickSearch_Searching = false;
	char QuickSearch_KeyWordBuf[128];
	std::vector<Map> QuickSearch_GetMapList(std::string keyWord);

	void Render() override;
	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;

	void renderUnderLine(ImColor col_);
	void renderLink(std::string link);
	void renderImageButton(ImTextureID user_texture_id, ImVec2 size, std::function<void()> function);
	void CenterNexIMGUItItem(float itemWidth);
	void AlignRightNexIMGUItItem(float itemWidth, float borderGap);
	std::string LimitTextSize(std::string str, float maxTextSize); //à mettre dans utils

	void renderProgressBar(float value, float maxValue, ImVec2 pos, ImVec2 size, ImColor colorBackground, ImColor colorProgress, const char* label);

	//Popups
	void renderReleases(RLMAPS_MapResult mapResult);
	void renderNewUpdatePopup();
	void renderInfoPopup(const char* popupName, const char* label);
	void renderYesNoPopup(const char* popupName, const char* label, std::function<void()> yesFunc, std::function<void()> noFunc);
	void renderFolderErrorPopup();
	void renderExtractMapFilesPopup(Map curMap);
	void renderAcceptDownload();
	void renderDownloadFailedPopup();
	void renderDownloadTexturesPopup(std::vector<std::string> missingTextureFiles);


	bool AddedMapSccuessfully = false;
	void renderAddMapManuallyPopup();
	void renderFileExplorerToAddMap(char* filefullPathBuff, std::vector<std::string> extensions);
	void renderFileExplorer();


	void renderMaps(Gamepad controller);
	void renderMaps_DisplayMode_0(Map map);
	void renderMaps_DisplayMode_1(Map map, float buttonWidth);

	void RLMAPS_RenderAResult(int i, ImDrawList* drawList, static char mapspath[200]);
	void RLMAPS_renderSearchWorkshopResults(static char mapspath[200]);
	

	ImVec2 CalcRealTextSize(const char* text, float fontSize);
	void renderText(const char* text, float fontSize = 13.f);


	//////////////////////////////////////////////////////////////////////////////////////////////////////  Text Variables

	//Menubar
	std::string SettingsText;
	std::string MultiplayerText;
	std::string LastUpdateText;
	std::string JoinCWGText;
	std::string OpenCPCCText;
	std::string NoMapsCanBeJoinText;
	std::string MapsJoinableText;
	std::string DlTexturesText;
	std::string LanguageText;
	std::string ExtractMethodText;
	std::string WarningText;

	//Controller settings
	std::string ControllerText;
	std::string UseControllerText;
	std::string ControllsText;
	std::string ScrollSensitivityText;
	std::string SensitivityText;
	std::string ControllsLitText[6];

	//1st Tab
	std::string Tab1MapLoaderText;
	std::string Label1Text; //Put the folder's path of the maps, don't forget to add a  /  at the end.
	std::string SelectMapsFolderText;
	std::string RefreshMapsButtonText;
	std::string SavePathText;
	std::string MapsPerLineText;
	//context menu strip
	std::string OpenMapDirText;
	std::string DeleteMapText;

	//LauchMode Popup
	std::string CancelText;

	//Add Map
	std::string AddMapText;
	std::string NameText;
	std::string AuthorText;
	std::string MapFilePathText;
	std::string ImagePathText;
	std::string SelectFileText;
	std::string FieldEmptyText;
	std::string ConfirmLabelText;
	std::string MapAddedSuccessfullyText;
	
	//2nd Tab
	std::string Tab3SearchWorkshopText;
	std::string DownloadButtonText;
	std::string Label3Text;
	std::string SearchButtonText;
	std::string SearchingText;
	std::string WorkshopsFoundText;
	std::string BrowseMapsText;

	//Search Result
	std::string ResultByText;
	std::string ResultSizeText;
	std::string DownloadMapButtonText;

	//Warnings
	std::string DirNotExistText;
	std::string DownloadFailedText; std::string DownloadFailedErrorText;
	std::string FolderErrorText;
	std::string WantToDawnloadText;
	std::string YESButtonText;
	std::string NOButtonText;
	std::string IsDownloadDingWarningText;
	std::string PathSavedText;

	//ExtractMapFiles
	std::string EMFMessageText1;
	std::string EMFMessageText2;
	std::string EMFStillDoesntWorkText;
	//ExtractManually
	std::string EMLabelText;

	//Download Texutures
	std::string DLTLabel1Text;
	std::string DLTLabel2Text;
	std::string DLTMissingFilesText;
	std::string DLTTexturesInstalledText;
	std::string CloseText;
	std::string DontAskText;

	//File Explorer
	std::string NewFolderText;
	std::string ConfirmText;
	std::string SelectText;
};
