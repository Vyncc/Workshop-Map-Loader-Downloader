#include "pch.h"
#include "BakkesPluginsMapInstaller.h"

#include "IMGUI/imgui_internal.h"

namespace
{
	struct InstallMapDownloadContext
	{
		BakkesPluginsMapInstaller* installer = nullptr;
		std::ofstream* output = nullptr;
	};

	std::string TrimString(const std::string& p_value)
	{
		size_t begin = 0;
		while (begin < p_value.size() && std::isspace(static_cast<unsigned char>(p_value[begin])))
			begin++;

		size_t end = p_value.size();
		while (end > begin && std::isspace(static_cast<unsigned char>(p_value[end - 1])))
			end--;

		return p_value.substr(begin, end - begin);
	}

	std::string ToLowerString(std::string p_value)
	{
		std::transform(p_value.begin(), p_value.end(), p_value.begin(), [](unsigned char p_character) {
			return static_cast<char>(std::tolower(p_character));
			});
		return p_value;
	}

	void AppendUTF8(std::string& p_output, unsigned int p_codePoint)
	{
		if (p_codePoint <= 0x7F)
			p_output += static_cast<char>(p_codePoint);
		else if (p_codePoint <= 0x7FF)
		{
			p_output += static_cast<char>(0xC0 | (p_codePoint >> 6));
			p_output += static_cast<char>(0x80 | (p_codePoint & 0x3F));
		}
		else if (p_codePoint <= 0xFFFF)
		{
			p_output += static_cast<char>(0xE0 | (p_codePoint >> 12));
			p_output += static_cast<char>(0x80 | ((p_codePoint >> 6) & 0x3F));
			p_output += static_cast<char>(0x80 | (p_codePoint & 0x3F));
		}
		else if (p_codePoint <= 0x10FFFF)
		{
			p_output += static_cast<char>(0xF0 | (p_codePoint >> 18));
			p_output += static_cast<char>(0x80 | ((p_codePoint >> 12) & 0x3F));
			p_output += static_cast<char>(0x80 | ((p_codePoint >> 6) & 0x3F));
			p_output += static_cast<char>(0x80 | (p_codePoint & 0x3F));
		}
	}

	std::string DecodeHTMLEntities(const std::string& p_value)
	{
		std::string output;
		for (size_t index = 0; index < p_value.size(); index++)
		{
			if (p_value[index] != '&')
			{
				output += p_value[index];
				continue;
			}

			size_t semicolon = p_value.find(';', index + 1);
			if (semicolon == std::string::npos || semicolon - index > 12)
			{
				output += p_value[index];
				continue;
			}

			std::string entity = p_value.substr(index + 1, semicolon - index - 1);
			if (entity == "amp")
				output += '&';
			else if (entity == "lt")
				output += '<';
			else if (entity == "gt")
				output += '>';
			else if (entity == "quot")
				output += '"';
			else if (entity == "apos" || entity == "#39")
				output += '\'';
			else if (entity == "nbsp")
				output += ' ';
			else if (!entity.empty() && entity[0] == '#')
			{
				try
				{
					unsigned int codePoint = entity.size() > 2 && (entity[1] == 'x' || entity[1] == 'X')
						? static_cast<unsigned int>(std::stoul(entity.substr(2), nullptr, 16))
						: static_cast<unsigned int>(std::stoul(entity.substr(1)));
					AppendUTF8(output, codePoint);
				}
				catch (...)
				{
					output += '&' + entity + ';';
				}
			}
			else
			{
				output += '&' + entity + ';';
			}

			index = semicolon;
		}
		return output;
	}

	std::string CleanHTMLText(const std::string& p_html)
	{
		std::string text;
		for (size_t index = 0; index < p_html.size();)
		{
			if (p_html.compare(index, 4, "<!--") == 0)
			{
				size_t commentEnd = p_html.find("-->", index + 4);
				index = commentEnd == std::string::npos ? p_html.size() : commentEnd + 3;
				continue;
			}

			if (p_html[index] != '<')
			{
				text += p_html[index++];
				continue;
			}

			size_t tagEnd = p_html.find('>', index + 1);
			if (tagEnd == std::string::npos)
				break;

			std::string tag = ToLowerString(p_html.substr(index + 1, tagEnd - index - 1));
			if (tag.rfind("br", 0) == 0 || tag.rfind("/p", 0) == 0 || tag.rfind("/div", 0) == 0 || tag.rfind("/li", 0) == 0 ||
				tag.rfind("/h", 0) == 0)
				text += '\n';
			else if (tag.rfind("li", 0) == 0)
				text += "- ";

			index = tagEnd + 1;
		}

		text = DecodeHTMLEntities(text);
		std::stringstream input(text);
		std::string line;
		std::string cleaned;
		bool previousLineEmpty = true;
		while (std::getline(input, line))
		{
			std::string normalized;
			bool previousSpace = false;
			for (char character : TrimString(line))
			{
				bool isSpace = std::isspace(static_cast<unsigned char>(character)) != 0;
				if (!isSpace || !previousSpace)
					normalized += isSpace ? ' ' : character;
				previousSpace = isSpace;
			}

			if (normalized.empty())
			{
				if (!previousLineEmpty)
					cleaned += '\n';
				previousLineEmpty = true;
				continue;
			}

			if (!cleaned.empty() && cleaned.back() != '\n')
				cleaned += '\n';
			cleaned += normalized;
			previousLineEmpty = false;
		}

		return TrimString(cleaned);
	}

	std::string ExtractAttribute(const std::string& p_tag, const std::string& p_attribute)
	{
		std::string marker = p_attribute + "=\"";
		size_t begin = p_tag.find(marker);
		if (begin == std::string::npos)
			return "";

		begin += marker.size();
		size_t end = p_tag.find('"', begin);
		if (end == std::string::npos)
			return "";

		return DecodeHTMLEntities(p_tag.substr(begin, end - begin));
	}

