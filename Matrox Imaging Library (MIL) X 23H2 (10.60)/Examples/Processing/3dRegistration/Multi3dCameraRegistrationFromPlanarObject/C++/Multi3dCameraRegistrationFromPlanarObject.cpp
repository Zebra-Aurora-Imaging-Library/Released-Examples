﻿//***************************************************************************************/
//
// File name: Multi3dCameraRegistrationFromPlanarObject.cpp
//
// Synopsis:  This example demonstrate various ways to registers multiple point clouds from 
//            multiple 3d cameras using a planar object.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/
#include <mil.h>
#if M_MIL_USE_LINUX
#include <math.h>
#endif


// Source file specification.
static const MIL_INT    NB_POINT_CLOUDS = 4;
static const MIL_STRING PT_CLD_FILE = M_IMAGE_PATH MIL_TEXT("Multi3dCameraRegistrationFromPlanarObject/DatamatrixCam");

// Tolerance for the plane fit. This distance should be adjusted according to the camera's distance units.
static const MIL_DOUBLE PLANE_TOLERANCE = 0.025;

// Cropping size in the middle of the FOV for the general fixture mode.
static const MIL_DOUBLE GENERAL_CROPPING_SIZE_RATIO = 0.5;

// General constants.
static const MIL_DOUBLE DIV_PI_180                       = 0.017453292519943295769236907684886;
static const MIL_DOUBLE DIV_180_PI                       = 57.295779513082320866997945294156;
static const MIL_DOUBLE DISPLAY_ROBUST_BOX_OUTLIER_RATIO = 0.05;
static const MIL_DOUBLE DISPLAY_INITIAL_VIEW_BOX_RATIO   = 0.65;
static const MIL_DOUBLE DISPLAY_MERGED_VIEW_BOX_RATIO    = 0.8;
static const MIL_DOUBLE PLANE_OPACITY                    = 20;
static const MIL_DOUBLE POINT_CLOUD_OPACITY              = 40;
static const MIL_INT    AXIS_THICKNESS                   = 5;

// Constant to control the steps pauses of the examples.
static const bool       ALWAYS_DISPLAY_STEPS  = false;
static const MIL_INT    AUTO_STEPS_SLEEP_TIME = 0;    //in ms

// Structure that holds the result of a point cloud with reflectance projection.
struct SDepthIntensity
   {
	SDepthIntensity() {};
	SDepthIntensity(SDepthIntensity&& o) : MilDepthMap(std::move(o.MilDepthMap)), MilIntensityMap(std::move(o.MilIntensityMap)) {};
   MIL_UNIQUE_BUF_ID MilDepthMap;
   MIL_UNIQUE_BUF_ID MilIntensityMap;
   };

// Function declarations.
void CheckForRequiredMILFile(const MIL_STRING& FileName);
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);
bool ConvertReflectanceToGray(MIL_ID MilPointCloud);
SDepthIntensity GenerateDepthMap(MIL_ID MilPointCloudContainer);
MIL_UNIQUE_3DGEO_ID FitPlane(MIL_ID MilPointCloud);
MIL_UNIQUE_3DGEO_ID FixtureToPlane(MIL_ID MilPointCloud, MIL_ID MilPlaneGeo);
void SetTransformationMatrixFrom2dFixture(MIL_ID MilMatrix, MIL_DOUBLE PosX, MIL_DOUBLE PosY, MIL_DOUBLE Angle);
void DisplayMergedResult(MIL_ID MilRefectanceDisp, MIL_ID MilRangeDisp, MIL_ID MilMergePointCloud);
void WaitForKey(MIL_INT SleepTime = -1);

// Example functions declarations.
void LocatePlaneRegion(const MIL_UNIQUE_CODE_ID& MilFixtureContext, MIL_ID MilPointCloud);
void LocatePlaneRegion(const MIL_UNIQUE_MOD_ID& MilFixtureContext, MIL_ID MilPointCloud);
MIL_UNIQUE_3DGEO_ID Find2dFixture(const MIL_UNIQUE_CODE_ID& MilFixtureContext, MIL_ID MilPlaneReflectance, MIL_ID MilGraList);
MIL_UNIQUE_3DGEO_ID Find2dFixture(const MIL_UNIQUE_MOD_ID& MilFixtureContext, MIL_ID MilPlaneReflectance, MIL_ID MilGraList);
void DatamatrixPlanarObjectExample(MIL_ID MilSystem);
void GeneralPlanarObjectExample(MIL_ID MilSystem);
template <class T> void PlanarObjectExample(MIL_ID MilSystem, const T& MilFixtureContext,
                                            MIL_CONST_TEXT_PTR PlaneRegionMessage, MIL_CONST_TEXT_PTR FixtureRefMessage);

