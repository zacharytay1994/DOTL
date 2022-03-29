#pragma once

#include "Statemachine.h"

namespace DOTL
{
	/*
		MINION STATEMACHINE
	*/
	struct MinionAI : public Statemachine
	{
		// minion data
		float speed_ { 100.0f };
		float separating_force_ { 50.0f };
		float separating_threshold_ { 100.0f };
		
		// non-tower aggro range
		float aggro_range_ { 150.0f };
		uint16_t aggro_id_ { 0 };

		float attack_rate_ { 1.0f };
		float attack_rate_timer_ { 0.0f };

		void Reset ();
	};

	/*
		MINION SEEKING STATE
		:
		In this state the minion constantly looks for a target and moves towards it
	*/
	struct MinionAISeekTower : public State
	{
		void Pre () override;

		void Update ( Statemachine* statemachine , float dt , GameData& data , NetworkEntityExtended* thisEntity ) override;

		void Post () override;
	};

	/*
		MINION ATTACK STATE
		:
		In this state the minion attacks the target
	*/
	struct MinionAIAttack : public State
	{
		void Pre () override;

		void Update ( Statemachine* statemachine , float dt , GameData& data , NetworkEntityExtended* thisEntity ) override;

		void Post () override;
	};

	/*
		TOWER STATEMACHINE
	*/
	struct TowerAI : public Statemachine
	{
		float spawn_interval_ { 20.0f };
		float spawn_timer_ { spawn_interval_ };
		uint16_t spawn_amount_ { 2 };

		float attack_radius_ { 300.0f };
		float attack_rate_ { 1.0f };
		float attack_rate_timer_ { 0.0f };

		bool spawner_ { true };

		void Reset ();
	};

	struct TowerAIDefault : public State
	{
		void Pre ( ) override;

		void Update ( Statemachine* statemachine , float dt , GameData& data , NetworkEntityExtended* thisEntity ) override;

		void Post () override;
	};

	/*
		BULLET STATEMACHINE
	*/
	struct BulletAI : public Statemachine
	{
		uint16_t target_id_ { 0 };
		float speed_ { 500.0f };
		uint16_t damage_ { 1 };

		void Reset ();
	};

	struct BulletAISeek : public State
	{
		void Pre ( ) override;

		void Update ( Statemachine* statemachine , float dt , GameData& data , NetworkEntityExtended* thisEntity ) override;

		void Post () override;
	};

	float GetDistance ( float front_x , float front_y , float back_x , float back_y , float& vx , float& vy );

	void CreateBullet ( GameData& data , float x , float y , bool team , uint16_t target );
}