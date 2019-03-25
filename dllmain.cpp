#include "stdafx.h"

//////////////////////////////////////////////////////////////////////

HINSTANCE ghDllHandle = NULL;
BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{
	BOOLEAN bSuccess = TRUE;


	//  Perform global initialization.

	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:

		//  For optimization.

		ghDllHandle = hDllHandle;

		break;

	case DLL_PROCESS_DETACH:

		break;
	}



	return bSuccess;

}
//  end DllMain
