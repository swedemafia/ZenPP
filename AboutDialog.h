#ifndef DIALOGS_ABOUTDIALOG_H
#define DIALOGS_ABOUTDIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

class AboutDialog : public DialogBase
{
protected:
	// Protected callback method for central window message processing
	INT_PTR CALLBACK HandleMessage(_In_ CONST UINT Message, _In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);

private:
	// Private methods for handling specific window messages
	INT_PTR OnClose(VOID);
	INT_PTR OnCommand(_In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);
	INT_PTR OnCtlColorStatic(_In_ CONST WPARAM wParam, _In_ CONST LPARAM lParam);
	INT_PTR OnDestroy(VOID);
	INT_PTR OnInitDialog(VOID);
};

#endif