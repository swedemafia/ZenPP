#include "Precomp.h"

CONST CronusZen::ConnectionState& CronusZen::GetConnectionState(VOID) CONST
{
	return m_ConnectionState;
}

CONST std::wstring CronusZen::GetDeviceDescription(_In_ CONST USHORT VendorID, _In_ CONST USHORT ProductID)
{
	switch (VendorID) {
	case 0x045e:
		// Microsoft
		switch (ProductID) {
		case 0x0202:	return L"Microsoft Xbox Controller";
		case 0x0285:	return L"Microsoft Xbox Controller S";
		case 0x0288:	return L"Microsoft Xbox Controller S Hub";
		case 0x0289:	return L"Microsoft Xbox Controller S";
		case 0x028e:	return L"Microsoft Xbox 360 Controller";
		case 0x028f:	return L"Microsoft Xbox 360 Wireless Controller via Plug & Charge Cable";
		case 0x02b6:	return L"Microsoft Xbox 360 Bluetooth Wireless Headset";
		case 0x02d1:	return L"Microsoft Xbox One Controller";
		case 0x02dd:	return L"Microsoft Xbox One Controller";
		case 0x02e0:	return L"Microsoft Xbox One Wireless Controller";
		case 0x02e3:	return L"Microsoft Xbox One Elite Controller";
		case 0x02e6:	return L"Microsoft Xbox One Wireless Adapter for Windows";
		case 0x02ea:	return L"Microsoft Xbox One Controller";
		case 0x02fd:	return L"Microsoft Xbox One S Controller";
		case 0x02ff:	return L"Microsoft Xbox One S Controller";
		case 0x0719:	return L"Microsoft Xbox 360 Wireless Adapter";
		case 0x0b00:	return L"Microsoft Xbox Elite Series 2 Controller";
		case 0x0b12:	return L"Microsoft Xbox Series X|S Controller";
		default:		return L"An unrecognized Microsoft device";
		}
	case 0x054c:
		// Sony
		switch (ProductID) {
		case 0x0268:	return L"Sony PlayStation 3 Controller";
		case 0x03d5:	return L"Sony PlayStation Move Motion Controller";
		case 0x042f:	return L"Sony PlayStation Move Navigation Controller";
		case 0x05c4:	return L"Sony DualShock 4 Controller";
		case 0x09cc:	return L"Sony DualShock 4 Controller";
		case 0x0ba0:	return L"Sony DualShock 4 Wireless Adapter";
		case 0x0cda:	return L"Sony PlayStation Classic Controller";
		case 0x0ce6:	return L"Sony DualSense 5 Controller";
		case 0x0df2:	return L"Sony DualSense 5 Edge Controller";
		case 0x0d5e:	return L"Sony PULSE 3D Headset";
		default:		return L"An unrecognized Sony device";
		}
	case 0x0f0d:
		//  Hori
	case 0x10f5:
		// Turtle Beach
		switch (ProductID) {
		case 0x0042:	return L"Turtle Beach Stealth 600X Gen 2 MAX Headset";
		case 0x2169:	return L"Turtle Beach Stealth 600 Headset";
		default:		return L"An unrecognized Turtle Beach device";
		}
	case 0x1532:
		// Razer
		switch (ProductID) {
		case 0x1000:	return L"Razer Raiju Gaming Controller";
		case 0x1004:	return L"Razer Raiju Ultimate Controller";
		case 0x1007:	return L"Razer Raiju 2 Tournament Edition Controller";
		case 0x1009:	return L"Razer Raiju 2 Ultimate Edition Controller";
		case 0x100A:	return L"Razer Raiju 2 Tournament Edition Controller";
		case 0x0a00:	return L"Razer Atrox Aracade Stick for Xbox One";
		case 0x0a03:	return L"Razer Wildcat";
		case 0x0a15:	return L"Razer Wolverine Tournament Edition Controller";
		default:		return L"An unrecognized Razer device";
		}
	case 0x2008:
		// Collective Minds
		switch (ProductID) {
		case 0x0010:	return L"Collective Minds Cronus Zen";
		default:		return L"An unrecognized Collective Minds device";
		}
	default:
		return L"An unrecognized device";
	}
}

CONST UCHAR CronusZen::GetBluetoothDeviceCount(VOID) CONST
{
	return m_BluetoothDevices;
}

CONST UCHAR CronusZen::GetWiredDeviceCount(VOID) CONST
{
	return m_WiredDevices;
}

VOID CronusZen::DestroyWorkerThread(VOID)
{
	if (m_WorkerThread != INVALID_HANDLE_VALUE) {
		TerminateThread(m_WorkerThread, 0);
		CloseHandle(m_WorkerThread);

		// Invalidate thread handle
		m_WorkerThread = INVALID_HANDLE_VALUE;
	}
}

VOID CronusZen::SetConnectionState(CONST ConnectionState& NewState)
{
	// Variable for ease of accessibility
	HMENU MainDialogMenu = App->GetMainDialog().GetMenuHandle();

	m_ConnectionState = NewState;

	if (m_ConnectionState == Disconnected) {
		EnableMenuItem(MainDialogMenu, MENU_CONNECTION_DISCONNECT, MF_BYCOMMAND | MF_DISABLED);
		EnableMenuItem(MainDialogMenu, MENU_CONNECTION_RECONNECT, MF_BYCOMMAND | MF_ENABLED);
		DestroyWorkerThread();
	} else {
		EnableMenuItem(MainDialogMenu, MENU_CONNECTION_DISCONNECT, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(MainDialogMenu, MENU_CONNECTION_RECONNECT, MF_BYCOMMAND | MF_ENABLED);
	}
}

VOID CronusZen::SetFragment(CONST FragmentIDList& Fragment, CONST UCHAR NewValue)
{
	// Variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	switch (Fragment) {
	case OutputMode:
		// Terminate the action if the mode is already set
		if (m_Settings.OutputMode == NewValue)
			return;

		// Alert user to the action being taken
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GRAY, L"Attempting to set output mode to %ws...\r\n", m_OutputMode[NewValue].c_str());

		break;
	case OperationalMode:
		// Terminate the action if the mode is already set
		if (m_Settings.OperationalMode == NewValue)
			return;

		// Alert user to the action being taken
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GRAY, L"Attempting to set operational mode to %ws...\r\n", m_OperationalMode[NewValue].c_str());

		break;
	case RemoteSlot:
		// Terminate the action if the mode is already set
		if (m_Settings.RemoteSlot == NewValue)
			return;

		// Alert user to the action being taken
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GRAY, L"Attempting to set remote slot change to %ws...\r\n", m_RemoteSlot[NewValue].c_str());

		break;
	}

	// Set target operational mode
	m_TargetMode = NewValue;

	// Signal we are updating the operational mode
	m_UpdateFragmentType = Fragment;

	// Create worker thread to handle the update request
	CreateWorkerThread(CronusZen::UpdateConfig);
}

VOID CronusZen::CreateWorkerThread(CONST ConnectionState& NewState)
{
	// Variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Verify that an operation is not already executing
	if (m_WorkerThread != INVALID_HANDLE_VALUE) {
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(RED, L"Please wait for the current operation to complete before attempting this action.\r\n");
		return;
	}

	// Update current connection state
	SetConnectionState(NewState);

	// Create the worker thread to handle the operation
	if ((m_WorkerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CronusWorkerThreadProc, (LPVOID)this, 0, NULL)) == INVALID_HANDLE_VALUE)
		App->DisplayError(L"An error occured while creating the worker thread.");
}

