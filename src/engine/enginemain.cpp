#include "enginemain.h"
#include "engine_handler.h"
#include <iostream>
#include "basehelpers.h"

int engine_startup( int argc, char *argv[], IGame *game )
{
	EngineHandler::ERETURNCODE result =	g_DNEngineHandler.InitalizeEngine( game );

	if ( result == EngineHandler::ENGINE_INIT_FAIL )
	{
		Msg( "Engine failed to initialize!\n" );
		return -1;
	}

	Msg( "Hello Game, I'm the engine!\n" );

	return 0;
}

void engine_run()
{
	if ( GetEngine() == nullptr )
	{
		Msg( "Engine not initialized yet!\n" );
		return;
	}

	g_DNEngineHandler.RunEngine();
}

IEngine *GetEngine()
{
	return g_DNEngineHandler.GetEngine();
}