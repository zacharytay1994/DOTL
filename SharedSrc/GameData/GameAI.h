#pragma once

#include "Statemachine.h"

namespace DOTL
{
	struct MinionAI : public Statemachine
	{
		// minion data
		float speed_ { 200.0f };
	};

	/*
		SEEKING STATE
		:
		In this state the minion constantly looks for a target and moves towards it
	*/
	struct MinionAISeek : public State
	{
		void Pre () override;

		void Update ( Statemachine* statemachine , float dt , ENTITIES const& entities , NetworkEntity* thisEntity ) override;

		void Post () override;
	};

	/*
		ATTACK STATE
		:
		In this state the minion attacks the target
	*/
}