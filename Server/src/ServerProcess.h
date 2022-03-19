#pragma once

#include "../../SharedSrc/NetworkWrapper/NetworkProcess.h"

namespace DOTL
{
	struct ServerProcess : public NetworkProcess
	{
		ServerProcess ();

		virtual void Initialize ( SOCKET serverSocket ) override;
		virtual void Update ( SOCKET serverSocket , bool& terminate ) override;
	};
}