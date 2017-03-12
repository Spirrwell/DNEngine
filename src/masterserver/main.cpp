#include <iostream>
#include <thread>
#include <future>
#include <algorithm>
#include <conio.h>
#include <vector>

#include "enet/enet.h"
#include "basehelpers.h"
#include "networkmanager.h"

std::string GetCommand( const std::atomic_bool &cancel )
{
	//Msg( "]" );
	std::string str = "";

	while ( !cancel )
	{
		char input;

		while ( std::cin.peek() == EOF )
		{
			std::this_thread::yield();
		}

		input = std::cin.get();

		if ( input == '\n' )
			break;

		str += input;
	}

	return str;
}



class ConCommand
{
public:
	ConCommand( const std::string &command, const std::string &description )
	{
		m_strCommand = command;
		m_strDescription = description;
	}

	std::string GetCommand() { return m_strCommand; }
	std::string GetDescription() { return m_strDescription; }

private:
	std::string m_strCommand;
	std::string m_strDescription;
};

ConCommand g_cmdAvailableCommands[] =
{
	ConCommand( "exit", "Exits the program." ),
	ConCommand( "quit", "Exits the program." ),
};

void PrintCommands()
{
	for ( ConCommand cmd : g_cmdAvailableCommands )
		Msg( cmd.GetCommand() + " : " + cmd.GetDescription() + "\n" );
}

int main( int argc, char *argv[] )
{
	Msg( "DNEngine Master Server\n\n" );

	if ( !GetNetworkManager().SetupHosts() )
		return -1;

	using namespace std::literals;

	std::atomic_bool bQuit = false;

	std::future<std::string> future = std::async( std::launch::async, GetCommand, std::ref( bQuit ) );

	GetNetworkManager().StartServers( bQuit );

	while ( !bQuit )
	{
		if ( future.wait_for( 0s ) == std::future_status::ready )
		{
			std::string result = future.get(); //We store this value because we can't call std::future::get twice
			std::string cmp = result;
			std::transform( cmp.begin(), cmp.end(), cmp.begin(), ::tolower );
			if ( cmp == "quit" || cmp == "exit" )
				bQuit = true;
			else
			{
				Msg( result + " is not a valid command\n\nValid commands are:\n\n" );
				PrintCommands();
				future = std::async( std::launch::async, &GetCommand, std::ref( bQuit ) );
			}
		}
	}


	system( "PAUSE" );

	return 0;
}