DWORD CronusZen::CronusWorkerThreadProc(LPVOID Parameter)
{
	// Cast the thread parameter to a CronusZen pointer
	CronusZen* Cronus = reinterpret_cast<CronusZen*>(Parameter);

	try {
		// Validate the parameter was correctly passed into the thread
		if (!Cronus)
			throw std::wstring(L"An error occured creating the worker thread proc.");

		// Initialize variables for ease of accessibility
		CONST ConnectionState& State = Cronus->GetConnectionState();
		MainDialog& MainDialog = App->GetMainDialog();

		// Handle device state specific actions
		if (State == ClearBluetooth && (MessageBox(MainDialog.GetHwnd(), L"Proceeding will clear all registered Bluetooth devices on your Cronus Zen.\r\n\r\nNOTICE: To ensure your Bluetooth device does not reconnect to your Cronus Zen, pair it with another host device (such as a gaming console or PC, etc.).\r\n\r\nDo you wish to proceed?", L"Clear Registered Bluetooth Devices", MB_ICONQUESTION | MB_YESNO) == IDYES)) {
			// This block handles clearing registered Bluetooth devices
			
			// Disable features on the main dialog
			MainDialog.UpdateFeatureAvailability(FALSE);

			// Notify user of the action
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Initiating Bluetooth device cleanup; please wait...\r\n");

			// Build commands required to perform a clearing of registered Bluetooth devices
			std::unique_ptr<ClearBluetoothCommand> ClearBluetooth = std::make_unique<ClearBluetoothCommand>();
			std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);
			std::unique_ptr<TurnOffControllerCommand> TurnOffController = std::make_unique<TurnOffControllerCommand>();

			// Queue commands to be sent to the device
			Cronus->QueueCommand(1, *StreamIoStatus);
			Cronus->QueueCommand(1, *StreamIoStatus);
			Cronus->QueueCommand(1, *TurnOffController);
			Cronus->QueueCommand(1, *StreamIoStatus);
			Cronus->QueueCommand(1, *ClearBluetooth);
			StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(InputReport | Mouse | Keyboard | Navcon));
			Cronus->QueueCommand(1, *StreamIoStatus);

		} else if (State == DeviceCleanup && (MessageBox(MainDialog.GetHwnd(), L"Proceeding will erase all memory slots on your Cronus Zen.\r\n\r\nNOTICE: This will erase all scripts and/or gamepacks that are programmed on your Cronus Zen.\r\n\r\nDo you wish to proceed?", L"Device Cleanup", MB_ICONQUESTION | MB_YESNO) == IDYES)) {
			// This block handles performing a device cleanup
			
			// Disable features on the main dialog
			MainDialog.UpdateFeatureAvailability(FALSE);

			// Notify user of the action
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Initiating cleanup of your device; please wait...\r\n");

			// Build commands required to perform a device cleanup
			std::unique_ptr<DeviceCleanupCommand> DeviceCleanup = std::make_unique<DeviceCleanupCommand>();
			std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);

			// Queue commands to be sent to the device with an 5-second delay between them
			Cronus->QueueCommand(1, *StreamIoStatus);
			Cronus->QueueCommand(1, *StreamIoStatus);
			Cronus->QueueCommand(1, *DeviceCleanup);
			Sleep(5500);
			StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(InputReport | OutputReport | Mouse | Keyboard | Navcon));
			Cronus->QueueCommand(1, *StreamIoStatus);

		} else if (State == FactoryReset && (MessageBox(MainDialog.GetHwnd(), L"Proceeding will perform a factory reset of your Cronus Zen.\r\n\r\nNOTICE: This will erase all scripts and/or gamepacks that are programmed on your Cronus Zen as well as resetting the device to factory defaults.\r\n\r\nDo you wish to proceed?", L"Factory Reset", MB_ICONQUESTION | MB_YESNO) == IDYES)) {
			// This block handles performing a factory reset
			
			// Disable features on the main dialog
			MainDialog.UpdateFeatureAvailability(FALSE);

			// Notify user of the action
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Initiating factory reset of your device; please wait...\r\n");

			// Build commands required to perform a factory reset
			std::unique_ptr<FactoryResetCommand> FactoryReset = std::make_unique<FactoryResetCommand>();
			std::unique_ptr<RequestMkFileCommand> RequestMkFile = std::make_unique<RequestMkFileCommand>();
			std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);

			// Queue commands to be sent to the device with an 8-second delay between them
			Cronus->QueueCommand(1, *StreamIoStatus);
			Cronus->QueueCommand(1, *StreamIoStatus);
			Cronus->QueueCommand(1, *FactoryReset);
			Sleep(8000);
			Cronus->QueueCommand(1, *StreamIoStatus);
			Cronus->QueueCommand(1, *RequestMkFile);

		}
		else if (State == RefreshAttachedDevices) {
			// This block handles refreshing attached devices

			// Disable features on the main dialog
			MainDialog.UpdateFeatureAvailability(FALSE);

			// Notify user of the action
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Requesting attached devices and Bluetooth connections...\r\n");

			Sleep(2000);

			// Ensure the features are available
			MainDialog.UpdateFeatureAvailability(TRUE);

			// Build commands required to request attached devices
			std::unique_ptr<RequestAttachedDevicesCommand> RequestAttachedDevices = std::make_unique<RequestAttachedDevicesCommand>();

			// Queue commands to be sent to the device
			Cronus->QueueCommand(1, *RequestAttachedDevices);

		} else if (State == ProgramDevice) {
			// This block handles programming the device
			
			// Disable features on the main dialog and reset slots manager caption
			MainDialog.UpdateSlotsData(0, 0);
			MainDialog.UpdateFeatureAvailability(FALSE);

			// Build commands required for programming to the device
			std::unique_ptr<DeviceCleanupCommand> DeviceCleanup = std::make_unique<DeviceCleanupCommand>();
			std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);

			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Initiating cleanup of your device; please wait...\r\n");

			// Queue commands to be sent to the device
			Cronus->QueueCommand(1, *StreamIoStatus);
			Cronus->QueueCommand(1, *DeviceCleanup);
			Sleep(5500);
			Cronus->SetConnectionState(ProgramDevice);

			// Begin process
			Cronus->FlashNextConfig();

		} else if (State == SoftReset) {
			// This block handles a soft reset of the device
			
			// Disable features on the main dialog
			MainDialog.UpdateFeatureAvailability(FALSE);

			// Notify user of the action
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Initiating soft reset of your device; please wait...\r\n");

			// Build commands required to soft reset
			std::unique_ptr<ResetDeviceCommand> ResetDevice = std::make_unique<ResetDeviceCommand>();
			std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);

			// Queue commands to be sent to the device with a 5-second delay between them
			Cronus->QueueCommand(1, *StreamIoStatus);
			Sleep(5000);
			Cronus->QueueCommand(1, *ResetDevice);


		} else if (State == ToggleRemotePlay) {
			// This block handles toggling PlayStation Remote Play
			
			// Disable features on the main dialog
			MainDialog.UpdateFeatureAvailability(FALSE);

			if (Cronus->m_Settings.RemotePlay) {
				// Notify user of the action
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(GRAY, L"Attempting to disable Remote Play...\r\n");

				// Build the fragment
				CONST CronusZen::FragmentData Fragment = { RemotePlay, Cfgs, FALSE, 0xff };

				// Build commands required to disable Remote Play
				std::unique_ptr<ResetDeviceCommand> ResetDevice = std::make_unique<ResetDeviceCommand>();
				std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);
				std::unique_ptr<SendSingleFragmentCommand> SendSingleFragment = std::make_unique<SendSingleFragmentCommand>(Fragment);

				// Queue commands to be sent to the device
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->QueueCommand(1, *SendSingleFragment);
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->SetFragment(OutputMode, Auto);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(InputReport | Mouse | Keyboard | Navcon));
				Cronus->QueueCommand(1, *StreamIoStatus);
				Sleep(5000);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>(Off);
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->QueueCommand(1, *ResetDevice);
			} else {
				// Notify user of the action
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(GRAY, L"Attempting to enable Remote Play...\r\n");

				// Build the fragment
				CONST CronusZen::FragmentData Fragment = { RemotePlay, Cfgs, TRUE, 0xff };

				// Build commands required to enable Remote Play
				std::unique_ptr<ResetDeviceCommand> ResetDevice = std::make_unique<ResetDeviceCommand>();
				std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);
				std::unique_ptr<SendSingleFragmentCommand> SendSingleFragment = std::make_unique<SendSingleFragmentCommand>(Fragment);

				// Queue commands to be sent to the device
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->QueueCommand(1, *SendSingleFragment);
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->SetFragment(OutputMode, PlayStation4);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(InputReport | Mouse | Keyboard | Navcon));
				Cronus->QueueCommand(1, *StreamIoStatus);
				Sleep(5000);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>(Off);
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->QueueCommand(1, *ResetDevice);
			}

		} else if (State == TogglePs4Specialty) {
			// This block handles toggling the PS4 Specialty feature
			
			// Disable features on the main dialog
			MainDialog.UpdateFeatureAvailability(FALSE);

			if (Cronus->m_Settings.Ps4Specialty) {
				// PS4 Specialty is currently enabled, so let us disable it

				// Notify user of the action
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(GRAY, L"Attempting to disable PS4 Specialty...\r\n");

				// Build the fragment
				CONST CronusZen::FragmentData Fragment = { Ps4Specialty, Cfgs, FALSE, 0xff };

				// Build commands required to disable PS4 Specialty
				std::unique_ptr<ResetDeviceCommand> ResetDevice = std::make_unique<ResetDeviceCommand>();
				std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);
				std::unique_ptr<SendSingleFragmentCommand> SendSingleFragment = std::make_unique<SendSingleFragmentCommand>(Fragment);

				// Queue commands to be sent to the device
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->QueueCommand(1, *SendSingleFragment);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(InputReport | Mouse | Keyboard | Navcon));
				Cronus->QueueCommand(1, *StreamIoStatus);
				Sleep(5000);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>(Off);
				Cronus->QueueCommand(1, *StreamIoStatus);
				Sleep(100);
				Cronus->QueueCommand(1, *ResetDevice);
			} else {
				// PS4 Specialty is currently disabled, so let us enable it

				// Notify user of the action
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(GRAY, L"Attempting to enable PS4 Specialty...\r\n");

				// Build the fragment
				CONST CronusZen::FragmentData Fragment = { Ps4Specialty, Cfgs, TRUE, 0xff };

				// Build commands required to disable PS4 Specialty
				std::unique_ptr<ResetDeviceCommand> ResetDevice = std::make_unique<ResetDeviceCommand>();
				std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);
				std::unique_ptr<SendSingleFragmentCommand> SendSingleFragment = std::make_unique<SendSingleFragmentCommand>(Fragment);

				// Queue commands to be sent to the device
				Cronus->QueueCommand(1, *StreamIoStatus);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(InputReport | Mouse | Keyboard | Navcon));
				Cronus->QueueCommand(1, *StreamIoStatus);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>(Off);
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->QueueCommand(1, *SendSingleFragment);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(InputReport | Mouse | Keyboard | Navcon));
				Cronus->QueueCommand(1, *StreamIoStatus);
				Sleep(5000);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>(Off);
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->QueueCommand(1, *ResetDevice);
			}

		} else if (State == TurnOffController) {
			// This block handles a turning off a connected wireless controller
			
			// Disable features on the main dialog
			MainDialog.UpdateFeatureAvailability(FALSE);

			// Check if a wireless controller is present
			if (Cronus->GetBluetoothDeviceCount()) {
				// Notify user of the action
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(GRAY, L"Requesting to turn off wireless controller; please wait...\r\n");

				// Build commands required to turn off the controller
				std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);
				std::unique_ptr<TurnOffControllerCommand> TurnOffController = std::make_unique<TurnOffControllerCommand>();

				// Queue commands to be sent to the device
				Cronus->QueueCommand(1, *StreamIoStatus);
				Cronus->QueueCommand(1, *TurnOffController);
				StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(InputReport | OutputReport));
				Cronus->QueueCommand(1, *StreamIoStatus);
			} else {
				// Inform the user that no Bluetooth devices are currently connected
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(ORANGE, L"There is no wireless controller connected to turn off.\r\n");

				// Reset availability of features on the main dialog
				MainDialog.UpdateFeatureAvailability(TRUE);
			}

		}
		else if (State == UpdateConfig) {
			// This block handles updating the device configuration
			
			// Disable features on the main dialog
			MainDialog.UpdateFeatureAvailability(FALSE);

			// Build the fragment
			CONST CronusZen::FragmentData Fragment = { Cronus->m_UpdateFragmentType, Cfgs, Cronus->m_TargetMode, 0xff };
			
			// Build command used to set the desired emulator output protocol 
			std::unique_ptr<SendSingleFragmentCommand> SendSingleFragment = std::make_unique<SendSingleFragmentCommand>(Fragment);
			std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>((CronusZen::StreamIoStatusMask)(InputReport | Mouse | Keyboard | Navcon));
			Cronus->QueueCommand(1, *SendSingleFragment);
			Cronus->QueueCommand(1, *StreamIoStatus);

			// Wait 2 seconds for the device to update
			Sleep(2000);

			// Check connection state
			if (Cronus->m_ConnectionState != Connected) {
				// Alert user of the result
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(ORANGE, L"Update device config may have failed; the device did not respond within the expected timeframe.\r\n");
			}
		}

	} catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Insufficient memory available to execute the worker thread.");
	} catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	}

	// Close out the worker thread
	CloseHandle(Cronus->m_WorkerThread);
	
	// Invalidate the thread handle
	Cronus->m_WorkerThread = INVALID_HANDLE_VALUE;

	return 0;
}

