#include "Precomp.h"

// Constructor for DialogBase, initializing instance
DialogBase::DialogBase()
{
	m_Instance = App->GetInstance();
}

// Destructor for DialogBase, responsible for cleanup
DialogBase::~DialogBase()
{
	// Unregister device notifications, if active
	if (m_DeviceNotify)
		UnregisterDeviceNotification(m_DeviceNotify);

	if (m_RichEditThread != INVALID_HANDLE_VALUE)
		CloseHandle(m_RichEditThread);
}

// Thread for RichEdit output
DWORD DialogBase::RichEditThreadProc(LPVOID Parameter)
{
	DialogBase* Base = reinterpret_cast<DialogBase*>(Parameter);
	
	while (Base->m_RichEditThread != INVALID_HANDLE_VALUE) {
		RichEditOutputData Data = Base->m_RichEditQueue.front();

		// Prepare character formatting
		CHARFORMAT CharFormat = { NULL };
		CharFormat.cbSize = sizeof(CharFormat);
		CharFormat.dwMask = CFM_COLOR;
		CharFormat.crTextColor = Data.Color;

		// Chunk loop
		UINT Offset = 0;
		UINT Length = Data.Text.size();

		while (Offset < Length) {
			// Calculate chunk size, ensuring it doesn't exceed remaining text
			UINT ChunkSize = min(Length - Offset, 1024);

			// Prepare stream data
			EditStreamDataStruct EditStreamData{ ChunkSize, Data.Text.data() + Offset};

			// Prepare EDITSTREAM data
			EDITSTREAM EditStream = { NULL };
			EditStream.dwCookie = (DWORD_PTR)&EditStreamData;
			EditStream.pfnCallback = (EDITSTREAMCALLBACK)EditStreamCallback;

			// Send messages for current chunk
			SendMessage(Base->m_hWndRichEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
			SendMessage(Base->m_hWndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&CharFormat));
			SendMessage(Base->m_hWndRichEdit, EM_STREAMIN, SF_TEXT | SF_UNICODE | SFF_SELECTION, reinterpret_cast<LPARAM>(&EditStream));

			// Advance offset
			Offset += ChunkSize;
		}

		Base->m_RichEditQueue.pop_front();

		if (Base->m_RichEditQueue.empty())
			SuspendThread(Base->m_RichEditThread);
	}

	return 0;
}

// Method to bring the dialog to the foreground
VOID DialogBase::BringToForeground(VOID) CONST
{
	// Set the dialog as the foreground window
	SetForegroundWindow(m_hWnd);
}

// Method to create the dialog based on the provided resource identifier
VOID DialogBase::Create(CONST UINT ResourceID)
{
	try
	{
		// Create the dialog
		if(!(m_hWnd = CreateDialogParam(m_Instance, MAKEINTRESOURCE(ResourceID), nullptr, (DLGPROC)MessageRouter, (LPARAM)this)))
			throw std::wstring(L"An error occured while creating a dialog.  Please restart the application or contact support for assistance.");

		// Load the dialog icon
		if(!(m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(ICON_ZENPP))))
			throw std::wstring(L"An error occured while loading the dialog icon.");

		// Assign the dialog icon
		SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)m_Icon);
		SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)m_Icon);

		// Destroy the loaded icon
		DestroyIcon(m_Icon);

	}
	catch (CONST std::wstring& CustomMessage)
	{
		// Display error message
		App->DisplayError(CustomMessage);
	}
}

// Callback method for RichEdit stream operations
DWORD DialogBase::EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cbBuff, LONG* pcbRead)
{
	// Cast the cookie to the EditStreamDataStruct pointer
	EditStreamDataStruct* OutputStream = reinterpret_cast<EditStreamDataStruct*>(dwCookie);

	// Determine if there's any data to be streamed
	if (OutputStream->Length) {
		// Calculate how many characters can be read
		DWORD CharactersToRead = min(OutputStream->Length, cbBuff);

		// Copy Unicode characters to the buffer
		memcpy(pbBuff, OutputStream->Text, CharactersToRead << 1);

		// Update the stream data for the next call
		OutputStream->Text += CharactersToRead;
		OutputStream->Length -= CharactersToRead;

		// Set the number of bytes that were processed
		*pcbRead = (CharactersToRead << 1);

		return 0; // Signal success and to keep streaming
	}

	return ERROR_INVALID_PARAMETER; // No more data to stream
}

