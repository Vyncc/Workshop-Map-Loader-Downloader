#pragma once

namespace Utils
{
	std::wstring StringToWideString(const std::string& p_value);
	std::string ConvertToMB(const std::string& p_value);
	bool DirectoryOrFileExists(const std::filesystem::path& p_path, std::filesystem::file_status p_status = std::filesystem::file_status{});
	void CreateUnzipBatchFile(const std::filesystem::path& p_bakkesmodPath, const std::string& p_destinationPath, const std::string& p_zipFilePath);
	void RenderProgressBar(float p_value, float p_maxValue, ImVec2 p_position, ImVec2 p_size, ImColor p_backgroundColor, ImColor p_progressColor,
		const char* p_label);
	void RenderYesNoPopup(const char* p_popupName, const char* p_label, const char* p_yesText, const char* p_noText,
		std::function<void()> p_yesFunction, std::function<void()> p_noFunction);
}
