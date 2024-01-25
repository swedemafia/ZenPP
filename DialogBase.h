#ifndef BASECLASSES_DIALOGBASE_H
#define BASECLASSES_DIALOGBASE_H

#ifdef _MSC_VER
#pragma once
#endif

class DialogBase {
public:
    // Constructor and destructor
    DialogBase();
    ~DialogBase();

    // Public methods for dialog management
    HWND GetHwnd(VOID) CONST;
    VOID BringToForeground(VOID) CONST;
    VOID Create(CONST UINT ResourceID);
    VOID SetTitle(CONST std::wstring& DialogTitle) CONST;

    // Public methods for text output
    VOID PrintTimestamp(VOID) CONST;
    VOID PrintText(COLORREF Color, LPCWSTR Format, ...) CONST;

    // Public methods for RichEdit control
    VOID RichEditInitialize(CONST UINT ResourceID, CONST std::wstring& FontFace, CONST LONG Height, CONST LONG Offset);
    VOID RichEditToggleUrlDetection(VOID);
    VOID RichEditSubClass(VOID);

protected:
    // Protected members for RichEdit control
    HWND m_hWndRichEdit = NULL;
    WNDPROC m_OriginalRichEditProc = NULL;

    // Protected members for device notifications and popup menu
    HDEVNOTIFY m_DeviceNotify = NULL;
    std::unique_ptr<PopupMenu> m_PopupMenu;

    // Protected members for general dialog handling
    HWND m_hWnd = NULL;

    // Protected methods for RichEdit control
    BOOL RichEditClearText(VOID) CONST;
    BOOL RichEditCopySelectionToClipboard(VOID) CONST;
    BOOL RichEditSelectAll(VOID) CONST;

    // Protected methods for message handling and device notifications
    virtual INT_PTR CALLBACK HandleMessage(CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam) = 0;
    virtual INT_PTR OnClose(VOID) = 0;
    virtual INT_PTR OnInitDialog(VOID) = 0;
    VOID RegisterDeviceNotifications(CONST GUID Guid);

private:
    // Private members for dialog resources
    HICON m_Icon = NULL;
    HINSTANCE m_Instance = NULL;

    // Private struct for EditStreamCallback
    struct EditStreamDataStruct {
        PWCHAR Text;
        UINT Length;
    };

    // Private static callback functions
    static DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb);
    static INT_PTR CALLBACK MessageRouter(CONST HWND hWnd, CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam);
    static LRESULT CALLBACK RichEditSubClassRouter(CONST HWND hWnd, CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam);
};

#endif
