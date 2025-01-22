#include "stdafx.h"
#include "filereadpoints.h"

IMPLEMENT_SERIAL(fileReadPoints, CObject, 1)

///////////////////////////////////////////////////////////////////////////////////////////////

//default constructor
fileReadPoints::fileReadPoints() {
		error = FILE_READ_POINTS_CONST + FILE_NOT_OPEN;
}
///////////////////////////////////////////////////////////////////////////////////////////////

//main constructor
fileReadPoints::fileReadPoints(CString inFile) {
	
	UINT totalBytes, rem ;
	
	//Initialize pointers
	curBkPlane  = NULL;
	curFtPlane  = NULL;
	nextPlane   = NULL;
	
	//open the existing file
	if(!fp.Open(inFile, CFile::modeRead | CFile::shareExclusive)) {
		//set the flag to false
		error = FILE_READ_POINTS_CONST + FILE_ACCESS_ERROR;
		return ;
	}	
	
	m_uintFileSize = fp.GetLength();
	
	if(m_uintFileSize <= MOLD_HEADER_SIZE ) {
		error = FILE_READ_POINTS_CONST + FILE_SIZE_DOES_NOT_MATCH ;
		return ;
	}

	//get the cuboid values
	fp.SeekToBegin();
	
	//First get the point density (number of points per 10 cm) which is stored in the file
	cbInfo.x = readInt();
	cbInfo.y = readInt();
	cbInfo.z = readInt();
	
	//Get the range in 3D space along which the points are defined.
	cbInfo.minx = readFloat();
	cbInfo.maxx = readFloat();
	cbInfo.miny = readFloat();
	cbInfo.maxy = readFloat();
	cbInfo.minz = readFloat();
	cbInfo.maxz = readFloat();

	if(cbInfo.x < 2 || cbInfo.y < 2 || cbInfo.z < 2 ) {
		error = FILE_READ_POINTS_CONST + INSUFFICIENT_POINTS ;
		return ;
	}

	totalBytes = cbInfo.x * cbInfo.y * cbInfo.z ;
	rem = totalBytes % 8;
	if(rem != 0 ) 
		totalBytes = (totalBytes + 8 - rem)/8 ;
	else
		totalBytes = totalBytes/8 ;

	if(m_uintFileSize != (MOLD_HEADER_SIZE + totalBytes)) {
		error = FILE_READ_POINTS_CONST + FILE_SIZE_DOES_NOT_MATCH ;
		return ;
	}

	//Allocate memory to pointers~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			
	bytesPerPlane = cbInfo.x * cbInfo.y ;

	rem = bytesPerPlane % 8 ;
	
	if(rem == 0 ) 
		bytesPerPlane = bytesPerPlane  / 8 ;
	else
		bytesPerPlane = (bytesPerPlane + 8 - rem ) / 8 ;

	bytesPerPlane++ ; 
	//Extra byte added to accomodate sliding of data due to shifting of the first bit on 
	//accessing consecutive planes.
	
	try {
		
		if((curBkPlane  = ( BYTE * )malloc(bytesPerPlane)) == NULL) {
			fp.Close();
			error = FILE_READ_POINTS_CONST + MEMORY_ALLOCATION_FAILURE;
			return;

		}
		if((curFtPlane  = ( BYTE * )malloc(bytesPerPlane)) == NULL) {
			fp.Close();
			free(curBkPlane) ;  curBkPlane = NULL;
			error = FILE_READ_POINTS_CONST + MEMORY_ALLOCATION_FAILURE;
			return;

		}
		if((nextPlane   = ( BYTE * )malloc(bytesPerPlane)) == NULL) {
			free(curBkPlane) ;  curBkPlane = NULL;
			free(curFtPlane) ; curFtPlane = NULL;
			fp.Close();
			error = FILE_READ_POINTS_CONST + MEMORY_ALLOCATION_FAILURE;
			return;

		}
		
	} catch ( CException *e ) { 
	
		e->Delete();

		if(curBkPlane != NULL ) {
				free(curBkPlane) ;  curBkPlane = NULL;
		}

		if(curFtPlane != NULL ) {
			free(curFtPlane) ; curFtPlane = NULL;
		}

		if(nextPlane  != NULL ) {
			free(nextPlane) ; nextPlane = NULL;
		}

		fp.Close() ; 
		error = FILE_READ_POINTS_CONST + MEMORY_ALLOCATION_FAILURE;
		return;
		
	}
	
	curZ = 1;
	flgFirstCall = true;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//set the flag to true
	error = SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void fileReadPoints::updatePlaneData() {
	
	/*~~~~~~~~~Variable declaration~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	UINT bits, bytePos, uintActualBytesToRead ;
	BYTE *temp ;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	if(flgFirstCall) {

		flgFirstCall = false;	
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Assign to curBkPlane~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		bits	= ((curZ - 1) * cbInfo.x * cbInfo.y) + 1;
	
		curBkOffsetBits = bits%8; 
		
		if(curBkOffsetBits > 0)
			bits = bits + 8 - curBkOffsetBits;	
		
		bytePos = bits/8; 	
		bytePos += MOLD_HEADER_SIZE; 
		bytePos -= 1;
			
		fp.Seek(bytePos, CFile::begin);

		
		//We have increased the value of bytesPerPlane by one to accomodate increase in size 
		//due to sliding of bits. However, here we should ensure that we dont read one byte 
		//more than the end of the file
		uintActualBytesToRead = bytesPerPlane;
		if((bytePos + uintActualBytesToRead) > m_uintFileSize) 
			uintActualBytesToRead--; //Reduce by one 

		if(fp.Read(curBkPlane, uintActualBytesToRead) //Reading without using CArchive
			!= 	uintActualBytesToRead)
			throw new CException();
		

		//~~~~Set the offset bits after which the first value in the new plane( x=1, y=1 ) 
		//is stored 
		if(curBkOffsetBits == 0 ) 
			curBkOffsetBits = 8;
		curBkOffsetBits--;

		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Assign to curFtPlane~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		bits	= ( curZ * cbInfo.x * cbInfo.y ) + 1;
	
		curFtOffsetBits = bits%8; 
		
		if(curFtOffsetBits > 0 )
			bits = bits + 8 - curFtOffsetBits;	
		
		bytePos = bits/8; 	
		bytePos += MOLD_HEADER_SIZE; 
		bytePos -= 1;
	
		fp.Seek(bytePos, CFile::begin);

		//We have increased the value of bytesPerPlane by one to accomodate increase in size 
		//due to sliding of bits. However, here we should ensure that we dont read one byte 
		//more than the end of the file

		uintActualBytesToRead = bytesPerPlane;
		if((bytePos + uintActualBytesToRead) > m_uintFileSize) 
			uintActualBytesToRead--; //Reduce by one 

		if(fp.Read(curFtPlane, uintActualBytesToRead) != uintActualBytesToRead )
			throw new CException();

		//~~~~Set the offset bits after which the first value in the new plane( x=1, y=1 ) 
		//is stored 
		if(curFtOffsetBits == 0) 
			curFtOffsetBits = 8;
		curFtOffsetBits--;
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	} else {
		
		//Shift by one z index
		
		temp          = curBkPlane ;
		curBkPlane    = curFtPlane;
		curFtPlane    = nextPlane ;
		nextPlane     = temp ; //we allocate curBkPlane's set of bytes to nextPlane so
		//that we don't lose the allocated memory space of curBkPlane.
		
		curBkOffsetBits = curFtOffsetBits;
		curFtOffsetBits = nextOffsetBits ;
	}
	
	//If possible assign values to nextPlane or free the memory space
	if((curZ + 2) <= cbInfo.z) { //Such a plane exists
		
		bits	= ( (curZ + 1) * cbInfo.x * cbInfo.y ) + 1; //accessing the plane with z index
		//which is the third with respect to curZ
	
		nextOffsetBits = bits%8; 
		
		if(nextOffsetBits > 0 )
			bits = bits + 8 - nextOffsetBits ;	
		
		bytePos = bits/8; 	
		bytePos += MOLD_HEADER_SIZE ; 
		bytePos -= 1 ;
	
		fp.Seek(bytePos, CFile::begin);

		
		//We have increased the value of bytesPerPlane by one to accomodate increase in size 
		//due to sliding of bits. However, here we should ensure that we dont read one byte 
		//more than the end of the file

		uintActualBytesToRead = bytesPerPlane;
		if((bytePos + uintActualBytesToRead) > m_uintFileSize) 
			uintActualBytesToRead--; //Reduce by one 

		if(fp.Read(nextPlane, uintActualBytesToRead) != uintActualBytesToRead )
			throw new CException();                   

		//~~~~Set the offset bits after which the first value in the new plane( x=1, y=1 ) 
		//is stored 
		if(nextOffsetBits == 0 ) 
			nextOffsetBits = 8;
		nextOffsetBits-- ;

	} else {

		free(nextPlane) ;
		nextPlane = NULL ;

	}
	
}

///////////////////////////////////////////////////////////////////////////////////////////////

BYTE fileReadPoints::read() {
	
	BYTE outByte;
	CArchive ar (&fp, CArchive::load);
	ar >> outByte;
	return outByte;
}
///////////////////////////////////////////////////////////////////////////////////////////////

UINT fileReadPoints::readInt() {
	
	UINT outVal;
	CArchive ar (&fp, CArchive::load);
	ar >> outVal;
	return outVal;
}
///////////////////////////////////////////////////////////////////////////////////////////////

float fileReadPoints::readFloat() {
	
	float outVal;
	CArchive ar (&fp, CArchive::load);
	ar >> outVal;
	return outVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int fileReadPoints::errorStatus() {
	return  error;
}

///////////////////////////////////////////////////////////////////////////////////////////////

CUBOIDFILEINFO fileReadPoints::getCuboidValues() {
	
	VERIFY(error == SUCCESS );

	return cbInfo;
}

///////////////////////////////////////////////////////////////////////////////////////////////

CUBECORNERS  fileReadPoints::getNextXFace(UINT inX, UINT inY, UINT inZ) {
		
	CUBECORNERS outCb ;	

	VERIFY (inX > 0 && inX < cbInfo.x && inY > 0 && inY < cbInfo.y 
		&& inZ > 0 && inZ < cbInfo.z) ; //cube indexes should be one less than total points
	
	outCb.backBottomRight  = isValidPoint(inX+1, inY  , inZ  )  ;
	outCb.backTopRight     = isValidPoint(inX+1, inY+1, inZ  )  ;
	outCb.frontBottomRight = isValidPoint(inX+1, inY  , inZ+1)  ;
	outCb.frontTopRight    = isValidPoint(inX+1, inY+1, inZ+1)  ;
	
	return outCb ;
	
}
/////////////////////////////////////isFaceWithValidPt//////////////////////////////////////////
//Description:Returns true if it gets even one valid point on the specified face, else 
//returns false

BOOL fileReadPoints::isFaceWithValidPt(UINT inX, UINT inY, UINT inZ, BYTE inFace) {
	
	VERIFY (error == SUCCESS ) ;

	VERIFY (inX > 0 && inX < cbInfo.x && inY > 0 && inY < cbInfo.y 
		&& inZ > 0 && inZ < cbInfo.z) ; //cube indexes should be one less than total points
	
	VERIFY (inFace == MAXX || inFace == MAXY || inFace == MAXZ ) ;

	if(inFace == MAXX) {
		if( isValidPoint(inX+1, inY  , inZ  ) ||  //backBottomRight
			
			isValidPoint(inX+1, inY+1, inZ  ) ||  //backTopRight
				
			isValidPoint(inX+1, inY  , inZ+1) ||  //frontBottomRight
				
			isValidPoint(inX+1, inY+1, inZ+1))  //frontTopRight
			return true;
		else 
			return false;
	
	} else if(inFace == MAXY) {
		if( isValidPoint(inX  , inY+1, inZ  ) ||  //backTopLeft
			
			isValidPoint(inX+1, inY+1, inZ  ) ||   //backTopRight
			
			isValidPoint(inX  , inY+1, inZ+1) ||   //frontTopLeft
			
			isValidPoint(inX+1, inY+1, inZ+1))  //frontTopRight
			return true;
		else
			return false;
	
	} else if(inFace == MAXZ) {
		if( isValidPoint(inX  , inY+1, inZ+1) ||  //frontTopLeft
			
			isValidPoint(inX  , inY  , inZ+1) ||   //frontBottomLeft
				
			isValidPoint(inX+1, inY  , inZ+1) ||   //frontBottomRight
				
			isValidPoint(inX+1, inY+1, inZ+1))  //frontTopRight
			return true;
		else
			return false;	
	}	
	
}
///////////////////////////////////////////////////////////////////////////////////////////////

CUBECORNERS fileReadPoints::getCube(UINT inX, UINT inY, UINT inZ) {
	
	CUBECORNERS outCb;
	
	VERIFY (error == SUCCESS ) ;

	VERIFY (inX > 0 && inX < cbInfo.x && inY > 0 && inY < cbInfo.y 
		&& inZ > 0 && inZ < cbInfo.z) ; //cube indexes should be one less than total points
	
	outCb.backTopLeft      = isValidPoint(inX  , inY+1, inZ  );
	outCb.backBottomLeft   = isValidPoint(inX  , inY  , inZ  );
	outCb.backBottomRight  = isValidPoint(inX+1, inY  , inZ  );
	outCb.backTopRight     = isValidPoint(inX+1, inY+1, inZ  );
	
	outCb.frontTopLeft     = isValidPoint(inX  , inY+1, inZ+1);
	outCb.frontBottomLeft  = isValidPoint(inX  , inY  , inZ+1);
	outCb.frontBottomRight = isValidPoint(inX+1, inY  , inZ+1);
	outCb.frontTopRight    = isValidPoint(inX+1, inY+1, inZ+1);
	
	return outCb;		
}

///////////////////////////////////////////////////////////////////////////////////////////////

BOOL fileReadPoints::isValidPoint(UINT inX, UINT inY, UINT inZ) {

	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~DESCRIPTION~~~~~~~~~~~~~~~~~~~~~*/
	/*The way points are stored in the file:
	The points are stored the way the cuboid exists in the 
	coordinate system. First the point (1,1,1) is stored. Then we
	increase x and store points till (cuboidWidth, 1, 1).We repeat
	this series from y index 1 to cuboidHeight. So the first plane
	at z =1 is stored. We then store bits for z = 2 and so on till
	cuboidThickness.
	To minimize the file read and write operations, we read and 
	store the data for three z planes at a time. If the z index 
	exceeds, we update the pointers with data of the next planes.
	In the current function, we access the point through the data
	stored in these pointers, rather than accessing the file
	directly.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/*~~~~~~~~~Variable declaration~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	UINT bits, bytePos, bitPos ;
	BYTE curByte, checkByte ;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	VERIFY (error == SUCCESS ) ;

	VERIFY (inX > 0 && inX <= cbInfo.x && inY > 0 && inY <= cbInfo.y 
		&& inZ > 0 && inZ <= cbInfo.z ) ; 

	VERIFY (flgFirstCall || (inZ >= curZ && inZ <= curZ + 3)) ; 
		
	if(flgFirstCall) {
		curZ = inZ;
		updatePlaneData();

	}else if(inZ == curZ + 3) {
		curZ ++;
		updatePlaneData(); //update the data stored in the planes
	}

	//find the position in terms of individual bits
	if( inZ == curZ)
		bits = curBkOffsetBits  + ((inY - 1) * cbInfo.x) + inX; 
	
	else if (inZ == curZ + 1)
		bits = curFtOffsetBits  + ((inY - 1) * cbInfo.x) + inX; 
	
	else
		bits = nextOffsetBits   + ((inY - 1) * cbInfo.x) + inX; 
	
	bitPos = bits%8; //helps in deciding where the point is located
	//in the byte of 8 bits. For eg, if the value of 
	//bitPos is 0, then the bit to be accessed is the 8th one in the
	//current byte; If its 7 then it means the first element in the next byte. 
	
	//find the byte that has to be accessed.
	if(bitPos > 0 )
		bits = bits + 8 - bitPos ;//if it is not divisible by 8 then 
		//we make it so, by suitable addition 
	
	bytePos = bits/8; 
		
	bytePos = bytePos - 1 ; //if we want to read the nth byte then we
							//need to bypass n-1 bytes
	if( inZ == curZ ) 
		curByte = curBkPlane[bytePos];
	
	else if ( inZ == curZ + 1 ) 
		curByte = curFtPlane[bytePos];
	
	else 
		curByte = nextPlane[bytePos];

	
	switch(bitPos) {	
		case 1 :
			checkByte = 128;
			break;
		case 2 :
			checkByte =  64;
			break;
		case 3 :
			checkByte =  32;
			break;
		case 4 :
			checkByte =  16;
			break;
		case 5 :
			checkByte =   8;
			break;
		case 6 :
			checkByte =   4;
			break;
		case 7 :
			checkByte =   2;
			break;
		case 0 :
			checkByte =   1;
			break;
	}
					
	if( (curByte  & checkByte) == checkByte)
		return true ; //The point is marked as valid
	else
		return false ; //The point is invalid
}

///////////////////////////////////////////////////////////////////////////////////////////////
fileReadPoints::~fileReadPoints() {
	//if( error != FILE_READ_POINTS_CONST + FILE_NOT_OPEN || 
	//	error != FILE_READ_POINTS_CONST + FILE_ACCESS_ERROR ) 
		
		if(fp.m_hFile != NULL && fp.m_hFile != (UINT)(-1)) //(UINT)(-1) is the check against which 
		fp.Close();    								   //an assertion failure is raised in CFile
													  //close function, so we check it here itself																	
	
	if(curBkPlane != NULL ) 
		free(curBkPlane);
	if(curFtPlane != NULL )
		free(curFtPlane);
	if(nextPlane != NULL) 
		free(nextPlane);
	
}

///////////////////////////////////////////////////////////////////////////////////////////////