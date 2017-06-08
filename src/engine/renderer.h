#ifndef RENDERER_H
#define RENDERER_H

#include "sdlmgr.h"

class Renderer
{
public:
	Renderer();

	void Init( RenderingAPI_Type type );

	void Swap();
	void Clear();

private:
	static void SwapGL_Internal();
	static void SwapVulkan_Internal();
	static void SwapNULL_Internal() {};

	static void ClearGL_Internal();
	static void ClearVulkan_Internal();
	static void ClearNULL_Internal() {};

	void ( *m_pSwapFunc )();
	void ( *m_pClearFunc )();
};

extern Renderer *GetRenderer();

#endif