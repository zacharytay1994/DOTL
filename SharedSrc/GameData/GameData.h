#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <iostream>

#include "GameAI.h"
#include <SFML/System/Vector2.hpp>

namespace DOTL
{
	struct NetworkEntity
	{
		enum class TYPE
		{
			MINION = 0 ,
			TOWER ,
			BULLET ,
			PLAYER ,
			COUNT
		};

		uint16_t	id_ { 0 };
		uint16_t	owner_ { 0 };
		TYPE		type_ { TYPE::COUNT };
		uint64_t	sequence_ { 0 };
		bool		active_ { false };

		// health
		uint8_t	health_ { 10 };
		uint8_t	max_health_ { 10 };

		// if not team 1 means team 2
		bool team_1_ { true };

		enum class DATA
		{
			POS_X = 0 ,
			POS_Y ,
			VEL_X ,
			VEL_Y ,
			COUNT
		};
		float data[ static_cast< int >( DATA::COUNT ) ] =
		{
			0.0f,	// position x 
			0.0f,	// position y
			0.0f,	// velocity x
			0.0f	// velocity y
		};

		float GetData ( DATA id ) const
		{
			return data[ static_cast< int >( id ) ];
		}

		void SetPosition ( float x , float y )
		{
			data[ 0 ] = x;
			data[ 1 ] = y;
		}

		void SetVelocity ( float x , float y )
		{
			data[ 2 ] = x;
			data[ 3 ] = y;
		}
	};

	using ED = NetworkEntity::DATA;	// ED = entity data
	using ET = NetworkEntity::TYPE; // ET = entity type

	struct NetworkEntityExtended
	{
		NetworkEntity entity_;
		float interpolated_x { 0 } , interpolated_y { 0 };
		float m_player_non_cp_x { 0 } , m_player_non_cp_y { 0 };
		float sort_y { 0.0f };

		/*
			These variables are used by the server to process entity AI
		*/
		// Minion
		MinionAI minion_ai_;
		// Tower 
		TowerAI tower_ai_;
		// Bullet
		BulletAI bullet_ai_;

		NetworkEntityExtended () = default;
		NetworkEntityExtended ( NetworkEntity entity )
			:
			entity_ ( entity ) ,
			interpolated_x ( entity.GetData ( ED::POS_X ) ) ,
			interpolated_y ( entity.GetData ( ED::POS_Y ) )
		{
		}
	};

	struct GameData
	{
		std::vector<NetworkEntityExtended> entities_;
		uint64_t time_stamp_ { 0 };
		double sync_delta_time_ { 1.0 };

		float player_speed_ { 500.0f };
		float minion_speed_ { 250.0f };

		// store players separately for maybe more refined syncing, key is entity id NOT client id

		std::unordered_map<uint16_t , std::string> player_names_;

		/*
		*	THESE VARIABLES ARE MAINLY ACCESSED BY THE SERVER
		*/
		uint16_t unique_id_ { 0 };
		std::queue<uint16_t> free_ids_;

		// ai states
		std::shared_ptr<MinionAISeekTower> ai_state_minion_seek_ = std::make_shared<MinionAISeekTower> ();
		std::shared_ptr<MinionAIAttack> ai_state_minion_attack_ = std::make_shared<MinionAIAttack> ();
		std::shared_ptr<TowerAIDefault> ai_state_tower_default_ = std::make_shared<TowerAIDefault> ();
		std::shared_ptr<BulletAISeek> ai_state_bullet_seek_ = std::make_shared<BulletAISeek> ();

		GameData ()
		{
			entities_.reserve ( 100 );
			entities_.emplace_back ();
		}