// Method to retrieve the dialog window handle
HWND DialogBase::GetHwnd(VOID) CONST
{
	// Return dialog window handle
	return m_hWnd;
}

// Callback function for routing messages
INT_PTR DialogBase::MessageRouter(CONST HWND hWnd, CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam)
{
	// Declare pointer to the dialog instance associated with the window
	DialogBase* Base = NULL;

	switch (Message)
	{
	case WM_INITDIALOG:
		// Handle dialog creation
		Base = reinterpret_cast<DialogBase*>(lParam);

		// Validate the parameters passed over successfully
		if (!Base) {
			// Display an error if initialization failed
			App->DisplayError(L"An error occured while initializing the applicaiton window.\r\n\r\nPlease restart the application or contact support for assistance.");
		}
		else {
			// Store window handle and dialog instance pointer for future access
			Base->m_hWnd = hWnd;

			// Create RichEdit thread
			if ((Base->m_RichEditThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RichEditThreadProc, (LPVOID)Base, CREATE_SUSPENDED, NULL)) == INVALID_HANDLE_VALUE) {
				MessageBox(hWnd, L"An error occured while creating the RichEdit output thread.\r\n\r\nPlease restart the application or contact support for assistance.", L"Zen++", MB_ICONERROR | MB_OK);
				App->QuitProgram();
			}

			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LPARAM)Base);
		}

		break;

	default:
		// Handle all other messages
		Base = reinterpret_cast<DialogBase*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		// Handle RichEdit popup menu commands
		if (Base && (Message == WM_COMMAND)) {
			switch (LOWORD(wParam)) {
			case MENU_POPUP_CLEAR:			return Base->RichEditClearText();
			case MENU_POPUP_COPY:			return Base->RichEditCopySelectionToClipboard();
			case MENU_POPUP_SELECTALL:		return Base->RichEditSelectAll();
			}
		}
	}

	// Delegate remaining messages to the dialog's HandleMessage method
	return Base ? Base->HandleMessage(Message, wParam, lParam) : FALSE;
}

// Subclassed window procedure for the RichEdit controls associated with DialogBase
LRESULT DialogBase::RichEditSubClassRouter(CONST HWND hWnd, CONST UINT Message, CONST WPARAM wParam, CONST LPARAM lParam)
{
	// Retrieve associated dialog instance
	DialogBase* Base = reinterpret_cast<DialogBase*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (Message) {
	case WM_RBUTTONUP:
		// Handle right mouse button released
		Base->m_PopupMenu->Display();

		return FALSE; // Message handled
	}

	// Pass any unhandled messages to the original RichEdit window procedure
	return CallWindowProc(Base->m_OriginalRichEditProc, hWnd, Message, wParam, lParam);
}

// Method for clearing the RichEdit window
BOOL DialogBase::RichEditClearText(VOID) CONST
{
	SendMessage(m_hWndRichEdit, EM_SETSEL, 0, -1);
	SendMessage(m_hWndRichEdit, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(L""));

	return TRUE;
}

// Method for copying the selected RichEdit window text to the system clipboard
BOOL DialogBase::RichEditCopySelectionToClipboard(VOID) CONST
{
	SendMessage(m_hWndRichEdit, WM_COPY, 0, 0);

	return TRUE;
}

