#include "Precomp.h"

// Method for displaying information regarding administrative privileges
VOID MainDialog::DisplayAdministratorStatus(VOID)
{
	if (!App->IsAdministrator()) {
		PrintTimestamp();
		PrintText(PINK, L"This application may require administrator privileges to run effectively and perform all functions.\r\n");
		PrintTimestamp();
		PrintText(PINK, L"To launch with administrative privileges, close this application and right-click the application's shortcut or executable icon and select 'Run as administrator' from the popup context menu.\r\n");
		DisplaySupportInfo();
	}
}

// Method for displaying the startup greeting
VOID MainDialog::DisplayStartupInfo(VOID)
{
	// Set dialog caption
	SetTitle(L"Zen++ Copyright © 2023-2024 Swedemafia");

	// Display greeting
	PrintTimestamp();
	PrintText(TEAL, L"Zen++ Copyright © 2023-2024 Swedemafia - version %u.%u.%u.\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
	PrintTimestamp();
	PrintText(TEAL, L"An Asgard production (https://discord.gg/tGH7QxtPam).\r\n");
	PrintTimestamp();
#ifdef _DEBUG
	PrintText(TEAL, L"Private debug build compiled at %s on %s.\r\n", BUILD_TIME, BUILD_DATE);
#else
	PrintText(TEAL, L"Public release build compiled at %s on %s.\r\n", BUILD_TIME, BUILD_DATE);

#endif
}

// Method to disable information about contacting support/system administrator for various issues
VOID MainDialog::DisplaySupportInfo(VOID)
{
	PrintTimestamp();
	PrintText(PURPLE, L"If you need further assistance, please refer to the Discord server (https://discord.gg/tGH7QxtPam) or contact your system administrator.\r\n");
}

// Method used to recursively uncheck all menu items within a specific menu and its submenus
VOID MainDialog::UncheckAllMenuItems(CONST HMENU Menu)
{
	INT MenuItemCount = GetMenuItemCount(Menu);

	// Iterate through each menu item
	for (unsigned i = 0; i < MenuItemCount; ++i) {
		// Retrieve menu item state by position
		UINT MenuState = GetMenuState(Menu, i, MF_BYPOSITION);

		// Check menu state
		if (MenuState & MF_CHECKED) {
			// Uncheck the item
			CheckMenuItem(Menu, i, MF_BYPOSITION | MF_UNCHECKED);
		}

		// Check if a submenu exists
		HMENU SubMenu = GetSubMenu(Menu, i);

		// Recursively uncheck items in submenus
		if (SubMenu)
			UncheckAllMenuItems(SubMenu);
	}
}

// Method for updating the state of the 'Device' menu items
VOID MainDialog::UpdateDeviceMenu(_In_ CONST CronusZen::SettingsLayout& Settings)
{
	// Initialize constant used for checking a menu item
	CONST UINT Checked = MF_BYCOMMAND | MF_CHECKED;

	// Uncheck all menu items and start fresh
	UncheckAllMenuItems(GetSubMenu(m_Menu, 3));

	// Handle Ps4 Specialty
	if(Settings.Ps4Specialty)
		CheckMenuItem(m_Menu, MENU_DEVICE_PS4SPECIALTY, Checked);

	// Handle Remote Play
	if(Settings.RemotePlay)
		CheckMenuItem(m_Menu, MENU_DEVICE_REMOTEPLAY, Checked);

	// Handle emulator output protocol
	switch (Settings.OutputMode) {
	case CronusZen::Auto:
		CheckMenuItem(m_Menu, MENU_DEVICE_EOP_AUTO, Checked);
		break;
	case CronusZen::NintendoSwitch:
		CheckMenuItem(m_Menu, MENU_DEVICE_EOP_NINTENDOSWITCH, Checked);
		break;
	case CronusZen::PlayStation3:
		CheckMenuItem(m_Menu, MENU_DEVICE_EOP_PLAYSTATION3, Checked);
		break;
	case CronusZen::PlayStation4:
		CheckMenuItem(m_Menu, MENU_DEVICE_EOP_PLAYSTATION4, Checked);
		break;
	case CronusZen::PlayStation5:
		CheckMenuItem(m_Menu, MENU_DEVICE_EOP_PLAYSTATION5, Checked);
		break;
	case CronusZen::Xbox360:
		CheckMenuItem(m_Menu, MENU_DEVICE_EOP_PCMOBILEXBOX360, Checked);
		break;
	case CronusZen::XboxOne:
		CheckMenuItem(m_Menu, MENU_DEVICE_EOP_XBOXONEX, Checked);
		break;
	}

	// Handle operational mode
	switch (Settings.OperationalMode) {
	case CronusZen::WheelMode:
		CheckMenuItem(m_Menu, MENU_DEVICE_OM_WHEEL, Checked);
		break;
	case CronusZen::GamepadMode:
		CheckMenuItem(m_Menu, MENU_DEVICE_OM_NORMAL, Checked);
		break;
	case CronusZen::TournamentMode:
		CheckMenuItem(m_Menu, MENU_DEVICE_OM_TOURNAMENT, Checked);
		break;
	}

	// Handle remote slot change
	switch (Settings.RemoteSlot) {
	case CronusZen::Disabled:
		CheckMenuItem(m_Menu, MENU_DEVICE_RSC_DISABLED, Checked);
		break;
	case CronusZen::PS_Share:
		CheckMenuItem(m_Menu, MENU_DEVICE_RSC_PSSHARE, Checked);
		break;
	case CronusZen::PS_L3:
		CheckMenuItem(m_Menu, MENU_DEVICE_RSC_PSL3, Checked);
		break;
	}

	// Check for PlayStation 5 option disable
	EnableMenuItem(m_Menu, MENU_DEVICE_EOP_PLAYSTATION5, MF_BYCOMMAND | (m_CronusZen.GetFirmwareVersion().IsBeta() ? MF_DISABLED : MF_ENABLED));
}

// Method to enable/disable features based on the connection state
VOID MainDialog::UpdateFeatureAvailability(CONST BOOL Enabled)
{
	// Enable/disable the "Reconnect To Device" menu item
	EnableMenuItem(m_Menu, MENU_CONNECTION_RECONNECT, MF_BYCOMMAND | (Enabled ? MF_ENABLED : MF_DISABLED));

	// Enable/disable the entire "Device" menu
	EnableMenuItem(m_Menu, 3, MF_BYPOSITION | (Enabled ? MF_ENABLED : MF_DISABLED));
	
	// Refresh/redraw/force update to menu bar
	DrawMenuBar(m_hWnd);

	// Enable/disable buttons based on the connection state
	for (unsigned i = BUTTON_MAIN_ADDSCRIPT; i <= BUTTON_MAIN_PROGRAMDEVICE; i++)
		EnableWindow(GetDlgItem(m_hWnd, i), Enabled);
}

// Method to update the edit control caption above the listbox
VOID MainDialog::UpdateSlotsData(CONST UCHAR SlotsUsed, CONST UINT BytesUsed)
{
	// Initialize a string to hold the slot information
	std::wstring SlotsString = L"";

	// Handle scenarios based on slot usage and byte count
	if (!SlotsUsed) {
		if (BytesUsed) {
			// No slots in use but dummy value provided for bytes
			// - Indicates the device is not connected
			SlotsString = L"Not connected";
		} else {
			// No slots in use and no bytes used
			// - Indicates there's no slots data
			SlotsString = L"No slots configuration";
		}
	} else {
		// Format string with proper pluralization for slots and byte count
		SlotsString = std::to_wstring(SlotsUsed) + L" slot";
		if (SlotsUsed > 1) {
			SlotsString += L"s";
		}
		SlotsString += L" (" + std::to_wstring(BytesUsed) + L" bytes, " + std::to_wstring(262120 - BytesUsed) + L" free)";
	}

	// UIpdate the edit box caption with the formatted slots information
	SetWindowText(m_hWndSlotsTitle, SlotsString.c_str());
}

HMENU MainDialog::GetMenuHandle(VOID) CONST
{
	return m_Menu;
}

INT_PTR MainDialog::HandleMessage(CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam)
{
	MainDialog* Dialog = reinterpret_cast<MainDialog*>(GetWindowLongPtr(m_hWnd, GWLP_USERDATA));

	switch (Message) {
	case WM_CLOSE:									return Dialog->OnClose();
	case WM_COMMAND:								return Dialog->OnCommand(wParam, lParam);
	case WM_CTLCOLORLISTBOX:						return Dialog->OnCtlColorListBox(wParam);
	case WM_CTLCOLORSTATIC:							return Dialog->OnCtlColorStatic(wParam);
	case WM_DESTROY:								return Dialog->OnDestroy();
	case WM_DEVICECHANGE:							return Dialog->OnDeviceChange(wParam);
	case WM_INITDIALOG:								return Dialog->OnInitDialog();
	case WM_GETMINMAXINFO:							return Dialog->OnGetMinMaxInfo(lParam);
	case WM_NOTIFY:									return Dialog->OnNotify(lParam);
	case WM_SIZE:									return Dialog->OnSize(wParam, lParam);
	}

	return FALSE;
}

INT_PTR MainDialog::OnClose(VOID)
{
	// Initialize variable for ease of accessibility
	CronusZen& Cronus = App->GetCronusZen();

	// Exit API mode if connected
	if (Cronus.GetConnectionState() == CronusZen::Connected) {
		std::unique_ptr<ExitApiModeCommand> ExitApiMode(new ExitApiModeCommand);
		Cronus.QueueCommand(1, *ExitApiMode);
	}

	// Destroy dialog
	DestroyWindow(m_hWnd);

	return TRUE;
}

INT_PTR MainDialog::OnCommand(CONST WPARAM wParam, CONST LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case BUTTON_MAIN_ERASEALLSCRIPTS:				return OnCommandMainEraseAllScripts();
	case BUTTON_MAIN_FACTORYRESET:					return OnCommandMainFactoryReset();
	case BUTTON_MAIN_SOFTRESET:						return OnCommandMainSoftReset();

	// 'Connection'
	case MENU_CONNECTION_DISCONNECT:				return OnCommandConnectionDisconnect();
	case MENU_CONNECTION_RECONNECT:					return OnCommandConnectionReconnect();

	// 'Device'
	case MENU_DEVICE_CLEARBLUETOOTHDEVICES:			return OnCommandDeviceClearBluetoothDevices();
	case MENU_DEVICE_CYCLESLOTS:					return OnCommandDeviceCycleSlots();
	case MENU_DEVICE_REFRESHATTACHEDDEVICES:		return OnCommandDeviceRefreshAttachedDevices();
	case MENU_DEVICE_PS4SPECIALTY:					return OnCommandDevicePs4Specialty();
	case MENU_DEVICE_REMOTEPLAY:					return OnCommandDeviceRemotePlay();
	case MENU_DEVICE_TURNOFFCONTROLLER:				return OnCommandDeviceTurnOffController();

	// 'Device' -> 'Emulator Output Protocol'
	case MENU_DEVICE_EOP_AUTO:						return OnCommandDeviceEopAuto();
	case MENU_DEVICE_EOP_NINTENDOSWITCH:			return OnCommandDeviceEopNintendoSwitch();
	case MENU_DEVICE_EOP_PCMOBILEXBOX360:			return OnCommandDeviceEopPcMobileXbox360();
	case MENU_DEVICE_EOP_PLAYSTATION3:				return OnCommandDeviceEopPlayStation3();
	case MENU_DEVICE_EOP_PLAYSTATION4:				return OnCommandDeviceEopPlayStation4();
	case MENU_DEVICE_EOP_PLAYSTATION5:				return OnCommandDeviceEopPlayStation5();
	case MENU_DEVICE_EOP_XBOXONEX:					return OnCommandDeviceEopXboxOne();

	// 'Device' -> 'Operational Mode'
	case MENU_DEVICE_OM_NORMAL:						return OnCommandDeviceOmNormal();
	case MENU_DEVICE_OM_TOURNAMENT:					return OnCommandDeviceOmTournament();
	case MENU_DEVICE_OM_WHEEL:						return OnCommandDeviceOmWheel();

	// 'Device' -> 'Remote Slot Change'
	case MENU_DEVICE_RSC_DISABLED:					return OnCommandDeviceRscDisable();
	case MENU_DEVICE_RSC_PSL3:						return OnCommandDeviceRscPSL3();
	case MENU_DEVICE_RSC_PSSHARE:					return OnCommandDeviceRscPSShare();

	// 'File'
	case MENU_FILE_EXIT:							return OnCommandFileExit();

	// 'Firmware'
	case MENU_FIRMWARE_COMPATIBLE:					return OnCommandFirmwareCompatible();
	case MENU_FIRMWARE_CUSTOM:						return OnCommandFirmwareCustom();
	case MENU_FIRMWARE_ERASE:						return OnCommandFirmwareErase();
	case MENU_FIRMWARE_LATEST:						return OnCommandFirmwareLatest();

	// 'Help'
	case MENU_HELP_ABOUT:							return OnCommandHelpAbout();
	case MENU_HELP_NEWS:							return OnCommandHelpZenPPNews();
	}
	return FALSE;
}

