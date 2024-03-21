#ifndef CLIENTS_CRONUSZEN_H
#define CLIENTS_CRONUSZEN_h

#ifdef _MSC_VER
#pragma once
#endif

class MkFile;

class CronusZen : public HidDeviceBase
{
public:
	// Publically accessible enumeration for device state
	enum ConnectionState : UCHAR {
		Disconnected = 0,
		Connecting,
		Connected,
		ClearBluetooth,
		DeviceCleanup,
		FactoryReset,
		RefreshAttachedDevices,
		ProgramDevice,
		SoftReset,
		TogglePs4Specialty,
		ToggleRemotePlay,
		TurnOffController,
		UpdateConfig
	};

	// Publically accessible fragment identifier list
	enum FragmentIDList : UCHAR {
		FragmentUnused = 0,
		LightbarPercentage,
		OperationalMode,
		OutputMode,
		RemoteSlot,
		Ps4Specialty,
		RemotePlay
	};

	// Publically accessible operational mode identifiers
	enum OperationalModeList : UCHAR {
		WheelMode = 0,
		GamepadMode,
		TournamentMode
	};

	// Publically accessible emulator output protocol identifiers
	enum OutputModeList : UCHAR {
		Auto = 0,
		PlayStation3,
		Xbox360,
		PlayStation4,
		XboxOne,
		NintendoSwitch,
		PlayStation5
	};

	// Publically accessible remote slot change identifiers
	enum RemoteSlotChangeList : UCHAR {
		Disabled = 0,
		PS_Share,
		PS_L3,
	};

	// Publically accessible enumeration of command identifiers
	enum PacketID : UCHAR {
		// Reports
		INPUTREPORT = 0x01,
		OUTPUTREPORT = 0x02,
		MOUSEREPORT = 0x03,
		KEYBOARDREPORT = 0x04,
		NAVCONREPORT = 0x05,
		G13REPORT = 0x06,
		DEBUGREPORT = 0x07,
		PS5ADTREPORT = 0x08,

		// All other commands
		REQUESTIOSTATUS = 0x02,
		RUNSCRIPT = 0x03,
		APIMODE = 0x04,
		RESETDEVICE = 0x06,
		ENTERAPIMODE = 0x07,
		EXITAPIMODE = 0x08,
		UNLOADGPC = 0x09,
		CHANGESLOTA = 0x0a,
		CHANGESLOTB = 0x0b,
		TURNOFFCONTROLLER = 0x0c,
		CLEARBTCOMMAND = 0x0d,
		GETSTATUS = 0x20,
		STREAMIOSTATUS = 0xab,
		EXCLUSIONLISTREAD = 0xb0,
		EXCLUSIONLISTWRITE = 0xb1,
		CL = 0xb2,
		TOGGLEVMRUNS = 0xb3,
		GETZENETDATA = 0xc0,
		WRITEZENETDATA = 0xc1,
		DFUPREPAREDEVICE = 0xde,
		DFUUPDATEFIRMWARE = 0xdf,
		DEVICECLEANUP = 0xe2,
		GETCONFIG = 0xe3,
		FACTORYRESET = 0xe4,
		REQUESTATTACHEDDEVICES = 0xe5,
		REQUESTMKFILE = 0xe6,
		SENDSINGLEFRAGMENT = 0xe9,
		REFRESHEEPROM = 0xee,
		GETFW = 0xf0,
		GETSERIAL = 0xf1,
		FLASHGAMEPACK = 0xf2,
		FLASHCONFIG = 0xf3,
		SENDMKFILE = 0xf4,
		SAVEMKFINALIZECONFIG = 0xf6,
		FINALIZEMKFILE = 0xf7,
		CIRCLETEST = 0xf8,
		FRAGMENTSWRITE = 0xf9,
		FRAGMENTREAD = 0xfa,
		READSLOTSCFG = 0xfb,
		SETVMCTRL = 0xfc,
		READBYTECODE = 0xfd
	};

