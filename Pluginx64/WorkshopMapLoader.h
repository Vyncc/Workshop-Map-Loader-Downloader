#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"


struct Map
{
	std::filesystem::path Folder; //Map folder
	std::filesystem::path UpkFile; //Map(.upk) in the map directory
	std::filesystem::path ZipFile; //Map(.zip) in the map directory
	std::string JsonFile; //.json file found in the map directory
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

struct GameSetting
{
	std::string Name;
	int selectedValue = 0;
	std::vector<std::string> DisplayValuesNames;
	std::vector<std::string> Values;

	std::string GetSelectedValue();
};


//Hey Jerry or Martinn, have a good time reviewing my awful code ;)

class Pluginx64 : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow, public BakkesMod::Plugin::PluginSettingsWindow
{
public:
	virtual void onLoad();
	virtual void onUnload();

	int widthTest;
	int heightTest;

	std::vector<std::vector<RLMAPS_MapResult>> RLMAPS_Pages;
	int RLMAPS_PageSelected;

	bool JoinServerBool;

	
	//Host multiplayer game
	int nbPlayers = 6; //choose number of players in the server
	std::string getMutatorsCommandString();

	/*MIT License

		Copyright (c) 2020 Stanbroek

		Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
		to dealin the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
		and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

		The above copyright notice and this permission notice shall be included in all
		copies or substantial portions of the Software.

		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
		WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	*/
	std::shared_ptr<GameSetting> gameModes =
		std::make_shared<GameSetting>(GameSetting({
		"GameModes", 0,
		{ "Soccar", "Hoops", "Snow Day", "Rumble", "Dropshot", "Heatseeker", "Gridiron" },
		{ "TAGame.GameInfo_Soccar_TA", "TAGame.GameInfo_Basketball_TA", "TAGame.Gameinfo_Hockey_TA", "TAGame.GameInfo_Items_TA", "TAGame.GameInfo_Breakout_TA", "TAGame.GameInfo_GodBall_TA", "TAGame.GameInfo_Football_TA" } }));


