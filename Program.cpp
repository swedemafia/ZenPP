#include "Precomp.h"

Program::Program(CONST HINSTANCE Instance, CONST std::string& CommandLine) :
	m_Instance(Instance)
{
	// Create a mutex to ensure another instance of this app is not running
	m_Mutex = CreateMutex(NULL, NULL, L"Zen++ Instance");

	// Check if the mutex already exists, if so, terminate this process
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// Alert user
		DisplayError(L"It appears that another instance of Zen++ is already running.\r\n\r\nPlease close the existing instance before starting a new one.");

		// Close mutex object if it was created
		if (m_Mutex)
			CloseHandle(m_Mutex);

		// Terminate the process
		ExitProcess(0);
	}

	// Check if launching from ZppUpdater
	if (CommandLine == "updated")
		m_Updated = TRUE;
}

Program::~Program()
{
	// Close mutex and terminate the process
	if (m_Mutex)
		CloseHandle(m_Mutex);

	// Free RichEdit library
	if (m_RichEdit)
		FreeLibrary(m_RichEdit);
}

AboutDialog& Program::GetAboutDialog(VOID) CONST
{
	return *m_AboutDialog;
}

CronusZen& Program::GetCronusZen(VOID) CONST
{
	return *m_CronusZen;
}

FirmwareDialog& Program::GetFirmwareDialog(VOID) CONST
{
	return *m_FirmwareDialog;
}

MainDialog& Program::GetMainDialog(VOID) CONST
{
	return *m_MainDialog;
}

VersionCheck& Program::GetVersionCheck(VOID) CONST
{
	return *m_VersionCheck;
}


