#include "Precomp.h"

CommandBase::CommandBase(CONST CronusZen::PacketID Command)
{
	InsertByte(0);
	InsertByte(Command);
}

// Callback method for the HID device connected event
BOOL CronusZen::OnConnect(VOID)
{
	// Notify user that the connection has been established
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(GREEN, L"Successfully opened a connection to your Cronus Zen!\r\n");

	try
	{
		// Query HID device capabilities
		if (QueryHIDDeviceCapabilities()) {
			// Send initial communication command sequence
			SendInitialCommunication();
		}
		else {
			throw std::wstring(L"The connection to your device has been aborted.");
		}

		return TRUE; // Signal success
	}
	catch (CONST std::wstring& CustomMessage)
	{
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
	}
	else {
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

	for (unsigned j = 0; j < PacketSize; j += 16)
	{
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
	try
	{
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

			//App->GetMainDialog().PrintTimestamp();
			//App->GetMainDialog().PrintText(RED, L"Bytes remain: %u\r\n", BytesRemaining);

			// Allocate a buffer to hold the packet
			m_PreparseBuffer = std::make_unique<StoreBuffer>(TotalSize);

			// Handle a single-packet case
			if ((Count == 1) && (ID != CronusZen::REQUESTATTACHEDDEVICES)) {
				m_PreparseBuffer->InsertData(&m_ReceiveBuffer.get()[1], TotalSize);
				BytesRemaining = 0; // All of the data received
			}
			else {
				// Handle first chunk of a multi-packet command
				m_PreparseBuffer->InsertData(&m_ReceiveBuffer.get()[1], 64);
				BytesRemaining -= 60; // Update remaining bytes of data required
			}
		}
		else {
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
	}
	catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Insufficient memory is available to handle the incoming data.");
	}

	DisconnectFromDevice(); // Disconnect from device as an exception was caught

	return FALSE; // Signal failure
}

VOID CronusZen::HandleReadCommand(CONST PUCHAR PacketData, CONST std::size_t PacketSize)
{
	try
	{
		// Allocate a new parse buffer to process the read command
		m_ParseBuffer = std::make_unique<ParseBuffer>(PacketData, PacketSize);

		// Extract the packet header from the command
		UCHAR Command = m_ParseBuffer->ExtractByte();
		USHORT PayloadSize = m_ParseBuffer->ExtractShort();

		m_ParseBuffer->Advance(1); // We do not care about the packet number

		// Dispatch to the appropriate packet handler based on it's packet ID
		switch ((PacketID)Command)
		{
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
	}
	catch (CONST UnexpectedSize& BadData) {
		App->DisplayError(L"Unrecognized " + BadData.Command + L" command received; got " + std::to_wstring(BadData.Received) + L" bytes and expected at least " + std::to_wstring(BadData.Expected) + L".");
	}
	catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Insufficient memory is available to handle the incoming data.");
	}
	catch (CONST std::exception&) {
		App->DisplayError(L"A buffer overrun was reached during HandleReadCommand.");
	}

	DisconnectFromDevice(); // Disconnect from device as an exception was caught
}

// Handle the ExclusionListRead command which currently does unknown
VOID CronusZen::OnExclusionListRead(VOID)
{
	// Static variable to track exclusion list read retry requests
	static UCHAR ExclusionListReadRetry = 0;

	// Validate length of exclusion list and retry if necessary
	// - Ensures that the received exclusion list data is the expected size (56 bytes)
	// - If the size is incorrect, it initiates a retry command up to 3 times
	if (m_PayloadLength != 56) {
		if (ExclusionListReadRetry++ < 3) {
			// Retry fragment read command and notify user
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(ORANGE, L"Retrying exclusion list read (retry attempt %u of 3)...\r\n", ExclusionListReadRetry);
			std::unique_ptr<ExclusionListReadCommand> ExclusionListRead(new ExclusionListReadCommand);
			QueueCommand(1, *ExclusionListRead);
			return;
		}
		else {
			// Reset retry counter and notify user of failure
			ExclusionListReadRetry = 0;
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(ORANGE, L"Communication with Cronus Zen failed; device is unresponsive!\r\n");
			DisconnectFromDevice();
			return;
		}
	}
	else {
		ExclusionListReadRetry = 0;
	}

	// Allocate exclusion list data
	m_ExclusionList = std::make_unique<ExclusionListData>();

	// Extract keyboard and mouse exclusion lists
	m_ParseBuffer->ExtractData(m_ExclusionList.get(), sizeof(ExclusionListData));

	// Display user the status of the connection
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(GRAY, L"Requesting device configuration...\r\n");

	// Prepare fragment read command
	std::unique_ptr<FragmentReadCommand> FragmentRead(new FragmentReadCommand);
	QueueCommand(1, *FragmentRead);
}

// Handle the FragmentRead command which includes device configuration
VOID CronusZen::OnFragmentRead(VOID)
{
	// Static variable to track fragment retry requests
	static UCHAR FragmentRetry = 0;

	// Notify user that the slots configuration is being processed
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(TEAL, L"Processing device configuration data...\r\n");

	// Validate length of fragment data and retry if necessary
	// - Ensures that the received fragment data is the expected size (60 bytes)
	// - If the size is incorrect, it initiates a retry command up to 3 times
	if (m_PayloadLength != (sizeof(FragmentData) * 60)) {
		if (FragmentRetry++ < 3) {
			// Retry fragment read command; notify user
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(ORANGE, L"Retrying fragment read (retry attempt %u of 3)...\r\n", FragmentRetry);
			std::unique_ptr<FragmentReadCommand> FragmentRead(new FragmentReadCommand);
			QueueCommand(1, *FragmentRead);
			return;
		}
		else {
			// Maximum retries exceeded; notify user and disconnect
			FragmentRetry = 0;
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(ORANGE, L"Communication with Cronus Zen failed; device is unresponsive!\r\n");
			DisconnectFromDevice();
			return;
		}
	}
	else {
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
			}
			else {
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
			case 1:
				m_Settings.LightbarPercent = Value;
				break;
			case 2:
				m_Settings.OperationalMode = static_cast<OperationalModeList>(Value);
				if (m_Settings.OperationalMode <= OperationalModeList::TournamentMode) {
					App->GetMainDialog().PrintTimestamp();
					App->GetMainDialog().PrintText(YELLOW, L"Operational mode is set to %ws mode.\r\n", m_OperationalMode[m_Settings.OperationalMode].c_str());
				}
				break;
			case 3:
				m_Settings.OutputMode = static_cast<OutputModeList>(Value);
				if (m_Settings.OutputMode <= OutputModeList::PlayStation5) {
					App->GetMainDialog().PrintTimestamp();
					App->GetMainDialog().PrintText(YELLOW, L"Emulator output protocol is set to %ws.\r\n", m_OutputMode[m_Settings.OutputMode].c_str());
				}
				break;
			case 4:
				m_Settings.RemoteSlot = static_cast<RemoteSlotChangeList>(Value);
				if (m_Settings.RemoteSlot <= RemoteSlotChangeList::PS_L3) {
					App->GetMainDialog().PrintTimestamp();
					App->GetMainDialog().PrintText(YELLOW, L"Remote slot change is set to %ws.\r\n", m_RemoteSlot[m_Settings.RemoteSlot].c_str());
				}
				break;
			case 5:
				if (m_Settings.Ps4Specialty = Value) {
					App->GetMainDialog().PrintTimestamp();
					App->GetMainDialog().PrintText(YELLOW, L"PS4 Specialty is enabled.\r\n");
				}
				break;
			case 6:
				if (m_Settings.RemotePlay = Value) {
					App->GetMainDialog().PrintTimestamp();
					App->GetMainDialog().PrintText(YELLOW, L"Remote Play is enabled.\r\n");
				}
				break;
			}
		}
	}

	// RequestMkFile is the next command in the sequence
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(GRAY, L"Requesting mouse and keyboard settings...\r\n");
	
	std::unique_ptr<RequestMkFileCommand> RequestMkFile(new RequestMkFileCommand);
	QueueCommand(1, *RequestMkFile);
}

