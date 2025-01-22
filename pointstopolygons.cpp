#include "stdafx.h"
#include "pointstopolygons.h"

//pointsToPolygons::pointsToPolygons(RGBCOLOR inClr, CString inInputFile, CString inOutputFile, 
//								   UINT inMinx, UINT inMaxx, UINT inMiny, UINT inMaxy, 
//								   UINT inMinz, UINT inMaxz ) {

pointsToPolygons::pointsToPolygons(CREATE_MODEL_INFO *inData) {
	
	dataPtr = inData;

	VERIFY (inData->intMinx >= 0 &&  inData->intMaxx >= 0 &&  inData->intMiny >= 0 &&  inData->intMaxy >= 0 &&  inData->intMinz >= 0 
			&& inData->intMaxz >= 0);
								   
	VERIFY ((inData->intMaxx == 0 || (inData->intMaxx > inData->intMinx && inData->intMaxx > 1) ) 
		 && (inData->intMaxy == 0 || (inData->intMaxy > inData->intMiny && inData->intMaxy > 1) )
		 && (inData->intMaxz == 0 || (inData->intMaxz > inData->intMinz && inData->intMaxz > 1) ));		
	
	pols    = NULL;
	cubes   = NULL;
	points  = NULL;
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~fileReadPoints~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	try {
		points = new fileReadPoints(dataPtr->g_strInputFile);
	} catch(CException *e) {
		e->Delete();
		error = POINTS_TO_POLYGONS_CONST + FILE_ACCESS_ERROR;
		return ;
	}

	
	if( points->errorStatus() != SUCCESS) {
		error = POINTS_TO_POLYGONS_CONST + points->errorStatus();
		return ;
	}

	cbInfo = points->getCuboidValues();
		
	cbInfo.x--; cbInfo.y--; cbInfo.z--; //The number of cubes is one
										//less than the number of points	
	if(dataPtr->intMinx  < 0 || dataPtr->intMinx > cbInfo.x || dataPtr->intMiny < 0 || dataPtr->intMiny > cbInfo.y || 
		dataPtr->intMinz < 0 || dataPtr->intMinz > cbInfo.z ) {
		error = POINTS_TO_POLYGONS_CONST + INDEX_OUT_OF_RANGE ;
		return ;
	}

	//Calculate the lengths of the sides of the  individual cubes
	xSpan = ( cbInfo.maxx - cbInfo.minx ) /(float) cbInfo.x ;

	ySpan = ( cbInfo.maxy - cbInfo.miny ) /(float) cbInfo.y ;

	zSpan = ( cbInfo.maxz - cbInfo.minz ) /(float) cbInfo.z ;

	if(dataPtr->intMinx == 0 ) 
		minx = 1 ;
	else 
		minx = dataPtr->intMinx ;
	
	if(dataPtr->intMaxx == 0 )
		maxx = cbInfo.x ;
	else
		maxx = dataPtr->intMaxx - 1;//cube index is one less than the max points

	if(dataPtr->intMiny == 0 ) 
		miny = 1 ;
	else 
		miny = dataPtr->intMiny ;
	
	if(dataPtr->intMaxy == 0 )
		maxy = cbInfo.y ;
	else
		maxy = dataPtr->intMaxy - 1;//cube index is one less than the max points

	if(dataPtr->intMinz == 0 ) 
		minz = 1 ;
	else 
		minz = dataPtr->intMinz ;
	
	if(dataPtr->intMaxz == 0 )
		maxz = cbInfo.z ;
	else
		maxz = dataPtr->intMaxz - 1;//cube index is one less than the max points		

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~filePolygons~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	try {
		pols   = new filePolygons(dataPtr->g_strOutputFile, dataPtr->modelClr);	
	} catch(CException *e) {
		e->Delete();
		error = POINTS_TO_POLYGONS_CONST + FILE_ACCESS_ERROR;
		return ;
	
	}
	
	if( pols->errorStatus() != SUCCESS) {
		error = POINTS_TO_POLYGONS_CONST + pols->errorStatus();
		return ;
	}

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~cubeStatus~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	cubes  = new cubeStatus();
	UINT uintReturnCode;
	uintReturnCode = cubes->initCubeStatus(maxx + 1 - minx, maxy + 1 - miny );
	if(uintReturnCode!= SUCCESS)
		error = POINTS_TO_POLYGONS_CONST + uintReturnCode; 

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	dataPtr->initMaxNumber((maxx+1-minx)*(maxy+1-miny)*(maxz+1-minz));
	
	error = SUCCESS ;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int pointsToPolygons::errorStatus() {
	return  error;
}

///////////////////////////////////////////////////////////////////////////////////////////////
BOOL pointsToPolygons::process() { 

	VERIFY(error == SUCCESS ) ;
	
	
	//Variables~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	BOOL flgFound = false;
	BOOL flgPrevValid ;
	UINT x, y, z, uintPtsProcessed; //Counters
	CUBECORNERS cb, cb1 ;//To store the cube corners
	FACES f; //shared faces
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		
		
	//identify a new object and generate outer surfaces for it~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	uintPtsProcessed =0;

	for(z = minz ; z <= maxz; z++) { 

		for(y = miny ; y <= maxy ; y++) {

			for(x = minx ; x <= maxx ; x++) {
				
				uintPtsProcessed++;

				if(dataPtr->isCancelled())
					return FAILURE;

				//get the cube 
				try {
					if(x == minx ) 
						cb = points->getCube(x, y, z); 
					else 
						cb = getNextXCube(cb, x, y, z);
				
				} catch (CException *e) {
						e->Delete();
						return POINTS_TO_POLYGONS_PROCESS + FILE_ACCESS_ERROR;
				}
				if(isValidCube(cb)) {
									
					if(!flgFound )
						flgFound = true ;
					
					//set the faces 
					f.initialize() ;
					/*~~~~~~~~~~~~~~~~~~~~~~~~~Faces at min Values~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
			
					if(x == minx )
						f.minx = false ;
					else 
						f.minx = flgPrevValid ;

					if(y == miny ) 
						f.miny = false ;
					else
						f.miny = isValidCube(x, y - 1, CURRENT_PLANE ) ; 

					if(z == minz ) 
						f.minz = false ; 
					else 
						f.minz = isValidCube(x, y, BACK_PLANE ) ;

					/*~~~~~~~~~~~~~~~~~~~~~~~~~Faces at max Values~~~~~~~~~~~~~~~~~~~~~~~~~~~*/	
					try {
						if(x == maxx ) 
							f.maxx = false ;
						else 
							f.maxx = points->isFaceWithValidPt(x + 1, y, z, MAXX);
					
						if(y == maxy ) 
							f.maxy = false ;
						else 
							f.maxy = points->isFaceWithValidPt(x, y + 1, z, MAXY);
		
						if(z == maxz ) 
							f.maxz = false ;
						else
							f.maxz = points->isFaceWithValidPt(x, y, z + 1, MAXZ);
					} catch (CException *e) {
						e->Delete();
						return POINTS_TO_POLYGONS_PROCESS + FILE_ACCESS_ERROR;
					}
					/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/	
					
					//generate the surfaces 
					try {
						drawSurfaces(x, y, z, cb, f );	
					} catch (CException *e) {
						e->Delete();
						return POINTS_TO_POLYGONS_PROCESS + FILE_ACCESS_ERROR;
					}


					//Set the indicator as valid in the cubes file
					setStatusAsValid(x, y);

					//Set the flag for the next iteration
					flgPrevValid = true  ;

				} else
					flgPrevValid = false ;
				
				//Report the status of work
				dataPtr->setNewPercent(uintPtsProcessed);
					
			}
		}
		if(z != maxz )
			cubes->goToNextPlane() ; //Reassign planes for new z index
		
	}   
	
	//All objects are drawn, so update the information of the last
	//triangle in the pols file
	try {
		pols->updateBeforeClose() ;
	} catch(CException *e) {
		e->Delete();
		return POINTS_TO_POLYGONS_PROCESS + FILE_ACCESS_ERROR;
	}

	if(flgFound)
		return SUCCESS; //true if some polygon were generated, else false
	else 
		return POINTS_TO_POLYGONS_PROCESS + NO_POLYGONS_GENERATED;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void pointsToPolygons::setStatusAsValid(UINT inX, UINT inY) {

	cubes->setStatusAsValid( inX  + 1 - minx, inY + 1 - miny);
}
///////////////////////////////////////////////////////////////////////////////////////////////
BOOL pointsToPolygons::isValidCube(UINT inX, UINT inY, BYTE inPlane) {

	return cubes->isValidCube( inX  + 1 - minx, inY + 1 - miny , inPlane ) ;
}

///////////////////////////////////////////////////////////////////////////////////////////////
CUBECORNERS pointsToPolygons::getNextXCube(CUBECORNERS inCb, UINT inX, UINT inY, UINT inZ) {

	CUBECORNERS outCb ;

	//Half of the points are to be retrieved from the file 
	outCb = points->getNextXFace(inX, inY, inZ) ;
	
	//Set the other four points of the cube using the values of the current cube
	outCb.backTopLeft      = inCb.backTopRight ;
	outCb.backBottomLeft   = inCb.backBottomRight ;
	outCb.frontTopLeft     = inCb.frontTopRight ;
	outCb.frontBottomLeft  = inCb.frontBottomRight ;

	return outCb ;
}

///////////////////////////////////////////////////////////////////////////////////////////////	

BOOL pointsToPolygons::isValidCube(CUBECORNERS inCb) {
	
	BYTE validPts = 0;
	
	if ( inCb.backTopLeft )
		validPts++;
	if ( inCb.backBottomLeft )
		validPts++;
	if ( inCb.backBottomRight )
		validPts++;
	if ( inCb.backTopRight )
		validPts++;
	if ( inCb.frontTopLeft )
		validPts++;
	if ( inCb.frontBottomLeft )
		validPts++;
	if ( inCb.frontBottomRight )
		validPts++;
	if ( inCb.frontTopRight )
		validPts++;
	
	if(validPts > 4 ) 
		return true;
	else if (validPts == 4 ) {
		//The cube can form a closed figure only if the four points do not lie on a plane
		if(	(  inCb.backTopLeft      && inCb.backBottomLeft 
			&& inCb.backBottomRight  && inCb.backTopRight     ) || //back face
			
			(  inCb.frontTopLeft     && inCb.frontBottomLeft 
			&& inCb.frontBottomRight && inCb.frontTopRight    ) || //front face
			
			(  inCb.backTopLeft      && inCb.backBottomLeft 
			&& inCb.frontTopLeft     && inCb.frontBottomLeft  ) || //left face
			
			(  inCb.backBottomRight  && inCb.backTopRight 
			&& inCb.frontBottomRight && inCb.frontTopRight    ) || //right face
			
			(  inCb.backTopLeft      && inCb.backTopRight 
			&& inCb.frontTopLeft     && inCb.frontTopRight    ) || //top face
			
			(  inCb.backBottomLeft   && inCb.backBottomRight 
			&& inCb.frontBottomLeft  && inCb.frontBottomRight ) || //bottom face 
			//diognal faces
			(  inCb.backTopLeft      && inCb.frontTopLeft 
			&& inCb.backBottomRight  && inCb.frontBottomRight ) || 
			
			(  inCb.backBottomLeft   && inCb.frontBottomLeft 
			&& inCb.backTopRight     && inCb.frontTopRight    ) || 
			
			(  inCb.frontTopLeft     && inCb.frontTopRight 
			&& inCb.backBottomLeft   && inCb.backBottomRight  ) || 
			
			(  inCb.frontBottomLeft  && inCb.frontBottomRight 
			&& inCb.backTopLeft      && inCb.backTopRight     ) || 
			
			(  inCb.frontTopLeft     && inCb.frontBottomLeft 
			&& inCb.backBottomRight  && inCb.backTopRight     ) || 
			
			(  inCb.frontBottomRight && inCb.frontTopRight 
			&& inCb.backTopLeft      && inCb.backBottomLeft ) )
			
			return false;

		else //4 points not coplanar
			
			return true; 

	} else //number of valid points are less than 4
		
		return false ; 
}

///////////////////////////////////////////////////////////////////////////////////////////////
void pointsToPolygons::drawSurfaces(UINT inX, UINT inY, UINT inZ, CUBECORNERS inCb, 
									FACES inSharedFaces) {
	
	/*--------------------------------Variable declarations---------------------------------*/
	CUBECORNERS t, temp ;

	BOOL flgCutCorner5, flgCutCorner6 ;

	BYTE ctr, ctrMinx, ctrMaxx, ctrMiny, ctrMaxy, ctrMinz, ctrMaxz ;
	BYTE ctrDgP1, ctrDgP2, ctrDgP3, ctrDgP4, ctrDgP5, ctrDgP6 ; //Diognal planes
	/*--------------------------------------------------------------------------------------*/
	
	
	//Initialize
	flgCutCorner5 = flgCutCorner6 = false ;	
	
	
	ctr = ctrMinx = ctrMaxx = ctrMiny = ctrMaxy = ctrMinz = ctrMaxz = 0 ;
	ctrDgP1 = ctrDgP2 = ctrDgP3 = ctrDgP4 = ctrDgP5 = ctrDgP6 = 0 ;

	//Calculate the number of overall valid points and valid points along different faces
	if(inCb.backBottomLeft) {
		ctrMinz++; ctrMiny++; ctrMinx++;  
		ctrDgP2++; ctrDgP4++; ctrDgP5++; ctr++; 
	}
	if(inCb.backBottomRight) {
		ctrMinz++; ctrMiny++; ctrMaxx++; 
		ctrDgP1++; ctrDgP3++; ctrDgP5++; ctr++; 
	} 
	if(inCb.backTopLeft) {
		ctrMinz++; ctrMaxy++; ctrMinx++; 
		ctrDgP2++; ctrDgP3++; ctrDgP6++; ctr++; 
	} 
	if(inCb.backTopRight) {
		ctrMinz++; ctrMaxy++; ctrMaxx++; 
		ctrDgP1++; ctrDgP4++; ctrDgP6++; ctr++; 
	} 
	if(inCb.frontBottomLeft) {
		ctrMaxz++; ctrMiny++; ctrMinx++; 
		ctrDgP1++; ctrDgP4++; ctrDgP6++; ctr++; 
	} 
	if(inCb.frontBottomRight) {
		ctrMaxz++; ctrMiny++; ctrMaxx++; 
		ctrDgP2++; ctrDgP3++; ctrDgP6++; ctr++; 
	} 
	if(inCb.frontTopLeft) {
		ctrMaxz++; ctrMaxy++; ctrMinx++; 
		ctrDgP1++; ctrDgP3++; ctrDgP5++; ctr++; 
	} 
	if(inCb.frontTopRight) {
		ctrMaxz++; ctrMaxy++; ctrMaxx++; 
		ctrDgP2++; ctrDgP4++; ctrDgP5++; ctr++; 
	}

	
	//first draw the possible rectangles or triangles on all the non shared faces
	
	//left face 
	if(!inSharedFaces.minx) { //Draw only if the face is not shared
		
		if(ctrMinx == 4 ) {
			t.initialize() ;
			t.backBottomLeft = t.backTopLeft = t.frontBottomLeft = true ;
			storeTriangle(inX, inY, inZ, inCb, t, MINX) ;
			
			t.initialize() ;
			t.frontBottomLeft = t.frontTopLeft = t.backTopLeft = true ;
			storeTriangle(inX, inY, inZ, inCb, t, MINX) ;

		} else if(ctrMinx == 3 ) { 
			t.initialize() ;
			
			t.backBottomLeft  = inCb.backBottomLeft;
			t.backTopLeft     = inCb.backTopLeft;
			t.frontBottomLeft = inCb.frontBottomLeft;
			t.frontTopLeft    = inCb.frontTopLeft;
			
			storeTriangle(inX, inY, inZ, inCb, t, MINX);

		}
	}
	
	//right face 
	if(!inSharedFaces.maxx) { //Draw only if the face is not shared

		if(ctrMaxx == 4 ) {
			t.initialize() ;
			t.backBottomRight = t.backTopRight = t.frontBottomRight = true;
			storeTriangle(inX, inY, inZ, inCb, t, MAXX);
			
			t.initialize();
			t.frontBottomRight = t.frontTopRight = t.backTopRight = true;
			storeTriangle(inX, inY, inZ, inCb, t, MAXX) ;
		
		} else if(ctrMaxx == 3 ) { 
			t.initialize();
			
			t.backBottomRight  = inCb.backBottomRight;
			t.backTopRight     = inCb.backTopRight;
			t.frontBottomRight = inCb.frontBottomRight;
			t.frontTopRight    = inCb.frontTopRight;
			

			storeTriangle(inX, inY, inZ, inCb, t, MAXX);

		}
	}
	
	//front face 
	if(!inSharedFaces.maxz) { //Draw only if the face is not shared
		
		if(ctrMaxz == 4 ) {

			t.initialize() ;
			t.frontBottomLeft = t.frontTopLeft = t.frontTopRight = true ;
			storeTriangle(inX, inY, inZ, inCb, t, MAXZ) ;
			
			t.initialize() ;
			t.frontBottomRight = t.frontTopRight = t.frontBottomLeft = true ;
			storeTriangle(inX, inY, inZ, inCb, t, MAXZ) ;
		
		} else if(ctrMaxz == 3 ) { 
			t.initialize() ;
			
			t.frontBottomLeft  = inCb.frontBottomLeft ;  
			t.frontBottomRight = inCb.frontBottomRight ;
			t.frontTopLeft     = inCb.frontTopLeft ;
			t.frontTopRight    = inCb.frontTopRight ;

			storeTriangle(inX, inY, inZ, inCb, t, MAXZ) ;

		}
	}
	
	//back face 
	if(!inSharedFaces.minz) { //Draw only if the face is not shared
		
		if(ctrMinz == 4 ) {
			t.initialize() ;
			t.backBottomLeft = t.backTopLeft = t.backTopRight = true ;
			storeTriangle(inX, inY, inZ, inCb, t, MINZ) ;
			
			t.initialize() ;
			t.backBottomRight = t.backTopRight = t.backBottomLeft = true ;
			storeTriangle(inX, inY, inZ, inCb, t, MINZ) ;
		
		} else if(ctrMinz == 3 ) { 
			t.initialize() ;

			t.backBottomLeft  = inCb.backBottomLeft ;  
			t.backBottomRight = inCb.backBottomRight ;
			t.backTopLeft     = inCb.backTopLeft ;
			t.backTopRight    = inCb.backTopRight ;
			
			storeTriangle(inX, inY, inZ, inCb, t, MINZ);

		}
	}

	//top face 
	if(!inSharedFaces.maxy) { //Draw only if the face is not shared
		
		if(ctrMaxy == 4 ) {
			t.initialize() ;
			t.frontTopLeft = t.frontTopRight = t.backTopLeft = true ;
			storeTriangle(inX, inY, inZ, inCb, t, MAXY);
			
			t.initialize() ;
			t.backTopLeft = t.backTopRight = t.frontTopRight = true ;
			storeTriangle(inX, inY, inZ, inCb, t, MAXY);
			
		} else if(ctrMaxy == 3 ) { 
			t.initialize() ;

			t.backTopLeft   = inCb.backTopLeft; 
			t.backTopRight  = inCb.backTopRight;
			t.frontTopLeft  = inCb.frontTopLeft;
			t.frontTopRight = inCb.frontTopRight;
			
			storeTriangle(inX, inY, inZ, inCb, t, MAXY) ;

		}
	}

	//bottom face 
	if(!inSharedFaces.miny) { //Draw only if the face is not shared
		
		if(ctrMiny == 4 ) {
			t.initialize() ;
			t.frontBottomLeft = t.frontBottomRight = t.backBottomLeft = true ;
			storeTriangle(inX, inY, inZ, inCb, t,MINY) ;
			
			t.initialize() ;
			t.backBottomLeft = t.backBottomRight = t.frontBottomRight = true ;
			storeTriangle(inX, inY, inZ, inCb, t, MINY) ;
			
		} else if(ctrMiny == 3) { 
			t.initialize() ;
			
			t.backBottomLeft   = inCb.backBottomLeft; 
			t.backBottomRight  = inCb.backBottomRight;
			t.frontBottomLeft  = inCb.frontBottomLeft;
			t.frontBottomRight = inCb.frontBottomRight;
			
			storeTriangle(inX, inY, inZ, inCb, t, MINY);

		}
	}
	
	
	switch(ctr) {
	case 8 :
		//All triangles along the faces which are drawn above
		break;
	case 7 :
		//All triangles along faces are drawn above
		//search for the invalid corner with all three valid neighbouring corners(done at the 
		//Bottom)	

		break;
	case 6 :
		//Look for any invalid side out of the possible 12 sides of the cube
		 if( ( !inCb.frontBottomRight && !inCb.frontTopRight   ) ||
				   ( !inCb.backBottomLeft   && !inCb.backTopLeft     ) ) {
					
			t.initialize(); t.backTopRight     = t.frontTopLeft   = t.frontBottomLeft  = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE1) ;
					
			t.initialize(); t.frontBottomLeft  = t.backTopRight   = t.backBottomRight  = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE1) ;

		} else if(( !inCb.frontBottomLeft  && !inCb.frontTopLeft   ) ||
				   ( !inCb.backBottomRight  && !inCb.backTopRight   ) ) {
						
			t.initialize(); t.backTopLeft      = t.frontTopRight  = t.backBottomLeft   = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE2) ;
					
			t.initialize(); t.backBottomLeft   = t.frontTopRight  = t.frontBottomRight = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE2) ;
		
		} else if( ( !inCb.frontBottomLeft  && !inCb.backBottomLeft  ) ||
			       ( !inCb.frontTopRight    && !inCb.backTopRight    ) ) {
					
			t.initialize(); t.frontTopLeft     = t.backTopLeft    = t.frontBottomRight = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE3);
					
			t.initialize(); t.frontBottomRight = t.backTopLeft    = t.backBottomRight  = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE3);
		
		} else if( ( !inCb.frontTopLeft     && !inCb.backTopLeft     ) ||
			       ( !inCb.frontBottomRight && !inCb.backBottomRight ) ) {
					
			t.initialize(); t.frontBottomLeft  = t.backBottomLeft = t.frontTopRight    = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE4) ;
					
			t.initialize(); t.frontTopRight    = t.backBottomLeft = t.backTopRight     = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE4) ;
	
		
		} else if( ( !inCb.frontBottomLeft  && !inCb.frontBottomRight) ||
			       ( !inCb.backTopLeft      && !inCb.backTopRight    ) ) {
					
			t.initialize(); t.frontTopLeft     = t.frontTopRight  = t.backBottomLeft   = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE5) ;
					
			t.initialize(); t.backBottomLeft   = t.frontTopRight  = t.backBottomRight  = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE5) ;


		} else if( ( !inCb.frontTopLeft     && !inCb.frontTopRight   ) ||
			       ( !inCb.backBottomLeft   && !inCb.backBottomRight ) ) {
					
			t.initialize(); t.backTopLeft      = t.backTopRight   = t.frontBottomRight = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE6) ;
					
			t.initialize(); t.frontBottomRight = t.backTopLeft    = t.frontBottomLeft  = true ;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE6) ;

		} else 
			//Identify the two invalid points and join the neighbours for each of them
			flgCutCorner6 = true ;
			
		break;
	case 5 :
		/*y
		The closed figure having five valid points can be categorized into two:
		One in which there are four coplanar points and a point away from this plane.
		In the second type we can have all five points as non-coplanar and in this case, each
		invalid point forms a side of the cube, with three points; all of which will be valid.
		*/
		//Check for four coplanar points along the twelve planes
		if( ctrMinx == 4 ) {
			temp.initialize() ;
			//assign the single point outside of the plane	
			temp.backBottomRight  = inCb.backBottomRight ;
			temp.backTopRight     = inCb.backTopRight ;
			temp.frontBottomRight = inCb.frontBottomRight ;
			temp.frontTopRight    = inCb.frontTopRight ;

			t = temp ;
			t.backTopLeft = t.backBottomLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backBottomLeft = t.frontBottomLeft = true ;

			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.frontBottomLeft = t.frontTopLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;
			t.frontTopLeft = t.backTopLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		} else if ( ctrMaxx == 4 ) {
			temp.initialize() ;
			//assign the single point outside of the plane
			temp.backBottomLeft  = inCb.backBottomLeft ;
			temp.backTopLeft     = inCb.backTopLeft ;
			temp.frontBottomLeft = inCb.frontBottomLeft ;
			temp.frontTopLeft    = inCb.frontTopLeft ;
				
			t = temp ;
			t.frontTopRight = t.frontBottomRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
						
			t = temp ;
			t.frontBottomRight = t.backBottomRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backBottomRight = t.backTopRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backTopRight = t.frontTopRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		} else if ( ctrMiny == 4 ) {
			temp.initialize() ;

			//assign the single point outside of the plane
			temp.backTopLeft   = inCb.backTopLeft ; 
			temp.backTopRight  = inCb.backTopRight ;
			temp.frontTopLeft  = inCb.frontTopLeft ;
			temp.frontTopRight = inCb.frontTopRight ;
			
			t = temp ;
			t.frontBottomLeft = t.backBottomLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backBottomLeft = t.backBottomRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
	
			t = temp ;
			t.backBottomRight = t.frontBottomRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.frontBottomRight = t.frontBottomLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		} else if ( ctrMaxy == 4 ) {
			temp.initialize() ;

			//assign the single point outside of the plane
			temp.backBottomLeft   = inCb.backBottomLeft ; 
			temp.backBottomRight  = inCb.backBottomRight ;
			temp.frontBottomLeft  = inCb.frontBottomLeft ;
			temp.frontBottomRight = inCb.frontBottomRight ;
			
			t.initialize() ;
			t = temp ;
			t.frontTopLeft = t.backTopLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backTopLeft = t.backTopRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backTopRight = t.frontTopRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.frontTopRight = t.frontTopLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		} else if ( ctrMinz == 4 ) {
			temp.initialize() ;

			//assign the single point outside of the plane
			temp.frontBottomLeft  = inCb.frontBottomLeft ;  
			temp.frontBottomRight = inCb.frontBottomRight ;
			temp.frontTopLeft     = inCb.frontTopLeft ;
			temp.frontTopRight    = inCb.frontTopRight ;
			
			t = temp ;
			t.backTopLeft = t.backBottomLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backBottomLeft = t.backBottomRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backBottomRight = t.backTopRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backTopRight = t.backTopLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		} else if ( ctrMaxz == 4 ) {
			temp.initialize() ;

			//assign the single point outside of the plane
			temp.backBottomLeft  = inCb.backBottomLeft ;  
			temp.backBottomRight = inCb.backBottomRight ;
			temp.backTopLeft     = inCb.backTopLeft ;
			temp.backTopRight    = inCb.backTopRight ;	
			
			t = temp ;
			t.frontTopLeft = t.frontBottomLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.frontBottomLeft = t.frontBottomRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.frontBottomRight = t.frontTopRight = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.frontTopRight = t.frontTopLeft = true ;
			
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		} else if ( ctrDgP1 == 4 ) {

			temp.initialize() ;
			//assign the single point outside of the plane
			temp.backTopLeft      =  inCb.backTopLeft  ;
			temp.backBottomLeft   =  inCb.backBottomLeft  ;
			temp.frontBottomRight =  inCb.frontBottomRight  ;
			temp.frontTopRight    =  inCb.frontTopRight  ;
			
			t = temp ;			
			t.frontTopLeft     = t.frontBottomLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;
			t.frontBottomLeft  = t.backBottomRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;			
			t.backBottomRight  = t.backTopRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;			
			t.backTopRight     = t.frontTopLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			//draw the plane

			t.initialize();
			t.frontBottomLeft = t.frontTopLeft = t.backTopRight = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE1);

			t.initialize();
			t.backTopRight = t.backBottomRight = t.frontBottomLeft = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE1);

		} else if ( ctrDgP2 == 4 ) {

			temp.initialize() ;
			//assign the single point outside of the plane
			temp.backBottomRight  =  inCb.backBottomRight  ;
			temp.backTopRight     =  inCb.backTopRight  ;
			temp.frontTopLeft     =  inCb.frontTopLeft  ;
			temp.frontBottomLeft  =  inCb.frontBottomLeft  ;
			
			t = temp ;
			t.backTopLeft      = t.backBottomLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backBottomLeft   = t.frontBottomRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;
			t.frontBottomRight = t.frontTopRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.frontTopRight    = t.backTopLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			//draw the plane
			t.initialize();
			t.backBottomLeft = t.backTopLeft = t.frontTopRight = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE2);
			
			t.initialize();
			t.frontTopRight = t.frontBottomRight = t.backBottomLeft = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE2);

			
		} else if ( ctrDgP3 == 4 ) {

			temp.initialize() ;
			//assign the single point outside of the plane
			temp.backBottomLeft   =  inCb.backBottomLeft  ;
			temp.backTopRight     =  inCb.backTopRight  ;
			temp.frontBottomLeft  =  inCb.frontBottomLeft  ;
			temp.frontTopRight    =  inCb.frontTopRight  ;
		
			t = temp ;
			t.backTopLeft      = t.frontTopLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;
			t.frontTopLeft     = t.frontBottomRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;
			t.frontBottomRight = t.backBottomRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backBottomRight  = t.backTopLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			//draw the plane
			t.initialize();
			t.frontTopLeft = t.backTopLeft = t.backBottomRight = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE3);
			
			t.initialize();
			t.backBottomRight = t.frontBottomRight = t.frontTopLeft = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE3);


		} else if ( ctrDgP4 == 4 ) {

			temp.initialize() ;
			//assign the single point outside of the plane
			temp.backTopLeft      =  inCb.backTopLeft  ;
			temp.backBottomRight  =  inCb.backBottomRight  ;
			temp.frontTopLeft     =  inCb.frontTopLeft  ;
			temp.frontBottomRight =  inCb.frontBottomRight  ;
			
			t = temp ;
			t.backBottomLeft  = t.frontBottomLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;			
			t.frontBottomLeft = t.frontTopRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.frontTopRight   = t.backTopRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;
			t.backTopRight    = t.backBottomLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			//draw the plane
			t.initialize();
			t.backBottomLeft = t.backTopRight = t.frontTopRight = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE4);
			
			t.initialize();
			t.frontTopRight = t.frontBottomLeft = t.backBottomLeft = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE4);

			
		} else if ( ctrDgP5 == 4 ) {
			
			temp.initialize() ;
			//assign the single point outside of the plane
			temp.backTopLeft      =  inCb.backTopLeft  ;
			temp.backTopRight     =  inCb.backTopRight  ;
			temp.frontBottomLeft  =  inCb.frontBottomLeft  ;
			temp.frontBottomRight =  inCb.frontBottomRight  ;
			
			t = temp ;
			t.frontTopLeft    = t.frontTopRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;			
			t.frontTopRight   = t.backBottomRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;			
			t.backBottomRight = t.backBottomLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;			
			t.backBottomLeft  = t.frontTopLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			//draw the plane
			t.initialize();
			t.frontTopLeft = t.frontTopRight = t.backBottomRight = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE5);
			
			t.initialize();
			t.backBottomRight = t.backBottomLeft = t.frontTopLeft = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE5);
			
		} else if ( ctrDgP6 == 4 ) {
			
			temp.initialize() ;
			//assign the single point outside of the plane
			temp.backBottomLeft   =  inCb.backBottomLeft  ;
			temp.backBottomRight  =  inCb.backBottomRight  ;
			temp.frontTopLeft     =  inCb.frontTopLeft  ;
			temp.frontTopRight    =  inCb.frontTopRight  ;
			
			t = temp ;			
			t.backTopLeft      = t.backTopRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			t = temp ;			
			t.backTopRight     = t.frontBottomRight = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;			
			t.frontBottomRight = t.frontBottomLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;
			
			t = temp ;			
			t.frontBottomLeft  = t.backTopLeft = true ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

			//draw the plane
			t.initialize();
			t.backTopLeft = t.backTopRight = t.frontBottomRight = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE6);
			
			t.initialize();
			t.frontBottomRight = t.frontBottomLeft = t.backTopLeft = true;
			storeTriangle(inX, inY, inZ, inCb, t, DGPLANE6);
			
		} else 
			flgCutCorner5 = true ;


		break;
	case 4 :
			
		//Logic: There are four points and all can form a group with the other three to form 
		//triangles. So if a corner is valid, we set it to invalid, so that only three points 
		//are left in the set of cube corners passed for making the triangle. We do this for 
		//all the triangles

		if(inCb.backBottomLeft) {
			t = inCb ;
			t.backBottomLeft = false ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, OF_FOUR) ;
			
		}
		if(inCb.backBottomRight) {
			t = inCb ;
			t.backBottomRight = false ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, OF_FOUR) ;
		}
		if(inCb.backTopLeft) {
			t = inCb ;
			t.backTopLeft = false ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, OF_FOUR) ;
		}
		if(inCb.backTopRight) {
			t = inCb ;
			t.backTopRight = false ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, OF_FOUR) ;
		}
		if(inCb.frontBottomLeft) {
			t = inCb ;
			t.frontBottomLeft = false ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, OF_FOUR) ;
		}
		if(inCb.frontBottomRight) {
			t = inCb ;
			t.frontBottomRight = false ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, OF_FOUR) ;
		}
		if(inCb.frontTopLeft) {
			t = inCb ;
			t.frontTopLeft = false ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, OF_FOUR) ;	
		}
		if(inCb.frontTopRight) {
			t = inCb ;
			t.frontTopRight = false ;
			if(!t.triangleOnCubeFace() ) 
				storeTriangle(inX, inY, inZ, inCb, t, OF_FOUR) ;
		}
		
		break;
	}
	//Draw triangles for invalid corners connected to all three valid neighbours
	if( ctr == 7 || flgCutCorner5 || flgCutCorner6 ) {
		
		if(!inCb.backBottomLeft) {
			
			t.initialize() ;
			t.frontBottomLeft = t.backBottomRight = t.backTopLeft   = true ;	
			storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		}
		if(!inCb.backBottomRight) {
			
			t.initialize() ;
			t.frontBottomRight = t.backBottomLeft = t.backTopRight  = true ;
			storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		}
		if(!inCb.backTopLeft ) {
			
			t.initialize() ;
			t.backBottomLeft = t.frontTopLeft = t.backTopRight      = true ;
			storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		}if(!inCb.backTopRight ) {
			
			t.initialize() ;
			t.backBottomRight = t.frontTopRight = t.backTopLeft     = true ;
			storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		}
		if(!inCb.frontBottomLeft ) {
			
			t.initialize() ;
			t.frontBottomRight = t.frontTopLeft = t.backBottomLeft  = true ;
			storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		}
		if(!inCb.frontBottomRight ) {
			
			t.initialize() ;
			t.frontBottomLeft = t.backBottomRight = t.frontTopRight = true ;
			storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		}
		if(!inCb.frontTopLeft ) {
			
			t.initialize() ;
			t.frontBottomLeft = t.frontTopRight = t.backTopLeft     = true ;
			storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		}
		if(!inCb.frontTopRight ) {
			
			t.initialize() ;
			t.frontTopLeft = t.frontBottomRight = t.backTopRight    = true ;
			storeTriangle(inX, inY, inZ, inCb, t, INNER) ;

		}
	}	 

}	

