#include "renderer.h"

Renderer::Renderer()
{
	m_pSwapFunc = &Renderer::SwapNULL_Internal;
	m_pClearFunc = &Renderer::ClearNULL_Internal;
}

void Renderer::Init( RenderingAPI_Type type )
{
	switch ( type )
	{
	case RenderingAPI_OpenGL:
		m_pSwapFunc = &Renderer::SwapGL_Internal;
		m_pClearFunc = &Renderer::ClearGL_Internal;
		break;
	case RenderingAPI_Vulkan:
		m_pSwapFunc = &Renderer::SwapVulkan_Internal;
		m_pClearFunc = &Renderer::ClearVulkan_Internal;
		break;
	default:
		break;
	}
}

void Renderer::Swap()
{
	m_pSwapFunc();
}

void Renderer::Clear()
{
	m_pClearFunc();
}

void Renderer::SwapGL_Internal()
{
	SDL_GL_SwapWindow( GetSDLManager()->GetWindow() );
}

void Renderer::ClearGL_Internal()
{
	glClear( GL_COLOR_BUFFER_BIT );
}

void Renderer::SwapVulkan_Internal()
{
	// This is kind of stupid
	GetSDLManager()->GetVulkanManager()->DrawFrame();
}

void Renderer::ClearVulkan_Internal()
{

}

static Renderer s_Renderer;
Renderer *GetRenderer() { return &s_Renderer; }