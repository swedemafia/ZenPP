#ifndef BASECLASSES_WINUSBBASE_H
#define BASECLASSES_WINUSBBASE_H

#ifdef _MSC_VER
#pragma once
#endif

class WinUsbBase
{
public:
	// Destructor for cleanup
	~WinUsbBase();

	// Public methods for WinUSB connection management
	BOOL WinUsbOpenConnection(_In_ CONST std::wstring& DevicePath);
	VOID WinUsbCloseConnection(VOID);

protected:
	// Protected methods for WinUSB control transfers and descriptor retrieval
	BOOL WinUsbControlTransfer(_In_ CONST UCHAR RequestType, _In_ CONST UCHAR Request, _In_ CONST USHORT Value, _In_ CONST USHORT Index);
	BOOL WinUsbControlTransfer(_In_ CONST UCHAR RequestType, _In_ CONST UCHAR Request, _In_ CONST USHORT Value, _In_ CONST USHORT Index, _In_ CONST USHORT Length, _Inout_ PBYTE Data);
	std::wstring WinUsbGetStringDescriptor(_In_ CONST UCHAR Index);
	USB_DEVICE_DESCRIPTOR WinUsbGetDeviceDescriptor(VOID);

private:
	// Private members for WinUSB resources
	std::unique_ptr<File> m_Device;
	WINUSB_INTERFACE_HANDLE m_InterfaceHandle = nullptr;
};

#endif