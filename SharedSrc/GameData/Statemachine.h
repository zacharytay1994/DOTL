#pragma once

#include <memory>
#include <vector>

namespace DOTL
{
	struct NetworkEntityExtended;
	struct NetworkEntity;
	struct Statemachine;
	using ENTITIES = std::vector<NetworkEntityExtended>;
	struct State
	{
		virtual void Pre () = 0;
		virtual void Update ( Statemachine* statemachine , float dt , ENTITIES const& entities , NetworkEntity* entity ) = 0;
		virtual void Post () = 0;
	};

	struct Statemachine
	{
		Statemachine () = default;

		void Update ( float dt , ENTITIES const& entities , NetworkEntity* entity );

		void SwitchState ( std::shared_ptr<State> state );

	private:
		std::shared_ptr<State> current_state_ { nullptr };
		std::shared_ptr<State> old_state_ { nullptr };
	};
}
