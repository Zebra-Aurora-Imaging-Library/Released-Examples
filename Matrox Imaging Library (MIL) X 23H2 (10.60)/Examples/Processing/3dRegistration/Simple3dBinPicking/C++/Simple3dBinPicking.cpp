﻿//***************************************************************************************/
// 
// File name: Simple3dBinPicking.cpp  
//
// Synopsis:  This program contains an example of simple 3D bin picking
//            by combining the 2D Model Finder and 3D registration modules.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <math.h>
#include <stdlib.h>
#include <vector>

//--------------------------------------------------------------------------
// Example description.
//--------------------------------------------------------------------------
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Simple3dBinPicking\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example shows how to combine 2D Model Finder and \n"));
   MosPrintf(MIL_TEXT("3D registration to estimate the pose of 3D objects stacked \n"));
   MosPrintf(MIL_TEXT("with minor variations in pitch and roll. \n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: 3D Geometry, 3D Image Processing, 3D Registration,\n")
             MIL_TEXT("Geometric Model Finder, Buffer, Calibration, Display, Graphics, \n")
             MIL_TEXT("3D Display, Image Processing and 3D Metrology. \n\n"));
   }

// Enumerator to index model and scene objects (values must not change).
enum { eModel = 0, eScene = 1 };

// File names of the point clouds acquired from a 3D device such as a Gocator 3110 from LMI Technologies.
static const MIL_INT NUM_SCENE_SCANS = 3;
static const MIL_TEXT_CHAR* const FILE_POINT_CLOUD[NUM_SCENE_SCANS + 1] =
   { M_IMAGE_PATH MIL_TEXT("Simple3dBinPicking/3dPlugCloudModel.ply"),  // Equivalent index: eModel     (0)
     M_IMAGE_PATH MIL_TEXT("Simple3dBinPicking/BinCloudScene_0.ply") ,  // Equivalent index: eScene + 0 (1)
     M_IMAGE_PATH MIL_TEXT("Simple3dBinPicking/BinCloudScene_1.ply") ,  // Equivalent index: eScene + 1 (2)
     M_IMAGE_PATH MIL_TEXT("Simple3dBinPicking/BinCloudScene_2.ply") }; // Equivalent index: eScene + 2 (3)

// Depth map parameters.
static const MIL_INT    DEPTHMAP_SIZE_X = 300;   // in pixels
static const MIL_INT    DEPTHMAP_SIZE_Y = 480;   // in pixels
static const MIL_INT    DEPTHMAP_NUM_VALUES = 65536; // 16 bits image (2^16)
static const MIL_DOUBLE DEPTHMAP_MISSING_DATA = DEPTHMAP_NUM_VALUES - 1; // Numeric value of missing depth-map data.

// 3D scanner field of view, excluding the floor.
static const MIL_DOUBLE SCANNER_FOV_MIN_X = -44.0; // (mm)
static const MIL_DOUBLE SCANNER_FOV_MIN_Y = -80.0; // (mm)
static const MIL_DOUBLE SCANNER_FOV_MIN_Z =  -6.0; // (mm)
static const MIL_DOUBLE SCANNER_FOV_MAX_X =  50.0; // (mm)
static const MIL_DOUBLE SCANNER_FOV_MAX_Y =  80.0; // (mm)
static const MIL_DOUBLE SCANNER_FOV_MAX_Z = -60.0; // (mm)

// ROI margins to add to model's bounding box.
static const MIL_DOUBLE MODEL_ROI_MARGIN_X =  5.0; // (mm)
static const MIL_DOUBLE MODEL_ROI_MARGIN_Y =  5.0; // (mm)
static const MIL_DOUBLE MODEL_ROI_MARGIN_Z = 20.0; // (mm)

// 2d Model Finder parameters.
static const MIL_DOUBLE FINDER_ACCEPTANCE = 50.0;  // (%)

// 2d Model Finder graphic colors.
#define FOUND_OCCURRENCES_COLOR     M_RGB888(192, 0, 0)
#define SELECTED_OCCURRENCE_COLOR   M_RGB888(0, 255, 0)

// 3D registration parameters.
static const MIL_INT    DECIMATION_STEP = 4;    
static const MIL_DOUBLE OVERLAP = 90.0;         // (%)          
static const MIL_INT    MAX_ITERATIONS = 50;
static const MIL_INT    ERROR_MINIMIZATION_METRIC = M_POINT_TO_POINT;

// Structure grouping the six components of a 3D pose.
struct SPose
   { MIL_DOUBLE Tx, Ty, Tz, Rx, Ry, Rz; };

// Structure defining a 3D box and its corresponding 2d ROI in a depth-map.
struct SBox
   {
   MIL_DOUBLE MinX, MinY, MinZ, MaxX, MaxY, MaxZ; // 3D bounding intervals in each dimension.
   MIL_INT OffsetX, OffsetY, SizeX, SizeY;        // 2d ROI in the depth-map.
   };