// Specific message for the different algorithms.
static MIL_CONST_TEXT_PTR PLANE_REGION_GENERAL =
   {
   MIL_TEXT("The point cloud was cropped to only include a region\n")
   MIL_TEXT("in the middle of acquired area.\n")
   };

static MIL_CONST_TEXT_PTR FIXTURE_REFERENCE_GENERAL =
   {
   MIL_TEXT("The reference 2d fixturing location is based on the position\n")
   MIL_TEXT("and orientation of a model finder model created from the\n")
   MIL_TEXT("intensity map of the first camera.\n")
   MIL_TEXT("A flat region mask was used to mask out edges that may be due\n")
   MIL_TEXT("to invalid data.\n")
   };

static MIL_CONST_TEXT_PTR PLANE_REGION_DATAMATRIX =
   {
   MIL_TEXT("The datamatrix was found in the reflectance component.\n")
   MIL_TEXT("The point cloud was cropped to only include that region.\n")
   };

static MIL_CONST_TEXT_PTR FIXTURE_REFERENCE_DATAMATRIX =
   {
   MIL_TEXT("The reference 2d fixturing location is based on the position\n")
   MIL_TEXT("and orientation of the datamatrix code of the first camera.\n")
   };

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("Multi3dCameraRegistrationFromPlanarObject\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows various ways to register the point clouds from\n")
             MIL_TEXT("multiple 3d cameras using some planar objects with features.\n")
             MIL_TEXT("A plane is first fitted in a certain region of each point cloud.\n")
             MIL_TEXT("The point cloud is then projected on this plane to create an intensity map.\n")
             MIL_TEXT("Finally, features in the intensity map are used to find the 2d transformation\n")
             MIL_TEXT("between the different views. By combining the 2d transformation and the\n")
             MIL_TEXT("3d transformation used to project the point, we get a 3d transformation that\n")
             MIL_TEXT("can be used to register and merge all the 3d cameras' point clouds together.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Display, 3D Geometry, 3D Graphics,\n")
             MIL_TEXT("3D Image Processing, 3D Metrology, Buffer, Code Reader, Display,\n")
             MIL_TEXT("Graphics, Image Processing, Geometric Model Finder\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//****************************************************************************
// Main.
//****************************************************************************
int MosMain()
   {
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Print the header.
   PrintHeader();

   // Run the datamatrix object example.
   DatamatrixPlanarObjectExample(MilSystem);

   // Run the general object example.
   GeneralPlanarObjectExample(MilSystem);
   }

//****************************************************************************
// Datamatrix planar object example.
//****************************************************************************
void DatamatrixPlanarObjectExample(MIL_ID MilSystem)
   {
   // Print the sub example header.
   MosPrintf(MIL_TEXT("[DATA MATRIX PLANAR OBJECT EXAMPLE]\n")
             MIL_TEXT("A planar data matrix will be used to register the 3d cameras.\n")
             MIL_TEXT("The data matrix will define the plane fit region.\n")
             MIL_TEXT("The data matrix will provide the 2d fixture in the projected intensity.\n\n"));

   // Allocate the code reader context.
   auto MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_IMPROVED_RECOGNITION, M_UNIQUE_ID);
   McodeModel(MilCodeContext, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, M_NULL);

   // Run the example.
   PlanarObjectExample(MilSystem, MilCodeContext, PLANE_REGION_DATAMATRIX, FIXTURE_REFERENCE_DATAMATRIX);
   }


//****************************************************************************
// General planar object example.
//****************************************************************************
void GeneralPlanarObjectExample(MIL_ID MilSystem)
   {
   // Print the sub example header.
   MosPrintf(MIL_TEXT("[GENERAL PLANAR OBJECT EXAMPLE]\n")
             MIL_TEXT("A planar model finder model will be used to register the 3d cameras.\n")
             MIL_TEXT("The plane fit region will be defined as the middle region of acquisition.\n")
             MIL_TEXT("The model finder model will provide the 2d fixture in the projected intensity.\n\n"));

   // Allocate model finder context.
   auto MilModContext = MmodAlloc(MilSystem, M_GEOMETRIC, M_DEFAULT, M_UNIQUE_ID);

   // Run the example.
   PlanarObjectExample(MilSystem, MilModContext, PLANE_REGION_GENERAL, FIXTURE_REFERENCE_GENERAL);
   }

