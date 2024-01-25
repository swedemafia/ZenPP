#include "Precomp.h"

ResourceFile::ResourceFile(CONST HINSTANCE Instance, CONST UINT ResourceID)
{
	HRSRC ResourceFile = NULL;

	try
	{
		if (m_LoadedResource)
			FreeResource(m_LoadedResource);

		if (!(ResourceFile = FindResource(Instance, MAKEINTRESOURCE(ResourceID), L"File")))
			throw std::wstring(L"An error occured while locating the resource file.");

		if (!(m_FileSize = SizeofResource(Instance, ResourceFile)))
			throw std::wstring(L"An error occured while retrieving the resource file size.");

		if (!(m_LoadedResource = LoadResource(Instance, ResourceFile)))
			throw std::wstring(L"An error occured while loading the resource file.");

		if (!(m_FileData = reinterpret_cast<PBYTE>(LockResource(m_LoadedResource))))
			throw std::wstring(L"An error occured while copying the resource file.");

	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
	}
}

ResourceFile::~ResourceFile()
{
	if (m_LoadedResource)
		FreeResource(m_LoadedResource);
}

CONST PBYTE ResourceFile::GetFileData(VOID) CONST
{
	return m_FileData;
}

CONST DWORD ResourceFile::GetFileSize(VOID) CONST
{
	return m_FileSize;
}
