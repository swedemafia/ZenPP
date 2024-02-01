#ifndef DIALOGS_MAINDIALOG_H
#define DIALOGS_MAINDIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

class MainDialog : public DialogBase
{
public:
	// Destructor for cleanup
	~MainDialog() = default;

	// Public method for menu handle retrieval
	HMENU GetMenuHandle(VOID) CONST;

	// Public methods for displaying RichEdit messages
	VOID DisplayAdministratorStatus(VOID);
	VOID DisplayStartupInfo(VOID);
	VOID DisplaySupportInfo(VOID);

	// Public methods for device management
	VOID UpdateDeviceMenu(_In_ CONST CronusZen::SettingsLayout& Settings);
	VOID UpdateFeatureAvailability(_In_ CONST BOOL Enabled);
	VOID UpdateSlotsData(_In_ CONST UCHAR SlotsUsed, _In_ CONST UINT BytesUsed);

protected:
	// Protected callback method for central window message processing
	INT_PTR CALLBACK HandleMessage(_In_ CONST UINT Message, _In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);

private:
	// Private members for UI elements
	HMENU m_Menu = NULL;
	HWND m_hWndButtonAddScript = NULL;
	HWND m_hWndButtonRemoveSelection = NULL;
	HWND m_hWndButtonEraseAllScripts = NULL;
	HWND m_hWndButtonFactoryReset = NULL;
	HWND m_hWndButtonSoftReset = NULL;
	HWND m_hWndButtonProgramDevice = NULL;
	HWND m_hWndSlotsListBox = NULL;
	HWND m_hWndSlotsTitle = NULL;

	// Private methods for handling specific commands
	// Dialog buttons
	INT_PTR OnCommandMainEraseAllScripts(VOID);
	INT_PTR OnCommandMainFactoryReset(VOID);
	INT_PTR OnCommandMainSoftReset(VOID);

	// 'Connection' menu
	INT_PTR OnCommandConnectionDisconnect(VOID);
	INT_PTR OnCommandConnectionReconnect(VOID);

	// 'Device' menu
	INT_PTR OnCommandDeviceClearBluetoothDevices(VOID);
	INT_PTR OnCommandDeviceCycleSlots(VOID);
	INT_PTR OnCommandDevicePs4Specialty(VOID);
	INT_PTR OnCommandDeviceRefreshAttachedDevices(VOID);
	INT_PTR OnCommandDeviceRemotePlay(VOID);
	INT_PTR OnCommandDeviceTurnOffController(VOID);

	// 'Device' -> 'Emulator Output Protocol' submenu
	INT_PTR OnCommandDeviceEopAuto(VOID);
	INT_PTR OnCommandDeviceEopNintendoSwitch(VOID);
	INT_PTR OnCommandDeviceEopPcMobileXbox360(VOID);
	INT_PTR OnCommandDeviceEopPlayStation3(VOID);
	INT_PTR OnCommandDeviceEopPlayStation4(VOID);
	INT_PTR OnCommandDeviceEopPlayStation5(VOID);
	INT_PTR OnCommandDeviceEopXboxOne(VOID);

	// 'Device' -> 'Operational Mode' submenu
	INT_PTR OnCommandDeviceOmNormal(VOID);
	INT_PTR OnCommandDeviceOmTournament(VOID);
	INT_PTR OnCommandDeviceOmWheel(VOID);

	// 'Device' -> 'Remote Slot Change' submenu
	INT_PTR OnCommandDeviceRscDisable(VOID);
	INT_PTR OnCommandDeviceRscPSL3(VOID);
	INT_PTR OnCommandDeviceRscPSShare(VOID);

	// 'File' menu
	INT_PTR OnCommandFileExit(VOID);

	// 'Firmware' menu
	INT_PTR OnCommandFirmwareCompatible(VOID);
	INT_PTR OnCommandFirmwareCustom(VOID);
	INT_PTR OnCommandFirmwareErase(VOID);
	INT_PTR OnCommandFirmwareLatest(VOID);

	// 'Help' menu
	INT_PTR OnCommandHelpAbout(VOID);
	INT_PTR OnCommandHelpZenPPNews(VOID);

	// Private methods for handling specific window messages
	INT_PTR OnClose(VOID);
	INT_PTR OnCommand(_In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);
	INT_PTR OnCtlColorListBox(_In_ CONST WPARAM wParam);
	INT_PTR OnCtlColorStatic(_In_ CONST WPARAM wParam);
	INT_PTR OnDeviceChange(_In_ CONST WPARAM wParam);
	INT_PTR OnDestroy(VOID);
	INT_PTR OnGetMinMaxInfo(_In_ CONST LPARAM lParam);
	INT_PTR OnInitDialog(VOID);
	INT_PTR OnNotify(_In_ CONST LPARAM lParam);
	INT_PTR OnSize(_In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);

	// Private method for managing checkboxes on menus
	VOID UncheckAllMenuItems(CONST HMENU Menu);
};

#endif
