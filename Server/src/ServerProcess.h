#pragma once

#include "../../SharedSrc/NetworkWrapper/NetworkProcess.h"

#include <string>
#include <sstream>
#include <unordered_map>
#include <iomanip>
#include <chrono>

namespace DOTL
{
	struct ClientMapInfo
	{
		SOCKET		socket_;
		std::string username_;
		bool		spawned_ { false };
	};

	using CLIENT_MAP = std::unordered_map<uint16_t , ClientMapInfo>;

	struct ServerInstance_WinSock2;
	struct ServerProcess : public NetworkProcess
	{
		ServerProcess ( ServerInstance_WinSock2* serverInstance );

		virtual void Initialize ( SOCKET clientSocket ) override;
		virtual void Update ( SOCKET clientSocket , bool& connected , double dt ) override;

	private:
		uint16_t id_ { 0 };
		uint16_t player_id_ { 0 };
		std::string username_ { "" };
		ServerInstance_WinSock2* const server_instance_;

		template <typename...ARGS>
		std::string FormatNamedMessage ( char const* name , ARGS...args ) const
		{
			std::stringstream format;
			format << "[" << std::setw ( 10 ) << name << "] " << std::setw ( 0 );
			( ( format << args ) , ... );
			format << "\n";
			return format.str ();
		}

		template <typename...ARGS>
		void SendNamedMessage ( char const* name , SOCKET clientSocket , ARGS...args ) const
		{
			NetworkSend ( clientSocket , NetworkPacket ( FormatNamedMessage ( name , args... ).c_str () ) );
		}

		template <typename...ARGS>
		void SendNamedMessage ( char const* name , CLIENT_MAP const& clients , ARGS...args ) const
		{
			for ( auto const& client : clients )
			{
				SendNamedMessage ( name , client.second.socket_ , args... );
			}
		}

		void SendNetworkPacketToAll ( NetworkPacket const& packet );

		template <typename...ARGS>
		void ServerLog ( char status , ARGS...args ) const
		{
			std::cout << "[" << status << "] ";
			( ( std::cout << args ) , ... );
			std::cout << std::endl;
		}
	};
}