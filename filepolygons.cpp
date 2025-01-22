#include "stdafx.h"
#include "filepolygons.h"

//////////////////////////////////////////////////////////////////////////////////////////////
/* Description: Stores the polygons(triangles) generated using the points in 3D space. */

/*	~~~~~~~~ Format for storing the polygon data in the form of triangles ~~~~~~~~~~~~~~

	Right now, we are attempting to keep the size of the file to a minimum. Hence we will
	store only the important information. Other informations, like the calculation of 
	normal vectors etc. which can be generated from the information in the file, will not 
	be stored.

	At the start of the file, we will store the following:
	1. The minx, maxx, miny, maxy, minz and maxz values within which all the triangles 
	stored lie.
    2. Color to be used while painting all the triangles( rgb value).

	This would be followed by triangle data for all the polygons.

	The data for each object would consist of  the co-ordinates of the three vertices 
	of the triangle in ccw sequence.*/
//////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(filePolygons, CObject, 1)

///////////////////////////////////////////////////////////////////////////////////////////////
filePolygons::filePolygons() {
	//default constructor
	error = FILE_POLYGONS_CONST + FILE_NOT_OPEN;
	
}

///////////////////////////////////////////////////////////////////////////////////////////////
filePolygons::filePolygons(CString inFile, RGBCOLOR inClr) {
	// Main constructor	
	/* Here we open the file in write mode and assign some default values to the header. */
	/* The header is updated later on during processing.                                 */ 
	
	
	flgFirstData = true; //Is set to false after the first triangle data is saved
	updatedBeforeClose = false  ; //this flag is checked in the destructor to ensure that a 
	//call to updateBeforeClose function has been given at the end of the processing
	
	
	//Initialize all the values which specify the range in 3d space within which all the 
	//co-ordinate points lie
	minxAll = maxxAll = minyAll = maxyAll = minzAll = maxzAll = 0.0f;
	
	g_strFilePath = inFile;
	
	if(!fp.Open(g_strFilePath, CFile::modeCreate | CFile::modeReadWrite 
			| CFile::shareExclusive )) {
	
		//set the flag to false
		error = FILE_POLYGONS_CONST + FILE_CREATION_ERROR ;
		return ;
	}	
	
	fp.SeekToBegin();
	
	//write the header information
	write(minxAll); write(maxxAll); write(minyAll); write(maxyAll); 
	write(minzAll); write(maxzAll); //4 bytes each for type float
	write(inClr.red); 
	write(inClr.green);
	write(inClr.blue); //One byte each

	g_uintPtsIndex = 0; //Initialize the index of the points stored in an array

	error = SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int filePolygons::errorStatus() {
	return  error;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void filePolygons::write(BYTE inByte)	{
		
	CArchive ar(&fp, CArchive::store);
	ar << inByte;			
}
///////////////////////////////////////////////////////////////////////////////////////////////

void filePolygons::write(float inVal)	{
		
	CArchive ar(&fp, CArchive::store);
	ar << inVal;			
}

///////////////////////////////////////////////////////////////////////////////////////////////
void filePolygons::storeTriangle( FLTPOINT3D inP1, FLTPOINT3D inP2, FLTPOINT3D inP3	) {
	
	VERIFY(error == SUCCESS ) ; //Raise an assertion if there is an error in the constructor
	
	if(flgFirstData) {

		//initialize the min-max data for all triangles 
		minxAll = maxxAll = inP1.x;
		minyAll = maxyAll = inP1.y;
		minzAll = maxzAll = inP1.z;
		flgFirstData = false ;
	}
	
	
	if(g_uintPtsIndex == UINT_POLS_ARR_SIZE) { 
		//Write to file and re-initialize
		fp.SeekToEnd() ; //move to the end of the file for writing
		fp.Write(g_arrFltPoints, UINT_POLS_ARR_SIZE*4);
		g_uintPtsIndex = 0;
	}
	
	//write the coordinates of the vertices of the triangle
	g_arrFltPoints[g_uintPtsIndex]=inP1.x ; 
	g_uintPtsIndex++;
	g_arrFltPoints[g_uintPtsIndex]=inP1.y ; 
	g_uintPtsIndex++;
	g_arrFltPoints[g_uintPtsIndex]=inP1.z ; 
	g_uintPtsIndex++;
	g_arrFltPoints[g_uintPtsIndex]=inP2.x ; 
	g_uintPtsIndex++;
	g_arrFltPoints[g_uintPtsIndex]=inP2.y ;
	g_uintPtsIndex++;
	g_arrFltPoints[g_uintPtsIndex]=inP2.z ; 
	g_uintPtsIndex++;
	g_arrFltPoints[g_uintPtsIndex]=inP3.x ; 
	g_uintPtsIndex++;
	g_arrFltPoints[g_uintPtsIndex]=inP3.y ; 
	g_uintPtsIndex++;
	g_arrFltPoints[g_uintPtsIndex]=inP3.z ; 
	g_uintPtsIndex++;
	

	
	/*
	write(inP1.x) ; write(inP1.y) ; write(inP1.z) ; 
	write(inP2.x) ; write(inP2.y) ; write(inP2.z) ; 
	write(inP3.x) ; write(inP3.y) ; write(inP3.z) ; 
	*/
	//reset the min, max using the new values	
	
	minxAll = Min(minxAll, inP1.x, inP2.x, inP3.x);
	minyAll = Min(minyAll, inP1.y, inP2.y, inP3.y);
	minzAll = Min(minzAll, inP1.z, inP2.z, inP3.z);

	maxxAll = Max(maxxAll, inP1.x, inP2.x, inP3.x);
	maxyAll = Max(maxyAll, inP1.y, inP2.y, inP3.y);
	maxzAll = Max(maxzAll, inP1.z, inP2.z, inP3.z);
				 
}

///////////////////////////////////////////////////////////////////////////////////////////////
void filePolygons::updateBeforeClose() {
	
	updatedBeforeClose = true ; //function has been called before exiting

	if(!flgFirstData) { //if any polygons were stored
		
		/* ~~~~~~~~~~~~~~~~~~~~ WRITE THE REST OF THE FILE ~~~~~~~~~~~~~~~~*/
		//Write to file and re-initialize
		fp.SeekToEnd() ; //move to the end of the file for writing
		fp.Write(g_arrFltPoints, g_uintPtsIndex*4);
		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		

		/* ~~~~~~~~~~~~~~~~~~~~ UPDATE FILE HEADER ~~~~~~~~~~~~~~~~*/
		fp.SeekToBegin();
		
		write(minxAll); write(maxxAll); 
		write(minyAll); write(maxyAll); 
		write(minzAll); write(maxzAll); 
		
		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		//close the file
		fp.Close() ;
		fp.m_hFile = NULL;
	} 
	else {
		fp.SetLength(0) ;
		fp.Close() ;
		fp.Remove(g_strFilePath); //Delete the file
		fp.m_hFile = NULL;
	}
	

	
}

///////////////////////////////////////////////////////////////////////////////////////////////
float filePolygons::Min(float inV1, float inV2, float inV3, float inV4)	{
	
	float outV = inV1;
	
	if(outV > inV2 )
		outV = inV2;
	
	if(outV > inV3 )
		outV = inV3;
	
	if(outV > inV4 )
		outV = inV4;
	
	return outV;
}

///////////////////////////////////////////////////////////////////////////////////////////////
float filePolygons::Max(float inV1, float inV2, float inV3, float inV4)	{
	
	float outV = inV1;
	
	if(outV < inV2 )
		outV = inV2;
	
	if(outV < inV3 )
		outV = inV3;
	
	if(outV < inV4 )
		outV = inV4;
	
	return outV;
}

///////////////////////////////////////////////////////////////////////////////////////////////
filePolygons::~filePolygons() {
	
	//VERIFY ( updatedBeforeClose ) ; //to ensure that a call
	//to updateBeforeClose function has been given at the end of the processing
	//09-Mar-04: Commmented because in case of abrupt cancellation, it might not be called

	if(fp.m_hFile != NULL && fp.m_hFile != (UINT)(-1)) //(UINT)(-1) is the check against which 
		fp.Close();    								   //an assertion failure is raised in CFile
													  //close function, so we check it here itself

		
}

///////////////////////////////////////////////////////////////////////////////////////////////
