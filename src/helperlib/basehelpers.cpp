#include <iostream>

#include "basehelpers.h"
#include <stdarg.h>

//Will expand on these later when we have an in-game console
void Msg( const std::string &msg )
{
	std::cout << msg;
}

void MsgC( const char *msg, ... )
{
	va_list argptr;
	char str[1024];

	va_start( argptr, msg );
	vsnprintf( str, sizeof( str ), msg, argptr );
	va_end( argptr );

	Msg( str );
}