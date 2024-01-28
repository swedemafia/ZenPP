#include "Precomp.h"

CONST std::wstring MkFile::GetName(VOID) CONST
{
	return App->AnsiToUnicode((PCHAR)m_LayoutValues.Header.Name);
}

CONST std::wstring MkFile::GetRevision(VOID) CONST
{
	return App->AnsiToUnicode((PCHAR)m_LayoutValues.Header.Revision);
}

CONST std::wstring MkFile::GetType(VOID) CONST
{
	return App->AnsiToUnicode((PCHAR)m_LayoutValues.Header.Type);
}

CONST std::wstring MkFile::GetVersion(VOID) CONST
{
	return App->AnsiToUnicode((PCHAR)m_LayoutValues.Header.Version);
}

// Method for notifying the user upon failure to create the MkFile directory
VOID MkFile::HandleCreateDirectoryError(VOID)
{
	// Retrieve error code from the failed operation
	DWORD ErrorCode = GetLastError();

	// Handle various errors based on the error code
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
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(ORANGE, L"Unable to create the MkFile and its folder in this location.  The application does not have the necessary permissions to perform this operation in this location.\r\n");
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(ORANGE, L"Please make sure you have removed this application from your Downloads folder and placed it somewhere else such as its own folder on your Desktop.\r\n");
		App->GetMainDialog().DisplaySupportInfo();
		throw std::wstring(L"");
		break;
	case ERROR_ELEVATION_REQUIRED:
		// Administrative privileges required:
		// - Inform the user about the need for administrative privileges
		// - Display the administrator status notification
		// - Provide support information
		// - Throw an empty string to provide the system error message
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(ORANGE, L"Creating the MkFile and its folder in this location requires administrative privileges.  Please run this application as an administrator and try again.\r\n");
		App->GetMainDialog().DisplayAdministratorStatus();
		App->GetMainDialog().DisplaySupportInfo();
		throw std::wstring(L"");
		break;
	default:
		// Handle any other error that may arrise
		throw std::wstring(L"An error occured while creating the MkFile folder.");
	}
}

// Method for processing the MkFile data while creating the directory and saving the extracted file data
VOID MkFile::SetMkFileData(CONST PUCHAR FileData, CONST std::size_t FileSize)
{
	// Attempt to process MkFile data and handle any potential errors
	try
	{
		// Create a ParseBuffer object to handle the parsing of the provided file data
		m_ParseBuffer = std::make_unique<ParseBuffer>(FileData, FileSize);

		// Extract LayoutValues from the ParseBuffer
		m_ParseBuffer->ExtractData(&m_LayoutValues, sizeof(LayoutValuesT));

		// Construct our File object and provide the path for the MkFile
		m_File = std::make_unique<File>(L"MkFile\\" + GetName() + L".zmk");

		// If the MkFile doesn't exist, create and save the data
		if (!m_File->Exists()) {
			// Notify the user about the attempted save
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(GRAY, L"Attempting to save mouse and keyboard settings...\r\n");

			// Create the "MkFile" directory and handle any access/elevation required errors
			if (!CreateDirectory(L"MkFile", NULL))
				HandleCreateDirectoryError();

			// Open the MkFile for creation and writing
			if(!m_File->Open(GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, FALSE))
				throw std::wstring(L"An error occured while creating the MkFile.");

			// Write ParseBuffer's contents to our newly created MkFile
			if (!m_File->Write(m_ParseBuffer->Buffer(), FileSize, NULL, NULL))
				throw std::wstring(L"An error occured while writing to the MkFile.");

			// Notify the user about a successful save
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(GREEN, L"Successfully extracted and saved mouse and keyboard settings to %ws.zmk!\r\n", GetName().c_str());
		}

	}
	catch (CONST std::bad_alloc&) {
		// Memory allocation errors
		App->DisplayError(L"Insufficient memory is available to handle the mouse and keyboard settings file.");
	}
	catch (CONST std::wstring& CustomMessage) {
		// Handle other errors
		App->DisplayError(CustomMessage);
	}

	// Close the file upon completion of this method
	m_File->Close();
}