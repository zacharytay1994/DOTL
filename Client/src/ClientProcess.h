#pragma once

#include "../../SharedSrc/SFMLWrapper/SFMLWrapper.h"
#include "../../SharedSrc/NetworkWrapper/NetworkProcess.h"

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

		void NetworkThreadUpdate ( SOCKET clientSocket );
		void InputThreadUpdate ( SOCKET clientSocket );

	private:
		SFMLInstance	sfml_instance_;
		std::thread		network_thread_;
		std::thread		input_thread_;
		bool			network_connected_ { false };

		// input 
		bool			reading_input_ { false };
	};
}