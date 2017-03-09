#include "sound.h"

#include "soundsystem.h"
#include "grp.h"
#include "enginemain.h"
#include "basehelpers.h"

#include <algorithm>

void PrintCodecID( VOCCodecID id )
{
	switch ( id )
	{
	case CodecID_8bitsUnsignedPCM:
		Msg( "CodecID_8bitsUnsignedPCM\n" );
		break;
	case CodecID_4bitsto8bitsCreativeADPCM:
		Msg( "CodecID_4bitsto8bitsCreativeADPCM\n" );
		break;
	case CodecID_3bitsto8bitsCreativeADPCM:
		Msg( "CodecID_3bitsto8bitsCreativeADPCM\n" );
		break;
	case CodecID_2bitsto8bitsCreativeADPCM:
		Msg( "CodecID_2bitsto8bitsCreativeADPCM\n" );
		break;
	case CodecID_16bitsSignedPCM:
		Msg( "CodecID_16bitsSignedPCM\n" );
		break;
	case CodecID_alaw:
		Msg( "CodecID_alaw\n" );
		break;
	case CodecID_ulaw:
		Msg( "CodecID_ulaw\n" );
		break;
	case CodecID_4bitsto16bitsCreativeADPCM:
		Msg( "CodecID_4bitsto16bitsCreativeADPCM\n" );
		break;
	default:
		break;
	}
}

