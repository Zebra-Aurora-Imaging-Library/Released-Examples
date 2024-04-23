﻿//***************************************************************************************
// 
// File name: PointCloudProjection.cpp  
//
// Synopsis: This program contains an example of 3D projection into a depth map 
//           using the 3D image processing module.
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
   MosPrintf(MIL_TEXT("PointCloudProjection\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how "));
   MosPrintf(MIL_TEXT("to create a depth map and how to fixture a 3D\n")
             MIL_TEXT("scan to a plane.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, 3D Image Processing, 3D Metrology,\n")
             MIL_TEXT("3D Display, Display, Buffer, and 3D Graphics. \n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_STRING POINT_CLOUD_FILE = M_IMAGE_PATH MIL_TEXT("PointCloudProjection/PointCloudScan.mbufc");

//****************************************************************************
// Function Declaration.
//****************************************************************************
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);
bool   CheckForRequiredMILFile(MIL_STRING FileName);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
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

   MIL_UNIQUE_SYS_ID    MilSystem ;        // System identifier.
   MIL_UNIQUE_DISP_ID   MilDisplay2d;      // 2D Mil Display.
   MIL_ID               MilDisplay3d;      // 3D Mil Display.
   MIL_UNIQUE_BUF_ID    MilPointCloud;     // 3D point cloud.
   MIL_UNIQUE_3DIM_ID   MilMapSizeContext; // Context for calculating the depthmap sizes.
   MIL_UNIQUE_3DMET_ID  MilFitResult;
   MIL_UNIQUE_3DGEO_ID  MilPlane;
   MIL_UNIQUE_3DGEO_ID  MilMatrix;

   // Allocate MIL objects. 
   MilSystem         = MsysAlloc  (M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MilDisplay2d      = MdispAlloc (MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MilDisplay3d      = Alloc3dDisplayId(MilSystem);
   MilMapSizeContext = M3dimAlloc (MilSystem, M_CALCULATE_MAP_SIZE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   MilPlane          = M3dgeoAlloc(MilSystem, M_GEOMETRY                  , M_DEFAULT, M_UNIQUE_ID);
   MilMatrix         = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX     , M_DEFAULT, M_UNIQUE_ID);
   MilFitResult      = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Control 2d display settings.
   MdispControl(MilDisplay2d, M_WINDOW_INITIAL_POSITION_X, 800);

   // Generate Lut function for the 2d display.
   MIL_UNIQUE_BUF_ID  MilLut = MbufAllocColor(MilSystem, 3, 255 + 1, 1, 8 + M_UNSIGNED, M_LUT, M_UNIQUE_ID);
   MgenLutFunction(MilLut, M_COLORMAP_TURBO + M_LAST_GRAY, M_DEFAULT, M_RGB888(250, 250, 250), M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MdispLut(MilDisplay2d, MilLut);

   // Restore the point cloud.
   MilPointCloud = MbufRestore(POINT_CLOUD_FILE, MilSystem, M_UNIQUE_ID);

   // Display the point cloud.
   MIL_ID MilGraphicList = M_NULL;
   if(MilDisplay3d)
      {
      M3ddispInquire(MilDisplay3d, M_3D_GRAPHIC_LIST_ID, &MilGraphicList);
      MIL_INT64 MilContainerGraphics = M3ddispSelect(MilDisplay3d, MilPointCloud, M_SELECT, M_DEFAULT);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_USE_LUT  , M_TRUE);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
      MosPrintf(MIL_TEXT("A 3D point cloud is restored from file and displayed.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }

   // Calculate the size required for the depth map.
   MIL_INT DepthMapSizeX = 0;
   MIL_INT DepthMapSizeY = 0;
   // Set the pixel size aspect ratio to be unity.
   MIL_DOUBLE PixelAspectRatio = 1.0; 

   M3dimControl(MilMapSizeContext, M_CALCULATE_MODE, M_ORGANIZED);
   M3dimControl(MilMapSizeContext, M_PIXEL_ASPECT_RATIO, PixelAspectRatio);
   M3dimCalculateMapSize(MilMapSizeContext, MilPointCloud, M_NULL, M_DEFAULT, &DepthMapSizeX, &DepthMapSizeY);

   MosPrintf(MIL_TEXT("The depth map's size is calculated based on the point cloud:\n"));
   MosPrintf(MIL_TEXT("M_SIZE_X is %i and M_SIZE_Y is %i.\n\n"), DepthMapSizeX, DepthMapSizeY);
   MIL_UNIQUE_BUF_ID MilDepthMap = MbufAlloc2d(MilSystem, DepthMapSizeX, DepthMapSizeY, M_UNSIGNED + 8, M_IMAGE | M_PROC | M_DISP, M_UNIQUE_ID);

   // Calibrate the depth map based on the given point cloud.
   M3dimCalibrateDepthMap(MilPointCloud, MilDepthMap, M_NULL, M_NULL, PixelAspectRatio, M_DEFAULT, M_DEFAULT);

   // Project the point cloud in a point based mode.
   M3dimProject(MilPointCloud, MilDepthMap, M_NULL, M_POINT_BASED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Display the depthmap.
   MdispSelect(MilDisplay2d, MilDepthMap);
   MosPrintf(MIL_TEXT("The 3D point cloud is projected based on its points into the depth map.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("A plane is fit to the point cloud.\n\n"));

   M3dmetFit(M_DEFAULT, MilPointCloud, M_PLANE, MilFitResult, 10, M_DEFAULT);
   M3dmetCopyResult(MilFitResult, MilPlane, M_FITTED_GEOMETRY, M_DEFAULT);
   MIL_INT64 MilLabel = 0;
   if(MilDisplay3d)
      {
      MilLabel = M3dgeoDraw3d(M_DEFAULT, MilPlane, MilGraphicList, M_DEFAULT, M_DEFAULT);
      M3dgraControl(MilGraphicList, MilLabel, M_OPACITY, 60);
      }

   MosPrintf(MIL_TEXT("The point cloud is fixtured to the fit plane.\n"));
   M3dgeoMatrixSetTransform(MilMatrix, M_FIXTURE_TO_PLANE, MilPlane, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3dimMatrixTransform(MilPointCloud, MilPointCloud, MilMatrix, M_DEFAULT);

   if(MilDisplay3d)
      {
      M3dgraCopy(MilMatrix, M_DEFAULT, MilGraphicList, MilLabel, M_TRANSFORMATION_MATRIX + M_COMPOSE_WITH_CURRENT, M_DEFAULT);
      M3ddispSetView(MilDisplay3d, M_VIEW_BOX, M_WHOLE_SCENE, 1.0, M_DEFAULT, M_DEFAULT);
      }

   M3dimCalculateMapSize(MilMapSizeContext, MilPointCloud, M_NULL, M_DEFAULT, &DepthMapSizeX, &DepthMapSizeY);
   MilDepthMap.reset();
   MilDepthMap = MbufAlloc2d(MilSystem, DepthMapSizeX, DepthMapSizeY, M_UNSIGNED + 8, M_IMAGE | M_PROC | M_DISP, M_UNIQUE_ID);

   // Calibrate the depth map.
   M3dimCalibrateDepthMap(MilPointCloud, MilDepthMap, M_NULL, M_NULL, PixelAspectRatio, M_DEFAULT, M_DEFAULT);

   // Project the point cloud in a point based mode.
   M3dimProject(MilPointCloud, MilDepthMap, M_NULL, M_POINT_BASED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Display the depthmap.
   MdispSelect(MilDisplay2d, MilDepthMap);
   MosPrintf(MIL_TEXT("The point cloud is projected into a depth map in the scan's plane.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Create an arbitary sized depthmap.
   MIL_INT NewSizeX = 640;
   MIL_INT NewSizeY = 640;
   MIL_UNIQUE_BUF_ID MilLargeDepthMap = MbufAlloc2d(MilSystem, NewSizeX, NewSizeY, M_UNSIGNED + 8, M_IMAGE | M_PROC | M_DISP, M_UNIQUE_ID);

   // Calibrate the depth map.
   M3dimCalibrateDepthMap(MilPointCloud, MilLargeDepthMap, M_NULL, M_NULL, PixelAspectRatio, M_DEFAULT, M_DEFAULT);

   // Project the point cloud in point based mode. 
   M3dimProject(MilPointCloud, MilLargeDepthMap, M_NULL, M_POINT_BASED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MdispSelect(MilDisplay2d, MilLargeDepthMap);
   MosPrintf(MIL_TEXT("The point cloud is projected into a much larger depth map.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // The projected depthmap has some invalid values
   // To fill the invalid vaules with neighbouring values we can use M3dimFillGaps with default options.
   M3dimFillGaps(M_DEFAULT, MilLargeDepthMap, M_NULL, M_DEFAULT);
   MdispSelect(MilDisplay2d, MilLargeDepthMap);
   MosPrintf(MIL_TEXT("Invalid values are filled with default values, using M3dimFillGaps.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Control the options of the fill gap context to yield better results.
   MIL_UNIQUE_3DIM_ID FillGapsContext = M3dimAlloc(MilSystem, M_FILL_GAPS_CONTEXT , M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(FillGapsContext, M_FILL_THRESHOLD_X, 2);
   M3dimControl(FillGapsContext, M_FILL_THRESHOLD_Y, 2);
   M3dimControl(FillGapsContext, M_INPUT_UNITS     , M_PIXEL);

   M3dimProject (MilPointCloud  , MilLargeDepthMap, M_NULL, M_POINT_BASED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3dimFillGaps(FillGapsContext, MilLargeDepthMap, M_NULL, M_DEFAULT);
   MdispSelect(MilDisplay2d, MilLargeDepthMap);
   MosPrintf(MIL_TEXT("Invalid values are filled using M3dimFillGaps with threshold options.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // With arbitrary sized depthmap, the projection in mesh-based mode yields better results.
   // The given point cloud is organized, so the mesh can be created in mesh orgainzed mode.
   MIL_UNIQUE_BUF_ID MilMeshedContainer = MbufAllocContainer(MilSystem, M_PROC | M_DISP, M_DEFAULT, M_UNIQUE_ID);
   MosPrintf(MIL_TEXT("A mesh component is created and added to the point cloud.\n\n"));
   MIL_UNIQUE_3DIM_ID MilMeshContext = M3dimAlloc(MilSystem, M_MESH_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MilMeshContext, M_MAX_DISTANCE, 5);
   M3dimControl(MilMeshContext, M_MESH_MODE, M_MESH_ORGANIZED);
   M3dimMesh(MilMeshContext, MilPointCloud, MilMeshedContainer, M_DEFAULT);

   // Update the 3D display with the meshed point cloud.
   if(MilDisplay3d)
      {
      MIL_INT64 MilContainerGraphics = M3ddispSelect(MilDisplay3d, MilMeshedContainer, M_SELECT, M_DEFAULT);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_USE_LUT  , M_TRUE);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
      }

   // Project the point-cloud in a mesh based mode.
   M3dimProject(MilMeshedContainer, MilLargeDepthMap, M_NULL, M_MESH_BASED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MdispSelect(MilDisplay2d, MilLargeDepthMap);
   MosPrintf(MIL_TEXT("The point cloud is projected based on its mesh into a depth map.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   // Free allocated objects.
   if(MilDisplay3d)
      M3ddispFree(MilDisplay3d);
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
                MIL_TEXT("Press any key to continue.\n"));
      MosGetch();
      }
   return MilDisplay3D;
  }

//****************************************************************************
// Check for required files to run the example.
//****************************************************************************
bool CheckForRequiredMILFile(MIL_STRING FileName)
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
