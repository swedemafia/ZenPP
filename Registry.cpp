#include "Precomp.h"

// Method used to query a DWORD value from the Windows Registry
CONST DWORD Registry::GetDword(CONST std::wstring& ValueName, CONST DWORD DefaultValue)
{
	DWORD Data = DefaultValue, DataSize = sizeof(DWORD);
	HKEY KeyHandle = 0;

	try {
		// Attempt to open the key
		switch (RegOpenKeyEx(HKEY_CURRENT_USER, m_RootPath.c_str(), 0, KEY_ALL_ACCESS, &KeyHandle)) {
		case ERROR_FILE_NOT_FOUND:
			// This case handles the key not existing, therefore, return the default value
			return DefaultValue;

		case ERROR_SUCCESS:
			// This case handles the key existing, therefore, attempt to query the value (don't care about success or not)
			RegQueryValueEx(KeyHandle, ValueName.c_str(), 0, NULL, reinterpret_cast<LPBYTE>(&Data), &DataSize);

			break;

		default:
			throw std::wstring(L"An error occured while opening the key holding a numeric value to be read from the Windows Registry.");
		}

	} catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	}

	// Close handle to the key being read
	CloseKey(KeyHandle);

	return Data;
}

// Method used to query a string value from the Windows Registry
CONST std::wstring Registry::GetString(CONST std::wstring& ValueName, CONST std::wstring& DefaultValue)
{
	DWORD DataSize = 0;
	std::wstring Data = DefaultValue;
	HKEY KeyHandle = 0;

	try {
		// Attempt to open the key
		switch (RegOpenKeyEx(HKEY_CURRENT_USER, m_RootPath.c_str(), 0, KEY_ALL_ACCESS, &KeyHandle)) {
		case ERROR_FILE_NOT_FOUND:
			// This case handles the key not existing, therefore, return the default value
			return DefaultValue;

		case ERROR_SUCCESS:
			// This case handles the key existing, therefore, attempt to query the value
			RegQueryValueEx(KeyHandle, ValueName.c_str(), 0, NULL, NULL, &DataSize);

			if (DataSize > 0) {
				// Resize the string buffer to prepare for the data
				Data.resize(DataSize);

				// Attempt to query the actual data
				RegQueryValueEx(KeyHandle, ValueName.c_str(), 0, NULL, reinterpret_cast<LPBYTE>(&Data[0]), &DataSize);

				// Resize the string to remove null character
				Data.resize((DataSize >> 1) - 1);
			}

			break;

		default:
			throw std::wstring(L"An error occured while opening the key holding a string value to be read from the Windows Registry.");
		}

	} catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	}

	// Close handle to the key being read
	CloseKey(KeyHandle);

	return Data;
}


// Method used for closing the handle to the Registry data used in HKEY_CURRENT_USER
VOID Registry::CloseKey(CONST HKEY KeyHandle)
{
	if (RegCloseKey(KeyHandle) != ERROR_SUCCESS)
		App->DisplayError(L"An error occured while closing the handle to the Windows Registry.");
}

// Method used for storing a numeric DWORD value to the Windows Registry
VOID Registry::SetValue(CONST std::wstring& ValueName, CONST DWORD Value)
{
	HKEY KeyHandle = 0;

	try {
		// Attempt to create the registry key in case it does not exist
		if (RegCreateKeyEx(HKEY_CURRENT_USER, m_RootPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &KeyHandle, NULL) != ERROR_SUCCESS)
			throw std::wstring(L"An error occured while creating a numeric value in the Windows Registry.");

		// Attempt to write to the registry key
		if (RegSetValueEx(KeyHandle, ValueName.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&Value), static_cast<DWORD>(sizeof(DWORD))) != ERROR_SUCCESS)
			throw std::wstring(L"An error occured while setting a numeric value in the Windows Registry.");

	}
	catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	}

	// Close the key that was just written to
	if (KeyHandle)
		CloseKey(KeyHandle);
}

// Method used for saving a Unicode string to the Windows Registry
VOID Registry::SetValue(CONST std::wstring& ValueName, CONST std::wstring& Value)
{
	HKEY KeyHandle = 0;

	try {
		// Attempt to create the registry key in case it does not exist
		if (RegCreateKeyEx(HKEY_CURRENT_USER, m_RootPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &KeyHandle, NULL) != ERROR_SUCCESS)
			throw std::wstring(L"An error occured while creating a string value in the Windows Registry.");

		// Attempt to write to the registry key
		if (RegSetValueEx(KeyHandle, ValueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(Value.c_str()), static_cast<DWORD>(Value.size() << 1)) != ERROR_SUCCESS)
			throw std::wstring(L"An error occured while setting a string value in the Windows Registry.");

	} catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	}

	// Close the key that was just written to
	if (KeyHandle)
		CloseKey(KeyHandle);
}