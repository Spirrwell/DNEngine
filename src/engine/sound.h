#ifndef SOUND_H
#define SOUND_H

#include <string>

#include "fmod.hpp"

enum VOCBlockType
{
	BlockType_Terminator = 0x00,
	BlockType_SoundData = 0x01,
	BlockType_SoundDataContinued = 0x02,
	BlockType_Silence = 0x03,
	BlockType_Marker = 0x04,
	BlockType_Text = 0x05,
	BlockType_RepeatStart = 0x06,
	BlockType_RepeatEnd = 0x07,
	BlockType_ExtraInfo = 0x08,
	BlockType_SoundDataNewFormat = 0x09,
};

typedef struct
{
	unsigned char blockType;
	unsigned char blockSize[3];
	
} VOCDataBlock_t;

typedef struct
{
	unsigned char frequencyDivisor;
	unsigned char codecID;
	void *audioData;
} VOCDataBlock_SoundData_t;

typedef struct
{
	char identifierStr[19];
	unsigned char byteEOF;
	short headerSize;
	unsigned char version[2]; //0 = Minor, 1 = Major
	short validity;

} VOCMainHeader_t;

class Sound
{
public:
	Sound( std::string fileName, bool bLooping = false, bool bGlobal = false, float flRadius = 200.0f );
	~Sound();

private:

	bool m_bGlobal;
	bool m_bLooping;

	float m_flRadius;

	FMOD::Channel *m_pChannel;
	FMOD::Sound *m_pSound;

	FMOD_CREATESOUNDEXINFO m_SoundExInfo;
};

#endif