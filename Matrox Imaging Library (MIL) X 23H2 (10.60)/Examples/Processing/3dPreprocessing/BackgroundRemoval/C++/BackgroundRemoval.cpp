﻿//***************************************************************************************/
//
// File name: BackgroundRemoval.cpp
//
// Synopsis:  This program demonstrates various ways of removing the background in a point cloud.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include "DisplayLinker.h"

// Source file specification.
static const MIL_STRING BOX_SCENE_FILE       = M_IMAGE_PATH MIL_TEXT("BackgroundRemoval/Clementine.ply");
static const MIL_STRING PLANE_SCENE_FILE     = M_IMAGE_PATH MIL_TEXT("3dPlaneFit/MechanicalPart.ply");
static const MIL_STRING REF_SCENE_FILE       = M_IMAGE_PATH MIL_TEXT("BackgroundRemoval/scene.ply");
static const MIL_STRING REF_BACKGROUND_FILE  = M_IMAGE_PATH MIL_TEXT("BackgroundRemoval/scene_ref.ply");
static const MIL_STRING BOX_FILE             = M_IMAGE_PATH MIL_TEXT("BackgroundRemoval/Box.m3dgeo");

static const MIL_INT DISPLAY_SIZE_X = 500;
static const MIL_INT DISPLAY_SIZE_Y = 400;

// Function declarations.
bool                 CheckForRequiredMILFile(const MIL_STRING& FileName);
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);

void RemoveBackgroundCrop(MIL_ID SrcContainer, MIL_ID DstContainer, MIL_ID GraphicList, MIL_INT64 AnnotationNode);
void RemoveBackgroundFit(MIL_ID SrcContainer, MIL_ID DstContainer, MIL_ID GraphicList, MIL_INT64 AnnotationNode);
void RemoveBackgroundRef(MIL_ID SrcContainer, MIL_ID DstContainer, MIL_ID RefContainer);

//-----------------------------------------------------------------------------
// Example description.
//-----------------------------------------------------------------------------
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("BackgroundRemoval\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates various ways of removing the background in a point cloud.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Image Processing, 3D Metrology, 3D Blob Analysis\n")
             MIL_TEXT("3D Display, 3D Graphics, and Buffer.\n\n"));
   }

