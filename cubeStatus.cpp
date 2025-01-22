#include "stdafx.h"
#include "cubeStatus.h"

//constructor
UINT cubeStatus::initCubeStatus(UINT inX, UINT inY) {
	
	/////////////////////////Variable Declaration////////////////////////////////////////
	UINT totalBits, rem ;	
	/////////////////////////////////////////////////////////////////////////////////////
		
	VERIFY(inX > 0 && inX <= UINT_MAX_AXES_POINTS && inY > 0 && inY <= UINT_MAX_AXES_POINTS ) ;
	
	/*~~~~~~~~~~~~~~~~set the cuboid values~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	curCubes = prevCubes = NULL ; 
		
	cubesX = inX; cubesY = inY; 

	//find the size of the file in bits
	totalBits = cubesX * cubesY ; 	
	
	//increase the value to make it divisible by 8
	rem = totalBits%8 ;

	if(rem != 0 )
		totalBits = totalBits + (8 - rem);
	
	//find the total number of bytes required to store the points
	totalBytes = totalBits / 8;

	//Allocate memory ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	try {

		if((curCubes  = (BYTE *)malloc(totalBytes)) == NULL)
			return CUBE_STATUS_INIT_CUBE_STATUS + MEMORY_ALLOCATION_FAILURE;

	} catch (CException *e) {
		e->Delete();
		return CUBE_STATUS_INIT_CUBE_STATUS + MEMORY_ALLOCATION_FAILURE;
	}
	
	try {
		if((prevCubes = (BYTE *)malloc(totalBytes))== NULL) {
			free(curCubes); curCubes = NULL;
			return CUBE_STATUS_INIT_CUBE_STATUS + MEMORY_ALLOCATION_FAILURE; 
		}
	
	} catch (CException *e) {		
		e->Delete();
		free(curCubes); curCubes = NULL;		
		return CUBE_STATUS_INIT_CUBE_STATUS + MEMORY_ALLOCATION_FAILURE;		
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Initialize~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* A one at a particular position signifies the cube is not valid   				*/
	/* A zero at a particular position signifies the cube is valid.	     				*/
	/* So we need to set all the bits to 1(invalid).                                    */
	
	
	memset(curCubes, 255, totalBytes);
	//for (ctr = 0; ctr < totalBytes; ctr++) 		
		//rCubes[ctr] = 255 ;
	return SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Function : setStatusAsValid(UINT inX, UINT inY)

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~DESCRIPTION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	Goes to specified index and sets the bit value to zero
	
	For a particular cube index:
	1.	If the bit value is zero then the cube is valid.
	2.	If the bit value is one then the cube is not valid.

~~~~~~~~~~~~~~~~Order in which the cube indices are stored~~~~~~~~~~~~~~~~~~~~
  	 
	  The cube info is always stored for two planes. The current plane and the
	  the previous plane. When the cubes for the first plane are accessed the 
	  previous plane has no significance. For each plane, we store all cubes 
	  along a particular height and then move to next y index. 
*/	
///////////////////////////////////////////////////////////////////////////////////////////////
void cubeStatus::setStatusAsValid(UINT inX, UINT inY) { 


	/*~~~~~~~~~Variable declaration~~~~*/
	UINT bits, bytePos, zeroBitPos;
	BYTE readByte, byteToWrite;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	//Handle errors~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	VERIFY(inX > 0 && inX <= cubesX && inY > 0 && inY <= cubesY);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	//find the position in terms of individual bits
	
	bits	=  ( (inY - 1) * cubesX ) + inX; 
	
	zeroBitPos = bits%8; //helps in deciding where the point is located
	//in the byte of 8 bits that is to be set to zero. For eg, if the value of 
	//zeroBitPos is 0, then the bit to be altered is the 8th one in the
	//current byte; If its 7 then it means the first element in the next byte. 
	
	//find the byte that have to be traversed
	
	if(zeroBitPos > 0 )
		bits = bits + 8 - zeroBitPos ;//if it is not divisible by 8 then 
		//we make it so, by suitable addition 
	
	bytePos = bits/8; 
	
	bytePos = bytePos - 1 ; //if we want to read the nth byte then we
							//need to bypass n-1 bytes
	
	readByte = curCubes[bytePos];

	//set the appropriate bit to zero to make it invalid 
	switch(zeroBitPos) {
	case 7 :
		byteToWrite = 253;
		break;
	case 6 :
		byteToWrite = 251;
		break;
	case 5 :
		byteToWrite = 247;
		break;
	case 4 :
		byteToWrite = 239;
		break;
	case 3 :
		byteToWrite = 223;
		break;
	case 2 :
		byteToWrite = 191;
		break;
	case 1 :
		byteToWrite = 127;
		break;
	case 0 :
		byteToWrite = 254;
		break;
	}

	byteToWrite = readByte & byteToWrite ; //the particular bit pos will be set to zero
										   // while the other bits will remain as they are	
	curCubes[bytePos] = byteToWrite ;
	
}
///////////////////////////////////////////////////////////////////////////////////////////////

BOOL cubeStatus::isValidCube(UINT inX, UINT inY, BYTE inPlane ) {

	/*~~~~~~~~~Variable declaration~~~~*/
	UINT bits, bytePos, bitPos;
	BYTE curByte, checkByte;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		
	//Handle errors~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	VERIFY(inPlane == CURRENT_PLANE ||	inPlane == BACK_PLANE);
		
	VERIFY(inX > 0 && inX <= cubesX && inY > 0 && inY <= cubesY);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	//find the position in terms of individual bits
	bits = ((inY - 1) * cubesX) + inX; 
	
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
	if(inPlane == CURRENT_PLANE) 
		curByte = curCubes[bytePos];
	else 
		curByte = prevCubes[bytePos];
	
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
		return false ; //The point is set to 1 which means it is not processed
	else
		return true ; //The point is set to zero which means it is processed
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Description : As we are moving to the next z plane, the status of cubes for the current z 
//index which is stored in curCubes should now be stored as prevCubes. We also need a new
//curCubes. We do it by assigning this pointer to prevCubes which already contains outdated
//data for a plane. All we need is to allocate a similar chunk of memory to curCubes which
//is accomplished through this step. Then we initialize the new data for curCubes.

void cubeStatus::goToNextPlane() {
	
	BYTE *temp ;
	
	//swap the addresses 
	temp      = curCubes;
	curCubes  = prevCubes;
	prevCubes = temp;
	
	//initialize
	memset(curCubes, 255, totalBytes);
	//for (ctr = 0; ctr < totalBytes; ctr++ ) 		
	//	curCubes[ctr] = 255 ;
}
///////////////////////////////////////////////////////////////////////////////////////////////

cubeStatus::~cubeStatus() {
	
	if(curCubes != NULL ) 
		free(curCubes) ;

	if(prevCubes != NULL ) 
		free(prevCubes) ;
}

////////////////////////////////// END ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////