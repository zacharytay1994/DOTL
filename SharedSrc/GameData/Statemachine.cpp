#include "Statemachine.h"

namespace DOTL
{
	void Statemachine::Update ( float dt , ENTITIES const& entities , NetworkEntity* entity )
	{
		if ( current_state_ != nullptr )
		{
			current_state_->Update ( this , dt , entities , entity );
		}
	}

	void Statemachine::SwitchState ( std::shared_ptr<State> state )
	{
		old_state_ = current_state_;
		if ( current_state_ != nullptr )
		{
			current_state_->Post ();
		}
		current_state_ = state;
		if ( current_state_ != nullptr )
		{
			current_state_->Pre ();
		}
	}
}