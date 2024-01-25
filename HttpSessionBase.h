#ifndef BASECLASSES_HTTPSESSIONBASE_H
#define BASECLASSES_HTTPSESSIONBASE_H

#ifdef _MSC_VER
#pragma once
#endif

class HttpSessionBase
{
protected:
	// Protected members for HTTP session data
	std::wstring m_Domain, m_PathToFile;

	// Protected methods for HTTP session functionality
	VOID DownloadFile(VOID);
	virtual VOID HttpSessionDataCallback(CONST std::deque<std::string> ResponseData) = 0;

private:
	// Private thread procedure for HTTP session handling
	static DWORD WINAPI HttpSessionThreadProc(LPVOID Parameter);
};

#endif