	std::string ExtractMetaContent(const std::string& p_html, const std::string& p_attribute, const std::string& p_value)
	{
		size_t position = 0;
		while ((position = p_html.find("<meta ", position)) != std::string::npos)
		{
			size_t end = p_html.find('>', position);
			if (end == std::string::npos)
				return "";

			std::string tag = p_html.substr(position, end - position + 1);
			if (ExtractAttribute(tag, p_attribute) == p_value)
				return ExtractAttribute(tag, "content");

			position = end + 1;
		}
		return "";
	}

	std::string ExtractDefinitionValue(const std::string& p_html, const std::string& p_label)
	{
		std::string marker = ">" + p_label + "</dt>";
		size_t position = p_html.find(marker);
		if (position == std::string::npos)
			return "";

		size_t valueBegin = p_html.find("<dd", position + marker.size());
		if (valueBegin == std::string::npos)
			return "";

		valueBegin = p_html.find('>', valueBegin);
		size_t valueEnd = p_html.find("</dd>", valueBegin);
		if (valueBegin == std::string::npos || valueEnd == std::string::npos)
			return "";

		return CleanHTMLText(p_html.substr(valueBegin + 1, valueEnd - valueBegin - 1));
	}

	size_t FindMatchingDivEnd(const std::string& p_html, size_t p_openingDiv)
	{
		int depth = 0;
		size_t position = p_openingDiv;
		while (position < p_html.size())
		{
			size_t nextOpen = p_html.find("<div", position);
			size_t nextClose = p_html.find("</div>", position);
			if (nextClose == std::string::npos)
				return std::string::npos;

			if (nextOpen != std::string::npos && nextOpen < nextClose)
			{
				depth++;
				position = nextOpen + 4;
			}
			else
			{
				depth--;
				if (depth == 0)
					return nextClose;
				position = nextClose + 6;
			}
		}
		return std::string::npos;
	}

	std::string ExtractMapDescription(const std::string& p_html)
	{
		size_t detailsHeading = p_html.find(">Map Details</h2>");
		if (detailsHeading == std::string::npos)
			return ExtractMetaContent(p_html, "name", "description");

		size_t markdownClass = p_html.find("class=\"markdown-body\"", detailsHeading);
		if (markdownClass == std::string::npos)
			return ExtractMetaContent(p_html, "name", "description");

		size_t openingDiv = p_html.rfind("<div", markdownClass);
		size_t contentBegin = p_html.find('>', markdownClass);
		size_t contentEnd = FindMatchingDivEnd(p_html, openingDiv);
		if (openingDiv == std::string::npos || contentBegin == std::string::npos || contentEnd == std::string::npos)
			return ExtractMetaContent(p_html, "name", "description");

		return CleanHTMLText(p_html.substr(contentBegin + 1, contentEnd - contentBegin - 1));
	}

	std::vector<std::string> ExtractSpanValues(const std::string& p_html, const std::string& p_class)
	{
		std::vector<std::string> values;
		std::string classMarker = "class=\"" + p_class + "\"";
		size_t position = 0;
		while ((position = p_html.find("<span", position)) != std::string::npos)
		{
			size_t tagEnd = p_html.find('>', position);
			if (tagEnd == std::string::npos)
				break;

			std::string tag = p_html.substr(position, tagEnd - position + 1);
			if (tag.find(classMarker) != std::string::npos)
			{
				size_t spanEnd = p_html.find("</span>", tagEnd);
				if (spanEnd == std::string::npos)
					break;

				values.push_back(CleanHTMLText(p_html.substr(tagEnd + 1, spanEnd - tagEnd - 1)));
				position = spanEnd + 7;
			}
			else
			{
				position = tagEnd + 1;
			}
		}
		return values;
	}

	std::string RemoveSuffix(std::string p_value, const std::string& p_suffix)
	{
		size_t suffixPosition = ToLowerString(p_value).rfind(ToLowerString(p_suffix));
		if (suffixPosition != std::string::npos)
			p_value.erase(suffixPosition);
		return TrimString(p_value);
	}

	bool IsValidBakkesPluginsDownloadURL(const std::string& p_url)
	{
		std::string lowerURL = ToLowerString(p_url);
		if (lowerURL.rfind("https://cdn.bakkesplugins.com/", 0) != 0)
			return false;

		size_t query = lowerURL.find_first_of("?#");
		std::string path = lowerURL.substr(0, query);
		return path.size() >= 4 && path.substr(path.size() - 4) == ".zip";
	}

	std::string GetBakkesPluginsImageExtension(const std::string& p_url)
	{
		std::string lowerURL = ToLowerString(p_url);
		if (lowerURL.rfind("https://cdn.bakkesplugins.com/", 0) != 0)
			return "";

		size_t query = lowerURL.find_first_of("?#");
		std::string path = lowerURL.substr(0, query);
		if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")
			return ".png";
		if (path.size() >= 5 && path.substr(path.size() - 5) == ".jfif")
			return ".jfif";
		if (path.size() >= 5 && path.substr(path.size() - 5) == ".jpeg")
			return ".jpg";
		if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg")
			return ".jpg";
		return "";
	}

	size_t CurlWriteString(char* p_data, size_t p_size, size_t p_count, void* p_userData)
	{
		size_t byteCount = p_size * p_count;
		std::string* output = static_cast<std::string*>(p_userData);
		output->append(p_data, byteCount);
		return byteCount;
	}

	size_t CurlWriteFile(char* p_data, size_t p_size, size_t p_count, void* p_userData)
	{
		size_t byteCount = p_size * p_count;
		InstallMapDownloadContext* context = static_cast<InstallMapDownloadContext*>(p_userData);
		context->output->write(p_data, byteCount);
		return context->output->good() ? byteCount : 0;
	}

