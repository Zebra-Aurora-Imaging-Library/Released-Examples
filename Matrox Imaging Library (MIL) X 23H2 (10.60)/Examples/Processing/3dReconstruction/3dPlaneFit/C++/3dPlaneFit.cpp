﻿//***************************************************************************************
// 
// File name: 3dPlaneFit.cpp  
//
// Synopsis: This program contains an example of a 3D plane fit using the 3D 
//           metrology module.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************

#include <mil.h>

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("3dPlaneFit\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the definition and usage of "));
   MosPrintf(MIL_TEXT("a 3D plane fit.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, 3D Display, Buffer, 3D Graphics,\n")
             MIL_TEXT("and 3D Metrology.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Structures, constants and functions
//*****************************************************************************
struct CylinderStruct
   {
   MIL_DOUBLE CenterX;
   MIL_DOUBLE CenterY;
   MIL_DOUBLE Radius;
   };

// Source image files specification. 
static const MIL_INT STRING_LENGTH = 1024;

static const MIL_TEXT_CHAR POINT_CLOUD_FILE[STRING_LENGTH] =
M_IMAGE_PATH MIL_TEXT("3dPlaneFit/MechanicalPart.ply");//MIL_TEXT("MechanicalPart.ply"); 

static const MIL_TEXT_CHAR SIDE_VIEW_IMAGE_FILE[STRING_LENGTH] =
M_IMAGE_PATH MIL_TEXT("3dPlaneFit/SideView.png");

static const MIL_DOUBLE REGION_DISPLAY_OFFSET = 2;

static const CylinderStruct PLANE_REGION =
   {
   87,   117,   20
   };

static const MIL_INT NUM_LOCATIONS = 8;

static const MIL_DOUBLE MAX_PLANE_DEVIATION = 5;

static const CylinderStruct MEASURE_REGION[NUM_LOCATIONS] =
   {
      {  80,    131,    2},
      {  55,    85,     2},
      {  130,   162,    2},
      {  155,   188,    2},
      {  2,     55,     2},
      {  130,   200,    2},
      {  144,   230,    2},
      {  137,   118,    2}
   };

//****************************************************************************
// Function Declaration.
//****************************************************************************
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);
bool   CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName);
//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Print Header. 
   PrintHeader();

   // Allocate the MIL application.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Check for required example files.
   if(!CheckForRequiredMILFile(POINT_CLOUD_FILE))
      {
      return -1;
      }

   MIL_UNIQUE_SYS_ID MilSystem;             // System identifier.
   MIL_ID            MilDisplay;            // Display identifier.
   MIL_UNIQUE_BUF_ID   MilLut;              // Jet LUT.

   MIL_UNIQUE_BUF_ID    MilPtCldContainer;  // Original point cloud.
   MIL_UNIQUE_BUF_ID    MilPtCldRegion;     // Cropped region in the point cloud.

   MIL_UNIQUE_3DMET_ID  MilFitResult;       // Fit result.
   MIL_UNIQUE_3DMET_ID  MilStatResult;      // Stat result.
   MIL_UNIQUE_3DGEO_ID  MilPlane;           // Fitted plane.
   MIL_UNIQUE_3DGEO_ID  MilCylinder;        // Region of interest cylinder.

   // Allocate MIL objects. 
   MilSystem      = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MilDisplay     = Alloc3dDisplayId(MilSystem);
   MilPtCldRegion = MbufAllocContainer(MilSystem, M_PROC, M_DEFAULT, M_UNIQUE_ID);
   MilPlane       = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   MilCylinder    = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   MilFitResult   = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_UNIQUE_ID);
   MilStatResult  = M3dmetAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   M3ddispSetView(MilDisplay, M_ELEVATION, 60.0, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilDisplay, M_AZIMUTH  , 95.0, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Restore the point cloud.
   MosPrintf(MIL_TEXT("A 3D point cloud is restored from a PLY file and displayed.\n\n"));
   MilPtCldContainer = MbufRestore(POINT_CLOUD_FILE, MilSystem, M_UNIQUE_ID);

   // Display the point cloud.
   MIL_ID MilGraphicList = M_NULL;
   M3ddispInquire(MilDisplay, M_3D_GRAPHIC_LIST_ID, &MilGraphicList);

   M3ddispControl(MilDisplay, M_UPDATE, M_DISABLE);
   MIL_INT64 MilContainerGraphics = M3ddispSelect(MilDisplay, MilPtCldContainer, M_SELECT, M_DEFAULT);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_USE_LUT, M_TRUE);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT_BAND, 2);
   M3ddispControl(MilDisplay, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Remove the background plane.
   M3dmetFit(M_DEFAULT, MilPtCldContainer, M_PLANE, MilFitResult, MAX_PLANE_DEVIATION, M_DEFAULT);
   MIL_ID MilConfidence = MbufInquireContainer(MilPtCldContainer, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   M3ddispControl(MilDisplay, M_UPDATE, M_DISABLE);
   M3dmetCopyResult(MilFitResult, MilConfidence, M_OUTLIER_MASK, M_DEFAULT);
   M3ddispControl(MilDisplay, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("A plane is fit on the background floor.\n"));
   MosPrintf(MIL_TEXT("Only points above the fit plane are kept.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Crop the plane's region of interest around a cylinder.
   M3dgeoCylinder(MilCylinder, M_POINT_AND_VECTOR, PLANE_REGION.CenterX, PLANE_REGION.CenterY, 0, 0, 0, 1, PLANE_REGION.Radius, M_INFINITE,M_DEFAULT);
   M3dimCrop(MilPtCldContainer, MilPtCldRegion, MilCylinder, M_NULL, M_UNORGANIZED, M_DEFAULT);

   // Display the region of interest.
   MIL_UNIQUE_3DIM_ID MilResultId = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilPtCldRegion, MilResultId, M_DEFAULT);
   MIL_DOUBLE MinZ, MaxZ;
   M3dimGetResult(MilResultId, M_MIN_Z,  &MinZ);
   M3dimGetResult(MilResultId, M_MAX_Z,  &MaxZ);
   MIL_INT64 MilGraCylinder = M3dgraCylinder(MilGraphicList, M_ROOT_NODE, M_TWO_POINTS, PLANE_REGION.CenterX, PLANE_REGION.CenterY, MinZ - REGION_DISPLAY_OFFSET,
                                             PLANE_REGION.CenterX, PLANE_REGION.CenterY, MaxZ + REGION_DISPLAY_OFFSET, PLANE_REGION.Radius, M_DEFAULT, M_DEFAULT);
   M3dgraControl(MilGraphicList, MilGraCylinder, M_OPACITY, 50);
   M3dgraControl(MilGraphicList, MilGraCylinder, M_COLOR , M_COLOR_GREEN);

   MosPrintf(MIL_TEXT("A plane is fit on the region in green.\n"));
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, M_NULL);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR, M_COLOR_GRAY);
   MosGetch();

   // Fit a plane on the region.
   M3dgraRemove(MilGraphicList, MilGraCylinder, M_DEFAULT);
   M3dmetFit(M_DEFAULT, MilPtCldRegion, M_PLANE, MilFitResult, M_INFINITE, M_DEFAULT);

   MIL_INT Status = 0;
   M3dmetGetResult(MilFitResult, M_STATUS, &Status);
   if(Status == M_SUCCESS)
      {
      MIL_DOUBLE AverageHeight = 0.0;
      MIL_DOUBLE AverageDifference = 0.0;
      MIL_TEXT_CHAR OutputText[STRING_LENGTH] = MIL_TEXT("");

      // Retrieve and display the fitted plane.
      M3dmetCopyResult(MilFitResult, MilPlane, M_FITTED_GEOMETRY, M_DEFAULT);
      MIL_DOUBLE CenterX = M3dmetGetResult(MilFitResult, M_CENTER_X, M_NULL);
      MIL_DOUBLE CenterY = M3dmetGetResult(MilFitResult, M_CENTER_Y, M_NULL);
      MIL_DOUBLE CenterZ = M3dmetGetResult(MilFitResult, M_CENTER_Z, M_NULL);
      MIL_DOUBLE NormalX = M3dmetGetResult(MilFitResult, M_NORMAL_X, M_NULL);
      MIL_DOUBLE NormalY = M3dmetGetResult(MilFitResult, M_NORMAL_Y, M_NULL);
      MIL_DOUBLE NormalZ = M3dmetGetResult(MilFitResult, M_NORMAL_Z, M_NULL);
      MIL_INT64 GraPlane = M3dgraPlane(MilGraphicList, M_ROOT_NODE, M_POINT_AND_NORMAL, CenterX, CenterY, CenterZ, NormalX, NormalY, NormalZ, M_DEFAULT, M_DEFAULT, M_DEFAULT, 350, M_DEFAULT);
      M3dgraControl(MilGraphicList, GraPlane, M_OPACITY, 10);
      M3dgraControl(MilGraphicList, GraPlane, M_COLOR, M_COLOR_GREEN);

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      M3ddispControl(MilDisplay, M_UPDATE, M_DISABLE);
      for(MIL_INT i = 0; i < NUM_LOCATIONS; i++)
         {
         // Crop the region of interest.
         M3dgeoCylinder(MilCylinder, M_POINT_AND_VECTOR, MEASURE_REGION[i].CenterX, MEASURE_REGION[i].CenterY, 0, 0, 0, 1, MEASURE_REGION[i].Radius, M_INFINITE,M_DEFAULT);
         M3dimCrop(MilPtCldContainer, MilPtCldRegion, MilCylinder, M_NULL, M_UNORGANIZED, M_DEFAULT);

         // Display the region of interest.
         M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilPtCldRegion, MilResultId, M_DEFAULT);
         MIL_DOUBLE MinZ, MaxZ;
         M3dimGetResult(MilResultId, M_MIN_Z, &MinZ);
         M3dimGetResult(MilResultId, M_MAX_Z, &MaxZ);
         MilGraCylinder = M3dgraCylinder(MilGraphicList, M_ROOT_NODE, M_TWO_POINTS, MEASURE_REGION[i].CenterX, MEASURE_REGION[i].CenterY, MinZ,
                                         MEASURE_REGION[i].CenterX, MEASURE_REGION[i].CenterY, MaxZ, MEASURE_REGION[i].Radius, M_DEFAULT, M_DEFAULT);
         M3dgraControl(MilGraphicList, MilGraCylinder, M_OPACITY, 60);
         M3dgraControl(MilGraphicList, MilGraCylinder, M_COLOR , M_COLOR_GREEN);

         MIL_DOUBLE PointX = M3dimGetResult(MilResultId, M_BOX_CENTER_X, M_NULL);
         MIL_DOUBLE PointY = M3dimGetResult(MilResultId, M_BOX_CENTER_Y, M_NULL);
         MIL_DOUBLE PointZ = M3dimGetResult(MilResultId, M_BOX_CENTER_Z, M_NULL);
         MIL_DOUBLE Dot = (CenterX - PointX) * NormalX + (CenterY - PointY) * NormalY + (CenterZ - PointZ) * NormalZ;
         MIL_INT64 MilGraLine = M3dgraLine(MilGraphicList, M_ROOT_NODE, M_POINT_AND_VECTOR, M_DEFAULT, PointX, PointY, PointZ, NormalX * Dot, NormalY * Dot, NormalZ * Dot, M_DEFAULT, M_DEFAULT);
         M3dgraControl(MilGraphicList, MilGraLine, M_THICKNESS, 3);
         M3dgraControl(MilGraphicList, MilGraLine, M_COLOR, M_COLOR_GREEN);

         // Get the average height with respect to the plane.
         M3dmetStat(M_STAT_CONTEXT_MEAN, MilPtCldRegion, MilPlane, MilStatResult, M_ABSOLUTE_DISTANCE_TO_SURFACE, M_ALL, M_NULL, M_NULL, M_DEFAULT);
         M3dmetGetResult(MilStatResult, M_STAT_MEAN, &AverageHeight);

         // Display the distance on the 3D display.
         MosSprintf(OutputText, STRING_LENGTH, MIL_TEXT("  %.2f"), AverageHeight);
         MIL_INT64 MilText = M3dgraText(MilGraphicList, MilGraCylinder, OutputText, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         M3dgraControl(MilGraphicList, MilText, M_FONT_SIZE, 10);
         }

      M3ddispControl(MilDisplay, M_UPDATE, M_ENABLE);
      MosPrintf(MIL_TEXT("The distances to the plane are displayed in mm.\n\n"));
      }
   else
      {
      MosPrintf(MIL_TEXT("Plane fit unsuccessful.\n"));
      }

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   if(MilDisplay)
      { M3ddispFree(MilDisplay); }

   return 0;
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.  
//*****************************************************************************
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
  {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
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

//****************************************************************************
// Check for required files to run the example.    
//****************************************************************************
bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName)
  {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The file needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
  }