INT_PTR MainDialog::OnCommandMainEraseAllScripts(VOID)
{
	m_CronusZen.CreateWorkerThread(CronusZen::DeviceCleanup);
	return TRUE;
}

INT_PTR MainDialog::OnCommandMainFactoryReset(VOID)
{
	m_CronusZen.CreateWorkerThread(CronusZen::FactoryReset);
	return TRUE;
}

INT_PTR MainDialog::OnCommandMainSoftReset(VOID)
{
	m_CronusZen.CreateWorkerThread(CronusZen::SoftReset);
	return TRUE;
}

INT_PTR MainDialog::OnCommandConnectionDisconnect(VOID)
{
	m_CronusZen.DisconnectFromDevice();
	m_CronusZen.SetConnectionState(CronusZen::Disconnected);
	return TRUE;
}

INT_PTR MainDialog::OnCommandConnectionReconnect(VOID)
{
	m_CronusZen.DisconnectFromDevice();
	m_CronusZen.ConnectToDevice();
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceClearBluetoothDevices(VOID)
{
	m_CronusZen.CreateWorkerThread(CronusZen::ClearBluetooth);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceCycleSlots(VOID)
{
	// Build required commands to change the running slot
	std::unique_ptr<ChangeSlotACommand> ChangeSlotA(new ChangeSlotACommand);
	std::unique_ptr<ChangeSlotBCommand> ChangeSlotB(new ChangeSlotBCommand);
	std::unique_ptr<StreamIoStatusCommand> StreamIoStatus(new StreamIoStatusCommand(CronusZen::Off));

	// Queue commands to be sent to the device
	m_CronusZen.QueueCommand(1, *StreamIoStatus);
	m_CronusZen.QueueCommand(1, *ChangeSlotA);
	m_CronusZen.QueueCommand(1, *StreamIoStatus);
	m_CronusZen.QueueCommand(1, *ChangeSlotB);
	StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(CronusZen::InputReport | CronusZen::OutputReport));
	m_CronusZen.QueueCommand(1, *StreamIoStatus);

	return TRUE;
}

INT_PTR MainDialog::OnCommandDevicePs4Specialty(VOID)
{
	m_CronusZen.CreateWorkerThread(CronusZen::TogglePs4Specialty);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceRemotePlay(VOID)
{
	m_CronusZen.CreateWorkerThread(CronusZen::ToggleRemotePlay);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceRefreshAttachedDevices(VOID)
{
	m_CronusZen.CreateWorkerThread(CronusZen::RefreshAttachedDevices);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceTurnOffController(VOID)
{
	m_CronusZen.CreateWorkerThread(CronusZen::TurnOffController);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceEopAuto(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OutputMode, CronusZen::Auto);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceEopNintendoSwitch(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OutputMode, CronusZen::NintendoSwitch);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceEopPcMobileXbox360(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OutputMode, CronusZen::Xbox360);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceEopPlayStation3(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OutputMode, CronusZen::PlayStation3);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceEopPlayStation4(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OutputMode, CronusZen::PlayStation4);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceEopPlayStation5(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OutputMode, CronusZen::PlayStation5);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceEopXboxOne(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OutputMode, CronusZen::XboxOne);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceOmNormal(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OperationalMode, CronusZen::GamepadMode);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceOmTournament(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OperationalMode, CronusZen::TournamentMode);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceOmWheel(VOID)
{
	m_CronusZen.SetFragment(CronusZen::OperationalMode, CronusZen::WheelMode);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceRscDisable(VOID)
{
	m_CronusZen.SetFragment(CronusZen::RemoteSlot, CronusZen::Disabled);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceRscPSL3(VOID)
{
	m_CronusZen.SetFragment(CronusZen::RemoteSlot, CronusZen::PS_L3);
	return TRUE;
}

INT_PTR MainDialog::OnCommandDeviceRscPSShare(VOID)
{
	m_CronusZen.SetFragment(CronusZen::RemoteSlot, CronusZen::PS_Share);
	return TRUE;
}

INT_PTR MainDialog::OnCommandFileExit(VOID)
{
	OnClose();
	return TRUE;
}

INT_PTR MainDialog::OnCommandFirmwareCompatible(VOID)
{
	App->CreateFirmwareDialog(FirmwareManager::InstallCompatibleFirmware);
	return TRUE;
}

INT_PTR MainDialog::OnCommandFirmwareCustom(VOID)
{
	App->CreateFirmwareDialog(FirmwareManager::InstallCustomFirmware);
	return TRUE;
}

INT_PTR MainDialog::OnCommandFirmwareErase(VOID)
{
	App->CreateFirmwareDialog(FirmwareManager::EraseFirmware);
	return TRUE;
}

INT_PTR MainDialog::OnCommandFirmwareLatest(VOID)
{
	App->CreateFirmwareDialog(FirmwareManager::InstallLatestFirmware);
	return TRUE;
}

INT_PTR MainDialog::OnCommandHelpAbout(VOID)
{
	App->ShowAboutDialog();
	return TRUE;
}

INT_PTR MainDialog::OnCommandHelpZenPPNews(VOID)
{
	App->GetVersionCheck().DisplayNews();
	return TRUE;
}

INT_PTR MainDialog::OnCtlColorListBox(CONST WPARAM wParam)
{
	HDC ListBox = reinterpret_cast<HDC>(wParam);
	SetTextColor(ListBox, WHITE);
	SetBkColor(ListBox, BLACK);
	return reinterpret_cast<INT_PTR>(GetStockObject(BLACK_BRUSH));
}

INT_PTR MainDialog::OnCtlColorStatic(CONST WPARAM wParam)
{
	HDC Static = reinterpret_cast<HDC>(wParam);
	SetTextColor(Static, WHITE);
	SetBkColor(Static, BLACK);
	return reinterpret_cast<INT_PTR>(GetStockObject(BLACK_BRUSH));
}

INT_PTR MainDialog::OnDestroy(VOID)
{
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG)NULL);
	App->QuitProgram();
	return TRUE;
}

INT_PTR MainDialog::OnDeviceChange(CONST WPARAM wParam)
{
	// When a device arrives or is removed
	if (wParam == DBT_DEVICEARRIVAL) {
		// Attempt to connect when a device arrives to the system
		m_CronusZen.ConnectToDevice();
	}

	return TRUE;
}

INT_PTR MainDialog::OnInitDialog(VOID)
{
	// Get dialog control handles
	m_hWndButtonAddScript = GetDlgItem(m_hWnd, BUTTON_MAIN_ADDSCRIPT);
	m_hWndButtonRemoveSelection = GetDlgItem(m_hWnd, BUTTON_MAIN_REMOVESELECTION);
	m_hWndButtonEraseAllScripts = GetDlgItem(m_hWnd, BUTTON_MAIN_ERASEALLSCRIPTS);
	m_hWndButtonFactoryReset = GetDlgItem(m_hWnd, BUTTON_MAIN_FACTORYRESET);
	m_hWndButtonSoftReset = GetDlgItem(m_hWnd, BUTTON_MAIN_SOFTRESET);
	m_hWndButtonProgramDevice = GetDlgItem(m_hWnd, BUTTON_MAIN_PROGRAMDEVICE);
	m_hWndSlotsListBox = GetDlgItem(m_hWnd, LIST_MAIN_SLOTS);
	m_hWndSlotsTitle = GetDlgItem(m_hWnd, EDIT_MAIN_SLOTS);
	m_Menu = GetMenu(m_hWnd);

	// Register device change notifications
	RegisterDeviceNotifications(m_CronusZen.GetGUID());

	// Update slots manager information
	UpdateSlotsData(0, 1);

	return TRUE;
}

INT_PTR MainDialog::OnGetMinMaxInfo(CONST LPARAM lParam)
{
	LPMINMAXINFO MinMaxInfo = reinterpret_cast<LPMINMAXINFO>(lParam);
	MinMaxInfo->ptMinTrackSize.x = 600;
	MinMaxInfo->ptMinTrackSize.y = 400;
	return FALSE;
}

INT_PTR MainDialog::OnNotify(CONST LPARAM lParam)
{
	// Cast the notification data to a usable format
	LPNMHDR Notification = reinterpret_cast<LPNMHDR>(lParam);

	// Determine if the notification is a link click in the RichEdit window
	if ((Notification->hwndFrom == m_hWndRichEdit) && (Notification->code == EN_LINK)) {
		ENLINK* Link = (ENLINK*)lParam;

		// Specifically handle left mouse button clicks on links only
		if (Link->msg == WM_LBUTTONDOWN) {
			// Allocate memory to thold the URL based on the link's character range
			std::unique_ptr<WCHAR[]> Url(new WCHAR[Link->chrg.cpMax - Link->chrg.cpMin + 1]{ 0 });

			// Define a TEXTRANGE structure for retrieving the URL
			TEXTRANGE TextRange = { Link->chrg, Url.get() };

			try {
				// Attempt to retrieve the URL from the RichEdit control
				if (!SendMessage(m_hWndRichEdit, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&TextRange)))
					throw std::wstring(L"An error occured while trying to read the URL.");

				// Attempt to open the URL via ShellExecute
				if ((UINT)ShellExecute(m_hWnd, L"open", Url.get(), NULL, NULL, SW_SHOWNORMAL) <= 32)
					throw std::wstring(L"An error ocurred while trying to open the URL.");
			} catch (CONST std::wstring& CustomMessage) {
				// Provide any errors during the URL retrieval or execution
				App->DisplayError(CustomMessage);
			}
		}
	}

	return TRUE;
}

INT_PTR MainDialog::OnSize(CONST WPARAM wParam, CONST LPARAM lParam)
{
	// Initialize variables for dialog dimensions
	WORD DialogHeight = HIWORD(lParam);
	WORD DialogWidth = LOWORD(lParam);

	// Initialize variables for right panel coordinates
	WORD RightPanelXPos = DialogWidth - 298;
	WORD RightPanelWidth = 290;

	// Resize right side
	MoveWindow(m_hWndButtonAddScript, RightPanelXPos, DialogHeight - 240, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonRemoveSelection, RightPanelXPos, DialogHeight - 202, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonEraseAllScripts, RightPanelXPos, DialogHeight - 160, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonFactoryReset, RightPanelXPos, DialogHeight - 120, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonSoftReset, RightPanelXPos, DialogHeight - 82, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonProgramDevice, RightPanelXPos, DialogHeight - 40, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndSlotsListBox, RightPanelXPos, 44, RightPanelWidth, DialogHeight - 290, TRUE);
	MoveWindow(m_hWndSlotsTitle, RightPanelXPos, 9, RightPanelWidth, 28, TRUE);

	// Resize left side
	MoveWindow(m_hWndRichEdit, 8, 9, DialogWidth - 312, DialogHeight - 17, TRUE);

	// Enumerate through each of the controls on the dialog
	EnumChildWindows(m_hWnd, [](HWND hWnd, LPARAM lParam) -> BOOL {
		RECT ControlRect;
		GetClientRect(hWnd, &ControlRect);
		InvalidateRect(hWnd, &ControlRect, TRUE);
		return TRUE; // Return TRUE to continue enumeration
	}, NULL);

	return TRUE;
}