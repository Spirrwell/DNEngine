#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <string>

// Documentation on the .map format for the BUILD engine can be found here: http://www.shikadi.net/moddingwiki/MAP_Format_(Build)/

// This is the original sector limit of the .map format.
#define MAX_SECTORS 1024

// This is the original wall limit of the .map format
#define MAX_WALLS 8192

// Come up with a better name for this!
class CeilFloorStat
{
	CeilFloorStat( int16_t stat );

	inline bool IsParallaxing() { return m_bParallaxing; }
	inline bool IsSloped() { return m_bSloped; }
	inline bool IsXYSwapped() { return m_bXYSwap; }
	inline bool IsDoubleSmooshiness() { return m_bDoubleSmooshiness; }
	inline bool IsXFlip() { return m_bXFlip; }
	inline bool IsYFlip() { return m_bYFlip; }
	inline bool IsTextureAlignedToFirstWall() { return m_bAlignTextureToFirstWall; }

private:
	bool m_bParallaxing;
	bool m_bSloped;
	bool m_bXYSwap;
	bool m_bDoubleSmooshiness;
	bool m_bXFlip;
	bool m_bYFlip;
	bool m_bAlignTextureToFirstWall;
};

typedef struct
{
	int16_t wallptr; // Index to first wall in sector
	int16_t wallnum; // Number of walls in sector
	int32_t ceilingz; // Z-coordinate (height) of ceiling at first point of sector
	int32_t floorz; // Z-coordinate (height) of floor at first point of sector

	// bit 0: 1 = parallaxing, 0 = not
	// bit 1: 1 = sloped, 0 = not
	// bit 2: 1 = swap x&y, 0 = not
	// bit 3: 1 = double smooshiness (what the hell does this mean?)
	// bit 4: 1 = x-flip
	// bit 5: 1 = y-flip
	// bit 6: 1 = align texture to first wall of sector
	// bits 7-15: reserved
	int16_t ceilingstat;
	int16_t floorstat;

	int16_t ceilingpicnum; // Ceiling texture (index into ART file)
	int16_t ceilinghtheinum; // Slope value (rise/run; 0 = parallel to floor, 4096 = 45 degrees)
	int8_t ceilingshade; // Shade offset
	uint8_t ceilingpal; // Palette lookup table number (0 = standard colors)
	uint8_t ceilingxpanning; // Texture coordinate X-offset for ceiling
	uint8_t ceilingypanning; // Texture coordinate Y-offset for ceiling

	int16_t floorpicnum; // Floor texture (index into ART file)
	int16_t floorheinum; // Slope value (rise/run; 0 = parallel to floor, 4096 = 45 degrees)
	int8_t floorshade; // Shade offset
	uint8_t floorpal; // Palette lookup table number (0 = standard colors)
	uint8_t floorxpanning; // Texture coordinate X-offset for floor
	uint8_t floorypanning; // Texture coordinate Y-offset for floor

	uint8_t visibility; // How fast an area changes shade relative to distance
	uint8_t filler; // Padding byte
	int16_t lotag; // Significance is game-specific (Triggers, etc.)
	int16_t hitag; // Significance is game-specific (Triggers, etc.)
	int16_t extra; // Significance is game-specific
} Sector_t;

