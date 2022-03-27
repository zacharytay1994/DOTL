#include "ClientProcess.h"

#include <iomanip>
#include <sstream>
#include <chrono>

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

	void SFMLProcess::Update ( SOCKET clientSocket , bool& connected , double unusedDT )
	{
		UNREFERENCED_PARAMETER ( unusedDT );

		double dt { 0.0 };
		std::chrono::time_point<std::chrono::high_resolution_clock> time_stamp;
		double time { 0.0 };
		double sync_interval { 0.05 };

		while ( sfml_instance_.IsOpen () )
		{
			auto current_time = std::chrono::high_resolution_clock::now ();
			auto duration = std::chrono::duration_cast< std::chrono::nanoseconds >( current_time - time_stamp );
			dt = static_cast< double >( duration.count () ) / 1000'000'000.0;
			// stall to 60 fps
			while ( dt < 0.02 )
			{
				auto new_current = std::chrono::high_resolution_clock::now ();
				dt += static_cast< double >( std::chrono::duration_cast< std::chrono::nanoseconds >( new_current - current_time ).count () ) / 1000'000'000.0;
				current_time = new_current;
			}
			time_stamp = current_time;

			// poll events here
			sfml_instance_.PollMouseEvents ( mouse_data_ );

			// do player logic here
			// player update
			// check if player id is valid and exist
			if ( game_data_.player_names_.find ( player_id_ ) != game_data_.player_names_.end () &&
				player_id_ < game_data_.entities_.size () )
			{
				// set to vector code
				//game_data_.UpdateClient ( player_id_ , static_cast< float >( dt ) , mouse_data_ );
				UpdateClient ( dt );

				// sync - pass to server the player position
				NetworkEntity& player = game_data_.GetEntity ( player_id_ );

				if ( time > sync_interval )
				{
					time = 0.0;
					NetworkPacket packet;
					packet.type_ = PACKET_TYPE::SYNC_ENTITY;
					memcpy ( packet.buffer_ , &player , sizeof ( NetworkEntity ) );
					NetworkSend ( clientSocket , packet ); // error here
				}
				else
				{
					time += dt;
				}
			}

			// sfml render
			sfml_instance_.Update ( game_data_ , static_cast< float >( dt ) , player_id_ );
		}

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
				{
					NETWORK_COMMAND i = static_cast< NETWORK_COMMAND >( *reinterpret_cast< int* >( packet.buffer_ ) );
					switch ( i )
					{
					case ( NETWORK_COMMAND::QUIT ):
						connected = false;
						break;
					}
					break;
				}

				/* ______________________________________________________________________
					RECEIVE CREATE PACKET FROM SERVER - Create the entity on client side
				*/
				case ( PACKET_TYPE::CREATE ):
				{
					NetworkEntity* entity = reinterpret_cast< NetworkEntity* >( packet.buffer_ );
					game_data_.AddEntity ( *entity );
					break;
				}

				case ( PACKET_TYPE::ASSIGN_PLAYER ):
				{
					player_id_ = *reinterpret_cast< uint16_t* >( packet.buffer_ );
					player_username_ = packet.buffer_ + sizeof ( uint16_t );

					// set mouse click position to player position upon creation

					mouse_data_.x_ = game_data_.entities_[ player_id_ ].entity_.GetData ( ED::POS_X );
					mouse_data_.y_ = game_data_.entities_[ player_id_ ].entity_.GetData ( ED::POS_Y );
					break;
				}

				/* ______________________________________________________________________
					SYNC ENTITIES FROM SERVER WHEN RECEIVING THIS COMMAND
				*/
				case ( PACKET_TYPE::SYNC_ENTITY ):
				{
					SyncGameDataFromServer ( packet );
					break;
				}

				/* ______________________________________________________________________
					SYNC PLAYER NAMES FROM THE SERVER
				*/
				case ( PACKET_TYPE::SYNC_PLAYERNAME ):
				{
					game_data_.player_names_[ *reinterpret_cast< uint16_t* >( packet.buffer_ ) ] = packet.buffer_ + sizeof ( uint16_t );
					break;
				}

				case ( PACKET_TYPE::TIME_STAMP ):
				{
					uint64_t old_time_stamp = game_data_.time_stamp_;
					game_data_.time_stamp_ = *reinterpret_cast< uint64_t* >( packet.buffer_ );
					if ( game_data_.time_stamp_ > 0 )
					{
						game_data_.sync_delta_time_ = static_cast< double >( game_data_.time_stamp_ - old_time_stamp ) * 0.001;
					}
					break;
				}
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
					std::string check;
					std::stringstream command_string;
					command_string << input_string_;
					command_string >> check;
					// displays all players on the server
					if ( check == "/serverplayers" )
					{
						// get players command
						NetworkSend ( clientSocket , NetworkPacket ( NETWORK_COMMAND::PLAYERS ) );
					}
					// displays all players syncs on client
					if ( check == "/localplayers" )
					{
						std::cout << "PLAYERS IN LOBBY:" << std::endl;
						for ( auto const& player : game_data_.player_names_ )
						{
							std::cout << "- " << player.second << " , EntityID: " << player.first << std::endl;
						}
					}
					// create a new entity
					else if ( check == "/create" )
					{
						bool valid { false };
						NetworkEntity entity;
						// get type of enemy
						command_string >> check;
						if ( check == "minion" )
						{
							entity.type_ = ET::MINION;
							valid = true;
						}
						else if ( check == "tower" )
						{
							entity.type_ = ET::TOWER;
							valid = true;
						}
						else if ( check == "bullet" )
						{
							entity.type_ = ET::BULLET;
							valid = true;
						}

						if ( valid )
						{
							float x , y;
							command_string >> x >> y;
							entity.SetPosition ( x , y );
							NetworkSend ( clientSocket , NetworkPacket ( entity ) );
						}
					}
					// print snapshot of all entities
					else if ( check == "/localsnapshot" )
					{
						std::cout << "LOCAL ENTITY SNAPSHOT:" << std::endl;
						for ( auto i = 0; i < game_data_.entities_.size (); ++i )
						{
							NetworkEntity& entity = game_data_.GetEntity ( static_cast< uint16_t >( i ) );
							std::cout << "-" << std::setw ( 5 ) << i << "." << std::setw ( 0 );
							// if active entity
							if ( entity.id_ > 0 )
							{
								std::cout << "[" << entity.id_ << "]";
								switch ( entity.type_ )
								{
								case ( ET::MINION ):
									std::cout << " minion (";
									break;
								case ( ET::TOWER ):
									std::cout << " tower (";
									break;
								case ( ET::BULLET ):
									std::cout << " bullet (";
									break;
								case ( ET::PLAYER ):
									std::cout << " player (";
									break;
								}
								std::cout << entity.GetData ( ED::POS_X ) << " , " << entity.GetData ( ED::POS_Y ) << ")";
							}
							else
							{
								std::cout << " inactive.";
							}
							std::cout << std::endl;
						}
					}
					else if ( check == "/serversnapshot" )
					{

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

	void SFMLProcess::SyncGameDataFromServer ( NetworkPacket const& packet )
	{
		int entity_size = static_cast< int >( sizeof ( NetworkEntity ) );

		NetworkEntity const* entity;
		unsigned int number_of_entities = *reinterpret_cast< unsigned int const* >( packet.buffer_ );
		for ( unsigned int i = 0; i < number_of_entities; ++i )
		{
			entity = reinterpret_cast< NetworkEntity const* >( packet.buffer_ + ( i * entity_size ) + 4 );
			// do server reconciliation for player here
			if ( entity->id_ == player_id_ )
			{
				// server reconciliation logic - for now no validation to sequence so it just 
				// does not update the player data is the sequence number is outdated
				if ( entity->sequence_ < game_data_.GetEntity ( player_id_ ).sequence_ )
				{
					// expected response from server
					// do stuff with response, validation etc.

					// ...
				}
			}
			else
			{
				game_data_.AddEntity ( *entity );
			}
		}
	}

	void SFMLProcess::UpdateClient ( double dt )
	{
		// UPDATE PLAYER 

		NetworkEntity& player = game_data_.GetEntity ( player_id_ );

		// get direction vector between player and mouse
		sf::Vector2f dir ( mouse_data_.x_ - player.GetData ( ED::POS_X ) , mouse_data_.y_ - player.GetData ( ED::POS_Y ) );
		// normalize direction
		float length = sqrt ( dir.x * dir.x + dir.y * dir.y );
		dir /= length;
		// set velocity of player = speed * dir
		player.SetVelocity ( dir.x * game_data_.player_speed_ , dir.y * game_data_.player_speed_ );

		// move to mouse position
		// calculate distance away
		if ( length > game_data_.player_speed_ * dt * 0.5f )
		{
			player.SetPosition ( player.GetData ( ED::POS_X ) + player.GetData ( ED::VEL_X ) * static_cast< float >( dt ) , player.GetData ( ED::POS_Y ) + player.GetData ( ED::VEL_Y ) * static_cast< float >( dt ) );
		}

		// increment update sequence to be used in server reconciliation
		++player.sequence_;
	}
}