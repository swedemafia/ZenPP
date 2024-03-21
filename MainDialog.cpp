#include "Precomp.h"

// Method for inserting an item to the listbox
VOID MainDialog::ListBoxAdd(CONST ListBoxItem* Item)
{
	// Append to end of the list
	SendMessage(m_hWndSlotsListBox, LB_INSERTSTRING, -1, (LPARAM)Item);
}

// Method for clearing the listbox and deleting all associated memory for each listbox item
VOID MainDialog::ListBoxClear(VOID)
{
	// Iterate though each item of the listbox and delete it's data
	for (int i = 0; i < SendMessage(m_hWndSlotsListBox, LB_GETCOUNT, 0, 0); i++) {
		// Get the listbox item data
		ListBoxItem* ItemData = reinterpret_cast<ListBoxItem*>(SendMessage(m_hWndSlotsListBox, LB_GETITEMDATA, i, 0));

		// Validate a pointer to the object was retreived and delete it
		if(ItemData != reinterpret_cast<ListBoxItem*>(LB_ERR))
			delete ItemData;
	}

	// Reset the listbox content
	SendMessage(m_hWndSlotsListBox, LB_RESETCONTENT, 0, 0);
}

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
	SetTitle(L"Zen++ Copyright © 2023-2024 Swedemafia (Version " + std::to_wstring(VERSION_MAJOR) + L"." + std::to_wstring(VERSION_MINOR) + L"." + std::to_wstring(VERSION_REVISION) + L")");

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
	for (INT i = 0; i < MenuItemCount; ++i) {
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
	case CronusZen::PlayStation5:
		CheckMenuItem(m_Menu, MENU_DEVICE_EOP_PLAYSTATION4, Checked);
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
	for (unsigned i = BUTTON_MAIN_ADDSCRIPT; i <= BUTTON_MAIN_SOFTRESET; i++)
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
		SlotsString = std::to_wstring(SlotsUsed) + L" slot" + ((SlotsUsed > 1) ? L"s" : L"");
		SlotsString += L" " + std::to_wstring(BytesUsed) + L" used, " + std::to_wstring(262136 - BytesUsed) + L" free";
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
	case WM_COPYDATA:								return Dialog->OnCopyData(lParam);
	case WM_CTLCOLORDLG:							return Dialog->OnCtlColorDlg(wParam);
	case WM_CTLCOLORLISTBOX:						return Dialog->OnCtlColorListBox(wParam);
	case WM_CTLCOLORSTATIC:							return Dialog->OnCtlColorStatic(wParam);
	case WM_DESTROY:								return Dialog->OnDestroy();
	case WM_DEVICECHANGE:							return Dialog->OnDeviceChange(wParam);
	case WM_DRAWITEM:								return Dialog->OnDrawItem(wParam, lParam);
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
		std::unique_ptr<ExitApiModeCommand> ExitApiMode = std::make_unique<ExitApiModeCommand>();
		Cronus.QueueCommand(1, *ExitApiMode);
	}

	// Destroy dialog and terminate the process
	DestroyWindow(m_hWnd);
	ExitProcess(0);

	return TRUE;
}

INT_PTR MainDialog::OnCommand(CONST WPARAM wParam, CONST LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	// Dialog Buttons
	case BUTTON_MAIN_ADDSCRIPT:						return OnCommandMainAddScript();
	case BUTTON_MAIN_REMOVESELECTION:				return OnCommandMainRemovedSelectedScripts();
	case BUTTON_MAIN_ERASEALLSCRIPTS:				return OnCommandMainEraseAllScripts();
	case BUTTON_MAIN_FACTORYRESET:					return OnCommandMainFactoryReset();
	case BUTTON_MAIN_PROGRAMDEVICE:					return OnCommandMainProgramDevice();
	case BUTTON_MAIN_SOFTRESET:						return OnCommandMainSoftReset();

	// 'Connection'
	case MENU_CONNECTION_CONNECTONSTARTUP:			return OnCommandConnectionConnectOnStartup();
	case MENU_CONNECTION_DISCONNECT:				return OnCommandConnectionDisconnect();
	case MENU_CONNECTION_LOWPERFORMANCEMODE:		return OnCommandConnectionLowPerformanceMode();
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

	// 'View'
	case MENU_VIEW_24HOURTIMESTAMPS:				return OnCommandViewDisplay24HourTimestamps();
	case MENU_VIEW_BLACKBACKGROUND:					return OnCommandViewBlackBackground();
	case MENU_VIEW_DISPLAYVMSPEED:					return OnCommandViewDisplayVMSpeed();
	case MENU_VIEW_SCRIPTFOLDER:					return OnCommandViewScriptFolder();
	}
	return FALSE;
}