		/*
		*	THIS FUNCTION CREATES A NEW ENTITY, I.E a new id and owner is assigned to it
		*	:
		*	Mainly called by the server to create new objects
		*/
		NetworkEntity const& CreateEntity ( NetworkEntity const& entity , uint16_t owner = 0 )
		{
			uint16_t entity_id { 0 };
			if ( free_ids_.empty () )
			{
				entities_.push_back ( entity );
				++entities_.back ().entity_.sequence_;
				entities_.back ().entity_.id_ = ++unique_id_;
				entity_id = static_cast< uint16_t >( entities_.size () ) - 1;
			}
			else
			{
				entity_id = free_ids_.front ();
				free_ids_.pop ();
				uint64_t sequence = entities_[ entity_id ].entity_.sequence_;
				entities_[ entity_id ].entity_ = entity;
				entities_[ entity_id ].entity_.sequence_ = ++sequence;
				entities_[ entity_id ].entity_.id_ = entity_id;
			}

			NetworkEntityExtended& entity_extended = GetEntityExtended ( entity_id );
			entity_extended.entity_.owner_ = owner;
			entity_extended.entity_.active_ = true;
			//entity_extended.entity_.sequence_ = 0;

			// type specific changes
			switch ( entity.type_ )
			{
			case ( ET::MINION ):
			{
				entity_extended.minion_ai_.SwitchState ( ai_state_minion_seek_ );
				break;
			}
			case ( ET::TOWER ):
			{
				entity_extended.tower_ai_.SwitchState ( ai_state_tower_default_ );
				break;
			}
			case ( ET::BULLET ):
			{
				entity_extended.bullet_ai_.SwitchState ( ai_state_bullet_seek_ );
				break;
			}
			}

			entity_extended.interpolated_x = entity.GetData ( ED::POS_X );
			entity_extended.interpolated_y = entity.GetData ( ED::POS_Y );

			return entity_extended.entity_;
		}

		NetworkEntity const& CreateEntity ( ET type , float x , float y , bool team = false )
		{
			NetworkEntity entity;
			entity.type_ = type;
			entity.team_1_ = team;
			entity.SetPosition ( x , y );
			return CreateEntity ( entity );
		}

		/*
		*	THIS FUNCTION JUST ADDS AN ENTITY TO THE ENTITIES CONTAINER, i.e. it does not assign an id or owner
		*	:
		*	Mainly called by clients when the server sends a create command.
		*/
		NetworkEntity const& AddEntity ( NetworkEntity const& entity )
		{
			// if id received is more then current game data size, expand game data container and place in
			if ( entity.id_ >= entities_.size () )
			{
				entities_.resize ( entity.id_ + 1 );
			}
			// sequence is used by all entities beside players to keep track of entity version,
			// players use sequence for server reconciliation
			// set interpolated position
			if ( entities_[ entity.id_ ].entity_.sequence_ != entity.sequence_ && entity.type_ != ET::PLAYER )
			{
				entities_[ entity.id_ ].interpolated_x = entity.GetData ( ED::POS_X );
				entities_[ entity.id_ ].interpolated_y = entity.GetData ( ED::POS_Y );
			}
			entities_[ entity.id_ ].entity_ = entity;
			return entities_[ entity.id_ ].entity_;
		}

		void RemoveEntity ( uint16_t id )
		{
			if ( id < entities_.size () )
			{
				entities_[ id ].entity_.active_ = false;

				entities_[ id ].entity_.SetPosition ( 0.0f , 0.0f );
				entities_[ id ].interpolated_x = 0.0f;
				entities_[ id ].interpolated_y = 0.0f;
				entities_[ id ].entity_.SetVelocity ( 0.0f , 0.0f );

				entities_[ id ].tower_ai_.Reset ();
				entities_[ id ].minion_ai_.Reset ();
				entities_[ id ].bullet_ai_.Reset ();

				free_ids_.push ( id );
			}
		}

		NetworkEntity& GetEntity ( uint16_t id )
		{
			if ( id < entities_.size () )
			{
				return entities_[ id ].entity_;
			}
			std::cerr << "DOTL::GameData::GetEntity() [FILE: GameData.h] - Getting entity that does not exist." << std::endl;
			return entities_[ 0 ].entity_;
		}