// Callback method for the HID device connected event
BOOL CronusZen::OnConnect(VOID)
{
	// Variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Notify user that the connection has been established
	MainDialog.PrintTimestamp();
	MainDialog.PrintText(GREEN, L"Successfully opened a connection to your Cronus Zen!\r\n");

	// Clear all settings
	memset(&m_Settings, 0, sizeof(SettingsLayout));

	// Clear last input report
	memset(&m_LastInputReport, 0, sizeof(InputReportData));

	// Set blank semantic version
	m_SemanticVersion = std::make_unique<SemanticVersion>("0.0.0");

	try {
		// Query HID device capabilities
		if (QueryHIDDeviceCapabilities()) {
			// Send initial communication command sequence
			SendInitialCommunication();
		} else {
			throw std::wstring(L"The connection to your device has been aborted.");
		}

		return TRUE; // Signal success
	} catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	}

	// Disconnect from the device
	DisconnectFromDevice();

	return FALSE; // Signal failure
}

BOOL CronusZen::OnDisconnect(VOID)
{
	DisconnectFromDevice();
	return TRUE;
}

SemanticVersion& CronusZen::GetFirmwareVersion(VOID) CONST
{
	return *m_SemanticVersion;
}

#include <iomanip>

std::wstring GetCharRepresentation(CONST BYTE ByteValue)
{
	if (std::isprint(ByteValue)) {
		// If the byte represents a printable character, return it as a wide character string
		wchar_t Character = static_cast<wchar_t>(ByteValue);

		if (Character == '%')
			return L"%%";
		else
			return std::wstring(1, Character);
	} else {
		// If the byte does not represent a printable character, return a period
		return L".";
	}
}

std::wstring GetHexRepresentation(CONST BYTE ByteValue)
{
	std::wstringstream HexStream;
	HexStream << std::setfill(L'0') << std::setw(2) << std::hex << static_cast<int>(ByteValue) << L' ';
	return HexStream.str();
}

VOID DumpHex(CONST PUCHAR PacketData, CONST USHORT PacketSize) {
	std::wstring TextToOutput = L"\r\n";

	for (unsigned j = 0; j < PacketSize; j += 16) {
		// Build hex representation
		for (unsigned i = 0; i < 16; i++)
			TextToOutput += (i + j < PacketSize) ? GetHexRepresentation(PacketData[i + j]) : L"   ";

		// Build ANSI representation
		for (unsigned i = 0; i < 16; i++)
			TextToOutput += (i + j < PacketSize) ? GetCharRepresentation(PacketData[i + j]) : L" ";

		// Format (add new line)
		TextToOutput += L"\r\n";
	}
	// Append new line
	TextToOutput += L"\r\n\0\0";

	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(GRAY, TextToOutput.c_str());
}

BOOL CronusZen::OnRead(CONST DWORD BytesRead)
{
	try {
		// Initialize variables for packet parsing
		UCHAR ID = 0;
		UCHAR Count = 0;
		USHORT TotalSize = 0;
		static USHORT BytesRemaining = 0;

		// Handle the start of a new packet
		if (!BytesRemaining) {
			// Extract packet information from the receive buffer
			ID = *(PUCHAR)&m_ReceiveBuffer[1];
			BytesRemaining = *(PUSHORT)&m_ReceiveBuffer[2]; // Total size of the payload excluding the packet headers
			Count = *(PUCHAR)&m_ReceiveBuffer[4]; // 1 signifies a single packet while 0 signifies multiple packets
			m_PayloadLength = BytesRemaining; // Set internal payload length used during command processing
			TotalSize = BytesRemaining + 4;

			if ((ID == CronusZen::INPUTREPORT) && (BytesRemaining == 0x2e)) {
				TotalSize = 64;
			} else if ((ID == CronusZen::OUTPUTREPORT) && (BytesRemaining == 0x24)) {
				TotalSize = 62;
			}

			//App->GetMainDialog().PrintTimestamp();
			//App->GetMainDialog().PrintText(RED, L"Bytes remain: %u\r\n", BytesRemaining);

			// Allocate a buffer to hold the packet
			m_PreparseBuffer = std::make_shared<StoreBuffer>(TotalSize);

			// Handle a single-packet case
			if ((Count == 1) && (ID != CronusZen::REQUESTATTACHEDDEVICES)) {
				// READBYTECODE likes to signal a count of 1 despite being many packets, so handle this accordingly
				m_PreparseBuffer->InsertData(&m_ReceiveBuffer.get()[1], TotalSize > 64 ? (BytesRemaining - TotalSize > 0 ? 60 : 64) : TotalSize);
				BytesRemaining = TotalSize < 64 ? 0 : TotalSize - (BytesRemaining - TotalSize > 0 ? 60 : 64);
			} else {
				// Handle first chunk of a multi-packet REQUESTATTACHEDDEVICES command
				m_PreparseBuffer->InsertData(&m_ReceiveBuffer.get()[1], 64);
				BytesRemaining -= 60; // Update remaining bytes of data required
			}
		} else {
			// Handle subsequent chunks of a multi-packet command
			m_PreparseBuffer->InsertData(&m_ReceiveBuffer.get()[5], ((BytesRemaining >= 60) ? 60 : BytesRemaining));
			BytesRemaining -= ((BytesRemaining >= 60) ? 60 : BytesRemaining);
		}

		//App->GetMainDialog().PrintTimestamp();
		//App->GetMainDialog().PrintText(YELLOW, L"Bytes remain: %u\r\n", BytesRemaining);

		// If all packet data has been received, process it
		if (!BytesRemaining) {
			//DumpHex(m_PreparseBuffer->Buffer(), m_PreparseBuffer->Size());
			HandleReadCommand(m_PreparseBuffer->Buffer(), m_PreparseBuffer->Size());
			
		}

		return TRUE; // Signal success
	} catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Insufficient memory is available to handle the incoming data.");
	} catch (...) {
		App->DisplayError(L"Unhandled exception caught during handling the incoming data.");
	}

	DisconnectFromDevice(); // Disconnect from device as an exception was caught

	return FALSE; // Signal failure
}

VOID CronusZen::HandleReadCommand(CONST PUCHAR PacketData, CONST std::size_t PacketSize)
{
	UCHAR Command = 0;
	USHORT PayloadSize = 0;

	try {
		// Allocate a new parse buffer to process the read command
		m_ParseBuffer = std::make_unique<ParseBuffer>(PacketData, PacketSize);

		// Extract the packet header from the command
		Command = m_ParseBuffer->ExtractByte();
		PayloadSize = m_ParseBuffer->ExtractShort();

		m_ParseBuffer->Advance(1); // We do not care about the packet number

		// Dispatch to the appropriate packet handler based on it's packet ID
		switch ((PacketID)Command) {
		case EXCLUSIONLISTREAD:
			OnExclusionListRead();
			break;
		case FRAGMENTREAD:
			OnFragmentRead();
			break;
		case GETFW:
			OnGetFirmware();
			break;
		case GETSERIAL:
			OnGetSerial();
			break;
		case GETSTATUS:
			OnGetStatus();
			break;
		case INPUTREPORT:
			OnInputReport();
			break;
		case NAVCONREPORT:
			OnNavconReport();
			return;
		case READBYTECODE:
			OnReadByteCode();
			return;
		case READSLOTSCFG:
			OnReadSlotsCfg();
			break;
		case REQUESTATTACHEDDEVICES:
			OnRequestAttachedDevices();
			break;
		case REQUESTMKFILE:
			OnRequestMkFile();
			break;
		}

		return; // Successfully terminate the method

	} catch (CONST UnexpectedSize& BadData) {
		App->DisplayError(L"Unrecognized " + BadData.Command + L" command received; got " + std::to_wstring(BadData.Received) + L" bytes and expected at least " + std::to_wstring(BadData.Expected + 4) + L".");
	} catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Insufficient memory is available to handle the incoming data (command=" + std::to_wstring(Command) + L", payload size=" + std::to_wstring(PayloadSize) + L").");
	} catch (CONST std::exception&) {
		App->DisplayError(L"A buffer overrun was reached during HandleReadCommand (command=" + std::to_wstring(Command) + L", payload size=" + std::to_wstring(PayloadSize) + L").");
	} catch (...) {
		App->DisplayError(L"Unhandled exception caught during HandleReadCommand (command=" + std::to_wstring(Command) + L", payload size=" + std::to_wstring(PayloadSize) + L").");
	}

	DisconnectFromDevice(); // Disconnect from device as an exception was caught
}

// Handle the ExclusionListRead command which currently does unknown
VOID CronusZen::OnExclusionListRead(VOID)
{
	// Static variable to track exclusion list read retry requests
	static UCHAR ExclusionListReadRetry = 0;

	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Validate length of exclusion list and retry if necessary
	// - Ensures that the received exclusion list data is the expected size (56 bytes)
	// - If the size is incorrect, it initiates a retry command up to 3 times
	if (m_PayloadLength != 56) {
		if (ExclusionListReadRetry++ < 3) {
			// Retry fragment read command and notify user
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(ORANGE, L"Retrying exclusion list read (retry attempt %u of 3)...\r\n", ExclusionListReadRetry);
			std::unique_ptr<ExclusionListReadCommand> ExclusionListRead = std::make_unique<ExclusionListReadCommand>();
			QueueCommand(1, *ExclusionListRead);
			return;
		} else {
			// Reset retry counter and notify user of failure
			ExclusionListReadRetry = 0;
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(ORANGE, L"Communication with Cronus Zen failed; device is unresponsive!\r\n");
			DisconnectFromDevice();
			return;
		}
	} else {
		ExclusionListReadRetry = 0;
	}

	// Allocate exclusion list data
	m_ExclusionList = std::make_unique<ExclusionListData>();

	// Extract keyboard and mouse exclusion lists
	m_ParseBuffer->ExtractData(m_ExclusionList.get(), sizeof(ExclusionListData));

	// Display user the status of the connection
	MainDialog.PrintTimestamp();
	MainDialog.PrintText(GRAY, L"Requesting device configuration...\r\n");

	// Prepare fragment read command
	std::unique_ptr<FragmentReadCommand> FragmentRead = std::make_unique<FragmentReadCommand>();
	QueueCommand(1, *FragmentRead);
}