// Method for processing an external command sent to the window
INT_PTR MainDialog::OnCopyData(CONST LPARAM lParam)
{
	PCOPYDATASTRUCT CopyData = reinterpret_cast<PCOPYDATASTRUCT>(lParam);

	if (CopyData == nullptr)
		return -1; // Invalid data was provided

	// Variables for ease of accessibility
	CONST CronusZen& Cronus = App->GetCronusZen();
	MainDialog& MainDialog = App->GetMainDialog();

	// 0 is SlotAdd command
	if (CopyData->dwData == 0) {
		std::wstring FilePath;

		// Allocate space for copying
		FilePath.resize(CopyData->cbData);

		// Copy the lpData to the filepath variable
		std::memcpy(FilePath.data(), CopyData->lpData, CopyData->cbData);

		// Determine if the Cronus is connected or not and also check for appropriate firmware
		if (Cronus.GetConnectionState() == CronusZen::Connected) {
			// Validate device is on compatible firmware
			if (Cronus.GetFirmwareVersion().IsBeta()) {
				// Check slot availability and attempt to add it to your Zen
				if (m_CronusZen.SlotsUsed() == 8) {
					return 3; // No slots available
				} else {
					// Notify user that a message has been received and being handled
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(TEAL, L"Adding script from external source: %ws.\r\n", FilePath.c_str());

					m_CronusZen.SlotsAdd(FilePath.c_str());
				}
			} else {
				return 2; // User is not on beta firmware
			}
		} else {
			return 1; // Cronus is not connected
		}
	}

	return 0; // Action was successful
}

// Method for processing the "Add Script" command button
INT_PTR MainDialog::OnCommandMainAddScript(VOID)
{
	if (m_CronusZen.SlotsUsed() == 8) {
		MessageBox(m_hWnd, L"You are unable to add more than 8 scripts to your Cronus Zen!\r\n\r\n- Select some scripts in the listbox.\r\n- Press \"Remove Selected Scripts.\"\r\n- Press \"Program Device.\"\r\n- Try adding a new script again.", L"Slots Manager", MB_ICONERROR | MB_OK);
	} else {
		m_CronusZen.SlotsAdd(L"");
	}

	return TRUE;
}

