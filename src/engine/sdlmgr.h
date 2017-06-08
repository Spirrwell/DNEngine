#ifndef SDLMGR_H
#define SDLMGR_H

#include "GL/glew.h"

#include "SDL.h"
#include "SDL_opengl.h"

#include <string>
#include "vulkanmgr.h"

enum RenderingAPI_Type
{
	RenderingAPI_OpenGL = 0,
	RenderingAPI_Vulkan = 1
};

class SDLManager
{
	friend class Renderer;

public:
	SDLManager();

	bool Init();
	void Shutdown();

	inline int GetWindowWidth() { return m_iWidth; }
	inline int GetWindowHeight() { return m_iHeight; }

	inline const std::string &GetWindowTitle() { return m_szTitle; }

	inline SDL_Window *GetWindow() { return m_pMainWindow; }

protected:
	inline VulkanManager *GetVulkanManager() { return m_pVulkanManager; }

private:
	int m_iWidth;
	int m_iHeight;

	std::string m_szTitle;

	bool m_bSDLInitialized;

	SDL_Window *m_pMainWindow;
	SDL_GLContext m_pGLContext; // Only valid if we use OpenGL

	RenderingAPI_Type m_iRenderingAPI;

	VulkanManager *m_pVulkanManager;
};

extern SDLManager *GetSDLManager();

#endif