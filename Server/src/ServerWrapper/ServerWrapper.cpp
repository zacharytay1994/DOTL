#include "ServerWrapper.h"

namespace DOTL
{
	ServerInstance_WinSock2::ServerInstance_WinSock2 ( char const* serverIPAddress , int serverPort , pNetworkProcess serverProcess )
		:
		server_ip_address_ ( serverIPAddress ) ,
		server_port_ ( serverPort ) ,
		server_process_ ( serverProcess )
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

		setup_success_ = true;

		serverProcess->Initialize ( server_socket_ );
	}

	ServerInstance_WinSock2::~ServerInstance_WinSock2 ()
	{
		closesocket ( server_socket_ );
		WSACleanup ();
	}

	void ServerInstance_WinSock2::Update ()
	{
		std::cout << "Listening on port " << server_port_ << " ..." << std::endl;
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
			else
			{
				HANDLE hClientThread;
				struct ClientInfo clientInfo;
				DWORD dwThreadId;

				clientInfo.client_address_ = clientAddr;
				clientInfo.client_socket_ = hClientSocket;
				clientInfo.server_process_ = server_process_;

				std::cout << "Client connected from " << inet_ntoa ( clientAddr.sin_addr ) << std::endl;

				// Start the client thread
				hClientThread = CreateThread ( NULL , 0 ,
					( LPTHREAD_START_ROUTINE ) ClientThread ,
					( LPVOID ) &clientInfo , 0 , &dwThreadId );
				if ( hClientThread == NULL )
				{
					std::cout << "Unable to create client thread" << std::endl;
				}
				else
				{
					CloseHandle ( hClientThread );
				}
			}
		}
	}

	bool ServerInstance_WinSock2::SetupSuccess ()
	{
		return setup_success_;
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

		return TRUE;
	}
}