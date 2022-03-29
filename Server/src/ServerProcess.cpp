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

	void ServerProcess::Update ( SOCKET clientSocket , bool& connected , double dt )
	{
		UNREFERENCED_PARAMETER ( dt );

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
					srand ( static_cast< unsigned int >( time ( NULL ) ) );
					entity.SetPosition ( static_cast< float >( rand () % 1200 ) , static_cast< float >( rand () % 1200 ) );
					entity.SetVelocity ( 0.0f , 0.0f );
					entity.type_ = ET::PLAYER;
					entity = server_instance_->game_data_.CreateEntity ( entity , id_ );
					player_id_ = entity.id_;

					// create this player on all clients
					SendNetworkPacketToAll ( NetworkPacket ( entity ) );
					server_instance_->GetClientInfo ( id_ ).spawned_ = true;

					// this is to assign the id and username to the client
					// the constructor constructs a PACKET_TYPE::ASSIGN_PLAYER packet

					NetworkPacket username_packet = NetworkPacket ( entity.id_ , username_.c_str () );
					NetworkSend ( clientSocket , username_packet );

					server_instance_->game_data_.player_names_[ entity.id_ ] = username_;

					// resync player names database
					for ( auto const& player : server_instance_->game_data_.player_names_ )
					{
						NetworkPacket name_packet = NetworkPacket ( player.first , player.second.c_str () );
						name_packet.type_ = PACKET_TYPE::SYNC_PLAYERNAME;
						SendNetworkPacketToAll ( name_packet );
					}

					// bring this client up to date with the current server entities
					server_instance_->SyncGameDataToClient ( clientSocket );
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

					server_instance_->game_data_.RemoveEntity ( player_id_ );

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

				case ( NETWORK_COMMAND::START ):
				{
					// create 2 tower on each team diagonally across
					// create team 1 towers - make first tower a spawner
					uint16_t t1_first_tower = server_instance_->game_data_.CreateEntity ( ET::TOWER , 100 , 100 , false ).id_;
					server_instance_->game_data_.GetEntityExtended ( t1_first_tower ).tower_ai_.spawner_ = true;
					server_instance_->game_data_.CreateEntity ( ET::TOWER , 300 , 300 , false );

					// create team 2 turrets
					uint16_t t2_first_tower = server_instance_->game_data_.CreateEntity ( ET::TOWER , 1100 , 1100 , true ).id_;
					server_instance_->game_data_.GetEntityExtended ( t2_first_tower ).tower_ai_.spawner_ = true;
					server_instance_->game_data_.CreateEntity ( ET::TOWER , 900 , 900 , true );

					float red_x { 150 } , red_y { 150 } , blue_x { 1050 } , blue_y { 1050 };
					// set player teams
					bool team { true };
					for ( auto const& client : server_instance_->GetClients () )
					{
						if ( team )
						{
							NetworkSend ( client.second.socket_ , NetworkPacket ( blue_x , blue_y ) );
							NetworkSend ( client.second.socket_ , NetworkPacket ( team ) );
						}
						else
						{
							NetworkSend ( client.second.socket_ , NetworkPacket ( red_x , red_y ) );
							NetworkSend ( client.second.socket_ , NetworkPacket ( team ) );
						}
						team = !team;
					}
					break;
				}

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

				//SendNetworkPacketToAll ( NetworkPacket ( entity ) );
				break;
			}

			/* ______________________________________________________________________
				HANDLE CREATE COMMANDS TO SYNC ENTITY
				:
				USUALLY ONLY WHEN ITS THEIR PLAYER
			*/
			case ( PACKET_TYPE::SYNC_ENTITY ):
			{
				NetworkEntity* player = reinterpret_cast< NetworkEntity* >( packet.buffer_ );
				// keep server side health - hard code for now until more stuff needs to be synced
				auto player_health = server_instance_->game_data_.GetEntity ( player->id_ ).health_;
				// should do some validation but maybe later
				server_instance_->game_data_.AddEntity ( *player );
				server_instance_->game_data_.GetEntity ( player->id_ ).health_ = player_health;
				break;
			}

			case ( PACKET_TYPE::CREATE_BULLET ):
			{
				uint16_t id = server_instance_->game_data_.CreateEntity (
					ET::BULLET ,
					*reinterpret_cast< float* >( packet.buffer_ ) ,
					*reinterpret_cast< float* >( packet.buffer_ + sizeof ( float ) ) ,
					*reinterpret_cast< bool* >( packet.buffer_ + sizeof ( float ) + sizeof ( float ) + sizeof ( uint16_t ) )
				).id_;

				server_instance_->game_data_.GetEntityExtended ( id ).bullet_ai_.target_id_ = *reinterpret_cast< uint16_t* >( packet.buffer_ + sizeof ( float ) + sizeof ( float ) );
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
}