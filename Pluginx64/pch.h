#pragma once

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "cpr.lib")
#pragma comment(lib, "jsoncpp.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "crypt32")
#pragma comment (lib, "Wldap32.lib")
#pragma comment( lib, "pluginsdk.lib" )
#include <urlmon.h>                 //Needed for the URLDownloadToFile() function
#pragma comment(lib, "urlmon.lib")  //Needed for the URLDownloadToFile() function

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <cpr/cpr.h>
#include <curl/curl.h>
#include <json/json.h>

namespace fs = std::filesystem;


#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <fstream>
#include <iostream>
#include <cstddef>
#include <thread>
#include <filesystem>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <optional>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>

#include "IMGUI/imgui.h"
#include "PluginConfig.h"
#include "PluginTexts.h"
#include "Utils.h"

extern std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
extern std::shared_ptr<GameWrapper> _globalGameWrapper;

inline std::string PluginVersion = "1.16";