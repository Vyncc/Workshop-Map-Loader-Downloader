#include "pch.h"

namespace Utils
{
	std::wstring StringToWideString(const std::string& p_value)
	{
		const int length = MultiByteToWideChar(CP_ACP, 0, p_value.c_str(), static_cast<int>(p_value.length()) + 1, nullptr, 0);
		std::vector<wchar_t> buffer(length);
		MultiByteToWideChar(CP_ACP, 0, p_value.c_str(), static_cast<int>(p_value.length()) + 1, buffer.data(), length);
		return std::wstring(buffer.data());
	}

	std::string ConvertToMB(const std::string& p_value)
	{
		unsigned long long bytes = 0;
		try
		{
			bytes = std::stoull(p_value);
		}
		catch(const std::exception&)
		{
			return "0 Bytes";
		}

		const char* units[] = { "Bytes", "KB", "MB", "GB", "TB" };
		double value = static_cast<double>(bytes);
		int unitIndex = 0;
		while(value >= 1024.0 && unitIndex < 4)
		{
			value /= 1024.0;
			unitIndex++;
		}

		std::ostringstream output;
		if(unitIndex == 0)
			output << bytes;
		else
			output << std::fixed << std::setprecision(value >= 100.0 ? 0 : 1) << value;
		output << " " << units[unitIndex];
		return output.str();
	}

	bool DirectoryOrFileExists(const std::filesystem::path& p_path, std::filesystem::file_status p_status)
	{
		std::error_code error;
		if(p_status.type() == std::filesystem::file_type::none)
			p_status = std::filesystem::status(p_path, error);

		return !error && std::filesystem::exists(p_status);
	}

	void CreateUnzipBatchFile(const std::filesystem::path& p_bakkesmodPath, const std::string& p_destinationPath, const std::string& p_zipFilePath)
	{
		const std::filesystem::path batchPath = p_bakkesmodPath / "data" / "WorkshopMapLoader" / "temp" / "unzip.bat";
		std::filesystem::create_directories(batchPath.parent_path());
		std::ofstream batchFile(batchPath);
		std::string destinationPath = p_destinationPath;
		std::string zipFilePath = p_zipFilePath;
		std::replace(destinationPath.begin(), destinationPath.end(), '/', '\\');
		std::replace(zipFilePath.begin(), zipFilePath.end(), '/', '\\');

		batchFile << "@echo off\nsetlocal\ncd /d %~dp0\n";
		batchFile << "Call :UnZipFile \"" + destinationPath + "\" \"" + zipFilePath + "\"\nexit /b\n\n";
		batchFile << ":UnZipFile <ExtractTo> <newzipfile>\n";
		batchFile << "set vbs=\"%temp%\\_.vbs\"\nif exist %vbs% del /f /q %vbs%\n";
		batchFile << ">%vbs% echo Set fso = CreateObject(\"Scripting.FileSystemObject\")\n";
		batchFile << ">>%vbs% echo If NOT fso.FolderExists(%1) Then\n>>%vbs% echo fso.CreateFolder(%1)\n>>%vbs% echo End If\n";
		batchFile << ">>%vbs% echo set objShell = CreateObject(\"Shell.Application\")\n";
		batchFile << ">>%vbs% echo set FilesInZip=objShell.NameSpace(%2).items\n";
		batchFile << ">>%vbs% echo objShell.NameSpace(%1).CopyHere(FilesInZip)\n";
		batchFile << ">>%vbs% echo Set fso = Nothing\n>>%vbs% echo Set objShell = Nothing\n";
		batchFile << "cscript //nologo %vbs%\nif exist %vbs% del /f /q %vbs%\n";
		batchFile.close();
		system(batchPath.string().c_str());
	}

	void RenderProgressBar(float p_value, float p_maxValue, ImVec2 p_position, ImVec2 p_size, ImColor p_backgroundColor, ImColor p_progressColor,
		const char* p_label)
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const float maximum = p_maxValue > 0.f ? p_maxValue : 1.f;
		const float percent = std::clamp(p_value / maximum, 0.f, 1.f);
		ImGui::BeginChild("##ProgessBar", p_size);
		drawList->AddRectFilled(p_position, ImVec2(p_position.x + p_size.x, p_position.y + p_size.y), p_backgroundColor, 15.f, 15.f);
		if(p_value > 0.f)
			drawList->AddRectFilled(p_position, ImVec2(p_position.x + percent * p_size.x, p_position.y + p_size.y), p_progressColor, 15.f, 15.f);
		const float textWidth = ImGui::CalcTextSize(p_label).x;
		drawList->AddText(ImVec2(p_position.x + (p_size.x - textWidth) * 0.5f, p_position.y + 5.f), ImColor(255, 255, 255, 255), p_label);
		ImGui::EndChild();
	}

	void RenderYesNoPopup(const char* p_popupName, const char* p_label, const char* p_yesText, const char* p_noText,
		std::function<void()> p_yesFunction, std::function<void()> p_noFunction)
	{
		if(!ImGui::BeginPopupModal(p_popupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		ImGui::Text("%s", p_label);
		ImGui::NewLine();
		if(ImGui::Button(p_yesText, ImVec2(100.f, 25.f)))
			p_yesFunction();
		ImGui::SameLine();
		if(ImGui::Button(p_noText, ImVec2(100.f, 25.f)))
			p_noFunction();
		ImGui::EndPopup();
	}
}