// Handle the FragmentRead command which includes device configuration
VOID CronusZen::OnFragmentRead(VOID)
{
	// Static variable to track fragment retry requests
	static UCHAR FragmentRetry = 0;

	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Notify user that the slots configuration is being processed
	MainDialog.PrintTimestamp();
	MainDialog.PrintText(TEAL, L"Processing device configuration data...\r\n");

	// Validate length of fragment data and retry if necessary
	// - Ensures that the received fragment data is the expected size (60 bytes)
	// - If the size is incorrect, it initiates a retry command up to 3 times
	if (m_PayloadLength != (sizeof(FragmentData) * 60)) {
		if (FragmentRetry++ < 3) {
			// Retry fragment read command; notify user
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(ORANGE, L"Retrying fragment read (retry attempt %u of 3)...\r\n", FragmentRetry);
			std::unique_ptr<FragmentReadCommand> FragmentRead = std::make_unique<FragmentReadCommand>();
			QueueCommand(1, *FragmentRead);
			return;
		} else {
			// Maximum retries exceeded; notify user and disconnect
			FragmentRetry = 0;
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(ORANGE, L"Communication with Cronus Zen failed; device is unresponsive!\r\n");
			DisconnectFromDevice();
			return;
		}
	} else {
		FragmentRetry = 0;
	}

	// Allocate the fragment data
	m_Fragments = std::make_unique<FragmentData[]>(60);

	// Extract fragment data
	m_ParseBuffer->ExtractData(m_Fragments.get(), sizeof(FragmentData) * 60);

	// Process each fragment
	for (unsigned i = 0; i < 60; i++) {
		UCHAR ID = m_Fragments[i].ID;
		UCHAR Value = m_Fragments[i].Value;
		UCHAR Value2 = m_Fragments[i].Value2;

		// Handle fragment based on its source
		switch (m_Fragments[i].Source) {
		case MouseCfg:
			if (ID == 9 || ID == 11) {
				// Ads.MouseStickAxis = (ID == 9 ? 2 * Value + 2 : 2 * Value + 1);
			} else {
				m_Settings.MouseMaps[ID] = Value;
			}
		case KeyboardCfg:
			switch (ID) {
			case 9:
				m_Settings.RightStickMaps[0] = Value;
				m_Settings.RightStickMaps[1] = Value2;
				break;
			case 10:
				m_Settings.RightStickMaps[2] = Value;
				m_Settings.RightStickMaps[3] = Value2;
				break;
			case 11:
				m_Settings.LeftStickMaps[0] = Value;
				m_Settings.LeftStickMaps[1] = Value2;
				break;
			case 12:
				m_Settings.LeftStickMaps[2] = Value;
				m_Settings.LeftStickMaps[3] = Value2;
				break;
			default:
				m_Settings.BtnMaps[ID] = Value;
			}
			break;
		case NavconCfg:
			if (ID != 0xff) {
				m_Settings.NavconMaps[ID] = Value;
			}
			break;
		case GenericCfg:
			m_Settings.MouseKeybMaps[ID] = Value;
		case Cfgs:
			switch (ID) {
			case LightbarPercentage:
				m_Settings.LightbarPercent = Value;
				break;
			case OperationalMode:
				m_Settings.OperationalMode = static_cast<OperationalModeList>(Value);
				if (m_Settings.OperationalMode <= OperationalModeList::TournamentMode) {
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(YELLOW, L"Operational mode is set to %ws mode.\r\n", m_OperationalMode[m_Settings.OperationalMode].c_str());
				}
				break;
			case OutputMode:
				m_Settings.OutputMode = static_cast<OutputModeList>(Value);
				if (m_Settings.OutputMode <= OutputModeList::PlayStation5) {
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(YELLOW, L"Emulator output protocol is set to %ws.\r\n", m_OutputMode[m_Settings.OutputMode].c_str());
				}
				break;
			case RemoteSlot:
				m_Settings.RemoteSlot = static_cast<RemoteSlotChangeList>(Value);
				if (m_Settings.RemoteSlot <= RemoteSlotChangeList::PS_L3) {
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(YELLOW, L"Remote slot change is set to %ws.\r\n", m_RemoteSlot[m_Settings.RemoteSlot].c_str());
				}
				break;
			case Ps4Specialty:
				if (Value <= 1) {
					if (m_Settings.Ps4Specialty = Value) {
						MainDialog.PrintTimestamp();
						MainDialog.PrintText(YELLOW, L"PS4 Specialty is enabled.\r\n");
					}
				}
				break;
			case RemotePlay:
				if (Value <= 1) {
					if (m_Settings.RemotePlay = Value) {
						MainDialog.PrintTimestamp();
						MainDialog.PrintText(YELLOW, L"Remote Play is enabled.\r\n");
					}
				}
				break;
			}
		}
	}

	if (m_ConnectionState != TogglePs4Specialty && m_ConnectionState != ToggleRemotePlay) {
		// Restore connection state
		SetConnectionState(Connecting);
	}

	// Update 'Device' menu items
	MainDialog.UpdateDeviceMenu(m_Settings);

	// RequestMkFile is the next command in the sequence
	MainDialog.PrintTimestamp();
	MainDialog.PrintText(GRAY, L"Requesting mouse and keyboard settings...\r\n");
	
	// Initialize next command and queue it for being sent to the device
	std::unique_ptr<RequestMkFileCommand> RequestMkFile = std::make_unique<RequestMkFileCommand>();
	QueueCommand(1, *RequestMkFile);
}

// Handle the GetFirmware command which includes the firmware version and checksum
VOID CronusZen::OnGetFirmware(VOID)
{
	// Ensure the parse buffer contains enough data
	if (m_ParseBuffer->SizeWithoutHeader() < 10)
		throw UnexpectedSize(L"GetFirmware", m_ParseBuffer->Size(), 10);

	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Extract the firmware version
	std::string Firmware = m_ParseBuffer->ExtractStringA();

	// Assign Unicode firmware string and extract checksum
	m_Firmware = App->AnsiToUnicode(Firmware);
	m_Checksum[0] = m_ParseBuffer->ExtractByte();
	m_Checksum[1] = m_ParseBuffer->ExtractByte();
	m_Checksum[2] = m_ParseBuffer->ExtractByte();
	m_Checksum[3] = m_ParseBuffer->ExtractByte();
	
	// Create a new semantic version object to process the firmware
	m_SemanticVersion = std::make_unique<SemanticVersion>(Firmware);

	// Display the retrieved firmware version to the output window
	MainDialog.PrintTimestamp();
	MainDialog.PrintText(YELLOW, L"Device is using firmware: %ws.\r\n", m_Firmware.c_str());

	// Validate the firmware version
	if (!m_SemanticVersion->IsBeta()) {
		// Notify user the recommendation to downgrade when using this program
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(ORANGE, L"Certain features are unavailable with this firmware version.\r\n");
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(PINK, L"Full application functionality requires a firmware modification.\r\n");
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(PINK, L"Go to \'Firmware\' > \'Install Compatible Firmware\' for instructions.\r\n", m_Firmware.c_str());
		MainDialog.DisplaySupportInfo();
	}
}

// Handle the GetSerial command which returns the device serial number
VOID CronusZen::OnGetSerial(VOID)
{
	// Ensure the parse buffer contains enough data for a valid serial number
	if (m_ParseBuffer->SizeWithoutHeader() < 32)
		throw UnexpectedSize(L"GetSerial", m_ParseBuffer->Size(), 32);

	// Extract the serial number from the buffer
	UCHAR Serial[32] = { 0 };
	m_ParseBuffer->ExtractData(Serial, 32);

	// Translate the non null-terminated byte array into a Unicode string
	m_Serial = App->BytesToUnicode(Serial, sizeof(Serial));

	// Apply a checksum modification for beta firmware versions
	if (m_SemanticVersion->IsBeta()) {
		// Specific logic depends on build number
		if (m_SemanticVersion->GetBuild() == 68) {
			m_Checksum[0] ^= m_Serial[8];
			m_Checksum[1] ^= m_Serial[0];
			m_Checksum[2] ^= m_Serial[4];
			m_Checksum[3] ^= m_Serial[12];
		} else {
			m_Checksum[0] ^= m_Serial[4];
			m_Checksum[1] ^= m_Serial[8];
			m_Checksum[2] ^= m_Serial[12];
			m_Checksum[3] ^= m_Serial[0];
		}
	}

	// Send next command
	std::unique_ptr<ExclusionListReadCommand> ExclusionListRead = std::make_unique<ExclusionListReadCommand>();
	QueueCommand(1, *ExclusionListRead);

}

// Handle the GetStatus command to determine the state of the Cronus Zen
VOID CronusZen::OnGetStatus(VOID)
{
	// Ensure the parse buffer contains enough data for a valid serial number
	if (m_ParseBuffer->SizeWithoutHeader() < sizeof(DeviceStatus))
		throw UnexpectedSize(L"GetStatus", m_ParseBuffer->Size(), sizeof(DeviceStatus));

	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Allocate DeviceStatus structure
	std::unique_ptr<DeviceStatus> Status = std::make_unique<DeviceStatus>();

	// Extract DeviceStatus structure from the read command
	m_ParseBuffer->ExtractData(Status.get(), sizeof(DeviceStatus));

	//if (Status->Error) {
		// TODO: display some sort of error message here
	//	return;
	//}

	// Determine next step based on the command the status was requested for
	switch (Status->Command) {
	case CronusZen::FLASHCONFIG:

		for (unsigned i = 0; i < 8; i++) {
			if (m_SlotConfig[i].MustFlashConfig) {
				// Mark slot as config being flashed
				m_SlotConfig[i].MustFlashConfig = FALSE;

				// Check for success
				if (!Status->Error) {
					// Alert user of action
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(GREEN, L"Successfully flashed config for slot #%u!\r\n", i + 1);
				} else {
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(RED, L"The Cronus Zen rejected the request to flash the config for slot #%u!\r\n", i + 1);
				}

				// Program the script to the slot
				FlashNextGamepack();

				return;
			}
		}

		break;

	case CronusZen::FLASHGAMEPACK:

		for (unsigned i = 0; i < 8; i++) {
			if (m_SlotConfig[i].MustFlashGamepack) {
				// Mark slot as script being flashed
				m_SlotConfig[i].MustFlashGamepack = FALSE;

				// Check for success
				if (!Status->Error) {
					// Alert user of action
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(GREEN, L"Successfully flashed script to slot #%u!\r\n", i + 1);
				}
				else {
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(RED, L"The Cronus Zen rejected the request to flash the script to slot #%u!\r\n", i + 1);
				}

				// Program the script to the slot
				FlashNextConfig();

				return;
			}
		}

		break;

	case CronusZen::REQUESTMKFILE:

		if (m_ConnectionState != TogglePs4Specialty && m_ConnectionState != ToggleRemotePlay) {
			// RequestAttachedDevices is the next command in the sequence
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Requesting attached devices and Bluetooth connections...\r\n");
		}

		// Queue command to request attached devices
		std::unique_ptr<RequestAttachedDevicesCommand> RequestAttachedDevices = std::make_unique<RequestAttachedDevicesCommand>();
		QueueCommand(1, *RequestAttachedDevices);

		break;
	}
}

