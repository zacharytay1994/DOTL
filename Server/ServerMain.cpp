#include "src/ServerWrapper/ServerWrapper.h"
//#include "src/ServerProcess.h"

#include <memory>

int main ()
{
	int max_players { 2 };

	DOTL::ServerInstance_WinSock2 server_instance ( "127.0.0.1" , 5050 , max_players );
	if ( !server_instance.SetupSuccess () )
	{
		std::cerr << "Failed to setup server!" << std::endl;
	}
	else
	{
		server_instance.Update<DOTL::ServerProcess> ();
	}
}