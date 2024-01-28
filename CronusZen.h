#ifndef CLIENTS_CRONUSZEN_H
#define CLIENTS_CRONUSZEN_h

#ifdef _MSC_VER
#pragma once
#endif

class CommandBase;
class MkFile;

class CronusZen : public HidDeviceBase
{
public:
	// Publically accessible enumeration of command identifiers
	enum PacketID : UCHAR {
		INPUTREPORT = 0x01,
		OUTPUTREPORT = 0x02,
		REQUESTIOSTATUS = 0x02,
		RUNSCRIPT = 0x03,
		APIMODE = 0x04,
		RESETDEVICE = 0x06,
		ENTERAPIMODE = 0x07,
		EXITAPIMODE = 0x08,
		PS5ADTDATA = 0x08,
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
		Navcon = 16,
		G13 = 32,
		Debug = 64,
		Ps5Adt = 128,
	};

protected:
	// Virtual callback methods for various IOCP operations
	BOOL OnConnect(VOID);
	BOOL OnDisconnect(VOID);
	BOOL OnRead(_In_ CONST DWORD BytesRead);
	BOOL OnWrite(_In_ CONST DWORD BytesWritten);

private:

	CONST std::wstring m_OperationalMode[3] = {
		L"wheel", L"normal", L"tournament"
	};

	CONST std::wstring m_OutputMode[7] = {
		L"auto", L"PlayStation 3", L"PC/Mobile/Xbox 360", L"PlayStation 4", L"Xbox One Series S|X", L"Nintendo Switch", L"PlayStation 5"
	};

	CONST std::wstring m_RemoteSlot[3] = {
		L"disabled", L"PS & Share / Xbox & View", L"PS & L3 / Xbox & LS"
	};

	enum OperationalModeList : UCHAR {
		WheelMode = 0,
		GamepadMode,
		TournamentMode
	};

	enum OutputModeList : UCHAR {
		Auto = 0,
		PlayStation3,
		Xbox360,
		PlayStation4,
		XboxOne,
		NintendoSwitch,
		PlayStation5
	};

	enum RemoteSlotChangeList : UCHAR {
		Disabled = 0,
		PS_Share = 0,
		PS_L3
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

#pragma pack(1)
	struct AttachedDevice {
		USHORT VendorID;
		USHORT ProductID;
		UCHAR Level;
		UCHAR Port;
		UCHAR DAddress;
		UCHAR Parent;
		BOOLEAN IsHub;
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

	struct FragmentData {
		UCHAR ID;
		SourceType Source;
		UCHAR Value;
		UCHAR Value2;
	};

	struct PVarConfigData {
		USHORT Zero;
		UINT Value;
	};

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
#pragma pack()

	struct UnexpectedSize {
		std::wstring Command;
		std::size_t Received;
		USHORT Expected;
	};

	std::deque<PUCHAR> m_Queue;

	std::unique_ptr<AttachedDevice[]> m_AttachedDevices;
	std::unique_ptr<ExclusionListData> m_ExclusionList;
	std::unique_ptr<FragmentData[]> m_Fragments;
	std::unique_ptr<MkFile> m_MkFile;
	std::unique_ptr<SlotConfigData[]> m_SlotConfig;

	// Firmware version information
	std::unique_ptr<SemanticVersion> m_SemanticVersion;
	
	// Read command processing
	std::unique_ptr<ParseBuffer> m_ParseBuffer;
	std::unique_ptr<StoreBuffer> m_PreparseBuffer;
	USHORT m_PayloadLength = 0;

	UCHAR m_Checksum[4] = { NULL };
	std::wstring m_Firmware;
	std::wstring m_Serial;

	SettingsLayout m_Settings;

	VOID OnExclusionListRead(VOID);
	VOID OnFragmentRead(VOID);
	VOID OnGetFirmware(VOID);
	VOID OnGetSerial(VOID);
	VOID OnGetStatus(VOID);
	VOID OnReadSlotsCfg(VOID);
	VOID OnRequestAttachedDevices(VOID);
	VOID OnRequestMkFile(VOID);

	VOID HandleReadCommand(_In_ CONST PUCHAR PacketData, _In_ CONST std::size_t PacketSize);
	VOID SendInitialCommunication(VOID);
	VOID QueueCommand(_In_ CONST UCHAR Count, CommandBase& Command);
};

class CommandBase : public StoreBuffer
{
public:
	explicit CommandBase(_In_ CONST CronusZen::PacketID Command);
	~CommandBase() = default;
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

class ReadSlotsCfgCommand : public CommandBase
{
public:
	explicit ReadSlotsCfgCommand(VOID) : CommandBase(CronusZen::PacketID::READSLOTSCFG) { };
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

class StreamIoStatusCommand : public CommandBase
{
public:
	explicit StreamIoStatusCommand(_In_ CONST CronusZen::StreamIoStatusMask StatusMask) : CommandBase(CronusZen::PacketID::STREAMIOSTATUS) {
		InsertByte(StatusMask);
	}
};

class UnloadGpcCommand : public CommandBase
{
public:
	explicit UnloadGpcCommand(VOID) : CommandBase(CronusZen::PacketID::UNLOADGPC) {

	}
};
#endif
