#ifndef BASECLASSES_DIALOGBASE_H
#define BASECLASSES_DIALOGBASE_H

#ifdef _MSC_VER
#pragma once
#endif

// Forward declarations
class CronusZen;

class DialogBase {
public:
    // Constructor and destructor
    DialogBase();
    ~DialogBase();

    // Public methods for dialog management
    CONST HWND GetHwnd(VOID) CONST;
    CONST HWND GetRichEditHwnd(VOID) CONST;
    VOID BringToForeground(VOID) CONST;
    VOID Create(CONST UINT ResourceID);
    VOID SetTitle(CONST std::wstring& DialogTitle) CONST;

    // Public methods for RichEdit output
    VOID PrintTimestamp(VOID);
    VOID PrintText(COLORREF Color, LPCWSTR Format, ...);

    // Public methods for RichEdit control
    VOID RichEditInitialize(CONST UINT ResourceID, CONST std::wstring& FontFace, CONST LONG Height, CONST LONG Offset);
    VOID RichEditToggleUrlDetection(VOID);
    VOID RichEditSubClass(VOID);

protected:
    // Protected member Cronus Zen object for ease of accessibility
    CronusZen& m_CronusZen;

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
    // Private struct for RichEdit output
    struct RichEditOutputData {
        COLORREF Color;
        std::wstring Text;
    };

    // Private struct for EditStreamCallback
    struct EditStreamDataStruct {
        UINT Length;
        PWCHAR Text;
    };

    // Private members for dialog resources
    HICON m_Icon = NULL;
    HINSTANCE m_Instance = NULL;

    // Private members for RichEdit routines
    HANDLE m_RichEditThread = INVALID_HANDLE_VALUE;
    std::deque<RichEditOutputData> m_RichEditQueue;

    // Private thread for RichEdit routine
    static DWORD CALLBACK RichEditThreadProc(LPVOID Parameter);

    // Private static callback functions
    static DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb);
    static INT_PTR CALLBACK MessageRouter(CONST HWND hWnd, CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam);
    static LRESULT CALLBACK RichEditSubClassRouter(CONST HWND hWnd, CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam);
};

#endif