BOOLEAN Program::InitializeProgram(VOID)
{
	try
	{
		// Load RichEdit library
		if (!(m_RichEdit = LoadLibrary(L"RichEd20.dll")))
			throw std::wstring(L"An error occured while loading the RichEdit library.\r\n\r\nPlease restart the application or contact support for assistance.");

		// Initialize objects
		m_CronusZen = std::make_unique<CronusZen>();
		m_MainDialog = std::make_unique<MainDialog>();
		m_Registry = std::make_unique<Registry>(L"Software\\Asgard Productions\\");

		// Read configuration data
		ReadUserVariables();

		// Delete update and download files
		DeleteFile(L"ZppUpdater.exe");
		DeleteFile(L"..\\ZppUpdater.exe");

		// Create dialog
		m_MainDialog->Create(DIALOG_MAIN);
		m_MainDialog->RichEditInitialize(RICHEDIT_MAIN_OUTPUT, L"Courier New", 180, 1160);
		m_MainDialog->DisplayStartupInfo();

		// Display message to RichEdit control if the app was just updated
		if (m_Updated) {
			m_MainDialog->PrintTimestamp();
			m_MainDialog->PrintText(LIGHTBLUE, L"Successfully updated to latest version %u.%u.%u.\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
		}

		// Check for elevated privileges
		CheckAdministrator();

		// Display administrative privileges status
		m_MainDialog->DisplayAdministratorStatus();

		// Create version check object
#ifndef _DEBUG
		m_VersionCheck = std::make_unique<VersionCheck>(L"pastebin.com", L"/raw/tU0aHMq0");
#else
		m_VersionCheck = std::make_unique<VersionCheck>(L"pastebin.com", L"/raw/tU0aHMq0");
#endif
		// Validate VersionCheck object was created
		if (!m_VersionCheck.get())
			EnableMenuItem(GetMainDialog().GetMenuHandle(), MENU_HELP_NEWS, MF_BYCOMMAND | MF_DISABLED);

		// Check for connect on startup
		if(GetConnectOnStartup())
			m_CronusZen->ConnectToDevice();

//#ifndef _DEBUG
		m_VersionCheck->CheckUpdatesAndNews();
//#endif
	}
	catch (CONST std::wstring& CustomMessage)
	{
		DisplayError(CustomMessage);
		return FALSE;
	}
	catch (CONST std::bad_alloc&) {
		DisplayError(L"Insufficient memory available!\r\n\r\nClose other applications to free up memory.\r\n\r\nPlease restart the application or contact support for assistance.");
		return FALSE;
	}

	return TRUE;
}

BOOLEAN Program::IsAdministrator(VOID) CONST
{
	return m_Administrator;
}

BOOL Program::IsQuitting(VOID) CONST
{
	return m_Quitting;
}

CONST BOOL Program::GetBlackBackground(VOID) CONST
{
	return m_BlackBackground;
}

CONST BOOL Program::GetConnectOnStartup(VOID) CONST
{
	return m_ConnectOnStartup;
}

CONST BOOL Program::GetDisplayVMSpeed(VOID) CONST
{
	return m_DisplayVMSpeed;
}

CONST BOOL Program::GetDisplay24HourTimestamps(VOID) CONST
{
	return m_24HourTimestamps;
}

CONST BOOL Program::GetLowPerformanceMode(VOID) CONST
{
	return m_LowPerformanceMode;
}

VOID Program::SetBlackBackground(CONST BOOL Value)
{
	m_Registry->SetValue(L"Black Background", Value);
	m_BlackBackground = Value;
}

VOID Program::SetConnectOnStartup(CONST BOOL Value)
{
	m_Registry->SetValue(L"Connect on Startup", Value);
	m_ConnectOnStartup = Value;
}

VOID Program::SetDisplayVMSpeed(CONST BOOL Value)
{
	m_Registry->SetValue(L"Display VM Speed", Value);
	m_DisplayVMSpeed = Value;
}

VOID Program::SetDisplay24HourTimestamps(CONST BOOL Value)
{
	m_Registry->SetValue(L"Display 24-hour Timestamps", Value);
	m_24HourTimestamps = Value;
}

VOID Program::SetLowPerformanceMode(CONST BOOL Value)
{
	m_Registry->SetValue(L"Low Performance Mode", Value);
	m_LowPerformanceMode = Value;
}

// Method for converting an ANSI string to a Unicode string
CONST std::wstring Program::AnsiToUnicode(CONST std::string& String)
{
	// Determine the code page for the current user locale
	UINT CodePage = GetACP();

	// Calculate the size of the output buffer in wide characters
	int BufferSize = MultiByteToWideChar(CodePage, 0, String.c_str(), -1, nullptr, 0);

	// Validate size of buffer
	if (BufferSize == 0) {
		DisplayError(L"An error occured while converting an ANSI string to Unicode.");
		return L"";
	}

	// Allocate a buffer to hold the resulting Unicode string
	std::wstring Result(BufferSize - 1, L'\0');

	// Perform the actual conversion
	MultiByteToWideChar(CodePage, 0, String.c_str(), -1, &Result[0], BufferSize);

	return Result;
}

// Method for converting raw bytes to a Unicode string
CONST std::wstring Program::BytesToUnicode(CONST PBYTE& Bytes, CONST UINT BytesSize)
{
	// Determine the code page for the current user locale
	UINT CodePage = GetACP();

	// Calculate the size of the output buffer in wide characters
	int BufferSize = MultiByteToWideChar(CodePage, 0, reinterpret_cast<const char*>(Bytes), BytesSize, NULL, 0);

	// Validate size of buffer
	if (BufferSize == 0) {
		// Handle conversion error
		throw std::exception("Failed to determine wide character count");
	}

	// Allocate a buffer to hold the resulting Unicode string
	std::wstring UnicodeString(BufferSize - 1, L'\0');

	// Perform the actual conversion
	int Result = MultiByteToWideChar(CodePage, 0, reinterpret_cast<const char*>(Bytes), BytesSize, &UnicodeString[0], BufferSize);

	// Validate result
	if (Result == 0) {
		// Handle conversion error
		throw std::exception("Failed to convert bytes to Unicode string");
	}

	return UnicodeString;
}

// Method for converting a Unicode string to an ANSI string
CONST std::string Program::UnicodeToAnsi(CONST std::wstring& String)
{
	// Determine the code page for the current user locale
	UINT CodePage = GetACP();

	// Calculate the size of the buffer in ANSI characters
	INT BufferSize = WideCharToMultiByte(CodePage, 0, String.c_str(), -1, nullptr, 0, nullptr, nullptr);

	// Validate size of buffer
	if (BufferSize == 0) {
		DisplayError(L"An error occured while converting the Unicode string " + String + L" to ANSI.");
		return "";
	}

	// Allocate a buffer to hold the resulting ANSI string
	std::string Result(BufferSize - 1, '\0');

	// Perform the actual conversion
	WideCharToMultiByte(CodePage, 0, String.c_str(), -1, &Result[0], BufferSize, nullptr, nullptr);

	return Result;
}

HINSTANCE Program::GetInstance(VOID) CONST
{
	return m_Instance;
}

INT Program::RunMessageLoop(VOID)
{
	MSG Message = { NULL };
	HACCEL Accelerator = LoadAccelerators(m_Instance, MAKEINTRESOURCE(ACCELERATOR_ZENPP));

	try {
		// Validate accelerator table was loaded
		if (!Accelerator)
			throw std::wstring(L"An error occured while initializing keyboard shortcuts.  Please restart the application or contact support for assistance.");

		// Begin message loop
		while (GetMessage(&Message, nullptr, 0, 0) != 0) {
			if (!TranslateAccelerator(m_MainDialog->GetHwnd(), Accelerator, &Message)) {
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}

		// Free the accelerator table
		DestroyAcceleratorTable(Accelerator);

	} catch (CONST std::wstring& CustomMessage) {
		DisplayError(CustomMessage);
	}

	return static_cast<INT>(Message.wParam);

}

VOID Program::QuitProgram(VOID)
{
	m_Quitting = TRUE;
	PostQuitMessage(0);
}

VOID Program::CreateFirmwareDialog(CONST FirmwareManager::FirmwareModificationPurpose Purpose)
{
	try {
		// Check if dialog already exists and display it, if so; otherwise create and display it
		if (m_FirmwareDialog.get()) {
			m_FirmwareDialog->BringToForeground();
		} else {
			m_FirmwareDialog = std::make_unique<FirmwareDialog>(Purpose);
			m_FirmwareDialog->Create(DIALOG_FIRMWARE);
		}
	}
	catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Unable to create the FirmwareDialog object; insufficient memory is available to complete the required operation.");
	}
}

VOID Program::DestroyAboutDialog(VOID)
{
	m_AboutDialog.reset();
}

VOID Program::DestroyFirmwareDialog(VOID)
{
	m_FirmwareDialog.reset();
}

VOID Program::CheckAdministrator(VOID)
{
	// Query administrator privileges
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;

	// Allocate and initialize a SID of the administrators group
	if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
		// Check if the calling process is a member of the administrators group
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &m_Administrator))
			m_Administrator = FALSE;

		// Free the SID
		FreeSid(AdministratorsGroup);
	}
}

