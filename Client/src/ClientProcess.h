#pragma once

#include "../../SharedSrc/SFMLWrapper/SFMLWrapper.h"
#include "../../SharedSrc/NetworkWrapper/NetworkProcess.h"
#include "../../SharedSrc/GameData/GameData.h"

#include <memory>
#include <thread>
#include <string>

namespace DOTL
{
	struct SFMLProcess : public NetworkProcess
	{
		SFMLProcess ( int windowWidth , int windowHeight , char const* name );

		virtual void Initialize ( SOCKET clientSocket ) override;
		virtual void Update ( SOCKET clientSocket , bool& connected ) override;

	private:
		SFMLInstance	sfml_instance_;
		std::thread		network_thread_;
		std::thread		input_thread_;
		bool			network_connected_ { false };

		uint16_t		player_id_ { 0 };
		std::string		player_username_ { "UNASSIGNED" };
		GameData		game_data_;

		void NetworkThreadUpdate ( SOCKET clientSocket );
		void InputThreadUpdate ( SOCKET clientSocket );

		void SyncGameDataFromServer ( NetworkPacket const& packet );
	};
}