// Handle the GetFirmware command which includes the firmware version and checksum
VOID CronusZen::OnGetFirmware(VOID)
{
	// Ensure the parse buffer contains enough data
	if (m_ParseBuffer->Size() < 12)
		throw UnexpectedSize(L"GetFirmware", m_ParseBuffer->Size(), 12);

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
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(YELLOW, L"Device is using firmware: %ws.\r\n", m_Firmware.c_str());

	// Validate the firmware version
	if (!m_SemanticVersion->IsBeta()) {
		// Notify user the recommendation to downgrade when using this program
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(ORANGE, L"Certain features are unavailable with this firmware version.\r\n");
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(PINK, L"Full appplication functionality requires a firmware modification.\r\n");
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(PINK, L"Go to \'Firmware\' > \'Install Compatible Firmware\' for instructions.\r\n", m_Firmware.c_str());
		App->GetMainDialog().DisplaySupportInfo();
	}
}

// Handle the GetSerial command which returns the device serial number
VOID CronusZen::OnGetSerial(VOID)
{
	// Ensure the parse buffer contains enough data for a valid serial number
	if (m_ParseBuffer->Size() < 32)
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
			m_Checksum[0] ^= m_Serial[4];
			m_Checksum[1] ^= m_Serial[0];
			m_Checksum[2] ^= m_Serial[2];
			m_Checksum[3] ^= m_Serial[6];
		}
		else {
			m_Checksum[0] ^= m_Serial[2];
			m_Checksum[1] ^= m_Serial[4];
			m_Checksum[2] ^= m_Serial[6];
			m_Checksum[3] ^= m_Serial[0];
		}
	}

	// Send next command
	std::unique_ptr<ExclusionListReadCommand> ExclusionListRead(new ExclusionListReadCommand);
	QueueCommand(1, *ExclusionListRead);

}

