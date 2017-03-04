#ifndef ENGINEMAIN_H
#define ENGINEMAIN_H

#include "iengine.h"
#include <string>

#ifdef _MSC_VER
__declspec( dllexport ) int engine_startup( int argc, char *argv[], IGame *game );
__declspec( dllexport ) void engine_run();
__declspec( dllexport ) IEngine *GetEngine();
#else
#error Need to implement DLL exports for other compilers.
#endif


#endif