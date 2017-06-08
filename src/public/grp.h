#ifndef GRP_H
#define GRP_H

#include <fstream>
#include <string>
#include <vector>

#include "memoryfile.h"

typedef struct
{
	char		szLabel[12]; // File name
	uint32_t	nSize; // Size in bytes
} GRPEmbeddedFile_t;

//Stores information from GRPEmbeddedFile_t
class GRPEmbeddFileContainer
{
public:
	GRPEmbeddFileContainer( const std::string &fileName, unsigned long size, unsigned long position );

	std::string GetFileName() { return m_szfileName; }
	unsigned long GetSize_Bytes() { return m_Size; }
	unsigned long GetSize_KBytes() { return ( m_Size / 1000 ); }

	unsigned long GetPosition() { return m_Position; }
private:
	std::string m_szfileName; // Name of the file this refers to
	unsigned long m_Size; // Size of file in bytes
	unsigned long m_Position; // Position of file in GRP
};

class GRP
{
public:
	GRP( const std::string &grpFileName );
	~GRP();

	// Dumps GRP Contents
	void DumpGRP();

	// Just some information spew
	void SpewFileInformation();

	unsigned long GetNumFileEntries() { return m_FileEntries; }
	bool IsValid() { return m_bValid; }

	void GetFileInGRP( const std::string &fileName, MemoryReader &fileReader );

private:
	std::ifstream m_GRPFile;

	std::vector< GRPEmbeddFileContainer > m_vecFiles;
	std::string m_szfileName;

	unsigned long m_FileEntries;
	bool m_bValid;
};

#endif