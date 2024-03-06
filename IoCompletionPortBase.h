#ifndef BASECLASSES_IOCOMPLETIONPORTBASE_H
#define BASECLASSES_IOCOMPLETIONPORTBASE_H

#ifdef _MSC_VER
#pragma once
#endif

class IoCompletionPortBase
{
public:
	// Constructor for initializing IOCP event handling
	IoCompletionPortBase();

	// Destructor for cleanup
	~IoCompletionPortBase();

	// Enumeration declaring completion keys for connect, read, send
	enum IocpCompletionKey : UCHAR {
		Connect = 0,
		Disconnect,
		Read,
		Write
	};

	// Public method for associating an IOCP with a file handle
	CONST BOOL AssociateWithIocp(_In_ CONST HANDLE FileHandle);
	VOID CancelIocp(VOID);

protected:
	// Protected methods for IOCP information retrieval
	CONST HANDLE GetIocpHandle(VOID) CONST;
	CONST HANDLE GetIocpThreadHandle(VOID) CONST;

	// Protected methods to retrieve overlapped structures for IOCP events
	LPOVERLAPPED GetOverlappedConnect(VOID);
	LPOVERLAPPED GetOverlappedDisconnect(VOID);
	LPOVERLAPPED GetOverlappedRead(VOID);
	LPOVERLAPPED GetOverlappedWrite(VOID);

	// Virtual callback methods for various IOCP operations
	virtual BOOL OnConnect(VOID) = 0;
	virtual BOOL OnDisconnect(VOID) = 0;
	virtual BOOL OnRead(_In_ CONST DWORD BytesRead) = 0;
	virtual BOOL OnWrite(_In_ CONST DWORD BytesWritten) = 0;
	virtual VOID AsynchronousRead(VOID) = 0;

private:
	// Structure for overlapped IOCP events
	struct OverlappedEx {
		OVERLAPPED Overlapped;
		IocpCompletionKey CompletionKey;
	};

	// Private members for the IOCP and event thread
	HANDLE m_IocpHandle = INVALID_HANDLE_VALUE;
	HANDLE m_IocpThreadHandle = INVALID_HANDLE_VALUE;

	// Private members for IOCP event handling
	OverlappedEx m_OverlappedConnect = { NULL };
	OverlappedEx m_OverlappedDisconnect = { NULL };
	OverlappedEx m_OverlappedRead = { NULL };
	OverlappedEx m_OverlappedWrite = { NULL };

	// Private method for handling and dispatching IOCP events
	VOID HandleCompletionEvent(_In_ CONST DWORD BytesTransferred, _In_ CONST LPOVERLAPPED Overlapped);

	// Private thread procedure for IOCP event handling
	static DWORD WINAPI IocpEventThreadProc(LPVOID Parameter);
};

#endif