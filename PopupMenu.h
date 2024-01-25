#ifndef UTILITIES_POPUPMENU_H
#define UTILITIES_POPUPMENU_H

#ifdef _MSC_VER
#pragma once
#endif

class PopupMenu
{
public:
	// Constructor for popup menu initialization
	explicit PopupMenu(_In_ CONST HWND hWndParent, _In_ CONST UINT ResourceID);

	// Destructor for cleanup
	~PopupMenu();

	// Public method for displaying the popup menu
	VOID Display(VOID);

private:
	// Private members for menu information
	HMENU m_hParentMenu = NULL;
	HMENU m_hPopupMenu = NULL;
	HWND m_hWndParent = NULL;
};

#endif
