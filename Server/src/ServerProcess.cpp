#include "ServerProcess.h"
#include "ServerWrapper/ServerWrapper.h"

namespace DOTL
{
	ServerProcess::ServerProcess ( ServerInstance_WinSock2* serverInstance )
		:
		server_instance_ ( serverInstance )
	{
	}

	void ServerProcess::Initialize ( SOCKET clientSocket )
	{
		NetworkSend ( clientSocket , NetworkPacket ( "Welcome, enter your DOTL username!" ) );
	}

	void ServerProcess::Update ( SOCKET clientSocket , bool& connected )
	{
		ReceiveBuffer ( clientSocket );

		// process all packets
		while ( !packets_.empty () )
		{
			NetworkPacket& packet = packets_.front ();
			switch ( packet.type_ )
			{
			/* ______________________________________________________________________
				HANDLE MESSAGES BY CLIENT
				:
				Only allows sending of messages once username has been established.
			*/
			case ( PACKET_TYPE::MESSAGE ):
				// if username not yet established

				if ( username_.empty () )
				{
					// convert user name to uppercase

					std::string potential_username { packet.buffer_ };
					for ( auto& c : potential_username ) c = static_cast< char >( std::toupper ( static_cast< int >( c ) ) );
					auto const& clients = server_instance_->GetClients ();

					// if username is not taken - register the player

					if ( clients.find ( potential_username ) == clients.end () )
					{
						server_instance_->RegisterPlayer ( potential_username.c_str () , clientSocket );
						username_ = potential_username;

						// let all clients know a player has arrived

						SendNamedMessage ( "SERVER" , clients , "Welcome Amazing " , username_ , "!" );
						ServerLog ( 'I' , username_ , " registered to server." );
					}
					else
					{
						// send a request to resend another username

						SendNamedMessage ( "SERVER" , clientSocket , "Username in use, please choose another!" );
					}
				}
				// if username already established - forward messages to all clients
				else
				{
					SendNamedMessage ( username_.c_str () , server_instance_->GetClients () , packet.buffer_ );
				}
				break;

			/* ______________________________________________________________________
				HANDLE SERVER COMMANDS BY CLIENT
			*/
			case ( PACKET_TYPE::COMMAND ):
				NETWORK_COMMAND i = static_cast< NETWORK_COMMAND >( *reinterpret_cast< int* >( packet.buffer_ ) );
				switch ( i )
				{
				// 1. QUIT COMMAND

				case ( NETWORK_COMMAND::QUIT ):
					SendNamedMessage ( "SERVER" , server_instance_->GetClients () , username_ , " has left the server." );
					SendNamedMessage ( "SERVER" , clientSocket , "Connection terminated successfully. Press enter to continue." );
					NetworkSend ( clientSocket , NetworkPacket ( NETWORK_COMMAND::QUIT ) );
					server_instance_->ErasePlayer ( username_ );
					ServerLog ( 'I' , username_ , " left the server." );

					// setting connected to false terminates the thread handling this client

					connected = false;

					break;

				// 2. PLAYERS COMMAND

				case ( NETWORK_COMMAND::PLAYERS ):
					std::stringstream format;
					format << "PLAYERS IN LOBBY:\n";
					auto const& clients = server_instance_->GetClients ();
					int player_count { 0 };
					for ( auto const& client : clients )
					{
						format << ++player_count << ". " << client.first << "\n";
					}
					SendNamedMessage ( "SERVER" , clientSocket , format.str ().c_str () );
					break;
				}
				break;
			}

			packets_.pop ();
		}
	}
}