#include "engine.h"
#include "basehelpers.h"
#include "soundsystem.h"
#include "memoryfile.h"

extern SoundSystem *g_SoundSystem;

#include <fstream>

Engine::Engine()
{
	SetGame( nullptr );

	m_bRunning = false;
	m_pMainWindow = nullptr;
	m_bSDLInitialized = false;
	m_pMainGRP = nullptr;
}

Engine::~Engine()
{
	if ( m_pGame )
		delete m_pGame;

	if ( m_pMainGRP )
		delete m_pMainGRP;

	if ( g_SoundSystem )
		delete g_SoundSystem;

	if ( m_bSDLInitialized )
	{
		if ( m_GLContext != 0 )
			SDL_GL_DeleteContext( m_GLContext );

		if ( m_pMainWindow )
			SDL_DestroyWindow( m_pMainWindow );

		SDL_Quit();
	}
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
	if ( m_pMainWindow )
		SDL_SetWindowTitle( m_pMainWindow, title.c_str() );
}

bool Engine::Initialize()
{
	if ( !m_bSDLInitialized && SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	{
		std::string error = SDL_GetError();
		Msg( "[SDL]Failed to initialize: " + error + "\n" );

		return false;
	}
	else if ( m_bSDLInitialized )
	{
		Msg( "[SDL]Already initialized!\n" );
		return false; //May not want to do this.
	}

	m_bSDLInitialized = true;

	m_pMainWindow = SDL_CreateWindow( GetGame()->GetTitle().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

	if ( m_pMainWindow == nullptr )
	{
		std::string error = SDL_GetError();
		Msg( "[SDL]Failed to create window: " + error + "\n" );
		return false;
	}

	m_GLContext = SDL_GL_CreateContext( m_pMainWindow );

	if ( m_GLContext == 0 )
	{
		std::string error = SDL_GetError();
		Msg( "[SDL]Failed to create OpenGL context: " + error + "\n" );
		return false;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	SDL_GL_SetSwapInterval( 1 );

	glewExperimental = true;
	if ( glewInit() != GLEW_OK )
	{
		Msg( "[GL]Failed to initialize GLEW!\n" );
		return false;
	}

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
	SDL_GL_SwapWindow( m_pMainWindow );

	g_SoundSystem = new SoundSystem;

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
	//m_pMainGRP->SpewFileInformation();
	//m_pMainGRP->DumpGRP();

	//Bunch of test code
	Sound test( "ANNOY03.VOC", true, true );
	Msg( ( std::string )"Size of VOCMainHeader_t: " + std::to_string( sizeof( VOCMainHeader_t ) ) + "\n" );
	Msg( ( std::string )"Size of VOCDataBlock_SoundData_t: " + std::to_string( sizeof( VOCDataBlock_SoundData_t ) ) + "\n" );
	Msg( ( std::string )"Size of VOCDataBlock_SoundDataNewFormat_t: " + std::to_string( sizeof( VOCDataBlock_SoundDataNewFormat_t ) ) + "\n" );

	//DN3D.SpewFileInformation();
	//DN3D.DumpGRP();
	//DN3D.DumpGRPSeek();

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

		glClear( GL_COLOR_BUFFER_BIT );
		SDL_GL_SwapWindow( m_pMainWindow );
	}
}