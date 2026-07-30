#pragma once

struct BakkesPluginsMapVersion
{
	std::string m_version;
	std::string m_releaseDate;
	std::string m_fileSize;
	std::string m_downloads;
	std::string m_downloadUrl;
	bool m_isLatest = false;
};

struct BakkesPluginsMapInfo
{
	std::string m_mapID;
	std::string m_sourceUrl;
	std::string m_name;
	std::string m_author;
	std::string m_description;
	std::string m_downloads;
	std::string m_views;
	std::string m_publishedDate;
	std::string m_lastUpdatedDate;
	std::string m_previewUrl;
	std::vector<BakkesPluginsMapVersion> m_versions;
};

enum class InstallMapState
{
	Idle,
	Loading,
	Ready,
	Downloading,
	Installing,
	Error
};

class BakkesPluginsMapInstaller
{
public:
	// Lifecycle

	~BakkesPluginsMapInstaller();
	void Initialize(std::shared_ptr<CVarManagerWrapper> p_cvarManager);
	void Shutdown();
	bool ConsumeRefreshRequested();

public:
	// UI

	void Render();

public:
	// Download callbacks

	bool IsCancelRequested() const { return m_cancelRequested; }
	void SetDownloadProgress(unsigned long long p_downloadedBytes, unsigned long long p_totalBytes);

private:
	// Map retrieval and installation

	bool ParseMapURL(std::string p_url, std::string& p_mapID, std::string& p_normalizedUrl);
	bool ParseMapHTML(const std::string& p_detailsHTML, const std::string& p_versionsHTML, const std::string& p_mapID,
		const std::string& p_sourceUrl, BakkesPluginsMapInfo& p_mapInfo, std::string& p_error);
	void StartLookup();
	void StartInstallation(bool p_replaceExisting);
	void FetchMapInfo(std::string p_mapID, std::string p_sourceUrl);
	void InstallMap(BakkesPluginsMapInfo p_mapInfo, BakkesPluginsMapVersion p_version, bool p_replaceExisting);
	std::filesystem::path GetInstalledMapPath(const BakkesPluginsMapInfo& p_mapInfo);
	std::filesystem::path GetDestinationPath(const BakkesPluginsMapInfo& p_mapInfo);
	void WriteMapMetadata(const std::filesystem::path& p_directory, const BakkesPluginsMapInfo& p_mapInfo, const BakkesPluginsMapVersion& p_version);
	void SetError(std::string p_error);
	void JoinWorker();

private:
	// Data

	std::shared_ptr<CVarManagerWrapper> m_cvarManager;
	char m_urlBuffer[512] = {};
	BakkesPluginsMapInfo m_mapInfo;
	InstallMapState m_state = InstallMapState::Idle;
	std::string m_error;
	std::thread m_worker;
	std::mutex m_mutex;
	std::atomic<bool> m_cancelRequested = false;
	std::atomic<bool> m_refreshRequested = false;
	std::atomic<unsigned long long> m_downloadedBytes = 0;
	std::atomic<unsigned long long> m_totalBytes = 0;
	int m_selectedVersion = 0;
	bool m_installedSuccessfully = false;
	std::filesystem::path m_previewCachePath;
	std::shared_ptr<ImageWrapper> m_previewImage;
	std::atomic<bool> m_previewReady = false;
};
