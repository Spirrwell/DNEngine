#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "enet/enet.h"

#include <thread>
#include <atomic>
#include <vector>

//Define these for now until we allow passing command line parameters
#define MASTERSERVER_PORT 27010;
#define MASTERSERVER_RELAY_PORT 27011;

static_assert( sizeof( unsigned short ) == 2, "Unsigned short needs to be 2 bytes" );
struct MasterServerHeader_t
{
	char header[8]; //Non-Null terminated "DNEngine", we could ditch this, but the master server shouldn't process much
	unsigned short packetType; //Tells master server we're a normal server header
	char serverName[256];
	unsigned char version[3];
	unsigned short maxPlayers;
	unsigned short numPlayers;
	char serverTags[128];
};

class ServerInfo
{
public:
	unsigned int GetServerIndex() { return m_uiIndex; }

private:
	unsigned int m_uiIndex;
};

class NetworkManager
{
public:
	NetworkManager();
	~NetworkManager();

	//May expand this later
	bool SetupHosts();

	bool StartServers( std::atomic_bool &quit );

private:
	static void RunMaster( std::atomic_bool &quit );
	static void RunRelay( std::atomic_bool &quit );

	bool m_bInit, m_bSetup;

	ENetAddress m_MasterAddress, m_RelayAddress;

	//The "Master Server" is composed of two hosts. The max amount of peers a host can have is 4095
	//These hosts are connected. The Master Server host will receive server info, and the Relay Server host will
	//handle client requests for that information. So there can be 4094 clients and 4094 servers. It's
	//a tad overkill, but it works.
	ENetHost *m_pMasterServer, *m_pRelayServer;

	std::thread m_masterThread, m_relayThread;

	//Only to be used by Master Server thread, NOT the Relay thread
	std::vector<MasterServerHeader_t*> m_vecMasterServerHeaders;
};

NetworkManager &GetNetworkManager();

#endif