// Utility functions declaration.
void   DefineModelRoiBox                        (MIL_ID MilSystem, MIL_ID MilPtCldCtn, MIL_ID MilDepthMap, SBox& ModelRoiBox);
void   MapDynamicRangeTo8Bits                   (MIL_ID MilSystem, MIL_ID MilSrcImage, MIL_ID MilTgtImage);
bool   FindPreregistrationWithTopFoundOccurrence(MIL_ID MilSystem, MIL_UNIQUE_BUF_ID MilPtCldCtn[], MIL_DOUBLE ModelMeanElevation, MIL_UNIQUE_BUF_ID MilDepthMap[],
                                                 MIL_ID MilFixturingOffset, MIL_ID MilFinderResult, MIL_ID MilPreregistrationMatrix, MIL_INT* pTopOccIdx);
void   ExtractRegisteredDepthMaps               (MIL_ID MilSystem, MIL_UNIQUE_BUF_ID MilPtCldCtn[], MIL_UNIQUE_BUF_ID& MilTransformedModel, MIL_UNIQUE_BUF_ID MilDepthMap[], MIL_ID MilRegisterationResult, SPose& Pose);
void   GetPose                                  (MIL_ID MilMatrixId, SPose& Pose);
bool   CheckForRequiredMILFile                  (const MIL_STRING& FileName);

MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);

// 3D display parameters.
static const MIL_DOUBLE DISPLAY_3D_EYE_AZIM = 85.0;   // (deg)
static const MIL_DOUBLE DISPLAY_3D_EYE_ELEV = 230.0;  // (deg)
static const MIL_DOUBLE DISPLAY_3D_EYE_ROLL = 0.0;    // (deg)

