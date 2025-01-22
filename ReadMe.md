The PointsToPolygons repo contains the source code of the ptstopolygons.dll that the IBModeler 2.0 application uses to convert a cloud of points in 3D space to a 3D model. It creates triangular meshes defining the complete surface of the model, deriving it from the original cloud of points in the 3D space.

Point data in 3D space can grow exponentially when you multiply X axes times Y axes times Z. So this library does a lot of optimizations to store the data in the most optimal way. Bits are used instead of a byte for boolean data. Intelligent swapping between in memory data and file storage data ensures the best use of storage space while also keeping it performant by reducing the number of reads and writes to the file system.

========================================================================
MICROSOFT FOUNDATION CLASS LIBRARY : ptstopolygons
========================================================================

AppWizard has created this ptstopolygons DLL for you. This DLL not only
demonstrates the basics of using the Microsoft Foundation classes but
is also a starting point for writing your DLL.

This file contains a summary of what you will find in each of the files that
make up your ptstopolygons DLL.

ptstopolygons.dsp
This file (the project file) contains information at the project level and
is used to build a single project or subproject. Other users can share the
project (.dsp) file, but they should export the makefiles locally.

ptstopolygons.cpp
This is the main DLL source file that contains the definition of
DllMain().

ptstopolygons.rc
This is a listing of all of the Microsoft Windows resources that the
program uses. It includes the icons, bitmaps, and cursors that are stored
in the RES subdirectory. This file can be directly edited in Microsoft
Visual C++.

ptstopolygons.clw
This file contains information used by ClassWizard to edit existing
classes or add new classes. ClassWizard also uses this file to store
information needed to create and edit message maps and dialog data
maps and to create prototype member functions.

res\ptstopolygons.rc2
This file contains resources that are not edited by Microsoft
Visual C++. You should place all resources not editable by
the resource editor in this file.

ptstopolygons.def
This file contains information about the DLL that must be
provided to run with Microsoft Windows. It defines parameters
such as the name and description of the DLL. It also exports
functions from the DLL.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
These files are used to build a precompiled header (PCH) file
named ptstopolygons.pch and a precompiled types file named StdAfx.obj.

Resource.h
This is the standard header file, which defines new resource IDs.
Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
