#include "Precomp.h"

CONST std::wstring& DfuController::GetBootloaderVersion(VOID) CONST
{
	return m_BootloaderVersion;
}


CONST std::wstring& DfuController::GetFirmwareVersion(VOID) CONST
{
	return m_FirmwareVersion;
}

CONST std::wstring& DfuController::GetManufacturer(VOID) CONST
{
	return m_Manufacturer;
}

CONST std::wstring& DfuController::GetProduct(VOID) CONST
{

	return m_Product;
}

CONST std::wstring& DfuController::GetSerialNumber(VOID) CONST
{
	return m_SerialNumber;
}

// Method for clearing any errors and aborting, if necessary
DfuController::DfuStatus DfuController::DfuClearErrorAndAbort(CONST DfuController::DfuState Expected)
{
	DfuController::DfuStatus Status = DfuGetStatus();

	if (Status.State == DfuState::Error) {
		DfuClearStatus();
		Status = DfuGetStatus();
	}
	if (Status.State != Expected) {
		DfuAbort();
		Status = DfuGetStatus();
	}

	return Status;
}

// Method used for polling the status of the device during a transfer
DfuController::DfuStatus DfuController::DfuGetStatus(VOID)
{
	UCHAR Data[6] = { 0 };
	WinUsbControlTransfer(0xA1, DFU_GETSTATUS, 0, 3, 6, Data);
	return DfuController::DfuStatus{ (DfuError)Data[0], ((UINT)Data[1] | (UINT)Data[2] << 8 | (UINT)Data[3] << 16), (DfuState)Data[4], Data[5] };
}

// Method used to abort a DFU download
VOID DfuController::DfuAbort(VOID)
{
	WinUsbControlTransfer(0x21, DFU_ABORT, 0, 3);
}

// Method used to clear the status of the connection state
VOID DfuController::DfuClearStatus(VOID)
{
	WinUsbControlTransfer(0x21, DFU_CLEARSTATUS, 0, 3);
}

// Method for downloading data to the client device (writing firmware)
VOID DfuController::DfuDownload(CONST USHORT Block, CONST PUCHAR Data, CONST USHORT Size)
{
	WinUsbControlTransfer(0x21, DFU_DOWNLOAD, Block, 3, Size, Data);
}

// Method used to enter the user application (load and execute the firmware)
VOID DfuController::DfuEnterUserApplication(VOID)
{
	WinUsbControlTransfer(0x41, DFU_GETSTATUS, 1, 0);
}

// Method used to request a DFU unlock
VOID DfuController::DfuUnlock(CONST PUCHAR LockBlock)
{
	WinUsbControlTransfer(0x21, DFU_UNLOCK, 0, 0, 0x70, LockBlock);
}

// Method to obtain the required device descriptors including manufacturer; product name; serial number; bootloader and firmware versions
VOID DfuController::GetDeviceDescriptors(VOID)
{
	m_DeviceDescriptor = WinUsbGetDeviceDescriptor();

	BYTE Index = m_DeviceDescriptor.iManufacturer;

	// Manufacturer
	if (Index > 0)
		m_Manufacturer = WinUsbGetStringDescriptor(Index);

	// Product
	Index = m_DeviceDescriptor.iProduct;
	if (Index > 0)
		m_Product = WinUsbGetStringDescriptor(Index);

	// Serial Number
	Index = m_DeviceDescriptor.iSerialNumber;
	if (Index > 0)
		m_SerialNumber = WinUsbGetStringDescriptor(Index);

	// Bootloader & Firmware Version
	m_BootloaderVersion = WinUsbGetStringDescriptor(6);
	m_FirmwareVersion = WinUsbGetStringDescriptor(7);
}