#include "engine.h"
#include "basehelpers.h"
#include "soundsystem.h"
#include "memoryfile.h"
#include "map.h"
#include "sdlmgr.h"
#include "renderer.h"

#include <fstream>

extern SoundSystem *g_pSoundSystem;

Engine::Engine()
{
	SetGame( nullptr );

	m_bRunning = false;
	m_pMainGRP = nullptr;
}

Engine::~Engine()
{
	GetSDLManager()->Shutdown();

	if ( m_pGame )
		delete m_pGame;

	if ( m_pMainGRP )
		delete m_pMainGRP;

	if ( g_pSoundSystem )
		delete g_pSoundSystem;
}

std::string Engine::GetPlatform()
{
	std::string platform = "Unknown Platform";
	std::string configuration = "Unknown Configuration";

#ifdef _DEBUG
	configuration = "Debug";
#else
	configuration = "Release";
#endif
#ifdef _WIN64
	platform = "Windows x64";
#endif

#if defined ( _WIN32 ) && !defined( _WIN64 )
	platform = "Windows x86";
#endif

	return platform + " - " + configuration;
}

void Engine::UpdateTitle( const std::string &title )
{
	if ( GetSDLManager()->GetWindow() )
		SDL_SetWindowTitle( GetSDLManager()->GetWindow(), title.c_str() );
}

bool Engine::Initialize()
{
	if ( !GetSDLManager()->Init() )
		return false;

	g_pSoundSystem = new SoundSystem;

	if ( g_pSoundSystem == nullptr )
	{
		Msg( "Failed to create sound system. Out of memory.\n" );
		return false;
	}

	return true;
}

void Engine::RunEngine()
{
	if ( GetGame() == nullptr )
	{
		Msg( "Invalid game pointer passed to engine!\n" );
		return;
	}

	m_bRunning = true;

	m_pMainGRP = new GRP( "duke3d.grp" );
	Sound test( "GRABBAG.MID", true, true );
	
	Msg( ( std::string )"Size of VOCMainHeader_t: " + std::to_string( sizeof( VOCMainHeader_t ) ) + "\n" );
	Msg( ( std::string )"Size of VOCDataBlock_SoundData_t: " + std::to_string( sizeof( VOCDataBlock_SoundData_t ) ) + "\n" );
	Msg( ( std::string )"Size of VOCDataBlock_SoundDataNewFormat_t: " + std::to_string( sizeof( VOCDataBlock_SoundDataNewFormat_t ) ) + "\n" );

	Map e1l1( "E1L1.map" );

	//Run main loop
	MainLoop();
}

void Engine::MainLoop()
{
	SDL_Event event;

	while ( m_bRunning )
	{
		while ( SDL_PollEvent( &event ) )
		{
			if ( event.type == SDL_QUIT )
				m_bRunning = false;

			if ( event.type == SDL_KEYDOWN )
			{
				switch ( event.key.keysym.sym )
				{
				case SDLK_ESCAPE:
					m_bRunning = false;
					break;
				default:
					break;
				}
			}
		}

		GetGame()->Update();
		GetSoundSystem()->Update();

		GetRenderer()->Clear();
		GetRenderer()->Swap();
	}
}