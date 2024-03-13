#ifndef DIALOGS_MKSETTINGSDIALOG_H
#define DIALOGS_MKSETTINGSDIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

class MkSettingsDialog : public DialogBase
{
public:
	// Destructor for cleanup
	~MkSettingsDialog() = default;

protected:
	// Protected callback method for central window message processing
	INT_PTR CALLBACK HandleMessage(_In_ CONST UINT Message, _In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);

};

#endif