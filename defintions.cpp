#include "stdafx.h" //The DLL does not compile without putting this statement here

#ifndef _DEFINITIONS_CPP_
#define _DEFINITIONS_CPP_

#include "definitions.h"

///////////////////FLTPOINT3D///////////////////////////////////////////////
FLTPOINT3D::FLTPOINT3D() {
	x = y = z = 0.0f;
}

FLTPOINT3D::FLTPOINT3D( float inX, float inY, float inZ ) {
	x = inX; y = inY; z = inZ;
}

FLTPOINT3D::operator = ( FLTPOINT3D inP ) {
	x = inP.x ; y = inP.y ; z = inP.z ; 
}

FLTPOINT3D FLTPOINT3D::operator + ( FLTPOINT3D inP ) {
	FLTPOINT3D outP;
	outP.x = x + inP.x ; outP.y = y + inP.y ; outP.z = z + inP.z ; 
	return outP;
}

FLTPOINT3D FLTPOINT3D::operator * ( float inK ) {
	FLTPOINT3D outP;
	outP.x = x * inK; outP.y = y * inK; outP.z = z * inK; 
	return outP;
}

FLTPOINT3D FLTPOINT3D::operator / ( float inK ) {
	FLTPOINT3D outP;
	outP.x = x / inK; outP.y = y / inK; outP.z = z / inK; 
	return outP;
}

/////////////////////////////RGBCOLOR/////////////////////////////////////////
RGBCOLOR::RGBCOLOR() 
	{red = blue = green = 0 ; }

BOOL RGBCOLOR::operator == ( RGBCOLOR inC ) {
	if(red == inC.red && green == inC.green && blue == inC.blue ) 
		return true ;
	else 
		return false;
}

BOOL RGBCOLOR::operator != ( RGBCOLOR inC ) {
	if(red == inC.red && green == inC.green && blue == inC.blue ) 
		return false;
	else 
		return true;
}
///////////////////////////CUBECORNERS///////////////////////////////////////////
CUBECORNERS::CUBECORNERS() {
	 
	//Set all the flags to zero
	 backTopLeft      = false ;
	 backBottomLeft   = false ;
	 backBottomRight  = false ;
	 backTopRight     = false ;

	 frontTopLeft     = false ;
	 frontBottomLeft  = false ;
	 frontBottomRight = false ;
	 frontTopRight    = false ;
}

CUBECORNERS::operator = ( CUBECORNERS inCb ) {	
	 
	 backTopLeft      = inCb.backTopLeft;
	 backBottomLeft   = inCb.backBottomLeft;
	 backBottomRight  = inCb.backBottomRight ;
	 backTopRight     = inCb.backTopRight;

	 frontTopLeft     = inCb.frontTopLeft;
	 frontBottomLeft  = inCb.frontBottomLeft ;
	 frontBottomRight = inCb.frontBottomRight;
	 frontTopRight    = inCb.frontTopRight;
}
void CUBECORNERS::initialize() {	
	 
	 //Set all the flags to zero
	 backTopLeft      = false ;
	 backBottomLeft   = false ;
	 backBottomRight  = false ;
	 backTopRight     = false ;

	 frontTopLeft     = false ;
	 frontBottomLeft  = false ;
	 frontBottomRight = false ;
	 frontTopRight    = false ;
}

BOOL CUBECORNERS::triangleOnCubeFace() {
	//If three points are on one face, then the other face will be blank
	if(
		//left face
		(	backBottomLeft   ==  false &&
			backTopLeft      ==  false &&
			frontBottomLeft  ==  false &&
			frontTopLeft     ==  false ) //all three vertices on the right face
		||

		//right face
	    (	backBottomRight  ==  false &&
			backTopRight     ==  false &&
			frontBottomRight ==  false &&
			frontTopRight    ==  false  ) //all three vertices on the left face
		||

		//front face
		(	frontBottomLeft  ==  false &&  
			frontBottomRight ==  false &&
			frontTopLeft     ==  false &&
			frontTopRight    ==  false  ) //all three vertices on the back face
		||

		//back face
		(	backBottomLeft   ==  false &&  
			backBottomRight  ==  false &&
			backTopLeft      ==  false &&
			backTopRight     ==  false  ) //all three vertices on the front face
		||

		//top face
		(	backTopLeft      ==  false && 
			backTopRight     ==  false &&
			frontTopLeft     ==  false &&
			frontTopRight    ==  false  ) //all three vertices on the bottom face
		||

		//bottom face
		(	backBottomLeft   ==  false && 
			backBottomRight  ==  false &&
			frontBottomLeft  ==  false &&
			frontBottomRight ==  false  ) //all three vertices on the top face
		)
	
		return true ;

	else
		return false ;
}

///////////////////////////FACES///////////////////////////////////////////
FACES::FACES() {
	minx = maxx = miny = maxy = minz = maxz = 0 ;
}
void FACES::initialize() {
	minx = maxx = miny = maxy = minz = maxz = 0;
}

/////////////////////////////////////////////////////////////////////////////////
 
#endif