// Handle the GetStatus command to determine the state of the Cronus Zen
VOID CronusZen::OnGetStatus(VOID)
{
	// Ensure the parse buffer contains enough data for a valid serial number
	if (m_ParseBuffer->Size() < sizeof(DeviceStatus))
		throw UnexpectedSize(L"GetStatus", m_ParseBuffer->Size(), sizeof(DeviceStatus));

	// Allocate DeviceStatus structure
	std::unique_ptr<DeviceStatus> Status(new DeviceStatus);

	// Extract DeviceStatus structure from the read command
	m_ParseBuffer->ExtractData(Status.get(), sizeof(DeviceStatus));

	if (Status->Error) {
		// TODO: display some sort of error message here
		return;
	}

	// Determine next step based on the command the status was requested for
	switch (Status->Command)
	{
	case CronusZen::REQUESTMKFILE:
		std::unique_ptr<RequestAttachedDevicesCommand> RequestAttachedDevices(new RequestAttachedDevicesCommand);
		QueueCommand(1, *RequestAttachedDevices);
		break;
	}
}

// Handle the ReadSlotsCfg command for processing slot data
VOID CronusZen::OnReadSlotsCfg(VOID)
{
	// Static variable to track read slots config retry requests
	static UCHAR ReadSlotsCfgRetry = 0;

	// Notify user that the slots configuration is being processed
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(TEAL, L"Processing slots configuration data...\r\n");

	// Validate length of slots config and retry if necessary
	// - Ensures that the received slots config data is the expected size (4064 bytes)
	// - If the size is incorrect, it initiates a retry command up to 3 times
	if (m_PayloadLength != 4064) {
		if (ReadSlotsCfgRetry++ < 3) {
			// Retry read slots config command and notify user
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(ORANGE, L"Retrying read slots config (retry attempt %u of 3)...\r\n", ReadSlotsCfgRetry);

			// Create read slots config command object
			std::unique_ptr<ReadSlotsCfgCommand> ReadSlotsCfg(new ReadSlotsCfgCommand);

			// Send the read slots config command
			QueueCommand(1, *ReadSlotsCfg);

			return; // Failure
		}
		else {
			// Reset retry counter
			ReadSlotsCfgRetry = 0;

			// Notify user that the device is unresponsive
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(ORANGE, L"Communication with Cronus Zen failed; device is unresponsive!\r\n");

			// Disconnect due to unresponsiveness
			DisconnectFromDevice();

			return; // Failure
		}
	}
	else {
		ReadSlotsCfgRetry = 0;
	}

	// Allocate slots config data
	m_SlotConfig = std::make_unique<SlotConfigData[]>(8);

	// Extract slots config data
	m_ParseBuffer->ExtractData(m_SlotConfig.get(), 8 * sizeof(SlotConfigData));

	// Initialize variables used to track slot usage and storage
	UCHAR TotalSlots = 0;
	UINT TotalBytes = 0;

	// Iterate through all 8 slots to gather usage information
	for (unsigned i = 0; i < 8; i++) {
		// Get bytecode length for the current slot
		UINT ByteCodeLength = m_SlotConfig[i].ByteCodeLength;

		// If the slot is occupied (bytecode length is non-zero)
		if (ByteCodeLength--) {
			// Print information about the slot's contents
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(YELLOW, L"Slot #%u has %ws \'%ws\' using %u bytes.\r\n",
				m_SlotConfig[i].Slot - 0x2f,
				m_SlotConfig[i].GamepackID == 0xffff ? L"script" : L"gamepack",
				App->AnsiToUnicode((CONST PCHAR)m_SlotConfig[i].Title).c_str(),
				ByteCodeLength);

			// Update total slot and byte counts
			TotalSlots++;
			TotalBytes += ByteCodeLength;
		}
	}

	// Print a summary based on the slots usage information
	if (!TotalBytes) {
		// No scripts/gamepacks found
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(YELLOW, L"There are no scripts/gamepacks currently on your device.\r\n");
	}
	else {
		// Print slots usage information
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(YELLOW, L"%u slot%ws using %u bytes of storage with %u bytes of storage available.\r\n", TotalSlots, TotalSlots > 1 ? L"s" : L"", TotalBytes, 262136 - TotalBytes);
	}

	// Update main window
	App->GetMainDialog().UpdateSlotsData(TotalSlots, TotalBytes);
	App->GetMainDialog().UpdateFeatureAvailability(TRUE);
}

