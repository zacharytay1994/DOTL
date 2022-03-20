#include "ClientProcess.h"

#include <iomanip>

namespace DOTL
{
	SFMLProcess::SFMLProcess ( int windowWidth , int windowHeight , char const* name )
		:
		sfml_instance_ ( windowWidth , windowHeight , name )
	{
	}

	void SFMLProcess::Initialize ( SOCKET clientSocket )
	{
		network_connected_ = true;
		network_thread_ = std::thread ( &SFMLProcess::NetworkThreadUpdate , this , clientSocket );
		input_thread_ = std::thread ( &SFMLProcess::InputThreadUpdate , this , clientSocket );
	}

	void SFMLProcess::Update ( SOCKET clientSocket , bool& connected )
	{
		sfml_instance_.Update ();

		NetworkPacket packet ( NETWORK_COMMAND::QUIT );
		NetworkSend ( clientSocket , packet );

		// client has ended as window is closed
		network_connected_ = false;

		// join all threads
		network_thread_.join ();
		input_thread_.join ();

		connected = false;
	}

	void SFMLProcess::NetworkThreadUpdate ( SOCKET clientSocket )
	{
		bool connected { true };
		while ( connected )
		{
			ReceiveBuffer ( clientSocket );

			// process all packets
			while ( !packets_.empty () )
			{
				NetworkPacket& packet = packets_.front ();
				switch ( packet.type_ )
				{
				case ( PACKET_TYPE::MESSAGE ):
					std::cout << packet.buffer_ << std::endl;
					break;
				case ( PACKET_TYPE::COMMAND ):
					NETWORK_COMMAND i = static_cast< NETWORK_COMMAND >( *reinterpret_cast< int* >( packet.buffer_ ) );
					switch ( i )
					{
					case ( NETWORK_COMMAND::QUIT ):
						connected = false;
						break;
					}
					break;
				}

				packets_.pop ();
			}
		}
	}

	void SFMLProcess::InputThreadUpdate ( SOCKET clientSocket )
	{
		while ( network_connected_ )
		{
			std::getline ( std::cin , input_string_ );
			if ( !input_string_.empty () )
			{
				std::cout << std::setfill ( '_' ) << std::setw ( 50 ) << "." << std::setw ( 0 ) << std::endl;
				// process client side commands
				if ( input_string_[ 0 ] == '/' )
				{
					if ( input_string_.size () > 1 )
					{
						switch ( input_string_[ 1 ] )
						{
						case ( 'p' ):
							// get players command
							NetworkSend ( clientSocket , NetworkPacket ( NETWORK_COMMAND::PLAYERS ) );
							break;
						}
					}
				}
				// send message as string
				else
				{
					NetworkSend ( clientSocket , NetworkPacket ( input_string_.c_str () ) );
				}
			}
		}
	}
}