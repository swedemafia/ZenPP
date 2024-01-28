#ifndef BASECLASSES_HIDDEVICEBASE_H
#define BASECLASSES_HIDDEVICEBASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "IoCompletionPortBase.h"

class HidDeviceBase : public DeviceLocatorBase, IoCompletionPortBase
{
public:
	// Constructor to initialize the device locator class
	HidDeviceBase();

	// Destructor for cleanup
	~HidDeviceBase();

	// Public method for device operations
	VOID ConnectToDevice(VOID);
	VOID DisconnectFromDevice(VOID);

protected:
	std::unique_ptr<BYTE[]> m_ReceiveBuffer;

	// Protected methods for querying HID device information
	BOOL QueryHIDDeviceCapabilities(VOID);

	// Protected method for asynchronous read/write HID device operations
	VOID AsynchronousRead(VOID);
	VOID AsynchronousWrite(_In_ CONST PUCHAR Data);

private:
	std::unique_ptr<File> m_Device;
	USHORT m_ReadInputLength = 0;
	USHORT m_WriteOutputLength = 0;
};

#endif
