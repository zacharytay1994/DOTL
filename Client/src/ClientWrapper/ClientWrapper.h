#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#include "../../../SharedSrc/NetworkWrapper/NetworkProcess.h"

namespace DOTL
{
	struct ClientInstance_WinSock2
	{
		ClientInstance_WinSock2 ( char const* serverIPAddress , int serverPort , pNetworkProcess clientProcess );

		void Update ();

		bool SetupSuccess ();

	private:
		SOCKET			client_socket_;
		std::string		server_ip_address_;
		int				server_port_;
		pNetworkProcess	client_process_;

		bool setup_success_ { false };

		bool InitWinSock2_0 ();
	};
}