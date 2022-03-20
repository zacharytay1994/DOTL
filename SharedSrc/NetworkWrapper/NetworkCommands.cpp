#include "NetworkCommands.h"
#include <winsock2.h>

namespace DOTL
{
	NetworkPacket::NetworkPacket ( const char* message )
		:
		type_ ( PACKET_TYPE::MESSAGE )
	{
		std::string s_message { message };
		memcpy ( buffer_ , s_message.c_str () , s_message.size () );
	}

	NetworkPacket::NetworkPacket ( NETWORK_COMMAND command )
		:
		type_ ( PACKET_TYPE::COMMAND )
	{
		int i_command = static_cast< int >( command );
		memcpy ( buffer_ , &i_command , sizeof ( int ) );
	}

	int NetworkReceive ( SOCKET socket , char* buffer , int size )
	{
		return recv ( socket , buffer , size , 0 );
	}

	void NetworkSend ( SOCKET socket , NetworkPacket const& packet )
	{
		int result = 0;
		int outgoing_size = PACKET_SIZE;
		char const* packet_data = reinterpret_cast< char const* >( &packet );
		while ( ( result = send ( socket , packet_data , PACKET_SIZE , 0 ) ) != outgoing_size )
		{
			if ( result == -1 )
			{
				std::cerr << "DOTL::NetworkSend (FILE: NetworkCommands.h) - Error sending data." << std::endl;
				break;
			}
			packet_data += result;
			outgoing_size -= result;
		}
	}

	void NetworkSendAll ( std::unordered_map<std::string , SOCKET> const& clients , NetworkPacket const& packet )
	{
		for ( auto const& client : clients )
		{
			NetworkSend ( client.second , packet );
		}
	}
}