﻿//***************************************************************************************
// 
// File name: 3dmodmodelfinder.cpp  
//
// Synopsis: This example demonstrates how to use rectangle and box finders to define
//           models and search for them in 3D point clouds. A simple example is presented
//           first (multiple occurrences in a simple scene), followed by a more complex
//           example (multiple occurrences in a complex scene with advanced search conditions).
//           In each scene, both the rectangle finder and box finder are used to find
//           different objects.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*****************************************************************************
#include <mil.h>

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_STRING SIMPLE_SCENE_FILE = M_IMAGE_PATH MIL_TEXT("RectangleAndBoxFinder/SyntheticBoxes.mbufc");
static const MIL_STRING COMPLEX_RECTANGLE_SCENE_FILE = M_IMAGE_PATH MIL_TEXT("RectangleAndBoxFinder/LightSocketCovers.mbufc");
static const MIL_STRING COMPLEX_BOX_SCENE_FILE = M_IMAGE_PATH MIL_TEXT("RectangleAndBoxFinder/TissueBoxes.mbufc");

// All in mm.
static const MIL_DOUBLE LIGHT_SOCKET_LENGTH = 65;
static const MIL_DOUBLE LIGHT_SOCKET_WIDTH  = 32;
static const MIL_DOUBLE LIGHT_SOCKET_TOLERANCE = 5;

static const MIL_DOUBLE TISSUE_BOX_LENGTH = 215;
static const MIL_DOUBLE TISSUE_BOX_WIDTH = 115;
static const MIL_DOUBLE TISSUE_BOX_HEIGHT = 70;
static const MIL_DOUBLE TISSUE_BOX_TOLERANCE = 15;

//****************************************************************************
// Function declaration.
//****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);
bool CheckForRequiredMILFile(MIL_STRING FileName);
void ConvertAndAddNormalsIfRequired(MIL_ID Container);

void SimpleAnyRectangleExample(MIL_ID Container, MIL_ID GraList);
void SimpleAnyBoxExample(MIL_ID Container, MIL_ID GraList);
void ComplexLightSocketExample(MIL_ID Container, MIL_ID GraList);
void ComplexTissueBoxExample(MIL_ID Container, MIL_ID GraList);

MIL_INT64 FindAndDraw(MIL_ID ModContext, MIL_ID Container, MIL_ID GraList);
void PrintOccurrenceInfo(MIL_ID ModResult, MIL_INT TimeTaken);
bool IsBoxObject(MIL_ID ModContextOrResult);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Rectangle and Box Finder\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to use rectangle and box finders\n")
             MIL_TEXT("to define models and search for them in 3D point clouds.\n")
             MIL_TEXT("A simple example is presented first (multiple\n")
             MIL_TEXT("occurrences in a simple scene), followed by a more complex\n")
             MIL_TEXT("example (multiple occurrences in a complex scene with advanced \n")
             MIL_TEXT("search conditions). In each scene, both the rectangle finder and\n")
             MIL_TEXT("box finder are used to find different objects.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, 3D Model Finder, \n")
             MIL_TEXT("3D Image Processing, 3D Display, and 3D Graphics. \n\n"));
   }
//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
   {
   // Print Header. 
   PrintHeader();

   // Allocate MIL objects.
   auto Application = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto System = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   auto Container = MbufAllocContainer(System, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);

   // Check for the required example files.
   if(!CheckForRequiredMILFile(SIMPLE_SCENE_FILE))
      return -1;

   // Allocate the 3d display.
   auto Display = Alloc3dDisplayId(System);
   if(Display == M_NULL)
      return -1;

   M3ddispSelect(Display, Container, M_DEFAULT, M_DEFAULT);

   SimpleAnyRectangleExample(Container, Display);
   SimpleAnyBoxExample(Container, Display);
   ComplexLightSocketExample(Container, Display);
   ComplexTissueBoxExample(Container, Display);
   }

