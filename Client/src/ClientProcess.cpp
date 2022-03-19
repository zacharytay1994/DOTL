#include "ClientProcess.h"

namespace DOTL
{
	SFMLProcess::SFMLProcess ( int windowWidth , int windowHeight , char const* name )
		:
		sfml_instance_ ( windowWidth , windowHeight , name )
	{
	}

	void SFMLProcess::Initialize ( SOCKET clientSocket )
	{
	}

	void SFMLProcess::Update ( SOCKET clientSocket , bool& terminate )
	{
		sfml_instance_.Update ();
	}
}