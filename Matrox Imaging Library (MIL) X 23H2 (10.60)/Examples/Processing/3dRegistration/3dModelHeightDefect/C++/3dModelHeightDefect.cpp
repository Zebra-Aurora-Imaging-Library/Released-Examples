﻿//***************************************************************************************/
// 
// File name: 3dModelHeightDefect.cpp  
//
// Synopsis:  This program contains an example of 3D surface registration followed 
//            by defect detection using the 3dreg/3dmet modules.
//            See the PrintHeader() function below for detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/
#include <mil.h>
#include <math.h>
#include <stdlib.h>

//--------------------------------------------------------------------------
// Example description.
//--------------------------------------------------------------------------
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("3dModelHeightDefect\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to use the 3D surface registration   \n"));
   MosPrintf(MIL_TEXT("operation to register the acquired point cloud of a 3D object with \n"));
   MosPrintf(MIL_TEXT("its 3D reference model in order to detect defects.                 \n"));
   MosPrintf(MIL_TEXT("\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: 3D Registration, 3D Image Processing, 3D Metrology, Buffer, \n")
             MIL_TEXT("3D Display, 3D Graphics, Image Processing, and Blob.\n\n"));
   }

// Functions declarations.
void   Draw3dBoxes(MIL_ID MilSystem, MIL_ID MilResult, MIL_ID MilDisplay[3]);
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);
bool   CheckForRequiredMILFile(const MIL_STRING& FileName);

MIL_UNIQUE_3DGEO_ID FindDefect(MIL_ID MilSystem, MIL_ID DefectPointCloud, MIL_ID MilDefectDistance, MIL_ID MilGraphicList);

// Enumerators definitions.
enum { eModel = 0, eObject = 1, eDistance = 2, eDefects = 3 };

// Input data files.
static const MIL_TEXT_CHAR* const FILE_MODEL_POINT_CLOUD  = M_IMAGE_PATH MIL_TEXT("3dModelHeightDefect/3dModel.ply");
static const MIL_TEXT_CHAR* const FILE_OBJECT_POINT_CLOUD = M_IMAGE_PATH MIL_TEXT("3dModelHeightDefect/3dObject.mbufc");

static const MIL_INT    DISP_SIZE_X = 380;
static const MIL_INT    DISP_SIZE_Y = 420;

// Registration context controls definitions.
static const MIL_DOUBLE GRID_SIZE = 1.5;
static const MIL_DOUBLE OVERLAP = 95.0; // %
static const MIL_INT    MAX_ITERATIONS = 20;
static const MIL_DOUBLE RMS_ERROR_RELATIVE_THRESHOLD = 1.0;  // %

// Verification constant.
static const MIL_FLOAT  TOLERANCE = 1.0f;

// LUT.
static const MIL_INT    NUM_LUT_VALUES = MIL_UINT8_MAX;
static const MIL_DOUBLE BOX_MIN_X = -3.0;
static const MIL_DOUBLE BOX_MIN_Y = -10.0;
static const MIL_DOUBLE BOX_MIN_Z = -53.0;
static const MIL_DOUBLE BOX_MAX_X = 160.0;
static const MIL_DOUBLE BOX_MAX_Y = 190.0;
static const MIL_DOUBLE BOX_MAX_Z = 13.0;