//****************************************************************************
// The example function that uses the right method to register the views.
//****************************************************************************
template <class T>
void PlanarObjectExample(MIL_ID MilSystem,
                         const T& MilFixtureContext,
                         MIL_CONST_TEXT_PTR PlaneRegionMessage,
                         MIL_CONST_TEXT_PTR FixtureRefMessage)
   {
   auto Mil3dDisp = Alloc3dDisplayId(MilSystem);
   auto MilProc3dDisp = Alloc3dDisplayId(MilSystem);
   auto MilDisp = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   // Position the processing windows.
   auto DisplaySizeX = M3ddispInquire(Mil3dDisp, M_SIZE_X, M_NULL);
   M3ddispControl(MilProc3dDisp, M_WINDOW_INITIAL_POSITION_X, DisplaySizeX);
   MdispControl(MilDisp, M_WINDOW_INITIAL_POSITION_X, DisplaySizeX);

   // Get the id of the graphic list associated to the 3D display.
   MIL_ID Mil3dGraList = M_NULL;
   MIL_ID MilProc3dGraList = M_NULL;
   M3ddispInquire(Mil3dDisp, M_3D_GRAPHIC_LIST_ID, &Mil3dGraList);
   M3ddispInquire(MilProc3dDisp, M_3D_GRAPHIC_LIST_ID, &MilProc3dGraList);

   // Associate a graphic list to the 2d display.
   auto Mil2dGraList = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);
   MdispControl(MilDisp, M_ASSOCIATED_GRAPHIC_LIST_ID, Mil2dGraList);

   std::vector<MIL_UNIQUE_BUF_ID> MilOriginalPointClouds;
   std::vector<MIL_UNIQUE_3DGEO_ID> MilTransformationMatrices;

   bool IsSuccess = true;
   MIL_INT StepsSleepTime = -1;
   bool DisplaySteps = true;
   for(MIL_INT p = 0; p < NB_POINT_CLOUDS && IsSuccess; p++)
      {
      if(p == 1)
         {
         StepsSleepTime = AUTO_STEPS_SLEEP_TIME;
         DisplaySteps = ALWAYS_DISPLAY_STEPS;
         MosPrintf(MIL_TEXT("The same process will be applied to the other point clouds.\n"));
         WaitForKey();
         }

      // Restore the point cloud.
      MIL_STRING PointCloudFile = PT_CLD_FILE + M_TO_STRING(p) + MIL_TEXT(".mbufc");
      CheckForRequiredMILFile(PointCloudFile);
      MilOriginalPointClouds.push_back(MbufRestore(PointCloudFile, MilSystem, M_UNIQUE_ID));

      // Get a copy of the point cloud.
      auto MilPointCloud = MbufClone(MilOriginalPointClouds.back(), M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_UNIQUE_ID);

      // Make sure the point cloud is organized
      auto MilRangeComponent = MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_COMPONENT_ID, M_NULL);
      if(MbufInquire(MilRangeComponent, M_3D_REPRESENTATION, M_NULL) != M_CALIBRATED_XYZ_UNORGANIZED)
         {
         // Display the point cloud.
         auto PointCloudLabel = M3ddispSelect(Mil3dDisp, MilOriginalPointClouds.back(), M_SELECT, M_DEFAULT);
         M3dgraControl(Mil3dGraList, PointCloudLabel, M_OPACITY, POINT_CLOUD_OPACITY);
         M3ddispSetView(Mil3dDisp, M_VIEW_ORIENTATION, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         M3ddispSetView(Mil3dDisp, M_VIEW_BOX, M_WHOLE_SCENE, DISPLAY_INITIAL_VIEW_BOX_RATIO, M_DEFAULT, M_DEFAULT);
         MosPrintf(MIL_TEXT("The point cloud from camera %d is displayed.\n"), p);
         WaitForKey(StepsSleepTime);

         // Add the display processing point cloud.
         auto ProcPointCloudLabel = M3ddispSelect(MilProc3dDisp, MilPointCloud, M_ADD, M_DEFAULT);
         M3dgraControl(MilProc3dGraList, ProcPointCloudLabel, M_OPACITY, POINT_CLOUD_OPACITY);
         M3ddispSetView(MilProc3dDisp, M_VIEW_ORIENTATION, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         M3ddispSetView(MilProc3dDisp, M_VIEW_BOX, M_WHOLE_SCENE, DISPLAY_INITIAL_VIEW_BOX_RATIO, M_DEFAULT, M_DEFAULT);

         // Convert the reflectance of the point cloud to grayscale if necessary.
         if(ConvertReflectanceToGray(MilPointCloud))
            {
            if(DisplaySteps)
               {
               M3ddispSelect(MilProc3dDisp, M_NULL, M_OPEN, M_DEFAULT);
               MosPrintf(MIL_TEXT("The reflectance was converted to grayscale.\n"));
               }
            WaitForKey(StepsSleepTime);
            }
         
         // Locate a region where to fit the plane if necessary.
         LocatePlaneRegion(MilFixtureContext, MilPointCloud);
         if(DisplaySteps)
            {
            M3ddispSelect(MilProc3dDisp, M_NULL, M_OPEN, M_DEFAULT);
            MosPrintf(PlaneRegionMessage);
            }
         WaitForKey(StepsSleepTime);

         // Fit a plane on the point cloud using a small outlier distance.
         auto MilPlaneGeo = FitPlane(MilPointCloud);

         // Display the fit plane in the display.
         M3ddispControl(MilProc3dDisp, M_UPDATE, M_DISABLE);
         MIL_INT64 PlaneLabel = M3dgeoDraw3d(M_DEFAULT, MilPlaneGeo, MilProc3dGraList, M_ROOT_NODE, M_DEFAULT);
         M3dgraControl(MilProc3dGraList, PlaneLabel, M_OPACITY, PLANE_OPACITY);
         M3dgraControl(MilProc3dGraList, PlaneLabel, M_COLOR, M_COLOR_RED);
         M3ddispControl(MilProc3dDisp, M_UPDATE, M_ENABLE);
         if(DisplaySteps)
            MosPrintf(MIL_TEXT("A plane was fit on the reference surface.\n"));
         WaitForKey(StepsSleepTime);

         // Clear the graphic list.
         M3dgraRemove(MilProc3dGraList, M_ALL, M_DEFAULT);

         // Fixture the point cloud.
         auto MilPlaneFixtureMatrix = FixtureToPlane(MilPointCloud, MilPlaneGeo);
         M3ddispSelect(MilProc3dDisp, M_NULL, M_CLOSE, M_DEFAULT);

         // Generate a depth map and an intensity map by projecting the point cloud on the plane.
         auto DepthAndIntensity = GenerateDepthMap(MilPointCloud);

         // Display the intensity map.
         MdispSelect(MilDisp, DepthAndIntensity.MilIntensityMap);
         if(DisplaySteps)
            MosPrintf(MIL_TEXT("By projecting the grayscale reflectance data, we can create an intensity map.\n"));
         WaitForKey(StepsSleepTime);

         // Find the fixture in the plane coordinate system.
         auto Mil2dFixtureMatrix = Find2dFixture(MilFixtureContext, DepthAndIntensity.MilIntensityMap, Mil2dGraList);
         if(DisplaySteps && p == 0)
            MosPrintf(MIL_TEXT("%s\n"), FixtureRefMessage);

         if(Mil2dFixtureMatrix)
            {
            // Compute a global transformation matrix that is the composition of the two transformation matrices.
            MilTransformationMatrices.push_back(M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID));
            M3dgeoMatrixSetTransform(MilTransformationMatrices.back(), M_COMPOSE_TWO_MATRICES, Mil2dFixtureMatrix, MilPlaneFixtureMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT);

            // Draw the axis of the transformation.
            auto MilFixtureTransformation = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
            M3dgeoMatrixSetTransform(MilFixtureTransformation, M_INVERSE, MilTransformationMatrices.back(), M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
            M3dgraControl(Mil3dGraList, M_DEFAULT_SETTINGS, M_THICKNESS, AXIS_THICKNESS);
            MIL_INT64 AxisLabel = M3dgraAxis(Mil3dGraList, M_DEFAULT, MilFixtureTransformation, 0.25, M_NULL, M_DEFAULT, M_DEFAULT);
            M3dgraControl(Mil3dGraList, M_DEFAULT_SETTINGS, M_THICKNESS, M_DEFAULT);

            // Print message.
            MosPrintf(MIL_TEXT("The 2d fixture, found in the intensity map, is displayed.\n"));
            WaitForKey();

            // Remove the axis.
            M3dgraRemove(Mil3dGraList, AxisLabel, M_DEFAULT);
            }
         else
            {
            IsSuccess = false;
            MosPrintf(MIL_TEXT("Unable to find the 2d fixture!\n\n"));
            }

         // Clear the graphic list and close the display.
         MdispSelect(MilDisp, M_NULL);
         MgraClear(M_DEFAULT, Mil2dGraList);
         }
      else
         {
         IsSuccess = false;
         MosPrintf(MIL_TEXT("This application cannot use unorganized point clouds.\n\n"), p);
         }
      }

   // Close the 3d display.
   if(IsSuccess)
      {
      MosPrintf(MIL_TEXT("Transforming and merging the point clouds...\n\n"));

      // Transform all the point clouds.
      for(MIL_INT p = 0; p < (MIL_INT)MilOriginalPointClouds.size(); p++)
         M3dimMatrixTransform(MilOriginalPointClouds[p], MilOriginalPointClouds[p], MilTransformationMatrices[p], M_DEFAULT);

      // Merge all the point cloud into one.
      auto MilMergePointCloud = MbufAllocContainer(MilSystem, M_DISP + M_PROC, M_DEFAULT, M_UNIQUE_ID);
      M3dimMerge(MilOriginalPointClouds.data(), MilMergePointCloud, MilOriginalPointClouds.size(), M_NULL, M_DEFAULT);

      // Display the merged result.
      DisplayMergedResult(Mil3dDisp, MilProc3dDisp, MilMergePointCloud);
      WaitForKey();
      }
   else
      WaitForKey();
   }

//****************************************************************************
// Converts the reflectance of the point cloud to grayscale if necessary.
//****************************************************************************
bool ConvertReflectanceToGray(MIL_ID MilPointCloud)
   {
   MIL_ID MilReflectance = MbufInquireContainer(MilPointCloud, M_COMPONENT_REFLECTANCE, M_COMPONENT_ID, M_NULL);

   // Convert the reflectance in the point cloud if required.
   if(MbufInquire(MilReflectance, M_SIZE_BAND, M_NULL) > 1)
      {
      MIL_INT SizeX = MbufInquire(MilReflectance, M_SIZE_X, M_NULL);
      MIL_INT SizeY = MbufInquire(MilReflectance, M_SIZE_Y, M_NULL);
      MIL_ID MilReflectanceGray = MbufAllocComponent(MilPointCloud, 1, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, M_COMPONENT_REFLECTANCE, M_NULL);

      // Perform a color to luminance conversion.
      MimConvert(MilReflectance, MilReflectanceGray, M_RGB_TO_L);

      // Free the old multi-band reflectance component.
      MbufFreeComponent(MilPointCloud, M_COMPONENT_BY_ID(MilReflectance), M_DEFAULT);
      MilReflectance = MilReflectanceGray;

      return true;
      }
   else
      return false;
   }

