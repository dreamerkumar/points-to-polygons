#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <afxext.h>

//Variable definition

//Common naming  conventions:
//All input parameters should have prefix in.
//All output parameters should have prefix out.
//All structures should be defined in capital letters without underscores.
//All constants should be in capital letters and can have underscores. 

#define INNER			0
#define MINX			1
#define MINY			2
#define MINZ			3
#define MAXX			4
#define MAXY			5
#define MAXZ			6
#define MINX_MINY_MAXZ  7
#define DGPLANE1        8
#define DGPLANE2        9
#define DGPLANE3        10
#define DGPLANE4        11
#define DGPLANE5        12
#define DGPLANE6        13
#define OF_FOUR			14

#define BACK_TOP_LEFT      1 
#define BACK_BOTTOM_LEFT   2 
#define BACK_BOTTOM_RIGHT  3 
#define BACK_TOP_RIGHT     4 
#define FRONT_TOP_LEFT     5
#define FRONT_BOTTOM_LEFT  6 
#define FRONT_BOTTOM_RIGHT 7
#define FRONT_TOP_RIGHT    8

#define CURRENT_PLANE      1
#define BACK_PLANE         2 

#define MOLD_HEADER_SIZE   141

#define UINT_MAX_AXES_POINTS			   1000

#define UINT_POLS_ARR_SIZE                 25002 //Array will take approximately 100 KB

class FLTPOINT3D {//FLTPOINT3D fltPoint3d; Can be used to define points in 3d space
public: 
	float x;
	float y;
	float z;
	FLTPOINT3D() ;
	FLTPOINT3D( float inX, float inY, float inZ ) ;
	operator = ( FLTPOINT3D inP ) ;
	FLTPOINT3D operator + ( FLTPOINT3D inP ) ;
	FLTPOINT3D operator * ( float inK ) ;
	FLTPOINT3D operator / ( float inK ) ;	
}; 

//Color
class RGBCOLOR {
public:
	BYTE red;
	BYTE blue;
	BYTE green;
	RGBCOLOR() ;
	BOOL operator == ( RGBCOLOR inC ) ; 
	BOOL operator != ( RGBCOLOR inC ) ; 
};

//Cuboid Info:
struct CUBOIDFILEINFO {
	UINT x, y, z; //Number of points along the three axes
	float minx, maxx, miny, maxy, minz, maxz; //Positions of the cuboid corners
};

//Cube Index :
struct CUBEINDEX {
	UINT x, y, z; //Index
	BYTE face; //Face that is already processed
};

//Cube corner values 
class CUBECORNERS {
public:
	CUBECORNERS();
	
	operator = ( CUBECORNERS inCb ) ;
	void initialize() ;
	BOOL triangleOnCubeFace();
	
	BOOL backTopLeft ;
	BOOL backBottomLeft ;
	BOOL backBottomRight ;
	BOOL backTopRight ; 

	BOOL frontTopLeft ;
	BOOL frontBottomLeft ;
	BOOL frontBottomRight ;
	BOOL frontTopRight ;
};

//Cube index and corners 
struct CUBEDATA {
	CUBEINDEX I;
	CUBECORNERS cb ;
};

//Faces of a cube
class FACES {
public:
	FACES();
	void initialize();
	BOOL minx, maxx, miny, maxy, minz, maxz ;
};


#define SECURITY_STRING_PTSTOPOLYGONS "<C!@%GM&&I^(&E%jhKqLqJj#!@~hwJqAj"

struct PROCESS_INFO {
	PROCESS_INFO() {
		g_blnCancelProcess = false;
		g_fltPercentComplete = 0.0f;
		g_uintTotalCount = 0;
		g_strInputFile = "";
		g_strOutputFile = "";
		g_strSecurity = "";

	}
private:
	BOOL g_blnCancelProcess;
	UINT g_uintTotalCount;
	float g_fltPercentComplete;
	CString g_strSecurity;

public:
	CString g_strInputFile;
	CString g_strOutputFile;	

	void setSecurity(CString inStrSecurity) {
		g_strSecurity = inStrSecurity;
	}

	CString getSecurity() {
		return g_strSecurity;
	}

	void cancelProcess() {
		g_blnCancelProcess = true;
	}
		
	float getCompletedPercent() {
		return g_fltPercentComplete;
	}
	
	BOOL isCancelled() {
		return g_blnCancelProcess;
	}

	void initMaxNumber(UINT inUintMaxNumber) {
		g_uintTotalCount = inUintMaxNumber;
	}
	
	void setNewPercent(UINT inUintCompleted) {
		VERIFY(g_uintTotalCount > 0 && inUintCompleted <= g_uintTotalCount);
		float fltPercentComplete = ((float)inUintCompleted/(float)g_uintTotalCount)*100.0f;
		VERIFY(fltPercentComplete >= g_fltPercentComplete);
		g_fltPercentComplete = fltPercentComplete;
	}
};


struct CREATE_MODEL_INFO : PROCESS_INFO {
	HMODULE moduleHandle;
	RGBCOLOR modelClr;
	UINT intMinx;
	UINT intMaxx;
	UINT intMiny;
	UINT intMaxy;
	UINT intMinz;
	UINT intMaxz;
};


#endif
