#ifndef ZENPP_PRECOMP_H
#define ZENPP_PRECOMP_H

#ifdef _MSC_VER
#pragma once
#endif

#pragma comment (lib, "hid.lib")
#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "winhttp.lib")
#pragma comment (lib, "winusb.lib")

#define WIN32_LEAN_AND_MEAN
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <codecvt>
#include <deque>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <Windows.h>

#include <AclAPI.h>
#include <CommCtrl.h>
#include <commdlg.h>
#include <Dbt.h>
#include <hidsdi.h>
#include <Richedit.h>
#include <SetupAPI.h>
#include <shellapi.h>
#include <winhttp.h>
#include <winusb.h>

// Utilities:
#include "File.h"
#include "ParseBuffer.h"
#include "PopupMenu.h"
#include "ResourceFile.h"
#include "SemanticVersion.h"
#include "StoreBuffer.h"

// Base classes:
#include "CommandBase.h"
#include "DeviceLocatorBase.h"
#include "DialogBase.h"
#include "HidDeviceBase.h"
#include "HttpSessionBase.h"
#include "IoCompletionPortBase.h"
#include "WinUsbBase.h"

// Clients:
#include "CronusZen.h"
#include "DfuController.h"
#include "FirmwareManager.h"
#include "MkFile.h"
#include "VersionCheck.h"

// Dialogs:
#include "AboutDialog.h"
#include "CodeDialog.h"
#include "FirmwareDialog.h"
#include "MainDialog.h"

// Zen++:
#include "BuildDateTime.h"
#include "Constants.h"
#include "Program.h"
#include "resource.h"


#endif
