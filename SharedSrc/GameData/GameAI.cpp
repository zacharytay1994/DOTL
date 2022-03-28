#include "GameAI.h"
#include "GameData.h"

#include <iostream>
#include <limits>

namespace DOTL
{
	void MinionAISeekTower::Pre ()
	{
	}

	void MinionAISeekTower::Update ( Statemachine* statemachine , float dt , GameData& data , NetworkEntityExtended* thisEntity )
	{
		MinionAI* minion_ai = reinterpret_cast< MinionAI* >( statemachine );

		float nearest_tower_max = std::numeric_limits<float>::max ();
		float nearest_aggro_max = std::numeric_limits<float>::max ();
		uint16_t nearest_tower_id { 0 };
		uint16_t nearest_aggro_id { 0 };
		for ( auto const& entity : data.entities_ )
		{
			bool diff_team = entity.entity_.team_1_ != thisEntity->entity_.team_1_;
			// look for nearest tower and walk towards it
			if ( entity.entity_.type_ == ET::TOWER && diff_team )
			{
				float v_x , v_y;
				float length = GetDistance ( entity.entity_.GetData ( ED::POS_X ) , entity.entity_.GetData ( ED::POS_Y ) ,
					thisEntity->entity_.GetData ( ED::POS_X ) , thisEntity->entity_.GetData ( ED::POS_Y ) , v_x , v_y );

				if ( length < nearest_tower_max )
				{
					nearest_tower_max = length;
					nearest_tower_id = entity.entity_.id_;
					thisEntity->entity_.SetVelocity ( ( v_x / length ) * minion_ai->speed_ , ( v_y / length ) * minion_ai->speed_ );
				}
			}
			// check if should switch aggro
			else if ( ( entity.entity_.type_ == ET::PLAYER || entity.entity_.type_ == ET::MINION ) && diff_team )
			{
				float v_x , v_y;
				float length = GetDistance ( entity.entity_.GetData ( ED::POS_X ) , entity.entity_.GetData ( ED::POS_Y ) ,
					thisEntity->entity_.GetData ( ED::POS_X ) , thisEntity->entity_.GetData ( ED::POS_Y ) , v_x , v_y );

				if ( length < minion_ai->aggro_range_ && length < nearest_aggro_max )
				{
					nearest_aggro_max = length;
					nearest_aggro_id = entity.entity_.id_;
				}
			}
		}

		// if no nearest target found and reached tower
		if ( nearest_tower_max < minion_ai->aggro_range_ && nearest_aggro_id == 0 )
		{
			nearest_aggro_id = nearest_tower_id;
		}

		// if nearest aggro found - switch to attack state to attack it
		if ( nearest_aggro_id != 0 )
		{
			minion_ai->aggro_id_ = nearest_aggro_id;
			thisEntity->entity_.SetVelocity ( 0 , 0 );
			statemachine->SwitchState ( data.ai_state_minion_attack_ );
		}
	}

	void MinionAISeekTower::Post ()
	{
	}

	void MinionAIAttack::Pre ()
	{
	}

	void MinionAIAttack::Update ( Statemachine* statemachine , float dt , GameData& data , NetworkEntityExtended* thisEntity )
	{
		MinionAI* minion_ai = reinterpret_cast< MinionAI* >( statemachine );

		// check if target still exists
		if ( minion_ai->aggro_id_ != 0 )
		{
			// check if still within aggro range
			float v_x , v_y;
			float length = GetDistance ( data.GetEntity ( minion_ai->aggro_id_ ).GetData ( ED::POS_X ) , data.GetEntity ( minion_ai->aggro_id_ ).GetData ( ED::POS_Y ) ,
				thisEntity->entity_.GetData ( ED::POS_X ) , thisEntity->entity_.GetData ( ED::POS_Y ) , v_x , v_y );

			if ( length < minion_ai->aggro_range_ )
			{
				if ( minion_ai->attack_rate_timer_ < minion_ai->attack_rate_ )
				{
					minion_ai->attack_rate_timer_ += dt;
				}
				else
				{
					minion_ai->attack_rate_timer_ = 0.0f;

					CreateBullet ( data , thisEntity->entity_.GetData ( ED::POS_X ) , thisEntity->entity_.GetData ( ED::POS_Y ) , thisEntity->entity_.team_1_ , minion_ai->aggro_id_ );
				}
			}
			else
			{
				statemachine->SwitchState ( data.ai_state_minion_seek_ );
			}
		}
		else
		{
			statemachine->SwitchState ( data.ai_state_minion_seek_ );
		}
	}

	void MinionAIAttack::Post ()
	{
	}

	void TowerAIDefault::Pre ()
	{
	}

