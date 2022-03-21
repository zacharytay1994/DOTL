#include "ServerProcess.h"
#include "ServerWrapper/ServerWrapper.h"

#include <cstdlib>	// for rand
#include <time.h>	// for srand time

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

					id_ = server_instance_->RegisterPlayer ( potential_username.c_str () , clientSocket );
					username_ = potential_username;

					// let all clients know a player has arrived

					SendNamedMessage ( "SERVER" , clients , "Welcome Amazing " , username_ , "!" );
					ServerLog ( 'I' , username_ , " registered to server." );

					// create player object in the game

					NetworkEntity entity;
					srand ( time ( NULL ) );
					entity.SetPosition ( rand() % 1200 , rand() % 1200 );
					entity.type_ = ET::PLAYER;
					entity = server_instance_->game_data_.CreateEntity ( entity , id_ );

					// create this player on all clients
					SendNetworkPacketToAll ( NetworkPacket ( entity ) );
					server_instance_->GetClientInfo ( id_ ).spawned_ = true;

					// this is to assign the id and username to the client
					// the constructor constructs a PACKET_TYPE::ASSIGN_PLAYER packet

					NetworkPacket packet = NetworkPacket ( id_ , username_.c_str () );
					NetworkSend ( clientSocket , packet );

					server_instance_->game_data_.player_names_[ entity.id_ ] = username_;

					// resync player names database
					for ( auto const& player : server_instance_->game_data_.player_names_ )
					{
						NetworkPacket name_packet = NetworkPacket ( player.first , player.second.c_str() );
						name_packet.type_ = PACKET_TYPE::SYNC_PLAYERNAME;
						SendNetworkPacketToAll ( name_packet );
					}

					// bring this client up to date with the current server entities
					SyncGameDataToClient ( clientSocket );
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
			{
				NETWORK_COMMAND i = static_cast< NETWORK_COMMAND >( *reinterpret_cast< int* >( packet.buffer_ ) );
				switch ( i )
				{
				// 1. QUIT COMMAND

				case ( NETWORK_COMMAND::QUIT ):
					SendNamedMessage ( "SERVER" , server_instance_->GetClients () , username_ , " has left the server." );
					SendNamedMessage ( "SERVER" , clientSocket , "Connection terminated successfully. Press enter to continue." );
					NetworkSend ( clientSocket , NetworkPacket ( NETWORK_COMMAND::QUIT ) );
					server_instance_->ErasePlayer ( id_ );
					ServerLog ( 'I' , username_ , " left the server." );

					// setting connected to false terminates the thread handling this client

					connected = false;

					break;

				// 2. PLAYERS COMMAND

				case ( NETWORK_COMMAND::PLAYERS ):
				{
					std::stringstream format;
					format << "PLAYERS IN LOBBY:" << std::endl;
					format << "Client Database:" << std::endl;
					auto const& clients = server_instance_->GetClients ();
					int player_count { 0 };
					for ( auto const& client : clients )
					{
						format << ++player_count << ". " << client.second.username_ << std::endl;
					}
					format << "Entity Database:" << std::endl;;
					for ( auto const& player : server_instance_->game_data_.player_names_ )
					{
						format << "- " << player.second << ", EntityID: " << player.first << std::endl;
					}
					SendNamedMessage ( "SERVER" , clientSocket , format.str ().c_str () );
					break;
				}

				// 3. SPAWN COMMAND

				/*case ( NETWORK_COMMAND::SPAWN ):
				{
					if ( !server_instance_->GetClientInfo ( id_ ).spawned_ )
					{
						NetworkEntity entity;
						entity.SetPosition ( 100.0f , 100.0f );
						entity.type_ = ET::PLAYER;
						entity = server_instance_->game_data_.CreateEntity ( entity , id_ );
						SendNetworkPacketToAll ( NetworkPacket ( entity ) );
						server_instance_->GetClientInfo ( id_ ).spawned_ = true;
						NetworkSend ( clientSocket , NetworkPacket ( id_ , username_.c_str () ) );
					}
					else
					{
						SendNamedMessage ( "SERVER" , clientSocket , "Player entity already assigned to this user!" );
					}
					break;
				}*/
				}
				break;
			}

			/* ______________________________________________________________________
				HANDLE CREATE COMMANDS FROM CLIENT
			*/
			case ( PACKET_TYPE::CREATE ):
			{
				// create new entity - this gives the entity a unique id and owner

				NetworkEntity const& entity = server_instance_->game_data_.CreateEntity ( *reinterpret_cast< NetworkEntity* >( packet.buffer_ ) );

				// relay object creation command to all clients

				SendNetworkPacketToAll ( NetworkPacket ( entity ) );
				break;
			}
			}

			packets_.pop ();
		}
	}

	void ServerProcess::SendNetworkPacketToAll ( NetworkPacket const& packet )
	{
		for ( auto const& client : server_instance_->GetClients () )
		{
			NetworkSend ( client.second.socket_ , packet );
		}
	}

	void ServerProcess::SyncGameDataToClient ( SOCKET clientSocket )
	{
		NetworkPacket packet;
		packet.type_ = PACKET_TYPE::SYNC_ENTITY;
		int entity_size = static_cast< int >( sizeof ( NetworkEntity ) );

		// first 4 bytes are reserved for number of entities in the packet
		int entities_per_packed_buffer = MAX_DATA_SIZE - 4 / entity_size;
		int pack_iterations = static_cast< int >( server_instance_->game_data_.entities_.size () ) / entities_per_packed_buffer;
		int i { 0 };
		for ( int pack_iteration = 0; pack_iteration < pack_iterations; ++pack_iteration )
		{
			// 0 out buffer
			memset ( packet.buffer_ , 0 , MAX_DATA_SIZE );
			*reinterpret_cast< unsigned int* >( packet.buffer_ ) = entities_per_packed_buffer;
			for ( int per_pack = 0; per_pack < entities_per_packed_buffer; ++per_pack )
			{
				memcpy ( packet.buffer_ + ( per_pack * entity_size ) + 4 , &server_instance_->game_data_.entities_[ i++ ] , entity_size );
			}

			// send packed packet
			NetworkSend ( clientSocket , packet );
		}

		// send remaining entities in last packet
		memset ( packet.buffer_ , 0 , MAX_DATA_SIZE );
		int j;
		for ( j = 0; i < server_instance_->game_data_.entities_.size (); ++i , ++j )
		{
			memcpy ( packet.buffer_ + ( j * entity_size ) + 4 , &server_instance_->game_data_.entities_[ i ] , entity_size );
		}

		// first 4 bytes for number of entities
		*reinterpret_cast< unsigned int* >( packet.buffer_ ) = static_cast< unsigned int >( j );

		NetworkSend ( clientSocket , packet );
	}
}