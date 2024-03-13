#ifndef UTILITIES_REGISTRY_H
#define UTILITIES_REGISTRY_H

#ifdef _MSC_VER
#pragma once
#endif

class Registry {
public:
	Registry(_In_ CONST std::wstring& RootPath) : m_RootPath(RootPath) { };

	// Publically accessible functions for managing registry values
	CONST DWORD GetDword(_In_ CONST std::wstring& ValueName, _In_ CONST DWORD DefaultValue);
	CONST std::wstring GetString(_In_ CONST std::wstring& ValueName, _In_ CONST std::wstring& DefaultValue);
	VOID CloseKey(_In_ CONST HKEY KeyHandle);
	VOID SetValue(_In_ CONST std::wstring& ValueName, _In_ CONST DWORD Value);
	VOID SetValue(_In_ CONST std::wstring& ValueName, _In_ CONST std::wstring& Value);

private:
	std::wstring m_RootPath;
};

#endif
