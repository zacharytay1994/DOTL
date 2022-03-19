#pragma once

#include "../../SharedSrc/SFMLWrapper/SFMLWrapper.h"
#include "../../SharedSrc/NetworkWrapper/NetworkProcess.h"

#include <memory>

namespace DOTL
{
	struct SFMLProcess : public NetworkProcess
	{
		SFMLProcess ( int windowWidth , int windowHeight , char const* name );

		virtual void Initialize ( SOCKET clientSocket ) override;
		virtual void Update ( SOCKET clientSocket , bool& terminate ) override;

	private:
		SFMLInstance sfml_instance_;
	};
}