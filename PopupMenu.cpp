#include "Precomp.h"

PopupMenu::PopupMenu(CONST HWND hWndParent, CONST UINT ResourceID) :
	m_hWndParent(hWndParent)
{
	try
	{
		// Initialize popup menu
		if (!(m_hParentMenu = LoadMenu(App->GetInstance(), MAKEINTRESOURCE(MENU_RICHEDIT_POPUP))))
			throw std::wstring(L"An error occured while loading the RichEdit popup menu from the resource file.");

		if (!(m_hPopupMenu = GetSubMenu(m_hParentMenu, 0)))
			throw std::wstring(L"An error occured obtaining the RichEdit submenu.");
	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
	}
}

PopupMenu::~PopupMenu()
{
	if (m_hPopupMenu)
		DestroyMenu(m_hPopupMenu);

	if (m_hParentMenu)
		DestroyMenu(m_hParentMenu);
}

VOID PopupMenu::Display(VOID)
{
	if (m_hPopupMenu) {
		POINT CursorPoint = { NULL };
		GetCursorPos(&CursorPoint);
		TrackPopupMenu(m_hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN, CursorPoint.x, CursorPoint.y, 0, m_hWndParent, NULL);
	}
}