	int CurlInstallMapProgress(void* p_userData, curl_off_t p_total, curl_off_t p_downloaded, curl_off_t, curl_off_t)
	{
		InstallMapDownloadContext* context = static_cast<InstallMapDownloadContext*>(p_userData);
		if (context->installer->IsCancelRequested())
			return 1;

		context->installer->SetDownloadProgress(p_downloaded > 0 ? static_cast<unsigned long long>(p_downloaded) : 0,
			p_total > 0 ? static_cast<unsigned long long>(p_total) : 0);
		return 0;
	}

	int CurlCancelProgress(void* p_userData, curl_off_t, curl_off_t, curl_off_t, curl_off_t)
	{
		std::atomic<bool>* cancelRequested = static_cast<std::atomic<bool>*>(p_userData);
		return *cancelRequested ? 1 : 0;
	}

	void ConfigureCurlRequest(CURL* p_curl, const std::string& p_url, char* p_errorBuffer)
	{
		const std::string userAgent = "WorkshopMapLoader/" + PluginVersion;
		curl_easy_setopt(p_curl, CURLOPT_URL, p_url.c_str());
		curl_easy_setopt(p_curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(p_curl, CURLOPT_MAXREDIRS, 5L);
		curl_easy_setopt(p_curl, CURLOPT_CONNECTTIMEOUT, 10L);
		curl_easy_setopt(p_curl, CURLOPT_TIMEOUT, 60L);
		curl_easy_setopt(p_curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(p_curl, CURLOPT_ACCEPT_ENCODING, "");
		curl_easy_setopt(p_curl, CURLOPT_USERAGENT, userAgent.c_str());
		curl_easy_setopt(p_curl, CURLOPT_ERRORBUFFER, p_errorBuffer);
	}

	bool DownloadString(const std::string& p_url, std::string& p_output, std::atomic<bool>& p_cancelRequested, std::string& p_error)
	{
		CURL* curl = curl_easy_init();
		if (!curl)
		{
			p_error = "Unable to initialize the network request.";
			return false;
		}

		char errorBuffer[CURL_ERROR_SIZE] = {};
		ConfigureCurlRequest(curl, p_url, errorBuffer);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteString);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &p_output);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CurlCancelProgress);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &p_cancelRequested);

		CURLcode result = p_cancelRequested ? CURLE_ABORTED_BY_CALLBACK : curl_easy_perform(curl);
		long responseCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		curl_easy_cleanup(curl);

		if (p_cancelRequested)
		{
			p_error = "Request cancelled.";
			return false;
		}

		if (result != CURLE_OK)
		{
			p_error = errorBuffer[0] ? errorBuffer : curl_easy_strerror(result);
			return false;
		}

		if (responseCode < 200 || responseCode >= 300)
		{
			p_error = "BakkesPlugins returned HTTP " + std::to_string(responseCode) + ".";
			return false;
		}
		return true;
	}

	bool DownloadPreviewFile(const std::string& p_url, const std::filesystem::path& p_destination, std::atomic<bool>& p_cancelRequested)
	{
		std::ofstream output(p_destination, std::ios::binary);
		if(!output)
			return false;

		CURL* curl = curl_easy_init();
		if(!curl)
			return false;

		char errorBuffer[CURL_ERROR_SIZE] = {};
		ConfigureCurlRequest(curl, p_url, errorBuffer);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteFile);
		InstallMapDownloadContext context{ nullptr, &output };
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &context);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CurlCancelProgress);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &p_cancelRequested);

		const CURLcode result = curl_easy_perform(curl);
		long responseCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		curl_easy_cleanup(curl);
		output.close();

		if(result != CURLE_OK || responseCode < 200 || responseCode >= 300 || !std::filesystem::exists(p_destination))
			return false;

		return std::filesystem::file_size(p_destination) > 0;
	}

	bool DownloadFile(const std::string& p_url, const std::filesystem::path& p_destination, BakkesPluginsMapInstaller* p_installer, std::string& p_error)
	{
		std::ofstream output(p_destination, std::ios::binary);
		if (!output)
		{
			p_error = "Unable to create the downloaded file.";
			return false;
		}

		CURL* curl = curl_easy_init();
		if (!curl)
		{
			p_error = "Unable to initialize the download.";
			return false;
		}

		char errorBuffer[CURL_ERROR_SIZE] = {};
		InstallMapDownloadContext context{ p_installer, &output };
		ConfigureCurlRequest(curl, p_url, errorBuffer);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteFile);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &context);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CurlInstallMapProgress);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &context);

		CURLcode result = curl_easy_perform(curl);
		long responseCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		curl_easy_cleanup(curl);
		output.close();

		if (p_installer->IsCancelRequested())
		{
			p_error = "Download cancelled.";
			return false;
		}

		if (result != CURLE_OK)
		{
			p_error = errorBuffer[0] ? errorBuffer : curl_easy_strerror(result);
			return false;
		}

		if (responseCode < 200 || responseCode >= 300)
		{
			p_error = "The download server returned HTTP " + std::to_string(responseCode) + ".";
			return false;
		}

		if (!std::filesystem::exists(p_destination) || std::filesystem::file_size(p_destination) == 0)
		{
			p_error = "The downloaded file is empty.";
			return false;
		}
		return true;
	}

	std::string SanitizeFileName(const std::string& p_value)
	{
		std::string result;
		const std::string forbidden = "<>:\"/\\|?*";
		for (unsigned char character : p_value)
		{
			if (character < 32 || forbidden.find(static_cast<char>(character)) != std::string::npos)
				result += '_';
			else
				result += static_cast<char>(character);
		}

		while (!result.empty() && (result.back() == ' ' || result.back() == '.'))
			result.pop_back();
		while (!result.empty() && result.front() == ' ')
			result.erase(result.begin());

		return result.empty() ? "BakkesPluginsMap" : result;
	}

	std::string EscapePowerShellLiteral(std::string p_value)
	{
		size_t position = 0;
		while ((position = p_value.find('\'', position)) != std::string::npos)
		{
			p_value.insert(position, 1, '\'');
			position += 2;
		}
		return p_value;
	}

	std::optional<std::filesystem::path> FindMapFile(const std::filesystem::path& p_directory)
	{
		for (const auto& entry : std::filesystem::directory_iterator(p_directory))
		{
			if (!entry.is_regular_file())
				continue;

			std::string extension = ToLowerString(entry.path().extension().string());
			if (extension == ".udk" || extension == ".upk")
				return entry.path();
		}
		return std::nullopt;
	}

	void NormalizeSingleWrapperDirectory(const std::filesystem::path& p_directory, const std::filesystem::path& p_zipPath)
	{
		std::vector<std::filesystem::path> extractedEntries;
		for (const auto& entry : std::filesystem::directory_iterator(p_directory))
		{
			if (entry.path() != p_zipPath)
				extractedEntries.push_back(entry.path());
		}

		if (extractedEntries.size() != 1 || !std::filesystem::is_directory(extractedEntries[0]))
			return;

		std::filesystem::path wrapper = extractedEntries[0];
		for (const auto& entry : std::filesystem::directory_iterator(wrapper))
			std::filesystem::rename(entry.path(), p_directory / entry.path().filename());
		std::filesystem::remove(wrapper);
	}
}







