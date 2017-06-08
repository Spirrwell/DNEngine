#include "soundsystem.h"
#include "fmod_errors.h"
#include "basehelpers.h"

SoundSystem::SoundSystem()
{
	m_pSystem = nullptr;

	Setup();
}

SoundSystem::~SoundSystem()
{
	m_pSystem->release();
}

void SoundSystem::CheckError( FMOD_RESULT result )
{
	if ( result != FMOD_OK )
		Msg( ( std::string )"[FMOD]Error: " + FMOD_ErrorString( result ) + "\n" );
}

void SoundSystem::Setup()
{
	FMOD_RESULT result;

	result = FMOD::System_Create( &m_pSystem );

	if ( result != FMOD_OK )
	{
		CheckError( result );
		return;
	}

	result = m_pSystem->getVersion( &m_iVersion );
	CheckError( result );

	result = m_pSystem->getNumDrivers( &m_iNumDrivers );
	CheckError( result );

	if ( m_iNumDrivers == 0 )
	{
		result = m_pSystem->setOutput( FMOD_OUTPUTTYPE_NOSOUND );
		CheckError( result );
	}

	result = m_pSystem->init( 256, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, NULL );
	CheckError( result );
}

SoundSystem *g_pSoundSystem = nullptr;

SoundSystem *GetSoundSystem()
{
	return g_pSoundSystem;
}