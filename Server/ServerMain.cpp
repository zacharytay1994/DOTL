#include "src/ServerWrapper/ServerWrapper.h"
//#include "src/ServerProcess.h"

#include <memory>

int main ()
{
	int max_players { 2 };

	std::cout << "Please enter your IP address: " << std::endl;
	std::string ip_address;
	std::cin >> ip_address;
	if ( ip_address == "0" )
	{
		ip_address = "127.0.0.1";
	}

	DOTL::ServerInstance_WinSock2 server_instance ( ip_address.c_str () , 5050 , max_players );
	if ( !server_instance.SetupSuccess () )
	{
		std::cerr << "Failed to setup server!" << std::endl;
	}
	else
	{
		server_instance.Update<DOTL::ServerProcess> ();
	}
}