#include "Precomp.h"


INT_PTR MkSettingsDialog::HandleMessage(CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam)
{
	MkSettingsDialog* Dialog = reinterpret_cast<MkSettingsDialog*>(GetWindowLongPtr(m_hWnd, GWLP_USERDATA));

	/*
	switch (Message) {
	case WM_CLOSE:									return Dialog->OnClose();
	case WM_COMMAND:								return Dialog->OnCommand(wParam, lParam);
	case WM_CTLCOLORDLG:							return Dialog->OnCtlColorDlg(wParam);
	case WM_CTLCOLORLISTBOX:						return Dialog->OnCtlColorListBox(wParam);
	case WM_CTLCOLORSTATIC:							return Dialog->OnCtlColorStatic(wParam);
	case WM_DESTROY:								return Dialog->OnDestroy();
	case WM_DEVICECHANGE:							return Dialog->OnDeviceChange(wParam);
	case WM_DRAWITEM:								return Dialog->OnDrawItem(wParam, lParam);
	case WM_INITDIALOG:								return Dialog->OnInitDialog();
	case WM_GETMINMAXINFO:							return Dialog->OnGetMinMaxInfo(lParam);
	case WM_NOTIFY:									return Dialog->OnNotify(lParam);
	case WM_SIZE:									return Dialog->OnSize(wParam, lParam);
	}
	*/

	return FALSE;
}