	// Enumeration for StreamIoStatus
	enum StreamIoStatusMask : UCHAR {
		Off = 0,
		InputReport = 1,
		OutputReport = 2,
		Mouse = 4,
		Keyboard = 8,
		Navcon = 16,
		G13 = 32,
		Debug = 64,
		Ps5Adt = 128,
	};

	enum SourceType : UCHAR {
		UnknownCfg = 0,
		GeneralCfg,
		MouseCfg,
		KeyboardCfg,
		NavconCfg,
		GenericCfg,
		G13Cfg,
		Cfgs = 16,
		UnusedCfg = 255
	};

	struct FragmentData {
		FragmentIDList ID;
		SourceType Source;
		UCHAR Value;
		UCHAR Value2;
	};

	VOID CheckNextWriteCommand(VOID);

	// Public method for creating a worker thread to perform a specific action
	VOID CreateWorkerThread(_In_ CONST ConnectionState& NewState);

	// Public methods for attached device information retrieval
	CONST UCHAR GetBluetoothDeviceCount(VOID) CONST;
	CONST UCHAR GetWiredDeviceCount(VOID) CONST;

	// Public methods for connection state operations
	CONST ConnectionState& GetConnectionState(VOID) CONST;
	VOID SetConnectionState(_In_ CONST ConnectionState& NewState);
	
	// Public methods for device state
	VOID SetFragment(_In_ CONST FragmentIDList& Fragment, _In_ CONST UCHAR NewValue);

	// Public method for firmware version retrieval
	SemanticVersion& GetFirmwareVersion(VOID) CONST;

	// Public method for queueing a command
	VOID QueueCommand(_In_ CONST UCHAR Count, _In_ CommandBase& Command);
	VOID QueueCommand(_In_ CONST UCHAR Count, _In_ CONST USHORT Size, _In_ CommandBase& Command);

	// Public method for slots configuration management
	VOID SlotsAdd(_In_opt_ CONST std::wstring& FullPathToBin);
	VOID SlotsRemove(_In_ CONST UINT Slot);
	VOID SlotsProgram(VOID);
	CONST UCHAR SlotsUsed(VOID) CONST;

protected:
	// Virtual callback methods for various IOCP operations
	BOOL OnConnect(VOID);
	BOOL OnDisconnect(VOID);
	BOOL OnRead(_In_ CONST DWORD BytesRead);
	BOOL OnWrite(_In_ CONST DWORD BytesWritten);

private:

	CONST std::wstring m_Console[9] = {
		L"", L"PlayStation 3", L"Xbox 360", L"PlayStation 4", L"Xbox One X|S", L"Nintendo Switch", L"PlayStation 5", L"", L"Wheel"
	};

	CONST std::wstring m_OperationalMode[3] = {
		L"wheel", L"normal", L"tournament"
	};

	CONST std::wstring m_OutputMode[7] = {
		L"auto", L"PlayStation 3", L"PC/Mobile/Xbox 360", L"PlayStation 4", L"Xbox One Series S|X", L"Nintendo Switch", L"PlayStation 5"
	};

	CONST std::wstring m_RemoteSlot[3] = {
		L"disabled", L"PS & Share / Xbox & View", L"PS & L3 / Xbox & LS"
	};

	enum CfgState : UCHAR {
		HIP = 0,
		ADS,
		AUX1,
		AUX2,
		AUX3,
		AUX4
	};

	enum ConsoleType : UCHAR {
		NoConsole,
		Ps3Console,
		Xb360Console,
		Ps4Console,
		Xb1Console,
		SwitchConsole,
		Ps5Console,
		Wheel = 8
	};

	enum ControllerType : UCHAR {
		NoController,
		Ps3 = 16,
		Xb360 = 32,
		Wii = 48,
		WiiN,
		WiiPro,
		SwitchPro,
		SwitchCon,
		WiiuPro,
		Ps4 = 64,
		Ps5,
		Xb1 = 80,
		G920H = 98,
		Df = 100,
		DfPro = 104,
		G25 = 105,
		Dfgt = 106,
		G27 = 107,
		G29 = 112,
		G920X = 128,
		KEYBMOUSE = 144
	};

