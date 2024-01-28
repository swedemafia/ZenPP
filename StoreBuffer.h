#ifndef UTILITIES_STOREBUFFER_H
#define UTILITIES_STOREBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

class StoreBuffer
{
public:
	// Constructor initializes a buffer with an initial capacity of 128 bytes
	explicit StoreBuffer(_In_ std::size_t SizeHint = 128);

	// Destructor cleanup
	~StoreBuffer() = default;

	// Data insertion methods
	VOID InsertByte(_In_ CONST UCHAR Byte); // Inserts a single byte
	VOID InsertData(_In_ CONST LPVOID Data, _In_ CONST std::size_t Size); // Inserts raw data
	VOID InsertLong(_In_ CONST ULONG Long); // Inserts a 32-bit unsigned long
	VOID InsertLongLong(_In_ CONST ULONGLONG LongLong); // Inserts a 64-bit unsigned long long
	VOID InsertShort(_In_ CONST USHORT Short); // Inserts a 16-bit unsigned short
	VOID InsertStringA(_In_ CONST PSTR String); // Inserts a null-terminated ANSI string
	VOID InsertStringA(_In_ CONST std::string& String); // Inserts a null-terminated ANSI string from std::string
	VOID InsertStringW(_In_ CONST PWSTR String); // Inserts a null-terminated wide string
	VOID InsertStringW(_In_ CONST std::wstring& String); // Inserts a null-terminated wide string from std::wstring

	// Buffer access
	CONST PUCHAR Buffer(VOID) CONST;
	CONST std::size_t Position(VOID) CONST;
	PUCHAR Buffer(VOID);

	// Buffer information retrieval
	CONST std::size_t AllocatedSize(VOID) CONST;
	CONST std::size_t Size(VOID) CONST;

	// Buffer manipulation
	VOID Resize(_In_ CONST std::size_t NewSize);
	VOID SetPosition(_In_ CONST std::size_t NewPosition);

private:
	// Internal members
	mutable std::size_t m_Position;
	std::unique_ptr<UCHAR[]> m_Buffer;
	std::size_t m_Size, m_ValidSize;

	// Private helper methods
	VOID InternalExpand(_In_ CONST std::size_t RequiredBytes);
	VOID UpdateValidSize(VOID);
};

#endif