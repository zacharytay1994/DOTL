#include "GameAI.h"
#include "GameData.h"

#include <iostream>
#include <limits>

namespace DOTL
{
	void MinionAISeek::Pre ()
	{
	}

	void MinionAISeek::Update ( Statemachine* statemachine , float dt , ENTITIES const& entities , NetworkEntity* thisEntity )
	{
		MinionAI* minion_ai = reinterpret_cast< MinionAI* >( statemachine );

		float nearest_player_max = std::numeric_limits<float>::max ();
		for ( auto const& entity : entities )
		{
			// look for nearest player
			switch ( entity.entity_.type_ )
			{
			case ( ET::PLAYER ):
			{
				float v_x = entity.entity_.GetData ( ED::POS_X ) - thisEntity->GetData ( ED::POS_X );
				float v_y = entity.entity_.GetData ( ED::POS_Y ) - thisEntity->GetData ( ED::POS_Y );
				float length = sqrt ( v_x * v_x + v_y * v_y );
				if ( length < nearest_player_max )
				{
					nearest_player_max = length;
					thisEntity->SetVelocity ( (v_x / length) * minion_ai->speed_ , (v_y / length) * minion_ai->speed_ );
				}
				break;
			}
			}
		}
	}

	void MinionAISeek::Post ()
	{
	}
}