VOID Program::DisplayError(CONST std::wstring& AdditionalMessage)
{
	LPVOID ErrorBuffer = nullptr;
	DWORD ErrorCode = GetLastError();
	DWORD ErrorBufferSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, ErrorCode, 0, (LPWSTR)&ErrorBuffer, 0, nullptr);
	BOOL HasSystemMessage = ((ErrorCode != ERROR_SUCCESS) && (ErrorCode != ERROR_IO_PENDING)) && ErrorBuffer;
	std::wstring ErrorMessage = AdditionalMessage;
	std::wstring SystemError;

	// Check if there is a system error message
	if (HasSystemMessage)
		SystemError = L"System: " + std::wstring((LPCWSTR)ErrorBuffer);

	// Determine whether to provide a message box or output to the main window
	if (m_MainDialog == nullptr) {
		// Append system error (if applicable)
		if (HasSystemMessage) {
			// Verify an additional system message exists and if so, append it
			if (AdditionalMessage.size())
				ErrorMessage += L"\r\n\r\n";

			// Append the system message whether blank or not
			ErrorMessage += SystemError;
		}

		// Alert error
		MessageBox(NULL, ErrorMessage.c_str(), L"Zen++ Error", MB_ICONERROR | MB_OK);

	} else {
		// Display error
		if (ErrorMessage.size()) {
			m_MainDialog->PrintTimestamp();
			m_MainDialog->PrintText(RED, L"Error: %ws\r\n", ErrorMessage.c_str());
		}

		// Display system error (if applicable)
		if (HasSystemMessage) {
			m_MainDialog->PrintTimestamp();
			m_MainDialog->PrintText(RED, L"%ws", SystemError.c_str());
		}
	}

	// Free memory
	if (ErrorBufferSize && (ErrorBuffer != nullptr))
		LocalFree(ErrorBuffer);
}