///////////////////////////////////////////////////////////////////////////////////////////////
void pointsToPolygons::storeTriangle(UINT inX, UINT inY, UINT inZ, CUBECORNERS inCorners,  
									 CUBECORNERS inVertices, BYTE inFace ) {

	//We need to decide which side should the normal to the plane
	//containing the triangle should face. The normal points in the
	//direction in which the vertices are stored in anticlockwise sequence.
	//Hence in effect we need to decide the sequence, in which the vertices are to be stored.

	BYTE ctrVertices, temp, vertices[3] ;
	BOOL flgCCW ;
	BOOL flgXZPlane;
	BYTE validPts, ctrDgP1, ctrDgP2, ctrDgP3, ctrDgP4, ctrDgP5, ctrDgP6;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
	//          Check whether only three vertices have been sent for the triangle.          //
	validPts = 0;
	
	if ( inVertices.backTopLeft )
		validPts++;
	if ( inVertices.backBottomLeft )
		validPts++;
	if ( inVertices.backBottomRight )
		validPts++;
	if ( inVertices.backTopRight )
		validPts++;
	if ( inVertices.frontTopLeft )
		validPts++;
	if ( inVertices.frontBottomLeft )
		validPts++;
	if ( inVertices.frontBottomRight )
		validPts++;
	if ( inVertices.frontTopRight )
		validPts++;
	
	if(validPts != 3 ) 
		throw VERTICES_NOT_THREE_FOR_TRIANGLE ;

	//Note : This piece of code can be removed later on in the release version.
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

	//If the defined face is set as OF_FOUR then check for triangles along the diognal faces
	if(inFace == OF_FOUR) {
	
		ctrDgP1 = ctrDgP2 = ctrDgP3 = ctrDgP4 = ctrDgP5 = ctrDgP6 = 0;	
	
		if(inVertices.backBottomLeft) {
			ctrDgP2++; ctrDgP4++; ctrDgP5++;
		}

		if(inVertices.backBottomRight) {
			ctrDgP1++; ctrDgP3++; ctrDgP5++;
		}
	
		if(inVertices.backTopLeft) {
			ctrDgP2++; ctrDgP3++; ctrDgP6++;
		}
	
		if(inVertices.backTopRight) {
			ctrDgP1++; ctrDgP4++; ctrDgP6++;
		}
	
		if(inVertices.frontBottomLeft) {
			ctrDgP1++; ctrDgP4++; ctrDgP6++;
		}
	 
		if(inVertices.frontBottomRight) {
			ctrDgP2++; ctrDgP3++; ctrDgP6++;
		}
	
		if(inVertices.frontTopLeft) {
			ctrDgP1++; ctrDgP3++; ctrDgP5++;
		}
	 
		if(inVertices.frontTopRight) {
			ctrDgP2++; ctrDgP4++; ctrDgP5++;
		}

		if(ctrDgP1 == 3) 
			inFace = DGPLANE1;

		else if(ctrDgP2 == 3) 
			inFace = DGPLANE2;

		else if(ctrDgP3 == 3) 
			inFace = DGPLANE3;

		else if(ctrDgP4 == 3) 
			inFace = DGPLANE4;

		else if(ctrDgP5 == 3) 
			inFace = DGPLANE5;

		else if(ctrDgP6 == 3) 
			inFace = DGPLANE6;
	}
	
	
	//Case 1: If the points are along the left plane of the cube
	if(inFace == MINX) {
		ctrVertices = 0 ;
		if( inVertices.backTopLeft ) {
			vertices[ctrVertices] = BACK_TOP_LEFT ;
			ctrVertices++;
		}
		if( inVertices.backBottomLeft ) {
			vertices[ctrVertices] = BACK_BOTTOM_LEFT;
			ctrVertices++;
		}
		if( inVertices.frontBottomLeft ) {
			vertices[ctrVertices] = FRONT_BOTTOM_LEFT;
			ctrVertices++;
		}
		if( inVertices.frontTopLeft ) {
			vertices[ctrVertices] = FRONT_TOP_LEFT;
			ctrVertices++;
		}
		//store the vertices in the CCW order for the left face
		storeTriangleToFile(inX, inY, inZ, vertices);
		return;
	} //End of Case 1		
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	//Case 2: If the points are along the right face of the cube
	if(inFace == MAXX) {
		ctrVertices = 0 ;
		if( inVertices.frontTopRight ) {
			vertices[ctrVertices] = FRONT_TOP_RIGHT ;
			ctrVertices++;
		}
		if( inVertices.frontBottomRight ) {
			vertices[ctrVertices] = FRONT_BOTTOM_RIGHT ;
			ctrVertices++;
		}	
		if( inVertices.backBottomRight ) {
			vertices[ctrVertices] = BACK_BOTTOM_RIGHT ;
			ctrVertices++;
		}
		if( inVertices.backTopRight ) {
			vertices[ctrVertices] = BACK_TOP_RIGHT ;
			ctrVertices++;
		}	
	
		//Store the vertices in the CCW order for the right face
		storeTriangleToFile(inX, inY, inZ, vertices);
		return;
	} //End of Case 2
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	//Case 3: If the points are along the back face of the cube
	if(inFace == MINZ) {
		ctrVertices = 0 ;
		if( inVertices.backTopLeft ) {
			vertices[ctrVertices] = BACK_TOP_LEFT;
			ctrVertices++;
		}
		if( inVertices.backTopRight ) {
			vertices[ctrVertices] = BACK_TOP_RIGHT;
			ctrVertices++;
		}	
		if( inVertices.backBottomRight ) {
			vertices[ctrVertices] = BACK_BOTTOM_RIGHT;
			ctrVertices++;
		}
		if( inVertices.backBottomLeft) {
			vertices[ctrVertices] = BACK_BOTTOM_LEFT;
			ctrVertices++;
		}	

		//Store the vertices in the CCW order
		storeTriangleToFile(inX, inY, inZ, vertices);
		return;
	} //End of Case 3
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	//Case 4: If the points are along the front face of the cube
	if(inFace == MAXZ) {
		ctrVertices = 0 ;
		if( inVertices.frontTopLeft ) {
			vertices[ctrVertices] = FRONT_TOP_LEFT;
			ctrVertices++;
		}
		if( inVertices.frontBottomLeft ) {
			vertices[ctrVertices] = FRONT_BOTTOM_LEFT;
			ctrVertices++;
		}	
		if( inVertices.frontBottomRight ) {
			vertices[ctrVertices] = FRONT_BOTTOM_RIGHT;
			ctrVertices++;
		}
		if( inVertices.frontTopRight ) {
			vertices[ctrVertices] = FRONT_TOP_RIGHT;
			ctrVertices++;
		}	
	
		//Store the vertices in the CCW order
		storeTriangleToFile(inX, inY, inZ, vertices);
		return;
	} //End of Case 4
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	//Case 5: If the points are along the bottom face of the cube
	if(inFace == MINY) {
		ctrVertices = 0 ;
		if(inVertices.backBottomLeft) {
			vertices[ctrVertices] = BACK_BOTTOM_LEFT;
			ctrVertices++;
		}
		if(inVertices.backBottomRight) {
			vertices[ctrVertices] = BACK_BOTTOM_RIGHT;
			ctrVertices++;
		}	
		if(inVertices.frontBottomRight) {
			vertices[ctrVertices] = FRONT_BOTTOM_RIGHT;
			ctrVertices++;
		}
		if(inVertices.frontBottomLeft) {
			vertices[ctrVertices] = FRONT_BOTTOM_LEFT;
			ctrVertices++;
		}	

		//Store the vertices in the CCW order
		storeTriangleToFile(inX, inY, inZ, vertices);
		return;
	} //End of Case 5
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	//Case 6: If the points are along the top face of the cube
	if(inFace == MAXY) {
		ctrVertices = 0 ;
		if(inVertices.backTopLeft) {
			vertices[ctrVertices] = BACK_TOP_LEFT;
			ctrVertices++;
		}
		if(inVertices.frontTopLeft) {
			vertices[ctrVertices] = FRONT_TOP_LEFT;
			ctrVertices++;
		}	
		if(inVertices.frontTopRight) {
			vertices[ctrVertices] = FRONT_TOP_RIGHT;
			ctrVertices++;
		}
		if(inVertices.backTopRight) {
			vertices[ctrVertices] = BACK_TOP_RIGHT;
			ctrVertices++;
		}	

		//Store the vertices in the CCW order
		storeTriangleToFile(inX, inY, inZ, vertices);
		return;
	} //End of Case 6
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	//Case 7: Triangle cutting a corner   
	//There can be eight such triangles
	
	//Case i:------------------------------------------------------------------------------
	//Around backTopLeft
	if(inVertices.backBottomLeft && inVertices.frontTopLeft && inVertices.backTopRight ) {
		
		if(!inCorners.backTopLeft ) {
			//anti-clockwise when viewed from the corner
			vertices[0] = BACK_BOTTOM_LEFT;  
			vertices[1] = FRONT_TOP_LEFT;  
			vertices[2] = BACK_TOP_RIGHT; 
			                                                                      
		} else {
			//clockwise
			vertices[0] = FRONT_TOP_LEFT;  
			vertices[1] = BACK_BOTTOM_LEFT;  
			vertices[2] = BACK_TOP_RIGHT; 
		}
		storeTriangleToFile(inX, inY, inZ, vertices);
		return;
	} 

	//Case ii:------------------------------------------------------------------------------
	//Around backBottomLeft
	if(inVertices.frontBottomLeft && inVertices.backTopLeft && inVertices.backBottomRight ) {
		
		if(!inCorners.backBottomLeft ) {
			//anti-clockwise when viewed from the corner
			vertices[0] =  FRONT_BOTTOM_LEFT;  
			vertices[1] =  BACK_TOP_LEFT    ;  
			vertices[2] =  BACK_BOTTOM_RIGHT; 
			                                                                      
		} else {
			//clockwise
			vertices[0] =  BACK_TOP_LEFT    ;  
			vertices[1] =  FRONT_BOTTOM_LEFT;  
			vertices[2] =  BACK_BOTTOM_RIGHT; 
			
		}
		storeTriangleToFile(inX, inY, inZ, vertices);
		return ;
	} 

	//Case iii:-----------------------------------------------------------------------------
	//Around backBottomRight
	if(inVertices.backBottomLeft && inVertices.backTopRight && inVertices.frontBottomRight ) {
		
		if(!inCorners.backBottomRight ) {
			//anti-clockwise when viewed from the corner
			vertices[0] =  BACK_BOTTOM_LEFT  ;  
			vertices[1] =  BACK_TOP_RIGHT    ;  
			vertices[2] =  FRONT_BOTTOM_RIGHT; 
			                                                                      
		} else {
			//clockwise
			vertices[0] =  BACK_TOP_RIGHT    ;  
			vertices[1] =  BACK_BOTTOM_LEFT  ;  
			vertices[2] =  FRONT_BOTTOM_RIGHT; 
		}
		storeTriangleToFile(inX, inY, inZ, vertices);
		return ;
	} 

	//Case iv:------------------------------------------------------------------------------
	//Around backTopRight
	if(inVertices.backTopLeft && inVertices.frontTopRight && inVertices.backBottomRight )  {
		
		if(!inCorners.backTopRight ) {
			//anti-clockwise when viewed from the corner
			vertices[0] =  BACK_TOP_LEFT    ;  
			vertices[1] =  FRONT_TOP_RIGHT  ;  
			vertices[2] =  BACK_BOTTOM_RIGHT; 
			                                                                      
		} else {
			//clockwise
			vertices[0] =  FRONT_TOP_RIGHT  ;  
			vertices[1] =  BACK_TOP_LEFT    ;  
			vertices[2] =  BACK_BOTTOM_RIGHT; 
			
		}
		storeTriangleToFile(inX, inY, inZ, vertices);
		return ;
	} 

	//Case v:-------------------------------------------------------------------------------
	//Around frontTopLeft
	if(inVertices.backTopLeft && inVertices.frontBottomLeft && inVertices.frontTopRight ) {
		
		if(!inCorners.frontTopLeft ) {
			//anti-clockwise when viewed from the corner
			vertices[0] =  BACK_TOP_LEFT    ;  
			vertices[1] =  FRONT_BOTTOM_LEFT;  
			vertices[2] =  FRONT_TOP_RIGHT  ; 
			                                                                      
		} else {
			//clockwise
			vertices[0] =  FRONT_BOTTOM_LEFT;  
			vertices[1] =  BACK_TOP_LEFT    ;  
			vertices[2] =  FRONT_TOP_RIGHT  ; 
			
		}
		storeTriangleToFile(inX, inY, inZ, vertices);
		return ;
	} 

	//Case vi:------------------------------------------------------------------------------
	//Around frontBottomLeft
	if(inVertices.frontBottomRight && inVertices.frontTopLeft && inVertices.backBottomLeft ) {
		
		if(!inCorners.frontBottomLeft ) {
			//anti-clockwise when viewed from the corner
			vertices[0] =  FRONT_BOTTOM_RIGHT;  
			vertices[1] =  FRONT_TOP_LEFT    ;  
			vertices[2] =  BACK_BOTTOM_LEFT  ; 
			                                                                      
		} else {
			//clockwise
			vertices[0] =  FRONT_TOP_LEFT    ;  
			vertices[1] =  FRONT_BOTTOM_RIGHT;  
			vertices[2] =  BACK_BOTTOM_LEFT  ; 
		}
		storeTriangleToFile(inX, inY, inZ, vertices);
		return ;
	} 

	//Case vii:-----------------------------------------------------------------------------
	//Around frontBottomRight
	if(inVertices.backBottomRight && inVertices.frontTopRight && inVertices.frontBottomLeft) {
		
		if(!inCorners.frontBottomRight ) {
			//anti-clockwise when viewed from the corner
			vertices[0] =  BACK_BOTTOM_RIGHT;  
			vertices[1] =  FRONT_TOP_RIGHT  ;  
			vertices[2] =  FRONT_BOTTOM_LEFT; 
			                                                                      
		} else {
			//clockwise
			vertices[0] =  FRONT_TOP_RIGHT  ;  
			vertices[1] =  BACK_BOTTOM_RIGHT;  
			vertices[2] =  FRONT_BOTTOM_LEFT; 
			
		}
		storeTriangleToFile(inX, inY, inZ, vertices);
		return ;
	} 
	
	//Case viii:----------------------------------------------------------------------------
	//Around frontTopRight
	if(inVertices.frontBottomRight && inVertices.backTopRight && inVertices.frontTopLeft ) {
		
		if(!inCorners.frontTopRight ) {
			//anti-clockwise when viewed from the corner
			vertices[0] = FRONT_BOTTOM_RIGHT ;  
			vertices[1] = BACK_TOP_RIGHT     ;  
			vertices[2] = FRONT_TOP_LEFT     ; 
			                                                                      
		} else {
			//clockwise
			vertices[0] = BACK_TOP_RIGHT     ;  
			vertices[1] = FRONT_BOTTOM_RIGHT ;  
			vertices[2] = FRONT_TOP_LEFT     ; 
			
		}
		storeTriangleToFile(inX, inY, inZ, vertices);
		return ;
	} 

	
	//End of case 7

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	//Case 8: If the triangle has a component in the x and
	//        z directions only.
	//Check whether all the points are along the two possible diognal planes 
	flgXZPlane = false;
	ctrVertices = 0;
	if(inVertices.backBottomLeft) 
		ctrVertices++;
	if(inVertices.backTopLeft) 
		ctrVertices++;
	if(inVertices.frontTopRight) 
		ctrVertices++;
	if(inVertices.frontBottomRight) 
		ctrVertices++;
	if(ctrVertices == 3) 
		flgXZPlane = true;
	else {

		ctrVertices = 0;
		if(inVertices.backBottomRight) 
			ctrVertices++;
		if(inVertices.backTopRight) 
			ctrVertices++;
		if(inVertices.frontTopLeft) 
			ctrVertices++;
		if(inVertices.frontBottomLeft) 
			ctrVertices++;
		if(ctrVertices == 3) 
			flgXZPlane = true;

	}		

	if( flgXZPlane) {
		//Decide whether the vertices should be in CW or CCW direction
		
		//Logic: If there's a valid point in front of any of the points of the given triangle
		//then the triangle is not facing the front and hence the vertices should be aligned 
		//in CW order, else the triangle is facing the front and hence the vertices should be
		//aligned in CCW order when looking from the front
		
		if(inFace == DGPLANE1) {

			if(inCorners.frontBottomRight || inCorners.frontTopRight) 
				flgCCW = false;
			else
				flgCCW = true;
		
		} else if(inFace == DGPLANE2) {

			if(inCorners.frontBottomLeft || inCorners.frontTopLeft) 
				flgCCW = false;
			else
				flgCCW = true;		
		
		} else {

			if( (inVertices.backTopLeft     && inCorners.frontTopLeft    ) ||
				(inVertices.backTopRight    && inCorners.frontTopRight   ) ||
				(inVertices.backBottomLeft  && inCorners.frontBottomLeft ) ||
				(inVertices.backBottomRight && inCorners.frontBottomRight))
				flgCCW = false;//Store the vertices in the CW order looking from front
			else 
				flgCCW = true; //Store the vertices in the CCW order looking from front
		}

		//Intially arrange the vertices in CCW direction
		ctrVertices = 0;
		
		//First priority to top left corner
		if(inVertices.backTopLeft) {
			vertices[ctrVertices] = BACK_TOP_LEFT ;
			ctrVertices++ ;
		} else if(inVertices.frontTopLeft ) {
			vertices[ctrVertices] = FRONT_TOP_LEFT ;
			ctrVertices++ ;
		}

		//Second priority to bottom left corner
		if(inVertices.backBottomLeft) {
			vertices[ctrVertices] = BACK_BOTTOM_LEFT ;
			ctrVertices++ ;
		} else if(inVertices.frontBottomLeft ) {
			vertices[ctrVertices] = FRONT_BOTTOM_LEFT ;
			ctrVertices++ ;
		}

		//Third priority to bottom right corner
		if(inVertices.backBottomRight) {
			vertices[ctrVertices] = BACK_BOTTOM_RIGHT ;
			ctrVertices++ ;
		} else if(inVertices.frontBottomRight ) {
			vertices[ctrVertices] = FRONT_BOTTOM_RIGHT;
			ctrVertices++ ;
		}

		//Last priority to top right corner
		if( ctrVertices != 3 ) {
			if(inVertices.backTopRight) {
				vertices[ctrVertices] = BACK_TOP_RIGHT ;
				
			} else if(inVertices.frontTopRight ) {
				vertices[ctrVertices] = FRONT_TOP_RIGHT ;
				
			}
		}

		if(!flgCCW) {
			//Swap two vertices 
			temp = vertices[1] ;
			vertices[1] = vertices[2];
			vertices[2] = temp ;
		}

		storeTriangleToFile(inX, inY, inZ, vertices) ;

		return ;
		
	} //End of Case 8	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	//Case 9: 
	//If all the previous cases are not satisfied, then this is the default case 
	
	//Decide whether the vertices should be in CW or CCW direction

	//Logic: If there's a valid point on top of any of the points of the given triangle
	//then the triangle is not facing the top and hence the vertices should be aligned 
	//in CW order, else the triangle is facing the top and hence the vertices should be
	//aligned in CCW order
	if(inFace == DGPLANE3) {

		if(inCorners.frontTopRight || inCorners.backTopRight)
			flgCCW = false;
		else
			flgCCW = true;

	} else if(inFace == DGPLANE4) {

		if(inCorners.frontTopLeft || inCorners.backTopLeft)
			flgCCW = false;
		else
			flgCCW = true;

	} else if(inFace == DGPLANE5) {

		if(inCorners.backTopLeft || inCorners.backTopRight)
			flgCCW = false;
		else
			flgCCW = true;

	} else if(inFace == DGPLANE6) {

		if(inCorners.frontTopLeft || inCorners.frontTopRight)
			flgCCW = false;
		else
			flgCCW = true;

	} else {

		if( (inVertices.backBottomLeft   && inCorners.backTopLeft  ) ||
			(inVertices.backBottomRight  && inCorners.backTopRight ) ||
			(inVertices.frontBottomLeft  && inCorners.frontTopLeft ) ||
			(inVertices.frontBottomRight && inCorners.frontTopRight) ) 
			flgCCW = false ;//Store the vertices in the CW order looking from top

		else 
			flgCCW = true ;//Store the vertices in the CCW order looking from top
	}

	//Intially arrange the vertices in CCW direction
	ctrVertices = 0 ;
	
	//First priority to back left corner
	if (inVertices.backTopLeft) {
		vertices[ctrVertices] = BACK_TOP_LEFT ;
		ctrVertices++ ;
	} else if(inVertices.backBottomLeft ) {
		vertices[ctrVertices] = BACK_BOTTOM_LEFT ;
		ctrVertices++ ;
	}

	//Next priority to front left corner
	if (inVertices.frontTopLeft) {
		vertices[ctrVertices] = FRONT_TOP_LEFT ;
		ctrVertices++ ;
	} else if(inVertices.frontBottomLeft ) {
		vertices[ctrVertices] = FRONT_BOTTOM_LEFT ;
		ctrVertices++ ;
	}

	//Next priority to front right corner
	if (inVertices.frontTopRight) {
		vertices[ctrVertices] = FRONT_TOP_RIGHT ;
		ctrVertices++ ;
	} else if(inVertices.frontBottomRight ) {
		vertices[ctrVertices] = FRONT_BOTTOM_RIGHT ;
		ctrVertices++ ;
	}

	//Last priority to back right corner
	if(ctrVertices !=3 ) {
		if (inVertices.backTopRight) {
			vertices[ctrVertices] = BACK_TOP_RIGHT ;
			
		} else if(inVertices.backBottomRight ) {
			vertices[ctrVertices] = BACK_BOTTOM_RIGHT ;
			
		}
	}
	
	if(!flgCCW) {
			//Swap two vertices 
			temp = vertices[1] ;
			vertices[1] = vertices[2];
			vertices[2] = temp ;
	}		
	
	storeTriangleToFile(inX, inY, inZ, vertices) ;
	return ;
		
	//End of Case 9
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
}

