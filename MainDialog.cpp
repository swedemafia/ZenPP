#include "Precomp.h"

MainDialog::~MainDialog()
{
	PostQuitMessage(0);
}

VOID MainDialog::DisplayAdministratorStatus(VOID) CONST
{
	if (!App->IsAdministrator()) {
		PrintTimestamp();
		PrintText(PINK, L"This application may require administrator privileges to run effectively and perform all functions.\r\n");
		PrintTimestamp();
		PrintText(PINK, L"To launch with administrative privileges, close this application and right-click the application's shortcut or executable icon and select 'Run as administrator' from the popup context menu.\r\n");
		DisplaySupportInfo();
	}
}

VOID MainDialog::DisplayStartupInfo(VOID) CONST
{
	// Set dialog caption
	SetTitle(L"Zen++ Copyright © 2023-2024 Swedemafia");

	// Display greeting to RichEdit
	PrintTimestamp();
	PrintText(TEAL, L"Zen++ Copyright © 2023-2024 Swedemafia - version %u.%u.%u.\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
	PrintTimestamp();
	PrintText(TEAL, L"An Asgard production (https://discord.gg/tGH7QxtPam).\r\n");
	PrintTimestamp();
#ifdef _DEBUG
	PrintText(TEAL, L"Private debug build compiled at %s on %s.\r\n", BUILD_TIME, BUILD_DATE);
#else
	PrintText(TEAL, L"Public release build compiled at %s on %s.\r\n", BUILD_TIME, BUILD_DATE);

#endif
}

VOID MainDialog::DisplaySupportInfo(VOID) CONST
{
	PrintTimestamp();
	PrintText(PURPLE, L"If you need further assistance, please refer to the Discord server (https://discord.gg/tGH7QxtPam) or contact your system administrator.\r\n");
}

HMENU MainDialog::GetMenuHandle(VOID) CONST
{
	return m_Menu;
}

INT_PTR MainDialog::HandleMessage(CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam)
{
	MainDialog* Dialog = reinterpret_cast<MainDialog*>(GetWindowLongPtr(m_hWnd, GWLP_USERDATA));

	switch (Message)
	{
	case WM_CLOSE:					return Dialog->OnClose();
	case WM_COMMAND:				return Dialog->OnCommand(wParam, lParam);
	case WM_CTLCOLORLISTBOX:		return Dialog->OnCtlColorListBox(wParam);
	case WM_CTLCOLORSTATIC:			return Dialog->OnCtlColorStatic(wParam);
	case WM_DESTROY:				return Dialog->OnDestroy();
	case WM_INITDIALOG:				return Dialog->OnInitDialog();
	case WM_GETMINMAXINFO:			return Dialog->OnGetMinMaxInfo(lParam);
	case WM_NOTIFY:					return Dialog->OnNotify(lParam);
	case WM_SIZE:					return Dialog->OnSize(wParam, lParam);
	}

	return FALSE;
}

INT_PTR MainDialog::OnClose(VOID)
{
	DestroyWindow(m_hWnd);
	return TRUE;
}

INT_PTR MainDialog::OnCommand(CONST WPARAM wParam, CONST LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case MENU_FILE_EXIT:							return OnCommandFileExit();
	case MENU_FIRMWARE_COMPATIBLE:					return OnCommandFirmwareCompatible();
	case MENU_FIRMWARE_CUSTOM:						return OnCommandFirmwareCustom();
	case MENU_FIRMWARE_ERASE:						return OnCommandFirmwareErase();
	case MENU_FIRMWARE_LATEST:						return OnCommandFirmwareLatest();
	case MENU_HELP_ABOUT:							return OnCommandHelpAbout();
	case MENU_HELP_ZENPPNEWS:						return OnCommandHelpZenPPNews();
	}
	return FALSE;
}

INT_PTR MainDialog::OnCommandFileExit(VOID)
{
	OnClose();
	return TRUE;
}

INT_PTR MainDialog::OnCommandFirmwareCompatible(VOID)
{
	App->CreateFirmwareDialog(FirmwareManager::InstallCompatibleFirmware);
	return TRUE;
}

INT_PTR MainDialog::OnCommandFirmwareCustom(VOID)
{
	App->CreateFirmwareDialog(FirmwareManager::InstallCustomFirmware);
	return TRUE;
}

INT_PTR MainDialog::OnCommandFirmwareErase(VOID)
{
	App->CreateFirmwareDialog(FirmwareManager::EraseFirmware);
	return TRUE;
}

INT_PTR MainDialog::OnCommandFirmwareLatest(VOID)
{
	App->CreateFirmwareDialog(FirmwareManager::InstallLatestFirmware);
	return TRUE;
}

INT_PTR MainDialog::OnCommandHelpAbout(VOID)
{
	App->ShowAboutDialog();
	return TRUE;
}

INT_PTR MainDialog::OnCommandHelpZenPPNews(VOID)
{
	App->GetVersionCheck().DisplayNews();
	return TRUE;
}

INT_PTR MainDialog::OnCtlColorListBox(CONST WPARAM wParam)
{
	HDC ListBox = reinterpret_cast<HDC>(wParam);
	SetTextColor(ListBox, WHITE);
	SetBkColor(ListBox, BLACK);
	return reinterpret_cast<INT_PTR>(GetStockObject(BLACK_BRUSH));
}

