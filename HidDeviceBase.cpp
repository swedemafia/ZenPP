#include "Precomp.h"

HidDeviceBase::HidDeviceBase() :
	DeviceLocatorBase(L"Cronus Bridge", L"HID")
{

}

// Destructor for cleaning up any open connections
HidDeviceBase::~HidDeviceBase()
{
	// Disconnect from device
	DisconnectFromDevice();
}

// Method for querying HID device preparsed data and capabilities
BOOL HidDeviceBase::QueryHIDDeviceCapabilities(VOID)
{
	// Allocate memory for HID device preparsed data and capabilities
	PHIDP_PREPARSED_DATA PreparsedData = NULL;
	HIDP_CAPS Capabilities = { NULL };

	try {
		// Retrieve preparsed data from the HID device
		if (!HidD_GetPreparsedData(m_Device->GetHandle(), &PreparsedData))
			throw std::wstring(L"An error occured while querying HID device preparsed collection data.");

		// Retrieve HID device capabilities
		if (HidP_GetCaps(PreparsedData, &Capabilities) != HIDP_STATUS_SUCCESS)
			throw std::wstring(L"An error occored while querying HID device capabilities.");

		// Store input and output report lengths as provided by capabilities
		m_ReadInputLength = Capabilities.InputReportByteLength;
		m_WriteOutputLength = Capabilities.OutputReportByteLength;

		// Allocate receive buffer based on the input report length
		m_ReceiveBuffer = std::make_unique<BYTE[]>(m_ReadInputLength);

		// Free preparsed data
		HidD_FreePreparsedData(PreparsedData);

		return TRUE; // Signal success
	} catch (CONST std::bad_alloc&) {
		// Handle memory allocation error
		App->DisplayError(L"Insufficient memory available to complete the required operation.");
	} catch (CONST std::wstring& CustomMessage) {
		// Handle other errors with custom messages
		App->DisplayError(CustomMessage);
	}

	// Free preparsed data
	if(PreparsedData)
		HidD_FreePreparsedData(PreparsedData);

	return FALSE; // Signal failure
}

VOID HidDeviceBase::DisconnectFromDevice(VOID)
{
	// Close out the IOCP
	CancelIocp();

	// Free up File object used to connect to the device
	if (m_Device.get()) {
		// If the app is being terminated, no need to inform the user of the disconnection
		if (!App->IsQuitting() && (m_Device->GetHandle() != INVALID_HANDLE_VALUE)) {
			App->GetMainDialog().PrintTimestamp();
			App->GetMainDialog().PrintText(RED, L"The connection to the Cronus Zen has been terminated.\r\n");
			App->GetMainDialog().UpdateSlotsData(0, 1);
			App->GetMainDialog().UpdateFeatureAvailability(FALSE);
		}
		m_Device->Close();
		m_Device.reset();
	}

	// Suspend thread
	SuspendThread(GetIocpThreadHandle());

	if (App->GetCronusZen().GetConnectionState() == CronusZen::Connected) {
		// Update device state
		App->GetCronusZen().SetConnectionState(CronusZen::Disconnected);
	}
}

// Method for scanning and calling to open a connection to the device
VOID HidDeviceBase::ConnectToDevice(VOID)
{
	static int counter = 0;

	// Query the target device
	if (FindDevice() && !m_Device.get()) {
		// Notify user that the device has been located
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(GRAY, L"Attempting to connect to your Cronus Zen...\r\n");

		if (App->GetCronusZen().GetConnectionState() == CronusZen::Disconnected) {
			// Update device state
			App->GetCronusZen().SetConnectionState(CronusZen::Connecting);
		}

		try {
			// Create new device file handle
			m_Device = std::make_unique<File>(GetDevicePath(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, FALSE);

			// Attempt to open a connection to the device
			if (!m_Device->Open())
				throw std::wstring(L"An error occured while opening a connection to the Cronus Zen.");

			// Associate device handle with an IOCP
			if (!AssociateWithIocp(m_Device->GetHandle()))
				throw std::wstring(L"An error occured while associating the device handle with an I/O completion port.");

			// Resume the IOCP event thread
			if (ResumeThread(GetIocpThreadHandle()) == -1)
				throw std::wstring(L"An error occured while resuming the I/O completion port event thread.");

			// Post completion status to IOCP to signal a successful connection
			if (!PostQueuedCompletionStatus(GetIocpHandle(), 0, IocpCompletionKey::Connect, GetOverlappedConnect()))
				throw std::wstring(L"An error occured while posting the connection completion status to the I/O completion port.");

		} catch (CONST std::wstring& CustomMessage) {
			// Display the custom error message to the user
			App->DisplayError(CustomMessage);

			// Disconnect from the device
			DisconnectFromDevice();

		} catch (CONST std::bad_alloc&) {
			// Display a notice regarding a bad allocation
			App->DisplayError(L"Insufficient memory available to complete the required operation.");
		}
	}
}

VOID HidDeviceBase::AsynchronousRead(VOID)
{
	// Reset the overlapped structure for the read operation
	ZeroMemory(GetOverlappedRead(), sizeof(OVERLAPPED));

	// Initiate asynchronous read request from the device
	if (!ReadFile(m_Device->GetHandle(), m_ReceiveBuffer.get(), m_ReadInputLength, nullptr, GetOverlappedRead())) {
		// Check for errors (excluding ERROR_IO_PENDING, which indicates asynchronous completion/success)
		if (GetLastError() != ERROR_IO_PENDING) {
			// Display error message and disconnect as the operation has failed
			App->DisplayError(L"An error occured while reading data from the device.");
			DisconnectFromDevice();
		}
	}
}

VOID HidDeviceBase::AsynchronousWrite(CONST PUCHAR Data)
{
	// Reset the overlapped structure for the write operation
	ZeroMemory(GetOverlappedWrite(), sizeof(OVERLAPPED));

	// Initiate asynchronous write request to the device
	if (!WriteFile(m_Device->GetHandle(), Data, m_WriteOutputLength, nullptr, GetOverlappedWrite())) {
		// Check for errors (excluding ERROR_IO_PENDING, which indicates asynchronous completion/success)
		if (GetLastError() != ERROR_IO_PENDING) {
			// Display error message and disconnect as the operation has failed
			App->DisplayError(L"An error occured while writing data to the device.");
			DisconnectFromDevice();
		}
	}

	// Free memory that was previously allocated prior to being passed to this method
	delete[] Data;
}