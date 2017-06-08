#include "map.h"

#include "basehelpers.h"
#include "memoryfile.h"
#include "enginemain.h"

CeilFloorStat::CeilFloorStat( int16_t stat )
{
	m_bParallaxing = ( stat >> 0 ) & 0x1;
	m_bSloped = ( stat >> 1 ) & 0x1;
	m_bXYSwap = ( stat >> 2 ) & 0x1;
	m_bDoubleSmooshiness = ( stat >> 3 ) & 0x1;
	m_bXFlip = ( stat >> 4 ) & 0x1;
	m_bYFlip = ( stat >> 5 ) & 0x1;
	m_bAlignTextureToFirstWall = ( stat >> 6 ) & 0x1;
}

void PrintSectorInfo( Sector_t *pSector )
{
	if ( pSector )
	{
		MsgC( "wallptr: %d\n", pSector->wallptr );
		MsgC( "wallnum: %d\n", pSector->wallnum );
		MsgC( "ceilingz: %d\n", pSector->ceilingz );
		Msg( "ceilingstat: " );

		for ( int i = 0; i < 16; i++ )
		{
			bool bit = ( pSector->ceilingstat >> i ) & 0x1;
			MsgC( "%d ", bit );
		}

		Msg( "\n" );
	}
}

Map::Map( const std::string &mapName )
{
	m_iMapVersion = 0;

	m_iPosX = 0;
	m_iPosY = 0;
	m_iPosZ = 0;
	m_iAng = 0;
	m_iCurSectNum = 0;
	m_iNumSectors = 0;
	m_pSectors = nullptr;
	m_iNumWalls = 0;
	m_pWalls = nullptr;
	m_iNumSprites = 0;
	m_pSprites = nullptr;

	MemoryReader mapFile;
	GRP *grp = GetEngine()->GetGRP();

	if ( grp != nullptr )
		grp->GetFileInGRP( mapName, mapFile );

	Msg( "sizeof( uint32_t ): " + std::to_string( sizeof( uint32_t ) ) + "\n" );
	Msg( "sizeof( uint16_t ): " + std::to_string( sizeof( uint16_t ) ) + "\n" );
	Msg( "sizeof( uint8_t ): " + std::to_string( sizeof( uint8_t ) ) + "\n" );

	while ( mapFile.IsOpen() && mapFile.Good() )
	{
		mapFile.Read( ( char* )&m_iMapVersion, sizeof( uint32_t ) );
		Msg( "Map version is: " + std::to_string( m_iMapVersion ) + "\n" );

		if ( m_iMapVersion != 7 )
		{
			mapFile.CloseAndFreeMemory();
			break;
		}

		mapFile.Read( ( char* )&m_iPosX, sizeof( int32_t ) );
		mapFile.Read( ( char* )&m_iPosY, sizeof( int32_t ) );
		mapFile.Read( ( char* )&m_iPosZ, sizeof( int32_t ) );

		Msg( "posX: " + std::to_string( m_iPosX ) + "\n" );
		Msg( "posY: " + std::to_string( m_iPosY ) + "\n" );
		Msg( "posZ: " + std::to_string( m_iPosZ ) + "\n" );

		mapFile.Read( ( char* )&m_iAng, sizeof( uint16_t ) );
		mapFile.Read( ( char* )&m_iCurSectNum, sizeof( uint16_t ) );
		mapFile.Read( ( char* )&m_iNumSectors, sizeof( uint16_t ) );

		Msg( "ang: " + std::to_string( m_iAng ) + "\n" );
		Msg( "cursectnum: " + std::to_string( m_iCurSectNum ) + "\n" );
		Msg( "Number of sectors: " + std::to_string( m_iNumSectors ) + "\n" );

		m_pSectors = new Sector_t*[m_iNumSectors];

		for ( uint16_t i = 0; i < m_iNumSectors; i++ )
		{
			Sector_t *sector = new Sector_t;
			mapFile.Read( ( char* )sector, ( sizeof( Sector_t ) ) );
			m_pSectors[i] = sector;

			//PrintSectorInfo( m_pSectors[i] );
		}

		PrintSectorInfo( m_pSectors[0] );

		mapFile.PrintStateFlags();
		mapFile.CloseAndFreeMemory();
	}
}

Map::~Map()
{
	if ( m_pSectors )
	{
		for ( uint16_t i = 0; i < m_iNumSectors; i++ )
		{
			if ( m_pSectors[i] )
				delete m_pSectors[i];
		}

		delete[] m_pSectors;
	}
}
