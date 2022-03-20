#pragma once

#include "../../SharedSrc/NetworkWrapper/NetworkProcess.h"

#include <string>
#include <sstream>
#include <unordered_map>
#include <iomanip>

namespace DOTL
{
	struct ServerInstance_WinSock2;
	struct ServerProcess : public NetworkProcess
	{
		using CLIENT_MAP = std::unordered_map<std::string , SOCKET>;

		ServerProcess ( ServerInstance_WinSock2* serverInstance );

		virtual void Initialize ( SOCKET clientSocket ) override;
		virtual void Update ( SOCKET clientSocket , bool& connected ) override;

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
				SendNamedMessage ( name , client.second , args... );
			}
		}

		template <typename...ARGS>
		void ServerLog ( char status , ARGS...args ) const
		{
			std::cout << "[" << status << "] ";
			( ( std::cout << args ) , ... );
			std::cout << std::endl;
		}

	private:
		std::string username_ { "" };
		ServerInstance_WinSock2* const server_instance_;
	};
}