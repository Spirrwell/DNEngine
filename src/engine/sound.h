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

enum VOCCodecID
{
	CodecID_8bitsUnsignedPCM = 0x00,
	CodecID_4bitsto8bitsCreativeADPCM = 0x01,
	CodecID_3bitsto8bitsCreativeADPCM = 0x02, //AKA 2.6 bits https://wiki.multimedia.cx/index.php?title=Creative_Voice
	CodecID_2bitsto8bitsCreativeADPCM = 0x03,
	CodecID_16bitsSignedPCM = 0x04,
	CodecID_alaw = 0x06,
	CodecID_ulaw = 0x07,
	CodecID_4bitsto16bitsCreativeADPCM = 0x0200, //Only valid in block type 0x09
	CodecID_Invalid = 0x666, //Imaginary codec type
};

typedef struct
{
	unsigned char frequencyDivisor;
	unsigned char codecID;

	//bytes at this point are audio data at size of blockSize - sizeof( VOCDataBlock_SoundData_t )
} VOCDataBlock_SoundData_t;

typedef struct
{
	uint32_t sampleRate;
	unsigned char bitsPerSample;
	unsigned char numChannels;
	unsigned char codecID[2];
	unsigned char reserved[4]; //Empty space?
	//bytes at this point are audio data at size of blockSize - sizeof( VOCDataBlock_SoundDataNewFormat_t )
} VOCDataBlock_SoundDataNewFormat_t;

typedef struct
{
	char identifierStr[19];
	unsigned char byteEOF;
	short headerSize;
	unsigned char version[2]; //0 = Minor, 1 = Major
	short validity;

} VOCMainHeader_t;

void PrintCodecID( VOCCodecID id );

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