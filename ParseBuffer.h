#ifndef UTILITIES_PARSEBUFFER_H
#define UTILITIES_PARSEBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

class ParseBuffer {
public:
	// Constructor for initializing the parse buffer data and size
	explicit ParseBuffer(_In_ CONST LPVOID Data, _In_ CONST std::size_t Size);

	// Destructor for automatic cleanup
	~ParseBuffer() = default;

	PCSTR ExtractStringA(VOID) CONST;
	PCSTR ExtractStringA(_In_ CONST std::size_t MaximumSize) CONST;
	PCWSTR ExtractStringW(VOID) CONST;
	PCWSTR ExtractStringW(_In_ CONST std::size_t MaximumSize) CONST;
	std::string& ExtractStringA(_Inout_ std::string& String) CONST;
	std::string& ExtractStringA(_Inout_ std::string& String, _In_ CONST std::size_t MaximumSize) CONST;
	std::wstring& ExtractStringW(_Inout_ std::wstring& String) CONST;
	std::wstring& ExtractStringW(_Inout_ std::wstring& String, _In_ CONST std::size_t MaximumSize) CONST;
	PVOID ExtractData(_Out_ PVOID Data, _In_ CONST std::size_t Size) CONST;
	UCHAR ExtractByte(VOID) CONST;
	USHORT ExtractShort(VOID) CONST;
	ULONG ExtractLong(VOID) CONST;
	ULONGLONG ExtractLongLong(VOID) CONST;

	// Buffer access
	CONST PUCHAR Buffer(VOID) CONST;

	// Buffer information retrieval
	CONST BOOL CanAdvance(_In_ CONST std::size_t Bytes) CONST;
	CONST std::size_t Position(VOID) CONST;
	CONST std::size_t Size(VOID) CONST;
	CONST std::size_t SizeWithoutHeader(VOID) CONST;

	// Buffer manipulation
	VOID Advance(_In_ CONST std::size_t Bytes);
	VOID Rewind(VOID) CONST;
	VOID SetBuffer(CONST LPVOID Data, CONST std::size_t Size);

private:
	mutable std::size_t m_Position;
	std::size_t m_Size;
	std::unique_ptr<UCHAR[]> m_Buffer;

	VOID CheckAdvance(_In_ CONST std::size_t Bytes) CONST;
};


#endif
