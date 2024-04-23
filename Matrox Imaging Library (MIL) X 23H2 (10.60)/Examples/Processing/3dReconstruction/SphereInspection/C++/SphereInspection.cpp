﻿//***************************************************************************************/
//
// File name: SphereInspection.cpp
//
// Synopsis:  This program contains an example where spheres are inspected using the
//            mil3dmet module.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>

// Structure holding the coordinates of a 3D box.
struct SBox
   {
   MIL_DOUBLE MinX, MinY, MinZ, MaxX, MaxY, MaxZ;
   };

// Source file specification.
static const MIL_STRING PT_CLD_FILE = M_IMAGE_PATH MIL_TEXT("SphereInspection/PlaneHemisphere.ply");
static const MIL_STRING ILLUSTRATION_FILE = M_IMAGE_PATH MIL_TEXT("SphereInspection/SphereInspectionIllustration.png");
static const MIL_INT ILLUSTRATION_OFFSET_X = 800;

// Extraction box around each sphere (in mm).
static const MIL_INT NUM_SPHERES = 6;
static const SBox SPHERE_BOX[NUM_SPHERES] =
   { // MinX,  MinY,  MinZ,  MaxX,  MaxY,  MaxZ
      {-16.0,  24.0, -15.0,   7.3,  50.0,  15.0},
      {-16.0,  -1.0, -15.0,   8.0,  24.0,  15.0},
      {-16.0, -26.0, -15.0,   8.5,  -1.0,  15.0},
      {  7.3,  24.0, -15.0,  32.0,  50.0,  15.0},
      {  8.0,  -1.0, -15.0,  32.0,  24.0,  15.0},
      {  8.5, -26.0, -15.0,  32.0,  -1.0,  15.0},
   };

// Safe distance above fitted plane (in mm).
static const MIL_DOUBLE DISTANCE_ABOVE_PLANE = 7.0;

// Values used for validation.
static const MIL_DOUBLE EXPECTED_RADIUS  = 11.3;
static const MIL_FLOAT  RADIUS_TOLERANCE =  1.0f;

// Distance component type.
static const MIL_INT DISTANCE_COMPONENT = M_COMPONENT_CUSTOM + 1;

// Forward declarations.
bool   CheckForRequiredMILFile(const MIL_STRING& FileName);
void   InspectSpheres(MIL_ID MilSystem);
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("SphereInspection\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to inspect 3D sphere objects.\n")
             MIL_TEXT("Objects are isolated from their underlying surface using \n")
             MIL_TEXT("3D geometry fitting operations (for plane and sphere).\n")
             MIL_TEXT("3D points near the fitted geometry are selected.\n")
             MIL_TEXT("The distance between each sphere center and the selected\n")
             MIL_TEXT("points is used to detect defects.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Geometry, 3D Metrology, 3D Image Processing,\n")
             MIL_TEXT("3D Display, Display, Buffer, Image Processing and 3D Graphics.\n\n"));
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_ID MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);

   // Show illustration of light orientations.
   MIL_ID IllustrationDispId = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID IllustrationImageId = MbufRestore(ILLUSTRATION_FILE, MilSystem, M_NULL);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Object to inspect."));
   MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_X, ILLUSTRATION_OFFSET_X);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   PrintHeader();

   // Check for required example files.
   if (CheckForRequiredMILFile(PT_CLD_FILE))
      {
      MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
      MosGetch();

      InspectSpheres(MilSystem);
      }

   MdispFree(IllustrationDispId);
   MbufFree(IllustrationImageId);

   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

const MIL_DOUBLE CameraPos    [3] = { -152.4, 104.7, 153.0 };
const MIL_DOUBLE InterestPoint[3] = {    2.9,  17.8, -13.8 };
const MIL_DOUBLE UpVector     [3] = {    0.4,  -0.6,   0.7 };