BakkesPluginsMapInstaller::~BakkesPluginsMapInstaller()
{
	Shutdown();
}

void BakkesPluginsMapInstaller::Initialize(std::shared_ptr<CVarManagerWrapper> p_cvarManager)
{
	m_cvarManager = p_cvarManager;
}

void BakkesPluginsMapInstaller::Shutdown()
{
	m_cancelRequested = true;
	JoinWorker();
}

bool BakkesPluginsMapInstaller::ConsumeRefreshRequested()
{
	return m_refreshRequested.exchange(false);
}

void BakkesPluginsMapInstaller::Render()
{
	BakkesPluginsMapInfo mapInfo;
	InstallMapState state;
	std::string error;
	int selectedVersion;
	bool installedSuccessfully;
	std::filesystem::path previewCachePath;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		mapInfo = m_mapInfo;
		state = m_state;
		error = m_error;
		selectedVersion = m_selectedVersion;
		installedSuccessfully = m_installedSuccessfully;
		previewCachePath = m_previewCachePath;
	}

	static std::string bakkesPluginsMapsURL = "https://bakkesplugins.com/maps";
	if(m_previewReady.exchange(false) && !previewCachePath.empty())
		m_previewImage = std::make_shared<ImageWrapper>(previewCachePath.string(), false, true);

	if(m_previewImage && m_previewImage->GetImGuiTex() && !previewCachePath.empty())
	{
		std::error_code removeError;
		std::filesystem::remove(previewCachePath, removeError);
		if(removeError)
			m_cvarManager->log("Unable to delete the temporary BakkesPlugins preview: " + removeError.message());
		else
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if(m_previewCachePath == previewCachePath)
				m_previewCachePath.clear();
		}
	}

	if (ImGui::Button(g_pluginTexts.InstallMapBrowse.c_str()))
	{
		std::wstring w_LINK = Utils::StringToWideString(bakkesPluginsMapsURL);
		LPCWSTR L_LINK = w_LINK.c_str();
		ShellExecute(0, 0, L_LINK, 0, 0, SW_SHOW); //open link in web browser
	}

	const bool isBusy = state == InstallMapState::Loading || state == InstallMapState::Downloading || state == InstallMapState::Installing;

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.f);
	ImGui::Text("%s", g_pluginTexts.InstallMapURL.c_str());

	ImGui::SameLine();

	ImGui::Text(" (?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%s", g_pluginTexts.InstallMapURLHelp.c_str());
		ImGui::EndTooltip();
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(-170.f);
	bool pressedEnter = ImGui::InputText("##BakkesPluginsMapURL", m_urlBuffer, IM_ARRAYSIZE(m_urlBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::SameLine();

	if (isBusy)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	bool retrieveClicked = ImGui::Button(g_pluginTexts.InstallMapRetrieve.c_str(), ImVec2(160.f, 0.f));
	if (isBusy)
	{
		ImGui::PopStyleVar();
		ImGui::PopItemFlag();
	}

	if ((pressedEnter || retrieveClicked) && !isBusy)
		StartLookup();

	ImGui::Spacing();
	if (state == InstallMapState::Loading)
	{
		ImGui::Text("%s", g_pluginTexts.InstallMapLoading.c_str());
		return;
	}

	if (state == InstallMapState::Error)
	{
		ImGui::TextColored(ImVec4(1.f, 0.25f, 0.25f, 1.f), "%s", g_pluginTexts.InstallMapError.c_str());
		ImGui::TextWrapped("%s", error.c_str());
		return;
	}

	if (state == InstallMapState::Idle || mapInfo.m_versions.empty())
	{
		ImGui::TextDisabled("%s", g_pluginTexts.InstallMapEmpty.c_str());
		return;
	}

	ImGui::Separator();
	const float columnSpacing = 10.f;
	const float availableWidth = ImGui::GetContentRegionAvailWidth();
	const float leftColumnWidth = std::clamp(availableWidth * 0.34f, 300.f, 460.f);
	const float contentHeight = ImGui::GetContentRegionAvail().y;

	ImGui::BeginChild("##InstallMapLeftColumn", ImVec2(leftColumnWidth, contentHeight), false);
	const float imageWidth = ImGui::GetContentRegionAvailWidth();
	const float imageHeight = imageWidth * 0.5625f;
	ImGui::BeginChild("##InstallMapPreview", ImVec2(imageWidth, imageHeight), true);
	if(m_previewImage && m_previewImage->GetImGuiTex())
	{
		const ImVec2 availableImageSize = ImGui::GetContentRegionAvail();
		ImGui::Image(m_previewImage->GetImGuiTex(), availableImageSize);
	}
	else
	{
		const ImVec2 textSize = ImGui::CalcTextSize(g_pluginTexts.InstallMapPreview.c_str());
		ImGui::SetCursorPos(ImVec2((imageWidth - textSize.x) * 0.5f, (imageHeight - textSize.y) * 0.5f));
		ImGui::TextDisabled("%s", g_pluginTexts.InstallMapPreview.c_str());
	}
	ImGui::EndChild();

	ImGui::Spacing();
	const bool mapsDirectoryValid = Utils::DirectoryOrFileExists(fs::path(g_pluginConfig.MapsFolderPathBuffer));
	float installControlsHeight = 32.f;
	if(state == InstallMapState::Downloading)
		installControlsHeight = 24.f;
	else if(state == InstallMapState::Installing)
		installControlsHeight = ImGui::GetTextLineHeight();
	else if(!mapsDirectoryValid)
		installControlsHeight += ImGui::GetTextLineHeightWithSpacing();

	if(installedSuccessfully)
		installControlsHeight += ImGui::GetTextLineHeightWithSpacing();

	const float bottomPadding = ImGui::GetStyle().WindowPadding.y;
	const float availableVersionsHeight = ImGui::GetContentRegionAvail().y - installControlsHeight - ImGui::GetStyle().ItemSpacing.y - bottomPadding;
	const float versionsHeight = availableVersionsHeight > 100.f ? availableVersionsHeight : 100.f;
	ImGui::BeginChild("##InstallMapVersions", ImVec2(0.f, versionsHeight), true);
	ImGui::Text("%s", g_pluginTexts.InstallMapVersions.c_str());
	ImGui::Separator();
	for(int index = 0; index < static_cast<int>(mapInfo.m_versions.size()); index++)
	{
		const BakkesPluginsMapVersion& version = mapInfo.m_versions[index];
		std::string label = version.m_version;
		if(version.m_isLatest)
			label += " (" + g_pluginTexts.InstallMapLatest + ")";

		const bool isSelected = index == selectedVersion;
		if(ImGui::Selectable((label + "##InstallMapVersion" + std::to_string(index)).c_str(), isSelected))
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_selectedVersion = index;
			selectedVersion = index;
		}

		if(isSelected)
		{
			ImGui::Indent();
			ImGui::TextDisabled("%s %s", g_pluginTexts.InstallMapDate.c_str(), version.m_releaseDate.c_str());
			ImGui::TextDisabled("%s %s", g_pluginTexts.InstallMapFileSize.c_str(), version.m_fileSize.c_str());
			ImGui::Unindent();
		}
	}
	ImGui::EndChild();

	ImGui::Spacing();
	if(state == InstallMapState::Downloading)
	{
		unsigned long long downloadedBytes = m_downloadedBytes.load();
		unsigned long long totalBytes = m_totalBytes.load();
		unsigned long long maximumBytes = totalBytes > 0 ? totalBytes : (downloadedBytes > 0 ? downloadedBytes : 1);
		std::string label = Utils::ConvertToMB(std::to_string(downloadedBytes)) + " / ";
		label += totalBytes > 0 ? Utils::ConvertToMB(std::to_string(totalBytes)) : "?";
		float progressValue = static_cast<float>(downloadedBytes < maximumBytes ? downloadedBytes : maximumBytes);
		float progressMaximum = static_cast<float>(maximumBytes);
		Utils::RenderProgressBar(progressValue, progressMaximum, ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetContentRegionAvailWidth(), 24.f),
			ImColor(112, 112, 112, 255), ImColor(33, 65, 103, 255), label.c_str());
	}
	else if(state == InstallMapState::Installing)
		ImGui::TextWrapped("%s", g_pluginTexts.InstallMapInstalling.c_str());
	else
	{
		if(!mapsDirectoryValid)
			ImGui::TextColored(ImVec4(1.f, 0.25f, 0.25f, 1.f), "%s", g_pluginTexts.InstallMapInvalidFolder.c_str());

		if(!mapsDirectoryValid)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if(ImGui::Button(g_pluginTexts.InstallMapButton.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 32.f)) && mapsDirectoryValid)
		{
			if(!GetInstalledMapPath(mapInfo).empty())
				ImGui::OpenPopup("Replace BakkesPlugins Map");
			else
				StartInstallation(false);
		}

		if(!mapsDirectoryValid)
		{
			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
		}
	}

	if(installedSuccessfully)
		ImGui::TextColored(ImVec4(0.2f, 0.85f, 0.35f, 1.f), "%s", g_pluginTexts.InstallMapSuccess.c_str());

	Utils::RenderYesNoPopup("Replace BakkesPlugins Map", g_pluginTexts.InstallMapReplace.c_str(), g_pluginTexts.Yes.c_str(),
		g_pluginTexts.No.c_str(),
		[this]() {
			StartInstallation(true);
			ImGui::CloseCurrentPopup();
		},
		[this]() {
			ImGui::CloseCurrentPopup();
		});
	ImGui::EndChild();

	ImGui::SameLine(0.f, columnSpacing);
	ImGui::BeginChild("##InstallMapInformation", ImVec2(0.f, contentHeight), true);
	ImGui::Text("%s", mapInfo.m_name.c_str());
	ImGui::Separator();
	ImGui::Text("%s %s", g_pluginTexts.InstallMapCreator.c_str(), mapInfo.m_author.c_str());
	ImGui::Text("%s %s", g_pluginTexts.InstallMapPublished.c_str(), mapInfo.m_publishedDate.c_str());
	ImGui::Text("%s %s", g_pluginTexts.InstallMapUpdated.c_str(), mapInfo.m_lastUpdatedDate.c_str());
	ImGui::Text("%s %s", g_pluginTexts.InstallMapViews.c_str(), mapInfo.m_views.c_str());
	ImGui::Text("%s %s", g_pluginTexts.InstallMapDownloads.c_str(), mapInfo.m_downloads.c_str());

	ImGui::Separator();
	ImGui::BeginChild("MapDescription");
	{
		ImGui::Text("%s", g_pluginTexts.InstallMapDescription.c_str());
		ImGui::NewLine();
		ImGui::TextWrapped("%s", mapInfo.m_description.c_str());
		ImGui::EndChild();
	}
	ImGui::EndChild();
}

