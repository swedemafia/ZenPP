#ifndef CLIENTS_CRONUSZEN_H
#define CLIENTS_CRONUSZEN_h

#ifdef _MSC_VER
#pragma once
#endif

class CommandBase;

class CronusZen : public HidDeviceBase
{
public:

	// Publically accessible enumeration for command identifiers
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
	std::deque<PUCHAR> m_Queue;

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

class ExitApiModeCommand : public CommandBase
{
public:
	explicit ExitApiModeCommand(VOID) : CommandBase(CronusZen::PacketID::EXITAPIMODE) { };
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