	void TowerAIDefault::Update ( Statemachine* statemachine , float dt , GameData& data , NetworkEntityExtended* thisEntity )
	{
		TowerAI* tower_ai = reinterpret_cast< TowerAI* >( statemachine );

		// TOWER SPAWNING LOGIC
		if ( tower_ai->spawner_ )
		{
			if ( tower_ai->spawn_timer_ < tower_ai->spawn_interval_ )
			{
				tower_ai->spawn_timer_ += dt;
			}
			else
			{
				tower_ai->spawn_timer_ = 0.0f;

				// spawn some minions
				for ( auto i = 0; i < tower_ai->spawn_amount_; ++i )
				{
					data.CreateEntity ( ET::MINION , thisEntity->entity_.GetData ( ED::POS_X ) , thisEntity->entity_.GetData ( ED::POS_Y ) , thisEntity->entity_.team_1_ );
				}
			}
		}

		// TOWER ATTACKING LOGIC
		// attack at intervals
		if ( tower_ai->attack_rate_timer_ < tower_ai->attack_rate_ )
		{
			tower_ai->attack_rate_timer_ += dt;
		}
		else
		{
			tower_ai->attack_rate_timer_ = 0.0f;

			// attack nearest target within range
			uint16_t nearest_target_id_ { 0 };
			float nearest_target_max = std::numeric_limits<float>::max ();
			for ( auto const& entity : data.entities_ )
			{
				// shoots everything that fulfills this condition
				if ( !entity.entity_.active_ || entity.entity_.type_ == ET::TOWER || entity.entity_.type_ == ET::BULLET || entity.entity_.type_ == ET::COUNT )
				{
					continue;
				}

				float v_x , v_y;
				float length = GetDistance ( entity.entity_.GetData ( ED::POS_X ) , entity.entity_.GetData ( ED::POS_Y ) ,
					thisEntity->entity_.GetData ( ED::POS_X ) , thisEntity->entity_.GetData ( ED::POS_Y ) , v_x , v_y );

				if ( length < nearest_target_max && length < tower_ai->attack_radius_ && thisEntity->entity_.team_1_ != entity.entity_.team_1_ )
				{
					nearest_target_max = length;
					nearest_target_id_ = entity.entity_.id_;
				}
			}

			// check if any target found
			if ( nearest_target_id_ != 0 )
			{
				//NetworkEntity const& bullet = data.CreateEntity ( ET::BULLET , thisEntity->entity_.GetData ( ED::POS_X ) , thisEntity->entity_.GetData ( ED::POS_Y ) , thisEntity->entity_.team_1_ );
				//// set bullet target
				//data.GetEntityExtended ( bullet.id_ ).bullet_ai_.target_id_ = nearest_target_id_;

				CreateBullet ( data , thisEntity->entity_.GetData ( ED::POS_X ) , thisEntity->entity_.GetData ( ED::POS_Y ) , thisEntity->entity_.team_1_ , nearest_target_id_ );
			}
		}

	}

	void TowerAIDefault::Post ()
	{
	}

	void BulletAISeek::Pre ()
	{
	}

	void BulletAISeek::Update ( Statemachine* statemachine , float dt , GameData& data , NetworkEntityExtended* thisEntity )
	{
		BulletAI* bullet_ai = reinterpret_cast< BulletAI* >( statemachine );

		// seek towards target if id != 0, else destroy
		if ( bullet_ai->target_id_ > 0 )
		{
			NetworkEntity& target = data.GetEntity ( bullet_ai->target_id_ );

			float v_x = target.GetData ( ED::POS_X ) - thisEntity->interpolated_x;
			float v_y = target.GetData ( ED::POS_Y ) - thisEntity->interpolated_y;
			float length = sqrt ( v_x * v_x + v_y * v_y );
			if ( length > bullet_ai->speed_ * dt * 2.0f )
			{
				thisEntity->entity_.SetVelocity ( ( v_x / length ) * bullet_ai->speed_ , ( v_y / length ) * bullet_ai->speed_ );
			}
			else
			{
				// deduct health of entity
				if ( target.health_ > 0 )
				{
					target.health_ -= bullet_ai->damage_;
				}

				// destroy bullet
				data.RemoveEntity ( thisEntity->entity_.id_ );
			}
		}
		else
		{
			// destroy bullet
			data.RemoveEntity ( thisEntity->entity_.id_ );
		}
	}

	void BulletAISeek::Post ()
	{
	}

	float GetDistance ( float front_x , float front_y , float back_x , float back_y , float& vx , float& vy )
	{
		vx = front_x - back_x;
		vy = front_y - back_y;
		return sqrt ( vx * vx + vy * vy );
	}

	void CreateBullet ( GameData& data , float x , float y , bool team , uint16_t target )
	{
		NetworkEntity const& bullet = data.CreateEntity ( ET::BULLET , x , y , team );
		// set bullet target
		data.GetEntityExtended ( bullet.id_ ).bullet_ai_.target_id_ = target;
	}
}