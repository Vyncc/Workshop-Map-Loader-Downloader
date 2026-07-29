#include "pch.h"
#include "FileExplorer.h"
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

FileExplorer::FileExplorer()
    : m_currentPath("C:/")
    , m_selectedEntry(nullptr)
    , m_sortColumn(SortColumn::Name)
    , m_sortAscending(true)
    , m_previewImage(nullptr)
    , m_currentPreviewPath("")
    , m_needsRefresh(true)
    , m_showNewFolderDialog(false)
    , m_isOpen(false)
{
    memset(m_pathBuffer, 0, sizeof(m_pathBuffer));
    memset(m_newFolderNameBuffer, 0, sizeof(m_newFolderNameBuffer));
    strncpy(m_pathBuffer, "C:/", sizeof(m_pathBuffer));
}

FileExplorer::~FileExplorer()
{
    ClearPreview();
}

void FileExplorer::Open(const std::string& initialPath)
{
    m_currentPath = initialPath;
    strncpy(m_pathBuffer, initialPath.c_str(), sizeof(m_pathBuffer));
    m_needsRefresh = true;
    m_isOpen = true;
    ImGui::OpenPopup("File Explorer");
}

void FileExplorer::Render(bool* p_open)
{
    if (!m_isOpen && p_open && !(*p_open))
        return;

    if (m_isOpen)
    {
        ImGui::OpenPopup("File Explorer");
        m_isOpen = false;
    }

    ImGui::SetNextWindowSize(m_config.windowSize, ImGuiCond_FirstUseEver);

    bool isOpen = true;
    if (ImGui::BeginPopupModal("File Explorer", &isOpen, ImGuiWindowFlags_NoCollapse))
    {
        m_currentPath = m_pathBuffer;

        // Refresh directory if needed
        if (m_needsRefresh)
        {
            LoadDirectory(m_currentPath);
            m_needsRefresh = false;
        }

        RenderPathNavigation();

        ImGui::Spacing();

        float contentHeight = ImGui::GetContentRegionAvail().y - 40.f;

        if (m_config.showPreview)
        {
            float listWidth = ImGui::GetContentRegionAvailWidth() - m_config.previewPanelWidth - 10.f;

            ImGui::BeginChild("##fileListWrapper", ImVec2(listWidth, contentHeight), false);
            RenderFileList();
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("##previewWrapper", ImVec2(m_config.previewPanelWidth, contentHeight), false);
            RenderPreviewPanel();
            ImGui::EndChild();
        }
        else
        {
            ImGui::BeginChild("##fileListWrapper", ImVec2(0, contentHeight), false);
            RenderFileList();
            ImGui::EndChild();
        }

        ImGui::Spacing();
        RenderBottomButtons();

        ImGui::EndPopup();
    }
    else
    {
        m_isOpen = false;
        if (p_open) *p_open = false;
    }

    RenderNewFolderDialog();
}

void FileExplorer::RenderPathNavigation()
{
    ImGui::BeginChild("##pathNavigation", ImVec2(0, 45.f), true, ImGuiWindowFlags_NoScrollbar);

    // Back button
    if (ImGui::Button("<--", ImVec2(40.f, 25.f)))
    {
        NavigateToParent();
    }

    ImGui::SameLine();

    // Path combo box (drive selector)
    std::vector<std::string> drives = GetAvailableDrives();
    float comboWidth = m_config.showNewFolderButton ?
        ImGui::GetContentRegionAvailWidth() - 110.f :
        ImGui::GetContentRegionAvailWidth();

    ImGui::SetNextItemWidth(comboWidth);
    if (ImGui::BeginCombo("##pathCombo", m_pathBuffer))
    {
        for (const auto& drive : drives)
        {
            std::string drivePath = drive + ":/";
            if (ImGui::Selectable(drivePath.c_str()))
            {
                NavigateToPath(drivePath);
            }
        }
        ImGui::EndCombo();
    }

    // New folder button
    if (m_config.showNewFolderButton)
    {
        ImGui::SameLine();
        if (ImGui::Button("New Folder", ImVec2(100.f, 25.f)))
        {
            memset(m_newFolderNameBuffer, 0, sizeof(m_newFolderNameBuffer));
            m_showNewFolderDialog = true;
            ImGui::OpenPopup("New Folder##Dialog");
        }
    }

    ImGui::EndChild();
}