	enum LedState : UCHAR {
		LedOff,
		LedOn
	};

#pragma pack(1)
	struct AttachedDevice {
		USHORT VendorID;
		USHORT ProductID;
		UCHAR Level;
		UCHAR Port;
		UCHAR DAddress;
		UCHAR Parent;
	};

	struct DeviceStatus {
		UCHAR Status;
		UCHAR Command;
		UCHAR Error;
		UCHAR Unknown;
		UINT Crc32;
		USHORT PayloadLength;
	};

	struct ExclusionListData {
		UCHAR ExclusionListKeyboard[28] = { 0 };
		UCHAR ExclusionListMouse[28] = { 0 };
	};

	struct InputReportData {
		USHORT CpuLoadValue;
		UCHAR SlotValue;
		ControllerType ConnectedController;
		ConsoleType ConnectedConsole;
		LedState LedState1;
		LedState LedState2;
		LedState LedState3;
		LedState LedState4;
		UCHAR RumbleValueA;
		UCHAR RumbleValueB;
		UCHAR RumbleValueRt;
		UCHAR RumbleValueLt;
		UCHAR BatteryValue;
		CHAR Input[38];
		UCHAR Unused[4];
		UCHAR ReportType;
		UCHAR VmSpeedValue;
		USHORT TimestampCounter;
	};

	struct OutputReportData {
		UCHAR Output[38];
		ULONG Trace1;
		ULONG Trace2;
		ULONG Trace3;
		USHORT Trace4;
		USHORT Trace5;
		USHORT Trace6;
		BYTE Unknown;
		CfgState CfgState;
	};

	struct PVarConfigData {
		USHORT Zero;
		UINT Value;
	};

public:
	struct SettingsLayout {
		UCHAR BtnMaps[22] = { 0xff };
		UCHAR MouseMaps[22] = { 0xff };
		UCHAR MouseKeybMaps[22] = {0xff};
		UCHAR NavconMaps[22] = {0xff};
		UCHAR RightStickMaps[4] = { 0xff };
		UCHAR LeftStickMaps[4] = { 0xff };
		UCHAR LightbarPercent;
		OperationalModeList OperationalMode;
		OutputModeList OutputMode;
		RemoteSlotChangeList RemoteSlot;
		BOOLEAN Ps4Specialty;
		BOOLEAN RemotePlay;
	};

private:
	struct SlotConfigData {
		USHORT GamepackID;
		USHORT Unknown1;
		UCHAR Flags;
		UCHAR Unknown2;
		UCHAR ConfigPVars;
		UCHAR Slot;
		UCHAR Unknown3[44] = { 0 };
		UCHAR Title[52] = { 0 };
		UCHAR Unknown4[12] = { 0 };
		USHORT ByteCodeLength;
		USHORT Unknown5;
		UINT Unknown6;
		PVarConfigData ConfigData[64] = { 0 };
	};

	struct SlotConfig {
		BOOLEAN NeedByteCode;
		BOOLEAN MustFlashConfig;
		BOOLEAN MustFlashGamepack;
		SlotConfigData Config;
		std::wstring ByteCodeFile;
		std::wstring ByteCodeFilePath;
		std::wstring ConfigFilePath;
	};

#pragma pack()

	struct UnexpectedSize {
		std::wstring Command;
		std::size_t Received;
		USHORT Expected;
	};

	// Outgoing command queue
	std::deque<PUCHAR> m_Queue;