void BakkesPluginsMapInstaller::SetDownloadProgress(unsigned long long p_downloadedBytes, unsigned long long p_totalBytes)
{
	m_downloadedBytes = p_downloadedBytes;
	m_totalBytes = p_totalBytes;
}

bool BakkesPluginsMapInstaller::ParseMapURL(std::string p_url, std::string& p_mapID, std::string& p_normalizedUrl)
{
	p_url = TrimString(p_url);
	std::regex urlPattern(R"(^https://bakkesplugins\.com/maps/([1-9][0-9]*)/?(?:[?#].*)?$)", std::regex_constants::icase);
	std::smatch matches;
	if (!std::regex_match(p_url, matches, urlPattern))
		return false;

	p_mapID = matches[1].str();
	p_normalizedUrl = "https://bakkesplugins.com/maps/" + p_mapID;
	return true;
}

bool BakkesPluginsMapInstaller::ParseMapHTML(const std::string& p_detailsHTML, const std::string& p_versionsHTML, const std::string& p_mapID,
	const std::string& p_sourceUrl, BakkesPluginsMapInfo& p_mapInfo, std::string& p_error)
{
	p_mapInfo = {};
	p_mapInfo.m_mapID = p_mapID;
	p_mapInfo.m_sourceUrl = p_sourceUrl;
	p_mapInfo.m_name = ExtractMetaContent(p_detailsHTML, "property", "og:title");
	p_mapInfo.m_author = ExtractMetaContent(p_detailsHTML, "name", "author");
	p_mapInfo.m_description = ExtractMapDescription(p_detailsHTML);
	p_mapInfo.m_previewUrl = ExtractMetaContent(p_detailsHTML, "property", "og:image");
	p_mapInfo.m_publishedDate = ExtractDefinitionValue(p_detailsHTML, "Created");
	p_mapInfo.m_lastUpdatedDate = ExtractDefinitionValue(p_detailsHTML, "Last Updated");
	p_mapInfo.m_views = ExtractDefinitionValue(p_detailsHTML, "Views");
	p_mapInfo.m_downloads = ExtractDefinitionValue(p_detailsHTML, "Downloads");

	size_t navigation = p_versionsHTML.find("<nav aria-label=\"Map detail tabs\"");
	size_t versionsEnd = navigation == std::string::npos ? std::string::npos : p_versionsHTML.find("</section>", navigation);
	if (navigation == std::string::npos || versionsEnd == std::string::npos)
	{
		p_error = "The versions section could not be found on this page.";
		return false;
	}

	size_t articlePosition = navigation;
	while ((articlePosition = p_versionsHTML.find("<article", articlePosition)) != std::string::npos && articlePosition < versionsEnd)
	{
		size_t articleEnd = p_versionsHTML.find("</article>", articlePosition);
		if (articleEnd == std::string::npos || articleEnd > versionsEnd)
			break;

		std::string article = p_versionsHTML.substr(articlePosition, articleEnd - articlePosition + 10);
		std::vector<std::string> versionValues = ExtractSpanValues(article, "text-lg font-semibold text-gray-900");
		std::vector<std::string> dateValues = ExtractSpanValues(article, "text-sm text-gray-500");
		std::vector<std::string> detailValues = ExtractSpanValues(article, "inline-flex items-center gap-1");

		size_t downloadPosition = article.find("href=\"https://cdn.bakkesplugins.com/");
		std::string downloadUrl;
		if (downloadPosition != std::string::npos)
		{
			size_t tagEnd = article.find('>', downloadPosition);
			size_t tagBegin = article.rfind("<a", downloadPosition);
			if (tagBegin != std::string::npos && tagEnd != std::string::npos)
				downloadUrl = ExtractAttribute(article.substr(tagBegin, tagEnd - tagBegin + 1), "href");
		}

		if (!versionValues.empty() && !dateValues.empty() && detailValues.size() >= 2 && IsValidBakkesPluginsDownloadURL(downloadUrl))
		{
			BakkesPluginsMapVersion version;
			version.m_version = versionValues[0];
			version.m_releaseDate = dateValues[0];
			version.m_fileSize = detailValues[0];
			version.m_downloads = RemoveSuffix(detailValues[1], "downloads");
			version.m_downloadUrl = downloadUrl;
			version.m_isLatest = article.find(">Latest</span>") != std::string::npos;
			p_mapInfo.m_versions.push_back(version);
		}

		articlePosition = articleEnd + 10;
	}

	if (p_mapInfo.m_name.empty() || p_mapInfo.m_author.empty() || p_mapInfo.m_description.empty())
	{
		p_error = "Required map information is missing from the BakkesPlugins page.";
		return false;
	}

	if (GetBakkesPluginsImageExtension(p_mapInfo.m_previewUrl).empty())
	{
		p_error = "No supported map image was found on the BakkesPlugins page.";
		return false;
	}

	if (p_mapInfo.m_versions.empty())
	{
		p_error = "No downloadable map version was found.";
		return false;
	}

	return true;
}

