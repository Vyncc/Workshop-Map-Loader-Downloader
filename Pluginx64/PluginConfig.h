#pragma once

struct PluginConfig
{
	std::string BakkesmodPath;
	std::string MapsFolderPath;
	char MapsFolderPathBuffer[200] = {};
	bool Language = false;
	std::string UnzipMethod = "Bat";
	bool HasSeeNewUpdateAlert = false;
	int DontAsk = 0;
	int MapsDisplayMode = 0;
	int NbTilesPerLine = 6;
	bool EnableAntiFreezeFix = false;
};

extern PluginConfig g_pluginConfig;
