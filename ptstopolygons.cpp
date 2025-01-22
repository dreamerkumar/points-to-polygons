// ptstopolygons.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "pointstopolygons.h"
#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static AFX_EXTENSION_MODULE PtstopolygonsDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("PTSTOPOLYGONS.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(PtstopolygonsDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(PtstopolygonsDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("PTSTOPOLYGONS.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(PtstopolygonsDLL);
	}
	return 1;   // ok
}


extern "C" _declspec (dllexport) UINT createModel(CREATE_MODEL_INFO *inData) {
		
	//Validate the calling component
	if(inData->getSecurity() != SECURITY_STRING_PTSTOPOLYGONS)
		return INVALID_SECURITY_STRING;

	pointsToPolygons *ptr;
		
	ptr = new pointsToPolygons(inData);
	//inData->modelClr, inData->strInputFile, inData->strOutputFile,
	//							   inData->intMinx, inData->intMaxx,
	//							   inData->intMiny, inData->intMaxy, 
	//							   inData->intMinz, inData->intMaxz );
	
	UINT intRtnVal = ptr->errorStatus();
	
	if(intRtnVal != SUCCESS) {
		delete ptr;
		return intRtnVal;
	}

	
	intRtnVal = ptr->process();

	delete ptr;

	return intRtnVal;
}
