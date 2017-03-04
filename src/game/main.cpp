#include "basehelpers.h"

#include "engine/enginemain.h"
#include "game.h"

int main( int argc, char *argv[] )
{
	//Starts up the engine, engine takes care of cleaning up Game
	if ( engine_startup( argc, argv, new Game( "Duke Nukem 3D" ) ) != 0 )
	{
		system( "PAUSE" );
		return -1;
	}

	Msg( "Platform is: " + GetEngine()->GetPlatform() + "\n" );

	//Tells engine to run game's main loop
	engine_run();

	return 0;
}