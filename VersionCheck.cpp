#include "Precomp.h"

VersionCheck::VersionCheck(CONST std::wstring& Domain, CONST std::wstring& PathToFile)
{
	m_Domain = Domain;
	m_PathToFile = PathToFile;
}

VOID VersionCheck::CheckUpdatesAndNews(VOID)
{
	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Notify user of status
	MainDialog.PrintTimestamp();
	MainDialog.PrintText(GRAY, L"Checking for news and updates...\r\n");

	// Query the updates and news file
	DownloadFile();
}

VOID VersionCheck::DisplayNews(VOID) CONST
{
	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	for (const auto NewsItem : m_News) {
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(LIGHTGREEN, L"%ws\r\n", NewsItem.c_str());
	}
}

VOID VersionCheck::ProcessAlertCommand(CONST UINT MessageBoxType, CONST std::string MessageBoxText)
{
	std::wstring Text(MessageBoxText.begin(), MessageBoxText.end());

	// Provide message box alert to user
	MessageBox(App->GetMainDialog().GetHwnd(), Text.c_str(), L"Zen++ Alert", MessageBoxType | MB_OK);
}

VOID VersionCheck::ProcessNewsCommand(CONST COLORREF Color, CONST std::string_view NewsString)
{
	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Convert ANSI string to Unicode
	std::wstring NewsText = std::wstring(NewsString.begin(), NewsString.end());
	m_News.push_back(NewsText);

	// Display news to output window
	MainDialog.PrintTimestamp();
	MainDialog.PrintText(Color, L"%ws\r\n", NewsText.c_str());
}

VOID VersionCheck::ProcessUpdateAddress(CONST std::string& UpdateAddress)
{
	// Process line
	size_t Position = UpdateAddress.find_first_of(L'/');

	if (Position != std::wstring::npos) {
		std::string Domain = UpdateAddress.substr(0, Position);
		std::string PathToFile = UpdateAddress.substr(Position + 1);
		m_CommandLine = "ZppUpdater.exe " + Domain + " " + PathToFile;
	}
}

VOID VersionCheck::ProcessVersionCommand(CONST std::string_view VersionString)
{
	std::unique_ptr<SemanticVersion> VersionInfo = std::make_unique<SemanticVersion>(VersionString.data());

	// Compare version
	CHAR Major = VersionInfo->GetMajor();
	CHAR Minor = VersionInfo->GetMinor();
	CHAR Revision = VersionInfo->GetRevision();
	MainDialog* Main = &App->GetMainDialog();

	if ((Major > VERSION_MAJOR) || (Minor > VERSION_MINOR) || (Revision > VERSION_REVISION)) {
		Main->PrintTimestamp();
		Main->PrintText(PINK, L"New version available: %u.%u.%u.\r\n", Major, Minor, Revision);

		// Print update notes
		for (const auto& Message : m_UpdateNotes) {
			Main->PrintTimestamp();
			Main->PrintText(YELLOW, L"%ws\r\n", Message.c_str());
		}

		// Notify user
		if (MessageBox(Main->GetHwnd(), L"An update is available for this application.\r\n\r\nPress OK to launch the updater.", L"Zen++", MB_ICONINFORMATION | MB_OK) == IDOK) {
			try {
				// Initialize variables used for the extraction and execution of the Zen++ Updater
				DWORD BytesWritten = 0;
				PROCESS_INFORMATION ProcessInfo = { NULL };
				STARTUPINFOA StartupInfo = { sizeof(STARTUPINFOA) };
				m_VersionUpdater = std::make_unique<ResourceFile>(App->GetInstance(), FILE_ZPPUPDATER);
				std::unique_ptr<File> UpdateFile = std::make_unique<File>(L"ZppUpdater.exe", GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, TRUE);

				// Attempt to open the target file
				if (!UpdateFile->Open())
					throw std::wstring(L"An error occured while executing ZppUpdater.");

				// Attempt to write the file data for the Zen++ Updater
				if (!UpdateFile->Write(m_VersionUpdater->GetFileData(), m_VersionUpdater->GetFileSize(), NULL, NULL))
					throw std::wstring(L"An error occured while creating ZppUpdater.");

				// Close the file after writing to it
				UpdateFile->Close();

				// Attempt to execute the Zen++ updater and provide command-line parameters
				if (!CreateProcessA("ZppUpdater.exe", &m_CommandLine[0], NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInfo)) {
					throw std::wstring(L"An error occured while launching ZppUpdater.");
				} 
				else
					ExitProcess(0);
			}
			catch (CONST std::wstring& CustomMessage) {
				App->DisplayError(CustomMessage);
			} catch (CONST std::bad_alloc&) {
				App->DisplayError(L"Unable to create the ResourceFile object; insufficient memory is available to complete the required operation.");
			}
		}
	}
}

VOID VersionCheck::HttpSessionDataCallback(CONST std::deque<std::string> ResponseData)
{
	// Iterate through each item of the HTTP response data and process it based on the command
	for (const auto& Data : ResponseData) {
		// Initialize a variable to hold the current line of data being processed
		std::string Line(Data);

		// Process line
		size_t Position = Line.find_first_of(L':');

		if (Position != std::wstring::npos) {
			std::string Command = Line.substr(0, Position);
			std::string Text = Line.substr(Position + 1);

			// Process commands
			if (!Command.compare("update_notes")) {
				m_UpdateNotes.push_back(std::wstring(Text.begin(), Text.end()));
			} else if (!Command.compare("address")) {
				ProcessUpdateAddress(Text);
			} else if (!Command.compare("version")) {
				ProcessVersionCommand(Text);
			} else if (!Command.compare("news")) {
				ProcessNewsCommand(LIGHTGREEN, Text);
			} else if (!Command.compare("news2")) {
				ProcessNewsCommand(YELLOW, Text);
			} else if (!Command.compare("notice")) {
				ProcessAlertCommand(MB_ICONINFORMATION, Text.data());
			} else if (!Command.compare("warning")) {
				ProcessAlertCommand(MB_ICONWARNING, Text.data());
			}
		}
	}
}