// Handle the RequestAttachedDevices command for processing wired connection sand Bluetooth devices
VOID CronusZen::OnRequestAttachedDevices(VOID)
{
	// Static variable to track request attached devices retry requests
	static UCHAR RequestAttachedDevicesRetry = 0;

	// Validate length of attached devices and retry if necessary
	// - Ensures that the received attached devices data is the expected size (96 bytes)
	// - If the size is incorrect, it initiates a retry command up to 3 times
	if (m_PayloadLength != 96) {
		if (RequestAttachedDevicesRetry++ < 3) {
			// Retry read slots config command and notify user
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(ORANGE, L"Retrying request attached devices (retry attempt %u of 3)...\r\n", RequestAttachedDevicesRetry);

			// Allocate and queue the RequestAttachedDevices command
			std::unique_ptr<RequestAttachedDevicesCommand> RequestAttachedDevices(new RequestAttachedDevicesCommand);
			QueueCommand(1, *RequestAttachedDevices);

			return; // Failure
		}
		else {
			// Reset retry counter
			RequestAttachedDevicesRetry = 0;

			// Notify user of unresponsive device
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(ORANGE, L"Communication with Cronus Zen failed; device is unresponsive!\r\n");

			// Disconnect from device due to unresponsiveness
			DisconnectFromDevice();

			return; // Failure
		}
	}
	else {
		RequestAttachedDevicesRetry = 0;
	}

	// Allocate the fragment data
	m_AttachedDevices = std::make_unique<AttachedDevice[]>(12);

	// Extract fragment data
	m_ParseBuffer->ExtractData(m_AttachedDevices.get(), 96);

	// Determine if a device is a hub
	for (unsigned i = 0; i < 12; i++) {
		UCHAR DAddress = m_AttachedDevices[i].DAddress;
		for (unsigned j = 0; j < 12; j++) {
			if (m_AttachedDevices[j].DAddress != DAddress && m_AttachedDevices[j].Parent == DAddress && DAddress != 0)
				m_AttachedDevices[i].IsHub = TRUE;
		}
	}

	// RequestMkFile is the next command in the sequence
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(GRAY, L"Requesting slots configuration...\r\n");

	std::unique_ptr<ReadSlotsCfgCommand> ReadSlotsCfg(new ReadSlotsCfgCommand);
	QueueCommand(1, *ReadSlotsCfg);
}

// Handle the RequestMkFile command to process the mouse and keyboard configuration
VOID CronusZen::OnRequestMkFile(VOID)
{
	// Ensure the parse buffer contains enough data for a valid mouse and keyboard settings file
	if (m_PayloadLength == 1283) {
		// Notify user that the valid is indeed valid and will be processed
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(TEAL, L"Processing mouse and keyboard settings data...\r\n");

		// Create MkFile object to manage the mouse and keyboard settings file
		m_MkFile = std::make_unique<MkFile>();

		// Set the MkFile data using the payload
		m_MkFile->SetMkFileData(m_ParseBuffer->Buffer() + 4, m_PayloadLength);

		// Notify user of the file being used
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(YELLOW, L"%ws file type (version %ws) using profile \'%ws\' (revision %ws).\r\n",
			m_MkFile->GetType().c_str(),
			m_MkFile->GetVersion().c_str(),
			m_MkFile->GetName().c_str(),
			m_MkFile->GetRevision().c_str());
	}
	else {
		// Notify user that the mouse and keyboard settings file returned is of an unexpected length
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(ORANGE, L"Unable to process mouse and keyboard settings; unexpected size returned (%u bytes).\r\n", m_PayloadLength);
	}

	// Allocate and queue the GetStatus command
	std::unique_ptr<GetStatusCommand> GetStatus(new GetStatusCommand);
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

VOID CronusZen::SendInitialCommunication(VOID)
{
	std::unique_ptr<StreamIoStatusCommand> StreamIoStatus(new StreamIoStatusCommand(CronusZen::Off));
	std::unique_ptr<ExitApiModeCommand> ExitApiMode(new ExitApiModeCommand);
	std::unique_ptr<UnloadGpcCommand> UnloadGpc(new UnloadGpcCommand);
	std::unique_ptr<CircleTestCommand> CircleTest(new CircleTestCommand(0, 0, 0));
	std::unique_ptr<GetFirmwareCommand> GetFirmware(new GetFirmwareCommand);
	std::unique_ptr<GetSerialCommand> GetSerial(new GetSerialCommand);

	// Notify user of the state of the connection
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(GRAY, L"Initiating communication with the Cronus Zen...\r\n");

	QueueCommand(1, *StreamIoStatus);
	QueueCommand(1, *ExitApiMode);
	QueueCommand(1, *UnloadGpc);
	QueueCommand(1, *CircleTest);
	QueueCommand(1, *GetFirmware);
	QueueCommand(1, *GetSerial);
}