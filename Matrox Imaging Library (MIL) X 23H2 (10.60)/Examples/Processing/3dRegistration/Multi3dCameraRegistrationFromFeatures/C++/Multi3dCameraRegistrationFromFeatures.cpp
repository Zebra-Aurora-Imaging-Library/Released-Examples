﻿//***************************************************************************************/
//
// File name: Multi3dCameraRegistrationFromFeatures.cpp
//
// Synopsis:  This example demonstrate how to merge point clouds using 
//            3d point features extracted using the reflectance.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <algorithm>
#include "FeatureFinder.h"

// Source file specification.
static const MIL_STRING GRID_PT_CLD_FILE = M_IMAGE_PATH MIL_TEXT("Multi3dCameraRegistrationFromFeatures/GridCam");
static const MIL_STRING DATAMATRIX_PT_CLD_FILE = M_IMAGE_PATH MIL_TEXT("Multi3dCameraRegistrationFromPlanarObject/DatamatrixCam");

// General example parameters.
static const MIL_INT    NB_POINT_CLOUDS = 4;
static const MIL_INT    POINT_COLOR = M_COLOR_GREEN;
static const MIL_INT    POINT_THICKNESS = 5;

// Specific example parameters.
struct SExampleParams
   {
   MIL_STRING FileName;
   MIL_DOUBLE ViewZoom;
   MIL_DOUBLE ViewTx;
   MIL_DOUBLE ViewTy;
   MIL_DOUBLE ViewTz;
   };
static const SExampleParams DATAMATRIX_EXAMPLE_PARAMS = {DATAMATRIX_PT_CLD_FILE, 1.2, 0.0, 0.35, 0.0};
static const SExampleParams GRID_EXAMPLE_PARAMS       = {      GRID_PT_CLD_FILE, 1.4, 130, 40, 0.0};

// Function declarations.
void                    CheckForRequiredMILFile(const MIL_STRING& FileName);
MIL_UNIQUE_3DDISP_ID    Alloc3dDisplayId(MIL_ID MilSystem);
int DatamatrixRelativeExample(MIL_ID MilSystem);
int GridRelativeExample(MIL_ID MilSystem);
int FeatureBasedRegistrationExample(MIL_ID MilSystem, const SExampleParams& Params, IFeatureFinder* pFeatureFinder);


//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("Multi3dCameraRegistrationFromFeatures\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to register and merge point clouds \n")
             MIL_TEXT("using 3d point features extracted using reflectance.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Display, 3D Geometry, 3D Graphics, 3D Image Processing,\n")
             MIL_TEXT("Buffer, Calibration, Code Reader.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Run the alignment in the absolute of the grid.
   GridRelativeExample(MilSystem);

   // Run the relative alignment using the corners of the data matrix.
   DatamatrixRelativeExample(MilSystem);

   return 0;
   }

//*****************************************************************************
// The data matrix feature based registration example.
//*****************************************************************************
int DatamatrixRelativeExample(MIL_ID MilSystem)
   {
   MosPrintf(MIL_TEXT("[REGISTRATION USING DATA MATRIX FEATURES]\n")
             MIL_TEXT("The 3d points corresponding to the corners of the data matrix\n")
             MIL_TEXT("will be used to register the 3d cameras.\n\n"));

   CDatamatrixFeatureFinder DatamatrixFeatureFinder(MilSystem);
   FeatureBasedRegistrationExample(MilSystem, DATAMATRIX_EXAMPLE_PARAMS, &DatamatrixFeatureFinder);
   return 0;
   }

//*****************************************************************************
// The grid feature based registration example.
//*****************************************************************************
int GridRelativeExample(MIL_ID MilSystem)
   {
   MosPrintf(MIL_TEXT("[REGISTRATION USING CHESSBOARD GRID FEATURES]\n")
             MIL_TEXT("The 3d points corresponding to the intersections of the calibration grid\n")
             MIL_TEXT("will be used to register the 3d cameras.\n\n"));

   CGridFeatureFinder GridFeatureFinder(MilSystem);
   FeatureBasedRegistrationExample(MilSystem, GRID_EXAMPLE_PARAMS, &GridFeatureFinder);
   return 0;
   }

