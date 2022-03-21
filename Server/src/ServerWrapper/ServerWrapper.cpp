#include "ServerWrapper.h"

namespace DOTL
{
	ServerInstance_WinSock2::ServerInstance_WinSock2 ( char const* serverIPAddress , int serverPort , int maxClients )
		:
		server_ip_address_ ( serverIPAddress ) ,
		server_port_ ( serverPort ) ,
		max_clients_ ( maxClients )
	{
		if ( !InitWinSock2_0 () )
		{
			std::cerr << "Unable to Initialize Windows Socket environment" << WSAGetLastError () << std::endl;
			return;
		}

		server_socket_ = socket (
			AF_INET ,        // The address family. AF_INET specifies TCP/IP
			SOCK_STREAM ,    // Protocol type. SOCK_STREM specified TCP
			0               // Protoco Name. Should be 0 for AF_INET address family
		);
		if ( server_socket_ == INVALID_SOCKET )
		{
			std::cerr << "Unable to create Server socket" << std::endl;
			// Cleanup the environment initialized by WSAStartup()
			WSACleanup ();
			return;
		}

		// Create the structure describing various Server parameters
		struct sockaddr_in serverAddr;

		serverAddr.sin_family = AF_INET;     // The address family. MUST be AF_INET
		serverAddr.sin_addr.s_addr = inet_addr ( server_ip_address_.c_str () );
		serverAddr.sin_port = htons ( static_cast< u_short >( server_port_ ) );

		// Bind the Server socket to the address & port
		if ( bind ( server_socket_ , ( struct sockaddr* ) &serverAddr , sizeof ( serverAddr ) ) == SOCKET_ERROR )
		{
			std::cerr << "Unable to bind to " << server_ip_address_ << " port " << server_port_ << std::endl;
			// Free the socket and cleanup the environment initialized by WSAStartup()
			closesocket ( server_socket_ );
			WSACleanup ();
			return;
		}

		// Put the Server socket in listen state so that it can wait for client connections
		if ( listen ( server_socket_ , SOMAXCONN ) == SOCKET_ERROR )
		{
			std::cerr << "Unable to put server in listen state" << std::endl;
			// Free the socket and cleanup the environment initialized by WSAStartup()
			closesocket ( server_socket_ );
			WSACleanup ();
			return;
		}

		client_infos_.reserve ( max_clients_ );

		setup_success_ = true;
	}

	ServerInstance_WinSock2::~ServerInstance_WinSock2 ()
	{
		closesocket ( server_socket_ );
		WSACleanup ();
	}

	bool ServerInstance_WinSock2::SetupSuccess ()
	{
		return setup_success_;
	}

	std::unordered_map<uint16_t , ClientMapInfo> const& ServerInstance_WinSock2::GetClients () const
	{
		return clients_;
	}

	uint16_t ServerInstance_WinSock2::RegisterPlayer ( char const* name , SOCKET socket )
	{
		clients_[ ++client_ids_ ] = { socket , name };
		return client_ids_;
	}

	void ServerInstance_WinSock2::ErasePlayer ( uint16_t id )
	{
		if ( clients_.find ( id ) != clients_.end () )
		{
			clients_.erase ( id );
			--connected_clients_;
		}
	}

	ClientMapInfo& ServerInstance_WinSock2::GetClientInfo ( uint16_t id )
	{
		return clients_.at ( id );
	}

	bool ServerInstance_WinSock2::InitWinSock2_0 ()
	{
		WSADATA wsaData;
		WORD wVersion = MAKEWORD ( 2 , 0 );

		if ( !WSAStartup ( wVersion , &wsaData ) )
			return true;

		return false;
	}

	BOOL WINAPI ClientThread ( LPVOID lpData )
	{
		ClientInfo* pClientInfo = ( ClientInfo* ) lpData;
		SOCKET client_socket = pClientInfo->client_socket_;

		bool connected { true };
		while ( connected )
		{
			pClientInfo->server_process_->Update ( client_socket , connected );
		}
		std::cout << "## Thread terminated." << std::endl;
		return TRUE;
	}
}