//--------------------------------------------------------------------------
int MosMain()
   {
   // Print example information in console.
   PrintHeader();

   //--------------------------------------------------------------------------
   // Allocate MIL objects. 
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_ID MilSystem = M_DEFAULT_HOST;

   if(!CheckForRequiredMILFile(FILE_OBJECT_POINT_CLOUD))
      { return 0; }

   // 3D registration and defect detection.
   MIL_UNIQUE_3DREG_ID MilContext = M3dregAlloc      (MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_3DREG_ID MilResult  = M3dregAllocResult(MilSystem, M_PAIRWISE_REGISTRATION_RESULT, M_DEFAULT, M_UNIQUE_ID);

   MIL_ID MilDisplay[4];      // Display for model and object, and defects distance map.
   MIL_ID MilGraphicList[4];  // Graphic list for model and object, and defects distance map.
   MIL_ID MilPointCloud[4];   // Model and object point cloud containers.
   MilPointCloud[eDistance] = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_NULL);
   MilPointCloud[eDefects] = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_NULL);

   //-------------------------------------------------------------------------------------------
   // Import 3D model from PLY file.
   MbufImport(FILE_MODEL_POINT_CLOUD, M_DEFAULT, M_RESTORE, MilSystem, &MilPointCloud[eModel]);
   MosPrintf(MIL_TEXT("The model's 3D point cloud is imported from a PLY file.\n"));

   // Import 3D object from mbufc files.
   MbufImport(FILE_OBJECT_POINT_CLOUD, M_DEFAULT, M_RESTORE, MilSystem, &MilPointCloud[eObject]);
   MosPrintf(MIL_TEXT("The object's 3D point cloud is imported from an mbufc file.\n\n"));

   //-------------------------------------------------------------------------------
   // Initialize displays that will show model and object point clouds.
   const MIL_INT NumDisplays = 4;
   for(MIL_INT i = 0; i < NumDisplays; i++)
      {
      // Allocate the display.
      MilDisplay[i] = Alloc3dDisplayId(MilSystem);
      M3ddispInquire(MilDisplay[i], M_3D_GRAPHIC_LIST_ID, &MilGraphicList[i]);
      M3ddispControl(MilDisplay[i], M_SIZE_X, DISP_SIZE_X);
      M3ddispControl(MilDisplay[i], M_SIZE_Y, DISP_SIZE_Y);
      M3ddispControl(MilDisplay[i], M_WINDOW_INITIAL_POSITION_X, (MIL_INT)(i * 1.04 * DISP_SIZE_X));
      M3ddispSetView(MilDisplay[i], M_AUTO, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      // Add titles to displays.
      switch(i)
         {
         case eModel:
            M3ddispControl(MilDisplay[eModel], M_TITLE, MIL_TEXT("Model Cloud"));
            break;
         case eObject:
            M3ddispControl(MilDisplay[eObject], M_TITLE, MIL_TEXT("Object Cloud"));
            break;
         case eDistance:
            M3ddispControl(MilDisplay[eDistance], M_TITLE, MIL_TEXT("Distance Map Cloud"));
            break;
         case eDefects:
            M3ddispControl(MilDisplay[eDefects], M_TITLE, MIL_TEXT("Defect Map Cloud"));
            break;
         }

      // Select the point clouds.
      if(i == eModel || i == eObject)
         {
         M3ddispControl(MilDisplay[i], M_UPDATE, M_DISABLE);
         MIL_INT64 ContainerGraphics = M3ddispSelect(MilDisplay[i], MilPointCloud[i], M_SELECT, M_DEFAULT);
         M3dgraCopy(M_COLORMAP_TURBO + M_FLIP, M_DEFAULT, MilGraphicList[i], ContainerGraphics, M_COLOR_LUT, M_DEFAULT);
         M3dgraControl(MilGraphicList[i], ContainerGraphics, M_COLOR_USE_LUT, M_TRUE);
         M3dgraControl(MilGraphicList[i], ContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
         M3dgraControl(MilGraphicList[i], ContainerGraphics, M_COLOR_COMPONENT_BAND, 2);
         M3ddispControl(MilDisplay[i], M_UPDATE, M_ENABLE);
         }
      }
   MosPrintf(MIL_TEXT("The model and object are displayed using pseudo colors.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MIL_INT NumPoints[2];
   MIL_UNIQUE_3DIM_ID ResultId = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   for(MIL_INT i = 0; i < 2; i++)
      {
      // Get the number of 3D points in each point cloud containers.
      M3dimStat(M_STAT_CONTEXT_NUMBER_OF_POINTS,MilPointCloud[i], ResultId, M_DEFAULT );
      M3dimGetResult(ResultId, M_NUMBER_OF_POINTS_VALID, &NumPoints[i]);
      }

   //--------------------------------------------------------------------------
   // 3D registration.

   // Subsampling context of the registration.
   MIL_ID MilRegSubsampleContext = M_NULL;
   M3dregInquire(MilContext, M_DEFAULT, M_SUBSAMPLE_CONTEXT_ID, &MilRegSubsampleContext);

   // Set the controls of the subsampling that will be used during the registration process.
   M3dimControl(MilRegSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GRID);
   M3dimControl(MilRegSubsampleContext, M_GRID_SIZE_X, GRID_SIZE);
   M3dimControl(MilRegSubsampleContext, M_GRID_SIZE_Y, GRID_SIZE);
   M3dimControl(MilRegSubsampleContext, M_GRID_SIZE_Z, M_INFINITE);
   M3dimControl(MilRegSubsampleContext, M_ORGANIZATION_TYPE, M_ORGANIZED);

   // Pairwise registration context controls.
   M3dregControl(MilContext, M_DEFAULT, M_SUBSAMPLE, M_ENABLE);
   M3dregControl(MilContext, M_DEFAULT, M_PREREGISTRATION_MODE, M_CENTROID);
   M3dregControl(MilContext, eObject, M_OVERLAP, OVERLAP);
   M3dregControl(MilContext, M_DEFAULT, M_MAX_ITERATIONS, MAX_ITERATIONS);
   M3dregControl(MilContext, M_DEFAULT, M_RMS_ERROR_RELATIVE_THRESHOLD, RMS_ERROR_RELATIVE_THRESHOLD);

   // Registration.
   MIL_INT RegistrationStatus = M_NULL;
   MIL_DOUBLE ComputationTime = 0.0;

   MappTimer(M_TIMER_RESET, M_NULL);

   M3dregCalculate(MilContext,
                   MilPointCloud,
                   2,    
                   MilResult,
                   M_DEFAULT );

   MappTimer(M_TIMER_READ, &ComputationTime);

   MosPrintf(MIL_TEXT("The 3D registration between the model and the object has been performed.\n\n"));

   M3dregGetResult(MilResult, eObject, M_STATUS_REGISTRATION_ELEMENT, &RegistrationStatus);

   // Interpret the result status.
   switch(RegistrationStatus)
      {
      case M_NOT_INITIALIZED:
         MosPrintf(MIL_TEXT("Registration failed: the registration result is not initialized.\n\n"));
         break;
      case M_NOT_ENOUGH_POINT_PAIRS:
         MosPrintf(MIL_TEXT("Registration failed: point clouds are not overlaping.\n\n"));
         break;
      case M_MAX_ITERATIONS_REACHED:
         MosPrintf(MIL_TEXT("Registration reached the maximum number of iterations allowed (%d)\n")
                   MIL_TEXT("in %.2f ms. Resulting registration may or may not be valid.\n\n"),
                   MAX_ITERATIONS, ComputationTime * 1000);
         break;
      case M_RMS_ERROR_THRESHOLD_REACHED:
      case M_RMS_ERROR_RELATIVE_THRESHOLD_REACHED:
         MIL_DOUBLE RegisterRmsError;
         M3dregGetResult(MilResult, eObject, M_RMS_ERROR + M_TYPE_MIL_DOUBLE, &RegisterRmsError);
         MosPrintf(MIL_TEXT("The registration of %d model points with %d object points\n")
                   MIL_TEXT("succeeded in %.2f ms with a final RMS error of %f mm.\n\n"),
                   (MIL_INT)NumPoints[eModel], (MIL_INT)NumPoints[eObject], ComputationTime * 1000, RegisterRmsError);
         break;
      default:
         MosPrintf(MIL_TEXT("Unknown registration status.\n\n"));
      }

   // Draw 3D box in each display to visualize the object pose obtained from registration.
   Draw3dBoxes(MilSystem, MilResult, MilDisplay);

   MosPrintf(MIL_TEXT("3D boxes are drawn to highlight the 3D pose estimation of the object\n")
             MIL_TEXT("relative to the model.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //--------------------------------------------------------------------------
   // Registration and defects highlighting.

   // Use M3dregMerge to register object's point cloud with model's point cloud.
   // The model cloud is at the M_REGISTRATION_GLOBAL, so we need to register only the object.
   MIL_ID Scene[2] = { M_NULL, MilPointCloud[eObject] };
   M3dregMerge(MilResult, Scene, 2, MilPointCloud[eDistance], M_NULL, M_DEFAULT);

   // Convert the point clouds into organized point clouds, in order to get an organized defect map.
   // Organized map is needed for blob analysis.
   MIL_UNIQUE_3DIM_ID MilSubsampleContext = M3dimAlloc(MilSystem, M_SUBSAMPLE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GRID);
   M3dimControl(MilSubsampleContext, M_ORGANIZATION_TYPE, M_ORGANIZED);
   M3dimControl(MilSubsampleContext, M_GRID_SIZE_Z, M_INFINITE);
   M3dimControl(MilSubsampleContext, M_GRID_SIZE_X, 0.2);
   M3dimControl(MilSubsampleContext, M_GRID_SIZE_Y, 0.2);
   M3dimSample(MilSubsampleContext, MilPointCloud[eDistance], MilPointCloud[eDistance], M_DEFAULT);
   M3dimSample(MilSubsampleContext, MilPointCloud[eModel], MilPointCloud[eModel], M_DEFAULT);

   // Add the defect distance as a user component of eDistance.
   MIL_INT64 DefectsComponentType = M_COMPONENT_CUSTOM;
   MIL_INT SizeX = MbufInquireContainer(MilPointCloud[eDistance],M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilPointCloud[eDistance],M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   MIL_ID MilDefectDistance = MbufAllocComponent(MilPointCloud[eDistance], 1,SizeX ,SizeY , M_FLOAT + 32, M_IMAGE + M_PROC, DefectsComponentType, M_NULL);

   // Compute the distances.
   M3dmetDistance(MilPointCloud[eDistance], MilPointCloud[eModel], MilDefectDistance, M_DISTANCE_TO_NEAREST_NEIGHBOR, M_DEFAULT, M_DEFAULT);

   // Display distance cloud.
   M3ddispControl(MilDisplay[eDistance], M_UPDATE, M_DISABLE);
   MIL_INT64 DistanceGraphics = M3ddispSelect(MilDisplay[eDistance], MilPointCloud[eDistance], M_SELECT, M_DEFAULT);
   M3dgraControl(MilGraphicList[eDistance], DistanceGraphics, M_COLOR_USE_LUT, M_TRUE);
   M3dgraControl(MilGraphicList[eDistance], DistanceGraphics, M_COLOR_COMPONENT, DefectsComponentType);
   M3dgraControl(MilGraphicList[eDistance], DistanceGraphics, M_COLOR_COMPONENT_BAND, 0);
   M3ddispControl(MilDisplay[eDistance], M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("The resulting distance map is displayed using pseudo colors.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Find the position of the defect by doing a blob analysis on the distance map.
   auto MilDefectBox = FindDefect(MilSystem, MilPointCloud[eDistance], MilDefectDistance, MilGraphicList[eDistance]);

   // Crop the defect point cloud.
   M3dimCrop(MilPointCloud[eDistance], MilPointCloud[eDefects], MilDefectBox, M_NULL, M_SHRINK, M_APPLY_TO_ALL_COMPONENTS);

   // Display the defect point cloud.
   M3ddispControl(MilDisplay[eDefects], M_UPDATE, M_DISABLE);
   MIL_INT64 DefectsGraphics = M3ddispSelect(MilDisplay[eDefects], MilPointCloud[eDefects], M_SELECT, M_DEFAULT);
   M3dgraControl(MilGraphicList[eDefects], DefectsGraphics, M_COLOR_USE_LUT, M_TRUE);
   M3dgraControl(MilGraphicList[eDefects], DefectsGraphics, M_COLOR_COMPONENT, DefectsComponentType);
   M3dgraControl(MilGraphicList[eDefects], DefectsGraphics, M_COLOR_COMPONENT_BAND, 0);
   M3ddispControl(MilDisplay[eDefects], M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("Blob analysis is performed on the distance map to find the location\n")
             MIL_TEXT("of the defect.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   //--------------------------------------------------------------------------
  // Free MIL objects.    
   for(MIL_INT i = 0; i < NumDisplays; i++)
      { M3ddispFree(MilDisplay[i]); }

   for(MIL_INT i = 0; i < 4; ++i)
      { MbufFree(MilPointCloud[i]); }
   }

//--------------------------------------------------------------------------
// Draw 3D boxes in model and object displays to illustrate the pose.
void Draw3dBoxes(MIL_ID MilSystem,
                 MIL_ID MilResult,
                 MIL_ID MilDisplay[3])
   {
   MIL_UNIQUE_3DGEO_ID MilBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoBox(MilBox, M_BOTH_CORNERS, BOX_MIN_X, BOX_MIN_Y, BOX_MIN_Z, BOX_MAX_X, BOX_MAX_Y, BOX_MAX_Z, M_DEFAULT);
   MIL_ID MilGraphicList = M_NULL;
   M3ddispInquire(MilDisplay[eModel], M_3D_GRAPHIC_LIST_ID, &MilGraphicList);
   MIL_INT64 MilBoxGraphics = M3dgeoDraw3d(M_DEFAULT, MilBox, MilGraphicList,M_ROOT_NODE,  M_DEFAULT);
   M3dgraControl(MilGraphicList, MilBoxGraphics, M_APPEARANCE, M_WIREFRAME);
   M3dgraControl(MilGraphicList, MilBoxGraphics, M_COLOR, M_COLOR_YELLOW);

   MIL_UNIQUE_3DGEO_ID MilMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dregCopyResult(MilResult, eModel, eObject, MilMatrix, M_REGISTRATION_MATRIX, M_DEFAULT);

   M3dimMatrixTransform(MilBox, MilBox, MilMatrix, M_DEFAULT);
   M3ddispInquire(MilDisplay[eObject], M_3D_GRAPHIC_LIST_ID, &MilGraphicList);
   MIL_INT64 MilBoxGraphics2 = M3dgeoDraw3d(M_DEFAULT, MilBox, MilGraphicList, M_ROOT_NODE,  M_DEFAULT);
   M3dgraControl(MilGraphicList, MilBoxGraphics2, M_APPEARANCE, M_WIREFRAME);
   M3dgraControl(MilGraphicList, MilBoxGraphics2, M_COLOR, M_COLOR_YELLOW);
   }

//--------------------------------------------------------------------------
// Find the position of the defect in the point cloud.
//--------------------------------------------------------------------------
MIL_UNIQUE_3DGEO_ID FindDefect(MIL_ID MilSystem, MIL_ID DefectPointCloud, MIL_ID MilDefectDistance, MIL_ID MilGraphicList)
   {
   auto MilBoxId   = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   auto DefectOnly = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);

   MbufCopyComponent(DefectPointCloud, DefectOnly, M_COMPONENT_ALL, M_REPLACE, M_DEFAULT);

   // Binarize the distance map and put it as the confidecne for the defect cloud.
   MIL_ID DefectConfidence = MbufInquireContainer(DefectOnly, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   MimBinarize(MilDefectDistance, DefectConfidence, M_FIXED + M_IN_RANGE, 2, 100);

   // Perform blob analysis on the defect distance map.
   MIL_UNIQUE_BLOB_ID MilBlobContext = MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_BLOB_ID MilBlobResult = MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   MblobControl(MilBlobContext, M_SORT1, M_AREA);
   MblobControl(MilBlobContext, M_SORT1_DIRECTION, M_SORT_DOWN);
   MblobControl(MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);
   MblobCalculate(MilBlobContext, DefectConfidence, M_NULL, MilBlobResult);
   MbufClear(DefectConfidence, 0.0);
   MblobDraw(M_DEFAULT, MilBlobResult, DefectConfidence, M_DRAW_BLOBS, M_BLOB_INDEX(0), M_DEFAULT);

   // Find the bounding box of the defect detected by blob analysis.
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, DefectOnly, MilBoxId, M_DEFAULT);
   MIL_INT64 MilBoxGraphics = M3dgeoDraw3d(M_DEFAULT, MilBoxId, MilGraphicList, M_ROOT_NODE, M_DEFAULT);
   M3dgraControl(MilGraphicList, MilBoxGraphics, M_APPEARANCE, M_WIREFRAME);
   M3dgraControl(MilGraphicList, MilBoxGraphics, M_COLOR, M_COLOR_WHITE);

   return MilBoxId;
   }

//--------------------------------------------------------------------------
// Creates a 3D display and returns its MIL identifier.
//--------------------------------------------------------------------------
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                  MIL_TEXT("The current system does not support the 3D display.\n")
                  MIL_TEXT("Press any key to exit..\n"));
      MosGetch();
      exit(0);
      }

   return MilDisplay3D;
   }

//--------------------------------------------------------------------------
// Check for required files to run the example.    
//--------------------------------------------------------------------------
bool CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }
