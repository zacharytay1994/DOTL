// Memory leak detection
#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>

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

	std::shared_ptr<DOTL::SFMLProcess> process = std::make_shared<DOTL::SFMLProcess> ( 1600 , 900 , "Client" );
	DOTL::ClientInstance_WinSock2 client_instance ( "192.168.1.117" , 5050 , process );

	if ( !client_instance.SetupSuccess () )
	{
		std::cerr << "Failed to setup client!" << std::endl;
	}
	else
	{
		client_instance.Update ();
	}
}