int MosMain()
   {
   // Print example information in console.
   PrintHeader();

   //--------------------------------------------------------------------------
   // Allocate MIL objects. 
   auto   MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_ID MilSystem      = M_DEFAULT_HOST;

   // Check the required files exist.
   if(!CheckForRequiredMILFile(FILE_POINT_CLOUD[eModel]))
      { return 0; }

   auto MilDisplay   = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   auto MilDisplay3d = Alloc3dDisplayId(MilSystem);

   MIL_UNIQUE_GRA_ID MilGraphicList; // 2D Graphic list in which to draw.
   MIL_UNIQUE_BUF_ID MilPtCldCtn[2]; // Model and scene point cloud containers.
   MIL_UNIQUE_BUF_ID MilDepthMap[2]; // Model and scene depth maps.

   MIL_DOUBLE ModelMeanElevation; // Model's mean elevation computed in depth-map.
   SBox       ModelRoiBox;        // Model's 3D bounding box and 2d ROI.

   MIL_UNIQUE_3DGEO_ID MilBox;
   MIL_UNIQUE_BUF_ID   MilFinderImage[2];           // Images used by model finder: the model and the scene (target).
   MIL_UNIQUE_MOD_ID   MilFinderContext;            // Model finder context.
   MIL_UNIQUE_MOD_ID   MilFinderResult;             // Model finder result.
   MIL_INT             TopOccurrenceIdx;            // Index of the found occurrence of top of the stack.

   MIL_UNIQUE_3DREG_ID     MilRegistrationContext;         // Pairwise registration context.
   MIL_UNIQUE_3DREG_ID     MilRegistrationResult;          // Pairwise registration result.
   MIL_UNIQUE_3DGEO_ID     MilPreregistrationMatrix;       // Pre-registration homogeneous matrix .
   MIL_INT                 RegistrationCompleted;          // Status of 3D registration.

   // Allocate and initialize the 2D display.
   MilGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //--------------------------------------------------------------------------
   // 3D Display.
   if(MilDisplay3d)
      {
      M3ddispControl(MilDisplay3d, M_WINDOW_INITIAL_POSITION_X, (MIL_INT)(1.04 * DEPTHMAP_SIZE_X));
      M3ddispSetView(MilDisplay3d, M_AZIM_ELEV_ROLL , DISPLAY_3D_EYE_AZIM, DISPLAY_3D_EYE_ELEV, DISPLAY_3D_EYE_ROLL, M_DEFAULT);
      }

   //--------------------------------------------------------------------------
   // Import model's point cloud and generate its 16 bits depth-map. 
   MbufImport(FILE_POINT_CLOUD[eModel], M_DEFAULT, M_RESTORE, MilSystem, &MilPtCldCtn[eModel]);

   // Allocate the model's depth-map.
   MilDepthMap[eModel] = MbufAlloc2d(MilSystem, DEPTHMAP_SIZE_X, DEPTHMAP_SIZE_Y, M_UNSIGNED + 16, M_IMAGE + M_DISP + M_PROC, M_UNIQUE_ID);

   // Set the model's extraction box to the scanner field of view, but removing the floor.
   MilBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoBox(MilBox, M_BOTH_CORNERS,
             SCANNER_FOV_MIN_X, SCANNER_FOV_MIN_Y, SCANNER_FOV_MIN_Z,
             SCANNER_FOV_MAX_X, SCANNER_FOV_MAX_Y, SCANNER_FOV_MAX_Z, M_DEFAULT);

   // Crop the point cloud to the extraction box, defined as the
   // scanner field of view, rejecting the floor.
   M3dimCrop(MilPtCldCtn[eModel], MilPtCldCtn[eModel],MilBox,M_NULL,M_DEFAULT,M_DEFAULT);

   MIL_INT SizeX = MbufInquireContainer(MilPtCldCtn[eModel], M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilPtCldCtn[eModel], M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   MIL_ID MilModelReflectance = MbufAllocComponent(MilPtCldCtn[eModel], 3, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC + M_DISP, M_COMPONENT_REFLECTANCE, M_NULL);//Colored reflectance
   MbufClear(MilModelReflectance, M_COLOR_GREEN);

   // Set depth-map's extraction overlap property such that points on top
   // overwrites points below.
   // Generate model's top-view depth-map.
   M3dimCalibrateDepthMap(MilBox, MilDepthMap[eModel], M_NULL, M_NULL, M_DEFAULT, M_NEGATIVE, M_DEFAULT);
   M3dimProject(MilPtCldCtn[eModel], MilDepthMap[eModel], M_NULL, M_POINT_BASED, M_MAX_Z, M_DEFAULT,M_DEFAULT);

   auto MilStatResult = M3dmetAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Compute model's mean depth-map elevation.
   M3dmetStat(M_STAT_CONTEXT_MEAN, MilDepthMap[eModel], M_XY_PLANE, MilStatResult, M_SIGNED_DISTANCE_Z_TO_SURFACE, M_ALL, M_NULL, M_NULL,M_DEFAULT);
   M3dmetGetResult(MilStatResult, M_STAT_MEAN, &ModelMeanElevation); 
  
   //--------------------------------------------------------------------------
   // Generate an 8 bits image with optimal dynamic range from the ROI in
   // the model's depth-map. Define it as Model Finder's model and
   // preprocess its context. 

   // Compute model's 3D bounding box and corresponding ROI in the depth-map,
   // from which the Model Finder's model will be defined.
   DefineModelRoiBox(MilSystem,MilPtCldCtn[eModel], MilDepthMap[eModel], ModelRoiBox);

   // Allocate a buffer that will contain a copy of the model's depth-map's ROI image
   // from which the Model Finder's model will be defined.
   MilFinderImage[eModel] = MbufAlloc2d(MilSystem, ModelRoiBox.SizeX, ModelRoiBox.SizeY, M_UNSIGNED + 8, M_IMAGE + M_DISP + M_PROC, M_UNIQUE_ID);

   // Create an 8 bits gray-scale image from 16 bits depth-map's dynamic range,
   // since Model Finder only works with 8 bits images. Use a child buffer to specify the
   // ROI in the model's depth-map.
   MIL_UNIQUE_BUF_ID MilModelRoiChild = MbufChild2d(MilDepthMap[eModel], ModelRoiBox.OffsetX, ModelRoiBox.OffsetY, ModelRoiBox.SizeX, ModelRoiBox.SizeY, M_UNIQUE_ID);
   MapDynamicRangeTo8Bits(MilSystem, MilModelRoiChild, MilFinderImage[eModel]);

   // Allocate model finder objects.
   MilFinderContext = MmodAlloc(MilSystem, M_GEOMETRIC, M_DEFAULT, M_UNIQUE_ID);
   MilFinderResult  = MmodAllocResult(MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Add the model's depth-map's ROI as the ModelFinder model.
   MmodDefine(MilFinderContext, M_IMAGE, MilFinderImage[eModel], M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Set control to find all occurrences in the scene.
   MmodControl(MilFinderContext, M_ALL, M_NUMBER, M_ALL);

   // Set acceptance relatively low to account for small deformations 
   // caused by object's 3D pose.
   MmodControl(MilFinderContext, M_ALL, M_ACCEPTANCE, FINDER_ACCEPTANCE);

   // Allocate the fixturing offset that will contain the relation between
   // the model's reference point and the origin.
   MIL_UNIQUE_CAL_ID MilFinderFixturingOffset = McalAlloc(MilSystem, M_FIXTURING_OFFSET, M_DEFAULT, M_UNIQUE_ID);

   // Determine the fixturing offset from the model defined in the context.
   McalFixture(M_NULL, MilFinderFixturingOffset, M_LEARN_OFFSET,
               M_MODEL_MOD, MilFinderContext, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Preprocess ModelFinder context.
   MmodPreprocess(MilFinderContext, M_DEFAULT);

   // Draw model's edges.
   MdispSelect(MilDisplay, MilFinderImage[eModel]);
   MgraColor(M_DEFAULT, SELECTED_OCCURRENCE_COLOR);
   MmodDraw(M_DEFAULT, MilFinderContext, MilGraphicList, M_DRAW_EDGES + M_DRAW_BOX + M_DRAW_POSITION, 0, M_DEFAULT);

   MosPrintf(MIL_TEXT("A scanned object sample, acquired using a third-party 3D scanner, \n"));
   MosPrintf(MIL_TEXT("is restored. A top-view depth-map of the object is generated and \n"));
   MosPrintf(MIL_TEXT("used to define a 2-dimensional Model Finder model (displayed \n"));
   MosPrintf(MIL_TEXT("in green). \n\n"));
   MosPrintf(MIL_TEXT("Press any key to continue. \n\n"));
   MosGetch();
 
   // ----------------------------------------------------------------------------------
   // Find model occurrences' 3D poses in bin stack scenes.

   // Allocate the scene's 3D point cloud container.
   MilPtCldCtn[eScene] = MbufAllocContainer(MilSystem, M_PROC+M_DISP, M_DEFAULT, M_UNIQUE_ID);

   // Allocate the scene's depth-map.
   MilDepthMap[eScene] = MbufAlloc2d(MilSystem, DEPTHMAP_SIZE_X, DEPTHMAP_SIZE_Y, M_UNSIGNED + 16, M_IMAGE + M_DISP + M_PROC, M_UNIQUE_ID);

   // Allocate a buffer that will contain a copy of the scene's depth-map image in
   // which Model Finder will find occurrences of the model.
   MilFinderImage[eScene] = MbufAlloc2d(MilSystem, DEPTHMAP_SIZE_X, DEPTHMAP_SIZE_Y, M_UNSIGNED + 8, M_IMAGE + M_DISP + M_PROC, M_UNIQUE_ID);

   // Allocate 3D pairwise registration objects.
   MilRegistrationContext   = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID );
   MilRegistrationResult    = M3dregAllocResult(MilSystem, M_PAIRWISE_REGISTRATION_RESULT, M_DEFAULT, M_UNIQUE_ID);
   MilPreregistrationMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   // Set 3D registration parameters
   MIL_ID MilSubsampleContext = M_NULL;
   M3dregInquire(MilRegistrationContext, M_DEFAULT, M_SUBSAMPLE_CONTEXT_ID, &MilSubsampleContext);

   // Subsampling is done along X only since the point cloud is unorganized.
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_DECIMATE);
   M3dimControl(MilSubsampleContext, M_STEP_SIZE_X, DECIMATION_STEP);

   M3dregControl(MilRegistrationContext, M_DEFAULT, M_SUBSAMPLE, M_ENABLE);
   M3dregControl(MilRegistrationContext, M_ALL, M_OVERLAP, OVERLAP);
   M3dregControl(MilRegistrationContext, M_DEFAULT, M_MAX_ITERATIONS, MAX_ITERATIONS);
   M3dregControl(MilRegistrationContext, M_DEFAULT, M_ERROR_MINIMIZATION_METRIC, ERROR_MINIMIZATION_METRIC);

   M3dimCalibrateDepthMap(MilBox, MilDepthMap[eScene], M_NULL, M_NULL, M_DEFAULT, M_NEGATIVE, M_DEFAULT);

   MIL_UNIQUE_BUF_ID MilTransformedModel = MbufAllocContainer(MilSystem, M_PROC+M_DISP, M_DEFAULT, M_UNIQUE_ID);

   // For each scene, find the model occurrence on top of the stack using
   // 2d model finder and determine its 3D pose using 3D registration.
   for(MIL_INT iScene = 0; iScene < NUM_SCENE_SCANS; iScene++)
      {
      // Import 3D stack scene acquired using a 3D device (e.g. a Gocator 3110 from LMI Technologies) 
      // and exported to a PLY file.
      MbufImport(FILE_POINT_CLOUD[eScene + iScene], M_DEFAULT, M_LOAD, MilSystem, &MilPtCldCtn[eScene]);

      if(MilDisplay3d)
         { M3ddispControl(MilDisplay3d, M_UPDATE, M_ENABLE); }

      MIL_INT Scene_SizeX = MbufInquireContainer(MilPtCldCtn[eScene], M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
      MIL_INT Scene_SizeY = MbufInquireContainer(MilPtCldCtn[eScene], M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
      MIL_ID MilReflectance = MbufAllocComponent(MilPtCldCtn[eScene], 3, Scene_SizeX, Scene_SizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC + M_DISP, M_COMPONENT_REFLECTANCE, M_NULL);//Colored reflectance
      MbufClear(MilReflectance,M_COLOR_BRIGHT_GRAY);

      // Show the scene's point cloud using a 3D display.
      if(MilDisplay3d)
         { M3ddispSelect(MilDisplay3d, MilPtCldCtn[eScene], M_SELECT, M_DEFAULT); }

      // Generate the scene's top-view depth-map.
      M3dimProject(MilPtCldCtn[eScene], MilDepthMap[eScene], M_NULL, M_DEFAULT, M_MAX_Z, M_DEFAULT, M_DEFAULT);
      
      // Create an 8 bits gray-scale image from 16 bits depth-map's dynamic range,
      // since Model Finder only works with 8 bits images.
      MapDynamicRangeTo8Bits(MilSystem, MilDepthMap[eScene], MilFinderImage[eScene]);

      // Display the scene's depth-map.
      MgraClear(M_DEFAULT, MilGraphicList);
      MdispSelect(MilDisplay, MilFinderImage[eScene]);

      // Find occurrences of the model in the scene's depth-map.
      MmodFind(MilFinderContext, MilFinderImage[eScene], MilFinderResult);

      // Draw edges of all found occurrences.
      MgraColor(M_DEFAULT, FOUND_OCCURRENCES_COLOR);
      MmodDraw(M_DEFAULT, MilFinderResult, MilGraphicList, M_DRAW_EDGES, M_ALL, M_DEFAULT);
      
      // Determine which found occurrence is on top of the stack and define a fixturing transform that 
      // pre-registeres the model and this occurrence, to be used as a starting point for M3dregCalculate.
      if(!FindPreregistrationWithTopFoundOccurrence(MilSystem, MilPtCldCtn, ModelMeanElevation, MilDepthMap,
                                                    MilFinderFixturingOffset,
                                                    MilFinderResult, MilPreregistrationMatrix,
                                                    &TopOccurrenceIdx))
         {
         MosPrintf(MIL_TEXT("No occurrence found. Press any key to continue.\n\n"));
         MosGetch();
         continue;
         }

      // Highlight the selected found occurrence, determined to be on top of the stack.
      MgraColor(M_DEFAULT, SELECTED_OCCURRENCE_COLOR);
      MmodDraw(M_DEFAULT, MilFinderResult, MilGraphicList, M_DRAW_EDGES + M_DRAW_BOX, TopOccurrenceIdx, M_DEFAULT);

      // Display information in prompt about the stack.
      if(iScene == 0)
         {
         MosPrintf(MIL_TEXT("The stack of objects has been scanned. \n\n"));
         }
      else
         {
         MosPrintf(MIL_TEXT("The first object located was removed from the stack of objects \n"));
         MosPrintf(MIL_TEXT("and a new scan was done. \n\n"));
         }

      // Display Model Finder information in prompt.
      MosPrintf(MIL_TEXT("STEP 1 : Object occurrences that are on top are located in the 2-dimensional \n"));
      MosPrintf(MIL_TEXT("         depth-map (left). The top-most occurrence is detected (displayed\n")
                MIL_TEXT("         in green). \n\n"));


      MosPrintf(MIL_TEXT("STEP 2 : Using its 2D position, the occurence's 3D pose is estimated in the\n"));
      MosPrintf(MIL_TEXT("          3D point cloud using 3D registration with the 3D model sample.\n\n"));
      MosPrintf(MIL_TEXT("Estimating 3D pose.."));

      // Pre-register model to scene, need to invert to set as scene to model.
      M3dgeoMatrixSetTransform(MilPreregistrationMatrix, M_INVERSE, MilPreregistrationMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      // Perform 3D registration of the model and the selected found occurrence in the scene.
      M3dregSetLocation(MilRegistrationContext, eScene, eModel, MilPreregistrationMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      M3dregCalculate(MilRegistrationContext,
                      MilPtCldCtn, 2,
                      MilRegistrationResult, M_DEFAULT);

      // Verify the success of the registration.
      M3dregGetResult(MilRegistrationResult, eScene, M_REGISTRATION_COMPLETED, &RegistrationCompleted);

      MosPrintf(MIL_TEXT(".done\n"));

      SPose RegistrationPose;
      if(RegistrationCompleted)
         {
         // Use the 3D registration result to generate a depth-map of the model registered,
         // on the occurrence found.
         ExtractRegisteredDepthMaps(MilSystem, MilPtCldCtn, MilTransformedModel, MilDepthMap, MilRegistrationResult, RegistrationPose);

         // Show the result of the 3D registration.
         if(MilDisplay3d)
            { M3ddispSelect(MilDisplay3d, MilTransformedModel, M_ADD, M_DEFAULT); }
         MosPrintf(MIL_TEXT("\tDisplayed in green in the point cloud.\n"));
         MosPrintf(MIL_TEXT("\t(X, Y, Z)         : (%9.4f mm ,%9.4f mm ,%9.4f mm ) \n"), RegistrationPose.Tx, RegistrationPose.Ty, RegistrationPose.Tz);
         MosPrintf(MIL_TEXT("\t(Roll, Pitch, Yaw): (%9.4f deg,%9.4f deg,%9.4f deg) \n\n"), RegistrationPose.Rx, RegistrationPose.Ry, RegistrationPose.Rz);
         }
      else
         {
         MosPrintf(MIL_TEXT("Occurrence's pose was not successfully determined. \n\n"));
         }

      if(iScene < NUM_SCENE_SCANS -1)
         { MosPrintf(MIL_TEXT("Press any key to continue.\n\n")); }
      else
         { MosPrintf(MIL_TEXT("Press any key to end.\n\n")); }
      MosGetch();

      if(MilDisplay3d)
         {
         // Disable the 3D display update before freeing the container components.
         M3ddispControl(MilDisplay3d, M_UPDATE, M_DISABLE);
         M3ddispSelect(MilDisplay3d, MilPtCldCtn[eScene], M_REMOVE, M_DEFAULT);
         M3ddispSelect(MilDisplay3d, MilTransformedModel, M_REMOVE, M_DEFAULT);
         }
      MbufFreeComponent(MilPtCldCtn[eScene], M_COMPONENT_ALL, M_DEFAULT);
      }

   // Hide the display by unselecting any buffer associated.
   MdispSelect(MilDisplay, M_NULL);
  
   return 0;
   }

//--------------------------------------------------------------------------
// Compute 3D bounding box and corresponding 2d ROI in the depth-map.
//--------------------------------------------------------------------------
void DefineModelRoiBox(MIL_ID MilSystem,MIL_ID MilPtCldCtn, MIL_ID MilDepthMap, SBox& ModelRoiBox)
   {
   MIL_UNIQUE_3DIM_ID MilStatContext = M3dimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_3DIM_ID MilStatResult  = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Compute a robust bounding box of all points.
   M3dimControl(MilStatContext, M_BOUNDING_BOX, M_ENABLE);
   M3dimControl(MilStatContext,  M_BOUNDING_BOX_ALGORITHM, M_ROBUST);
   M3dimStat(MilStatContext, MilPtCldCtn,  MilStatResult,  M_DEFAULT);

   // Inquire the computed bounding box.
   MIL_DOUBLE BoxMinX, BoxMinY, BoxMinZ;
   MIL_DOUBLE BoxMaxX, BoxMaxY, BoxMaxZ;
   M3dimGetResult(MilStatResult,  M_MIN_X + M_TYPE_MIL_DOUBLE, &BoxMinX);
   M3dimGetResult(MilStatResult,  M_MIN_Y , &BoxMinY);
   M3dimGetResult(MilStatResult,  M_MIN_Z , &BoxMinZ);
   M3dimGetResult(MilStatResult,  M_MAX_X , &BoxMaxX);
   M3dimGetResult(MilStatResult,  M_MAX_Y , &BoxMaxY);
   M3dimGetResult(MilStatResult,  M_MAX_Z , &BoxMaxZ);

   // Add a margin to the bounding box.
   BoxMinX -= MODEL_ROI_MARGIN_X;
   BoxMinY -= MODEL_ROI_MARGIN_Y;
   BoxMinZ -= MODEL_ROI_MARGIN_Z;
   BoxMaxX += MODEL_ROI_MARGIN_X;
   BoxMaxY += MODEL_ROI_MARGIN_Y;
   BoxMaxZ += MODEL_ROI_MARGIN_Z;

   // Convert x-y bounding corners from world to pixel units in the depth-map.
   MIL_DOUBLE RoiMinX, RoiMinY;
   MIL_DOUBLE RoiMaxX, RoiMaxY;
   McalTransformCoordinate(MilDepthMap, M_WORLD_TO_PIXEL, BoxMinX, BoxMinY, &RoiMinX, &RoiMinY);
   McalTransformCoordinate(MilDepthMap, M_WORLD_TO_PIXEL, BoxMaxX, BoxMaxY, &RoiMaxX, &RoiMaxY);

   // Copy values to output structure.
   ModelRoiBox.MinX = BoxMinX;
   ModelRoiBox.MinY = BoxMinY;
   ModelRoiBox.MinZ = BoxMinZ;
   ModelRoiBox.MaxX = BoxMaxX;
   ModelRoiBox.MaxY = BoxMaxY;
   ModelRoiBox.MaxZ = BoxMaxZ;
   ModelRoiBox.OffsetX = static_cast<MIL_INT>(RoiMinX);
   ModelRoiBox.OffsetY = static_cast<MIL_INT>(RoiMinY);
   ModelRoiBox.SizeX = static_cast<MIL_INT>(RoiMaxX - RoiMinX);
   ModelRoiBox.SizeY = static_cast<MIL_INT>(RoiMaxY - RoiMinY);

   }

//--------------------------------------------------------------------------
// Determine which found occurrence is on top of the stack and define a
// fixturing transform that  pre registers the model and this occurrence.
//--------------------------------------------------------------------------
bool FindPreregistrationWithTopFoundOccurrence(MIL_ID            MilSystem,
                                               MIL_UNIQUE_BUF_ID MilPtCldCtn[],
                                               MIL_DOUBLE        ModelMeanElevation,
                                               MIL_UNIQUE_BUF_ID MilDepthMap[],
                                               MIL_ID            MilFinderFixturingOffset,
                                               MIL_ID            MilFinderResult,
                                               MIL_ID            MilPreregistrationMatrix,
                                               MIL_INT*          pTopOccIdx)
   {
   // Get the number of found occurrences in the scene.
   MIL_INT NumOccurrences;
   MmodGetResult(MilFinderResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NumOccurrences);

   // Return if no occurrence.
   if(NumOccurrences == 0) return false;

   // Generate the top-view depth-map of the model in its initial pose 
   M3dimProject(MilPtCldCtn[eModel], MilDepthMap[eModel], M_NULL, M_POINT_BASED, M_MAX_Z, M_DEFAULT, M_DEFAULT);

   // Loop on each found occurrence in the scene, compute its mean elevation in the depth-map
   // (using the model's mask) and select the higher (the one on top).
   MIL_DOUBLE CurTopOccMeanElevation = 0.0;

   MIL_UNIQUE_BUF_ID   MilTransformedScene = MbufAllocContainer(MilSystem, M_PROC, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_3DGEO_ID MilFixturingMatrix  = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   // Create a masked buffer of points of the scene only where points are valid points in the model's depth-map.
   MIL_UNIQUE_BUF_ID MaskedScene = MbufAlloc2d(MilSystem, DEPTHMAP_SIZE_X, DEPTHMAP_SIZE_Y, M_UNSIGNED + 16, M_PROC + M_IMAGE + M_DISP, M_UNIQUE_ID);

   auto MilStatResult = M3dmetAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   for(MIL_INT iOcc = 0; iOcc < NumOccurrences; iOcc++)
      {
      // Fixture the scene's point cloud on the current found occurrence, taking 
      // into account the fixturing offset between the model's reference point
      // and the world's origin.
      McalFixture(MilFixturingMatrix, MilFinderFixturingOffset, M_MOVE_RELATIVE,
                  M_RESULT_MOD, MilFinderResult, iOcc, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      //McalFixture returns the matrix to transform the model.
      //Get the inverse of this matrix to transform the scene.
      M3dgeoMatrixSetTransform(MilFixturingMatrix, M_INVERSE, MilFixturingMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
  
      M3dimMatrixTransform(MilPtCldCtn[eScene], MilTransformedScene, MilFixturingMatrix, M_DEFAULT);
    
      // Extract the top-view depth-map of the scene transformed on the occurrence.
      M3dimProject(MilTransformedScene, MilDepthMap[eScene], M_NULL, M_DEFAULT, M_MAX_Z,M_DEFAULT, M_DEFAULT);
    
      // Compute occurrence's mean elevation in the scene only from the pixels
      // enabled by the model valid points.
      MbufClear(MaskedScene, DEPTHMAP_MISSING_DATA);
      MbufCopyCond(MilDepthMap[eScene], MaskedScene, MilDepthMap[eModel], M_NOT_EQUAL, DEPTHMAP_MISSING_DATA);
      McalAssociate(MilDepthMap[eScene], MaskedScene, M_DEFAULT);

      MIL_DOUBLE OccMeanElevation;
      // Compute model's mean depth-map elevation relative to the xy plane.
      M3dmetStat(M_STAT_CONTEXT_MEAN, MaskedScene, M_XY_PLANE, MilStatResult, M_SIGNED_DISTANCE_Z_TO_SURFACE, M_ALL, M_NULL, M_NULL, M_DEFAULT);
      M3dmetGetResult(MilStatResult, M_STAT_MEAN, &OccMeanElevation);
    
      // Since the z scale is negative, we actually search for the lowest elevation.
      if((iOcc == 0) || (iOcc > 0 && (OccMeanElevation < CurTopOccMeanElevation)))
         {
         // Save the index and mean elevation of the currently selected occurrence.
         *pTopOccIdx = iOcc;
         CurTopOccMeanElevation = OccMeanElevation;

         //To get pre-registration matrix of model to scene need to inverse this matrix.
         M3dgeoMatrixSetTransform( MilFixturingMatrix, M_INVERSE, MilFixturingMatrix,  M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

         // Apply an additional z offset to the scene's point cloud to register the
         // mean elevation of the occurrence with the mean elevation of the model.
         MIL_DOUBLE OffsetZ = CurTopOccMeanElevation - ModelMeanElevation;
         M3dgeoMatrixSetTransform(MilFixturingMatrix,  M_TRANSLATION, 0.0, 0.0, OffsetZ, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
       
         // The resulting pose of the model's point cloud, expressed as an homogeneous matrix, 
         // defines a transformation that coarsely 3D registers the model and the occurrence and  
         // it can be used as a preregistration for M3dregCalculate.
         M3dgeoCopy(MilFixturingMatrix, MilPreregistrationMatrix, M_TRANSFORMATION_MATRIX, M_DEFAULT);
         }
      }

   // Reset scene's pose and regenerate the scene's depth-map.
   M3dimProject(MilPtCldCtn[eScene], MilDepthMap[eScene], M_NULL, M_DEFAULT,M_MAX_Z, M_DEFAULT, M_DEFAULT);

   return true;
   }

//--------------------------------------------------------------------------
// Use the 3D registration result to generate a depth-map of the model registered
// on the occurrence.
//--------------------------------------------------------------------------
void ExtractRegisteredDepthMaps(MIL_ID MilSystem, MIL_UNIQUE_BUF_ID MilPtCldCtn[], MIL_UNIQUE_BUF_ID& MilTransformedModel, MIL_UNIQUE_BUF_ID MilDepthMap[], MIL_ID MilResult, SPose& Pose)
   {
   MIL_UNIQUE_3DGEO_ID MilMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   // Use the 3D registration result to fixture the model with the found occurrence in the scene.
   M3dregCopyResult(MilResult, eModel, eScene, MilMatrix, M_REGISTRATION_MATRIX, M_DEFAULT);

   // Fixture the model to the found occurrence in the scene using its 3D transformation matrix.
   M3dimMatrixTransform(MilPtCldCtn[eModel], MilTransformedModel, MilMatrix, M_DEFAULT);
   M3dimProject(MilTransformedModel, MilDepthMap[eModel], M_NULL, M_DEFAULT, M_MAX_Z, M_DEFAULT, M_DEFAULT);

   // Determine the found occurrence 3D pose after 3D registration.
   GetPose(MilMatrix, Pose);
   }

//--------------------------------------------------------------------------
// Create an 8 bits gray-scale image from 16 bits depth-map's dynamic range.
//--------------------------------------------------------------------------
void MapDynamicRangeTo8Bits(MIL_ID MilSystem, MIL_ID MilSrcImage, MIL_ID MilTgtImage)
   {
   // The remapping operation must exclude missing data pixels.
   // Create a raster region containing valid pixels only.
   MIL_UNIQUE_BUF_ID MilRegion = MbufAlloc2d(MilSystem,
                                             MbufInquire(MilSrcImage, M_SIZE_X, M_NULL),
                                             MbufInquire(MilSrcImage, M_SIZE_Y, M_NULL),
                                             8 + M_UNSIGNED, M_IMAGE + M_PROC, M_UNIQUE_ID);
   MimBinarize(MilSrcImage, MilRegion, M_FIXED + M_NOT_EQUAL, DEPTHMAP_MISSING_DATA, M_NULL);

   // Associate the raster region to the depth map.
   MbufSetRegion(MilSrcImage, MilRegion, M_DEFAULT, M_RASTERIZE, M_DEFAULT);

   // Missing data pixels are mapped to 0 in MilTgtImage.
   MbufClear(MilTgtImage, 0.0);

   // Remap depth-map's 16 bits dynamic range of valid data to 8 bits.
   MimRemap(M_DEFAULT, MilSrcImage, MilTgtImage, M_FIT_SRC_DATA);

   // Remove the raster region.
   MbufSetRegion(MilSrcImage, M_NULL, M_DEFAULT, M_DELETE, M_DEFAULT);
   }

//--------------------------------------------------------------------------
// Get 3D pose of specified point cloud container.
//--------------------------------------------------------------------------
void GetPose(MIL_ID MilMatrixId, SPose& Pose)
   {
   // Get 6 pose elements.
   M3dgeoMatrixGetTransform(MilMatrixId, M_TRANSLATION,  &Pose.Tx, &Pose.Ty, &Pose.Tz, M_NULL,M_DEFAULT);
   M3dgeoMatrixGetTransform(MilMatrixId, M_ROTATION_XYZ, &Pose.Rx, &Pose.Ry, &Pose.Rz, M_NULL,M_DEFAULT);

   // Keep absolute rotation values below 180 degrees for clarity.
   if(fabs(Pose.Rx) > 180.0) Pose.Rx = fmod(Pose.Rx - 360.0, 360.0);
   if(fabs(Pose.Ry) > 180.0) Pose.Ry = fmod(Pose.Ry - 360.0, 360.0);
   if(fabs(Pose.Rz) > 180.0) Pose.Rz = fmod(Pose.Rz - 360.0, 360.0);
   }

//--------------------------------------------------------------------------
// Allocates a 3D display and returns its MIL identifier.
//--------------------------------------------------------------------------
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n\n"));
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