void BakkesPluginsMapInstaller::StartLookup()
{
	std::string mapID;
	std::string sourceUrl;
	if (!ParseMapURL(m_urlBuffer, mapID, sourceUrl))
	{
		SetError("Use an URL formatted like https://bakkesplugins.com/maps/298.");
		return;
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_state == InstallMapState::Loading || m_state == InstallMapState::Downloading ||
			m_state == InstallMapState::Installing)
			return;
	}

	JoinWorker();
	m_cancelRequested = false;
	m_downloadedBytes = 0;
	m_totalBytes = 0;
	m_previewImage.reset();
	m_previewReady = false;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_mapInfo = {};
		m_error.clear();
		m_state = InstallMapState::Loading;
		m_selectedVersion = 0;
		m_installedSuccessfully = false;
		m_previewCachePath.clear();
	}

	m_worker = std::thread(&BakkesPluginsMapInstaller::FetchMapInfo, this, mapID, sourceUrl);
}

void BakkesPluginsMapInstaller::FetchMapInfo(std::string p_mapID, std::string p_sourceUrl)
{
	std::string detailsHTML;
	std::string versionsHTML;
	std::string error;
	if (!DownloadString(p_sourceUrl, detailsHTML, m_cancelRequested, error))
	{
		if (!m_cancelRequested)
			SetError("Unable to retrieve the map: " + error);
		return;
	}

	if (!DownloadString(p_sourceUrl + "?tab=versions", versionsHTML, m_cancelRequested, error))
	{
		if (!m_cancelRequested)
			SetError("Unable to retrieve the map versions: " + error);
		return;
	}

	BakkesPluginsMapInfo mapInfo;
	if (!ParseMapHTML(detailsHTML, versionsHTML, p_mapID, p_sourceUrl, mapInfo, error))
	{
		SetError(error);
		return;
	}

	const std::string imageExtension = GetBakkesPluginsImageExtension(mapInfo.m_previewUrl);
	const std::filesystem::path previewDirectory = std::filesystem::path(g_pluginConfig.BakkesmodPath) / "data" / "WorkshopMapLoader" / "temp";
	const std::filesystem::path previewPath = previewDirectory / ("map_" + p_mapID + imageExtension);
	try
	{
		std::filesystem::create_directories(previewDirectory);
		if(DownloadPreviewFile(mapInfo.m_previewUrl, previewPath, m_cancelRequested))
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_previewCachePath = previewPath;
			m_previewReady = true;
		}
	}
	catch(const std::exception& p_exception)
	{
		m_cvarManager->log("Unable to cache the BakkesPlugins preview: " + std::string(p_exception.what()));
	}

	int selectedVersion = 0;
	for (int index = 0; index < static_cast<int>(mapInfo.m_versions.size()); index++)
	{
		if (mapInfo.m_versions[index].m_isLatest)
		{
			selectedVersion = index;
			break;
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_mapInfo = std::move(mapInfo);
		m_error.clear();
		m_state = InstallMapState::Ready;
		m_selectedVersion = selectedVersion;
	}
}

