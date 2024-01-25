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
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(RED, L"The Cronus Zen has terminated the connection.\r\n");
	return TRUE;
}

BOOL CronusZen::OnRead(CONST DWORD BytesRead)
{
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(PURPLE, L"Received %u bytes.\r\n", BytesRead);
	return TRUE;
}

BOOL CronusZen::OnWrite(CONST DWORD BytesWritten)
{
	App->GetMainDialog().PrintTimestamp();
	App->GetMainDialog().PrintText(YELLOW, L"Sent %u bytes.\r\n", BytesWritten);

	// Remove command from queue
	m_Queue.pop_front();

	// Check if there are any remaining commands queued up and send the next one, if so
	if (!m_Queue.empty()) {
		AsynchronousWrite(m_Queue.front());
	}

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

	QueueCommand(1, *StreamIoStatus.get());
	QueueCommand(1, *ExitApiMode.get());
	QueueCommand(1, *UnloadGpc.get());
	QueueCommand(1, *CircleTest.get());
	QueueCommand(1, *GetFirmware.get());
	Sleep(100);
	QueueCommand(1, *GetSerial.get());
}