//*****************************************************************************
// Rectangle finder defining a range model.
//*****************************************************************************
void SimpleAnyRectangleExample(MIL_ID Container, MIL_ID Display)
   {
   // Restore the container from a file and display it.
   M3ddispControl(Display, M_UPDATE, M_DISABLE);

   MbufLoad(SIMPLE_SCENE_FILE, Container);
   ConvertAndAddNormalsIfRequired(Container);

   MIL_ID GraList = M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_COMPONENT + M_RECURSIVE, M_COMPONENT_RANGE);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_COMPONENT_BAND + M_RECURSIVE, 2);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_USE_LUT + M_RECURSIVE, M_TRUE);
   M3ddispSetView(Display, M_AUTO, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   M3ddispControl(Display, M_UPDATE, M_ENABLE);

   // Define a rectangle with an infinite size range.
   MIL_ID System   = MobjInquire(Container, M_OWNER_SYSTEM, M_NULL);
   auto ModContext = M3dmodAlloc(System, M_FIND_RECTANGULAR_PLANE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   MIL_INT64 ModelIndex = M3dmodDefine(ModContext, M_ADD, M_RECTANGLE_RANGE,
                                       0, 0,                               // Min size.
                                       M_INFINITE, M_INFINITE,             // Max size.
                                       M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Set to find any number of rectangles.
   M3dmodControl(ModContext, ModelIndex, M_NUMBER, M_ALL);

   // Set a maximum elongation to prevent lines from being seen as rectangles since the range is from 0 to inf.
   M3dmodControl(ModContext, ModelIndex, M_ELONGATION_MAX, 10);

   // Do the find.
   MosPrintf(MIL_TEXT("A simple synthetic scene is restored.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to find all rectangles with no size constraints.\n\n"));
   MosGetch();

   MIL_INT64 AnnotationNode = FindAndDraw(ModContext, Container, GraList);

   MosPrintf(MIL_TEXT("Press <Enter> to find all boxes with no size constraints.\n\n"));
   MosGetch();

   M3dgraRemove(GraList, AnnotationNode, M_DEFAULT);
   }

//*****************************************************************************
// Box finder defining a range model.
//*****************************************************************************
void SimpleAnyBoxExample(MIL_ID Container, MIL_ID Display)
   {
   // Restore the container from a file and display it.
   M3ddispControl(Display, M_UPDATE, M_DISABLE);

   MbufLoad(SIMPLE_SCENE_FILE, Container);
   ConvertAndAddNormalsIfRequired(Container);

   MIL_ID GraList = M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_COMPONENT + M_RECURSIVE, M_COMPONENT_RANGE);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_COMPONENT_BAND + M_RECURSIVE, 2);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_USE_LUT + M_RECURSIVE, M_TRUE);

   M3ddispControl(Display, M_UPDATE, M_ENABLE);

   // Define a box with an infinite size range.
   MIL_ID System = MobjInquire(Container, M_OWNER_SYSTEM, M_NULL);
   auto ModContext = M3dmodAlloc(System, M_FIND_BOX_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   MIL_INT64 ModelIndex = M3dmodDefine(ModContext, M_ADD, M_BOX_RANGE,
                                       0, 0, 0,                            // Min size.
                                       M_INFINITE, M_INFINITE, M_INFINITE, // Max size.
                                       M_DEFAULT);

   // Set to find any number of boxes.
   M3dmodControl(ModContext, ModelIndex, M_NUMBER, M_ALL);

   // Set a maximum elongation to prevent lines/planes from being seen as boxes since the range is from 0 to inf.
   M3dmodControl(ModContext, ModelIndex, M_ELONGATION_MAX, 10);

   // Set to find boxes even if only one face is visible.
   M3dmodControl(ModContext, ModelIndex, M_NUMBER_OF_VISIBLE_FACES_MIN, 1);

   // When building a box from a single face, extend it away from the camera, which points towards -Z.
   M3dmodControl(ModContext, M_DEFAULT, M_DIRECTION_MODE       , M_TOWARDS_DIRECTION);
   M3dmodControl(ModContext, M_DEFAULT, M_DIRECTION_REFERENCE_X, 0);
   M3dmodControl(ModContext, M_DEFAULT, M_DIRECTION_REFERENCE_Y, 0);
   M3dmodControl(ModContext, M_DEFAULT, M_DIRECTION_REFERENCE_Z, -1);

   // Do the find.
   MIL_INT64 AnnotationNode = FindAndDraw(ModContext, Container, GraList);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   M3dgraRemove(GraList, AnnotationNode, M_DEFAULT);
   }

