#ifndef _FILEREADPOINTS_H_
#define _FILEREADPOINTS_H_

#include <afxext.h>
#include "errorConstants.h"
#include "definitions.h"

class fileReadPoints : public CObject
{	
	DECLARE_SERIAL(fileReadPoints)

public: 
	fileReadPoints();
	fileReadPoints(CString inFile);
	~fileReadPoints();
	
	CUBOIDFILEINFO getCuboidValues();
	
	CUBECORNERS getNextXFace(UINT inX, UINT inY, UINT inZ);
	
	CUBECORNERS getCube(UINT inX, UINT inY, UINT inZ);

	BOOL isFaceWithValidPt(UINT inX, UINT inY, UINT inZ, BYTE inFace);
	
	int errorStatus();

private:
	CFile fp;
	UINT curZ;
	int error;
	BOOL flgFirstCall;
	UINT bytesPerPlane ;
	UINT m_uintFileSize;
	BYTE curBkOffsetBits, curFtOffsetBits, nextOffsetBits;
	CUBOIDFILEINFO cbInfo ;

	//Pointers to store planes along z axis
	BYTE *curBkPlane;
	BYTE *curFtPlane;
	BYTE *nextPlane;
	
	BYTE read();
	UINT readInt();
	float readFloat();
	BOOL isValidPoint(UINT inX, UINT inY, UINT inZ) ;
	void updatePlaneData(); 	
	
};

#endif