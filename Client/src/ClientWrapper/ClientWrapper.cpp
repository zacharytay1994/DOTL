#include "ClientWrapper.h"
#include <chrono>

namespace DOTL
{
	ClientInstance_WinSock2::ClientInstance_WinSock2 ( char const* serverIPAddress , int serverPort , pNetworkProcess clientProcess )
		:
		server_ip_address_ ( serverIPAddress ) ,
		server_port_ ( serverPort ) ,
		client_process_ ( clientProcess )
	{
		if ( !InitWinSock2_0 () )
		{
			std::cerr << "Unable to Initialize Windows Socket environment" << WSAGetLastError () << std::endl;
			return;
		}

		client_socket_ = socket (
			AF_INET ,        // The address family. AF_INET specifies TCP/IP
			SOCK_STREAM ,    // Protocol type. SOCK_STREM specified TCP
			0               // Protoco Name. Should be 0 for AF_INET address family
		);
		if ( client_socket_ == INVALID_SOCKET )
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

		// Connect to the server
		if ( connect ( client_socket_ , ( struct sockaddr* ) &serverAddr , sizeof ( serverAddr ) ) != 0 )
		{
			std::cerr << "Unable to connect to " << server_ip_address_ << " on port " << server_port_ << std::endl;
			closesocket ( client_socket_ );
			WSACleanup ();
			return;
		}
		else
		{
			std::cout << "Connected to server at " << server_ip_address_ << " on port " << server_port_ << "." << std::endl;
		}

		setup_success_ = true;

		client_process_->Initialize ( client_socket_ );
	}

	void ClientInstance_WinSock2::Update ()
	{
		bool connected { true };

		while ( connected )
		{
			client_process_->Update ( client_socket_ , connected , 1.0 );
		}
	}

	bool ClientInstance_WinSock2::SetupSuccess ()
	{
		return setup_success_;
	}

	bool ClientInstance_WinSock2::InitWinSock2_0 ()
	{
		WSADATA wsaData;
		WORD wVersion = MAKEWORD ( 2 , 0 );

		if ( !WSAStartup ( wVersion , &wsaData ) )
			return true;

		return false;
	}
}