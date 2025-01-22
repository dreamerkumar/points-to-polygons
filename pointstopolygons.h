#ifndef _POINTSTOPOLYGONS_H_
#define _POINTSTOPOLYGONS_H_

#include "filepolygons.h"
#include "cubeStatus.h"
#include "filereadpoints.h"

#include "definitions.h"
#include "errorConstants.h"

class pointsToPolygons {

public:
	//pointsToPolygons(RGBCOLOR inClr, CString inInputFile, CString inOutputFile, 
	//							   UINT inMinx, UINT inMaxx, UINT inMiny, UINT inMaxy, 
	//							   UINT inMinz, UINT inMaxz );
	
	pointsToPolygons(CREATE_MODEL_INFO *inData);
	
	~pointsToPolygons();
	int errorStatus();
	BOOL process();

private:
	CREATE_MODEL_INFO *dataPtr;
	int error;
	
	//objects
	cubeStatus      *cubes;
	filePolygons    *pols;
	fileReadPoints  *points;
	
	UINT minx, maxx, miny, maxy, minz, maxz ; 
		
	CUBOIDFILEINFO  cbInfo;

	float xSpan, ySpan, zSpan; //length of the sides of individual cubes
	
	CUBECORNERS getNextXCube(CUBECORNERS inCb, UINT inX, UINT inY, UINT inZ) ;
		
	BOOL isValidCube(CUBECORNERS inCb);

	BOOL isValidCube(UINT inX, UINT inY, BYTE inPlane) ;

	void setStatusAsValid(UINT inX, UINT inY);

	void drawSurfaces(UINT inX, UINT inY, UINT inZ, CUBECORNERS inCb, 
		FACES inSharedFaces ) ;

	void storeTriangle(UINT inX, UINT inY, UINT inZ, CUBECORNERS inCorners, 
		CUBECORNERS inVertices, BYTE inFace) ;
	
	void storeTriangleToFile(UINT inX, UINT inY, UINT inZ, BYTE inVertices[]);
};

#endif