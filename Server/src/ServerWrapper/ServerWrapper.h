#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <thread>

#include "../ServerProcess.h"
#include "../../../SharedSrc/GameData/GameData.h"

#pragma comment(lib, "Ws2_32.lib")


namespace DOTL
{
	struct ClientInfo
	{
		SOCKET				client_socket_;
		struct sockaddr_in	client_address_;
		pNetworkProcess		server_process_;
	};

	//BOOL WINAPI ClientThread ( LPVOID lpData );

	void ModernClientThread ( ClientInfo* clientInfo );

	struct ServerInstance_WinSock2
	{
		ServerInstance_WinSock2 ( char const* serverIPAddress , int serverPort , int maxClients );
		~ServerInstance_WinSock2 ();

		template <typename T>
		void Update ()
		{
			std::cout << "## Listening from " << server_ip_address_ << " on port " << server_port_ << " ..." << std::endl;
			// Start the infinite loop
			while ( true )
			{
				// As the socket is in listen mode there is a connection request pending.
				// Calling accept( ) will succeed and return the socket for the request.
				SOCKET hClientSocket;
				struct sockaddr_in clientAddr;
				int nSize = sizeof ( clientAddr );

				hClientSocket = accept ( server_socket_ , ( struct sockaddr* ) &clientAddr , &nSize );
				if ( hClientSocket == INVALID_SOCKET )
				{
					std::cout << "accept( ) failed" << std::endl;
				}
				else if ( connected_clients_ < max_clients_ )
				{
					/*HANDLE			hClientThread;
					DWORD			dwThreadId;*/

					client_infos_.emplace_back ();
					client_infos_.back ().client_address_ = clientAddr;
					client_infos_.back ().client_socket_ = hClientSocket;
					client_infos_.back ().server_process_ = std::make_shared<T> ( this );

					std::cout << "## Client connected from " << inet_ntoa ( clientAddr.sin_addr ) << std::endl;

					// Start the client thread
					/*hClientThread = CreateThread ( NULL , 0 ,
						( LPTHREAD_START_ROUTINE ) ClientThread ,
						( LPVOID ) &client_infos_.back () , 0 , &dwThreadId );
					if ( hClientThread == NULL )
					{
						std::cout << "## Unable to create client thread" << std::endl;
					}
					else
					{
						CloseHandle ( hClientThread );
					}*/
					client_threads_.emplace_back ( std::thread ( ModernClientThread , &client_infos_.back () ) );

					client_infos_.back ().server_process_->Initialize ( hClientSocket );
					++connected_clients_;
				}
				else
				{
					std::string full_server_message { "## The server is full." };
					NetworkPacket packet;
					packet.type_ = PACKET_TYPE::MESSAGE;
					memcpy ( packet.buffer_ , full_server_message.c_str () , full_server_message.size () );
					NetworkSend ( hClientSocket , packet );
				}
			}

			// join all client threads back
			for ( auto& thread : client_threads_ )
			{
				thread.join ();
			}
		}

		bool SetupSuccess ();

		CLIENT_MAP const& GetClients () const;
		uint16_t RegisterPlayer ( char const* name , SOCKET socket );
		void ErasePlayer ( uint16_t id );
		ClientMapInfo& GetClientInfo ( uint16_t id );

		GameData		game_data_;
		uint16_t		player_ids_ { 0 };

		void SyncGameDataToClient ( SOCKET clientSocket );

	private:
		SOCKET			server_socket_;
		std::string		server_ip_address_;
		int				server_port_;
		pNetworkProcess	server_process_;

		std::vector<ClientInfo> client_infos_;

		bool			setup_success_ { false };
		int const		max_clients_ { 0 };
		int				connected_clients_ { 0 };

		uint16_t		client_ids_ { 0 };	// note: client ids start at 1, 0 is reserved for the server
		CLIENT_MAP		clients_;

		std::vector<std::thread> client_threads_;

		bool InitWinSock2_0 ();

		std::thread game_update_thread_;

		void UpdateGameLogic ();
	};
}