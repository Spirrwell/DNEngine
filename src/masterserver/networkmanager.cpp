#include "networkmanager.h"
#include "basehelpers.h"
#include "memoryfile.h"

#include <iostream>

static NetworkManager g_NetworkManager;
NetworkManager &GetNetworkManager() { return g_NetworkManager; }

enum PacketType
{
	PacketType_MasterServerHeader = 0,
	PacketType_Invalid
};

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

NetworkManager::NetworkManager()
{
	m_bSetup = false;
	m_bInit = true;

	if ( enet_initialize() != 0 )
	{
		Msg( "An error occurred while initializing ENet.\n\n" );
		m_bInit = false;
	}

	m_pMasterServer = nullptr;
	m_pRelayServer = nullptr;
}

NetworkManager::~NetworkManager()
{
	if ( m_pRelayServer != nullptr )
		enet_host_destroy( m_pRelayServer );

	if ( m_pMasterServer != nullptr )
		enet_host_destroy( m_pMasterServer );

	if ( m_bInit )
		enet_deinitialize();
}

bool NetworkManager::SetupHosts()
{
	if ( !m_bInit )
	{
		Msg( "ENet failed to initialize, cannot setup host!\n" );
		return m_bSetup = false;
	}

	m_MasterAddress.host = ENET_HOST_ANY;
	m_MasterAddress.port = 27015;

	m_RelayAddress.host = ENET_HOST_ANY;
	m_RelayAddress.port = 27016;

	m_pMasterServer = enet_host_create( &m_MasterAddress,
		ENET_PROTOCOL_MAXIMUM_PEER_ID,
		2, //2 channels... do we need more?
		0,
		0 );

	m_pRelayServer = enet_host_create( &m_RelayAddress,
		ENET_PROTOCOL_MAXIMUM_PEER_ID,
		2, //2 channels... do we need more?
		0,
		0 );

	if ( m_pMasterServer == nullptr || m_pRelayServer == nullptr )
	{
		Msg( "An error occurred while trying to create an ENet server host.\n" );
		return m_bSetup = false;
	}

	return m_bSetup = true;
}

bool NetworkManager::StartServers( std::atomic_bool &quit )
{
	if ( !m_bSetup )
	{
		Msg( "Network manager has not been setup.\n" );
		quit = true;
		return false;
	}

	m_masterThread = std::thread( &NetworkManager::RunMaster, std::ref( quit ) );
	m_masterThread.detach();

	m_relayThread = std::thread( &NetworkManager::RunRelay, std::ref( quit ) );
	m_relayThread.detach();

	return true;
}

void NetworkManager::RunMaster( std::atomic_bool &quit )
{
	using namespace std::literals;

	ENetEvent event;

	while ( !quit )
	{
		while ( enet_host_service( g_NetworkManager.m_pMasterServer, &event, 0 ) > 0 )
		{
			switch ( event.type )
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					char ipAddr[16];
					snprintf( ipAddr, sizeof( ipAddr ), "%u.%u.%u.%u", ( event.peer->address.host & 0x000000ff ),
						( event.peer->address.host & 0x0000ff00 ) >> 8,
						( event.peer->address.host & 0x00ff0000 ) >> 16,
						( event.peer->address.host & 0xff000000 ) >> 24 );
					MsgC( "A new client connected from %s:%u.\n", ipAddr, event.peer->address.port );
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					MemoryReader dataReader;
					dataReader.Open( ( char* )event.packet->data, event.packet->dataLength );
					char header[9];
					dataReader.Read( header, 8 );
					header[8] = '\0';

					if ( strcmp( header, "DNEngine" ) == 0 )
					{
						unsigned short packetType = PacketType::PacketType_Invalid;
						dataReader.Read( ( char* )&packetType, sizeof( unsigned short ) );
						if ( packetType == PacketType::PacketType_MasterServerHeader )
						{
							MasterServerHeader_t *mHeader = ( MasterServerHeader_t* )dataReader.GetMemoryBuffer();
							MsgC( "Header: %s\nServer Name: %s\nPlayers: %d/%d\n", mHeader->header, mHeader->serverName, mHeader->numPlayers, mHeader->maxPlayers );
						}
						else
							MsgC("PacketType: %d\n", packetType);
					}

					dataReader.Close();
					enet_packet_destroy( event.packet );
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					break;
				}
				case ENET_EVENT_TYPE_NONE:
				{
					break;
				}
			}
		}

		std::this_thread::sleep_for( 16ms );
	}
}

void NetworkManager::RunRelay( std::atomic_bool &quit )
{
	using namespace std::literals;

	ENetEvent event;
	ENetPeer *peer = nullptr;
	ENetAddress masterAddr;

	enet_address_set_host( &masterAddr, "127.0.0.1" );
	masterAddr.port = g_NetworkManager.m_MasterAddress.port;

	peer = enet_host_connect( g_NetworkManager.m_pRelayServer, &masterAddr, 2, 0 );

	if ( peer == nullptr )
	{
		quit = true;
		Msg( "No available peers for initiating an ENet connection.\n" );
	}

	while ( !quit )
	{
		while ( enet_host_service( g_NetworkManager.m_pRelayServer, &event, 0 ) > 0 )
		{
			switch ( event.type )
			{
			case ENET_EVENT_TYPE_CONNECT:
				if ( event.peer == peer )
				{
					Msg( "Connection to master server succeeded!\n" );
					MasterServerHeader_t *header = new MasterServerHeader_t;
					sprintf( header->header, "DNEngine" );
					header->packetType = PacketType::PacketType_MasterServerHeader;

					sprintf( header->serverName, "Test Server" );

					header->maxPlayers = 24;
					header->numPlayers = 4;
					
					header->version[0] = 0;
					header->version[1] = 0;
					header->version[2] = 1;

					ENetPacket *packet = enet_packet_create( header, sizeof( MasterServerHeader_t ), ENET_PACKET_FLAG_RELIABLE );
					enet_peer_send( peer, 0, packet );

					enet_host_flush( g_NetworkManager.m_pRelayServer );

					delete header;
				}
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				break;
			}
		}
		std::this_thread::sleep_for( 16ms );
	}
}