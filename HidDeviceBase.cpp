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
	try
	{
		// Allocate memory for HID device preparsed data and capabilities
		m_PreparsedData = std::make_unique<PHIDP_PREPARSED_DATA>();
		m_Capabilities = std::make_unique<HIDP_CAPS>();

		// Retrieve preparsed data from the HID device
		if (!HidD_GetPreparsedData(m_Device->GetHandle(), m_PreparsedData.get()))
			throw std::wstring(L"An error occured while querying HID device preparsed collection data.");

		// Retrieve HID device capabilities
		if (HidP_GetCaps(*m_PreparsedData.get(), m_Capabilities.get()) != HIDP_STATUS_SUCCESS)
			throw std::wstring(L"An error occored while querying HID device capabilities.");

		// Store input and output report lengths as provided by capabilities
		m_ReadInputLength = m_Capabilities.get()->InputReportByteLength;
		m_WriteOutputLength = m_Capabilities.get()->OutputReportByteLength;

		// Allocate receive buffer based on the input report length
		m_ReceiveBuffer = std::make_unique<BYTE[]>(m_ReadInputLength);

		return TRUE; // Signal success
	}
	catch (CONST std::bad_alloc&) {
		// Handle memory allocation error
		App->DisplayError(L"Insufficient memory available to complete the required operation.");
	}
	catch (CONST std::wstring& CustomMessage) {
		// Handle other errors with custom messages
		App->DisplayError(CustomMessage);
	}

	return FALSE; // Signal failure
}

VOID HidDeviceBase::DisconnectFromDevice(VOID)
{
	// Determine if a device is currently located
	if (m_Device.get()) {
		// Close the connection to the device
		m_Device->Close();

		// Notify user that the device has been disconnected
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(RED, L"Disconnected from the Cronus Zen!\r\n");
	}

	// Free HID device preparsed data
	if(m_PreparsedData.get())
		HidD_FreePreparsedData(*m_PreparsedData.get());

	// Reset unique_ptr's
	m_PreparsedData.reset();
	m_Capabilities.reset();
	m_Device.reset();
}

// Method for scanning and calling to open a connection to the device
VOID HidDeviceBase::ConnectToDevice(VOID)
{
	// Query the target device
	if (FindDevice()) {
		// Notify user that the device has been located
		App->GetMainDialog().PrintTimestamp();
		App->GetMainDialog().PrintText(GRAY, L"Attempting to open a connection to the Cronus Zen located at %ws...\r\n", GetDevicePath().c_str());

		// Reset device file handle
		m_Device.reset();

		try
		{
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

		}
		catch (CONST std::wstring& CustomMessage) {
			// Display the custom error message to the user
			App->DisplayError(CustomMessage);

			// Disconnect from the device
			DisconnectFromDevice();
		}
		catch (CONST std::bad_alloc&) {
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

VOID HidDeviceBase::AsynchronousWrite(_In_ CONST PUCHAR Data)
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

	delete[] Data;
}