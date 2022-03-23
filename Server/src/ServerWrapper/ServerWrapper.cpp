#include "ServerWrapper.h"
#include <thread>

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

		// this value may need to change depending on disconnects and reconnects
		client_infos_.reserve ( 10 );

		setup_success_ = true;

		// start game update thread
		game_update_thread_ = std::thread ( &ServerInstance_WinSock2::UpdateGameLogic , this );
	}

	ServerInstance_WinSock2::~ServerInstance_WinSock2 ()
	{
		closesocket ( server_socket_ );
		WSACleanup ();
		game_update_thread_.join ();
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

	void ServerInstance_WinSock2::SyncGameDataToClient ( SOCKET clientSocket )
	{
		NetworkPacket packet;
		packet.type_ = PACKET_TYPE::SYNC_ENTITY;
		int entity_size = static_cast< int >( sizeof ( NetworkEntity ) );

		// first 4 bytes are reserved for number of entities in the packet
		int entities_per_packed_buffer = MAX_DATA_SIZE - 4 / entity_size;
		int pack_iterations = static_cast< int >( game_data_.entities_.size () ) / entities_per_packed_buffer;
		int i { 0 };
		for ( int pack_iteration = 0; pack_iteration < pack_iterations; ++pack_iteration )
		{
			// 0 out buffer
			memset ( packet.buffer_ , 0 , MAX_DATA_SIZE );
			*reinterpret_cast< unsigned int* >( packet.buffer_ ) = entities_per_packed_buffer;
			for ( int per_pack = 0; per_pack < entities_per_packed_buffer; ++per_pack )
			{
				memcpy ( packet.buffer_ + ( per_pack * entity_size ) + 4 , &game_data_.entities_[ i++ ] , entity_size );
			}

			// send packed packet
			NetworkSend ( clientSocket , packet );
		}

		// send remaining entities in last packet
		memset ( packet.buffer_ , 0 , MAX_DATA_SIZE );
		int j;
		for ( j = 0; i < game_data_.entities_.size (); ++i , ++j )
		{
			memcpy ( packet.buffer_ + ( j * entity_size ) + 4 , &game_data_.entities_[ i ] , entity_size );
		}

		// first 4 bytes for number of entities
		*reinterpret_cast< unsigned int* >( packet.buffer_ ) = static_cast< unsigned int >( j );

		NetworkSend ( clientSocket , packet );
	}

	bool ServerInstance_WinSock2::InitWinSock2_0 ()
	{
		WSADATA wsaData;
		WORD wVersion = MAKEWORD ( 2 , 0 );

		if ( !WSAStartup ( wVersion , &wsaData ) )
			return true;

		return false;
	}

	/*BOOL WINAPI ClientThread ( LPVOID lpData )
	{
		ClientInfo* pClientInfo = ( ClientInfo* ) lpData;
		SOCKET client_socket = pClientInfo->client_socket_;
		double dt { 0.0 };
		std::chrono::time_point<std::chrono::high_resolution_clock> time_stamp;

		bool connected { true };
		while ( connected )
		{
			auto current_time = std::chrono::high_resolution_clock::now ();
			auto duration = std::chrono::duration_cast< std::chrono::microseconds >( current_time - time_stamp );
			dt = duration.count () / 1000000.0;
			time_stamp = current_time;
			pClientInfo->server_process_->Update ( client_socket , connected , dt );
		}
		std::cout << "## Thread terminated." << std::endl;
		return TRUE;
	}*/

	void ModernClientThread ( ClientInfo* clientInfo )
	{
		SOCKET client_socket = clientInfo->client_socket_;
		double dt { 0.0 };
		std::chrono::time_point<std::chrono::high_resolution_clock> time_stamp;

		bool connected { true };
		while ( connected )
		{
			auto current_time = std::chrono::high_resolution_clock::now ();
			auto duration = std::chrono::duration_cast< std::chrono::microseconds >( current_time - time_stamp );
			dt = duration.count () / 1000000.0;
			time_stamp = current_time;
			clientInfo->server_process_->Update ( client_socket , connected , dt );
		}
		std::cout << "## Thread terminated." << std::endl;
	}

	void ServerInstance_WinSock2::UpdateGameLogic ()
	{
		double dt { 0.0 };
		double time { 0.0 };
		double sync_interval { 0.05 };
		std::chrono::time_point<std::chrono::high_resolution_clock> game_logic_time_stamp;
		while ( true )
		{
			auto current_time = std::chrono::high_resolution_clock::now ();
			auto duration = std::chrono::duration_cast< std::chrono::nanoseconds >( current_time - game_logic_time_stamp );
			dt = static_cast< double >( duration.count () ) / 1000'000'000.0;
			// stall to 60 fps
			while ( dt < 0.02 )
			{
				auto new_current = std::chrono::high_resolution_clock::now ();
				dt += static_cast< double >( std::chrono::duration_cast< std::chrono::nanoseconds >( new_current - current_time ).count () ) / 1000'000'000.0;
				current_time = new_current;
			}
			game_logic_time_stamp = current_time;

			game_data_.Update ( dt );

			// sync game data at intervals

			if ( time > sync_interval )
			{
				time = 0.0;
				for ( auto const& client : clients_ )
				{
					SyncGameDataToClient ( client.second.socket_ );
				}
			}
			else
			{
				time += dt;
			}
		}
	}
}