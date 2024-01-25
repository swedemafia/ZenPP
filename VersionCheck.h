#ifndef CLIENTS_VERSIONCHECK_H
#define CLIENTS_VERSIONCHECK_H

#ifdef _MSC_VER
#pragma once
#endif

class VersionCheck : public HttpSessionBase {
public:
	// Constructor for initialization
	VersionCheck(_In_ CONST std::wstring& Domain, _In_ CONST std::wstring& PathToFile);

	// Public methods for news and version management
	VOID CheckUpdatesAndNews(VOID);
	VOID DisplayNews(VOID) CONST;

protected:
	// Protected member for handling HTTP session data
	VOID HttpSessionDataCallback(CONST std::deque<std::string> ResponseData);

private:
	// Private members for update information and news data
	std::string m_CommandLine;
	std::unique_ptr<ResourceFile> m_VersionUpdater;
	std::vector<std::wstring> m_News;
	std::vector<std::wstring> m_UpdateNotes;

	// Private methods for processing various version check file commands
	VOID ProcessAlertCommand(_In_ CONST UINT MessageBoxType, _In_ CONST std::string MessageBoxText);
	VOID ProcessNewsCommand(_In_ CONST COLORREF Color, _In_ CONST std::string_view NewsString);
	VOID ProcessUpdateAddress(_In_ CONST std::string& UpdateAddress);
	VOID ProcessVersionCommand(_In_ CONST std::string_view VersionString);
};

#endif
