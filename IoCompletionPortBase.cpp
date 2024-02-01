#include "Precomp.h"

// Constructor for initializing completion keys
IoCompletionPortBase::IoCompletionPortBase()
{
	m_OverlappedDisconnect.CompletionKey = IocpCompletionKey::Disconnect;
	m_OverlappedConnect.CompletionKey = IocpCompletionKey::Connect;
	m_OverlappedRead.CompletionKey = IocpCompletionKey::Read;
	m_OverlappedWrite.CompletionKey = IocpCompletionKey::Write;
}

// Destructor for closing potentially open handles to the IOCP and the IOCP event thread
IoCompletionPortBase::~IoCompletionPortBase()
{
	CancelIocp();
}

// Method for associating an open file (device) handle with an IOCP
CONST BOOL IoCompletionPortBase::AssociateWithIocp(CONST HANDLE FileHandle)
{
	try {
		// Create IOCP to associate with an open file (device) handle
		if (!(m_IocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1)))
			throw std::wstring(L"An error occured while creating an I/O completion port.");

		// Associate IOCP with the an open file (device) handle
		if (!CreateIoCompletionPort(FileHandle, m_IocpHandle, NULL, 0))
			throw std::wstring(L"An error occured while associating the I/O completion port with the provided file handle.");

		// Create IOCP event thread
		if ((m_IocpThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)IocpEventThreadProc, (LPVOID)this, CREATE_SUSPENDED, NULL)) == INVALID_HANDLE_VALUE)
			throw std::wstring(L"An error occured while creating the I/O completion port event thread.");

	} catch (CONST std::wstring& CustomMessage) {
		// Notify user of the failure relevant to associating an IOCP with an open file (device) handle
		App->DisplayError(CustomMessage);

		return FALSE; // Signal failure
	}

	return TRUE; // Signal success
}

VOID IoCompletionPortBase::CancelIocp(VOID)
{
	// Release IOCP handle
	if (m_IocpHandle != INVALID_HANDLE_VALUE) {
		//CancelIo(m_IocpHandle);
		CloseHandle(m_IocpHandle);
		m_IocpHandle = INVALID_HANDLE_VALUE;
	}

	// Stop IOCP event thread, if running
	if (m_IocpThreadHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(m_IocpThreadHandle);
		m_IocpThreadHandle = INVALID_HANDLE_VALUE;
	}
}

// Method for IOCP handle retrieval
CONST HANDLE IoCompletionPortBase::GetIocpHandle(VOID) CONST
{
	return m_IocpHandle;
}

// Method for IOCP event thread handle retrieval
CONST HANDLE IoCompletionPortBase::GetIocpThreadHandle(VOID) CONST
{
	return m_IocpThreadHandle;
}

LPOVERLAPPED IoCompletionPortBase::GetOverlappedConnect(VOID)
{
	return &m_OverlappedConnect.Overlapped;
}

LPOVERLAPPED IoCompletionPortBase::GetOverlappedDisconnect(VOID)
{
	return &m_OverlappedDisconnect.Overlapped;
}

LPOVERLAPPED IoCompletionPortBase::GetOverlappedRead(VOID)
{
	return &m_OverlappedRead.Overlapped;
}

LPOVERLAPPED IoCompletionPortBase::GetOverlappedWrite(VOID)
{
	return &m_OverlappedWrite.Overlapped;
}

// Thread for processing IOCP events
DWORD IoCompletionPortBase::IocpEventThreadProc(LPVOID Parameter)
{
	// Declare pointer to the IOCP port object associated with the thread
	IoCompletionPortBase* Base = reinterpret_cast<IoCompletionPortBase*>(Parameter);

	while (Base->m_IocpThreadHandle != INVALID_HANDLE_VALUE) {
		// Completion event variables
		DWORD NumberOfBytesTransferred = 0;
		std::unique_ptr<LPOVERLAPPED> Overlapped = std::make_unique<LPOVERLAPPED>();
		ULONG_PTR CompletionKey = NULL;

		// Query completion status for an IOCP event
		if (GetQueuedCompletionStatus(Base->m_IocpHandle, &NumberOfBytesTransferred, &CompletionKey, Overlapped.get(), INFINITE)) {
			// Call IOCP event handler to dispatch the event
			Base->HandleCompletionEvent(NumberOfBytesTransferred, *Overlapped.get());
		} else {
			// Post queued completion status for the client device closing the connection 
			PostQueuedCompletionStatus(Base->m_IocpHandle, 0, IocpCompletionKey::Disconnect, &Base->m_OverlappedDisconnect.Overlapped);
		}
	}

	return 0; // Exit thread
}

VOID IoCompletionPortBase::HandleCompletionEvent(_In_ CONST DWORD BytesTransferred, _In_ CONST LPOVERLAPPED Overlapped)
{
	// Retrieve OverlappedEx structure containing extended operation information
	OverlappedEx* Operation = CONTAINING_RECORD(Overlapped, OverlappedEx, Overlapped);

	// Dispatch event handler based on the completion key
	switch (Operation->CompletionKey) {
	case IocpCompletionKey::Connect:
		// Handle successful connection
		if (OnConnect()) {
			// Initiate asynchronous read operation
			AsynchronousRead();
		}
		break;
	case IocpCompletionKey::Disconnect:
		// Handle client disconnection
		OnDisconnect();
		break;
	case IocpCompletionKey::Read:
		// Handle completed read operation
		if (OnRead(BytesTransferred)) {
			// Initiate another asynchronous read for continued communication
			AsynchronousRead();
		}
		break;
	case IocpCompletionKey::Write:
		// Handle completed write operation
		OnWrite(BytesTransferred);
		break;
	}
}