VOID CronusZen::CheckByteCodeFiles(VOID)
{
	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Request byte code if we are on beta firmware
	if (m_SemanticVersion->IsBeta()) {
		// Iterate through until we found a slot that we need the byte code from
		for (unsigned i = 0; i < 8; i++) {
			// Check if the byte code needs to be read
			if (m_SlotConfig[i].NeedByteCode) {
				// Alert user to action taken
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(GRAY, L"Requesting byte code from slot #%u...\r\n", i + 1);

				// Create the ReadByteCode command and queue it to be sent to the device
				std::unique_ptr<ReadByteCodeCommand> ReadByteCode = std::make_unique<ReadByteCodeCommand>(i);
				QueueCommand(1, *ReadByteCode);

				break;
			}
		}
	}
}

// Handle the input report command which includes various input device information
VOID CronusZen::OnInputReport(VOID)
{
	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Ensure the parse buffer contains enough data for a valid serial number
	if (m_ParseBuffer->SizeWithoutHeader() < sizeof(InputReportData))
		throw UnexpectedSize(L"InputReport", m_ParseBuffer->Size(), sizeof(InputReportData));

	InputReportData InputReport = { };

	// Determine if we need to check for byte code if we are just connecting
	if (m_ConnectionState == Connecting) {
		// Update connection state
		SetConnectionState(Connected);
		// Check for needing to pull byte code
		CheckByteCodeFiles();

	} else {
		// Extract input report
		m_ParseBuffer->ExtractData(&InputReport, sizeof(InputReportData));

		// Check for slot update
		if (InputReport.SlotValue != m_LastInputReport.SlotValue) {
			if (InputReport.SlotValue > 0) {
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(LIGHTBLUE, L"Now running \'%ws\' from slot #%u.\r\n", App->AnsiToUnicode((PCHAR)m_SlotConfigData[InputReport.SlotValue - 1].Title).c_str(), InputReport.SlotValue);
			}
			else {
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(LIGHTBLUE, L"No slot is currently loaded.\r\n");
			}
		}

		// Check for console update
		if (InputReport.ConnectedConsole != m_LastInputReport.ConnectedConsole) {
			if (InputReport.ConnectedConsole && InputReport.ConnectedConsole < 7) {
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(LIGHTBLUE, L"Last detected connection was to a %ws%ws.\r\n", m_Console[InputReport.ConnectedConsole].c_str(), InputReport.ConnectedConsole < 7 ? L" console" : L"");
			}
			else if (!InputReport.ConnectedConsole && m_LastInputReport.ConnectedConsole) {
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(LIGHTBLUE, L"Cronus Zen is no longer connected to a %ws%ws.\r\n", m_Console[m_LastInputReport.ConnectedConsole].c_str(), InputReport.ConnectedConsole < 7 ? L" console" : L"");
			}
		}

		// Check for VM speed update
		if (InputReport.VmSpeedValue != m_LastInputReport.VmSpeedValue) {
			if (m_LastInputReport.VmSpeedValue > 0) {
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(LIGHTBLUE, L"Cronus Zen is now operating at %ums.\r\n", InputReport.VmSpeedValue);
			}
		}
	}

	// Copy to last input report
	memcpy(&m_LastInputReport, &InputReport, sizeof(InputReportData));
}

// Handle the NavconReport command
VOID CronusZen::OnNavconReport(VOID)
{
	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	switch (m_ConnectionState) {
	case ClearBluetooth:
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully cleared registered Bluetooth devices!\r\n");
		break;
	case DeviceCleanup:
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully erased all memory slots on your Cronus Zen!\r\n");
		break;
	case TogglePs4Specialty:
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully %ws PS4 Specialty!\r\n", m_Settings.Ps4Specialty ? L"disabled" : L"enabled");
		break;
	case ToggleRemotePlay:
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully %ws Remote Play!\r\n", m_Settings.RemotePlay ? L"disabled" : L"enabled");
		break;
	case TurnOffController:
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully requested to turn off the Bluetooth-connected controller!\r\n");
		break;
	}

	// Handle the config update type
	switch (m_UpdateFragmentType) {
	case OutputMode:
		// Alert user of action
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully set emulator output protocol to %ws!\r\n", m_OutputMode[m_TargetMode].c_str());

		// Update emulator output protocol
		m_Settings.OutputMode = static_cast<OutputModeList>(m_TargetMode);

		break;

	case OperationalMode:
		// Alert user of action
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully set operational mode to %ws!\r\n", m_OperationalMode[m_TargetMode].c_str());

		// Update emulator output protocol
		m_Settings.OperationalMode = static_cast<OperationalModeList>(m_TargetMode);

		break;

	case RemoteSlot:
		// Alert user of action
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully set remote slot change to %ws!\r\n", m_RemoteSlot[m_TargetMode].c_str());

		// Update emulator output protocol
		m_Settings.RemoteSlot = static_cast<RemoteSlotChangeList>(m_TargetMode);

		break;
	}

	if (m_UpdateFragmentType) {
		// Destroy worker thread as the operation has completed
		DestroyWorkerThread();
		// Reset internal state
		m_UpdateFragmentType = FragmentUnused;
	}

	// Update menus
	MainDialog.UpdateDeviceMenu(m_Settings);

	// Set availability of features on the main dialog
	if (m_ConnectionState == DeviceCleanup) {
		// Request slots config
		std::unique_ptr<ReadSlotsCfgCommand> ReadSlotsCfg = std::make_unique<ReadSlotsCfgCommand>();
		QueueCommand(1, *ReadSlotsCfg);

	} else if (m_ConnectionState != TogglePs4Specialty && m_ConnectionState != ToggleRemotePlay) {
		// Ignore when toggling Ps4 Specialty and Remote Play
		MainDialog.UpdateFeatureAvailability(TRUE);
		// Reset connection state
		SetConnectionState(Connected);
	} else {
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GRAY, L"Please wait for your Cronus Zen to reset...\r\n");
	}

	// Reset IO stream
	std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(Off);
	QueueCommand(1, *StreamIoStatus);
}

// Handle the ReadByteCode command for handling byte code and writing to a file
VOID CronusZen::OnReadByteCode(VOID)
{
	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Variable for determining which slot the byte code belongs to
	UINT ByteCodeSlot = 0;
	USHORT ByteCodeExpectedLength = 0;
	USHORT ByteCodeReturnedLength = m_PayloadLength;

	// Find the first slot that needs the byte code and store it's identifier
	for (unsigned i = 0; i < 8; i++) {
		if (m_SlotConfig[i].NeedByteCode) {
			ByteCodeSlot = i;
			break;
		}
	}

	// Mark slot as no longer needed
	m_SlotConfig[ByteCodeSlot].NeedByteCode = FALSE;

	// Retrieve the expected byte code length
	ByteCodeExpectedLength = m_SlotConfig[ByteCodeSlot].Config.ByteCodeLength - 1;

	// Check the length of the data returned against what it should be
	if (ByteCodeReturnedLength >= ByteCodeExpectedLength) {
		// Obtain a copt of the Unicode version of the slot title
		std::wstring UnicodeTitle = App->AnsiToUnicode((CONST PCHAR)m_SlotConfig[ByteCodeSlot].Config.Title);

		// Alert the user of the action being taken
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully read byte code for \'%ws\' from slot #%u!\r\n", UnicodeTitle.c_str(), ByteCodeSlot + 1);

		// Create File object to write the byte code
		std::unique_ptr<File> OutputBinFile = std::make_unique<File>(m_SlotConfig[ByteCodeSlot].ByteCodeFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, FALSE);

		// Attempt to open the file
		if (!OutputBinFile->Open()) {
			// Alert user of the failure
			App->DisplayError(L"An error occured while preparing the file for writing the byte code from slot #" + std::to_wstring(ByteCodeSlot + 1) + L" to " + m_SlotConfig[ByteCodeSlot].ByteCodeFilePath);
		} else {
			// Attempt to write the byte code to the file
			if (!OutputBinFile->Write(m_PreparseBuffer->Buffer() + 4, ByteCodeExpectedLength, NULL, NULL)) {
				App->DisplayError(L"An error occured while writing the byte code for slot #" + std::to_wstring(ByteCodeSlot + 1) + L" to " + m_SlotConfig[ByteCodeSlot].ByteCodeFilePath);
			}

			// Close the file
			OutputBinFile->Close();
		}

	} else {
		// Alert the user of the unexpected amount of data returned
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(ORANGE, L"An error occured while retrieving the byte code from slot #%u (got %u bytes and expected %u)!\r\n", ByteCodeSlot + 1, ByteCodeReturnedLength, ByteCodeExpectedLength);

		// Notify about slot #8 error
		if ((ByteCodeSlot == 7) && !ByteCodeReturnedLength) {
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(PINK, L"The Cronus Zen device has a known issue where the device refuses to return the byte code from slot #8.\r\n");
			MainDialog.DisplaySupportInfo();
		}
	}

	// Check if another slot needs it's byte code read
	for (unsigned i = ByteCodeSlot; i < 8; i++) {
		// Check if the slot requires a byte code download
		if (m_SlotConfig[i].NeedByteCode) {
			// Alert user to action taken
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Requesting byte code from slot #%u...\r\n", i + 1);

			// Create ReadByteCode command and queue it to be sent to the device
			std::unique_ptr<ReadByteCodeCommand> ReadByteCode = std::make_unique<ReadByteCodeCommand>(i);
			QueueCommand(1, *ReadByteCode);

			break;
		}
	}
}

