#ifndef CLIENTS_MKFILE_H
#define CLIENTS_MKFILE_H

#ifdef _MSC_VER
#pragma once
#endif

class MkFile
{
public:
	// Destructor for cleanup
	~MkFile() = default;

	// Public methods for MkFile header information retieval
	CONST std::wstring GetName(VOID) CONST;
	CONST std::wstring GetRevision(VOID) CONST;
	CONST std::wstring GetType(VOID) CONST;
	CONST std::wstring GetVersion(VOID) CONST;

	// Public method for setting the MkFile data information
	VOID SetMkFileData(_In_ CONST PUCHAR FileData, _In_ CONST std::size_t FileSize);

protected:
private:
	enum SourceT : BYTE {
		Unknown = 0,
		Keyboard,
		Mouse,
		Joystick,
		Generic,
		G13
	};

	enum XimAnalogs : BYTE {
		None,
		Joystick_Left_X,
		Joystick_Left_Y,
		Joystick_Left_Z,
		Joystick_Right_X,
		Joystick_Right_Y,
		Joystick_Right_Z,
		Joystick_Left_Trigger,
		Joystick_Right_Trigger,
		Joystick_Slider
	};

	enum XimColors : BYTE {
		Red = 0,
		Green,
		Blue,
		Yellow,
		Magenta,
		Cyan,
		White,
		DarkGreen,
		Violet,
		Pink,
		Orange,
		Burgundy,
		Purple,
		Lime,
		Tangerine,
		Aqua
	};

#pragma pack(1)
	struct InputT {
		SourceT Source;
		UCHAR Input;
	};

	struct LayoutAnalogT {
		XimAnalogs KeyAnalogLx;
		XimAnalogs KeyAnalogLy;
		XimAnalogs KeyAnalogRx;
		XimAnalogs KeyAnalogRy;
		XimAnalogs KeyAnalogLt;
		XimAnalogs KeyAnalogRt;
		USHORT DzStickL;
		USHORT DzStickR;
		UCHAR DzTriggerL;
		UCHAR DzTriggerR;
	};

	struct LayoutCfgActivateT {
		InputT KeyActivate;
		UCHAR ActToggle;
		USHORT ActDelay;
		UCHAR SmoothAimTransition;
		InputT KeyDeactivate1;
		InputT KeyDeactivate2;
		InputT KeyDeactivate3;
		InputT KeyDeactivate4;
		USHORT DeActDelay;
	};

	struct LayoutCfgAimT {
		USHORT Sensitivity;
		USHORT XYRatio;
		UCHAR BcTranslatorHide;
		UCHAR Sync;
		InputT KeyTurnAssist;
		UCHAR KeyTurnAssistToggle;
		UCHAR Smoothing;
		UCHAR InvertY;
		UCHAR LeftStick;
		UCHAR Translator;
		USHORT SteadyAim;
		SHORT Boost;
	};

	struct LayoutCfgHeaderT {
		UCHAR Enable;
		UCHAR Inherit;
	};

	struct LayoutCfgMovementT {
		InputT LKeyUp;
		InputT LKeyLeft;
		InputT LKeyRight;
		InputT LKeyDown;
		InputT RKeyUp;
		InputT RKeyDown;
		InputT RKeyLeft;
		InputT RKeyRight;
		InputT LKeyWalk;
		InputT RKeyWalk;
		UCHAR byte_0;
		UCHAR byte_1;
		UCHAR Unk;
		UCHAR AnalogSim;
	};

	struct LayoutKeyMapsT {
		InputT R2;
		InputT L2;
		InputT R3;
		InputT L3;
		InputT R1;
		InputT L1;
		InputT Cross;
		InputT Circle;
		InputT Square;
		InputT Triangle;
		InputT Up;
		InputT Down;
		InputT Right;
		InputT Left;
		InputT Share;
		InputT Tpad;
		InputT Ps;
		InputT Options;
	};

	struct LayoutMapsT {
		LayoutKeyMapsT Primary;
		LayoutKeyMapsT Secondary;
	};

	struct LayoutTranslatorT {
		UCHAR DzShape;
		USHORT DzX;
		USHORT DzY;
		USHORT StickizeX;
		USHORT StickixeY;
		UCHAR Dummy[7] = { 0 };
	};

	struct LayoutConfigT {
		LayoutCfgHeaderT Header;
		LayoutCfgActivateT Activation;
		LayoutTranslatorT SmartTranslator;
		LayoutCfgAimT AimConfig;
		UCHAR BallisticCurve[22] = { 0 };
		UCHAR Dummy[10] = { 0 };
		LayoutCfgMovementT Movement;
		LayoutAnalogT Analog;
		LayoutMapsT Maps;
	};

	struct LayoutConfigsT {
		LayoutConfigT Hip;
		LayoutConfigT Ads;
		LayoutConfigT Aux1;
		LayoutConfigT Aux2;
		LayoutConfigT Aux3;
		LayoutConfigT Aux4;
	};

	struct LayoutHeaderT {
		UCHAR Type[8] = { 0 };
		UCHAR Revision[8] = { 0 };
		UCHAR Name[52] = { 0 };
		UCHAR Version[26] = { 0 };
		UCHAR ProfileID[2] = { 0 };
		InputT LoadKey;
		XimColors Color;
		UCHAR ActNotify;
		UCHAR MaxTurnSpeed;
		InputT PttKey;
		UCHAR PttToggle;
		UCHAR Unk;
	};

	struct LayoutValuesT {
		LayoutHeaderT Header;
		LayoutConfigsT Cfgs;
		UCHAR Iv[32] = { 0 };
	};
#pragma pack()

	// Private members for storing and processing the MkFile data
	std::unique_ptr<File> m_File;
	std::unique_ptr<ParseBuffer> m_ParseBuffer;

	// Private member storing the entire MkFile data
	LayoutValuesT m_LayoutValues;

	// Private method for processing errors relating to creating a directory
	VOID HandleCreateDirectoryError(VOID);
};

#endif
