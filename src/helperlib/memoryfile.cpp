#include "memoryfile.h"
#include <iostream>

MemoryReader::MemoryReader()
{
	m_pMemoryBuffer = nullptr;
	ResetFlags();
	m_ullPosition = 0;
	m_ullSize = 0;
}

MemoryReader::MemoryReader( char *memFile, unsigned long long size ) : MemoryReader()
{
	Open( memFile, size );
}

MemoryReader::~MemoryReader()
{

}

void MemoryReader::Open( char *memFile, unsigned long long size )
{
	if ( m_pMemoryBuffer != nullptr )
	{
		m_iStateBits &= ~( MemoryFile_State_Good );
		m_iStateBits |= ( MemoryFile_State_AlreadyOpen | MemoryFile_State_OpenFailed ) ;
		return;
	}

	if ( memFile == nullptr )
	{
		m_iStateBits &= ~( MemoryFile_State_Good );
		m_iStateBits |= ( MemoryFile_State_OpenFailed | MemoryFile_State_NullOpenBuffer );
		return;
	}

	m_pMemoryBuffer = memFile;
	m_iStateBits &= ~( MemoryFile_State_BufferNotSet );

	m_ullPosition = 0;
	m_ullSize = size;
}

void MemoryReader::Close()
{
	m_pMemoryBuffer = nullptr;
	m_ullPosition = 0;
	m_ullSize = 0;

	ResetFlags();
}

void MemoryReader::CloseAndFreeMemory()
{
	if ( m_pMemoryBuffer != nullptr )
		free( m_pMemoryBuffer );

	Close();
}

void MemoryReader::SeekTo( unsigned long long position )
{
	if ( m_pMemoryBuffer == nullptr )
	{
		m_iStateBits &= ~( MemoryFile_State_Good );
		m_iStateBits |= ( MemoryFile_State_SeekFailed | MemoryFile_State_BufferNotSet );
		return;
	}

	if ( position > GetSize() )
	{
		m_iStateBits &= ~( MemoryFile_State_Good );
		m_iStateBits |= ( MemoryFile_State_SeekFailed | MemoryFile_State_SeekOutOfRange );
		return;
	}

	//Should maybe set EOF if position is equal to size
	m_ullPosition = position;

	if ( m_ullPosition < GetSize() )
		m_iStateBits &= ~( MemoryFile_State_EOF );
}

void MemoryReader::ReadByte( unsigned char &byte )
{
	if ( m_pMemoryBuffer == nullptr )
	{
		m_iStateBits &= ~( MemoryFile_State_Good );
		m_iStateBits |= ( MemoryFile_State_ReadByteFailed | MemoryFile_State_BufferNotSet );
		return;
	}

	if ( m_ullPosition > GetSize() )
	{
		m_iStateBits &= ~( MemoryFile_State_Good );
		m_iStateBits |= ( MemoryFile_State_ReadByteFailed | MemoryFile_State_ReadOutOfRange );
		return;
	}

	byte = ( unsigned char )m_pMemoryBuffer[m_ullPosition];

	if ( m_ullPosition >= m_ullSize )
	{
		m_iStateBits |= ( MemoryFile_State_EOF );
		m_ullPosition = m_ullSize; //Should never be greater than the size, but let's make sure
	}
	else
		m_ullPosition++;
}

void MemoryReader::Read( char *data, unsigned long long count )
{
	if ( m_pMemoryBuffer == nullptr )
	{
		m_iStateBits &= ~( MemoryFile_State_Good );
		m_iStateBits |= ( MemoryFile_State_ReadFailed | MemoryFile_State_BufferNotSet );
		return;
	}

	if ( m_ullPosition > GetSize() || m_ullPosition + count > GetSize() )
	{
		m_iStateBits &= ~( MemoryFile_State_Good );
		m_iStateBits |= ( MemoryFile_State_ReadFailed | MemoryFile_State_ReadOutOfRange );
		return;
	}

	for ( unsigned long long i = 0; i < count; i++ )
	{
		data[i] = m_pMemoryBuffer[m_ullPosition + i];
	}

	m_ullPosition += count;

	if ( m_ullPosition >= m_ullSize )
	{
		m_iStateBits |= ( MemoryFile_State_EOF );
		m_ullPosition = m_ullSize; //Should never be greater than the size, but let's make sure
	}
}

void MemoryReader::PrintStateFlags()
{
	std::cout << "MemoryFileReader State Flags: \n\n";

	if ( m_iStateBits & MemoryFile_State_Good )
		std::cout << "MemoryFile_State_Good\n";

	if ( m_iStateBits & MemoryFile_State_OpenFailed )
		std::cout << "MemoryFile_State_OpenFailed\n";

	if ( m_iStateBits & MemoryFile_State_SeekFailed )
		std::cout << "MemoryFile_State_SeekFailed\n";

	if ( m_iStateBits & MemoryFile_State_SeekOutOfRange )
		std::cout << "MemoryFile_State_SeekOutOfRange\n";

	if ( m_iStateBits & MemoryFile_State_BufferNotSet )
		std::cout << "MemoryFile_State_BufferNotSet\n";

	if ( m_iStateBits & MemoryFile_State_AlreadyOpen )
		std::cout << "MemoryFile_State_AlreadyOpen\n";

	if ( m_iStateBits & MemoryFile_State_NullOpenBuffer )
		std::cout << "MemoryFile_State_NullOpenBuffer\n";

	if ( m_iStateBits & MemoryFile_State_ReadByteFailed )
		std::cout << "MemoryFile_State_ReadByteFailed\n";

	if ( m_iStateBits & MemoryFile_State_ReadFailed )
		std::cout << "MemoryFile_State_ReadFailed\n";

	if ( m_iStateBits & MemoryFile_State_ReadOutOfRange )
		std::cout << "MemoryFile_State_ReadOutOfRange\n";

	if ( m_iStateBits & MemoryFile_State_EOF )
		std::cout << "MemoryFile_State_EOF\n";
}