	std::vector<std::shared_ptr<GameSetting>> mutators =
	{
		std::make_shared<GameSetting>(GameSetting({
			"Free Play", 0,
			{"Disable Freeplay", "Enable Freeplay"},
			{"", "FreePlay"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Match Duration", 0,
			{"5 Minutes",  "10 Minutes", "20 Minutes", "Unlimited"},
			{"", "10Minutes",  "20Minutes",  "UnlimitedTime"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Max Score", 0,
			{"Default", "1 Goal", "3 Goals", "5 Goals", "7 Goals", "Unlimited"},
			{"", "Max1", "Max3", "Max5", "Max7", "UnlimitedScore"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Overtime", 0,
			{"Unlimited", "+5 Max, First Score", "+5 Max, Random Team"},
			{"", "Overtime5MinutesFirstScore", "Overtime5MinutesRandom"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Max Time Limit", 0,
			{"Default", "11 Minutes"},
			{"", "MaxTime11Minutes"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Series Length", 0,
			{"Unlimited", "3 Games", "5 Games", "7 Games"},
			{"", "3Games", "5Games", "7Games" } })),
		std::make_shared<GameSetting>(GameSetting({
			"Game Speed", 0,
			{"Default", "Slo-mo", "Time Warp"},
			{"", "SloMoGameSpeed", "SloMoDistanceBall"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Ball Max Speed", 0,
			{"Default", "Slow", "Fast", "Super Fast"},
			{"", "SlowBall", "FastBall", "SuperFastBall"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Ball Type", 0,
			{"Default", "Cube", "Puck", "BasketBall", "BeachBall", "Anniversary", "HauntedBall"},
			{"", "Ball_CubeBall", "Ball_Puck", "Ball_BasketBall", "Ball_BeachBall", "Ball_Anniversary", "Ball_Haunted"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Ball Gravity", 0,
			{"Default", "Low", "High", "Super High"},
			{"", "LowGravityBall", "HighGravityBall", "SuperGravityBall"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Ball Physics", 0,
			{ "Default", "Light", "Heavy", "Super Light", "CurveBall", "Beach Ball Curve" },
			{ "", "LightBall", "HeavyBall", "SuperLightBall", "MagnusBall", "MagnusBeachBall" } })),
		std::make_shared<GameSetting>(GameSetting({
			"Ball Size", 0,
			{ "Default", "Small", "Medium", "Large", "Gigantic"},
			{"", "SmallBall", "MediumBall", "BigBall", "GiantBall"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Ball Bounciness", 0,
			{"Default", "Low", "High", "Super High"},
			{"", "LowBounciness", "HighBounciness", "SuperBounciness"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Number Of Ball", 0,
			{"One", "Two", "Four", "Six"},
			{"", "TwoBalls", "FourBalls", "SixBalls"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Boosts Amount", 0,
			{"Default", "No Boost",  "Unlimited", "Recharge (slow)", "Recharge (fast)"},
			{"", "NoBooster", "UnlimitedBooster", "SlowRecharge",    "RapidRecharge"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Rumble", 0,
			{"None", "Default", "Slow", "Civilized", "Destruction Derby", "Spring Loaded", "Spikes Only", "Rugby", "Haunted Ball Beam", "Tactical Rumble"},
			{"", "ItemsMode", "ItemsModeSlow", "ItemsModeBallManipulators", "ItemsModeCarManipulators", "ItemsModeSprings", "ItemsModeSpikes", "ItemsModeRugby", "ItemsModeHauntedBallBeam", "ItemsModeSelection"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Boost Strength", 0,
			{"1X", "1.5X", "2X", "10X"},
			{"", "BoostMultiplier1_5x", "BoostMultiplier2x", "BoostMultiplier10x"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Gravity", 0,
			{"Default", "Low", "High", "Super High",   "Reverse"},
			{"", "LowGravity", "HighGravity", "SuperGravity", "ReverseGravity"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Demolish", 0,
			{"Default", "Disabled", "Friendly Fire", "On Contact", "On Contact (FF)"},
			{"", "NoDemolish", "DemolishAll", "AlwaysDemolishOpposing", "AlwaysDemolish"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Respawn Time", 0,
			{"3 Seconds", "2 Seconds", "1 Second", "Disable Goal Reset"},
			{"", "TwoSecondsRespawn", "OneSecondsRespawn", "DisableGoalDelay"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Bot Loadouts", 0,
			{"Default", "Random"},
			{"", "RandomizedBotLoadouts"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Audio", 0,
			{"Default", "Haunted"},
			{"", "HauntedAudio"} })),
		std::make_shared<GameSetting>(GameSetting({
			"Game Event", 0,
			{"Default", "Haunted", "Rugby"},
			{"", "HauntedGameEventRules", "RugbyGameEventRules"} }))
	};




	//Textures
	std::vector<std::string> WorkshopTexturesFilesList =
	{
		"EditorLandscapeResources.upk", "EditorMaterials.upk", "EditorMeshes.upk", "EditorResources.upk", "Engine_MI_Shaders.upk", "EngineBuildings.upk", "EngineDebugMaterials.upk",
		"EngineMaterials.upk", "EngineResources.upk", "EngineVolumetrics.upk", "MapTemplateIndex.upk", "MapTemplates.upk", "mods.upk", "NodeBuddies.upk"
	};
	bool MapWasAlreadyInCPCC(Map map);
	bool IsDownloading_WorkshopTextures;
	std::vector<std::string> CheckExist_TexturesFiles();
	std::vector<std::filesystem::path> MapsAlreadyInCPCC; //CPCC = CookedPCConsole
	void DownloadWorkshopTextures();
	int Download_Textrures_Progress;
	int DownloadTextrures_ProgressDisplayed;




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
	std::filesystem::path RLCookedPCConsole_Path;


	//Local Maps
	std::vector<Map> MapList;
	std::vector<std::string> GetJSONLocalMapInfos(std::string jsonFilePath);
	void RefreshMapsFunct(std::string mapsfolders);


	//Search Workshop
	//Steam
	std::vector<Steam_MapResult> Steam_MapResultList;
	std::vector<std::string> OtherPagesList;
	void StartSearchRequest(std::string fullurl);
	void CreateJSONSearchWorkshopInfos(std::string jsonFileName, std::string workshopMapPath, std::string mapSize, std::string mapDescription);
	std::vector<std::string> GetJSONSearchMapInfos(std::string jsonFilePath);
	bool STEAM_Searching = false;
	int STEAM_NumberOfMapsFound;
	bool STEAM_browsing;
	float widthBrowseGroup;
	std::shared_ptr<ImageWrapper> SteamLogoImage;

	//rocketleaguemaps.us
	std::vector<RLMAPS_MapResult> RLMAPS_MapResultList;
	void GetResults(std::string searchType, std::string keyWord);
	void GetResultsBrowseMaps(int offset);
	int GetNBOfMapsOnSite();
	int CurrentPage;
	int NBOfMapsOnSite;
	std::vector<int> listBrowsePages();
	bool RLMAPS_Searching;
	int RLMAPS_NumberOfMapsFound;
	bool RLMAPS_browsing;
	std::shared_ptr<ImageWrapper> RLMAPSLogoImage;



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
	int STEAM_WorkshopDownload_Progress;
	int STEAM_WorkshopDownload_FileSize;
	bool IsDownloadingPreview;

	//rocketleaguemaps.us
	void RLMAPS_DownloadWorkshop(std::string folderpath, RLMAPS_MapResult mapResult);
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
	void SaveInCFG(std::string cfgFilePath, std::string mapsfolderpathvariable, std::string languageVariable, std::string unzipMethodVariable, std::string hasSeenNewUpdateAlert);
	std::vector<std::string> GetMapsFolderPathInCfg(std::string cfgFilePath);
	void DownloadPreviewImage(std::string downloadUrl, std::string filePath);


	//ImGui SettingsWindow Functions
	void RenderSettings() override;
	std::string GetPluginName() override;


	//ImGui
	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "Workshop Map Loader & Downloader v1.14 | Made By Vync";

	void Render() override;
	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;

	void renderUnderLine(ImColor col_);
	void CenterNexIMGUItItem(float itemWidth);
	void AlignRightNexIMGUItItem(float itemWidth, float borderGap);

	void renderProgressBar(float value, float maxValue, ImVec2 pos, ImVec2 size, ImColor colorBackground, ImColor colorProgress, const char* label);

	//Popups
	void renderInfoPopup(const char* popupName, const char* label);
	void renderYesNoPopup(const char* popupName, const char* label, std::function<void()> yesFunc, std::function<void()> noFunc);
	void renderFolderErrorPopup();
	void renderLaunchModePopup(Map curMap);
	void renderExtractMapFilesPopup(Map curMap);
	void renderAcceptDownload();
	void renderDownloadFailedPopup();
	void renderDownloadTexturesPopup(std::vector<std::string> missingTextureFiles);
	void renderHostGamePopup(Map curMap);
	void renderJoinServerPopup();


	void renderMaps();

	void renderSortByCombos(std::string mostPopular_url);
	void Steam_RenderAResult(int i, ImDrawList* drawList, static char mapspath[200]);
	void Steam_renderSearchWorkshopResults(static char mapspath[200]);

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

	//LauchMode Popup
	std::string HostText;
	std::string JoinServerText;
	std::string CancelText;
	


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
	std::string BrowseMapsText;

	std::string PeriodText[8];
	std::string SortByText[4];


	//3rd Tab
	std::string Tab3SearchWorkshopText;

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
	std::string PathSavedText;//jsp si jutilise

	//LaunchMode
	std::string LMLabel1Text;
	std::string LMLabel2Text;
	std::string LMLabel3Text;

	//HostGame
	std::string GameSettingsText;
	std::string GameModeText;
	std::string NBOfPlayersText;
	std::string HostGameText;

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
};
