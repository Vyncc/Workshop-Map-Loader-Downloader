#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <memory>
#include "IMGUI/imgui.h"
#include "bakkesmod/wrappers/ImageWrapper.h"

namespace fs = std::filesystem;

class FileExplorer
{
public:
    enum class SortColumn { Name, Extension, Size };

    struct FileEntry
    {
        std::string filename;
        std::string extension;
        fs::path fullPath;
        bool isDirectory;
        uintmax_t size;

        FileEntry() : size(0), isDirectory(false) {}
    };

    struct Config
    {
        ImVec2 windowSize = ImVec2(950.f, 600.f);
        float previewPanelWidth = 280.f;
        bool showPreview = true;
        bool showNewFolderButton = true;
        bool showFiles = true;
        bool showDirectories = true;
        std::vector<std::string> fileFilters; // e.g., {".png", ".jpg"}

        Config() {}
    };

    // Callback types
    using OnPathSelectedCallback = std::function<void(const std::string&, const std::string&)>;
    using OnFileSelectedCallback = std::function<void(const fs::path&)>;
    using LogCallback = std::function<void(const std::string&)>;

    // Constructor / Destructor
    FileExplorer();
    ~FileExplorer();

    // Main render function
    void Render(bool* p_open = nullptr);

    // Open the file explorer modal
    void Open(const std::string& initialPath = "C:/");

    // Configuration
    void SetConfig(const Config& config) { m_config = config; }
    Config& GetConfig() { return m_config; }

    // Callbacks
    void SetOnPathSelected(OnPathSelectedCallback callback) { m_onPathSelected = callback; }
    void SetOnFileSelected(OnFileSelectedCallback callback) { m_onFileSelected = callback; }
    void SetLogCallback(LogCallback callback) { m_logCallback = callback; }

    // Getters
    const std::string& GetSelectedPath() const { return m_selectedPath; }
    const FileEntry* GetSelectedFile() const { return m_selectedEntry; }

private:
    // Core functionality
    void RenderPathNavigation();
    void RenderFileList();
    void RenderPreviewPanel();
    void RenderBottomButtons();
    void RenderNewFolderDialog();

    // Directory operations
    void LoadDirectory(const fs::path& path);
    void NavigateToParent();
    void NavigateToPath(const std::string& path);
    void CreateNewFolder(const std::string& folderName);
    std::vector<std::string> GetAvailableDrives();

    // Sorting
    void SortFileEntries();

    // Preview
    void LoadImagePreview(const fs::path& path);
    void ClearPreview();
    bool IsImageFile(const std::string& extension);

    // Utility
    std::string FormatFileSize(uintmax_t bytes);
    bool PassesFilter(const FileEntry& entry);
    void Log(const std::string& message);

private:
    // Configuration
    Config m_config;

    // State
    fs::path m_currentPath;
    std::string m_selectedPath;
    std::string m_selectedEntryPath;
    std::vector<FileEntry> m_fileEntries;
    FileEntry* m_selectedEntry;

    // Sorting
    SortColumn m_sortColumn;
    bool m_sortAscending;

    // Preview - Using ImageWrapper
    std::shared_ptr<ImageWrapper> m_previewImage;
    std::string m_currentPreviewPath;

    // UI State
    char m_pathBuffer[512];
    char m_newFolderNameBuffer[256];
    bool m_needsRefresh;
    bool m_showNewFolderDialog;
    bool m_isOpen;

    // Callbacks
    OnPathSelectedCallback m_onPathSelected;
    OnFileSelectedCallback m_onFileSelected;
    LogCallback m_logCallback;
};