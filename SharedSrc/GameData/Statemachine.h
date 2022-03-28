#pragma once

#include <memory>
#include <vector>

namespace DOTL
{
	struct GameData;
	struct NetworkEntityExtended;
	struct Statemachine;
	struct State
	{
		virtual void Pre () = 0;
		virtual void Update ( Statemachine* statemachine , float dt , GameData& GameData , NetworkEntityExtended* entity ) = 0;
		virtual void Post () = 0;
	};

	struct Statemachine
	{
		Statemachine () = default;

		void Update ( float dt , GameData& GameData , NetworkEntityExtended* entity );

		void SwitchState ( std::shared_ptr<State> state );

	private:
		std::shared_ptr<State> current_state_ { nullptr };
		std::shared_ptr<State> old_state_ { nullptr };
	};
}
