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
		virtual void Update ( SOCKET clientSocket , bool& connected , double unusedDT ) override;

	private:
		// player data
		float			player_attack_range { 200.0f };
		float			player_attack_interval { 1.0f };
		float			player_attack_timer { 0.0f };
		uint16_t		target_id_ { 0 };

		MouseEventData	mouse_data_;
		SFMLInstance	sfml_instance_;
		std::thread		network_thread_;
		std::thread		input_thread_;
		bool			network_connected_ { false };

		uint16_t		player_id_ { 0 };	// this is entity id not client id, i.e. NetworkEntity representing the player in GameData::entities_
		std::string		player_username_ { "UNASSIGNED" };
		GameData		game_data_;

		void NetworkThreadUpdate ( SOCKET clientSocket );
		void InputThreadUpdate ( SOCKET clientSocket );

		void SyncGameDataFromServer ( NetworkPacket const& packet );

		// player update code
		void UpdateClient ( double dt , SOCKET clientSocket );
	};
}