INT_PTR MainDialog::OnCtlColorStatic(CONST WPARAM wParam)
{
	HDC Static = reinterpret_cast<HDC>(wParam);
	SetTextColor(Static, WHITE);
	SetBkColor(Static, BLACK);
	return reinterpret_cast<INT_PTR>(GetStockObject(BLACK_BRUSH));
}

INT_PTR MainDialog::OnDestroy(VOID)
{
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG)NULL);
	PostQuitMessage(0);
	return TRUE;
}

INT_PTR MainDialog::OnInitDialog(VOID)
{
	// Get dialog control handles
	m_hWndButtonAddScript = GetDlgItem(m_hWnd, BUTTON_MAIN_ADDSCRIPT);
	m_hWndButtonRemoveSelection = GetDlgItem(m_hWnd, BUTTON_MAIN_REMOVESELECTION);
	m_hWndButtonEraseAllScripts = GetDlgItem(m_hWnd, BUTTON_MAIN_ERASEALLSCRIPTS);
	m_hWndButtonFactoryReset = GetDlgItem(m_hWnd, BUTTON_MAIN_FACTORYRESET);
	m_hWndButtonSoftReset = GetDlgItem(m_hWnd, BUTTON_MAIN_SOFTRESET);
	m_hWndButtonProgramDevice = GetDlgItem(m_hWnd, BUTTON_MAIN_PROGRAMDEVICE);
	m_hWndSlotsListBox = GetDlgItem(m_hWnd, LIST_MAIN_SLOTS);
	m_hWndSlotsTitle = GetDlgItem(m_hWnd, EDIT_MAIN_SLOTS);
	m_Menu = GetMenu(m_hWnd);

	return TRUE;
}

INT_PTR MainDialog::OnGetMinMaxInfo(CONST LPARAM lParam)
{
	LPMINMAXINFO MinMaxInfo = reinterpret_cast<LPMINMAXINFO>(lParam);
	MinMaxInfo->ptMinTrackSize.x = 600;
	MinMaxInfo->ptMinTrackSize.y = 400;
	return FALSE;
}

INT_PTR MainDialog::OnNotify(CONST LPARAM lParam)
{
	LPNMHDR Notification = (LPNMHDR)lParam;

	// Determine if a link was clicked in our RichEdit window
	if ((Notification->hwndFrom == m_hWndRichEdit) && (Notification->code == EN_LINK))
	{
		ENLINK* Link = (ENLINK*)lParam;
		if (Link->msg == WM_LBUTTONDOWN)
		{
			std::unique_ptr<WCHAR[]> Url(new WCHAR[Link->chrg.cpMax - Link->chrg.cpMin + 1]{ 0 });
			TEXTRANGE TextRange = { Link->chrg, Url.get() };

			try
			{
				// Fetch the URL
				if (!SendMessage(m_hWndRichEdit, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&TextRange)))
					throw std::wstring(L"An error occured while trying to read the URL.");

				// Execute the URL
				if ((UINT)ShellExecute(m_hWnd, L"open", Url.get(), NULL, NULL, SW_SHOWNORMAL) <= 32)
					throw std::wstring(L"An error ocurred while trying to open the URL.");
			}
			catch (CONST std::wstring& CustomMessage)
			{
				App->DisplayError(CustomMessage);
			}
		}
	}

	return TRUE;
}

INT_PTR MainDialog::OnSize(CONST WPARAM wParam, CONST LPARAM lParam)
{
	WORD DialogHeight = HIWORD(lParam);
	WORD DialogWidth = LOWORD(lParam);

	WORD RightPanelXPos = DialogWidth - 298;
	WORD RightPanelWidth = 290;

	// Resize right side
	MoveWindow(m_hWndButtonAddScript, RightPanelXPos, DialogHeight - 240, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonRemoveSelection, RightPanelXPos, DialogHeight - 202, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonEraseAllScripts, RightPanelXPos, DialogHeight - 160, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonFactoryReset, RightPanelXPos, DialogHeight - 120, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonSoftReset, RightPanelXPos, DialogHeight - 82, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndButtonProgramDevice, RightPanelXPos, DialogHeight - 40, RightPanelWidth, 30, TRUE);
	MoveWindow(m_hWndSlotsListBox, RightPanelXPos, 44, RightPanelWidth, DialogHeight - 290, TRUE);
	MoveWindow(m_hWndSlotsTitle, RightPanelXPos, 9, RightPanelWidth, 28, TRUE);

	// Resize left side
	MoveWindow(m_hWndRichEdit, 8, 9, DialogWidth - 312, DialogHeight - 17, TRUE);

	// Enumerate through each of the controls on the dialog
	EnumChildWindows(m_hWnd, [](HWND hWnd, LPARAM lParam) -> BOOL {
		RECT ControlRect;
		GetClientRect(hWnd, &ControlRect);
		InvalidateRect(hWnd, &ControlRect, TRUE);
		return TRUE; // Return TRUE to continue enumeration
		}, NULL);

	return TRUE;
}