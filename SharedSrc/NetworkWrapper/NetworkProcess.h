#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <memory>
#include <queue>
#include <string>

#include "NetworkCommands.h"
#include "../GameData/GameData.h"

namespace DOTL
{
	struct NetworkProcess
	{
		NetworkProcess () = default;

		virtual void Initialize ( SOCKET socket ) = 0;
		virtual void Update ( SOCKET socket , bool& terminate ) = 0;

	protected:
		// buffer to store and parse received data
		static constexpr int	RECEIVE_BUFFER_SIZE = 50 * PACKET_SIZE;

		int							overflow_bytes { 0 };
		char						receive_buffer_[ RECEIVE_BUFFER_SIZE ] = {};
		char						overflow_buffer_[ PACKET_SIZE ] = {};
		std::queue<NetworkPacket>	packets_;
		std::string					input_string_ { "" };

		/*!
		 * @brief parses the receive_buffer_ data into individual packets stored in packets_
		*/
		void ReceiveBuffer ( SOCKET socket )
		{
			int bytes_received { 0 };
			if ( ( bytes_received = NetworkReceive ( socket , receive_buffer_ , RECEIVE_BUFFER_SIZE ) ) > 0 )
			{
				// concat old overflow bytes to start of newly received bytes
				if ( overflow_bytes > 0 )
				{
					memcpy ( receive_buffer_ + overflow_bytes , receive_buffer_ , bytes_received );
					memcpy ( receive_buffer_ , overflow_buffer_ , overflow_bytes );
				}

				int full_packets = ( overflow_bytes + bytes_received ) / PACKET_SIZE;

				for ( int i = 0; i < full_packets; ++i )
				{
					packets_.emplace ( *reinterpret_cast< NetworkPacket* >( receive_buffer_ + ( i * PACKET_SIZE ) ) );
				}

				// store overflow bytes in overflow buffer
				overflow_bytes = ( overflow_bytes + bytes_received ) - ( full_packets * PACKET_SIZE );
				if ( overflow_bytes > 0 )
				{
					memcpy ( overflow_buffer_ , receive_buffer_ + ( full_packets * PACKET_SIZE ) , overflow_bytes );
				}
			}
		}
	};

	using pNetworkProcess = std::shared_ptr<NetworkProcess>;
}