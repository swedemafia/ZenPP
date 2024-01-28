#include "Precomp.h"

File::File(CONST std::wstring& FileName) :
	m_FileName(FileName)
{

}

File::File(CONST std::wstring& FileName, CONST DWORD DesiredAccess, CONST DWORD ShareMode, CONST DWORD CreationDisposition, CONST DWORD Flags, CONST BOOL QueryFileSize) :
	m_FileName(FileName), m_DesiredAccess(DesiredAccess), m_ShareMode(ShareMode), m_CreationDisposition(CreationDisposition), m_Flags(Flags), m_QueryFileSize(QueryFileSize)
{

}

File::~File()
{
	Close();
}

CONST BOOL File::Exists(VOID) CONST
{
	DWORD FileAttributes = GetFileAttributes(m_FileName.c_str());

	return (FileAttributes != INVALID_FILE_ATTRIBUTES && !(FileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

CONST HANDLE File::GetHandle(VOID) CONST
{
	return m_Handle;
}

CONST LONGLONG File::GetFileSize(VOID) CONST
{
	return m_FileSize.QuadPart;
}

CONST BOOL File::Read(LPVOID Buffer, CONST DWORD NumberOfBytesToRead, LPDWORD NumberOfBytesRead, LPOVERLAPPED Overlapped)
{
	try
	{
		if (!ReadFile(m_Handle, Buffer, NumberOfBytesToRead, NumberOfBytesRead, Overlapped))
			throw std::wstring(L"An error occured while reading from the file.");
	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
		return FALSE;
	}

	return TRUE;
}

CONST BOOL File::Open(VOID)
{
	return Open(m_FileName, m_DesiredAccess, m_ShareMode, m_CreationDisposition, m_Flags, m_QueryFileSize);
}

CONST BOOL File::Open(CONST DWORD DesiredAccess, CONST DWORD ShareMode, CONST DWORD CreationDisposition, CONST DWORD Flags, CONST BOOL QueryFileSize)
{
	return Open(m_FileName, DesiredAccess, ShareMode, CreationDisposition, Flags, QueryFileSize);
}

CONST BOOL File::Open(CONST std::wstring& FileName, CONST DWORD DesiredAccess, CONST DWORD ShareMode, CONST DWORD CreationDisposition, CONST DWORD Flags, CONST BOOL QueryFileSize)
{
	try
	{
		if ((m_Handle = CreateFile(FileName.c_str(), DesiredAccess, ShareMode, NULL, CreationDisposition, Flags, NULL)) == INVALID_HANDLE_VALUE)
			throw std::wstring(L"An error occured opening " + FileName + L".");

		if (QueryFileSize && !GetFileSizeEx(m_Handle, &m_FileSize))
			throw std::wstring(L"An error occured while getting the file size.");
	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
		return FALSE;
	}

	return TRUE;
}

CONST BOOL File::Write(LPCVOID Buffer, CONST DWORD NumberOfBytesToWrite, LPDWORD NumberOfBytesWritten, LPOVERLAPPED Overlapped)
{
	try
	{
		if (!WriteFile(m_Handle, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, Overlapped))
			throw std::wstring(L"An error occured while writing to the file.");

		if (NumberOfBytesWritten && (NumberOfBytesToWrite != *NumberOfBytesWritten))
			throw std::wstring(L"An error occured while writing to the file (byte size mismatch).");
			
	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
		return FALSE;
	}

	return TRUE;
}

VOID File::GetFileData(std::unique_ptr<UCHAR[]>& Buffer)
{
	DWORD FileSize = GetFileSize() & 0xffffffff;
	if (!FileSize) {
		Buffer = nullptr; // Indicate empty file
		return;
	}

	Buffer = std::unique_ptr<UCHAR[]>(new UCHAR[FileSize]{ 0 });
	DWORD numberOfBytesRead = 0;

	if (!Read(Buffer.get(), FileSize, &numberOfBytesRead, NULL)) {
		App->DisplayError(L"An error occured while reading the file data.");
		Buffer = nullptr; // Clear buffer on error
		return;
	}
}

VOID File::Close(VOID)
{
	if (m_Handle != INVALID_HANDLE_VALUE) {
		CancelIo(m_Handle);
		CloseHandle(m_Handle);
		m_Handle = INVALID_HANDLE_VALUE;
	}
}