//Sound::Sound( std::string fileName, void *vocData /*= nullptr*/, unsigned int vocSize /*= 0*/, int sampleRate /*= 0*/, VOCCodecID codecID /*= CodecID_Invalid*/, int numChannels /*= 1*/, bool bLooping /*= false*/, bool bGlobal /*= false*/, float flRadius /*= 200.0f*/ )
Sound::Sound( std::string fileName, bool bLooping /*= false*/, bool bGlobal /*= false*/, float flRadius /*= 200.0f*/ )
{
	m_bLooping = bLooping;
	m_bGlobal = bGlobal;
	m_flRadius = flRadius;

	FMOD_RESULT result;

	GRP *grp = GetEngine()->GetGRP();

	MemoryFileReader fileReader;
	grp->GetFileInGRP( fileName, fileReader );

	if ( fileReader.IsOpen() )
	{
		memset( &m_SoundExInfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
		m_SoundExInfo.cbsize = sizeof( FMOD_CREATESOUNDEXINFO );

		std::string fileExtension = ( fileName.substr( fileName.find_last_of( "." ) + 1 ) );
		std::transform( fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower );
		bool bIsVOC = ( fileExtension == "voc" );
		unsigned int fmodFlags = 0;
		void *memoryBuffer = nullptr;

		if ( m_bGlobal )
		{
			fmodFlags = ( FMOD_DEFAULT | FMOD_OPENMEMORY );
			if ( m_bLooping )
				fmodFlags |= FMOD_LOOP_NORMAL;
			if ( bIsVOC )
				fmodFlags |= FMOD_OPENRAW;
		}

		bool bValid = true;

		if ( bIsVOC )
		{
			char buffer[20];
			unsigned int audioSize = 0;
			int sampleRate = 0;
			int numChannels = 1;
			VOCCodecID codecID = CodecID_Invalid;

			VOCMainHeader_t *mainHeader = new VOCMainHeader_t;
			fileReader.Read( ( char* )mainHeader, sizeof( VOCMainHeader_t ) );

			memcpy( buffer, mainHeader->identifierStr, 19 );
			buffer[19] = '\0';

			if ( strcmp(buffer, "Creative Voice File") != 0 )
			{
				Msg( "Not a valid VOC file!\n" );
				bValid = false;
			}

			if ( bValid && mainHeader->validity != ( 0x1234 + ~( mainHeader->version[0] | ( mainHeader->version[1] << 8 ) ) ) )
			{
				Msg( "VOC validity failed!\n" );
				bValid = false;
			}

			delete mainHeader;

			if ( bValid )
			{
				while ( !fileReader.IsEOF() && fileReader.Good() && fileReader.IsOpen() )
				{
					unsigned char blockType;
					unsigned char blockSize[3];
					unsigned int iBlockSize = 0;

					fileReader.ReadByte( blockType );

					//If it's a terminator block, there's no block size to read, signifies EOF.
					if ( blockType != BlockType_Terminator )
					{
						fileReader.Read( ( char* )blockSize, 3 );
						iBlockSize = ( blockSize[0] | blockSize[1] << 8 | blockSize[2] << 16 );
					}

					switch ( blockType )
					{
						case BlockType_Terminator:
						{
#ifdef _DEBUG
							Msg( "[Terminator]\n\n" );
#endif
							fileReader.CloseAndFreeMemory();
							break;
						}
						case BlockType_SoundData:
						{
#ifdef _DEBUG
							Msg( "[SoundData]\n\n" );
#endif
							VOCDataBlock_SoundData_t *soundData = new VOCDataBlock_SoundData_t;
							fileReader.Read( ( char* )soundData, sizeof( VOCDataBlock_SoundData_t ) );
							sampleRate = 1000000 / ( 256 - soundData->frequencyDivisor );

#ifdef _DEBUG
							Msg( "Sample Rate: " + std::to_string( sampleRate ) + "\n" );
#endif
							codecID = (  VOCCodecID )soundData->codecID;

#ifdef _DEBUG
							PrintCodecID( codecID );
#endif

							if ( memoryBuffer == nullptr )
							{
								audioSize = ( iBlockSize - sizeof( VOCDataBlock_SoundData_t ) );
								memoryBuffer = malloc( audioSize );
								fileReader.Read( ( char* )memoryBuffer, audioSize );
							}
							else
							{
								unsigned int audioOffset = ( iBlockSize - sizeof( VOCDataBlock_SoundData_t ) );
								memoryBuffer = realloc( memoryBuffer, audioSize + audioOffset );
								char *localAudioData = ( char* )memoryBuffer;
								fileReader.Read( &localAudioData[audioSize], audioOffset );
								audioSize += audioOffset;
							}

							delete soundData;

							break;
						}
						case BlockType_SoundDataContinued:
						{
#ifdef _DEBUG
							Msg( "[SoundDataContinued]\n\n" );
#endif
							if ( memoryBuffer == nullptr )
							{
								Msg( "Hit continued sound data block without audio data being initialized\n" );
								fileReader.SeekTo( fileReader.GetPosition() + iBlockSize );
							}
							else
							{
								memoryBuffer = realloc( memoryBuffer, audioSize + iBlockSize );
								char *localAudioData = ( char* )memoryBuffer;
								fileReader.Read( &localAudioData[audioSize], iBlockSize );
								audioSize += iBlockSize;
							}
							break;
						}
						case BlockType_Silence:
						{
#ifdef _DEBUG
							Msg( "[Silence]\n\n" );
#endif
							fileReader.SeekTo( fileReader.GetPosition() + iBlockSize );
							break;
						}
						case BlockType_Marker:
						{
#ifdef _DEBUG
							Msg( "[Marker]\n\n" );
#endif
							fileReader.SeekTo( fileReader.GetPosition() + iBlockSize );
							break;
						}
						case BlockType_Text:
						{
#ifdef _DEBUG
							Msg( "[Text]\n\n" );
#endif
							fileReader.SeekTo( fileReader.GetPosition() + iBlockSize );
							break;
						}
						case BlockType_RepeatStart:
						{
#ifdef _DEBUG
							Msg( "[RepeatStart]\n\n" );
#endif
							fileReader.SeekTo( fileReader.GetPosition() + iBlockSize );
							break;
						}
						case BlockType_RepeatEnd:
						{
#ifdef _DEBUG
							Msg( "[RepeatEnd]\n\n" );
#endif
							fileReader.SeekTo( fileReader.GetPosition() + iBlockSize );
							break;
						}
						case BlockType_ExtraInfo:
						{
#ifdef _DEBUG
							Msg( "[ExtraInfo]\n\n" );
#endif
							fileReader.SeekTo( fileReader.GetPosition() + iBlockSize );
							break;
						}
						case BlockType_SoundDataNewFormat:
						{
#ifdef _DEBUG
							Msg( "[SoundDataNewFormat]\n" );
#endif
							VOCDataBlock_SoundDataNewFormat_t *soundData = new VOCDataBlock_SoundDataNewFormat_t;
							fileReader.Read( ( char* )soundData, sizeof( VOCDataBlock_SoundDataNewFormat_t ) );

							sampleRate = soundData->sampleRate;

#ifdef _DEBUG
							Msg( "Sample Rate: " + std::to_string( sampleRate ) + "\n" );
#endif

							codecID = ( VOCCodecID )( soundData->codecID[0] | soundData->codecID[1] << 8 );
#ifdef _DEBUG
							PrintCodecID( codecID );
#endif

							numChannels = soundData->numChannels;

#ifdef _DEBUG
							Msg( "Bits per sample: " + std::to_string( soundData->bitsPerSample ) + "\n" );
#endif

							if ( memoryBuffer == nullptr )
							{
								audioSize = ( iBlockSize - sizeof( VOCDataBlock_SoundDataNewFormat_t ) );
								memoryBuffer = malloc( audioSize );
								fileReader.Read( ( char* )memoryBuffer, audioSize );
							}
							else
							{
								unsigned int audioOffset = ( iBlockSize - sizeof( VOCDataBlock_SoundDataNewFormat_t ) );
								memoryBuffer = realloc( memoryBuffer, audioSize + audioOffset );
								char *localAudioData = ( char* )memoryBuffer;
								fileReader.Read( &localAudioData[audioSize], audioOffset );
								audioSize += audioOffset;
							}

							delete soundData;
							break;
						}
						default:
						{
							Msg( "Unknown block type\n" );
							fileReader.SeekTo( fileReader.GetPosition() + iBlockSize );
						}
					}
				}
				m_SoundExInfo.length = audioSize;
				m_SoundExInfo.defaultfrequency = sampleRate;
				m_SoundExInfo.numchannels = numChannels;

				switch ( codecID )
				{
				case CodecID_8bitsUnsignedPCM:
					m_SoundExInfo.format = FMOD_SOUND_FORMAT_PCM8;
					break;
				case CodecID_4bitsto8bitsCreativeADPCM:
					m_SoundExInfo.format = FMOD_SOUND_FORMAT_BITSTREAM; //Not really sure setting this will do any good
					break;
				case CodecID_3bitsto8bitsCreativeADPCM:
					m_SoundExInfo.format = FMOD_SOUND_FORMAT_BITSTREAM;
					break;
				case CodecID_2bitsto8bitsCreativeADPCM:
					m_SoundExInfo.format = FMOD_SOUND_FORMAT_BITSTREAM;
					break;
				case CodecID_16bitsSignedPCM:
					m_SoundExInfo.format = FMOD_SOUND_FORMAT_PCM16;
					break;
				case CodecID_alaw:
					m_SoundExInfo.format = FMOD_SOUND_FORMAT_BITSTREAM;
					break;
				case CodecID_ulaw:
					m_SoundExInfo.format = FMOD_SOUND_FORMAT_BITSTREAM;
					break;
				case CodecID_4bitsto16bitsCreativeADPCM:
					m_SoundExInfo.format = FMOD_SOUND_FORMAT_BITSTREAM;
					break;
				default:
					m_SoundExInfo.format = FMOD_SOUND_FORMAT_NONE;
					break;
				}

			}

		}
		else
		{
			memoryBuffer = fileReader.GetMemoryBuffer();
			m_SoundExInfo.length = ( unsigned int )fileReader.GetSize();
		}

		if ( bValid )
		{
			result = GetSoundSystem()->GetSystem()->createSound( (const char* )memoryBuffer, fmodFlags, &m_SoundExInfo, &m_pSound );
			GetSoundSystem()->CheckError( result );
		}

		fileReader.CloseAndFreeMemory();

		if ( bIsVOC && memoryBuffer != nullptr )
			free( memoryBuffer );

		memoryBuffer = nullptr;

		result = GetSoundSystem()->GetSystem()->playSound( m_pSound, nullptr, true, &m_pChannel );
		GetSoundSystem()->CheckError( result );

		if ( m_pChannel )
		{
			if ( !m_bGlobal )
			{
			}

			m_pChannel->setPaused( false );
		}
	}
}

Sound::~Sound()
{
	if ( m_pChannel )
		m_pChannel->stop();

	m_pChannel = nullptr;

	m_pSound->release();
}