void BakkesPluginsMapInstaller::StartInstallation(bool p_replaceExisting)
{
	BakkesPluginsMapInfo mapInfo;
	BakkesPluginsMapVersion version;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_state != InstallMapState::Ready || m_selectedVersion < 0 ||
			m_selectedVersion >= static_cast<int>(m_mapInfo.m_versions.size()))
			return;

		mapInfo = m_mapInfo;
		version = m_mapInfo.m_versions[m_selectedVersion];
		m_state = InstallMapState::Downloading;
		m_error.clear();
		m_installedSuccessfully = false;
	}

	JoinWorker();
	m_cancelRequested = false;
	m_downloadedBytes = 0;
	m_totalBytes = 0;
	m_worker = std::thread(&BakkesPluginsMapInstaller::InstallMap, this, mapInfo, version, p_replaceExisting);
}

std::filesystem::path BakkesPluginsMapInstaller::GetInstalledMapPath(const BakkesPluginsMapInfo& p_mapInfo)
{
	std::filesystem::path mapsPath = g_pluginConfig.MapsFolderPathBuffer;
	if (!Utils::DirectoryOrFileExists(mapsPath))
		return {};

	try
	{
		for (const auto& directory : fs::directory_iterator(mapsPath))
		{
			if (!directory.is_directory())
				continue;

			for (const auto& file : fs::directory_iterator(directory.path()))
			{
				if (!file.is_regular_file() || ToLowerString(file.path().extension().string()) != ".json")
					continue;

				std::ifstream jsonFile(file.path());
				Json::Value json;
				Json::Reader reader;
				if (reader.parse(jsonFile, json) && json["MapID"].asString() == p_mapInfo.m_mapID)
					return directory.path();
			}
		}
	}
	catch (...)
	{
		return {};
	}

	std::filesystem::path expectedPath = mapsPath / SanitizeFileName(p_mapInfo.m_name);
	return Utils::DirectoryOrFileExists(expectedPath) ? expectedPath : std::filesystem::path{};
}

std::filesystem::path BakkesPluginsMapInstaller::GetDestinationPath(const BakkesPluginsMapInfo& p_mapInfo)
{
	std::filesystem::path installedPath = GetInstalledMapPath(p_mapInfo);
	if (!installedPath.empty())
		return installedPath;

	return std::filesystem::path(g_pluginConfig.MapsFolderPathBuffer) / SanitizeFileName(p_mapInfo.m_name);
}

