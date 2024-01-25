#include "Precomp.h"

WinUsbBase::~WinUsbBase()
{
	WinUsbCloseConnection();
}

VOID WinUsbBase::WinUsbCloseConnection(VOID)
{
	if (m_InterfaceHandle) {
		WinUsb_Free(m_InterfaceHandle);
		m_InterfaceHandle = NULL;
	}
}

BOOL WinUsbBase::WinUsbControlTransfer(CONST UCHAR RequestType, CONST UCHAR Request, CONST USHORT Value, CONST USHORT Index)
{
	return WinUsbControlTransfer(RequestType, Request, Value, Index, 0, 0);
}

BOOL WinUsbBase::WinUsbControlTransfer(CONST UCHAR RequestType, CONST UCHAR Request, CONST USHORT Value, CONST USHORT Index, CONST USHORT Length, _Inout_ PBYTE Data)
{
	WINUSB_SETUP_PACKET SetupPacket = { RequestType, Request, Value, Index, Length };
	ULONG LengthTransferred = 0;

	if (!WinUsb_ControlTransfer(m_InterfaceHandle, SetupPacket, Data, (ULONG)Length, &LengthTransferred, NULL))
		throw std::wstring(L"An error occured while executing a control transfer on the USB device.");

	return TRUE;
}

std::wstring WinUsbBase::WinUsbGetStringDescriptor(CONST UCHAR Index)
{
	std::shared_ptr<UCHAR[]> Buffer(new UCHAR[256]{ 0 }); // TODO: unique?
	ULONG LengthTransferred = 0;

	// Get string descriptor
	if (!WinUsb_GetDescriptor(m_InterfaceHandle, USB_STRING_DESCRIPTOR_TYPE, Index, 0, Buffer.get(), 256, &LengthTransferred)) {
		App->DisplayError(L"Failed to get USB string descriptor (" + std::to_wstring(Index) + L").");
		return L"";
	}

	if (Buffer[0] - 2 <= 0)
		return L"";

	return std::wstring(reinterpret_cast<PWCHAR>(Buffer.get() + 2), LengthTransferred / sizeof(WCHAR) - 1);
}

BOOL WinUsbBase::WinUsbOpenConnection(CONST std::wstring& DevicePath)
{
	try
	{
		// Attempt to open device handle
		m_Device = std::make_unique<File>(DevicePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, FALSE);

		if(!m_Device->Open())
			throw std::wstring(L"An error occured while opening a handle to the device.");

		// Initialize WinUsb interface
		if (!WinUsb_Initialize(m_Device->GetHandle(), &m_InterfaceHandle))
			throw std::wstring(L"An error occured while initializing the WinUsb interface.");

	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
		return FALSE;
	}
	catch (CONST std::bad_alloc&)
	{
		App->DisplayError(L"An error occured while creating the File object.");
		return FALSE;
	}

	return TRUE;
}

USB_DEVICE_DESCRIPTOR WinUsbBase::WinUsbGetDeviceDescriptor(VOID)
{
	USB_DEVICE_DESCRIPTOR DeviceDescriptor = { NULL };
	ULONG LengthTransferred = 0;

	try
	{
		// Query device descriptor
		if (!WinUsb_GetDescriptor(m_InterfaceHandle, USB_DEVICE_DESCRIPTOR_TYPE, 0, 0, (PUCHAR)&DeviceDescriptor, sizeof(DeviceDescriptor), &LengthTransferred))
			throw std::wstring(L"An error occured while querying the USB device descriptor.");

		if (LengthTransferred != sizeof(DeviceDescriptor))
			throw std::wstring(L"An error occured while querying the USB device descriptor; incomplete data was returned.");

	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
	}

	return DeviceDescriptor;
}