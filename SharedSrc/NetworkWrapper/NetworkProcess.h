#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <memory>

namespace DOTL
{
	struct NetworkProcess
	{
		NetworkProcess () = default;

		virtual void Initialize ( SOCKET socket ) = 0;
		virtual void Update ( SOCKET socket , bool& terminate ) = 0;
	};

	using pNetworkProcess = std::shared_ptr<NetworkProcess>;
}