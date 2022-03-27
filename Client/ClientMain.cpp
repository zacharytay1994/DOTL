// Memory leak detection
#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>
#include <string>

//#include "../SharedSrc/SFMLWrapper/SFMLWrapper.h"
#include "src/ClientProcess.h"
#include "src/ClientWrapper/ClientWrapper.h"

#include <memory>

int main ()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode ( _CRT_WARN , _CRTDBG_MODE_DEBUG );
#endif

	std::cout << "Please enter your IP address: " << std::endl;
	std::string ip_address;
	std::cin >> ip_address;
	if ( ip_address == "0" )
	{
		ip_address = "127.0.0.1";
	}
	std::shared_ptr<DOTL::SFMLProcess> process = std::make_shared<DOTL::SFMLProcess> ( 1200 , 1200 , "Client" );
	DOTL::ClientInstance_WinSock2 client_instance ( ip_address.c_str () , 5050 , process );

	if ( !client_instance.SetupSuccess () )
	{
		std::cerr << "Failed to setup client!" << std::endl;
	}
	else
	{
		client_instance.Update ();
	}
}