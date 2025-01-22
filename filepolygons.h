#ifndef _FILEPOLYGONS_H_
#define _FILEPOLYGONS_H_

#include <afxext.h>
#include "errorConstants.h"
#include "definitions.h"

class filePolygons : public CObject
{	
	DECLARE_SERIAL(filePolygons)

public: 
	filePolygons();
	~filePolygons();
	filePolygons(CString inFile, RGBCOLOR inClr);
	void storeTriangle( FLTPOINT3D inP1, FLTPOINT3D inP2, FLTPOINT3D inP3);
	void updateBeforeClose();
	int   errorStatus();

private:
	CFile fp;
	
	float g_arrFltPoints[UINT_POLS_ARR_SIZE];
	UINT  g_uintPtsIndex; 

	CString g_strFilePath;

	int   error ;
	void  write(BYTE  inByte);
	void  write(float inVal);
	float minxAll, maxxAll, minyAll, maxyAll, minzAll, maxzAll;
	float Min(float inV1, float inV2, float inV3, float inV4);
	float Max(float inV1, float inV2, float inV3, float inV4);
	BOOL  flgFirstData; //set to false after the first data is processed
	BOOL updatedBeforeClose ;
};

#endif