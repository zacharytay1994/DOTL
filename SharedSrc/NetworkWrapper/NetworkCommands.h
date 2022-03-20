#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <string>
#include <unordered_map>

#pragma comment(lib, "Ws2_32.lib")

namespace DOTL
{
	static constexpr int MAX_DATA_SIZE = 1024;

	enum class PACKET_TYPE
	{
		MESSAGE ,
		COMMAND ,
		COUNT
	};

	enum class NETWORK_COMMAND
	{
		QUIT = 0 ,
		PLAYERS ,
	};

	struct NetworkPacket
	{
		PACKET_TYPE type_ { PACKET_TYPE::COUNT };
		char buffer_[ MAX_DATA_SIZE ] = {};
		NetworkPacket () = default;
		NetworkPacket ( const char* message );
		NetworkPacket ( NETWORK_COMMAND command );
	};

	static constexpr int PACKET_SIZE = sizeof ( NetworkPacket );

	int NetworkReceive ( SOCKET socket , char* buffer , int size );

	void NetworkSend ( SOCKET socket , NetworkPacket const& packet );

	void NetworkSendAll ( std::unordered_map<std::string , SOCKET> const& clients , NetworkPacket const& packet );
}