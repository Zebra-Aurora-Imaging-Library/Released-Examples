﻿//***************************************************************************************
// 
// File name: 3dCADRegistration.cpp  
//
// Synopsis: This program contains an example of sampling a 3D CAD model 
//           and a scene point clouds to the same resolution using the
//           3D image processing module.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************

#include <mil.h>
#include <algorithm>
//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("3dCADRegistration\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to sample a 3D CAD model\n"));
   MosPrintf(MIL_TEXT("and an acquired point cloud to the same resolution.\n"));
   MosPrintf(MIL_TEXT("In this case, the optimal registration between the model and \n"));
   MosPrintf(MIL_TEXT("the scene point clouds is chosen based on a 3D hole feature\n"));
   MosPrintf(MIL_TEXT("defined by a box.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: 3D Registration, 3D Geometry, 3D Metrology,\n")
             MIL_TEXT("3D Image Processing, 3D Display, and 3D Graphics. \n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_STRING POINT_CLOUD_FILE = M_IMAGE_PATH MIL_TEXT("3dCADRegistration/Scene.mbufc");
static const MIL_STRING MODEL_FILE       = M_IMAGE_PATH MIL_TEXT("3dCADRegistration/Model_CAD.PLY");
// Tolerance for the plane fits.
static const MIL_DOUBLE PLANE_TOLERANCE = 10.0;
// Registration context controls definitions.
static const MIL_INT    MAX_ITERATIONS = 50;
static const MIL_DOUBLE GRID_SIZE = 1;
// Enumerators definitions.
enum { eModel = 0, eObject = 1};

//****************************************************************************
// Function Declaration.
//****************************************************************************
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);
bool   CheckForRequiredMILFile(MIL_STRING FileName);
void   RemoveFloorPoints(MIL_ID MilSystem, MIL_UNIQUE_BUF_ID& MilSceneCloud);
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

   MIL_UNIQUE_SYS_ID    MilSystem;            // System identifier.
   MIL_ID               MilSceneDisplay;      // 3D Mil Display.
   MIL_UNIQUE_3DDISP_ID MilModelDisplay;
   MIL_UNIQUE_3DDISP_ID MilResultDisplay;
   MIL_UNIQUE_BUF_ID    MilSceneCloud;        // Scene point cloud.
   MIL_UNIQUE_BUF_ID    MilModelCloud;        // Model point cloud from a CAD drawing.
   MIL_UNIQUE_BUF_ID    MilMatchedCloud;      // Registered point cloud.
   MIL_UNIQUE_BUF_ID    MilSampledModel;
   MIL_UNIQUE_BUF_ID    MilSampledScene;
   MIL_UNIQUE_BUF_ID    MilCroppedScene;

   MIL_ID MilSceneGraphicList  = M_NULL;
   MIL_ID MilModelGraphicList  = M_NULL;
   MIL_ID MilResultGraphicList = M_NULL;

   // Allocate MIL objects. 
   MilSystem        = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MilSceneDisplay  = Alloc3dDisplayId(MilSystem);
   MilMatchedCloud  = MbufAllocContainer(MilSystem, M_PROC | M_DISP, M_DEFAULT, M_UNIQUE_ID);
   MilSampledModel  = MbufAllocContainer(MilSystem, M_PROC | M_DISP, M_DEFAULT, M_UNIQUE_ID);
   MilSampledScene  = MbufAllocContainer(MilSystem, M_PROC | M_DISP, M_DEFAULT, M_UNIQUE_ID);
   MilCroppedScene  = MbufAllocContainer(MilSystem, M_PROC | M_DISP, M_DEFAULT, M_UNIQUE_ID);
   MilModelDisplay  = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MilResultDisplay = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   // Get the graphic list's identifier.
   M3ddispInquire(MilSceneDisplay , M_3D_GRAPHIC_LIST_ID, &MilSceneGraphicList);
   M3ddispInquire(MilModelDisplay , M_3D_GRAPHIC_LIST_ID, &MilModelGraphicList);
   M3ddispInquire(MilResultDisplay, M_3D_GRAPHIC_LIST_ID, &MilResultGraphicList);

   // Adjust the view of the 3D displays.
   M3ddispSetView(MilSceneDisplay , M_AUTO, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilModelDisplay , M_AUTO, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilResultDisplay, M_AUTO, M_BOTTOM_VIEW  , M_DEFAULT, M_DEFAULT, M_DEFAULT);
      
   // Load the 3D data. 
   MilModelCloud  = MbufRestore(MODEL_FILE      , MilSystem, M_UNIQUE_ID);
   MilSceneCloud  = MbufRestore(POINT_CLOUD_FILE, MilSystem, M_UNIQUE_ID);

   // Unit information is lost in a ply file.
   MIL_ID ModelRangeId = MbufInquireContainer(MilModelCloud, M_COMPONENT_RANGE, M_COMPONENT_ID, M_NULL);
   MbufControl(ModelRangeId, M_3D_DISTANCE_UNIT, M_MILLIMETER);

   // Display the scene point cloud.
   M3ddispControl(MilSceneDisplay, M_SIZE_X, 300);
   M3ddispControl(MilSceneDisplay, M_SIZE_Y, 300);
   MIL_INT64 MilContainerGraphics = M3ddispSelect(MilSceneDisplay, MilSceneCloud, M_SELECT, M_DEFAULT);
   M3dgraControl(MilSceneGraphicList, MilContainerGraphics, M_COLOR_USE_LUT, M_TRUE);
   M3dgraControl(MilSceneGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);

   // Display the model point cloud.
   M3ddispControl(MilModelDisplay, M_WINDOW_INITIAL_POSITION_X, 300);
   M3ddispControl(MilModelDisplay, M_SIZE_X, 300);
   M3ddispControl(MilModelDisplay, M_SIZE_Y, 300);
   MIL_INT64 ModelLabel = M3ddispSelect(MilModelDisplay, MilModelCloud, M_SELECT, M_DEFAULT);
   M3dgraControl(MilModelGraphicList, ModelLabel, M_COLOR_USE_LUT, M_TRUE);
   M3dgraControl(MilModelGraphicList, ModelLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);

   MosPrintf(MIL_TEXT("The model and the scene 3D point clouds are restored and displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Remove the floor points from the scene point cloud.
   RemoveFloorPoints(MilSystem, MilSceneCloud);
   M3ddispSetView(MilSceneDisplay, M_VIEW_BOX, M_WHOLE_SCENE, 1.0, M_DEFAULT, M_DEFAULT);
   MosPrintf(MIL_TEXT("Background points are removed from the scene.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Sample the model cloud.
   MosPrintf(MIL_TEXT("The model point cloud is a CAD model with a mesh component and sparse points.\n"));
   MosPrintf(MIL_TEXT("The model point cloud is sampled in order to increase its point density with\n"));
   MosPrintf(MIL_TEXT("resolution of %f mm. The resolution defines the distance between\n"), GRID_SIZE);
   MosPrintf(MIL_TEXT(" generated points on the mesh faces .\n"));
   MosPrintf(MIL_TEXT("The sampled model point cloud is displayed in red.\n"));
   MIL_UNIQUE_3DIM_ID MilMeshSampleContext = M3dimAlloc(MilSystem, M_SURFACE_SAMPLE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MilMeshSampleContext, M_RESOLUTION, GRID_SIZE);
   M3dimSample(MilMeshSampleContext, MilModelCloud, MilSampledModel, M_DEFAULT);

   M3ddispControl(MilResultDisplay, M_WINDOW_INITIAL_POSITION_X, 600);
   M3ddispControl(MilResultDisplay, M_SIZE_X, 300);
   M3ddispControl(MilResultDisplay, M_SIZE_Y, 300);
   MIL_INT64 SampledModelLabel = M3ddispSelect(MilResultDisplay, MilSampledModel, M_SELECT, M_DEFAULT);
   M3dgraControl(MilResultGraphicList, SampledModelLabel, M_COLOR, M_COLOR_RED);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   MosPrintf(MIL_TEXT("The scene point cloud is sampled with the same resolution.\n"));
   MosPrintf(MIL_TEXT("The sampled scene point cloud is displayed in green.\n"));
   // Subsample the scene cloud to have the same resoultion as that of the model.
   MIL_UNIQUE_3DIM_ID MilSubsampleContext = M3dimAlloc(MilSystem, M_SUBSAMPLE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   // Set the controls of the subsampling.
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GRID);
   M3dimControl(MilSubsampleContext, M_GRID_SIZE_X, GRID_SIZE);
   M3dimControl(MilSubsampleContext, M_GRID_SIZE_Y, GRID_SIZE);
   M3dimControl(MilSubsampleContext, M_GRID_SIZE_Z, GRID_SIZE);
   M3dimSample(MilSubsampleContext, MilSceneCloud, MilSampledScene, M_DEFAULT);
   MIL_INT64 SampledGrabbedLabel = M3ddispSelect(MilResultDisplay, MilSampledScene, M_ADD, M_DEFAULT);
   M3dgraControl(MilResultGraphicList, SampledGrabbedLabel, M_FILL_COLOR, M_COLOR_GREEN);
   M3ddispSetView(MilResultDisplay, M_VIEW_BOX, M_WHOLE_SCENE, 1.0, M_DEFAULT, M_DEFAULT);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // 3D registration.
   MIL_UNIQUE_3DREG_ID MilContext = M3dregAlloc      (MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_3DREG_ID MilResult  = M3dregAllocResult(MilSystem, M_PAIRWISE_REGISTRATION_RESULT , M_DEFAULT, M_UNIQUE_ID);

   // Pairwise registration context controls.
   M3dregControl(MilContext, M_CONTEXT, M_PREREGISTRATION_MODE     , M_CENTROID);
   M3dregControl(MilContext, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_POINT);
   M3dregControl(MilContext, M_CONTEXT, M_MAX_ITERATIONS           , MAX_ITERATIONS);
   M3dregControl(MilContext, M_CONTEXT, M_SUBSAMPLE                , M_DISABLE);

   MIL_INT NbPoints;
   MIL_INT MinNbPoints;
   MIL_INT OptimumIter;
   MIL_UNIQUE_3DGEO_ID MilOptimumRegisteration = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   MosPrintf(MIL_TEXT("This object is symmetric, except for a small rectangular hole at the bottom.\n"));

   // The box is known from the given CAD model.
   MIL_UNIQUE_3DGEO_ID MilBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoBox(MilBox, M_BOTH_CORNERS, -7.5, 16, 10, 7.5, 22, 35, M_DEFAULT);
   MIL_INT64 Label = M3dgeoDraw3d(M_DEFAULT, MilBox, MilModelGraphicList, M_DEFAULT, M_DEFAULT);
   M3dgraControl(MilModelGraphicList, Label, M_OPACITY, 30);
   M3dgraControl(MilModelGraphicList, Label, M_COLOR  , M_COLOR_RED);
   MosPrintf(MIL_TEXT("This rectangular hole is shown by the red region.\n"));
   MosPrintf(MIL_TEXT("There is an ambiguity of 90 degrees in the registration results.\n"));
   MosPrintf(MIL_TEXT("Registration will be applied for the four possible rotations of 90 degrees.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to go from one registration to the next.\n\n"));
   MosGetch();

   MIL_UNIQUE_3DIM_ID MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Registration.
   MIL_ID MilPointClouds[2] = { MilSampledModel.get(), MilSampledScene.get() };

   MIL_UNIQUE_3DGEO_ID MilPreRegistrationMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   MosPrintf(MIL_TEXT("Index  Registration time(ms)   RmsError  #Points in the hole\n"));
   MosPrintf(MIL_TEXT("----------------------------------------------------------------\n"));

   for(MIL_INT iter = 0; iter < 4; ++iter)
      {
      M3dregSetLocation(MilContext, eObject, eModel, MilPreRegistrationMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      MIL_INT RegistrationStatus = M_NULL;
      MIL_DOUBLE ComputationTime = 0.0;

      MappTimer(M_TIMER_RESET, M_NULL);
      M3dregCalculate(MilContext,
                      MilPointClouds,
                      2,
                      MilResult,
                      M_DEFAULT);
      MappTimer(M_TIMER_READ, &ComputationTime);

      M3dregGetResult(MilResult, 1, M_STATUS_REGISTRATION_ELEMENT, &RegistrationStatus);

      MIL_DOUBLE RegisterRmsError = 0;
      // Interpret the result status.
      switch(RegistrationStatus)
         {
         case M_NOT_INITIALIZED:
            MosPrintf(MIL_TEXT("Registration failed: the registration result is not initialized.\n\n"));
            break;
         case M_NOT_ENOUGH_POINT_PAIRS:
            MosPrintf(MIL_TEXT("Registration failed: point clouds are not overlapping.\n\n"));
            break;
         case M_MAX_ITERATIONS_REACHED:
            MosPrintf(MIL_TEXT("Registration reached the maximum number of iterations allowed (%d)\n")
                      MIL_TEXT("in %.2f ms. Resulting registration may or may not be valid.\n\n"),
                      MAX_ITERATIONS, ComputationTime * 1000);
            break;
         case M_RMS_ERROR_THRESHOLD_REACHED:
         case M_RMS_ERROR_RELATIVE_THRESHOLD_REACHED:
            M3dregGetResult(MilResult, 1, M_RMS_ERROR + M_TYPE_MIL_DOUBLE, &RegisterRmsError);
           
            break;
         default:
            MosPrintf(MIL_TEXT("Unknown registration status.\n\n"));
         }

      // Use the registration result to register object's point cloud with the model's point cloud.
      MIL_UNIQUE_3DGEO_ID MilRegistrationMatrix = M3dgeoAlloc(M_DEFAULT_HOST, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
      M3dregCopyResult(MilResult, eObject, eModel, MilRegistrationMatrix, M_REGISTRATION_MATRIX , M_DEFAULT);
      M3dimMatrixTransform(MilSceneCloud, MilMatchedCloud, MilRegistrationMatrix, M_DEFAULT);

      if(iter == 0)
         {
         ModelLabel = M3ddispSelect(MilResultDisplay, MilModelCloud, M_DEFAULT, M_DEFAULT);
         M3dgraControl(MilResultGraphicList, ModelLabel, M_COLOR_USE_LUT  , M_TRUE);
         M3dgraControl(MilResultGraphicList, ModelLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
         M3dgraControl(MilResultGraphicList, ModelLabel, M_OPACITY        , 50);

         MilContainerGraphics = M3ddispSelect(MilResultDisplay, MilMatchedCloud, M_ADD, M_DEFAULT);
         M3dgraControl(MilResultGraphicList, MilContainerGraphics, M_COLOR, M_COLOR_WHITE);

         M3ddispSetView(MilResultDisplay, M_VIEW_BOX, M_WHOLE_SCENE, 1.0, M_DEFAULT, M_DEFAULT);
         }
      M3dimCrop(MilMatchedCloud, MilCroppedScene, MilBox, M_NULL, M_SAME, M_DEFAULT);
      M3dimStat(M_STAT_CONTEXT_NUMBER_OF_POINTS, MilCroppedScene, MilStatResult, M_DEFAULT);
      M3dimGetResult(MilStatResult, M_NUMBER_OF_POINTS_VALID, &NbPoints);
      if(iter == 0)
         {
         MinNbPoints = NbPoints;
         OptimumIter = iter;
         M3dgeoCopy(MilRegistrationMatrix, MilOptimumRegisteration, M_TRANSFORMATION_MATRIX, M_DEFAULT);
         }
      else
         {
         MinNbPoints = std::min(NbPoints, MinNbPoints);
         OptimumIter = iter;
         M3dgeoCopy(MilRegistrationMatrix, MilOptimumRegisteration, M_TRANSFORMATION_MATRIX, M_DEFAULT);
         }

      MosPrintf(MIL_TEXT("  %i        %.2f              % f            %i  \n"),
                iter, ComputationTime * 1000, RegisterRmsError, NbPoints);
      MosPrintf(MIL_TEXT("Press <ENTER> to continue."));
      MosGetch();
      MosPrintf(MIL_TEXT("\r"));
      
      M3dgeoMatrixSetTransform(MilPreRegistrationMatrix, M_ROTATION_Z, 90, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
      }

   MosPrintf(MIL_TEXT("\n\nThe best registration is the one with the minimum number of points in the\n"));
   MosPrintf(MIL_TEXT("hole. The optimal registration is that of iteration %i, as displayed.\n\n"), OptimumIter);

   M3dimMatrixTransform(MilSceneCloud, MilMatchedCloud, MilOptimumRegisteration, M_DEFAULT);
   M3ddispSetView(MilResultDisplay, M_AUTO, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispControl(MilResultDisplay, M_AUTO_ROTATE, M_ENABLE);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   // Free allocated objects.
   if(MilSceneDisplay)
      { M3ddispFree(MilSceneDisplay); }
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
                MIL_TEXT("Press any key to end.\n"));
      MosGetch();
      exit(0);
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

//****************************************************************************
// Remove background floor points from a scanned point cloud.
//****************************************************************************
void RemoveFloorPoints(MIL_ID MilSystem, MIL_UNIQUE_BUF_ID& MilSceneCloud)
   {
   // Fit and display a plane on the background.
   MIL_UNIQUE_3DMET_ID MilFitResult  = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_ID ConfidenceBuffer           = MbufInquireContainer(MilSceneCloud, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
 
   M3dmetFit(M_DEFAULT, MilSceneCloud, M_PLANE, MilFitResult, PLANE_TOLERANCE, M_DEFAULT);
   M3dmetCopyResult(MilFitResult, ConfidenceBuffer, M_OUTLIER_MASK, M_DEFAULT);

   M3dimRemovePoints(MilSceneCloud, MilSceneCloud, M_INVALID_POINTS_ONLY, M_DEFAULT);
   }