// Method for processing the "Remove Selected Scripts" command button
INT_PTR MainDialog::OnCommandMainRemovedSelectedScripts(VOID)
{
	// Loop through the listbox and remove selected items
	for (int i = 0; i < SendMessage(m_hWndSlotsListBox, LB_GETCOUNT, 0, 0); i++) {
		// Check if the current listbox item index is selected
		if (SendMessage(m_hWndSlotsListBox, LB_GETSEL, i, 0)) {
			// Cast and retreive listbox item data to usable data
			ListBoxItem* ItemData = reinterpret_cast<ListBoxItem*>(SendMessage(m_hWndSlotsListBox, LB_GETITEMDATA, i, 0));

			// Do not even bother if the item is already marked for deletion
			if (ItemData->IsBeingDeleted)
				return TRUE;

			// Build notification string
			std::wstring Confirmation = L"Slot #" + std::to_wstring(ItemData->Slot + 1) + L":\r\n\r\nAre you sure you want to mark " + std::wstring(ItemData->Title) + L" for deletion?";

			// Prompt user to ensure that they would like to proceed
			if (MessageBox(m_hWnd, Confirmation.c_str(), L"Slots Manager", MB_ICONQUESTION | MB_YESNO) == IDYES) {
				// If script is just added and not programmed, just delete it entirely
				if (ItemData->IsBeingProgrammed) {
					SendMessage(m_hWndSlotsListBox, LB_DELETESTRING, i, 0);
				} else {
					// Mark slot for deletion
					ItemData->IsBeingDeleted = TRUE;
					// Update item data in listbox
					SendMessage(m_hWndSlotsListBox, LB_SETITEMDATA, i, reinterpret_cast<LPARAM>(ItemData));
				}

				// Correct slot numbers when a slot is removed
				for (int j = ItemData->Slot + 1; j < SendMessage(m_hWndSlotsListBox, LB_GETCOUNT, 0, 0); j++) {
					ListBoxItem* CurrentItemData = reinterpret_cast<ListBoxItem*>(SendMessage(m_hWndSlotsListBox, LB_GETITEMDATA, j, 0));
					if (!CurrentItemData->IsBeingDeleted) {
						CurrentItemData->Slot--;
						SendMessage(m_hWndSlotsListBox, LB_SETITEMDATA, j, reinterpret_cast<LPARAM>(CurrentItemData));
					}
				}

				// Remove from internal slot programming list
				m_CronusZen.SlotsRemove(ItemData->Slot);
			}
		}
	}

	// Refresh the listbox
	InvalidateRect(m_hWndSlotsListBox, NULL, TRUE);

	return TRUE;
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

INT_PTR MainDialog::OnCommandMainProgramDevice(VOID)
{
	m_CronusZen.SlotsProgram();
	return TRUE;
}

INT_PTR MainDialog::OnCommandMainSoftReset(VOID)
{
	m_CronusZen.CreateWorkerThread(CronusZen::SoftReset);
	return TRUE;
}

INT_PTR MainDialog::OnCommandViewBlackBackground(VOID)
{
	BOOL BlackBackground = !App->GetBlackBackground();

	// Update value and save to Windows Registry
	App->SetBlackBackground(BlackBackground);

	// Update the menu item
	CheckMenuItem(m_Menu, MENU_VIEW_BLACKBACKGROUND, BlackBackground ? MF_CHECKED : MF_UNCHECKED);

	// Update the dialog
	SetBkColor(GetDC(m_hWnd), BlackBackground ? BLACK : GetSysColor(COLOR_BTNFACE));
	InvalidateRect(m_hWnd, NULL, TRUE);

	return TRUE;
}

INT_PTR MainDialog::OnCommandViewDisplay24HourTimestamps(VOID)
{
	BOOL Display24HourTimestamps = !App->GetDisplay24HourTimestamps();

	// Update value and save to Windows Registry
	App->SetDisplay24HourTimestamps(Display24HourTimestamps);

	// Update the menu item
	CheckMenuItem(m_Menu, MENU_VIEW_24HOURTIMESTAMPS, Display24HourTimestamps ? MF_CHECKED : MF_UNCHECKED);

	return TRUE;
}

INT_PTR MainDialog::OnCommandViewDisplayVMSpeed(VOID)
{
	BOOL DisplayVMSpeedUpdates = !App->GetDisplayVMSpeed();

	// Update value and save to Windows Registry
	App->SetDisplayVMSpeed(DisplayVMSpeedUpdates);

	// Update the menu item
	CheckMenuItem(m_Menu, MENU_VIEW_DISPLAYVMSPEED, DisplayVMSpeedUpdates ? MF_CHECKED : MF_UNCHECKED);

	return TRUE;
}

INT_PTR MainDialog::OnCommandViewScriptFolder(VOID)
{
	// Prepare SHELLEXECUTEINFO
	SHELLEXECUTEINFO ShellInfo = { 0 };

	// Build SHELLEXECUTE structure
	ShellInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShellInfo.fMask = SEE_MASK_ASYNCOK;
	ShellInfo.hwnd = m_hWnd;
	ShellInfo.lpVerb = L"open";
	ShellInfo.lpFile = L"SlotData";
	ShellInfo.nShow = SW_SHOWDEFAULT;

	if (!ShellExecuteEx(&ShellInfo))
		App->DisplayError(L"Unable to open the SlotData folder.");

	/*COPYDATASTRUCT cds = {NULL};
	std::wstring test = L"C:\\Programming\\pedro.bin";

	cds.dwData = 0;
	cds.lpData = test.data();
	cds.cbData = test.size() << 1;

	SendMessage(m_hWnd, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&cds));
	*/

	return TRUE;
}

INT_PTR MainDialog::OnCommandConnectionConnectOnStartup(VOID)
{
	BOOL ConnectOnStartup = !App->GetConnectOnStartup();

	// Update value and save to Windows Registry
	App->SetConnectOnStartup(ConnectOnStartup);

	// Update the menu item
	CheckMenuItem(m_Menu, MENU_CONNECTION_CONNECTONSTARTUP, ConnectOnStartup ? MF_CHECKED : MF_UNCHECKED);

	return TRUE;
}

INT_PTR MainDialog::OnCommandConnectionDisconnect(VOID)
{
	m_CronusZen.DisconnectFromDevice();
	m_CronusZen.SetConnectionState(CronusZen::Disconnected);
	return TRUE;
}

INT_PTR MainDialog::OnCommandConnectionLowPerformanceMode(VOID)
{
	BOOL LowPerformanceMode = !App->GetLowPerformanceMode();

	// Update value and save to Windows Registry
	App->SetLowPerformanceMode(LowPerformanceMode);

	// Update the menu item
	CheckMenuItem(m_Menu, MENU_CONNECTION_LOWPERFORMANCEMODE, LowPerformanceMode ? MF_CHECKED : MF_UNCHECKED);

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
	std::unique_ptr<ChangeSlotACommand> ChangeSlotA = std::make_unique<ChangeSlotACommand>();
	std::unique_ptr<ChangeSlotBCommand> ChangeSlotB = std::make_unique<ChangeSlotBCommand>();
	std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);

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

INT_PTR MainDialog::OnCommandFileNew(VOID)
{
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

INT_PTR MainDialog::OnCtlColorDlg(CONST WPARAM wParam)
{
	if (App->GetBlackBackground()) {
		SetBkColor(reinterpret_cast<HDC>(wParam), BLACK);
		return reinterpret_cast<INT_PTR>(GetStockObject(BLACK_BRUSH));
	} else {
		return static_cast<INT_PTR>(FALSE);
	}
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
	} else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
		// Notify that the device has disconnected
		m_CronusZen.DisconnectFromDevice();
	}

	return TRUE;
}

