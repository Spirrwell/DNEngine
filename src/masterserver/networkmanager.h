#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "enet/enet.h"

#include <thread>
#include <atomic>

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
	ENetHost *m_pMasterServer, *m_pRelayServer;

	std::thread m_masterThread, m_relayThread;
};

NetworkManager &GetNetworkManager();

#endif