	// Various structures for various information about the device
	std::unique_ptr<AttachedDevice[]> m_AttachedDevices;
	std::unique_ptr<ExclusionListData> m_ExclusionList;
	std::unique_ptr<FragmentData[]> m_Fragments;
	std::unique_ptr<MkFile> m_MkFile;
	std::unique_ptr<SlotConfig[]> m_SlotConfig; // Used for programming the device
	std::unique_ptr<SlotConfigData[]> m_SlotConfigData; // Used for actual slot config on the device as returned via ReadSlotsCfg

	// Firmware version information
	std::unique_ptr<SemanticVersion> m_SemanticVersion;

	// Read command processing
	std::unique_ptr<ParseBuffer> m_ParseBuffer;
	std::shared_ptr<StoreBuffer> m_PreparseBuffer;
	USHORT m_PayloadLength = 0;

	BOOLEAN m_IsHub[10];
	FragmentIDList m_UpdateFragmentType = FragmentUnused;
	ConnectionState m_ConnectionState = Disconnected;
	HANDLE m_WorkerThread = INVALID_HANDLE_VALUE;
	InputReportData m_LastInputReport;
	UCHAR m_TargetMode = 0;
	UCHAR m_Checksum[4];
	UCHAR m_BluetoothDevices = 0;
	UCHAR m_SlotsUsed = 0;
	UCHAR m_WiredDevices = 0;
	std::wstring m_Firmware;
	std::wstring m_Serial;

	SettingsLayout m_Settings;

	VOID DestroyWorkerThread(VOID);

	CONST std::wstring GetDeviceDescription(_In_ CONST USHORT VendorID, _In_ CONST USHORT ProductID);

	// Private method for downloading byte code off the device
	VOID CheckByteCodeFiles(VOID);

	// Private method for flashing scripts
	VOID FlashNextConfig(VOID);
	VOID FlashNextGamepack(VOID);

	VOID OnExclusionListRead(VOID);
	VOID OnFragmentRead(VOID);
	VOID OnGetFirmware(VOID);
	VOID OnGetSerial(VOID);
	VOID OnGetStatus(VOID);
	VOID OnReadByteCode(VOID);
	VOID OnReadSlotsCfg(VOID);
	VOID OnRequestAttachedDevices(VOID);
	VOID OnRequestMkFile(VOID);

	// Device report handlers
	VOID OnInputReport(VOID);
	VOID OnOutputReport(VOID);
	VOID OnMouseReport(VOID);
	VOID OnKeyboadReport(VOID);
	VOID OnNavconReport(VOID);
	VOID OnG13Report(VOID);
	VOID OnDebugReport(VOID);
	VOID OnPs5AdtReport(VOID);

	VOID HandleReadCommand(_In_ CONST PUCHAR PacketData, _In_ CONST std::size_t PacketSize);
	VOID SendInitialCommunication(VOID);

	static DWORD WINAPI CronusWorkerThreadProc(LPVOID Parameter);
};

class ChangeSlotACommand : public CommandBase
{
public:
	explicit ChangeSlotACommand(VOID) : CommandBase(CronusZen::PacketID::CHANGESLOTA) { };
};

class ChangeSlotBCommand : public CommandBase
{
public:
	explicit ChangeSlotBCommand(VOID) : CommandBase(CronusZen::PacketID::CHANGESLOTB) { };
};

class ClCommand : public CommandBase
{
public:
	explicit ClCommand(VOID) : CommandBase(CronusZen::PacketID::CL) { };
};

class ClearBluetoothCommand : public CommandBase
{
public:
	explicit ClearBluetoothCommand(VOID) : CommandBase(CronusZen::PacketID::CLEARBTCOMMAND) { };
};

class CircleTestCommand : public CommandBase
{
public:
	explicit CircleTestCommand(_In_ CONST UCHAR X, _In_ CONST UCHAR Y, _In_ CONST USHORT Speed) : CommandBase(CronusZen::PacketID::CIRCLETEST) {
		InsertByte(X);
		InsertByte(Y);
		InsertShort(Speed);
	}
};

class DeviceCleanupCommand : public CommandBase
{
public:
	explicit DeviceCleanupCommand(VOID) : CommandBase(CronusZen::PacketID::DEVICECLEANUP) { };
};

