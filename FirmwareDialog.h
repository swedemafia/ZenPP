#ifndef DIALOGS_FIRMWAREDIALOG_H
#define DIALOGS_FIRMWAREDIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

class FirmwareDialog : public DialogBase
{
public:
	// Constructor for initialization
	explicit FirmwareDialog(_In_ CONST FirmwareManager::FirmwareModificationPurpose& Purpose);

	// Destructor for cleanup
	~FirmwareDialog() = default;

	// Public methods for UI interaction and state management
	HWND GetDescriptorEditHandle(VOID) CONST;
	VOID ChangeModificationStep(_In_ CONST BOOLEAN Forwards);
	VOID UpdateCapabilities(VOID);
	VOID UpdateProgressBar(_In_ CONST DWORD Value, _In_ CONST DWORD Maximum);

protected:
	// Protected callback method for central window message processing
	INT_PTR CALLBACK HandleMessage(_In_ CONST UINT Message, _In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);

private:
	// Private members for firmware management and UI elements
	FirmwareManager::FirmwareModificationPurpose m_Purpose = FirmwareManager::EraseFirmware;
	HWND m_ButtonBack = NULL;
	HWND m_ButtonCancel = NULL;
	HWND m_ButtonNext = NULL;
	HWND m_EditDescriptors = NULL;
	HWND m_LabelInstructions = NULL;
	HWND m_LabelStatus = NULL;
	HWND m_LabelStep = NULL;
	HWND m_PictureControl = NULL;
	HWND m_ProgressBar = NULL;
	std::unique_ptr<FirmwareManager> m_Manager;
	UINT m_Step = 0;

	// Private constant wide-string arrays for instructions and messages
	CONST std::wstring m_DeviceState[2] = {
		L"Please enter Bootloader mode on your Cronus Zen to proceed.",
		L"Press \"Next\" to begin the firmware modification."
	};
	CONST std::wstring m_Instructions[3] = {
		L"Connect your Cronus Zen to your computer from the CONSOLE/PC USB port.",
		L"Press and hold the small round blue button on the bottom of your Cronus Zen until the OLED screen displays \"Zen Bootloader.\"",
		L"Firmware modification in progress; please wait..."
	};
	CONST std::wstring m_WindowTitle[4] = {
		L"Erase Firmware",
		L"Install Compatible Firmware",
		L"Install Custom Firmware",
		L"Install Latest Firmware"
	};

	// Private methods for handling specific commands
	INT_PTR OnCommandButtonFirmwareBack(VOID);
	INT_PTR OnCommandButtonFirmwareNext(VOID);

	// Private methods for handling specific window messages
	INT_PTR OnClose(VOID);
	INT_PTR OnCommand(_In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);
	INT_PTR OnDestroy(VOID);
	INT_PTR OnDeviceChange(_In_ CONST WPARAM wParam);
	INT_PTR OnInitDialog(VOID);

	// Private method for updating firmware management instructions
	VOID UpdateInstructions(VOID);
};

#endif