//*****************************************************************************
// Main processing function.
//*****************************************************************************
void InspectSpheres(MIL_ID MilSystem)
   {
   // Restore the point cloud.
   MIL_ID MilPointCloud   = MbufRestore(PT_CLD_FILE, MilSystem, M_NULL);
   MIL_ID MilCroppedCloud = MbufAllocContainer(MilSystem, M_PROC, M_DEFAULT, M_NULL);

   // Allocate the 3D display.
   MIL_ID Mil3dDisplay = Alloc3dDisplayId(MilSystem);

   MosPrintf(MIL_TEXT("A 3D point cloud is restored from a PLY file and displayed.\n\n"));

   MIL_ID MilGraphicList = M_NULL;
   M3ddispInquire(Mil3dDisplay, M_3D_GRAPHIC_LIST_ID, &MilGraphicList);

   M3ddispSetView(Mil3dDisplay, M_VIEWPOINT, CameraPos[0], CameraPos[1], CameraPos[2], M_NO_REFRESH);
   M3ddispSetView(Mil3dDisplay, M_INTEREST_POINT, InterestPoint[0], InterestPoint[1], InterestPoint[2], M_NO_REFRESH);
   M3ddispSetView(Mil3dDisplay, M_UP_VECTOR, UpVector[0], UpVector[1], UpVector[2], M_DEFAULT);

   // Create a distance component.
   MIL_INT SizeX = MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   MIL_ID MilDistances = MbufAllocComponent(MilPointCloud, 1, SizeX, SizeY, M_FLOAT + 32, M_IMAGE + M_PROC, DISTANCE_COMPONENT, M_NULL);

   // Display the point cloud.
   MIL_INT64 MilContainerGraphics = M3ddispSelect(Mil3dDisplay, MilPointCloud, M_SELECT, M_DEFAULT);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_USE_LUT, M_TRUE);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT_BAND, 2);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Fit a plane on the background.
   MIL_ID MilFitResult = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_NULL);
   M3dmetFit(M_DEFAULT, MilPointCloud, M_PLANE, MilFitResult, DISTANCE_ABOVE_PLANE, M_DEFAULT);

   // Only keep points that are not part of the background plane.
   MIL_ID MilConfidence = MbufInquireContainer(MilPointCloud, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   M3dmetCopyResult(MilFitResult, MilConfidence, M_OUTLIER_MASK, M_DEFAULT);

   MosPrintf(MIL_TEXT("A plane is fitted on the point cloud.\n"));
   MosPrintf(MIL_TEXT("Points above the fitted plane are kept.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Each sphere in the point cloud is isolated using an extraction box.\n"));
   MosPrintf(MIL_TEXT("A fitted sphere is used to find defects. The sphere points are\n"));
   MosPrintf(MIL_TEXT("displayed using color to indicate the distance to the expected sphere.\n\n"));

   MosPrintf(MIL_TEXT("Expected radius: %4.1f mm\n"  ), EXPECTED_RADIUS);
   MosPrintf(MIL_TEXT("Tolerance:       %4.1f mm\n\n"), RADIUS_TOLERANCE);

   MosPrintf(MIL_TEXT("Press <Enter> to go from one sphere to the next.\n\n"));

   MosPrintf(MIL_TEXT("Index   Center (X, Y, Z)   Radius  MaxError  Status\n"));
   MosPrintf(MIL_TEXT("---------------------------------------------------\n"));

   // Color the point cloud according to the distances.
   M3ddispControl(Mil3dDisplay, M_UPDATE, M_DISABLE);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, DISTANCE_COMPONENT);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT_BAND, M_ALL_BANDS);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_LIMITS, M_USER_DEFINED);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_LIMITS_MIN, 0);
   M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_LIMITS_MAX, RADIUS_TOLERANCE * 2);
   M3ddispControl(Mil3dDisplay, M_UPDATE, M_ENABLE);

   // Analyze each sphere separately.
   for (MIL_INT i = 0; i < NUM_SPHERES; ++i)
      {
      // Set the region where the sphere will be fitted.
      MIL_ID MilExtractionBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);
      M3dgeoBox(MilExtractionBox, M_BOTH_CORNERS,
                SPHERE_BOX[i].MinX, SPHERE_BOX[i].MinY, SPHERE_BOX[i].MinZ,
                SPHERE_BOX[i].MaxX, SPHERE_BOX[i].MaxY, SPHERE_BOX[i].MaxZ,
                M_DEFAULT);

      M3dimCrop(MilPointCloud, MilCroppedCloud, MilExtractionBox, M_NULL, M_SAME, M_DEFAULT);

      // Fit the sphere.
      MIL_ID MilSphereGeometry = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);
      M3dmetFit(M_DEFAULT, MilCroppedCloud, M_SPHERE, MilFitResult, M_INFINITE, M_DEFAULT);
      M3dmetCopyResult(MilFitResult, MilSphereGeometry, M_FITTED_GEOMETRY, M_DEFAULT);

      // Calculate the maximum error.
      MIL_ID MilStatResult = M3dmetAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_NULL);
      M3dmetStat(M_STAT_CONTEXT_MAX, MilCroppedCloud, MilSphereGeometry, MilStatResult, M_ABSOLUTE_DISTANCE_TO_SURFACE, M_ALL, M_NULL, M_NULL, M_DEFAULT);
      MIL_DOUBLE MaxError;
      M3dmetGetResult(MilStatResult, M_STAT_MAX, &MaxError);
      M3dmetFree(MilStatResult);

      // Calculate the distance from each point to the spheres and scale them.
      // This will appear as a heat map in the 3D display.
      MIL_ID MilSphereDistImg = MbufAlloc2d(MilSystem, SizeX, SizeY, M_FLOAT + 32, M_IMAGE + M_PROC, M_NULL);
      M3dmetDistance(MilCroppedCloud, MilSphereGeometry, MilSphereDistImg, M_ABSOLUTE_DISTANCE_TO_SURFACE, M_DEFAULT, M_DEFAULT);
      MilConfidence = MbufInquireContainer(MilCroppedCloud, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
      MbufSetRegion(MilSphereDistImg, MilConfidence, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MbufCopy(MilSphereDistImg, MilDistances);

      // Display the sphere parameters in the console.
      MIL_DOUBLE CenterX, CenterY, CenterZ, Radius;
      M3dgeoInquire(MilSphereGeometry, M_CENTER_X, &CenterX);
      M3dgeoInquire(MilSphereGeometry, M_CENTER_Y, &CenterY);
      M3dgeoInquire(MilSphereGeometry, M_CENTER_Z, &CenterZ);
      M3dgeoInquire(MilSphereGeometry, M_RADIUS, &Radius);
      bool HasFailed = (MaxError >= RADIUS_TOLERANCE);
      MosPrintf(MIL_TEXT("  %d   (% 5.1f, % 5.1f, % 4.1f)  %4.1f     %4.2f     %s\n"),
                static_cast<int>(i), CenterX, CenterY, CenterZ, Radius, MaxError,
                HasFailed ? MIL_TEXT("FAIL") : MIL_TEXT(" OK "));

      M3ddispControl(Mil3dDisplay, M_UPDATE, M_DISABLE);

      // Display the sphere in the 3D display.
      MIL_INT64 MilSphereGraphics = M3dgeoDraw3d(M_DEFAULT, MilSphereGeometry, MilGraphicList, M_DEFAULT,  M_DEFAULT);
      M3dgraControl(MilGraphicList, MilSphereGraphics, M_OPACITY, 30);
      M3dgraControl(MilGraphicList, MilSphereGraphics, M_COLOR , (HasFailed ? M_COLOR_RED : M_COLOR_GREEN));

      // Display the box in the 3D display.
      MIL_INT64 MilBoxGraphics = M3dgeoDraw3d(M_DEFAULT, MilExtractionBox, MilGraphicList, M_DEFAULT,  M_DEFAULT);
      M3dgraControl(MilGraphicList, MilBoxGraphics, M_APPEARANCE, M_WIREFRAME);
      M3ddispControl(Mil3dDisplay, M_UPDATE, M_ENABLE);

      MosGetch();
      M3dgraRemove(MilGraphicList, MilBoxGraphics, M_DEFAULT);

      // Display the sphere box in the 3D display.
      MIL_ID MilSphereBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);
      M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilSphereGeometry, MilSphereBox, M_DEFAULT);
      MIL_INT64 MilSphereBoxGraphics = M3dgeoDraw3d(M_DEFAULT, MilSphereBox, MilGraphicList, M_DEFAULT, M_DEFAULT);
      M3dgraControl(MilGraphicList, MilSphereBoxGraphics, M_APPEARANCE, M_WIREFRAME);
      M3dgraControl(MilGraphicList, MilSphereBoxGraphics, M_COLOR, (HasFailed ? M_COLOR_RED : M_COLOR_GREEN));

      // Free MIL objects.
      MbufFree(MilSphereDistImg);
      M3dgeoFree(MilSphereGeometry);
      M3dgeoFree(MilSphereBox);
      M3dgeoFree(MilExtractionBox);
      }

   MosPrintf(MIL_TEXT("\nThe final result is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   // Free MIL objects.
   M3ddispFree(Mil3dDisplay);
   M3dmetFree(MilFitResult);
   MbufFree(MilCroppedCloud);
   MbufFree(MilPointCloud);
   }

//****************************************************************************
// Check for required files to run the example.    
//****************************************************************************
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