void FileExplorer::RenderFileList()
{
    ImGui::BeginChild("##fileList", ImVec2(0, 0), true);

    // Column headers
    ImGui::Columns(3, "fileColumns", true);
    ImGui::SetColumnWidth(0, 300.f);
    ImGui::SetColumnWidth(1, 120.f);
    ImGui::SetColumnWidth(2, 100.f);

    // Name column
    bool nameClicked = ImGui::Selectable("Name##header",
        m_sortColumn == SortColumn::Name,
        ImGuiSelectableFlags_DontClosePopups);
    if (nameClicked)
    {
        if (m_sortColumn == SortColumn::Name)
            m_sortAscending = !m_sortAscending;
        else
        {
            m_sortColumn = SortColumn::Name;
            m_sortAscending = true;
        }
        SortFileEntries();
    }
    ImGui::NextColumn();

    // Extension column
    bool extClicked = ImGui::Selectable("Extension##header",
        m_sortColumn == SortColumn::Extension,
        ImGuiSelectableFlags_DontClosePopups);
    if (extClicked)
    {
        if (m_sortColumn == SortColumn::Extension)
            m_sortAscending = !m_sortAscending;
        else
        {
            m_sortColumn = SortColumn::Extension;
            m_sortAscending = true;
        }
        SortFileEntries();
    }
    ImGui::NextColumn();

    // Size column
    bool sizeClicked = ImGui::Selectable("Size##header",
        m_sortColumn == SortColumn::Size,
        ImGuiSelectableFlags_DontClosePopups);
    if (sizeClicked)
    {
        if (m_sortColumn == SortColumn::Size)
            m_sortAscending = !m_sortAscending;
        else
        {
            m_sortColumn = SortColumn::Size;
            m_sortAscending = true;
        }
        SortFileEntries();
    }
    ImGui::NextColumn();

    ImGui::Separator();

    // File entries
    for (auto& entry : m_fileEntries)
    {
        if (!PassesFilter(entry))
            continue;

        bool isSelected = (&entry == m_selectedEntry);
        std::string displayName = entry.isDirectory ?
            "[DIR] " + entry.filename :
            entry.filename;

        ImGuiSelectableFlags flags = ImGuiSelectableFlags_SpanAllColumns |
            ImGuiSelectableFlags_AllowDoubleClick;

        if (ImGui::Selectable(displayName.c_str(), isSelected, flags))
        {
            m_selectedEntry = &entry;

            // Handle double-click
            if (ImGui::IsMouseDoubleClicked(0))
            {
                if (entry.isDirectory)
                {
                    NavigateToPath(entry.fullPath.string());
                }
            }
            else if (!entry.isDirectory && m_config.showPreview)
            {
                // File double-clicked - trigger callback
                if (m_onFileSelected)
                    m_onFileSelected(entry.fullPath);

                // Load preview for selected file
                LoadImagePreview(entry.fullPath);
            }
        }

        ImGui::NextColumn();

        // Extension
        ImGui::TextUnformatted(entry.isDirectory ? "" : entry.extension.c_str());
        ImGui::NextColumn();

        // Size
        if (!entry.isDirectory)
            ImGui::TextUnformatted(FormatFileSize(entry.size).c_str());
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::EndChild();
}

void FileExplorer::RenderPreviewPanel()
{
    ImGui::BeginChild("##preview", ImVec2(0, 0), true);

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Preview");
    ImGui::Separator();

    if (m_previewImage && m_previewImage->IsLoadedForImGui() && m_selectedEntry != nullptr)
    {
        auto pTex = m_previewImage->GetImGuiTex();
        if (pTex)
        {
            auto rect = m_previewImage->GetSizeF();

            // Calculate scaled size to fit preview panel
            float maxWidth = m_config.previewPanelWidth - 20.f;
            float maxHeight = ImGui::GetContentRegionAvail().y - 80.f;

            float scale = min(maxWidth / rect.X, maxHeight / rect.Y);
            if (scale > 1.0f) scale = 1.0f;

            ImVec2 displaySize = ImVec2(rect.X * scale, rect.Y * scale);

            // Center the image
            float offsetX = (m_config.previewPanelWidth - displaySize.x) * 0.5f;
            if (offsetX > 0)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

            ImGui::Image(pTex, displaySize);

            ImGui::Separator();
            ImGui::Text("Dimensions: %.0fx%.0f", rect.X, rect.Y);
        }
    }
    else if (m_selectedEntry != nullptr)
    {
        if (!m_selectedEntry->isDirectory && IsImageFile(m_selectedEntry->extension))
        {
            ImGui::TextWrapped("Loading preview...");
        }
        else
        {
            ImGui::TextWrapped("No preview available");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Name:");
        ImGui::TextWrapped("%s", m_selectedEntry->filename.c_str());

        if (!m_selectedEntry->isDirectory)
        {
            ImGui::Spacing();
            ImGui::Text("Type: %s", m_selectedEntry->extension.c_str());
            ImGui::Text("Size: %s", FormatFileSize(m_selectedEntry->size).c_str());
        }
    }
    else
    {
        ImGui::TextWrapped("Select a file to preview");
    }

    ImGui::EndChild();
}

void FileExplorer::RenderBottomButtons()
{
    if (ImGui::Button("Cancel", ImVec2(100.f, 30.f)))
    {
        ClearPreview();
        m_isOpen = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    float offset = ImGui::GetContentRegionAvail().x - 100.f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

    if (ImGui::Button("Select", ImVec2(100.f, 30.f)))
    {
        m_selectedPath = m_pathBuffer;
		m_selectedEntryPath = m_selectedEntry ? m_selectedEntry->fullPath.string() : "";

        if (m_onPathSelected)
            m_onPathSelected(m_selectedPath, m_selectedEntryPath);

        ClearPreview();
        m_isOpen = false;
        ImGui::CloseCurrentPopup();
    }
}

void FileExplorer::RenderNewFolderDialog()
{
    if (ImGui::BeginPopupModal("New Folder##Dialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter folder name:");
        ImGui::SetNextItemWidth(300.f);

        bool enterPressed = ImGui::InputText("##newFolderName",
            m_newFolderNameBuffer,
            sizeof(m_newFolderNameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::Spacing();

        if (ImGui::Button("Confirm", ImVec2(145.f, 30.f)) || enterPressed)
        {
            if (strlen(m_newFolderNameBuffer) > 0)
            {
                CreateNewFolder(m_newFolderNameBuffer);
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(145.f, 30.f)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void FileExplorer::LoadDirectory(const fs::path& path)
{
    m_fileEntries.clear();
    m_selectedEntry = nullptr;
    ClearPreview();

    try
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            FileEntry fileEntry;
            fileEntry.filename = entry.path().filename().string();
            fileEntry.fullPath = entry.path();
            fileEntry.isDirectory = entry.is_directory();

            if (!fileEntry.isDirectory)
            {
                fileEntry.extension = entry.path().extension().string();
                try
                {
                    fileEntry.size = entry.file_size();
                }
                catch (...)
                {
                    fileEntry.size = 0;
                }
            }
            else
            {
                fileEntry.extension = "";
                fileEntry.size = 0;
            }

            m_fileEntries.push_back(fileEntry);
        }

        SortFileEntries();
    }
    catch (const std::exception& ex)
    {
        Log("Error loading directory: " + std::string(ex.what()));
        NavigateToParent();
    }
}

void FileExplorer::NavigateToParent()
{
    fs::path parentPath = m_currentPath.parent_path();
    if (!parentPath.empty())
    {
        NavigateToPath(parentPath.string());
    }
}

void FileExplorer::NavigateToPath(const std::string& path)
{
    strncpy(m_pathBuffer, path.c_str(), sizeof(m_pathBuffer));
    m_currentPath = path;
    m_needsRefresh = true;
}

void FileExplorer::CreateNewFolder(const std::string& folderName)
{
    try
    {
        fs::path newFolderPath = m_currentPath / folderName;
        fs::create_directory(newFolderPath);
        m_needsRefresh = true;
        Log("Created folder: " + newFolderPath.string());
    }
    catch (const std::exception& ex)
    {
        Log("Failed to create folder: " + std::string(ex.what()));
    }
}

std::vector<std::string> FileExplorer::GetAvailableDrives()
{
    std::vector<std::string> drives;

#ifdef _WIN32
    DWORD driveMask = GetLogicalDrives();
    for (int i = 0; i < 26; i++)
    {
        if (driveMask & (1 << i))
        {
            char driveLetter = 'A' + i;
            drives.push_back(std::string(1, driveLetter));
        }
    }
#else
    // Unix-like systems
    drives.push_back("/");
#endif

    return drives;
}

void FileExplorer::SortFileEntries()
{
    std::sort(m_fileEntries.begin(), m_fileEntries.end(),
        [this](const FileEntry& a, const FileEntry& b)
        {
            // Directories always first
            if (a.isDirectory != b.isDirectory)
                return a.isDirectory;

            bool result;
            switch (m_sortColumn)
            {
            case SortColumn::Name:
                result = a.filename < b.filename;
                break;
            case SortColumn::Extension:
                if (a.extension == b.extension)
                    result = a.filename < b.filename;
                else
                    result = a.extension < b.extension;
                break;
            case SortColumn::Size:
                if (a.size == b.size)
                    result = a.filename < b.filename;
                else
                    result = a.size < b.size;
                break;
            default:
                result = a.filename < b.filename;
            }

            return m_sortAscending ? result : !result;
        });
}

void FileExplorer::LoadImagePreview(const fs::path& path)
{
    // Don't reload the same image
    if (m_currentPreviewPath == path.string())
        return;

    ClearPreview();

    if (!IsImageFile(path.extension().string()))
        return;

    try
    {
        _globalGameWrapper->Execute([this, path](GameWrapper* gw) {
            // Create ImageWrapper and load for ImGui
            m_previewImage = std::make_shared<ImageWrapper>(path.string(), false, true);
            m_currentPreviewPath = path.string();
            });

        // Note: ImageWrapper loads asynchronously with the third parameter true
        // The image will appear when it's done loading
    }
    catch (const std::exception& ex)
    {
        Log("Failed to load image preview: " + std::string(ex.what()));
        m_previewImage = nullptr;
        m_currentPreviewPath = "";
    }
}

void FileExplorer::ClearPreview()
{
    m_previewImage = nullptr;
    m_currentPreviewPath = "";
}

bool FileExplorer::IsImageFile(const std::string& extension)
{
    static const std::vector<std::string> imageExtensions =
    {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".dds", ".hdr", ".psd"
    };

    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);

    return std::find(imageExtensions.begin(), imageExtensions.end(), lowerExt)
        != imageExtensions.end();
}

std::string FileExplorer::FormatFileSize(uintmax_t bytes)
{
    const char* units[] = { "B", "KB", "MB", "GB", "TB" };
    int unitIndex = 0;
    double size = (double)bytes;

    while (size >= 1024.0 && unitIndex < 4)
    {
        size /= 1024.0;
        unitIndex++;
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unitIndex]);
    return std::string(buffer);
}

bool FileExplorer::PassesFilter(const FileEntry& entry)
{
    // Show directories if enabled
    if (entry.isDirectory)
        return m_config.showDirectories;

    // Show files if enabled
    if (!m_config.showFiles)
        return false;

    // If no filters, show all files
    if (m_config.fileFilters.empty())
        return true;

    std::string lowerExtension = entry.extension;
    std::transform(lowerExtension.begin(), lowerExtension.end(), lowerExtension.begin(), ::tolower);

    // Check if file extension matches any filter
    for (const auto& filter : m_config.fileFilters)
    {
        std::string lowerFilter = filter;
        std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);

        if (lowerExtension == lowerFilter)
            return true;
    }

    return false;
}

void FileExplorer::Log(const std::string& message)
{
    if (m_logCallback)
        m_logCallback(message);
}