// Handle the ReadSlotsCfg command for processing slot data
VOID CronusZen::OnReadSlotsCfg(VOID)
{
	// Static variable to track read slots config retry requests
	static UCHAR ReadSlotsCfgRetry = 0;

	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Clear listbox
	MainDialog.ListBoxClear();

	if (m_ConnectionState != DeviceCleanup) {
		// Notify user that the slots configuration is being processed
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(TEAL, L"Processing slots configuration data...\r\n");
	} else {
		// Allow features to be used again
		MainDialog.UpdateFeatureAvailability(TRUE);
		// Reset connection state
		SetConnectionState(Connected);
	}

	// Validate length of slots config and retry if necessary
	// - Ensures that the received slots config data is the expected size (4064 bytes)
	// - If the size is incorrect, it initiates a retry command up to 3 times
	if (m_PayloadLength != 4064) {
		if (ReadSlotsCfgRetry++ < 3) {
			// Retry read slots config command and notify user
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(ORANGE, L"Retrying read slots config (retry attempt %u of 3)...\r\n", ReadSlotsCfgRetry);

			// Create read slots config command object
			std::unique_ptr<ReadSlotsCfgCommand> ReadSlotsCfg = std::make_unique<ReadSlotsCfgCommand>();

			// Send the read slots config command
			QueueCommand(1, *ReadSlotsCfg);

			return; // Failure
		} else {
			// Reset retry counter
			ReadSlotsCfgRetry = 0;

			// Notify user that the device is unresponsive
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(ORANGE, L"Communication with Cronus Zen failed; device is unresponsive!\r\n");

			// Disconnect due to unresponsiveness
			DisconnectFromDevice();

			return; // Failure
		}
	} else {
		ReadSlotsCfgRetry = 0;
	}

	// Allocate slots config data
	m_SlotConfig = std::make_unique<SlotConfig[]>(8);
	m_SlotConfigData = std::make_unique<SlotConfigData[]>(8);

	// Extract slots config data
	m_ParseBuffer->ExtractData(m_SlotConfigData.get(), 8 * sizeof(SlotConfigData));

	// Create the "SlotData" directory and handle any access/elevation required errors
	if (!CreateDirectory(L"SlotData", NULL))
		App->HandleCreateDirectoryError(L"SlotData");

	// Initialize variable used to track slot usage and storage space
	UINT TotalBytes = 0;

	// Reset number of slots used
	m_SlotsUsed = 0;

	// Iterate through all 8 slots to gather usage information
	for (unsigned i = 0; i < 8; i++) {
		// Get bytecode length for the current slot
		BOOLEAN IsScript = (m_SlotConfigData[i].GamepackID == 0xffff);
		INT ByteCodeLength = m_SlotConfigData[i].ByteCodeLength;

		// If the slot is occupied (bytecode length is non-zero)
		if (--ByteCodeLength > 0) {
			// Check for converting .gpc to .bin (scripts only)
			if (IsScript) {
				// Convert extension to .bin or add it if it isn't there
				*(PULONG)(m_SlotConfigData[i].Title + strlen((CONST PCHAR)m_SlotConfigData[i].Title) - 4) = 'nib.';
			}

			// Convert slot title to Unicode for display purposes
			std::wstring UnicodeTitle = App->AnsiToUnicode((CONST PCHAR)m_SlotConfigData[i].Title);

			// Print information about the slot's contents
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(YELLOW, L"Slot #%u has %ws \'%ws\' using %u bytes.\r\n",
				m_SlotConfigData[i].Slot - 0x2f,
				m_SlotConfigData[i].GamepackID == 0xffff ? L"script" : L"gamepack",
				UnicodeTitle.c_str(),
				ByteCodeLength);

			// Insert item to main dialog list box
			MainDialog::ListBoxItem* ListBoxItem = new MainDialog::ListBoxItem{ NULL };
			ListBoxItem->IsBeingDeleted = FALSE;
			ListBoxItem->GamepackID = m_SlotConfigData[i].GamepackID;
			ListBoxItem->Slot = m_SlotConfigData[i].Slot - 0x30;
			wcscpy_s(ListBoxItem->Title, UnicodeTitle.c_str());

			// Check for byte code file (.bin) if slot is a script
			if (IsScript) {
				// Generate slot data path
				std::wstring SlotDirectory = L"SlotData\\Slot" + std::to_wstring(ListBoxItem->Slot + 1);

				// Create the "SlotData" directory and handle any access/elevation required errors
				if (!CreateDirectory(SlotDirectory.c_str(), NULL))
					App->HandleCreateDirectoryError(SlotDirectory);

				// Set the file paths for the byte code and config file
				m_SlotConfig[i].ByteCodeFile = UnicodeTitle;
				m_SlotConfig[i].ByteCodeFilePath = SlotDirectory + L"\\" + UnicodeTitle;
				m_SlotConfig[i].ConfigFilePath = m_SlotConfig[i].ByteCodeFilePath;
				m_SlotConfig[i].ConfigFilePath.replace(m_SlotConfig[i].ConfigFilePath.size() - 3, 3, L"dat");

				// Set the config data
				memcpy(&m_SlotConfig[i].Config, &m_SlotConfigData[i], sizeof(SlotConfigData));

				// Set initial flags in the event of programming
				m_SlotConfig[i].MustFlashConfig = TRUE;
				m_SlotConfig[i].MustFlashGamepack = TRUE;

				// Include additional attributes such as byte code length and path to file
				ListBoxItem->ByteCodeLength = ByteCodeLength;
				ListBoxItem->Path = m_SlotConfig[i].ByteCodeFilePath;

				std::unique_ptr<File> FileExists = std::make_unique<File>(m_SlotConfig[i].ByteCodeFilePath);

				// Check if file exists, if not, mark it for requesting the byte code
				if (!FileExists->Exists()) {
					m_SlotConfig[i].NeedByteCode = TRUE;
				}

				// Create File object to dump config
				std::unique_ptr<File> ConfigFilePath = std::make_unique<File>(m_SlotConfig[i].ConfigFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, FALSE);

				// Attempt to open the slot config file for writing
				if (!ConfigFilePath->Open()) {
					App->DisplayError(L"An error occured while opening the config file for slot #" + std::to_wstring(i + 1));
				}
				else {
					// Attempt to write to the slot config file
					if (!ConfigFilePath->Write(&m_SlotConfigData[i], sizeof(SlotConfigData), NULL, NULL)) {
						App->DisplayError(L"An error occured while writing the config file for slot #" + std::to_wstring(i + 1));
					}
				}
			}

			// Add item to listbox
			MainDialog.ListBoxAdd(ListBoxItem);

			// Update total slot and byte counts
			m_SlotsUsed++;
			TotalBytes += ByteCodeLength;
		}
	}

	// Print a summary based on the slots usage information
	if (!TotalBytes) {
		// No scripts/gamepacks found
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(YELLOW, L"There are no scripts or gamepacks currently on your device.\r\n");
	} else {
		// Print slots usage information
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(YELLOW, L"%u slot%ws using %u bytes of storage with %u bytes of storage available.\r\n", m_SlotsUsed, m_SlotsUsed > 1 ? L"s" : L"", TotalBytes, 262136 - TotalBytes);
	}

	// Update main window
	MainDialog.UpdateSlotsData(m_SlotsUsed, TotalBytes);

	if (m_ConnectionState == TogglePs4Specialty || m_ConnectionState == ToggleRemotePlay) {
		CreateWorkerThread(CronusZen::RefreshAttachedDevices);
	} else {
		MainDialog.UpdateFeatureAvailability(m_SemanticVersion->IsBeta());
	}

	// Currently we can only process input reports on beta firmware
	if (m_SemanticVersion->IsBeta()) {
		std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(InputReport);
		QueueCommand(1, *StreamIoStatus);
	} else {
		// Provide message box to user if not on beta firmware
		MessageBox(MainDialog.GetHwnd(), L"Certain features are unavailable with this firmware version.\r\n\r\nFull application functionality requires a firmware modification.\r\n\r\nGo to \'Firmware\' > \'Install Compatible Firmware\' for instructions.", L"Warning: Incompatible Firmware", MB_ICONHAND | MB_OK);
	}

	if (m_ConnectionState == ProgramDevice) {
		CheckByteCodeFiles();
	}
}

