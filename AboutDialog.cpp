#include "Precomp.h"

INT_PTR AboutDialog::HandleMessage(CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam)
{
	AboutDialog* Dialog = reinterpret_cast<AboutDialog*>(GetWindowLongPtr(m_hWnd, GWLP_USERDATA));

	switch (Message) {
	case WM_CLOSE:					return Dialog->OnClose();
	case WM_COMMAND:				return Dialog->OnCommand(wParam, lParam);
	case WM_CTLCOLORSTATIC:			return Dialog->OnCtlColorStatic(wParam, lParam);
	case WM_DESTROY:				return Dialog->OnDestroy();
	case WM_INITDIALOG:				return Dialog->OnInitDialog();
	}

	return FALSE;
}

INT_PTR AboutDialog::OnClose(VOID)
{
	DestroyWindow(m_hWnd);
	return TRUE;
}

INT_PTR AboutDialog::OnCommand(CONST WPARAM wParam, CONST LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case BUTTON_ABOUT_OK:							return OnClose();
	}
	return FALSE;
}

INT_PTR AboutDialog::OnCtlColorStatic(CONST WPARAM wParam, CONST LPARAM lParam)
{
	if (reinterpret_cast<HWND>(lParam) == GetDlgItem(m_hWnd, EDIT_ABOUT_INFORMATION)) {
		HDC Static = reinterpret_cast<HDC>(wParam);
		SetTextColor(Static, WHITE);
		SetBkColor(Static, BLACK);
		return reinterpret_cast<INT_PTR>(GetStockObject(BLACK_BRUSH));
	} else {
		return DefWindowProc(m_hWnd, WM_CTLCOLORSTATIC, wParam, lParam);
	}
}

INT_PTR AboutDialog::OnDestroy(VOID)
{
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG)NULL);
	App->DestroyAboutDialog();
	return TRUE;
}

INT_PTR AboutDialog::OnInitDialog(VOID)
{
	std::wstring DisclaimerString(L"DISCLAIMER:\r\n\r\n");
	std::wstring VersionString(L"Version " + std::to_wstring(VERSION_MAJOR) + L"." + std::to_wstring(VERSION_MINOR) + L"." + std::to_wstring(VERSION_REVISION) + L" built on " + BUILD_DATE + L" at " + BUILD_TIME + L".");

	// Build disclaimer
	DisclaimerString += L"Zen++ demonstrates interoperability with the Cronus Zen gaming device.\r\n\r\n";
	DisclaimerString += L"Zen++ includes references to trademarks and registered trademarks owned by Collective Minds Gaming Company, Inc.; Nintendo Co., Ltd.; Sony Computer Entertainment; and Microsoft Inc.\r\n\r\n";
	DisclaimerString += L"Zen++, and its author, is not affiliated with, endorsed by, or sponsored by Collective Minds Gaming Company, Inc.; Nintendo Co., Ltd.; Sony Computer Entertainment; or Microsoft Inc.\r\n\r\n";
	DisclaimerString += L"\"Cronus Zen\", \"Nintendo\", \"Switch\", \"Wii\", \"Wii U\", \"PlayStation\", \"PlayStation 3\", \"PlayStation 4\", \"PlayStation 5\", \"DualShock\", \"DualSense\", \"PULSE 3D\", \"Remote Play\", \"Xbox\", \"Xbox 360\", \"Xbox One\", and \"Xbox Series X|S\" are trademarks or registered trademarks of their respective owners.\r\n\r\n";
	DisclaimerString += L"The use of these trademarks is for informational purposes only, to accurately describe the compatibility or features of Zen++ with products from these companies.";

	// Set label text
	SendDlgItemMessage(m_hWnd, LABEL_ABOUT_VERSIONINFO, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(VersionString.c_str()));
	SendDlgItemMessage(m_hWnd, EDIT_ABOUT_INFORMATION, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(DisclaimerString.c_str()));

	return TRUE;
}