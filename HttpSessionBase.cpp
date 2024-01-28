#include "Precomp.h"

VOID HttpSessionBase::DownloadFile(VOID)
{
	// Execute thread
	if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HttpSessionThreadProc, (LPVOID)this, 0, NULL) == INVALID_HANDLE_VALUE)
		App->DisplayError(L"An error occured while creating the HTTP session thread.");
}

DWORD HttpSessionBase::HttpSessionThreadProc(LPVOID Parameter)
{
	HttpSessionBase* Base = reinterpret_cast<HttpSessionBase*>(Parameter);
	HINTERNET Session = NULL, Connection = NULL, Request = NULL;

	try
	{
		// Open HTTP session
		if (!(Session = WinHttpOpen(L"Zen++ HTTP Client", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)))
			throw std::wstring(L"An error occured  a HTTP session.");

		// Connect to host
		if (!(Connection = WinHttpConnect(Session, Base->m_Domain.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0)))
			throw std::wstring(L"An error occured while  a connection to the HTTP host.");

		// Request the file
		if (!(Request = WinHttpOpenRequest(Connection, L"GET", Base->m_PathToFile.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0)))
			throw std::wstring(L"An error occured while  a HTTP request.");

		// Send the request
		if (!WinHttpSendRequest(Request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
			throw std::wstring(L"An error occured while sending the HTTP request.");

		// Receive the response
		if (!WinHttpReceiveResponse(Request, NULL))
			throw std::wstring(L"An error occured while receiving the HTTP response.");

		// Read the response data
		DWORD BytesRead = 0;
		std::string Response;

		// Read the response data
		do {
			std::unique_ptr<CHAR[]> Buffer(new CHAR[4096]{ 0 });
			if (!WinHttpReadData(Request, Buffer.get(), sizeof(Buffer), &BytesRead))
				throw std::wstring(L"An error occured while reading the data from the HTTP response.");
			Response.append(Buffer.get(), BytesRead);

		} while (BytesRead > 0);


		// Break up the data into a deque
		std::shared_ptr<std::deque<std::string>> Segments = std::make_shared<std::deque<std::string>>();
		size_t StartPosition = 0;
		size_t FoundPosition = Response.find("\r\n");

		// Break up the response into segments
		while (FoundPosition != std::string::npos) {
			// Extract the segment and add it to the deque
			Segments->push_back(Response.substr(StartPosition, FoundPosition - StartPosition));

			// Update next starting position to skip two characters
			StartPosition = FoundPosition + 2;

			// Locate next occurance
			FoundPosition = Response.find("\r\n", StartPosition);
		}

		// Add final segment
		Segments->push_back(Response.substr(StartPosition));

		// Send to callback function
		Base->HttpSessionDataCallback(*Segments);

	}
	catch (CONST std::wstring& CustomMessage)
	{
		App->DisplayError(CustomMessage);
	}

	// Close handles
	if (Session)
		WinHttpCloseHandle(Session);

	if (Connection)
		WinHttpCloseHandle(Connection);

	if (Request)
		WinHttpCloseHandle(Request);

	ExitThread(0);

	return 0;
}