// Handle the RequestAttachedDevices command for processing wired connection sand Bluetooth devices
VOID CronusZen::OnRequestAttachedDevices(VOID)
{
	// Static variable to track request attached devices retry requests
	static UCHAR RequestAttachedDevicesRetry = 0;

	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	if (m_ConnectionState != TogglePs4Specialty && m_ConnectionState != ToggleRemotePlay) {
		// Notify user that the valid is indeed valid and will be processed
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(TEAL, L"Processing attached devices and Bluetooth connection data...\r\n");

		// Validate length of attached devices and retry if necessary
		// - Ensures that the received attached devices data is the expected size (96 bytes)
		// - If the size is incorrect, it initiates a retry command up to 3 times
		if (m_PayloadLength != 96) {
			if (RequestAttachedDevicesRetry++ < 3) {
				// Retry read slots config command and notify user
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(ORANGE, L"Retrying request attached devices (retry attempt %u of 3)...\r\n", RequestAttachedDevicesRetry);

				// Allocate and queue the RequestAttachedDevices command
				std::unique_ptr<RequestAttachedDevicesCommand> RequestAttachedDevices = std::make_unique<RequestAttachedDevicesCommand>();
				QueueCommand(1, *RequestAttachedDevices);

				return; // Failure

			} else {
				// Reset retry counter
				RequestAttachedDevicesRetry = 0;

				// Notify user of unresponsive device
				MainDialog.PrintTimestamp();
				MainDialog.PrintText(ORANGE, L"Communication with Cronus Zen failed; device is unresponsive!\r\n");

				// Disconnect from device due to unresponsiveness
				DisconnectFromDevice();

				return; // Failure

			}
		} else {
			RequestAttachedDevicesRetry = 0;
		}

		// Reset hub information
		memset(m_IsHub, FALSE, sizeof(m_IsHub));

		// Allocate the fragment data
		m_AttachedDevices = std::make_unique<AttachedDevice[]>(12);

		// Extract fragment data
		m_ParseBuffer->ExtractData(m_AttachedDevices.get(), 96);

		// Iterate through attached devices to identify USB hubs
		for (unsigned i = 0; i < 10; i++) {
			UCHAR DAddress = m_AttachedDevices[i].DAddress;
			for (unsigned j = 0; j < 10; j++) {
				if ((m_AttachedDevices[j].DAddress != DAddress) && (m_AttachedDevices[j].Parent == DAddress) && (DAddress != 0)) {
					// Mark the device as hub if it has children
					m_IsHub[i] = TRUE;
				}
			}
		}

		// Reset wired/Bluetooth devices
		m_BluetoothDevices = m_WiredDevices = 0;

		// Iterate through attached devices to detect wired and Bluetooth connections
		for (unsigned i = 0; i < 12; i++) {
			if (m_AttachedDevices[i].Level == 1) {
				// Check for wired devices connected to ports A1, A2, or A3
				if (m_AttachedDevices[i].Port >= 1 && m_AttachedDevices[i].Port <= 3) {
					// Print device information and indicate if it's a hub
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(YELLOW, L"%ws is wired to port A%u%ws.\r\n",
						GetDeviceDescription(m_AttachedDevices[i].VendorID, m_AttachedDevices[i].ProductID).c_str(),
						m_AttachedDevices[i].Port,
						m_IsHub[i] ? L" (USB hub)" : L"");

					// Increment wired device count
					m_WiredDevices++;
				}
				// Check for Bluetooth devices connected to channels 11 or 12
				else if (m_AttachedDevices[i].Port > 10) {
					if (m_AttachedDevices[i].VendorID && m_AttachedDevices[i].ProductID) {
						// Print Bluetooth device information
						MainDialog.PrintTimestamp();
						MainDialog.PrintText(YELLOW, L"%ws is connected via Bluetooth to channel %X.\r\n",
							GetDeviceDescription(m_AttachedDevices[i].VendorID, m_AttachedDevices[i].ProductID).c_str(),
							m_AttachedDevices[i].Port - 1);

						// Increment Bluetooth device count
						m_BluetoothDevices++;
					}
				}
			}
		}

		// Print message if no wired devices are found
		if (!m_WiredDevices) {
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(LIGHTBLUE, L"No wired input devices are detected on your Cronus Zen.\r\n");
		}

		// Print message if no Bluetooth connections are found
		if (!m_BluetoothDevices) {
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(LIGHTBLUE, L"No Bluetooth connections are detected on your Cronus Zen.\r\n");
		}
	}

	// Request slots configuration as long as a refresh attached devices request was not submitted
	if (m_ConnectionState != RefreshAttachedDevices) {
		// RequestMkFile is the next command in the sequence
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GRAY, L"Requesting slots configuration...\r\n");

		std::unique_ptr<ReadSlotsCfgCommand> ReadSlotsCfg = std::make_unique<ReadSlotsCfgCommand>();
		QueueCommand(1, *ReadSlotsCfg);
	}
}

// Handle the RequestMkFile command to process the mouse and keyboard configuration
VOID CronusZen::OnRequestMkFile(VOID)
{
	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Check if this command was a result of a factory reset and notify the user
	if (m_ConnectionState == FactoryReset) {
		// Alert user of the action
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(GREEN, L"Successfully factory reset your Cronus Zen!\r\n");
		
		// Clear slots list
		MainDialog.ListBoxClear();

		// Use StreamIoStatusMask 0x1d
		// Cronus->QueueCommand(1, *StreamIoStatus);
	}

	// Ensure the parse buffer contains enough data for a valid mouse and keyboard settings file
	if (m_PayloadLength == 1283) {
		// Notify user that the valid is indeed valid and will be processed
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(TEAL, L"Processing mouse and keyboard settings data...\r\n");

		// Create MkFile object to manage the mouse and keyboard settings file
		m_MkFile = std::make_unique<MkFile>();

		// Set the MkFile data using the payload
		m_MkFile->SetMkFileData(m_ParseBuffer->Buffer() + 4, m_PayloadLength);

		// Notify user of the file being used
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(YELLOW, L"%ws file type (version %ws) using profile \'%ws\' (revision %ws).\r\n",
			m_MkFile->GetType().c_str(),
			m_MkFile->GetVersion().c_str(),
			m_MkFile->GetName().c_str(),
			m_MkFile->GetRevision().c_str());
	} else {
		// Notify user that the mouse and keyboard settings file returned is of an unexpected length
		MainDialog.PrintTimestamp();
		MainDialog.PrintText(ORANGE, L"Unable to process mouse and keyboard settings; unexpected size returned (%u bytes).\r\n", m_PayloadLength);
	}

	// Allocate and queue the GetStatus command
	std::unique_ptr<GetStatusCommand> GetStatus = std::make_unique<GetStatusCommand>();
	QueueCommand(1, *GetStatus);
}

BOOL CronusZen::OnWrite(CONST DWORD BytesWritten)
{
	// Remove command from queue
	m_Queue.pop_front();

	// Check if there are any remaining commands queued up and send the next one, if so
	if (!m_Queue.empty())
		AsynchronousWrite(m_Queue.front());

	return TRUE;
}

VOID CronusZen::QueueCommand(CONST UCHAR Count, CommandBase& Command)
{
	// Create a temporary buffer to hold the outgoing command structure
	PUCHAR OutgoingCommand = new UCHAR[65]{ 0 };

	// Copy the commands header (null-byte followed by packet identifier)
	memcpy(OutgoingCommand, Command.Buffer(), 2);

	// Set the command's size (excluding the header)
	*(PUSHORT)&OutgoingCommand[2] = static_cast<USHORT>(Command.Size() - 2);

	// Set the command count
	*(PBYTE)&OutgoingCommand[4] = Count;

	// Copy the remaining command data (excluding the header)
	memcpy(&OutgoingCommand[5], Command.Buffer() + 2, Command.Size() - 2);

	// Add packet to queue
	m_Queue.push_back(OutgoingCommand);

	// Check if command is the only item in queue
	if (m_Queue.size() == 1) {
		// If queue was empty, write command immediately
		AsynchronousWrite(OutgoingCommand);
	}
}

VOID CronusZen::QueueCommand(CONST UCHAR Count, CONST USHORT Size, CommandBase& Command)
{
	// Create a temporary buffer to hold the outgoing command structure
	PUCHAR OutgoingCommand = new UCHAR[65]{ 0 };

	// Copy the commands header (null-byte followed by packet identifier)
	memcpy(OutgoingCommand, Command.Buffer(), 2);

	// Set the command's size (excluding the header)
	*(PUSHORT)&OutgoingCommand[2] = Size;

	// Set the command count
	*(PBYTE)&OutgoingCommand[4] = Count;

	// Copy the remaining command data (excluding the header)
	memcpy(&OutgoingCommand[5], Command.Buffer() + 2, Command.Size() - 2);

	// Add packet to queue
	m_Queue.push_back(OutgoingCommand);

	// Check if command is the only item in queue
	if (m_Queue.size() == 1) {
		// If queue was empty, write command immediately
		AsynchronousWrite(OutgoingCommand);
	}
}

// Method for sending the initial communication upon connecting to the device
VOID CronusZen::SendInitialCommunication(VOID)
{
	// Build the initial communication command sequence
	std::unique_ptr<StreamIoStatusCommand> StreamIoStatus = std::make_unique<StreamIoStatusCommand>(CronusZen::Off);
	std::unique_ptr<ExitApiModeCommand> ExitApiMode = std::make_unique<ExitApiModeCommand>();
	std::unique_ptr<UnloadGpcCommand> UnloadGpc = std::make_unique<UnloadGpcCommand>();
	std::unique_ptr<CircleTestCommand> CircleTest = std::make_unique<CircleTestCommand>(0, 0, 0);
	std::unique_ptr<GetFirmwareCommand> GetFirmware = std::make_unique<GetFirmwareCommand>();
	std::unique_ptr<GetSerialCommand> GetSerial = std::make_unique<GetSerialCommand>();

	// Notify user of the state of the connection
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(GRAY, L"Initiating communication with the Cronus Zen...\r\n");

	// Queue commands to be sent to the device
	QueueCommand(1, *StreamIoStatus);
	QueueCommand(1, *ExitApiMode);
	QueueCommand(1, *UnloadGpc);
	QueueCommand(1, *CircleTest);
	QueueCommand(1, *GetFirmware);
	QueueCommand(1, *GetSerial);
}

// Method used for adding a script to be programmed to the device
VOID CronusZen::SlotsAdd(VOID)
{
	// Initialize variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Initialize other required variables
	OPENFILENAME OpenFileName = { NULL };
	WCHAR FilePath[MAX_PATH] = { NULL };

	// Build our OPENFILENAME struct
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = MainDialog.GetHwnd();
	OpenFileName.lpstrFile = FilePath;
	OpenFileName.nMaxFile = MAX_PATH;
	OpenFileName.lpstrFilter = L"Compiled Scripts (*.bin)\0*.bin\0";
	OpenFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;

	// Open the dialog and capture the file the user selects
	if (GetOpenFileName(&OpenFileName)) {

		// TODO: check for file size exceeding limit of the Cronus Zen
		
		// Initialize variables used for processing the incoming file
		MainDialog::ListBoxItem* ListBoxItem = new MainDialog::ListBoxItem{ NULL };
		std::wstring StrFilePath = FilePath;
		std::size_t LastSlash = StrFilePath.rfind(L"\\");
		UCHAR SlotToWrite = 0;

		// Loop through and find the first available slot
		for (unsigned i = 0; i < 8; i++) {
			if (!m_SlotConfig[i].ByteCodeFilePath.size()) {
				SlotToWrite = i;
				break;
			}
		}

		// Prepare listbox item
		ListBoxItem->IsBeingProgrammed = TRUE;
		ListBoxItem->GamepackID = 0xffff; // Script (.bin file)
		ListBoxItem->Path = StrFilePath;
		ListBoxItem->Slot = SlotToWrite;

		// Check if a backslash was found
		if (LastSlash != std::wstring::npos)
			StrFilePath = StrFilePath.substr(LastSlash + 1);

		// Set the listbox item title
		wcscpy_s(ListBoxItem->Title, StrFilePath.substr(0, 47).c_str());

		// Ensure that the file ends in ".bin"
		if (_wcsicmp(ListBoxItem->Title + wcslen(ListBoxItem->Title) - 4, L".bin")) {
			wcscat_s(ListBoxItem->Title, 52, L".bin");
		}

		// Insert into listbox
		MainDialog.ListBoxAdd(ListBoxItem);

		// Update internal slots config for programming
		m_SlotConfig[SlotToWrite].ByteCodeFile = StrFilePath;
		m_SlotConfig[SlotToWrite].ByteCodeFilePath = ListBoxItem->Path;
		m_SlotConfig[SlotToWrite].ConfigFilePath = L"";
		m_SlotConfig[SlotToWrite].NeedByteCode = FALSE;
		m_SlotConfig[SlotToWrite].MustFlashConfig = TRUE;
		m_SlotConfig[SlotToWrite].MustFlashGamepack = TRUE;
		memset(&m_SlotConfig[SlotToWrite].Config, 0, sizeof(SlotConfigData));

		m_SlotsUsed++;
	}
}

