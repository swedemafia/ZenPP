#include "Precomp.h"

// Constructor initializes members and allocates initial buffer
StoreBuffer::StoreBuffer(std::size_t SizeHint) :
	m_Size(SizeHint), m_Position(0), m_ValidSize(0)
{
	m_Buffer = std::make_unique<UCHAR[]>(SizeHint);
}

// Allocated buffer size retrieval
CONST std::size_t StoreBuffer::AllocatedSize(VOID) CONST
{
    return m_Size;
}

// Read-only buffer retrieval
CONST PUCHAR StoreBuffer::Buffer(VOID) CONST
{
    return m_Buffer.get();
}

// Current position within buffer retrieval
CONST std::size_t StoreBuffer::Position(VOID) CONST
{
    return m_Position;
}

// Valid data size retrieval
CONST std::size_t StoreBuffer::Size(VOID) CONST
{
    return m_ValidSize;
}

// Writable buffer pointer retrieval
PUCHAR StoreBuffer::Buffer(VOID)
{
    return m_Buffer.get();
}

// Inserts a single byte into the buffer
VOID StoreBuffer::InsertByte(CONST UCHAR Byte)
{
    InternalExpand(1);
    m_Buffer[m_Position] = Byte;
    m_Position++;
    UpdateValidSize();
}

// Inserts raw data into the buffer
VOID StoreBuffer::InsertData(CONST LPVOID Data, CONST std::size_t Size)
{
    InternalExpand(Size);
    std::memcpy(&m_Buffer[m_Position], Data, Size);
    m_Position += Size;
    UpdateValidSize();
}

// Inserts a 32-bit unsigned long into the buffer
VOID StoreBuffer::InsertLong(CONST ULONG Long)
{
    InternalExpand(4);
    *(PULONG)&m_Buffer[m_Position] = Long;
    m_Position += 4;
    UpdateValidSize();
}

// Inserts a 64-bit unsigned long long into the buffer
VOID StoreBuffer::InsertLongLong(CONST ULONGLONG LongLong)
{
    InternalExpand(8);
    *(PULONGLONG)&m_Buffer[m_Position] = LongLong;
    m_Position += 8;
    UpdateValidSize();
}

// Inserts a 16-bit unsigned short into the buffer
VOID StoreBuffer::InsertShort(CONST USHORT Short)
{
    InternalExpand(2);
    *(PUSHORT)&m_Buffer[m_Position] = Short;
    m_Position += 2;
    UpdateValidSize();
}

// Inserts a null-terminated ANSI string into the buffer
VOID StoreBuffer::InsertStringA(CONST PSTR String)
{
    std::size_t Length = strlen(String) + 1;
    InternalExpand(Length);
    std::memcpy(&m_Buffer[m_Position], String, Length);
    m_Position += Length;
    UpdateValidSize();
}

// Inserts a null-terminated ANSI string from an std::string into the buffer
VOID StoreBuffer::InsertStringA(CONST std::string& String)
{
    std::size_t Length = String.size() + 1;
    InternalExpand(Length);
    std::memcpy(&m_Buffer[m_Position], String.c_str(), Length);
    m_Position += Length;
    UpdateValidSize();
}

// Inserts a null-terminated wide string into the buffer
VOID StoreBuffer::InsertStringW(CONST PWSTR String)
{
    std::size_t Length = (wcslen(String) << 1) + 2;
    InternalExpand(Length);
    std::memcpy(&m_Buffer[m_Position], String, Length);
    m_Position += Length;
    UpdateValidSize();
}

// Inserts a null-terminated wide string from an std::wstring into the buffer
VOID StoreBuffer::InsertStringW(CONST std::wstring& String)
{
    std::size_t Length = (String.size() << 1) + 2;
    InternalExpand(Length);
    std::memcpy(&m_Buffer[m_Position], String.c_str(), Length);
    m_Position += Length;
    UpdateValidSize();
}

// Internally expands the buffer if the required space exceeds the current capacity
VOID StoreBuffer::InternalExpand(CONST std::size_t RequiredBytes)
{
    if (m_Position + RequiredBytes > m_Size) {
        // Resize to either double the current size or accomdate the required bytes, whichever is larger
        Resize((m_Size << 1) > m_Position + RequiredBytes ? m_Size << 1 : m_Position + RequiredBytes + (m_Size << 1));
    }
}

// Resizes the buffer to a new capacity
VOID StoreBuffer::Resize(CONST std::size_t NewSize)
{
    PUCHAR NewBuffer;

    if (m_Buffer.get()) {
        NewBuffer = reinterpret_cast<PUCHAR>(std::realloc(m_Buffer.release(), NewSize + 2));
    }
    else {
        NewBuffer = reinterpret_cast<PUCHAR>(malloc(NewSize + 2));
    }

    if (!NewBuffer)
        throw std::bad_alloc();

    m_Buffer = std::unique_ptr<UCHAR[], std::default_delete<UCHAR[]>>(NewBuffer);
    m_Size = NewSize;
}

// Sets the position within the buffer, expanding if needed
VOID StoreBuffer::SetPosition(CONST std::size_t NewPosition)
{
    if (NewPosition > m_Size)
        InternalExpand(NewPosition - m_Size);
    m_Position = NewPosition;
    UpdateValidSize();
}

// Updates the valid data size based on the current position
VOID StoreBuffer::UpdateValidSize(VOID)
{
    if (m_Position > m_ValidSize)
        m_ValidSize = m_Position;
}