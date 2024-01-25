#ifndef CLIENTS_DFUCONTROLLER_H
#define CLIENTS_DFUCONTROLLER_H

#ifdef _MSC_VER
#pragma once
#endif

class DfuController : public WinUsbBase {
public:
	// Enumerations for DFU states, errors and requests
	enum DfuError : BYTE {
		Ok = 0,
		Target,
		FileError,
		Write,
		Erase,
		CheckErased,
		Prog,
		Verify,
		Address,
		NotDone,
		Firmware,
		Vendor,
		UsbReset,
		PowerOnReset,
		Unknown,
		Stalled
	};

	enum DfuRequest : BYTE {
		DFU_DETACH = 0,
		DFU_DOWNLOAD,
		DFU_UPLOAD,
		DFU_GETSTATUS,
		DFU_CLEARSTATUS,
		DFU_GETSTATE,
		DFU_ABORT,
		DFU_UNLOCK
	};

	enum DfuState : BYTE {
		AppIdle = 0,
		AppDetach,
		Idle,
		DownloadSync,
		DownloadBusy,
		DownloadIdle,
		ManifestSync,
		Manifest,
		ManifestWaitReset,
		UploadIdle,
		Error,
		Locked,
		UploadSync = 145,
		UploadBusy
	};

	// Structure for DFU status information
	struct DfuStatus {
		DfuError Status;
		UINT PollTimeout;
		DfuState State;
		BYTE String;
	};

	// Public methods for device information retrieval
	CONST std::wstring& GetBootloaderVersion(VOID) CONST;
	CONST std::wstring& GetFirmwareVersion(VOID) CONST;
	CONST std::wstring& GetManufacturer(VOID) CONST;
	CONST std::wstring& GetProduct(VOID) CONST;
	CONST std::wstring& GetSerialNumber(VOID) CONST;

	// Public methods for DFU operations
	VOID DfuAbort(VOID);
	VOID DfuDownload(_In_ CONST USHORT Block, _In_ CONST PUCHAR Data, _In_ CONST USHORT Size);
	VOID DfuEnterUserApplication(VOID);
	VOID DfuUnlock(_In_ CONST PUCHAR LockBlock);

	// Public method for device descriptor retrieval
	VOID GetDeviceDescriptors(VOID);

protected:
	// Protected methods for DFU status handling
	DfuStatus DfuClearErrorAndAbort(_In_ CONST DfuState Expected);
	DfuStatus DfuGetStatus(VOID);
	VOID DfuClearStatus(VOID);

private:
	// Private members for device information
	std::wstring m_BootloaderVersion, m_FirmwareVersion, m_Manufacturer, m_Product, m_SerialNumber;
	USB_DEVICE_DESCRIPTOR m_DeviceDescriptor = { NULL };
};

#endif