// Method used to initialize programming slots to the device
VOID CronusZen::SlotsProgram(VOID)
{
	CreateWorkerThread(ProgramDevice);
}

// Method used for flashing the slot config file
VOID CronusZen::FlashNextConfig(VOID)
{
	// Variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	// Initialize required variables
	const UCHAR Unknown3[44] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
		0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
		0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
		0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
		0x28, 0x29, 0x2a, 0x2b
	};
	SlotConfigData FlashConfigData = { NULL };
	USHORT FlashConfigFileSize = 0;

	for (unsigned i = 0; i < 8; i++) {
		if (m_SlotConfig[i].MustFlashConfig) {
			// Alert user of the action being taken
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Attempting to flash config on slot #%u for %ws...\r\n", i + 1, m_SlotConfig[i].ByteCodeFile.c_str());

			try {
				std::unique_ptr<File> ScriptFile = std::make_unique<File>(m_SlotConfig[i].ByteCodeFilePath);

				// Attempt to open the script (.bin) file
				if (!ScriptFile->Open(GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, TRUE))
					throw std::wstring(L"An error occured reading " + m_SlotConfig[i].ByteCodeFilePath + L".");

				// Get the file size
				FlashConfigFileSize = static_cast<USHORT>(ScriptFile->GetFileSize() + 1);

				// Check if the config exists or not, if not, create our own config data
				
				// Create File object for reading config file
				std::unique_ptr<File> ConfigFile = std::make_unique<File>(m_SlotConfig[i].ConfigFilePath, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, FALSE);

				if (ConfigFile->Exists()) {
					// Alert user to action
					MainDialog.PrintTimestamp();
					MainDialog.PrintText(YELLOW, L"Attempting to use existing config for this script from its previous slot...\r\n");

					if (ConfigFile->Open()) {
						if (!ConfigFile->Read(&FlashConfigData, sizeof(SlotConfigData), NULL, NULL))
							throw std::wstring(L"An error occured reading the file data of the config data for " + m_SlotConfig[i].ConfigFilePath + L".");
					} else {
						throw std::wstring(L"An error occured reading the file data of the config data for " + m_SlotConfig[i].ConfigFilePath + L".");
					}

				} else {
					// Initialize variables used for the flash config command
					std::string AnsiTitle = App->UnicodeToAnsi(m_SlotConfig[i].ByteCodeFile).substr(0, 51);

					// Ensure there's a null-terminator so we don't run into any long name issues
					AnsiTitle.push_back('\0');

					// Build the flash config command
					FlashConfigData.GamepackID = 0xffff;
					FlashConfigData.Flags = 1 + (32 | 2);
					FlashConfigData.ByteCodeLength = FlashConfigFileSize;
					memcpy(&FlashConfigData.Unknown3, Unknown3, 44);
					strcpy_s((PCHAR)FlashConfigData.Title, AnsiTitle.size(), AnsiTitle.c_str());
				}

				// Ensure the slot is correct
				FlashConfigData.Slot = 0x30 + i;

				// Initialize a variable to hold the config data for inserting across each command packet
				UCHAR Config[508];
				memset(&Config, 0, 508);
				memcpy(&Config, &FlashConfigData, 508);

				// Queue flash config command
				for (unsigned i = 0; i < 8; i++) {
					std::shared_ptr<FlashConfigCommand> FlashConfig = std::make_shared<FlashConfigCommand>();
					FlashConfig->InsertData(Config + i * 60, 60);
					QueueCommand(i == 0 ? 1 : 0, 508, *FlashConfig);
				}

				// Queue the remainder of the config
				std::unique_ptr<FlashConfigCommand> FlashConfig = std::make_unique<FlashConfigCommand>();
				UCHAR EmptyData[32] = { 0 };
				FlashConfig->InsertData(Config + 480, 28);
				QueueCommand(0, 508, *FlashConfig);

				// Get status
				std::unique_ptr<GetStatusCommand> GetStatus = std::make_unique<GetStatusCommand>();
				QueueCommand(1, *GetStatus);

				return;

			} catch (CONST std::bad_alloc&) {
				App->DisplayError(L"Insufficient memory available while attempting to flash the config for " + m_SlotConfig[i].ByteCodeFilePath + L".");
			} catch (CONST std::wstring& CustomMessage) {
				App->DisplayError(CustomMessage);
			}
		}
	}

	// Enable features on the main dialog
	MainDialog.UpdateFeatureAvailability(TRUE);

	// Send checksum to affirm the flash
	std::unique_ptr<ClCommand> Cl = std::make_unique<ClCommand>();
	Cl->InsertByte(m_Checksum[0]);
	Cl->InsertByte(m_Checksum[1]);
	Cl->InsertByte(m_Checksum[2]);
	Cl->InsertByte(m_Checksum[3]);

	// Insert random data
	for (unsigned i = 0; i < 56; i++)
		Cl->InsertByte(rand() % 256);

	// Queue command
	QueueCommand(1, *Cl);

	// Queue command to read slots config
	std::unique_ptr<ReadSlotsCfgCommand> ReadSlotsCfg = std::make_unique<ReadSlotsCfgCommand>();
	QueueCommand(1, *ReadSlotsCfg);
}

// Method used for flashing the script to the slot
VOID CronusZen::FlashNextGamepack(VOID)
{
	// Variable for ease of accessibility
	MainDialog& MainDialog = App->GetMainDialog();

	for (unsigned i = 0; i < 8; i++) {
		if (m_SlotConfig[i].MustFlashGamepack) {
			// Alert user of the action being taken
			MainDialog.PrintTimestamp();
			MainDialog.PrintText(GRAY, L"Attempting to flash slot #%u with file %ws...\r\n", i + 1, m_SlotConfig[i].ByteCodeFile.c_str());

			try {
				std::unique_ptr<File> ScriptFile = std::make_unique<File>(m_SlotConfig[i].ByteCodeFilePath);

				// Attempt to open the script (.bin) file
				if (!ScriptFile->Open(GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, TRUE))
					throw std::wstring(L"An error occured reading " + m_SlotConfig[i].ByteCodeFilePath + L".");

				// Initialize a variable to store the file size
				USHORT ScriptFileSize = static_cast<USHORT>(ScriptFile->GetFileSize());

				// Allocate a buffer to hold the file data
				std::unique_ptr<UCHAR[]> ScriptFileData(new UCHAR[ScriptFileSize]{ 0 });

				// Attempt to read the file data
				if (!ScriptFile->Read(ScriptFileData.get(), ScriptFileSize, NULL, NULL))
					throw std::wstring(L"An error occured reading the file data for " + m_SlotConfig[i].ByteCodeFile + L".");

				// Initialize variables required
				std::unique_ptr<UCHAR[]> PaddingData(new UCHAR[60]{ 0 });
				USHORT BytesSent = 0;

				while (BytesSent != ScriptFileSize) {
					// Create our flash gamepack command object
					std::unique_ptr<FlashGamepackCommand> FlashGamepack = std::make_unique<FlashGamepackCommand>();

					if (!BytesSent) {
						// Insert slot number for the first packet
						FlashGamepack->InsertByte(0x30 + i);

						if (ScriptFileSize > 59) {
							// Insert first chunk
							FlashGamepack->InsertData(ScriptFileData.get(), 59);

							// Increase number of bytes sent
							BytesSent += 59;

							// Store the last data sent for the potential required padding
							*(PUCHAR)PaddingData.get() = 0x30 + i;
							memcpy(&PaddingData[1], ScriptFileData.get(), 59);

						}
						else {
							// Insert next chunk
							FlashGamepack->InsertData(ScriptFileData.get(), ScriptFileSize);

							// Increase number of bytes sent
							BytesSent += ScriptFileSize;
						}
					} else if (ScriptFileSize - BytesSent < 60) {
						// Insert the remaining file data and apply the padding
						FlashGamepack->InsertData(ScriptFileData.get() + BytesSent, ScriptFileSize - BytesSent);
						FlashGamepack->InsertData(PaddingData.get() + ScriptFileSize - BytesSent, 60 - (ScriptFileSize - BytesSent));

						// Increase number of bytes sent
						BytesSent += (ScriptFileSize - BytesSent);

					} else {
						// Copy padding data
						memcpy(PaddingData.get(), ScriptFileData.get() + BytesSent, 60);

						// Insert the next 60 bytes of the file dat
						FlashGamepack->InsertData(ScriptFileData.get() + BytesSent, 60);

						// Increase number of bytes sent
						BytesSent += 60;
					}

					// Queue the command
					QueueCommand(BytesSent <= 60 ? 1 : 0, ScriptFileSize + 1, *FlashGamepack);
				}

				// Request status
				std::unique_ptr<GetStatusCommand> GetStatus = std::make_unique<GetStatusCommand>();
				QueueCommand(1, *GetStatus);

			}
			catch (CONST std::bad_alloc&) {
				App->DisplayError(L"Insufficient memory available while attempting to flash " + m_SlotConfig[i].ByteCodeFilePath + L".");
			} catch (CONST std::wstring& CustomMessage) {
				App->DisplayError(CustomMessage);
			}

			break;
		}
	}
}

// Method used for marking a slot for deletion and advancing the following slots a position forwards
VOID CronusZen::SlotsRemove(_In_ CONST UINT Slot)
{
	// Clear the slot
	memset(&m_SlotConfig[Slot], 0, sizeof(SlotConfig));

	// Consolidate any open slots
	for (unsigned i = Slot, j = Slot + 1; j < 8; j++) {
		if (m_SlotConfig[i].ByteCodeFile.empty() && !m_SlotConfig[j].ByteCodeFile.empty()) {
			memcpy(&m_SlotConfig[i], &m_SlotConfig[j], sizeof(SlotConfig));
			memset(&m_SlotConfig[j], 0, sizeof(SlotConfig));
			j--;
		} else {
			i++;
		}
	}

	// Reduce number of slots used
	m_SlotsUsed--;
}

// Method used to return the number of slots used
CONST UCHAR CronusZen::SlotsUsed(VOID) CONST
{
	return m_SlotsUsed;
}