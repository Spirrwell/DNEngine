#include "grp.h"
#include "basehelpers.h"
#include <algorithm>

GRPEmbeddFileContainer::GRPEmbeddFileContainer( const std::string &fileName, unsigned long size, unsigned long position )
{
	//An unsigned long should always be at LEAST 4 bytes, we may just want to read 4 bytes directly instead of sizeof
	static_assert( sizeof ( unsigned long ) == 4, "Unsigned long needs to be 4 bytes for GRP reading." );

	m_szfileName = fileName;
	m_Size = size;
	m_Position = position;
}

GRP::GRP( const std::string &grpFileName )
{
	m_bValid = true;

	m_szfileName = grpFileName;

	m_GRPFile.open( m_szfileName, std::ifstream::in | std::ifstream::binary );

	if ( !m_GRPFile.good() )
	{
		Msg( "GRP failed to open\n" );
		m_GRPFile.close();
		m_bValid = false;
		return;
	}

	char buffer[13];
	unsigned long fileCount, fileIndex;
	GRPEmbeddedFile_t **fileList;

	m_GRPFile.read( buffer, 12 );
	buffer[12] = '\0';

	if ( strcmp( buffer, "KenSilverman" ) != 0 )
	{
		Msg( "Not a valid Duke3D GRP file!\n" );
		m_GRPFile.close();
		m_bValid = false;
		return;
	}

	m_GRPFile.read( ( char* )&fileCount, sizeof( unsigned long ) );

	m_FileEntries = fileCount;

	fileList = new GRPEmbeddedFile_t*[fileCount];

	for ( unsigned long i = 0; i < fileCount; i++ )
		fileList[i] = nullptr;

	for ( fileIndex = 0; fileIndex < fileCount; fileIndex++ )
	{
		GRPEmbeddedFile_t *file = new GRPEmbeddedFile_t;
		m_GRPFile.read( ( char* )file, sizeof( GRPEmbeddedFile_t ) );
		fileList[fileIndex] = file;
	}

	for ( fileIndex = 0; fileIndex < fileCount; fileIndex++ )
	{
		memcpy( buffer, fileList[fileIndex]->szLabel, 12 );
		buffer[12] = '\0';

		unsigned long position = ( unsigned long )m_GRPFile.tellg();
		m_GRPFile.seekg( position + fileList[fileIndex]->nSize );

		if ( !m_GRPFile.good() )
		{
			Msg( "GRP File failed to seek position!\n" );
			m_vecFiles.clear();
			m_bValid = false;
			m_GRPFile.close();
			break;
		}

		GRPEmbeddFileContainer fileContainer( buffer, fileList[fileIndex]->nSize, position );

		m_vecFiles.push_back( fileContainer );
	}

	for ( unsigned long i = 0; i < fileCount; i++ )
		delete fileList[i];

	delete[] fileList;

	m_GRPFile.clear();
	m_GRPFile.seekg( 0, std::ios::beg );
}

GRP::~GRP()
{
	if ( m_GRPFile.is_open() )
		m_GRPFile.close();
}

void GRP::DumpGRP()
{
	if ( !IsValid() )
	{
		Msg( "Attempting to dump invalid GRP file!\n" );
		return;
	}

	std::ofstream outputFile;

	if ( !m_GRPFile.is_open() || !m_GRPFile.good() )
	{
		Msg( "GRP read failed\n" );
		return;
	}

	unsigned long byteIndex;
	bool failed = false;

	for ( unsigned int i = 0; i < m_vecFiles.size(); i++ )
	{
		m_GRPFile.seekg( m_vecFiles[i].GetPosition() );

		if ( !m_GRPFile.good() )
		{
			Msg( "GRP file failed to seek!\n" );
			break;
		}

		outputFile.open( "output/" + m_vecFiles[i].GetFileName(), std::ofstream::out | std::ofstream::binary );

		for ( byteIndex = 0; byteIndex < m_vecFiles[i].GetSize_Bytes(); byteIndex++ )
		{
			char byte;
			m_GRPFile.get( byte );

			if ( m_GRPFile.eof() )
			{
				Msg( "Unexpected EOF in GRP!\n" );
				break;
			}

			outputFile.put( byte );

			if ( !outputFile.good() || outputFile.eof() )
			{
				Msg( "Unable to write file!\n" );
				break;
			}
		}

		outputFile.close();

		if ( failed )
			break;
	}

	m_GRPFile.clear();
	m_GRPFile.seekg( 0, std::ios::beg );
}

void GRP::SpewFileInformation()
{
	for ( unsigned int i = 0; i < m_vecFiles.size(); i++ )
		Msg( m_vecFiles[i].GetFileName() + " | " + std::to_string( m_vecFiles[i].GetSize_KBytes() ) + "KB | Position: " + std::to_string( m_vecFiles[i].GetPosition() ) + "\n" );
}

void GRP::GetFileInGRP( const std::string &fileName, MemoryFileReader &fileReader )
{
	if ( !IsValid() || !m_GRPFile.is_open() )
	{
		Msg( "Not valid GRP file!\n" );
		return;
	}

	GRPEmbeddFileContainer *fileContainer = nullptr;

	for ( unsigned int i = 0; i < m_vecFiles.size(); i++ )
	{
		std::string embeddedFileName = m_vecFiles[i].GetFileName();
		std::string inputFileName = fileName;

		std::transform( embeddedFileName.begin(), embeddedFileName.end(), embeddedFileName.begin(), ::tolower );
		std::transform( inputFileName.begin(), inputFileName.end(), inputFileName.begin(), ::tolower );

		if ( inputFileName == embeddedFileName )
		{
			fileContainer = &m_vecFiles[i];
			break;
		}
	}
	if ( fileContainer == nullptr )
	{
		Msg( "File container not found!\n" );
		return;
	}

	m_GRPFile.seekg( fileContainer->GetPosition() );

	if ( fileContainer != nullptr )
	{
		//Should really look into creating a virtual filesystem to avoid copying files into memory.
		void *buffer = malloc( fileContainer->GetSize_Bytes() );

		m_GRPFile.read( ( char* )buffer, fileContainer->GetSize_Bytes() );
		fileReader.Open( ( char* )buffer, fileContainer->GetSize_Bytes() );

		if ( !fileReader.Good() )
		{
			fileReader.Close();
			free( buffer );
		}
	}

	m_GRPFile.clear();
	m_GRPFile.seekg( 0, std::ios::beg );
}