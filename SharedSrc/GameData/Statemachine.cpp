#include "Statemachine.h"

namespace DOTL
{
	void Statemachine::Update ( float dt , GameData& data , NetworkEntityExtended* entity )
	{
		if ( current_state_ != nullptr )
		{
			current_state_->Update ( this , dt , data , entity );
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