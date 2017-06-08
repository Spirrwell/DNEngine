#include "sdlmgr.h"
#include "basehelpers.h"
#include "enginemain.h"
#include "engine.h"
#include "renderer.h"

// Note SDL_GLContext is just a void* typedef, this COULD change in future SDL versions, but probably not
SDLManager::SDLManager() : m_pMainWindow( nullptr ), m_pGLContext( nullptr ), m_pVulkanManager( nullptr )
{
	m_bSDLInitialized = false;
}

bool SDLManager::Init()
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
		return false; // May not want to do this.
	}

	m_bSDLInitialized = true;

	Engine *pEngineInternal = dynamic_cast< Engine* >( GetEngine() );

	if ( pEngineInternal == nullptr )
	{
		Msg( "Failed to get internal engine!\n" );
		return false;
	}

	// Temporarily hard-code API usage
	m_iRenderingAPI = RenderingAPI_Vulkan;

	// Temporarily hard-code window size
	m_iWidth = 800;
	m_iHeight = 600;

	switch ( m_iRenderingAPI )
	{
	case RenderingAPI_OpenGL:
		m_pMainWindow = SDL_CreateWindow( pEngineInternal->GetGame()->GetTitle().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_iWidth, m_iHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		break;
	case RenderingAPI_Vulkan:
		m_pMainWindow = SDL_CreateWindow( pEngineInternal->GetGame()->GetTitle().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_iWidth, m_iHeight, SDL_WINDOW_SHOWN );
		break;
	default:
		Msg( "Tried to initialize unknown rendering API!\n" );
		return false;
		break; // Should never reach this
	}

	if ( m_pMainWindow == nullptr )
	{
		std::string error = SDL_GetError();
		Msg( "[SDL]Failed to create window: " + error + "\n" );
		return false;
	}

	// We assume the rendering API is valid if we passed the above switch statement
	if ( m_iRenderingAPI == RenderingAPI_OpenGL )
	{
		m_pGLContext = SDL_GL_CreateContext( m_pMainWindow );

		if ( m_pGLContext == nullptr )
		{
			std::string error = SDL_GetError();
			Msg( "[SDL]Failed to create OpenGL context: " + error + "\n" );
			return false;
		}

		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

		SDL_GL_SetSwapInterval( 0 );

		glewExperimental = true;

		if ( glewInit() != GLEW_OK )
		{
			Msg( "[GL]Failed to initialize GLEW!\n" );
			return false;
		}

		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );
		SDL_GL_SwapWindow( m_pMainWindow );
	}
	else if ( m_iRenderingAPI == RenderingAPI_Vulkan )
	{
		m_pVulkanManager = new VulkanManager;
		
		if ( m_pVulkanManager == nullptr )
		{
			Msg( "Failed to create Vulkan manager. Out of memory\n" );
			return false;
		}

		if ( !m_pVulkanManager->Init() )
			return false;
	}

	GetRenderer()->Init( m_iRenderingAPI );

	return true;
}

void SDLManager::Shutdown()
{
	if ( m_pVulkanManager )
		delete m_pVulkanManager;

	if( m_bSDLInitialized )
	{
		if ( m_pGLContext != nullptr )
			SDL_GL_DeleteContext( m_pGLContext );

		if ( m_pMainWindow != nullptr )
			SDL_DestroyWindow( m_pMainWindow );

		SDL_Quit();
	}
}

static SDLManager s_SDLManager;
SDLManager *GetSDLManager() { return &s_SDLManager; }