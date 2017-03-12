#ifndef MEMORYFILE_H
#define MEMORYFILE_H

class MemoryReader
{
public:
	MemoryReader();
	MemoryReader( char* memFile, unsigned long long size ); //Opens file in memory on creation

	~MemoryReader();

	//Opens memory for reading
	void Open( char* memFile, unsigned long long size );

	//Completely resets file reader, can also free memory
	void Close();
	void CloseAndFreeMemory();

	bool Good() { return ( m_iStateBits & MemoryFile_State_Good ) != 0; }
	bool Bad() { return !Good(); }

	bool IsEOF() { return ( m_iStateBits & MemoryFile_State_EOF ) != 0; }

	bool IsOpen() { return ( m_pMemoryBuffer != nullptr ); }

	void *GetMemoryBuffer() { return m_pMemoryBuffer; }

	unsigned long long GetPosition() { return m_ullPosition; }
	unsigned long long GetSize() { return m_ullSize; }

	void SeekTo( unsigned long long position );

	int GetStateFlags() { return m_iStateBits; }

	void ReadByte( unsigned char &byte );
	void Read( char *data, unsigned long long count );

	enum StateFlags
	{
		MemoryFile_State_Good = ( 1 << 0 ),
		MemoryFile_State_OpenFailed = ( 1 << 1 ),
		MemoryFile_State_SeekFailed = ( 1 << 2 ),
		MemoryFile_State_SeekOutOfRange = ( 1 << 3 ),
		MemoryFile_State_BufferNotSet = ( 1 << 4 ), //Doesn't always mean good flag is not set
		MemoryFile_State_AlreadyOpen = ( 1 << 5 ),
		MemoryFile_State_NullOpenBuffer = ( 1 << 6 ),
		MemoryFile_State_ReadByteFailed = ( 1 << 7 ),
		MemoryFile_State_ReadFailed = ( 1 << 8 ),
		MemoryFile_State_ReadOutOfRange = ( 1 << 9 ),
		MemoryFile_State_EOF = ( 1 << 10 ),
	};

	void PrintStateFlags();

private:
	void ResetFlags() { m_iStateBits = MemoryFile_State_Good | MemoryFile_State_BufferNotSet; }

	char *m_pMemoryBuffer; //Memory we are reading from

	unsigned int m_iStateBits;

	//Using unsigned long longs is a bit absurd...
	unsigned long long m_ullPosition; //Position inside the memory buffer we are reading from
	unsigned long long m_ullSize; //Size of memory buffer
};

#endif