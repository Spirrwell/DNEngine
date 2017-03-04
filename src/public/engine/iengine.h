#ifndef IENGINE_H
#define IENGINE_H

#include <string>

#include "game/igame.h"

#include "grp.h"

class IEngine
{
	friend class EngineHandler;
public:

	//Just some helpful spew
	virtual std::string GetPlatform() = 0;

	virtual GRP *GetGRP() = 0;

	//Set Game pointer in engine. Engine runs the game.
	virtual void SetGame( IGame *game ) = 0;

	//For setting the game title after initialization.
	virtual void UpdateTitle( const std::string &title ) = 0;

protected:
	//Initializes the Engine
	virtual bool Initialize() = 0;

	//Starts the engine loop which runs the game.
	virtual void RunEngine() = 0;
};

#endif