#ifndef ZENPP_PROGRAM_H
#define ZENPP_PROGRAM_H

#ifdef _MSC_VER
#pragma once
#endif

// Forward declarations
class AboutDialog;
class FirmwareDialog;
class FirmwareManager;
class MainDialog;
class VersionCheck;

class Program
{
public:
	// Constructor for initialization
	explicit Program(_In_ CONST HINSTANCE Instance, _In_ CONST std::string& CommandLine);

	// Destructor for cleanup
	~Program();

	// Public methods for dialog and version check access
	AboutDialog& GetAboutDialog(VOID) CONST;
	CronusZen& GetCronusZen(VOID) CONST;
	FirmwareDialog& GetFirmwareDialog(VOID) CONST;
	MainDialog& GetMainDialog(VOID) CONST;
	VersionCheck& GetVersionCheck(VOID) CONST;

	// Public methods for program initialization, state and message loop
	BOOLEAN InitializeProgram(VOID);
	BOOLEAN IsAdministrator(VOID) CONST;
	BOOL IsQuitting(VOID) CONST;
	HINSTANCE GetInstance(VOID) CONST;
	INT RunMessageLoop(VOID);
	VOID QuitProgram(VOID);

	// Public methods for dialog management
	VOID CreateFirmwareDialog(_In_ CONST FirmwareManager::FirmwareModificationPurpose Purpose);
	VOID DestroyAboutDialog(VOID);
	VOID DestroyFirmwareDialog(VOID);
	VOID ShowAboutDialog(VOID);

	// Public method for error display
	VOID DisplayError(_In_ CONST std::wstring& AdditionalMessage);

	// Public methods for ANSI to Unicode converstions
	CONST std::string UnicodeToAnsi(_In_ CONST std::wstring& String);
	CONST std::wstring AnsiToUnicode(_In_ CONST std::string& String);
	CONST std::wstring BytesToUnicode(_In_ CONST PBYTE& Bytes, _In_ CONST UINT BytesSize);

private:
	// Private members for program objects, state and resources
	std::unique_ptr<AboutDialog> m_AboutDialog;
	std::unique_ptr<CronusZen> m_CronusZen;
	std::unique_ptr<FirmwareDialog> m_FirmwareDialog;
	std::unique_ptr<MainDialog> m_MainDialog;
	std::unique_ptr<VersionCheck> m_VersionCheck;
	BOOL m_Administrator = FALSE;
	BOOL m_Updated = FALSE;
	BOOL m_Quitting = FALSE;
	HANDLE m_Mutex = NULL;
	HINSTANCE m_Instance = NULL;
	HMODULE m_RichEdit = NULL;

	// Private method for checking administrator privileges
	VOID CheckAdministrator(VOID);
};

extern std::unique_ptr<Program> App;

#endif