class ExclusionListReadCommand : public CommandBase
{
public:
	explicit ExclusionListReadCommand(VOID) : CommandBase(CronusZen::PacketID::EXCLUSIONLISTREAD) { };
};

class ExitApiModeCommand : public CommandBase
{
public:
	explicit ExitApiModeCommand(VOID) : CommandBase(CronusZen::PacketID::EXITAPIMODE) { };
};

class FactoryResetCommand : public CommandBase
{
public:
	explicit FactoryResetCommand(VOID) : CommandBase(CronusZen::PacketID::FACTORYRESET) { };
};

class FlashConfigCommand : public CommandBase
{
public:
	explicit FlashConfigCommand(VOID) : CommandBase(CronusZen::PacketID::FLASHCONFIG) { };
};

class FlashGamepackCommand : public CommandBase
{
public:
	explicit FlashGamepackCommand(VOID) : CommandBase(CronusZen::PacketID::FLASHGAMEPACK) { };
};

class FragmentReadCommand : public CommandBase
{
public:
	explicit FragmentReadCommand(VOID) : CommandBase(CronusZen::PacketID::FRAGMENTREAD) { };
};

class GetFirmwareCommand : public CommandBase
{
public:
	explicit GetFirmwareCommand(VOID) : CommandBase(CronusZen::PacketID::GETFW) { };
};

class GetSerialCommand : public CommandBase
{
public:
	explicit GetSerialCommand(VOID) : CommandBase(CronusZen::PacketID::GETSERIAL) { };
};


class GetStatusCommand : public CommandBase
{
public:
	explicit GetStatusCommand(VOID) : CommandBase(CronusZen::PacketID::GETSTATUS) { };
};

class ReadByteCodeCommand : public CommandBase
{
public:
	explicit ReadByteCodeCommand(UCHAR Slot) : CommandBase(CronusZen::PacketID::READBYTECODE) {
		InsertByte(0x31 + Slot);
	}
};

class ReadSlotsCfgCommand : public CommandBase
{
public:
	explicit ReadSlotsCfgCommand(VOID) : CommandBase(CronusZen::PacketID::READSLOTSCFG) { };
};

class ResetDeviceCommand : public CommandBase
{
public:
	explicit ResetDeviceCommand(VOID) : CommandBase(CronusZen::PacketID::RESETDEVICE) { };
};

class RequestAttachedDevicesCommand : public CommandBase
{
public:
	explicit RequestAttachedDevicesCommand(VOID) : CommandBase(CronusZen::PacketID::REQUESTATTACHEDDEVICES) { };
};

class RequestMkFileCommand : public CommandBase
{
public:
	explicit RequestMkFileCommand(VOID) : CommandBase(CronusZen::PacketID::REQUESTMKFILE) { };
};

class SendSingleFragmentCommand : public CommandBase
{
public:
	explicit SendSingleFragmentCommand(_In_ CONST CronusZen::FragmentData Fragment) : CommandBase(CronusZen::PacketID::SENDSINGLEFRAGMENT) {
		InsertData((LPVOID)&Fragment, sizeof(CronusZen::FragmentData));
	};
};

class StreamIoStatusCommand : public CommandBase
{
public:
	explicit StreamIoStatusCommand(_In_ CONST CronusZen::StreamIoStatusMask StatusMask) : CommandBase(CronusZen::PacketID::STREAMIOSTATUS) {
		InsertByte(StatusMask);
	}
};

class TurnOffControllerCommand : public CommandBase
{
public:
	explicit TurnOffControllerCommand(VOID) : CommandBase(CronusZen::PacketID::TURNOFFCONTROLLER) { };
};

class UnloadGpcCommand : public CommandBase
{
public:
	explicit UnloadGpcCommand(VOID) : CommandBase(CronusZen::PacketID::UNLOADGPC) {

	}
};

#endif