//****************************************************************************
// Depth map generation from a point cloud.
//****************************************************************************
SDepthIntensity GenerateDepthMap(MIL_ID MilPointCloudContainer)
   {
	SDepthIntensity DepthIntensity;
   auto MilSystem = MbufInquire(MilPointCloudContainer, M_OWNER_SYSTEM, M_NULL);
   auto MilPointCloudContainerClone = MbufClone(MilPointCloudContainer, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Create the mesh component of the depth map.
   M3dimMesh(M_MESH_CONTEXT_ORGANIZED, MilPointCloudContainer, MilPointCloudContainerClone, M_DEFAULT);

   // Compute the image size required to hold the depth map.
   auto MilMapSizeContext = M3dimAlloc(MilSystem, M_CALCULATE_MAP_SIZE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   MIL_INT DepthMapSizeX, DepthMapSizeY;
   M3dimCalculateMapSize(MilMapSizeContext, MilPointCloudContainerClone, M_NULL, M_DEFAULT, &DepthMapSizeX, &DepthMapSizeY);
   
   // Calibrate the depth map and intensity map images in order to express coordinates with respect to the working coordinate system.
	DepthIntensity.MilDepthMap = MbufAlloc2d(MilSystem, DepthMapSizeX, DepthMapSizeY, M_UNSIGNED + 16, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
	DepthIntensity.MilIntensityMap = MbufAlloc2d(MilSystem, DepthMapSizeX, DepthMapSizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   M3dimCalibrateDepthMap(MilPointCloudContainerClone, DepthIntensity.MilDepthMap, DepthIntensity.MilIntensityMap, M_NULL, M_DEFAULT, M_POSITIVE, M_DEFAULT);

   // Project the point cloud on the depth map.
   M3dimProject(MilPointCloudContainerClone, DepthIntensity.MilDepthMap, DepthIntensity.MilIntensityMap, M_MESH_BASED, M_MAX_Z, M_DEFAULT, M_DEFAULT);
 
	return std::move(DepthIntensity);
   }

//****************************************************************************
// Fits a plane on the point cloud.
//****************************************************************************
MIL_UNIQUE_3DGEO_ID FitPlane(MIL_ID MilPointCloud)
   {
   auto MilSystem = MbufInquire(MilPointCloud, M_OWNER_SYSTEM, M_NULL);
   auto MilPlaneGeo = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   auto MilFitResult = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_UNIQUE_ID);
   auto MilFitContext = M3dmetAlloc(MilSystem, M_FIT_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dmetFit(M_DEFAULT, MilPointCloud, M_PLANE, MilFitResult, PLANE_TOLERANCE, M_DEFAULT);
   M3dmetCopyResult(MilFitResult, MilPlaneGeo, M_FITTED_GEOMETRY, M_DEFAULT);
   return MilPlaneGeo;
   }

//****************************************************************************
// Fixture the point cloud on the plane.
//****************************************************************************
MIL_UNIQUE_3DGEO_ID FixtureToPlane(MIL_ID MilPointCloud, MIL_ID MilPlaneGeo)
   {
   auto MilSystem = MbufInquire(MilPointCloud, M_OWNER_SYSTEM, M_NULL);

   // Compute the transformation matrix that moves the camera’s XY plane onto the fitted plane.
   auto MilPlaneTransformMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MilPlaneTransformMatrix, M_FIXTURE_TO_PLANE, MilPlaneGeo, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Apply the transformation matrix to the point cloud.
   M3dimMatrixTransform(MilPointCloud, MilPointCloud, MilPlaneTransformMatrix, M_DEFAULT);

   return MilPlaneTransformMatrix;
   }

//****************************************************************************
// Locate the plane region. With general feature fixturing we are just
// cropping the middle of the organized data.
//****************************************************************************
void LocatePlaneRegion(const MIL_UNIQUE_MOD_ID&, MIL_ID MilPointCloud)
   {
   MIL_ID MilConfidence = MbufInquireContainer(MilPointCloud, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   MIL_INT SizeX = MbufInquire(MilConfidence, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquire(MilConfidence, M_SIZE_Y, M_NULL);

   auto MilRegionConfidence = MbufClone(MilConfidence, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MbufClear(MilRegionConfidence, 0);
   MgraColor(M_DEFAULT, M_COLOR_WHITE);
   MIL_DOUBLE CenterX = (SizeX - 1) * 0.5;
   MIL_DOUBLE CenterY = (SizeY - 1) * 0.5;
   MIL_DOUBLE RectSizeX = SizeX * GENERAL_CROPPING_SIZE_RATIO;
   MIL_DOUBLE RectSizeY = SizeY * GENERAL_CROPPING_SIZE_RATIO;
   MgraRectAngle(M_DEFAULT, MilRegionConfidence, CenterX, CenterY, RectSizeX, RectSizeY, 0.0, M_CENTER_AND_DIMENSION + M_FILLED);
   MimArith(MilConfidence, MilRegionConfidence, MilConfidence, M_AND);
   }

//****************************************************************************
// Locate the plane region. With data matrix fixturing, the plane region is
// only the datamatrix, including its quiet zone.
//****************************************************************************
void LocatePlaneRegion(const MIL_UNIQUE_CODE_ID& MilCodeContext, MIL_ID MilPointCloud)
   {
   auto MilSystem = MbufInquire(MilPointCloud, M_OWNER_SYSTEM, M_NULL);
   MIL_ID MilReflectance = MbufInquireContainer(MilPointCloud, M_COMPONENT_REFLECTANCE, M_COMPONENT_ID, M_NULL);

   // Allocate a code reader result buffer.
   auto MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Read the code
   McodeRead(MilCodeContext, MilReflectance, MilCodeResult);

   MIL_INT ReadStatus = 0;
   McodeGetResult(MilCodeResult, M_GENERAL, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &ReadStatus);
   if(ReadStatus == M_STATUS_READ_OK)
      {
      // Get the corners of the code box.
      const MIL_INT CORNER_X[4] = {M_QUIET_ZONE_TOP_LEFT_X, M_QUIET_ZONE_TOP_RIGHT_X,
                                   M_QUIET_ZONE_BOTTOM_RIGHT_X, M_QUIET_ZONE_BOTTOM_LEFT_X};
      const MIL_INT CORNER_Y[4] = {M_QUIET_ZONE_TOP_LEFT_Y, M_QUIET_ZONE_TOP_RIGHT_Y,
                                   M_QUIET_ZONE_BOTTOM_RIGHT_Y, M_QUIET_ZONE_BOTTOM_LEFT_Y};
      MIL_DOUBLE Cx[4];
      MIL_DOUBLE Cy[4];
      for(MIL_INT c = 0; c < 4; c++)
         {
         McodeGetResult(MilCodeResult, 0, M_DEFAULT, CORNER_X[c], &Cx[c]);
         McodeGetResult(MilCodeResult, 0, M_DEFAULT, CORNER_Y[c], &Cy[c]);
         }

      MIL_ID MilConfidence = MbufInquireContainer(MilPointCloud, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);

      // Set the confidence to only contain the code box.
      auto MilRegionConfidence = MbufClone(MilConfidence, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
      MbufClear(MilRegionConfidence, 0);
      MgraColor(M_DEFAULT, M_COLOR_WHITE);
      MgraLines(M_DEFAULT, MilRegionConfidence, 4, Cx, Cy, M_NULL, M_NULL, M_POLYGON + M_FILLED);
      MimArith(MilConfidence, MilRegionConfidence, MilConfidence, M_AND);
      }
   }

//****************************************************************************
// Fixture the plane in 2d.
//****************************************************************************
MIL_UNIQUE_3DGEO_ID Find2dFixture(const MIL_UNIQUE_MOD_ID& MilModContext, MIL_ID MilPlaneReflectance, MIL_ID MilGraList)
   {
   auto MilSystem = MbufInquire(MilPlaneReflectance, M_OWNER_SYSTEM, M_NULL);

   // Allocate a tansformation matrix object.
   auto Mil2dFixtureMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   // Define the model from the reference image.
   if(MmodInquire(MilModContext, M_CONTEXT, M_NUMBER_MODELS, M_NULL) == 0)
      {
      MmodDefine(MilModContext, M_IMAGE, MilPlaneReflectance, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      // Add masking based on the missing data.
      auto MilMissingEdgeMask = MbufClone(MilPlaneReflectance, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
      MimBinarize(MilPlaneReflectance, MilMissingEdgeMask, M_EQUAL, 0, M_NULL);
      MimDilate(MilMissingEdgeMask, MilMissingEdgeMask, 2, M_BINARY);
      MmodMask(MilModContext, 0, MilMissingEdgeMask, M_FLAT_REGIONS, M_DEFAULT);

      // Setup and preprocess the context.
      MmodControl(MilModContext, 0, M_SCALE_MIN_FACTOR, 1.0);
      MmodControl(MilModContext, 0, M_SCALE_MAX_FACTOR, 1.0);
      MmodPreprocess(MilModContext, M_DEFAULT);
      }

   // Allocate a model finder result.
   auto MilModResult = MmodAllocResult(MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Find the model in the target image.
   MmodFind(MilModContext, MilPlaneReflectance, MilModResult);

   MIL_INT NbFound = 0;
   MmodGetResult(MilModResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbFound);
   if(NbFound)
      {
      // Get the position and angle of the model.
      MIL_DOUBLE PosX;
      MIL_DOUBLE PosY;
      MIL_DOUBLE Angle;
      MmodGetResult(MilModResult, 0, M_POSITION_X, &PosX);
      MmodGetResult(MilModResult, 0, M_POSITION_Y, &PosY);
      MmodGetResult(MilModResult, 0, M_ANGLE, &Angle);

      // Set the 2d fixture matrix.
      SetTransformationMatrixFrom2dFixture(Mil2dFixtureMatrix, PosX, PosY, Angle);

      // Draw the result in the graphic list.
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MmodDraw(M_DEFAULT, MilModResult, MilGraList, M_DRAW_EDGES, 0, M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      MmodDraw(M_DEFAULT, MilModResult, MilGraList, M_DRAW_BOX, 0, M_DEFAULT);
      MmodDraw(M_DEFAULT, MilModResult, MilGraList, M_DRAW_POSITION, 0, M_DEFAULT);
      }

   return Mil2dFixtureMatrix;
   }

//****************************************************************************
// Fixture the plane in 2d.
//****************************************************************************
MIL_UNIQUE_3DGEO_ID Find2dFixture(const MIL_UNIQUE_CODE_ID& MilCodeContext, MIL_ID MilPlaneReflectance, MIL_ID MilGraList)
   {
   auto MilSystem = MbufInquire(MilPlaneReflectance, M_OWNER_SYSTEM, M_NULL);

   // Allocate a tansformation matrix object.
   auto Mil2dFixtureMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   // Allocate a code reader result buffer.
   auto MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Read the code.
   McodeRead(MilCodeContext, MilPlaneReflectance, MilCodeResult);

   MIL_INT ReadStatus = 0;
   McodeGetResult(MilCodeResult, M_GENERAL, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &ReadStatus);
   if(ReadStatus == M_STATUS_READ_OK)
      {
      // Get the position and angle of the datamatrix.
      MIL_DOUBLE CodePosX;
      MIL_DOUBLE CodePosY;
      MIL_DOUBLE CodeAngle;
      McodeGetResult(MilCodeResult, 0, M_DEFAULT, M_POSITION_X, &CodePosX);
      McodeGetResult(MilCodeResult, 0, M_DEFAULT, M_POSITION_Y, &CodePosY);
      McodeGetResult(MilCodeResult, 0, M_DEFAULT, M_ANGLE, &CodeAngle);

      // Set the 2d fixture matrix.
      SetTransformationMatrixFrom2dFixture(Mil2dFixtureMatrix, CodePosX, CodePosY, CodeAngle);

      // Draw the result in the graphic list.
      MgraColor(M_DEFAULT, M_COLOR_RED);
      McodeDraw(M_DEFAULT, MilCodeResult, MilGraList, M_DRAW_CODE, 0, M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      McodeDraw(M_DEFAULT, MilCodeResult, MilGraList, M_DRAW_BOX, 0, M_DEFAULT);
      McodeDraw(M_DEFAULT, MilCodeResult, MilGraList, M_DRAW_POSITION, 0, M_DEFAULT);
      }

   return Mil2dFixtureMatrix;
   }

//****************************************************************************
// Sets a 3d transformation matrix that corresponds to a 2d fixturing in the XY plane.
// The X and Z axes will be used to describe the required translation and rotation.
// Mathematically compute the x and y components describing the rotation of the X-axis (note: the z component is 1)
// Note: The negative value of the angle is used because the angles in MIL are inverted from the mathematical convention.
//****************************************************************************
void SetTransformationMatrixFrom2dFixture(MIL_ID MilMatrix, MIL_DOUBLE PosX, MIL_DOUBLE PosY, MIL_DOUBLE Angle)
   {
   MIL_DOUBLE XAxisRotationComponentX = cos(-Angle * DIV_PI_180);
   MIL_DOUBLE XAxisRotationComponentY = sin(-Angle * DIV_PI_180);
   M3dgeoMatrixSetWithAxes(MilMatrix, M_XZ_AXES + M_COORDINATE_SYSTEM_TRANSFORMATION, PosX, PosY, 0.0,
                           XAxisRotationComponentX, XAxisRotationComponentY, 0, 0, 0, 1, M_DEFAULT);
   }

//****************************************************************************
// Displays the merged results.
//****************************************************************************
void DisplayMergedResult(MIL_ID MilReflectanceDisplay, MIL_ID MilRangeDisp, MIL_ID MilMergePointCloud)
   {
   auto MilSystem = MbufInquire(MilMergePointCloud, M_OWNER_SYSTEM, M_NULL);

   // For display purposes, determine the bounding box of the point cloud, ignore outlier points.
   auto MilStatContext = M3dimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto MilBoxGeo = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   auto MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MilStatContext, M_BOUNDING_BOX_ALGORITHM, M_ROBUST);
   M3dimControl(MilStatContext, M_BOUNDING_BOX, M_ENABLE);
   M3dimControl(MilStatContext, M_BOUNDING_BOX_OUTLIER_RATIO_Z, DISPLAY_ROBUST_BOX_OUTLIER_RATIO);
   M3dimStat(MilStatContext, MilMergePointCloud, MilStatResult, M_DEFAULT);
   M3dimCopyResult(MilStatResult, MilBoxGeo, M_BOUNDING_BOX, M_DEFAULT);

   // Crop the point cloud to exclude outliers.
   M3dimCrop(MilMergePointCloud, MilMergePointCloud, MilBoxGeo, M_NULL, M_DEFAULT, M_DEFAULT);

   // Add the point cloud to a first display.
   auto MilGralist = (MIL_ID)M3ddispInquire(MilReflectanceDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraAdd(MilGralist, M_DEFAULT, MilMergePointCloud, M_DEFAULT);

   // Create a second display where the points are colored with a lut on the range component.
   auto MilGralist2 = (MIL_ID)M3ddispInquire(MilRangeDisp, M_3D_GRAPHIC_LIST_ID, M_NULL);
   auto PointCloudLabel = M3dgraAdd(MilGralist2, M_DEFAULT, MilMergePointCloud, M_DEFAULT);
   M3dgraControl(MilGralist2, PointCloudLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
   M3dgraControl(MilGralist2, PointCloudLabel, M_COLOR_COMPONENT_BAND, 2);
   M3dgraControl(MilGralist2, PointCloudLabel, M_COLOR_USE_LUT, M_TRUE);
   MIL_INT SizeX = (MIL_INT)M3ddispInquire(MilRangeDisp, M_SIZE_X, M_NULL);
   M3ddispControl(MilRangeDisp, M_WINDOW_INITIAL_POSITION_X, SizeX);

   // Open the display windows.
   M3ddispSetView(MilReflectanceDisplay, M_VIEW_BOX, M_WHOLE_SCENE, DISPLAY_MERGED_VIEW_BOX_RATIO, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilReflectanceDisplay, M_VIEW_ORIENTATION, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilRangeDisp, M_VIEW_BOX, M_WHOLE_SCENE, DISPLAY_MERGED_VIEW_BOX_RATIO, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilRangeDisp, M_VIEW_ORIENTATION, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSelect(MilReflectanceDisplay, M_NULL, M_OPEN, M_DEFAULT);
   M3ddispSelect(MilRangeDisp, M_NULL, M_OPEN, M_DEFAULT);

   MosPrintf(MIL_TEXT("The merged point cloud, respectively colored with the reflectance and\n"));
   MosPrintf(MIL_TEXT("a color map of the Z-coordinates, is displayed.\n"));
   }

//****************************************************************************
// Check for required files to run the example.
//****************************************************************************
void CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));
      
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
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
                MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      exit(0);
      }
   return MilDisplay3D;
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.
//*****************************************************************************
void WaitForKey(MIL_INT SleepTime)
   {
   if(SleepTime == -1)
      {
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }
   else if(SleepTime != 0)
      {
      MosPrintf(MIL_TEXT("\n"));
      MosSleep(SleepTime);
      if(MosKbhit())
         {
         MosGetch();
         MosPrintf(MIL_TEXT("Press <Enter> to resume.\n\n"));
         MosGetch();
         }
      }
   }