// Method for printing a timestamp in [HH:MM:SS] format to the dialog's RichEdit text box
VOID DialogBase::PrintTimestamp(VOID)
{
	// Retrieve current system time
	SYSTEMTIME SystemTime;
	GetLocalTime(&SystemTime);

	// Format and print the timestamp in the color gray
	PrintText(GRAY, L"[%02u:%02u:%02u] ", SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
}

// Method for printing colored and formatted text to the dialog's RichEdit text box
VOID DialogBase::PrintText(COLORREF Color, LPCWSTR Format, ...)
{
	static INT counter = 0;

	// Prepare variable argument list
	va_list argptr;
	va_start(argptr, Format);

	// Determine string length
	UINT Length = _vscwprintf(Format, argptr) + 1;

	// Allocate buffer for current chunk
	std::unique_ptr<WCHAR[]> Output(new WCHAR[Length]);  // Use unique_ptr for automatic cleanup

	// Format the cunk using a modified format string adjusted by the offset
	vswprintf_s(Output.get(), Length, Format, argptr);

	// End use of argument pointer
	va_end(argptr);

	m_RichEditQueue.push_back(RichEditOutputData{ Color, std::wstring(Output.get()) });

	if (m_RichEditQueue.size() == 1)
		ResumeThread(m_RichEditThread);
}

// Method for initializing the RichEdit control
VOID DialogBase::RichEditInitialize(CONST UINT ResourceID, CONST std::wstring& FontFace, CONST LONG Height, CONST LONG Offset)
{
	try
	{
		// Get window handle
		if (!(m_hWndRichEdit = GetDlgItem(m_hWnd, ResourceID)))
			throw std::wstring(L"An error occured retrieving the handle to the RichEdit window.\r\n\r\nThe application may not perform as intended.");

		// Prepare character formatting
		CHARFORMAT CharFormat = { NULL };
		CharFormat.cbSize = sizeof(CHARFORMAT);
		CharFormat.dwMask = CFM_SIZE | CFM_FACE;
		CharFormat.yHeight = Height;
		wcscpy_s(CharFormat.szFaceName, FontFace.c_str());

		// Prepare paragraph formatting
		PARAFORMAT2 ParaFormat = { };
		ParaFormat.cbSize = sizeof(PARAFORMAT2);
		ParaFormat.dwMask = PFM_OFFSET;
		ParaFormat.dxOffset = Offset;

		// Enable URL detection
		RichEditToggleUrlDetection();

		// Send messages to the RichEdit
		SendMessage(m_hWndRichEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)BLACK);
		SendMessage(m_hWndRichEdit, EM_SETCHARFORMAT, SCF_DEFAULT, reinterpret_cast<LPARAM>(&CharFormat));
		SendMessage(m_hWndRichEdit, EM_SETEVENTMASK, 0, SendMessage(m_hWndRichEdit, EM_GETEVENTMASK, 0, 0) | ENM_LINK);
		SendMessage(m_hWndRichEdit, EM_SETPARAFORMAT, 0, reinterpret_cast<LPARAM>(&ParaFormat));

		// Initialize popup menu
		m_PopupMenu = std::make_unique<PopupMenu>(m_hWnd, MENU_RICHEDIT_POPUP);

		// Subclass RichEdit
		RichEditSubClass();
	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
	}
	catch (CONST std::bad_alloc&) {
		App->DisplayError(L"Unable to create the RichEdit popup context menu; insufficient memory is available to complete the required operation.");
	}
}

// Method for selecting all text within the RichEdit window from the popup context menu
BOOL DialogBase::RichEditSelectAll(VOID) CONST
{
	SendMessage(m_hWndRichEdit, EM_SETSEL, 0, -1);

	return TRUE;
}

// Method for toggling URL detection within the RichEdit window
VOID DialogBase::RichEditToggleUrlDetection(VOID)
{
	static BOOL DetectingUrls = FALSE;

	SendMessage(m_hWndRichEdit, EM_AUTOURLDETECT, (WPARAM)(DetectingUrls = !DetectingUrls), 0);
}

// Method for subclassing the RichEdit control
VOID DialogBase::RichEditSubClass(VOID)
{
	// Retrieve original window procedure used for unprocessed messages
	m_OriginalRichEditProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hWndRichEdit, GWLP_WNDPROC, reinterpret_cast<LONG>(RichEditSubClassRouter)));

	// Associate DialogBase instance with RichEdit control for future retrieval
	SetWindowLongPtr(m_hWndRichEdit, GWLP_USERDATA, reinterpret_cast<LONG>(this));
}

// Method for setting the caption for the dialog window
VOID DialogBase::SetTitle(CONST std::wstring& DialogTitle) CONST
{
	SetWindowText(m_hWnd, DialogTitle.c_str());
}

// Method for registering notifications upon devices attached or removed from the system
VOID DialogBase::RegisterDeviceNotifications(CONST GUID Guid)
{
	// Register for device change notifications
	DEV_BROADCAST_DEVICEINTERFACE BroadcastDeviceInterface = { NULL };

	BroadcastDeviceInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	BroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	BroadcastDeviceInterface.dbcc_classguid = Guid;

	// Attempt to register device notifications
	m_DeviceNotify = RegisterDeviceNotification(m_hWnd, &BroadcastDeviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

	// Verify notification was registered
	if (!m_DeviceNotify)
		App->DisplayError(L"An error occured registering device notifications to the window.");
}