//*****************************************************************************
// The feature based registration example.
//*****************************************************************************
int FeatureBasedRegistrationExample(MIL_ID MilSystem,
                                    const SExampleParams& Params,
                                    IFeatureFinder* pFeatureFinder)
   {
   // Restore the containers from files.
   MIL_UNIQUE_BUF_ID PointClouds[NB_POINT_CLOUDS];
   for(MIL_INT i = 0; i < NB_POINT_CLOUDS; i++)
      {
      MIL_STRING PointCloudFile = Params.FileName + M_TO_STRING(i) + MIL_TEXT(".mbufc");
      CheckForRequiredMILFile(PointCloudFile);
      PointClouds[i] = MbufImport(PointCloudFile, M_DEFAULT, M_RESTORE, MilSystem, M_UNIQUE_ID);
      }

   // Allocate the display.
   auto Display = Alloc3dDisplayId(MilSystem);
   MIL_ID GraList = (MIL_ID)M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3ddispSetView(Display, M_AUTO, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Set the color of the point annotations.
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_COLOR, POINT_COLOR);

   std::vector<MIL_DOUBLE> ImagePointsX(4), ImagePointsY(4);
   std::vector<MIL_FLOAT> TargetPointsX, TargetPointsY, TargetPointsZ;
   std::vector<MIL_FLOAT> SourcePointsX, SourcePointsY, SourcePointsZ;

   auto Matrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   for(MIL_INT i = 0; i < NB_POINT_CLOUDS; i++)
      {
      // Display the point cloud.
      M3ddispControl(Display, M_UPDATE, M_DISABLE);
      M3dgraRemove(GraList, M_ALL, M_DEFAULT);
      M3ddispSelect(Display, PointClouds[i], M_DEFAULT, M_DEFAULT);
      if(i == 0)
         {
         M3ddispSetView(Display, M_ZOOM, Params.ViewZoom, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         M3ddispSetView(Display, M_TRANSLATE, Params.ViewTx, Params.ViewTy, Params.ViewTz, M_DEFAULT);
         }

      M3ddispControl(Display, M_TITLE, MIL_TEXT("Point Cloud ") + M_TO_STRING(i + 1) + MIL_TEXT("/") + M_TO_STRING(NB_POINT_CLOUDS));
      M3ddispControl(Display, M_UPDATE, M_ENABLE);

      if(pFeatureFinder->FindFeatures(PointClouds[i], &ImagePointsX, &ImagePointsY))
         {
         // Get the world points in the container from the image points.
         M3dimGetList(PointClouds[i], M_COMPONENT_RANGE, M_DEFAULT, ImagePointsX, ImagePointsY, M_BILINEAR, SourcePointsX, SourcePointsY, SourcePointsZ, M_NULL);

         // Display the data matrix points.
         M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_THICKNESS, POINT_THICKNESS);
         M3dgraDots(GraList, M_ROOT_NODE, M_DEFAULT, SourcePointsX, SourcePointsY, SourcePointsZ, M_NULL, M_NULL, M_NULL, M_DEFAULT);
         M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_THICKNESS, M_DEFAULT);

         // The alignment is done relative to the first point cloud so we will put the source points as the target points.
         if(i == 0)
            {
            TargetPointsX = SourcePointsX;
            TargetPointsY = SourcePointsY;
            TargetPointsZ = SourcePointsZ;
            }

         // Create buffers on the points.
         void* SourceAddress[3] = {SourcePointsX.data(), SourcePointsY.data(), SourcePointsZ.data()};
         void* TargetAddress[3] = {TargetPointsX.data(), TargetPointsY.data(), TargetPointsZ.data()};
         auto Source = MbufCreateColor(MilSystem, 3, SourcePointsX.size(), 1, M_FLOAT + 32, M_ARRAY, M_HOST_ADDRESS + M_PITCH, SourcePointsX.size(), SourceAddress, M_UNIQUE_ID);
         auto Target = MbufCreateColor(MilSystem, 3, SourcePointsX.size(), 1, M_FLOAT + 32, M_ARRAY, M_HOST_ADDRESS + M_PITCH, SourcePointsX.size(), TargetAddress, M_UNIQUE_ID);

         // Calculate the transformation from source points to target points.
         M3dimFindTransformation(M_FIND_TRANSFORMATION_CONTEXT_RIGID, Source, Target, Matrix, M_DEFAULT);
         if(M3dgeoInquire(Matrix, M_RIGID, M_NULL))
            {
            MosPrintf(MIL_TEXT("Points cloud %i/%i: The points are shown in green. Press <Enter> to continue.\r"), i + 1, NB_POINT_CLOUDS);
            MosGetch();
            }
         else
            {
            MosPrintf(MIL_TEXT("Points cloud %i/%i: Could not locate enough points."));
            MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
            MosGetch();
            return 0;
            }

         // Apply the transformation. Disable updates to not see the transformation in the 3d display.
         M3ddispControl(Display, M_UPDATE, M_DISABLE);
         M3dimMatrixTransform(PointClouds[i], PointClouds[i], Matrix, M_DEFAULT);
         }
      else
         {
         MosPrintf(MIL_TEXT("Unable to read the code!\n"));
         MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
         MosGetch();
         return 0;
         }
      }

   // Merge the point clouds and display the result.
   auto MergedCloud = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   M3dimMerge(PointClouds, MergedCloud, NB_POINT_CLOUDS, M_NULL, M_DEFAULT);

   M3ddispControl(Display, M_UPDATE, M_DISABLE);
   M3dgraRemove(GraList, M_ALL, M_DEFAULT);
   M3ddispSelect(Display, MergedCloud, M_DEFAULT, M_DEFAULT);
   M3ddispControl(Display, M_TITLE, MIL_TEXT("Merged Point Cloud"));
   M3ddispSetView(Display, M_VIEW_MATRIX + M_COMPOSE_WITH_CURRENT, Matrix, M_DEFAULT, M_DEFAULT, M_DEFAULT); // Follow the transformation.
   M3ddispControl(Display, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("\n\n"));
   MosPrintf(MIL_TEXT("The points are used to register and merge the point clouds.\n"));
   MosPrintf(MIL_TEXT("The merged result is displayed.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   return 0;
   }

//****************************************************************************
// Check for required files to run the example.
//****************************************************************************
void CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to exit.\n\n"));
      MosGetch();
      exit(0);
      }
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.  
//*****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press <Enter> to exit.\n"));
      MosGetch();
      exit(0);
      }
   return MilDisplay3D;
   }
