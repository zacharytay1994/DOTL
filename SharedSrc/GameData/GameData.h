#pragma once

#include <vector>
#include <stack>
#include <unordered_map>
#include <iostream>

#include "GameAI.h"

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

		/*
			These variables are used by the server to process entity AI
		*/
		// Minion
		MinionAI minion_ai_;

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
		std::stack<uint16_t> free_ids_;

		// ai states
		std::shared_ptr<MinionAISeek> ai_state_minion_seek_ = std::make_shared<MinionAISeek> ();

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
				entities_.back ().entity_.id_ = ++unique_id_;
				entity_id = entities_.size () - 1;
			}
			else
			{
				uint16_t id = free_ids_.top ();
				free_ids_.pop ();
				entities_[ id ] = entity;
				entities_[ id ].entity_.id_ = id;
			}

			NetworkEntityExtended& entity_extended = GetEntityExtended ( entity_id );
			entity_extended.entity_.owner_ = owner;
			entity_extended.entity_.active_ = true;
			entity_extended.entity_.sequence_ = 0;

			// type specific changes
			switch ( entity.type_ )
			{
			case ( ET::MINION ):
			{
				entity_extended.minion_ai_.SwitchState ( ai_state_minion_seek_ );
			}
			}

			return entity_extended.entity_;
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
			// set initial position if first time initialize
			if ( !entities_[ entity.id_ ].entity_.active_ )
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

		void UpdateServer ( float dt )
		{
			// Game entities logic update

			for ( auto& extended_entity : entities_ )
			{
				if ( extended_entity.entity_.active_ )
				{
					NetworkEntity& entity = extended_entity.entity_;
					// increment entity update sequence if its not the player,
					// clients update their own player sequence
					switch ( entity.type_ )
					{
					case ( ET::MINION ):
					{
						// update position with velocity
						/*entity.SetPosition ( entity.GetData ( ED::POS_X ) + entity.GetData ( ED::VEL_X ) * dt , entity.GetData ( ED::POS_Y ) + entity.GetData ( ED::VEL_Y ) );
						++entity.sequence_;*/
						extended_entity.minion_ai_.Update ( dt , entities_ , &entity );

						// move towards destination
						entity.SetPosition ( entity.GetData ( ED::POS_X ) + entity.GetData ( ED::VEL_X ) * dt ,
							entity.GetData ( ED::POS_Y ) + entity.GetData ( ED::VEL_Y ) * dt );
						break;
					}
					case ( ET::TOWER ):
					{

						++entity.sequence_;
						break;
					}
					case ( ET::BULLET ):
					{

						++entity.sequence_;
						break;
					}
					}
				}
			}
		}
	};
}