// Method for notifying the user upon failure to create the a directory
VOID Program::HandleCreateDirectoryError(CONST std::wstring& Directory)
{
	// Retrieve error code from the failed operation
	DWORD ErrorCode = GetLastError();

	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();	// Handle various errors based on the error code

	switch (ErrorCode) {
	case ERROR_ALREADY_EXISTS:
		// Folder already exists, no action needed

		break;

	case ERROR_ACCESS_DENIED:
		// Permission issues:
		// - Inform user about insufficient permissions
		// - Suggest moving the application out of restricted locations (i.e. Downloads folder)
		// - Provide support information
		// - Throw an empty string to provide the system error message
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(ORANGE, L"Unable to create the \"%ws\" folder in this location.  The application does not have the necessary permissions to perform this operation in this location.\r\n", Directory.c_str());
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(ORANGE, L"Please make sure you have removed this application from your Downloads folder and placed it somewhere else such as its own folder on your Desktop.\r\n");
		MainDialog.DisplaySupportInfo();
		throw std::wstring(L"");

		break;

	case ERROR_ELEVATION_REQUIRED:
		// Administrative privileges required:
		// - Inform the user about the need for administrative privileges
		// - Display the administrator status notification
		// - Provide support information
		// - Throw an empty string to provide the system error message
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(ORANGE, L"Creating the \"%ws\" folder in this location requires administrative privileges.  Please run this application as an administrator and try again.\r\n", Directory.c_str());
		MainDialog.DisplayAdministratorStatus();
		MainDialog.DisplaySupportInfo();
		throw std::wstring(L"");

		break;

	default:
		// Handle any other error that may arrise
		throw std::wstring(L"An error occured while creating the " + Directory + L" folder.");
	}
}

// Method used for reading the user's configuration for the application from the Windows Registry
VOID Program::ReadUserVariables(VOID)
{
	m_24HourTimestamps = m_Registry->GetDword(L"Display 24-hour Timestamps", 0);
	m_DisplayVMSpeed = m_Registry->GetDword(L"Display VM Speed", 1);
	m_BlackBackground= m_Registry->GetDword(L"Black Background", 1);
	m_ConnectOnStartup = m_Registry->GetDword(L"Connect on Startup", 1);
	m_LowPerformanceMode = m_Registry->GetDword(L"Low Performance Mode", 0);
}

VOID Program::ShowAboutDialog(VOID)
{
	try {
		// Check if dialog already exists and display it, if so; otherwise create and display it
		if (m_AboutDialog.get()) {
			m_AboutDialog->BringToForeground();
		} else {
			m_AboutDialog = std::make_unique<AboutDialog>();
			m_AboutDialog->Create(DIALOG_ABOUT);
		}

	} catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Unable to create the AboutDialog object; insufficient memory is available to complete the required operation.");
	}
}