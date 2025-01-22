#ifndef _CUBESTATUS_H_
#define _CUBESTATUS_H_

#include "errorConstants.h"
#include "definitions.h"

class cubeStatus
{
		
public: 
	UINT initCubeStatus(UINT inX, UINT inY);
	~cubeStatus();	
	
	void setStatusAsValid(UINT inX, UINT inY) ;
	
	BOOL isValidCube(UINT inX, UINT inY, BYTE inPlane);//inPlane tells whether its the current 
													//or the previous plane					
	void goToNextPlane() ; 
private:
	UINT cubesX;
	UINT cubesY;
		
	BYTE *curCubes;
	BYTE *prevCubes;		

	UINT totalBytes ;
};

#endif