void BakkesPluginsMapInstaller::WriteMapMetadata(const std::filesystem::path& p_directory, const BakkesPluginsMapInfo& p_mapInfo,
	const BakkesPluginsMapVersion& p_version)
{
	Json::Value json;
	json["Title"] = p_mapInfo.m_name;
	json["Author"] = p_mapInfo.m_author;
	json["Description"] = p_mapInfo.m_description;
	json["PreviewUrl"] = p_mapInfo.m_previewUrl;
	json["Source"] = "BakkesPlugins";
	json["MapID"] = p_mapInfo.m_mapID;
	json["SourceURL"] = p_mapInfo.m_sourceUrl;
	json["Version"] = p_version.m_version;

	Json::StreamWriterBuilder writer;
	writer["indentation"] = "\t";
	std::ofstream jsonFile(p_directory / (SanitizeFileName(p_mapInfo.m_name) + ".json"));
	if (!jsonFile)
		throw std::runtime_error("Unable to create the local map metadata.");
	jsonFile << Json::writeString(writer, json);
}

void BakkesPluginsMapInstaller::InstallMap(BakkesPluginsMapInfo p_mapInfo, BakkesPluginsMapVersion p_version, bool p_replaceExisting)
{
	std::filesystem::path mapsPath = g_pluginConfig.MapsFolderPathBuffer;
	std::filesystem::path destinationPath;
	std::filesystem::path stagePath;
	std::filesystem::path backupPath;
	bool backupCreated = false;

	try
	{
		if (!Utils::DirectoryOrFileExists(mapsPath))
			throw std::runtime_error("The configured maps directory is not valid.");

		std::filesystem::path installedPath = GetInstalledMapPath(p_mapInfo);
		if (!installedPath.empty() && !p_replaceExisting)
			throw std::runtime_error("This map is already installed.");

		destinationPath = installedPath.empty() ? GetDestinationPath(p_mapInfo) : installedPath;
		stagePath = mapsPath / (".wml_install_" + p_mapInfo.m_mapID);
		backupPath = mapsPath / (".wml_backup_" + p_mapInfo.m_mapID);

		std::error_code errorCode;
		fs::remove_all(stagePath, errorCode);
		if (Utils::DirectoryOrFileExists(backupPath))
			throw std::runtime_error("A previous installation backup still exists: " + backupPath.string());

		fs::create_directory(stagePath);
		std::string zipName = SanitizeFileName(p_mapInfo.m_name + "-" + p_version.m_version) + ".zip";
		std::filesystem::path zipPath = stagePath / zipName;
		std::string downloadError;
		if (!DownloadFile(p_version.m_downloadUrl, zipPath, this, downloadError))
			throw std::runtime_error(downloadError);

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_state = InstallMapState::Installing;
		}

		if (g_pluginConfig.UnzipMethod == "Bat")
			Utils::CreateUnzipBatchFile(g_pluginConfig.BakkesmodPath, stagePath.string(), zipPath.string());
		else
		{
			std::string extractCommand = "powershell.exe -NoProfile -Command \"Expand-Archive -LiteralPath '" +
				EscapePowerShellLiteral(zipPath.string()) + "' -DestinationPath '" + EscapePowerShellLiteral(stagePath.string()) + "' -Force\"";
			if (system(extractCommand.c_str()) != 0)
				throw std::runtime_error("PowerShell failed to extract the downloaded ZIP.");
		}

		for (int attempt = 0; attempt < 30 && !FindMapFile(stagePath).has_value(); attempt++)
		{
			if (m_cancelRequested)
				throw std::runtime_error("Installation cancelled.");
			Sleep(500);
		}

		NormalizeSingleWrapperDirectory(stagePath, zipPath);
		std::optional<std::filesystem::path> mapFile = FindMapFile(stagePath);
		if (!mapFile.has_value())
			throw std::runtime_error("No .udk or .upk map file was found in the downloaded archive.");

		if (ToLowerString(mapFile->extension().string()) == ".udk")
		{
			std::filesystem::path upkPath = *mapFile;
			upkPath.replace_extension(".upk");
			fs::rename(*mapFile, upkPath);
		}

		std::string imageExtension = GetBakkesPluginsImageExtension(p_mapInfo.m_previewUrl);
		std::filesystem::path imagePath = stagePath / (SanitizeFileName(p_mapInfo.m_name) + imageExtension);
		if (!DownloadFile(p_mapInfo.m_previewUrl, imagePath, this, downloadError))
			throw std::runtime_error("Unable to download the map image: " + downloadError);

		WriteMapMetadata(stagePath, p_mapInfo, p_version);

		if (Utils::DirectoryOrFileExists(destinationPath))
		{
			fs::rename(destinationPath, backupPath);
			backupCreated = true;
		}

		try
		{
			fs::rename(stagePath, destinationPath);
		}
		catch (...)
		{
			if (backupCreated && !Utils::DirectoryOrFileExists(destinationPath))
				fs::rename(backupPath, destinationPath);
			throw;
		}

		if (backupCreated)
		{
			std::error_code removeBackupError;
			fs::remove_all(backupPath, removeBackupError);
		}

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_state = InstallMapState::Ready;
			m_installedSuccessfully = true;
		}
		m_refreshRequested = true;
	}
	catch (const std::exception& exception)
	{
		std::error_code errorCode;
		if (!stagePath.empty())
			fs::remove_all(stagePath, errorCode);

		if (backupCreated && !destinationPath.empty() && !Utils::DirectoryOrFileExists(destinationPath) &&
			Utils::DirectoryOrFileExists(backupPath))
			fs::rename(backupPath, destinationPath, errorCode);

		if (!m_cancelRequested)
			SetError(exception.what());
	}
}

void BakkesPluginsMapInstaller::SetError(std::string p_error)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_error = std::move(p_error);
	m_state = InstallMapState::Error;
	m_installedSuccessfully = false;
}

void BakkesPluginsMapInstaller::JoinWorker()
{
	if (m_worker.joinable() && m_worker.get_id() != std::this_thread::get_id())
		m_worker.join();
}