//-----------------------------------------------------------------------------
// Main.
//-----------------------------------------------------------------------------
int MosMain(void)
   {
   PrintHeader();

   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Check for required example files.
   if(!CheckForRequiredMILFile(BOX_SCENE_FILE) ||
      !CheckForRequiredMILFile(PLANE_SCENE_FILE) ||
      !CheckForRequiredMILFile(REF_SCENE_FILE) ||
      !CheckForRequiredMILFile(REF_BACKGROUND_FILE))
      {
      return 0;
      }
     
   auto MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Set up the source display.
   auto SrcDisplay = Alloc3dDisplayId(MilSystem);
   auto SrcPointCloud = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   auto SrcGraphicList = (MIL_ID)M3ddispInquire(SrcDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   M3ddispControl(SrcDisplay, M_SIZE_X, DISPLAY_SIZE_X);
   M3ddispControl(SrcDisplay, M_SIZE_Y, DISPLAY_SIZE_Y);
   M3ddispControl(SrcDisplay, M_TITLE, MIL_TEXT("Original scene"));

   // Set up the destination display.
   auto DstDisplay = Alloc3dDisplayId(MilSystem);
   auto DstPointCloud = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   auto DstGraphicList = (MIL_ID)M3ddispInquire(DstDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   M3ddispControl(DstDisplay, M_SIZE_X, DISPLAY_SIZE_X);
   M3ddispControl(DstDisplay, M_SIZE_Y, DISPLAY_SIZE_Y);
   M3ddispControl(DstDisplay, M_WINDOW_INITIAL_POSITION_X, DISPLAY_SIZE_X);
   M3ddispControl(DstDisplay, M_TITLE, MIL_TEXT("Background removed"));

   // Set up the reference display.
   auto RefDisplay = Alloc3dDisplayId(MilSystem);
   auto RefPointCloud = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);

   M3ddispControl(RefDisplay, M_SIZE_X, DISPLAY_SIZE_X);
   M3ddispControl(RefDisplay, M_SIZE_Y, DISPLAY_SIZE_Y);
   M3ddispControl(RefDisplay, M_WINDOW_INITIAL_POSITION_Y, DISPLAY_SIZE_Y + 30);
   M3ddispControl(RefDisplay, M_TITLE, MIL_TEXT("Reference background."));

   // Link all 3 displays together.
   CDisplayLinker DisplayLinker({DstDisplay, SrcDisplay, RefDisplay});

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   // Method 1: Geometric cropping.
   MosPrintf(MIL_TEXT("Ex 1: Geometric cropping.\n"));
   MosPrintf(MIL_TEXT("The points outside the box are removed.\n"));
   MosPrintf(MIL_TEXT("This is useful when the object is always at the same place in the scene.\n\n"));

   M3ddispControl(SrcDisplay, M_UPDATE, M_DISABLE);
   M3ddispControl(DstDisplay, M_UPDATE, M_DISABLE);
   MbufImport(BOX_SCENE_FILE, M_DEFAULT, M_LOAD, MilSystem, &SrcPointCloud);
   MIL_INT64 AnnotationNode = M3dgraNode(SrcGraphicList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);

   RemoveBackgroundCrop(SrcPointCloud, DstPointCloud, SrcGraphicList, AnnotationNode);

   M3ddispSetView(DstDisplay, M_AUTO, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispControl(SrcDisplay, M_UPDATE, M_ENABLE);
   M3ddispControl(DstDisplay, M_UPDATE, M_ENABLE);

   M3ddispSelect(SrcDisplay, SrcPointCloud, M_SELECT, M_DEFAULT);
   M3ddispSelect(DstDisplay, DstPointCloud, M_SELECT, M_DEFAULT);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Method 2: Plane fitting.
   MosPrintf(MIL_TEXT("Ex 2: Plane fitting.\n"));
   MosPrintf(MIL_TEXT("A plane is fitted on the background and all points close to or below it are removed.\n"));
   MosPrintf(MIL_TEXT("This is useful for large planar backgrounds.\n\n"));

   M3ddispControl(SrcDisplay, M_UPDATE, M_DISABLE);
   M3ddispControl(DstDisplay, M_UPDATE, M_DISABLE);
   MbufImport(PLANE_SCENE_FILE, M_DEFAULT, M_LOAD, MilSystem, &SrcPointCloud);

   M3dgraRemove(SrcGraphicList, AnnotationNode, M_DEFAULT);
   AnnotationNode = M3dgraNode(SrcGraphicList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);

   RemoveBackgroundFit(SrcPointCloud, DstPointCloud, SrcGraphicList, AnnotationNode);

   M3dgraControl(SrcGraphicList, M_ROOT_NODE, M_COLOR_COMPONENT + M_RECURSIVE, M_COMPONENT_RANGE);
   M3dgraControl(DstGraphicList, M_ROOT_NODE, M_COLOR_COMPONENT + M_RECURSIVE, M_COMPONENT_RANGE);
   M3dgraControl(SrcGraphicList, M_ROOT_NODE, M_COLOR_COMPONENT_BAND + M_RECURSIVE, 2);
   M3dgraControl(DstGraphicList, M_ROOT_NODE, M_COLOR_COMPONENT_BAND + M_RECURSIVE, 2);
   M3dgraControl(SrcGraphicList, M_ROOT_NODE, M_COLOR_USE_LUT + M_RECURSIVE, M_TRUE);
   M3dgraControl(DstGraphicList, M_ROOT_NODE, M_COLOR_USE_LUT + M_RECURSIVE, M_TRUE);

   M3ddispSetView(DstDisplay, M_AUTO, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispControl(SrcDisplay, M_UPDATE, M_ENABLE);
   M3ddispControl(DstDisplay, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Method 3: Reference subtraction.
   MosPrintf(MIL_TEXT("Ex 3: Reference cropping.\n"));
   MosPrintf(MIL_TEXT("A reference scene without an object is subtracted from the same scene with an object.\n"));
   MosPrintf(MIL_TEXT("This is useful when the background is complex but never changes.\n\n"));

   MbufImport(REF_BACKGROUND_FILE, M_DEFAULT, M_LOAD, MilSystem, &RefPointCloud);
   M3ddispSelect(RefDisplay, RefPointCloud, M_DEFAULT, M_DEFAULT);

   M3ddispControl(SrcDisplay, M_UPDATE, M_DISABLE);
   M3ddispControl(DstDisplay, M_UPDATE, M_DISABLE);
   MbufImport(REF_SCENE_FILE, M_DEFAULT, M_LOAD, MilSystem, &SrcPointCloud);

   M3dgraRemove(SrcGraphicList, AnnotationNode, M_DEFAULT);
   AnnotationNode = M3dgraNode(SrcGraphicList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);

   RemoveBackgroundRef(SrcPointCloud, DstPointCloud, RefPointCloud);

   M3dgraControl(SrcGraphicList, M_ROOT_NODE, M_COLOR_COMPONENT + M_RECURSIVE, M_AUTO_COLOR);
   M3dgraControl(DstGraphicList, M_ROOT_NODE, M_COLOR_COMPONENT + M_RECURSIVE, M_AUTO_COLOR);

   M3ddispSetView(DstDisplay, M_AUTO, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(DstDisplay, M_ROLL, 180, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(DstDisplay, M_ZOOM, 0.5, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispControl(SrcDisplay, M_UPDATE, M_ENABLE);
   M3ddispControl(DstDisplay, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("Press <Enter> to End.\n\n"));
   MosGetch();

   return 0;
   }

//-----------------------------------------------------------------------------
// Check for required files to run the example.
//-----------------------------------------------------------------------------
bool CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;
   
   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }

//-----------------------------------------------------------------------------
// Allocates a 3D display and returns its MIL identifier.
//-----------------------------------------------------------------------------
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to exit.\n"));
      MosGetch();
      exit(0);
      }
   return MilDisplay3D;
   }


//-----------------------------------------------------------------------------
// Removes the background by cropping with a box.
//-----------------------------------------------------------------------------
void RemoveBackgroundCrop(MIL_ID SrcContainer, MIL_ID DstContainer, MIL_ID GraphicList, MIL_INT64 AnnotationNode)
   {
   MIL_ID System = MobjInquire(SrcContainer, M_OWNER_SYSTEM, M_NULL);

   // Restore the box.
   auto Box = M3dgeoRestore(BOX_FILE, System, M_DEFAULT, M_UNIQUE_ID);

   // Draw the box on the 3d display.
   MIL_INT64 BoxLabel = M3dgeoDraw3d(M_DEFAULT, Box, GraphicList, AnnotationNode, M_DEFAULT);
   M3dgraControl(GraphicList, BoxLabel, M_OPACITY, 30);
   M3dgraControl(GraphicList, BoxLabel, M_APPEARANCE, M_SOLID_WITH_WIREFRAME);

   // Crop all points outside the box.
   M3dimCrop(SrcContainer, DstContainer, Box, M_NULL, M_UNORGANIZED, M_DEFAULT);
   }


//-----------------------------------------------------------------------------
// Removes the background by fitting a plane and excluding points below.
//-----------------------------------------------------------------------------
void RemoveBackgroundFit(MIL_ID SrcContainer, MIL_ID DstContainer, MIL_ID GraphicList, MIL_INT64 AnnotationNode)
   {
   static const MIL_DOUBLE PLANE_FIT_TOLERANCE = 2;     // Max deviation from the plane for points to be considered inliers, in mm.
   static const MIL_DOUBLE PLANE_CROP_TOLERANCE = 10;   // Max deviation from the plane for points not to be cropped, in mm.
   static const MIL_DOUBLE FONT_SIZE = 20;              // In mm.

   MIL_ID System = MobjInquire(SrcContainer, M_OWNER_SYSTEM, M_NULL);

   auto TextMatrix = M3dgeoAlloc(System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(TextMatrix, M_TRANSLATION, 0, -FONT_SIZE * 4, 1, M_DEFAULT, M_DEFAULT);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_FONT_SIZE, FONT_SIZE);

   // Fit a plane.
   auto Plane = M3dgeoAlloc(System, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dmetFit(M_DEFAULT, SrcContainer, M_PLANE, Plane, PLANE_FIT_TOLERANCE, M_DEFAULT);

   // Draw the fitted plane on the 3d display.
   MIL_INT64 FittedPlaneLabel = M3dgeoDraw3d(M_DEFAULT, Plane, GraphicList, AnnotationNode, M_DEFAULT);
   M3dgraControl(GraphicList, FittedPlaneLabel, M_OPACITY, 50);
   M3dgraText(GraphicList, FittedPlaneLabel, MIL_TEXT("Fitted plane"), TextMatrix, M_DEFAULT, M_DEFAULT);

   // Slide the plane up before cropping.
   MIL_DOUBLE Nx = M3dgeoInquire(Plane, M_NORMAL_X, M_NULL);
   MIL_DOUBLE Ny = M3dgeoInquire(Plane, M_NORMAL_Y, M_NULL);
   MIL_DOUBLE Nz = M3dgeoInquire(Plane, M_NORMAL_Z, M_NULL);
   M3dimTranslate(Plane, Plane, Nx * PLANE_CROP_TOLERANCE, Ny * PLANE_CROP_TOLERANCE, Nz * PLANE_CROP_TOLERANCE, M_DEFAULT);

   // Draw the cropping plane on the 3d display.
   MIL_INT64 CroppingPlaneLabel = M3dgeoDraw3d(M_DEFAULT, Plane, GraphicList, AnnotationNode, M_DEFAULT);
   M3dgraControl(GraphicList, CroppingPlaneLabel, M_OPACITY, 50);
   M3dgeoMatrixSetTransform(TextMatrix, M_TRANSLATION, 0, FONT_SIZE, 0, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
   M3dgraText(GraphicList, CroppingPlaneLabel, MIL_TEXT("Cropping plane"), TextMatrix, M_DEFAULT, M_DEFAULT);

   // Crop the container using the translated plane. 
   M3dimCrop(SrcContainer, DstContainer, Plane, M_NULL, M_SAME, M_DEFAULT);
   }


//-----------------------------------------------------------------------------
// Removes the background by performing 3d subtraction using a reference scene.
//-----------------------------------------------------------------------------
void RemoveBackgroundRef(MIL_ID SrcContainer, MIL_ID DstContainer, MIL_ID RefContainer)
   {
   static const MIL_DOUBLE DISTANCE_THRESHOLD = 5;     // Minimum distance for points to be considered part of the object.
   static const MIL_INT MIN_NB_POINTS = 1000;          // Number of points below which objects are considered artifacts.

   MIL_ID System = MobjInquire(SrcContainer, M_OWNER_SYSTEM, M_NULL);

   // Copy the source container into the destination.
   MbufCopy(SrcContainer, DstContainer);

   // Allocate a buffer that will store the distance data.
   MIL_INT SizeX = MbufInquireContainer(DstContainer, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(DstContainer, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   auto DistanceBuffer = MbufAlloc2d(System, SizeX, SizeY, M_FLOAT + 32, M_IMAGE + M_PROC, M_UNIQUE_ID);

   // Compute the distances between the two point clouds. Provide a max distance to prevent extremely long computations.
   M3dmetDistance(DstContainer, RefContainer, DistanceBuffer, M_DISTANCE_TO_NEAREST_NEIGHBOR, DISTANCE_THRESHOLD, M_DEFAULT);

   // Make any points with a distance smaller than the threshold invalid in the destination container.
   MIL_ID DstConfidence = MbufInquireContainer(DstContainer, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   MimBinarize(DistanceBuffer, DstConfidence, M_FIXED + M_GREATER, DISTANCE_THRESHOLD, M_NULL);

   // Make any invalid points in the source also invalid in destination.
   MIL_INT SrcConfidence = MbufInquireContainer(SrcContainer, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   MbufClearCond(DstConfidence, 0, 0, 0, SrcConfidence, M_EQUAL, 0);

   // Since the confidence was modified directly (instead of with a 3d function), the mesh still contains points which are now invalid.
   // Triangles containing invalid points must be removed with M3dimFix.
   M3dimFix(DstContainer, DstContainer, M_MESH_VALID_POINTS, M_DEFAULT, M_NULL);

   // At this point, the background has been removed, but there are still small artifacts.
   // Identify distinct mesh clusters and select only the larger ones.
   auto BlobContext = M3dblobAlloc(System, M_SEGMENTATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto BlobResult = M3dblobAllocResult(System, M_SEGMENTATION_RESULT, M_DEFAULT, M_UNIQUE_ID);

   M3dblobControl(BlobContext, M_DEFAULT, M_NEIGHBOR_SEARCH_MODE, M_MESH);
   M3dblobControl(BlobContext, M_DEFAULT, M_NUMBER_OF_POINTS_MIN, MIN_NB_POINTS);
   M3dblobSegment(BlobContext, DstContainer, BlobResult, M_DEFAULT);

   // Extract the larger clusters into the destination container.
   M3dblobExtract(DstContainer, BlobResult, M_ALL_BLOBS, DstContainer, M_AUTO, M_DEFAULT);

   }
