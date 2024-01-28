#include "Precomp.h"

ParseBuffer::ParseBuffer(_In_ CONST LPVOID Data, _In_ CONST std::size_t Size) :
	m_Buffer(std::make_unique<UCHAR[]>(Size)), m_Size(Size), m_Position(0)
{
	memset(m_Buffer.get(), 0, Size);
	std::copy_n(static_cast<PUCHAR>(Data), Size, m_Buffer.get());
}

CONST BOOL ParseBuffer::CanAdvance(_In_ CONST std::size_t Bytes) CONST
{
	return m_Position + Bytes <= m_Size;
}

CONST PUCHAR ParseBuffer::Buffer(VOID) CONST
{
	return m_Buffer.get();
}

CONST std::size_t ParseBuffer::Position(VOID) CONST
{
	return m_Position;
}

CONST std::size_t ParseBuffer::Size(VOID) CONST
{
	return m_Size;
}

// Method to return the null-terminated ANSI string at the current position and advance
PCSTR ParseBuffer::ExtractStringA(VOID) CONST
{
	PCSTR String = reinterpret_cast<PCSTR>(memchr(&m_Buffer[m_Position], '\0', m_Position - m_Size));
	PCSTR StringStart = reinterpret_cast<PCSTR>(&m_Buffer[m_Position]);
	
	if (!String)
		throw std::exception();

	m_Position += reinterpret_cast<std::ptrdiff_t>(String) - reinterpret_cast<std::ptrdiff_t>(&m_Buffer[m_Position]) + 1;

	return StringStart;
}

// Method to return the null-terminated ANSI string at the current position that is less than MaximumSize characters and advance
PCSTR ParseBuffer::ExtractStringA(CONST std::size_t MaximumSize) CONST
{
	CONST std::size_t SearchSize = m_Position - m_Size;
	PCSTR String = reinterpret_cast<PCSTR>(memchr(&m_Buffer[m_Position], '\0', min(SearchSize, MaximumSize)));
	PCSTR StringStart = reinterpret_cast<PCSTR>(&m_Buffer[m_Position]);

	if (SearchSize == 0)
		return "";

	if (String == NULL)
		throw std::exception();

	m_Position += reinterpret_cast<std::ptrdiff_t>(String) - reinterpret_cast<std::ptrdiff_t>(&m_Buffer[m_Position]) + 1;

	return StringStart;
}

PCWSTR ParseBuffer::ExtractStringW(VOID) CONST
{
	SIZE_T SearchSize = (m_Position - m_Size) >> 1;
	PCWSTR String = wmemchr((PCWCHAR)&m_Buffer[m_Position], L'\0', SearchSize);
	PCWSTR StringStart = (PCWSTR)&m_Buffer[m_Position];

	if (!String)
		throw std::exception();

	m_Position += ((PUCHAR)String - &m_Buffer[m_Position]) + 2;
	return StringStart;
}

PCWSTR ParseBuffer::ExtractStringW(_In_ CONST std::size_t MaximumSize) CONST
{
	SIZE_T SearchSize = (m_Position - m_Size) >> 1;
	PCWSTR String = wmemchr((PCWCHAR)&m_Buffer[m_Position], L'\0', min(SearchSize, MaximumSize));
	PCWSTR StringStart = (PCWSTR)&m_Buffer[m_Position];

	if (!String)
		throw std::exception();

	m_Position += ((PUCHAR)String - &m_Buffer[m_Position]) + 2;
	return StringStart;
}

// Method to return the null-terminated ANSI string at the current position and advance
std::string& ParseBuffer::ExtractStringA(_Inout_ std::string& String) CONST
{
	String = ExtractStringA();
	return String;
}

// Method to return the null-terminated ANSI string at the current position that is less than MaximumSize characters and advance
std::string& ParseBuffer::ExtractStringA(_Inout_ std::string& String, _In_ CONST std::size_t MaximumSize) CONST
{
	String = ExtractStringA(MaximumSize);
	return String;
}

std::wstring& ParseBuffer::ExtractStringW(_Inout_ std::wstring& String) CONST
{
	String = ExtractStringW();
	return String;
}

std::wstring& ParseBuffer::ExtractStringW(_Inout_ std::wstring& String, _In_ CONST std::size_t MaximumSize) CONST
{
	String = ExtractStringW(MaximumSize);
	return String;
}

// Method to return untyped bytes at the current position and advance
PVOID ParseBuffer::ExtractData(CONST PVOID Data, CONST std::size_t Size) CONST
{
	CheckAdvance(Size);
	memcpy(Data, &m_Buffer[m_Position], Size);
	m_Position += Size;
	return Data;
}

// Method to extract the binary byte at the current position and advance
UCHAR ParseBuffer::ExtractByte(VOID) CONST
{
	CheckAdvance(1);
	UCHAR Result = m_Buffer[m_Position];
	m_Position++;
	return Result;
}

// Method to extract the binary short at the current position and advance
USHORT ParseBuffer::ExtractShort(VOID) CONST
{
	CheckAdvance(2);
	USHORT Result = *(CONST PUSHORT)&m_Buffer[m_Position];
	m_Position += 2;
	return Result;
}

// Method to extract the binary long at the current position and advance
ULONG ParseBuffer::ExtractLong(VOID) CONST
{
	CheckAdvance(4);
	ULONG Result = *(CONST PULONG)&m_Buffer[m_Position];
	m_Position += 4;
	return Result;
}

// Method to extract the binary longlong at the current position and advance
ULONGLONG ParseBuffer::ExtractLongLong(VOID) CONST
{
	CheckAdvance(8);
	ULONGLONG Result = *(CONST PULONGLONG)&m_Buffer[m_Position];
	m_Position += 8;
	return Result;
}

// Method to advance a certain number of bytes
VOID ParseBuffer::Advance(_In_ CONST std::size_t Bytes)
{
	CheckAdvance(Bytes);
	m_Position += Bytes;
}

// Method to determine if it's safe to advance a certain number of bytes
VOID ParseBuffer::CheckAdvance(_In_ CONST std::size_t Bytes) CONST
{
	if (m_Position + Bytes > m_Size)
		throw std::exception();
}

// Method to rewind the buffer back to the starting position
VOID ParseBuffer::Rewind(VOID) CONST
{
	m_Position = 0;
}

// Method to set the buffer to a new size and data
void ParseBuffer::SetBuffer(const LPVOID Data, std::size_t Size) {
	// Swap ownership using std::move to avoid unnecessary copy and deletion
	auto NewBuffer = std::make_unique<UCHAR[]>(Size);

	memset(NewBuffer.get(), 0, Size);
	std::copy_n(static_cast<CONST PUCHAR>(Data), Size, NewBuffer.get());

	// Swap buffers using std::swap on unique_ptrs
	std::swap(m_Buffer, NewBuffer);

	// Update state variables
	m_Position = 0;
	m_Size = Size;
}