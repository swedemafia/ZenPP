#ifndef UTILITIES_FILE_H
#define UTILITIES_FILE_H

#ifdef _MSC_VER
#pragma once
#endif

class File
{
public:
	// Constructor for initialization
	explicit File(_In_ CONST std::wstring& FileName);
	explicit File(_In_ CONST std::wstring& FileName, _In_ CONST DWORD DesiredAccess, _In_ CONST DWORD ShareMode, _In_ CONST DWORD CreationDisposition, _In_ CONST DWORD Flags, _In_ CONST BOOL QueryFileSize);
	
	// Destructor for cleanup
	~File();

	// Public methods for file operations
	CONST BOOL Exists(VOID) CONST;
	CONST BOOL Read(_Out_writes_bytes_ (NumberOfBytesToRead) LPVOID Buffer, _In_ CONST DWORD NumberOfBytesToRead, _Out_ LPDWORD NumberOfBytesRead, _In_opt_ LPOVERLAPPED Overlapped);
	CONST BOOL Open(VOID);
	CONST BOOL Open(_In_ CONST DWORD DesiredAccess, _In_ CONST DWORD ShareMode, _In_ CONST DWORD CreationDisposition, _In_ CONST DWORD Flags, _In_ CONST BOOL QueryFileSize);
	CONST BOOL Open(_In_ CONST std::wstring& FileName, _In_ CONST DWORD DesiredAccess, _In_ CONST DWORD ShareMode, _In_ CONST DWORD CreationDisposition, _In_ CONST DWORD Flags, _In_ CONST BOOL QueryFileSize);
	CONST BOOL Write(_In_reads_bytes_ (NumberOfBytesToWrite) LPCVOID Buffer, _In_ CONST DWORD NumberOfBytesToWrite, _Out_opt_ LPDWORD NumberOfBytesWritten, _In_opt_ LPOVERLAPPED Overlapped);
	VOID Close(VOID);

	// Public methods for file information retrieval
	CONST HANDLE GetHandle(VOID) CONST;
	CONST LONGLONG GetFileSize(VOID) CONST;
	VOID GetFileData(std::shared_ptr<UCHAR[]>& Buffer);

private:
	// Private members for file state and information
	std::wstring m_FileName;
	BOOL m_QueryFileSize = FALSE;
	DWORD m_CreationDisposition = 0;
	DWORD m_DesiredAccess = 0;
	DWORD m_Flags = 0;
	DWORD m_ShareMode = 0;
	HANDLE m_Handle = INVALID_HANDLE_VALUE;
	LARGE_INTEGER m_FileSize = { NULL };
};

#endif