//*****************************************************************************
// Rectangle finder defining a nominal model.
//*****************************************************************************
void ComplexLightSocketExample(MIL_ID Container, MIL_ID Display)
   {
   // Restore the container from a file and display it.
   M3ddispControl(Display, M_UPDATE, M_DISABLE);

   MbufLoad(COMPLEX_RECTANGLE_SCENE_FILE, Container);
   ConvertAndAddNormalsIfRequired(Container);

   MIL_ID GraList = M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_COMPONENT + M_RECURSIVE, M_AUTO_COLOR);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_USE_LUT + M_RECURSIVE, M_FALSE);
   M3ddispSetView(Display, M_AZIM_ELEV_ROLL, 44.58, -54.26, 180.29, M_DEFAULT);
   M3ddispSetView(Display, M_AUTO, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   M3ddispControl(Display, M_UPDATE, M_ENABLE);

   // Define the light socket cover.
   MIL_ID System   = MobjInquire(Container, M_OWNER_SYSTEM, M_NULL);
   auto ModContext = M3dmodAlloc(System, M_FIND_RECTANGULAR_PLANE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   MIL_INT64 ModelIndex = M3dmodDefine(ModContext, M_ADD, M_RECTANGLE,
                                       LIGHT_SOCKET_LENGTH, LIGHT_SOCKET_WIDTH,
                                       LIGHT_SOCKET_TOLERANCE, LIGHT_SOCKET_TOLERANCE,
                                       M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Set to find any number of rectangles.
   M3dmodControl(ModContext, ModelIndex, M_NUMBER, M_ALL);

   // Subsample the scene.
   MosPrintf(MIL_TEXT("A more complex scene depicting a pile of light socket covers is restored.\n"));
   MosPrintf(MIL_TEXT("The scene is needlessly dense, so it is subsampled to speed up the match.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to find rectangles which match the sockets' size.\n\n"));
   MosGetch();

   auto SubsampleContext = M3dimAlloc(System, M_SUBSAMPLE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(SubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_DECIMATE);
   M3dimControl(SubsampleContext, M_STEP_SIZE_X, 4);
   M3dimControl(SubsampleContext, M_STEP_SIZE_Y, 4);
   M3dimSample(SubsampleContext, Container, Container, M_DEFAULT);

   // Do the find.
   MIL_INT64 AnnotationNode = FindAndDraw(ModContext, Container, GraList);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   M3dgraRemove(GraList, AnnotationNode, M_DEFAULT);
   }

//*****************************************************************************
// Box finder defining a nominal model.
//*****************************************************************************
void ComplexTissueBoxExample(MIL_ID Container, MIL_ID Display)
   {
   // Restore the container from a file and display it.
   M3ddispControl(Display, M_UPDATE, M_DISABLE);

   MbufLoad(COMPLEX_BOX_SCENE_FILE, Container);
   ConvertAndAddNormalsIfRequired(Container);

   MIL_ID GraList = M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_COMPONENT + M_RECURSIVE, M_AUTO_COLOR);
   M3dgraControl(GraList, M_ROOT_NODE, M_COLOR_USE_LUT + M_RECURSIVE, M_FALSE);
   M3ddispSetView(Display, M_AUTO, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(Display, M_ZOOM, 2, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   M3ddispControl(Display, M_UPDATE, M_ENABLE);

   // Define the tissue box.
   MIL_ID System = MobjInquire(Container, M_OWNER_SYSTEM, M_NULL);
   auto ModContext = M3dmodAlloc(System, M_FIND_BOX_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   MIL_INT64 ModelIndex = M3dmodDefine(ModContext, M_ADD, M_BOX,
                                       TISSUE_BOX_LENGTH, TISSUE_BOX_WIDTH, TISSUE_BOX_HEIGHT,
                                       TISSUE_BOX_TOLERANCE, TISSUE_BOX_TOLERANCE, TISSUE_BOX_TOLERANCE,
                                       M_DEFAULT);

   // Find any number of boxes.
   M3dmodControl(ModContext, ModelIndex, M_NUMBER, M_ALL);

   // Find boxes even if only one face is visible.
   M3dmodControl(ModContext, ModelIndex, M_NUMBER_OF_VISIBLE_FACES_MIN, 1);

   // When building a box from a single face, extend it away from the camera, which is at the origin.
   M3dmodControl(ModContext, M_DEFAULT, M_DIRECTION_MODE, M_AWAY_FROM_POSITION);
   M3dmodControl(ModContext, M_DEFAULT, M_DIRECTION_REFERENCE_X, 0);
   M3dmodControl(ModContext, M_DEFAULT, M_DIRECTION_REFERENCE_Y, 0);
   M3dmodControl(ModContext, M_DEFAULT, M_DIRECTION_REFERENCE_Z, 0);

   // Do the find.
   MosPrintf(MIL_TEXT("A scene depicting a bin of tissue boxes is restored.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to find boxes that match the tissue boxes' size.\n\n"));
   MosGetch();

   MIL_INT64 AnnotationNode = FindAndDraw(ModContext, Container, GraList);

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   M3dgraRemove(GraList, AnnotationNode, M_DEFAULT);
   }

//*****************************************************************************
// Run M3dmodFind, draw the results in the 3d display and print the relevant information on the console.
//*****************************************************************************
MIL_INT64 FindAndDraw(MIL_ID ModContext, MIL_ID Container, MIL_ID GraList)
   {
   // Allocate a result of the right type.
   MIL_ID System = MobjInquire(ModContext, M_OWNER_SYSTEM, M_NULL);
   bool IsBox = IsBoxObject(ModContext);
   MIL_INT64 ResultType = IsBox ? M_FIND_BOX_RESULT : M_FIND_RECTANGULAR_PLANE_RESULT;
   auto ModResult = M3dmodAllocResult(System, ResultType, M_DEFAULT, M_UNIQUE_ID);

   // Preprocess the context.
   M3dmodPreprocess(ModContext, M_DEFAULT);

   // Do the find and benchmark it.
   MIL_DOUBLE StartTime = MappTimer(M_TIMER_READ, M_NULL);        // In s.
   M3dmodFind(ModContext, Container, ModResult, M_DEFAULT);
   MIL_DOUBLE EndTime = MappTimer(M_TIMER_READ, M_NULL);          // In s.
   MIL_INT TimeTaken = (MIL_INT)((EndTime - StartTime) * 1000);   // In ms.

   // Draw the occurrences and show their size/score in the console.
   MIL_INT64 AnnotationNode = M3dmodDraw3d(M_DEFAULT, ModResult, M_ALL, GraList, M_ROOT_NODE, M_DEFAULT);
   PrintOccurrenceInfo(ModResult, TimeTaken);

   return AnnotationNode;
   }

//*****************************************************************************
// Prints the size and score of found rectangles/boxes in the console.
//*****************************************************************************
void PrintOccurrenceInfo(MIL_ID ModResult, MIL_INT TimeTaken)
   {
   // Print the number of occurrences.
   MIL_INT NbOccurrences = (MIL_INT)M3dmodGetResult(ModResult, M_DEFAULT, M_NUMBER, M_NULL);
   MosPrintf(MIL_TEXT("Found %i occurrences in %i ms.\n\n"), NbOccurrences, TimeTaken);
   if(NbOccurrences == 0)
      return;

   // Build the top of the table.
   MIL_STRING Header = MIL_TEXT("Index      Error (mm)   Score (%%)   Size X (mm)  Size Y (mm)");
   MIL_STRING Format = MIL_TEXT("  %02i          %.2f        %6.2f      %6.2f      %6.2f");

   bool IsBox = IsBoxObject(ModResult);
   if(IsBox)
      {
      Header += MIL_TEXT("  Size Z (mm)");
      Format += MIL_TEXT("      %6.2f");
      }
   Header += '\n';
   Format += '\n';

   MosPrintf(Header.c_str());
   MosPrintf((MIL_STRING(Header.size(), '-') + MIL_TEXT("\n")).c_str());

   // Print the info for each occurrence.
   for(MIL_INT i = 0; i < NbOccurrences; i++)
      {
      MIL_DOUBLE Error = M3dmodGetResult(ModResult, i, M_RMS_ERROR, M_NULL);
      MIL_DOUBLE Score = M3dmodGetResult(ModResult, i, M_SCORE, M_NULL);
      MIL_DOUBLE SizeX = M3dmodGetResult(ModResult, i, M_SIZE_X, M_NULL);
      MIL_DOUBLE SizeY = M3dmodGetResult(ModResult, i, M_SIZE_Y, M_NULL);
      MIL_DOUBLE SizeZ = IsBox ? M3dmodGetResult(ModResult, i, M_SIZE_Z, M_NULL) : 0;
      MosPrintf(Format.c_str(), i, Error, Score, SizeX, SizeY, SizeZ);
      }
   MosPrintf(MIL_TEXT("\n"));
   }

//*****************************************************************************
// Returns true if the 3dmod object is a box context or result, and false if it is a plane context or result.
//*****************************************************************************
bool IsBoxObject(MIL_ID ModContextOrResult)
   {
   MIL_INT64 ObjectType;
   MobjInquire(ModContextOrResult, M_OBJECT_TYPE, &ObjectType);
   switch(ObjectType)
      {
      case M_3DMOD_FIND_BOX_CONTEXT:
      case M_3DMOD_FIND_BOX_RESULT:
         return true;
      case M_3DMOD_FIND_RECTANGULAR_PLANE_CONTEXT:
      case M_3DMOD_FIND_RECTANGULAR_PLANE_RESULT:
         return false;
      }
   // Should never happen.
   return false;
   }

//*****************************************************************************
// Adds the component M_COMPONENT_NORMALS_MIL if it's not found.
//*****************************************************************************
void ConvertAndAddNormalsIfRequired(MIL_ID Container)
   {
   // Convert.
   if(MbufInquireContainer(Container, M_CONTAINER, M_3D_PROCESSABLE, M_NULL) != M_PROCESSABLE)
      {
      MbufConvert3d(Container, Container, M_NULL, M_DEFAULT, M_DEFAULT);
      }

   // Add normals.
   MIL_ID Normals = MbufInquireContainer(Container, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL);
   if(Normals == M_NULL)
      {
      MIL_INT SizeX = MbufInquireContainer(Container, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
      MIL_INT SizeY = MbufInquireContainer(Container, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
      bool Organized = SizeX > 20 && SizeY > 20;
      M3dimNormals(Organized ? M_NORMALS_CONTEXT_ORGANIZED : M_NORMALS_CONTEXT_TREE, Container, Container, M_DEFAULT);
      }
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.
//*****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to continue.\n"));
      MosGetch();
      }
   return MilDisplay3D;
   }

//*******************************************************************************
// Checks the required files exist.
//*****************************************************************************
bool CheckForRequiredMILFile(MIL_STRING FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);

   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }
