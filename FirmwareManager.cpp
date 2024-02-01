#include "Precomp.h"

FirmwareManager::FirmwareManager(_In_ CONST FirmwareModificationPurpose& Modification) :
	DeviceLocatorBase(L"Cronus ZEN", L"USB"), m_Modification(Modification)
{
	SearchForDevice(); // Set initial device state
}

FirmwareManager::~FirmwareManager()
{
	CloseThread(m_EraseThreadHandle);
	CloseThread(m_InstallThreadHandle);
	WinUsbCloseConnection();
}

BOOL FirmwareManager::IsDeviceFound(VOID) CONST
{
	return m_DeviceFound;
}

BOOL FirmwareManager::IsModifying(VOID) CONST
{
	return m_Modifying;
}

BOOL FirmwareManager::SearchForDevice(VOID)
{
	return m_DeviceFound = FindDevice();
}

BOOL FirmwareManager::PrepareCompatibleFirmware(VOID)
{
	// Verify if the current firmware version requires being erased
	if (GetVersionInfo().GetMajor() > 2 || GetVersionInfo().GetMinor() > 1)
		return SpawnEraseThread();

	// Load the compatible firmware
	LoadFirmware(TRUE);

	// Return the creation status of the firmware installation thread
	return SpawnInstallThread();
}

BOOL FirmwareManager::SpawnEraseThread(VOID)
{
	if ((m_EraseThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EraseFirmwareThreadProc, (LPVOID)this, 0, 0)) == INVALID_HANDLE_VALUE) {
		App->DisplayError(L"An error occured while creating the EraseFirmware thread.");
		return FALSE;
	}

	return TRUE;
}

BOOL FirmwareManager::SpawnInstallThread(VOID)
{
	if ((m_InstallThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InstallFirmwareThreadProc, (LPVOID)this, 0, 0)) == INVALID_HANDLE_VALUE) {
		App->DisplayError(L"An error occured while creating the InstallFirmware thread.");
		return FALSE;
	}

	return TRUE;
}

BOOL FirmwareManager::UpdateCompleted(VOID) CONST
{
	return m_UpdateComplete;
}

DWORD FirmwareManager::EraseFirmwareThreadProc(LPVOID Parameter)
{
	// Cast the thread parameter to a usable data type
	FirmwareManager* Manager = reinterpret_cast<FirmwareManager*>(Parameter);

	try {
		WORD BytesProcessed = 0;
		UCHAR LockBlock[0x70] = { 0 };
		UCHAR Payload[0x420] = { 0 };

		// Step 1: clear error and abort
		if (Manager->DfuClearErrorAndAbort(DfuController::Locked).State != DfuController::Locked)
			throw std::wstring(L"An error occured while erasing the firmware, unexpected state (ClearErrorAndAbort).");

		// Get status and wait
		if (Manager->GetStatusAndWait().State != DfuController::Locked)
			throw std::wstring(L"An error occured while checking status; the device is not locked.");

		// Step 2: prepare unlock
		CopyMemory(&LockBlock, Manager->m_ResourceFile->GetFileData(), 0x70);
		Manager->DfuUnlock(LockBlock);

		// Get status and wait
		if (Manager->GetStatusAndWait().State != DfuController::Idle)
			throw std::wstring(L"An error occured while unlocking the device firmware.");

		// Update Progress bar
		BytesProcessed += 0x70;
		App->GetFirmwareDialog().UpdateProgressBar(BytesProcessed, 1168);

		// Step 4: prepare payload
		CopyMemory(&Payload, Manager->m_ResourceFile->GetFileData() + BytesProcessed, 0x420);
		Manager->DfuDownload(0, Payload, 0x420);
		
		// Get status and wait
		if (Manager->GetStatusAndWait().State != DfuController::DownloadIdle)
			throw std::wstring(L"An error occured while flashing the device.");

		// Update Progress bar
		BytesProcessed += 0x420;
		App->GetFirmwareDialog().UpdateProgressBar(BytesProcessed, 1168);

		// Step 5: send abort
		Manager->DfuAbort();

		// Notify user of success
		if(Manager->m_Modification == FirmwareModificationPurpose::EraseFirmware)
			MessageBox(App->GetFirmwareDialog().GetHwnd(), Manager->m_SuccessMessage[FirmwareModificationPurpose::EraseFirmware].c_str(), L"Zen++ Firmware Manager", MB_ICONINFORMATION | MB_OK);

	} catch (CONST std::wstring& CustomMessage) {
		MessageBox(App->GetFirmwareDialog().GetHwnd(), CustomMessage.c_str(), L"Zen++ Firmware Manager", MB_ICONERROR | MB_OK);
	}

	// Update the edit control containing the device descriptors
	Manager->UpdateDescriptors();

	// Determine the next steps of this process
	// - Spawn an install firmware thread, if necessary
	// - OR -
	// - Signal the end of the firmware modification
	if (Manager->m_Modification == FirmwareModificationPurpose::InstallCompatibleFirmware) {
		Manager->LoadFirmware(TRUE);
		Manager->SpawnInstallThread();
	} else {
		Manager->m_Modifying = FALSE;
	}

	return 0;
}

