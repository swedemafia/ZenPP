#ifndef DIALOGS_MAINDIALOG_H
#define DIALOGS_MAINDIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

class MainDialog : public DialogBase
{
public:
	// Destructor for cleanup
	~MainDialog();

	// Public method for menu handle retrieval
	HMENU GetMenuHandle(VOID) CONST;

	// Public methods for displaying RichEdit messages
	VOID DisplayAdministratorStatus(VOID) CONST;
	VOID DisplayStartupInfo(VOID) CONST;
	VOID DisplaySupportInfo(VOID) CONST;

protected:
	// Protected callback method for central window message processing
	INT_PTR CALLBACK HandleMessage(_In_ CONST UINT Message, _In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);

private:
	// Private members for UI elements
	HMENU m_Menu = NULL;
	HWND m_hWndButtonAddScript = NULL;
	HWND m_hWndButtonRemoveSelection = NULL;
	HWND m_hWndButtonEraseAllScripts = NULL;
	HWND m_hWndButtonFactoryReset = NULL;
	HWND m_hWndButtonSoftReset = NULL;
	HWND m_hWndButtonProgramDevice = NULL;
	HWND m_hWndSlotsListBox = NULL;
	HWND m_hWndSlotsTitle = NULL;

	// Private methods for handling specific commands
	INT_PTR OnCommandFileExit(VOID);
	INT_PTR OnCommandFirmwareCompatible(VOID);
	INT_PTR OnCommandFirmwareCustom(VOID);
	INT_PTR OnCommandFirmwareErase(VOID);
	INT_PTR OnCommandFirmwareLatest(VOID);
	INT_PTR OnCommandHelpAbout(VOID);
	INT_PTR OnCommandHelpZenPPNews(VOID);

	// Private methods for handling specific window messages
	INT_PTR OnClose(VOID);
	INT_PTR OnCommand(_In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);
	INT_PTR OnCtlColorListBox(_In_ CONST WPARAM wParam);
	INT_PTR OnCtlColorStatic(_In_ CONST WPARAM wParam);
	INT_PTR OnDestroy(VOID);
	INT_PTR OnGetMinMaxInfo(_In_ CONST LPARAM lParam);
	INT_PTR OnInitDialog(VOID);
	INT_PTR OnNotify(_In_ CONST LPARAM lParam);
	INT_PTR OnSize(_In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);
};

#endif