typedef struct
{
	int32_t x; // X-coordinate of left side of wall (right side coordinate is obtained from the next wall's left side)
	int32_t y; // Y-coordinate of left side of wall (right side coordinate is obtained from the next wall's left side)
	int16_t point2; // Index to next wall on the right (always in the same sector)
	int16_t nextwall; // Index to wall on other side of wall (-1 if there is no sector there)
	int16_t nextsector; // Index to sector on other side of wall (-1 if there is no sector)

	// bit 0: 1 = Blocking wall (use with clipmove, getzrange)
	// bit 1: 1 = bottoms of invisible walls swapped, 0 = not
	// bit 2: 1 = align picture on bottom (for doors), 0 = top
	// bit 3: 1 = x-flipped, 0 = normal
	// bit 4: 1 = masking wall, 0 = not
	// bit 5: 1 = 1-way wall, 0 = not
	// bit 6: 1 = Blocking wall (use with hitscan / cliptype 1)
	// bit 7: 1 = Transluscence, 0 = not
	// bit 8: 1 = y-flipped, 0 = normal
	// bit 9: 1 = Transluscence reversing, 0 = normal
	// bits 10-15: reserved
	int16_t cstat;

	int16_t picnum; // Texture index into ART file
	int16_t overpicnum; // Texture index into ART file for masked/one-way walls
	int8_t shade; // Shade offset of wall
	uint8_t pal; // Palette lookup table number (0 = standard colors)

	// Change pixel size to stretch/shrink textures
	uint8_t xrepeat;
	uint8_t yrepeat;

	// Offset for aligning textures
	uint8_t xpanning;
	uint8_t ypanning;

	int16_t lotag; // Significance is game-specific (Triggers, etc.)
	int16_t hitag; // Significance is game-specific (Triggers, etc.)
	int16_t extra; // Significance is game-specific
} Wall_t;

typedef struct
{
	int32_t x; // X-coordinate of sprite
	int32_t y; // Y-coordinate of sprite
	int32_t z; // Z-coordinate of sprite

	// bit 0: 1 = Blocking sprite (use with clipmove, getzrange)
	// bit 1: 1 = Transluscence, 0 = Normal
	// bit 2: 1 = X-flipped, 0 = Normal
	// bit 3: 1 = Y-flipped, 0 = Normal
	// bits 4-5: 00 = FACE sprite (default), 01 = WALL sprite (like masked walls), 02 = FLOOR sprite (parallel to ceiling & floors)
	// bit 6: 1 = 1-sided sprite, 0 = Normal
	// bit 7: 1 = Real centered centering, 0 = Foot center
	// bit 8: 1 = Blocking sprite (use with hitscan / cliptype 1)
	// bit 9: 1 = Transluscence reversion, 0 = Normal
	// bits 10-14: reserved
	// bit 15: 1 = Invisible sprite, 0 = Not invisible
	int16_t cstat;

	int16_t picnum; // Texture index into ART file
	int8_t shade; // Shade offset of wall
	uint8_t pal; // Palette lookup table number (0 = standard colors)
	uint8_t clipdist; // Size of the movement clipping square (face sprites only)
	uint8_t filler;

	// Change pixel size to stretch/shrink textures
	uint8_t xrepeat;
	uint8_t yrepeat;

	// Center sprite animations
	int8_t xoffset;
	int8_t yoffset;

	int16_t sectnum; // Current sector of sprite's position
	int16_t statnum; // Current status of sprite (inactive, monster, bullet, etc.)
	int16_t ang; // Angle the sprite is facing

	int16_t owner;
	int16_t xvel;
	int16_t yvel;
	int16_t zvel;

	int16_t lotag; // Significance is game-specific (Triggers, etc.)
	int16_t hitag; // Significance is game-specific (Triggers, etc.)
	int16_t extra; // Significance is game-specific
} Sprite_t;

void PrintSectorInfo( Sector_t *pSector );

class Map
{
public:
	Map( const std::string &mapName );
	~Map();

private:
	// All integer types above 8bit seem to be little endian
	// File format version number (latest is 7 excluding formats introduced by source ports like eduke32)
	int32_t m_iMapVersion;

	// Player start coordinates
	int32_t m_iPosX;
	int32_t m_iPosY;
	int32_t m_iPosZ;

	// Player starting angle (Is it in radians, degrees?)
	int16_t m_iAng;

	// Sector number containing the start point
	int16_t m_iCurSectNum;

	// Number of sectors in the map
	uint16_t m_iNumSectors;

	// Information about each sector
	Sector_t **m_pSectors;

	// Number of walls in the map
	uint16_t m_iNumWalls;

	// Information about each wall
	Wall_t **m_pWalls;

	// Number of sprites in the map
	uint16_t m_iNumSprites;

	// Information about each sprite
	Sprite_t **m_pSprites;
};

#endif