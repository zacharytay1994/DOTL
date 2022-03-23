#pragma once

#include <vector>
#include <stack>
#include <unordered_map>

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
			10.0f,	// velocity x
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

	struct GameData
	{
		std::vector<NetworkEntity> entities_;

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
				entities_.back ().id_ = ++unique_id_;
				entities_.back ().owner_ = owner;
				return entities_.back ();
			}
			else
			{
				uint16_t id = free_ids_.top ();
				entities_[ id ] = entity;
				entities_[ id ].id_ = id;
				entities_[ id ].owner_ = owner;
				return entities_[ id ];
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
			entities_[ entity.id_ ] = entity;
			return entities_[ entity.id_ ];
		}

		void Update ( float dt )
		{
			// Game entities logic update

			for ( auto& entity : entities_ )
			{
				switch ( entity.type_ )
				{
				case ( ET::MINION ):
				{
					// update position with velocity
					entity.SetPosition ( entity.GetData ( ED::POS_X ) + entity.GetData ( ED::VEL_X ) * dt , entity.GetData ( ED::POS_Y ) + entity.GetData ( ED::VEL_Y ) );
					break;
				}
				case ( ET::TOWER ):
				{

					break;
				}
				case ( ET::BULLET ):
				{

					break;
				}
				}
			}
		}
	};
}