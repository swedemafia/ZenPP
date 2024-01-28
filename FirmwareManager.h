#ifndef CLIENTS_FIRMWAREMANAGER_H
#define CLIENTS_FIRMWAREMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

class FirmwareManager : public DeviceLocatorBase, DfuController
{
public:
	// Enumeration for firmware modification purpose
	enum FirmwareModificationPurpose {
		EraseFirmware = 0,
		InstallCompatibleFirmware,
		InstallCustomFirmware,
		InstallLatestFirmware
	};

	// Constructor for initialization
	explicit FirmwareManager(_In_ CONST FirmwareModificationPurpose& Modification);

	// Destructor for cleanup
	~FirmwareManager();

	// Public methods for device and firmware management
	BOOL IsDeviceFound(VOID) CONST;
	BOOL IsModifying(VOID) CONST;
	BOOL SearchForDevice(VOID);
	BOOL UpdateCompleted(VOID) CONST;
	SemanticVersion& GetVersionInfo(VOID) CONST;
	BOOL PerformModification(VOID);
	VOID Upload(VOID);

protected:
	// Protected member for modification state
	BOOL m_Modifying = FALSE;

private:
	// Private members for firmware management, state and data
	BOOL m_DeviceFound = FALSE;
	BOOL m_UpdateComplete = FALSE;
	FirmwareModificationPurpose m_Modification = EraseFirmware;
	HANDLE m_EraseThreadHandle = INVALID_HANDLE_VALUE;
	HANDLE m_InstallThreadHandle = INVALID_HANDLE_VALUE;
	std::unique_ptr<File> m_CustomFile;
	std::unique_ptr<ResourceFile> m_ResourceFile;
	std::unique_ptr<SemanticVersion> m_SemanticVersion;

	// Private firmware management successful operation messages
	CONST std::wstring m_SuccessMessage[4] = {
		L"Successfully erased the firmware on your device!",
		L"Successfully installed fully compatible firmware on your device!",
		L"Successfully installed custom firmware on your device!",
		L"Successfully installed the latest firmware on your device!"
	};

	// Private methods for firmware modification tasks
	BOOL PrepareCompatibleFirmware(VOID);
	BOOL SpawnEraseThread(VOID);
	BOOL SpawnInstallThread(VOID);
	DfuStatus GetStatusAndWait(VOID);
	VOID CloseThread(_Inout_ HANDLE& ThreadHandle);
	VOID LoadFirmware(_In_ CONST BOOL IgnoreModificationPurpose);
	VOID UpdateDescriptors(VOID);

	// Static thread procedures for firmware management operations
	static DWORD EraseFirmwareThreadProc(LPVOID Parameter);
	static DWORD InstallFirmwareThreadProc(LPVOID Parameter);
};

#endif
