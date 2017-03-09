#ifndef ENGINE_H
#define ENGINE_H

#include "iengine.h"

#include "GL/glew.h"

#include "SDL.h"
#include "SDL_opengl.h"
#include "sound.h"

//Implementation of IEngine
class Engine : public IEngine
{
public:
	Engine();
	~Engine();

	std::string GetPlatform();

	IGame *GetGame() { return m_pGame; }

	GRP *GetGRP() { return m_pMainGRP; }

protected:
	void SetGame( IGame *game ) { m_pGame = game; }

	void UpdateTitle( const std::string &title );

	bool Initialize();
	void RunEngine();

private:
	void MainLoop();

	IGame *m_pGame;

	SDL_Window *m_pMainWindow;
	SDL_GLContext m_GLContext;

	//Will add support for multiple GRPs later
	GRP *m_pMainGRP;

	bool m_bSDLInitialized;
	bool m_bRunning;
};

#endif