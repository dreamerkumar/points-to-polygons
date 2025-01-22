#include "stdafx.h"
#include "pointstopolygons.h"

extern "C" __declspec ( dllexport ) int createModel(RGBCOLOR inClr, CString inStrInputFile, 
								   CString inStrOutputFile, 
								   UINT inIntMinx, UINT inIntMaxx, UINT inIntMiny, 
								   UINT inIntMaxy, UINT inIntMinz, UINT inIntMaxz) {
	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	pointsToPolygons *ptr;
	
	ptr = new pointsToPolygons(inClr, inStrInputFile, inStrOutputFile, 
								   inIntMinx, inIntMaxx, inIntMiny, inIntMaxy, 
								   inIntMinz, inIntMaxz );
	
	UINT intRtnVal = ptr->errorStatus();
	
	if(intRtnVal != SUCCESS) {
		delete ptr;
		return intRtnVal;
	}

	
	intRtnVal = ptr->process();

	delete ptr;

	return intRtnVal;
}