INT_PTR MainDialog::OnDrawItem(CONST WPARAM wParam, CONST LPARAM lParam)
{
	// Cast the WM_DRAWITEM lParam into usable data
	LPDRAWITEMSTRUCT DrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);

	if (DrawItemStruct && (DrawItemStruct->itemID != -1) && (DrawItemStruct->hwndItem == m_hWndSlotsListBox)) {
		// Cast the DRAWITEMSTRUCT itemData into usable data
		ListBoxItem* ListBoxItemData = reinterpret_cast<ListBoxItem*>(DrawItemStruct->itemData);

		// Determine the background color of the listbox item based on whether it is selected or not
		if (DrawItemStruct->itemState & ODS_SELECTED && (DrawItemStruct->itemAction & ODA_FOCUS)) {
			// Highlight if item is selected
			SetBkColor(DrawItemStruct->hDC, GetSysColor(COLOR_HIGHLIGHT));
		} else {
			// Do not highlight if item is not selected
			SetBkColor(DrawItemStruct->hDC, BLACK);
		}

		// Determine the text color
		if (ListBoxItemData->IsBeingDeleted) {
			// Red for being deleted
			SetTextColor(DrawItemStruct->hDC, RED);

		} else if (ListBoxItemData->IsBeingProgrammed) {
			// Light green for needing to be programmed
			SetTextColor(DrawItemStruct->hDC, LIGHTGREEN);

		} else if (ListBoxItemData->GamepackID != 0xffff) {
			// Light blue for gamepacks
			SetTextColor(DrawItemStruct->hDC, LIGHTBLUE);

		} else {
			// White for scripts
			SetTextColor(DrawItemStruct->hDC, WHITE);
		}

		// Build our listbox item string
		std::wstring SlotString = L"#" + std::to_wstring(ListBoxItemData->Slot + 1) + L" " + ListBoxItemData->Title;

		// Draw the script title
		TextOut(DrawItemStruct->hDC, DrawItemStruct->rcItem.left, DrawItemStruct->rcItem.top, SlotString.c_str(), SlotString.size());
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

	// Update slots manager information5
	UpdateSlotsData(0, 1);

	// Update menus based on user settings
	CheckMenuItem(m_Menu, MENU_CONNECTION_CONNECTONSTARTUP, App->GetConnectOnStartup() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(m_Menu, MENU_CONNECTION_LOWPERFORMANCEMODE, App->GetLowPerformanceMode() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(m_Menu, MENU_VIEW_24HOURTIMESTAMPS, App->GetDisplay24HourTimestamps() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(m_Menu, MENU_VIEW_BLACKBACKGROUND, App->GetBlackBackground() ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(m_Menu, MENU_VIEW_DISPLAYVMSPEED, App->GetDisplayVMSpeed() ? MF_CHECKED : MF_UNCHECKED);

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