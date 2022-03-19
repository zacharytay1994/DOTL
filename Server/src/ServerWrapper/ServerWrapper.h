#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

#include "../ServerProcess.h"

namespace DOTL
{
	struct ClientInfo
	{
		SOCKET				client_socket_;
		struct sockaddr_in	client_address_;
		pNetworkProcess		server_process_;
	};

	struct ServerInstance_WinSock2
	{
		ServerInstance_WinSock2 ( char const* serverIPAddress , int serverPort , pNetworkProcess serverProcess );
		~ServerInstance_WinSock2 ();

		void Update ();

		bool SetupSuccess ();

	private:
		SOCKET			server_socket_;
		std::string		server_ip_address_;
		int				server_port_;
		pNetworkProcess	server_process_;

		bool			setup_success_ { false };

		bool InitWinSock2_0 ();
	};

	BOOL WINAPI ClientThread ( LPVOID lpData );
}