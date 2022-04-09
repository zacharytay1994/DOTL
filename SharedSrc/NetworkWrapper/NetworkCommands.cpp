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

	NetworkPacket::NetworkPacket ( NetworkEntity const& entity )
		:
		type_ ( PACKET_TYPE::CREATE )
	{
		memcpy ( buffer_ , &entity , sizeof ( entity ) );
	}

	NetworkPacket::NetworkPacket ( uint16_t i , char const* playerName )
		:
		type_ ( PACKET_TYPE::ASSIGN_PLAYER )
	{
		memcpy ( buffer_ , &i , sizeof ( uint16_t ) );
		std::string temp { playerName };
		memcpy ( buffer_ + sizeof ( uint16_t ) , playerName , temp.size () );
	}

	NetworkPacket::NetworkPacket ( bool team )
		:
		type_ ( PACKET_TYPE::SET_TEAM )
	{
		memcpy ( buffer_ , &team , sizeof ( bool ) );
	}

	NetworkPacket::NetworkPacket ( float x , float y )
		:
		type_ ( PACKET_TYPE::SET_POSITION )
	{
		memcpy ( buffer_ , &x , sizeof ( float ) );
		memcpy ( buffer_ + sizeof ( float ) , &y , sizeof ( float ) );
	}

	NetworkPacket::NetworkPacket ( float x , float y , uint16_t target , bool team )
		:
		type_ { PACKET_TYPE::CREATE_BULLET }
	{
		memcpy ( buffer_ , &x , sizeof ( float ) );
		memcpy ( buffer_ + sizeof ( float ) , &y , sizeof ( float ) );
		memcpy ( buffer_ + sizeof ( float ) + sizeof ( float ) , &target , sizeof ( uint16_t ) );
		memcpy ( buffer_ + sizeof ( float ) + sizeof ( float ) + sizeof ( uint16_t ) , &team , sizeof ( bool ) );
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

	/*void NetworkSendAll ( std::unordered_map<std::string , SOCKET> const& clients , NetworkPacket const& packet )
	{
		for ( auto const& client : clients )
		{
			NetworkSend ( client.second , packet );
		}
	}*/
}