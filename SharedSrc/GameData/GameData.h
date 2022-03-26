#pragma once

#include <vector>
#include <stack>
#include <unordered_map>
#include <iostream>

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

		NetworkEntityExtended () = default;
		NetworkEntityExtended ( NetworkEntity entity )
			:
			entity_ ( entity ) ,
			interpolated_x ( entity.GetData ( ED::POS_X ) ) ,
			interpolated_y ( entity.GetData ( ED::POS_Y ) )
		{}
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

		/*
		*	THIS FUNCTION CREATES A NEW ENTITY, I.E a new id and owner is assigned to it
		*	:
		*	Mainly called by the server to create new objects
		*/
		NetworkEntity const& CreateEntity ( NetworkEntity const& entity , uint16_t owner = 0 )
		{
			if ( free_ids_.empty () )
			{
				entities_.push_back ( entity );
				entities_.back ().entity_.id_ = ++unique_id_;
				entities_.back ().entity_.owner_ = owner;
				return entities_.back ().entity_;
			}
			else
			{
				uint16_t id = free_ids_.top ();
				entities_[ id ] = entity;
				entities_[ id ].entity_.id_ = id;
				entities_[ id ].entity_.owner_ = owner;
				return entities_[ id ].entity_;
			}
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
			entities_[ entity.id_ ].entity_ = entity;
			return entities_[ entity.id_ ].entity_;
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

		void Update ( float dt )
		{
			// Game entities logic update

			for ( auto& extended_entity : entities_ )
			{
				NetworkEntity& entity = extended_entity.entity_;
				// increment entity update sequence if its not the player,
				// clients update their own player sequence
				switch ( entity.type_ )
				{
				case ( ET::MINION ):
				{
					// update position with velocity
					entity.SetPosition ( entity.GetData ( ED::POS_X ) + entity.GetData ( ED::VEL_X ) * dt , entity.GetData ( ED::POS_Y ) + entity.GetData ( ED::VEL_Y ) );
					++entity.sequence_;
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
	};
}