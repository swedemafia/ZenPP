#ifndef BASECLASSES_DEVICELOCATORBASE_H
#define BASECLASSES_DEVICELOCATORBASE_H

#ifdef _MSC_VER
#pragma once
#endif

#undef DEFINE_DEVPROPKEY
#define DEFINE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const DEVPROPKEY DECLSPEC_SELECTANY name = { { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }, pid }
DEFINE_DEVPROPKEY(DEVPKEY_Device_BusReportedDeviceDesc, 0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 4);

class DeviceLocatorBase
{
public:
	// Constructor and destructor
	explicit DeviceLocatorBase(_In_ CONST std::wstring& DeviceName, _In_ CONST std::wstring& DeviceType);
	~DeviceLocatorBase() = default;

	// Public methods for obtaining device GUID
	GUID GetGUID(VOID) CONST;

protected:
	// Protected methods for device management
	BOOL FindDevice(VOID);
	CONST std::wstring& GetDevicePath(VOID) CONST;

private:
	// Private members for device 
	GUID m_GUID;
	std::wstring m_BusReportedDeviceDescription;
	std::wstring m_DeviceDescription;
	std::wstring m_DeviceName;
	std::wstring m_DevicePath;
	std::wstring m_DeviceType;
	std::wstring m_HardwareID;
	std::wstring m_ProductID;
	std::wstring m_VendorID;

	// Private helper method for device path discovery
	BOOL FindDevicePath(VOID);
};

#endif
