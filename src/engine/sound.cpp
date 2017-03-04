#include "sound.h"

#include "soundsystem.h"
#include "grp.h"
#include "enginemain.h"

Sound::Sound( std::string fileName, bool bLooping /*= false*/, bool bGlobal /*= false*/, float flRadius /*= 200.0f*/ )
{
	m_bLooping = bLooping;
	m_bGlobal = bGlobal;
	m_flRadius = flRadius;

	FMOD_RESULT result;

	GRP *grp = GetEngine()->GetGRP();

	//std::ifstream soundFile;
	//GRPEmbeddFileContainer *fileContainer = grp->SeekToFileInGRP( fileName, soundFile );

	MemoryFileReader fileReader;
	grp->GetFileInGRP( fileName, fileReader );

	if ( fileReader.IsOpen() )
	{
		memset( &m_SoundExInfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
		m_SoundExInfo.cbsize = sizeof( FMOD_CREATESOUNDEXINFO );
		m_SoundExInfo.length = ( unsigned int )fileReader.GetSize();

		if ( m_bLooping )
		{
			if ( !m_bGlobal )
			{
			}
			else
				result = GetSoundSystem()->GetSystem()->createSound( ( const char* )fileReader.GetMemoryBuffer(), FMOD_DEFAULT | FMOD_LOOP_NORMAL | FMOD_OPENMEMORY, &m_SoundExInfo, &m_pSound );
		}
		else
		{
			if ( !m_bGlobal )
			{
			}
			else
				result = GetSoundSystem()->GetSystem()->createSound( ( const char* )fileReader.GetMemoryBuffer(), FMOD_DEFAULT | FMOD_OPENMEMORY, &m_SoundExInfo, &m_pSound );
		}

		fileReader.CloseAndFreeMemory();

		GetSoundSystem()->CheckError( result );

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