///////////////////////////////////////////////////////////////////////////////////////////////
void pointsToPolygons::storeTriangleToFile(UINT inX, UINT inY, UINT inZ, BYTE inVertices[]) {
	BYTE ctr;
	FLTPOINT3D P1, P2, P3 ;

	float initX, initY, initZ ;

	FLTPOINT3D *ptr ;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	VERIFY(inVertices[0] == BACK_TOP_LEFT      || inVertices[0] == BACK_BOTTOM_LEFT 
		|| inVertices[0] == BACK_BOTTOM_RIGHT  || inVertices[0] == BACK_TOP_RIGHT 
		|| inVertices[0] == FRONT_TOP_LEFT     || inVertices[0] == FRONT_BOTTOM_LEFT 
		|| inVertices[0] == FRONT_BOTTOM_RIGHT || inVertices[0] == FRONT_TOP_RIGHT ) ;

	VERIFY(inVertices[1] == BACK_TOP_LEFT      || inVertices[1] == BACK_BOTTOM_LEFT 
		|| inVertices[1] == BACK_BOTTOM_RIGHT  || inVertices[1] == BACK_TOP_RIGHT 
		|| inVertices[1] == FRONT_TOP_LEFT     || inVertices[1] == FRONT_BOTTOM_LEFT 
		|| inVertices[1] == FRONT_BOTTOM_RIGHT || inVertices[1] == FRONT_TOP_RIGHT ) ;

	VERIFY(inVertices[2] == BACK_TOP_LEFT      || inVertices[2] == BACK_BOTTOM_LEFT 
		|| inVertices[2] == BACK_BOTTOM_RIGHT  || inVertices[2] == BACK_TOP_RIGHT 
		|| inVertices[2] == FRONT_TOP_LEFT     || inVertices[2] == FRONT_BOTTOM_LEFT 
		|| inVertices[2] == FRONT_BOTTOM_RIGHT || inVertices[2] == FRONT_TOP_RIGHT ) ;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	//calculate the position of the BACK_BOTTOM_LEFT point for the cube
	initX = cbInfo.minx + xSpan * (float) (inX - 1);
	initY = cbInfo.miny + ySpan * (float) (inY - 1);
	initZ = cbInfo.minz + zSpan * (float) (inZ - 1);

		
	for( ctr = 0 ; ctr <= 2; ctr++) {
		
		//decide which point's value has to be set
		switch(ctr) {
		case 0 :
			ptr = &P1;
			break;
		case 1:
			ptr = &P2;
			break;
		case 2: 
			ptr = &P3;
			break;
		}

		switch(inVertices[ctr]) {
		case BACK_TOP_LEFT :
			ptr->x = initX ;
			ptr->y = initY + ySpan;
			ptr->z = initZ ;
			break ;
		case BACK_BOTTOM_LEFT :
			ptr->x = initX ;
			ptr->y = initY ;
			ptr->z = initZ ;
			break ;  
		case BACK_BOTTOM_RIGHT :
			ptr->x = initX + xSpan;
			ptr->y = initY ;
			ptr->z = initZ ;
			break ; 
		case BACK_TOP_RIGHT :
			ptr->x = initX + xSpan;
			ptr->y = initY + ySpan;
			ptr->z = initZ ;
			break ;    
		case FRONT_TOP_LEFT :
			ptr->x = initX ;
			ptr->y = initY + ySpan;
			ptr->z = initZ + zSpan; 
			break ;     
		case FRONT_BOTTOM_LEFT :
			ptr->x = initX ;
			ptr->y = initY ;
			ptr->z = initZ + zSpan;
			break ;  
		case FRONT_BOTTOM_RIGHT : 
			ptr->x = initX + xSpan;
			ptr->y = initY ;
			ptr->z = initZ + zSpan;
			break ; 
		case FRONT_TOP_RIGHT : 
			ptr->x = initX + xSpan ;
			ptr->y = initY + ySpan ;
			ptr->z = initZ + zSpan ;
			break ; 
		default:
			//Raise an error
			break;
		}
	}//end of for loop for the three points
	
	//Add the triangle
	pols->storeTriangle(P1, P2, P3) ;

}

///////////////////////////////////////////////////////////////////////////////////////////////	

pointsToPolygons::~pointsToPolygons() {
	if(pols    != NULL)
		delete pols;
	if(cubes   != NULL)
		delete cubes;
	if(points  != NULL)
		delete points;
}

///////////////////////////////////////////////////////////////////////////////////////////////