DWORD FirmwareManager::InstallFirmwareThreadProc(LPVOID Parameter)
{
	// Cast the thread parameter to a usable data type
	FirmwareManager* Manager = reinterpret_cast<FirmwareManager*>(Parameter);

	try {
		// Initialize variables required during the firmware download process
		DWORD BytesProcessed = 0;
		DWORD BytesTotal = 0;
		PBYTE FileData = 0;
		std::unique_ptr<UCHAR[]> CustomFileData;
		UCHAR LockBlock[0x70] = { 0 };
		UCHAR Payload[0x420] = { 0 };
		USHORT Block = 0;

		// Step 1: clear error and abort, if necessary
		if (Manager->DfuClearErrorAndAbort(DfuController::Locked).State != DfuController::Locked)
			throw std::wstring(L"An error occured while erasing the firmware, unexpected state (ClearErrorAndAbort).");

		// Get status and wait
		if (Manager->GetStatusAndWait().State != DfuController::Locked)
			throw std::wstring(L"An error occured while checking status; the device is not locked.");

		// Determine the source of the firmware file data
		// - Load the file data for a custom file
		// - OR -
		// - Load the file data from an embedded resource
		if (Manager->m_Modification == FirmwareModificationPurpose::InstallCustomFirmware) {
			BytesTotal = Manager->m_CustomFile->GetFileSize() & 0xffffffff;
			Manager->m_CustomFile->GetFileData(CustomFileData);
			FileData = CustomFileData.get();
		}
		else {
			BytesTotal = Manager->m_ResourceFile->GetFileSize();
			FileData = Manager->m_ResourceFile->GetFileData();
		}

		// Step 2: prepare unlock
		CopyMemory(&LockBlock, FileData, 0x70);
		Manager->DfuUnlock(LockBlock);

		// Get status and wait
		if (Manager->GetStatusAndWait().State != DfuController::Idle)
			throw std::wstring(L"An error occured while unlocking the firmware on the device.");

		// Update Progress bar
		BytesProcessed += 0x70;
		App->GetFirmwareDialog().UpdateProgressBar(BytesProcessed, BytesTotal);

		while (BytesProcessed + 0x420 <= BytesTotal)
		{
			// Step 3: prepare payload
			CopyMemory(&Payload, FileData + BytesProcessed, 0x420);
			Manager->DfuDownload(Block++, Payload, 0x420);

			// Get status and wait
			if (Manager->GetStatusAndWait().State != DfuController::DownloadIdle)
				throw std::wstring(L"An error occured while flashing firmware to the device.");

			// Update Progress bar
			BytesProcessed += 0x420;
			App->GetFirmwareDialog().UpdateProgressBar(BytesProcessed, BytesTotal);
		}

		// Step 4: lock firmware
		Manager->DfuDownload(0, nullptr, 0);

		// Get status and wait
		if (Manager->GetStatusAndWait().State != DfuController::Locked)
			throw std::wstring(L"An error occured while locking the firmware.");

		// Step 5: update descriptors
		Manager->UpdateDescriptors();
		Manager->m_UpdateComplete = TRUE;

		// Step 6: load firmware
		Manager->DfuEnterUserApplication();

		// Notify user of success
		App->GetFirmwareDialog().UpdateProgressBar(BytesTotal, BytesTotal);
		MessageBox(App->GetFirmwareDialog().GetHwnd(), Manager->m_SuccessMessage[Manager->m_Modification].c_str(), L"Zen++ Firmware Manager", MB_ICONINFORMATION | MB_OK);

	} catch (CONST std::wstring& CustomMessage) {
		MessageBox(App->GetFirmwareDialog().GetHwnd(), CustomMessage.c_str(), L"Zen++ Firmware Manager", MB_ICONERROR | MB_OK);
	}
	
	// Signal the completion of the firmware modification
	Manager->m_Modifying = FALSE;

	return 0;
}

SemanticVersion& FirmwareManager::GetVersionInfo(VOID) CONST
{
	return *m_SemanticVersion;
}

