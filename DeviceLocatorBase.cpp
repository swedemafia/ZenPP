#include "Precomp.h"

// Constructor for DeviceLocatorBase, initializing device name, type and GUID
DeviceLocatorBase::DeviceLocatorBase(CONST std::wstring& DeviceName, CONST std::wstring& DeviceType) :
	m_DeviceName(DeviceName), m_DeviceType(DeviceType)
{
	// Initialize GUI based on device type
	if (DeviceType == L"HID") {
		// Retrieve GUID for HID devices
		HidD_GetHidGuid(&m_GUID);
	} else {
		// Attempt to create GUID for non-HID devices
		if (CLSIDFromString(L"{960bd7d9-a9a4-4922-9cab-169a1ce9bb58}", &m_GUID) != NOERROR) {
			// Display error if GUID creation fails
			App->DisplayError(L"An error occured while initializing the USB GUID descriptor.");
		}
	}
}

BOOL DeviceLocatorBase::FindDevice(VOID)
{
	// Get handle to device information set
	auto DevInfo = std::unique_ptr<void, decltype(&SetupDiDestroyDeviceInfoList)>(
		SetupDiGetClassDevs(NULL, L"USB", nullptr, DIGCF_PRESENT | DIGCF_ALLCLASSES),
		SetupDiDestroyDeviceInfoList
	);

	try {
		// Validate device info set
		if (DevInfo.get() == INVALID_HANDLE_VALUE)
			throw std::wstring(L"An error occured while querying SetupDiGetClassDevs.");

		DWORD MemberIndex = 0;
		SP_DEVINFO_DATA DeviceInfoData = { sizeof(SP_DEVINFO_DATA) };

		while (SetupDiEnumDeviceInfo(DevInfo.get(), MemberIndex++, &DeviceInfoData)) {
			DEVPROPTYPE PropertyType = { NULL };
			DWORD RequiredSize = 0;

			// Bus Reported Device Description
			if (!SetupDiGetDeviceProperty(DevInfo.get(), &DeviceInfoData, &DEVPKEY_Device_BusReportedDeviceDesc, &PropertyType, (PBYTE)m_BusReportedDeviceDescription.data(), 0, &RequiredSize, 0)) {
				m_BusReportedDeviceDescription.resize(RequiredSize / sizeof(WCHAR));
				if (!SetupDiGetDeviceProperty(DevInfo.get(), &DeviceInfoData, &DEVPKEY_Device_BusReportedDeviceDesc, &PropertyType, (PBYTE)m_BusReportedDeviceDescription.data(), m_BusReportedDeviceDescription.size() * sizeof(WCHAR), NULL, 0)) {
					continue;
				}
			}

			// Device Description
			if (!SetupDiGetDeviceRegistryProperty(DevInfo.get(), &DeviceInfoData, SPDRP_DEVICEDESC, &PropertyType, (PBYTE)m_DeviceDescription.data(), 0, &RequiredSize)) {
				m_DeviceDescription.resize(RequiredSize / sizeof(WCHAR));
				if (!SetupDiGetDeviceRegistryProperty(DevInfo.get(), &DeviceInfoData, SPDRP_DEVICEDESC, &PropertyType, (PBYTE)m_DeviceDescription.data(), m_DeviceDescription.size() * sizeof(WCHAR), NULL)) {
					break;
				}
			}

			// Hardware ID
			if (!SetupDiGetDeviceRegistryProperty(DevInfo.get(), &DeviceInfoData, SPDRP_HARDWAREID, &PropertyType, (PBYTE)m_HardwareID.data(), 0, &RequiredSize)) {
				m_HardwareID.resize(RequiredSize / sizeof(WCHAR));
				if (!SetupDiGetDeviceRegistryProperty(DevInfo.get(), &DeviceInfoData, SPDRP_HARDWAREID, &PropertyType, (PBYTE)m_HardwareID.data(), m_HardwareID.size() * sizeof(WCHAR), NULL)) {
					break;
				}
			}

			m_BusReportedDeviceDescription.pop_back();
			if(m_BusReportedDeviceDescription == m_DeviceName) {
				// Create a pattern to search for Vendor and Product IDs
				std::wregex Pattern(L"VID_(....)&PID_(....)");
				std::wsmatch Matches;

				if (std::regex_search(m_HardwareID, Matches, Pattern)) {
					m_VendorID = Matches[1].str();
					m_ProductID = Matches[2].str();
					return FindDevicePath();
				}

				break;
			}
		}

	} catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	}

	return FALSE;
}

BOOL DeviceLocatorBase::FindDevicePath(VOID)
{
	try {
		// Get handle to device information set
		auto DevInfo = std::unique_ptr<void, decltype(&SetupDiDestroyDeviceInfoList)>(
			SetupDiGetClassDevs(&m_GUID, nullptr, nullptr, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE),
			SetupDiDestroyDeviceInfoList
		);

		// Validate device info set 
		if (DevInfo.get() == INVALID_HANDLE_VALUE)
			throw std::wstring(L"An error occured while querying SetupDiGetClassDevs.");

		DWORD MemberIndex = 0;
		std::wstring ComparisonString = L"\\\\?\\" + m_DeviceType + L"#VID_" + m_VendorID + L"&PID_" + m_ProductID;
		SP_DEVICE_INTERFACE_DATA InterfaceData = {};
		InterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		// Enumerate device interfaces
		while (SetupDiEnumDeviceInterfaces(DevInfo.get(), NULL, &m_GUID, MemberIndex++, &InterfaceData)) {
			DWORD RequiredSize = 0;
			if (!SetupDiGetDeviceInterfaceDetail(DevInfo.get(), &InterfaceData, NULL, 0, &RequiredSize, NULL)) {
				if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
					throw std::wstring(L"An error occured while querying SetupDiGetDeviceInterfaceDetail (1).");
			}

			// Allocate device interface details
			PSP_DEVICE_INTERFACE_DETAIL_DATA InterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
			if (!InterfaceDetailData)
				throw std::wstring(L"An error occured while allocating memory for SetupDiGetDeviceInterfaceDetail.");

			InterfaceDetailData->cbSize = sizeof(int*) == 4 ? 6 : 8;

			// Retrieve device interface details
			if (!SetupDiGetDeviceInterfaceDetail(DevInfo.get(), &InterfaceData, InterfaceDetailData, RequiredSize, NULL, NULL)) {
				free(InterfaceDetailData);
				throw std::wstring(L"An error occured while querying SetupDiGetDeviceInterfaceDetail (2).");
			}

			// Compare device path and see if its our target
			if (_wcsnicmp(InterfaceDetailData->DevicePath, ComparisonString.c_str(), ComparisonString.size()) == 0) {
				m_DevicePath = InterfaceDetailData->DevicePath;
				free(InterfaceDetailData);
				return TRUE;
			}
		}

	} catch (CONST std::wstring& CustomMessage) {
		App->DisplayError(CustomMessage);
	}

	return FALSE;
}

// Method for returning the target GUID
GUID DeviceLocatorBase::GetGUID(VOID) CONST
{
	return m_GUID;
}

// Method for returning the resolved device path
CONST std::wstring& DeviceLocatorBase::GetDevicePath(VOID) CONST
{
	return m_DevicePath;
}