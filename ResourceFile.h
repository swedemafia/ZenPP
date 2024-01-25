#ifndef UTILITIES_RESOURCEFILE_H
#define UTILITIES_RESOURCEFILE_H

#ifdef _MSC_VER
#pragma once
#endif

class ResourceFile {
public:
	// Constructor for initialization
	ResourceFile(_In_ CONST HINSTANCE Instance, _In_ CONST UINT ResourceID);

	// Destructor for cleanup
	~ResourceFile();

	// Public methods for resource file information retrieval
	CONST DWORD GetFileSize(VOID) CONST;
	CONST PBYTE GetFileData(VOID) CONST;

private:
	// Private members for resource file information
	DWORD m_FileSize = 0;
	HGLOBAL m_LoadedResource = NULL;
	PBYTE m_FileData = nullptr;
	UINT m_ResourceID = 0;
};

#endif