VOID FirmwareManager::CloseThread(HANDLE& ThreadHandle)
{
	if (ThreadHandle != INVALID_HANDLE_VALUE)
		CloseHandle(ThreadHandle);

	ThreadHandle = INVALID_HANDLE_VALUE;
}

DfuController::DfuStatus FirmwareManager::GetStatusAndWait(VOID)
{
	DfuStatus Status;

	do {
		Status = DfuGetStatus();
		Sleep(Status.PollTimeout);
	} while (Status.State == DfuController::DownloadBusy || Status.State == DfuController::UploadBusy || Status.State == DfuState::Manifest);

	return Status;
}

VOID FirmwareManager::LoadFirmware(_In_ CONST BOOL IgnoreModificationPurpose)
{
	try {
		if (m_Modification == FirmwareModificationPurpose::InstallCustomFirmware) {
			OPENFILENAME OpenFileName = { NULL };
			std::unique_ptr<WCHAR[]> FileName(new WCHAR[MAX_PATH]{ 0 });

			// Build our OPENFILENAME struct
			OpenFileName.lStructSize = sizeof(OPENFILENAME);
			OpenFileName.hwndOwner = App->GetFirmwareDialog().GetHwnd();
			OpenFileName.lpstrFile = FileName.get();
			OpenFileName.nMaxFile = MAX_PATH;
			OpenFileName.lpstrFilter = L"DFU Firmware (*.dfu)\0*.dfu\0";
			OpenFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;

			// Open the dialog and capture the file the user selects
			if (GetOpenFileName(&OpenFileName)) {
				// Allocate a File object used for reading from a custom firmware file
				m_CustomFile = std::make_unique<File>(OpenFileName.lpstrFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, TRUE);

				// Open the firmware file
				if (!m_CustomFile->Open())
					throw std::wstring(L"An error occured while opening the firmware file.");
			}
		} else {
			UINT ResourceID = FILE_FIRMWARE_LATEST;

			// Check for IMP flag to be set because installing compatible firmware first requires loading the latest firmware
			if (IgnoreModificationPurpose && m_Modification == InstallCompatibleFirmware)
				ResourceID = FILE_FIRMWARE_COMPATIBLE;

			// Load our resource file
			m_ResourceFile = std::make_unique<ResourceFile>(App->GetInstance(), ResourceID);
		}
	} catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	} catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Unable to read resource file; insufficient memory is available to complete the required operation.");
	}
}

BOOL FirmwareManager::PerformModification(VOID)
{
	if (WinUsbOpenConnection(GetDevicePath())) {
		UpdateDescriptors(); // Update descriptors
		LoadFirmware(FALSE); // Load firmware

		if ((m_ResourceFile && m_ResourceFile->GetFileSize()) || (m_CustomFile && m_CustomFile->GetFileSize())) {
			m_Modifying = TRUE; // Set modifying state

			switch (m_Modification) {
			case FirmwareManager::FirmwareModificationPurpose::EraseFirmware:				return SpawnEraseThread();
			case FirmwareManager::FirmwareModificationPurpose::InstallCompatibleFirmware:	return PrepareCompatibleFirmware();
			case FirmwareManager::FirmwareModificationPurpose::InstallCustomFirmware:		return SpawnInstallThread();
			case FirmwareManager::FirmwareModificationPurpose::InstallLatestFirmware:		return SpawnInstallThread();
			}
		}
	}

	return FALSE;
}

VOID FirmwareManager::UpdateDescriptors(VOID)
{
	// Query device descriptors
	GetDeviceDescriptors();

	// Prepare semantic version object information
	std::wstring_view WideStrVersion = GetFirmwareVersion();

	try {
		// Initialize a new SemanticVersion object
		m_SemanticVersion = std::make_unique<SemanticVersion>(App->UnicodeToAnsi(WideStrVersion.data()));

		// Build display string
		std::wstring DescriptorText =
			L"DEVICE INFORMATION:\r\n\r\nManufacturer: " + GetManufacturer() + L"\r\n" +
			L"Product Name: " + GetProduct() + L"\r\n" +
			L"Serial Number: " + GetSerialNumber() + L"\r\n" +
			L"Bootloader Version: " + GetBootloaderVersion() + L"\r\n" +
			L"Firmware Version: " + WideStrVersion.data();

		// Set the window caption for the device descriptor information
		SetWindowText(App->GetFirmwareDialog().GetDescriptorEditHandle(), DescriptorText.c_str());

	} catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	} catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Unable to create the SemanticVersion object; insufficient memory is available to complete the required operation.");
	}
}