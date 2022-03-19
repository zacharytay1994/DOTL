#include "src/ServerWrapper/ServerWrapper.h"
#include "src/ServerProcess.h"

#include <memory>

int main ()
{
	std::shared_ptr<DOTL::ServerProcess> process = std::make_shared<DOTL::ServerProcess>();
	DOTL::ServerInstance_WinSock2 server_instance ( "192.168.1.117" , 5050 , process );
	if ( !server_instance.SetupSuccess () )
	{
		std::cerr << "Failed to setup server!" << std::endl;
	}
	else
	{
		server_instance.Update ();
	}
}