		NetworkEntityExtended& GetEntityExtended ( uint16_t id )
		{
			if ( id < entities_.size () )
			{
				return entities_[ id ];
			}
			std::cerr << "DOTL::GameData::GetEntityExtended() [FILE: GameData.h] - Getting entity that does not exist." << std::endl;
			return entities_[ 0 ];
		}

		float my_lerp2 ( float x , float y , float val )
		{
			return static_cast< float >( x + ( ( y - x ) * val ) );
		}

		sf::Vector2f server_lerp ( sf::Vector2f start , sf::Vector2f end , float dt , float spd )
		{
			sf::Vector2f dir = end - start;
			float length = sqrt ( dir.x * dir.x + dir.y * dir.y );
			if ( length > spd * dt * 2.0f )
			{
				return start + ( dir / length ) * spd * dt;
			}
			return end;
		}

		void UpdateServer ( float dt )
		{
			// Game entities logic update
			float lerp_val = static_cast< float >( sync_delta_time_ > 1.0f ? 1.0f : sync_delta_time_ );
			//float lerp_val = static_cast< float >( sync_delta_time_ ) * dt;
			for ( auto& extended_entity : entities_ )
			{
				if ( extended_entity.entity_.active_ )
				{
					NetworkEntity& entity = extended_entity.entity_;

					// if entity to be updated is dead remove it
					if ( entity.type_ != ET::PLAYER )
					{
						if ( entity.health_ <= 0 )
						{
							RemoveEntity ( entity.id_ );
							continue;
						}
					}
					// on player death
					else
					{

					}

					// calculate interpolated positions
					//extended_entity.interpolated_x = my_lerp2 ( extended_entity.interpolated_x ,
					//	extended_entity.entity_.GetData ( ED::POS_X ) , /*lerp_val * dt * 50*/ dt );

					//extended_entity.interpolated_y = my_lerp2 ( extended_entity.interpolated_y , 
					//	extended_entity.entity_.GetData ( ED::POS_Y ) , /*lerp_val * dt * 50*/ dt );

					sf::Vector2f start { extended_entity.interpolated_x, extended_entity.interpolated_y };
					sf::Vector2f end ( extended_entity.entity_.GetData ( ED::POS_X ) , extended_entity.entity_.GetData ( ED::POS_Y ) );

					float speed { 0.0f };
					switch ( extended_entity.entity_.type_ )
					{
					case ( ET::MINION ):
						speed = extended_entity.minion_ai_.speed_;
						break;
					case ( ET::BULLET ):
						speed = extended_entity.bullet_ai_.speed_;
						break;
					case ( ET::PLAYER ):
						speed = player_speed_;
						break;
					}

					start = server_lerp ( start , end , dt , speed );
					extended_entity.interpolated_x = start.x;
					extended_entity.interpolated_y = start.y;

					// increment entity update sequence if its not the player,
					// clients update their own player sequence
					// update ai
					switch ( entity.type_ )
					{
					case ( ET::MINION ):
					{
						extended_entity.minion_ai_.Update ( dt , *this , &extended_entity );

						// move position with velocity
						entity.SetPosition ( entity.GetData ( ED::POS_X ) + entity.GetData ( ED::VEL_X ) * dt ,
							entity.GetData ( ED::POS_Y ) + entity.GetData ( ED::VEL_Y ) * dt );
						break;
					}
					case ( ET::TOWER ):
					{
						extended_entity.tower_ai_.Update ( dt , *this , &extended_entity );
						break;
					}
					case ( ET::BULLET ):
					{
						extended_entity.bullet_ai_.Update ( dt , *this , &extended_entity );

						// move position with velocity
						entity.SetPosition ( entity.GetData ( ED::POS_X ) + entity.GetData ( ED::VEL_X ) * dt ,
							entity.GetData ( ED::POS_Y ) + entity.GetData ( ED::VEL_Y ) * dt );
						break;
					}
					}
				}
			}
		}
	};
}