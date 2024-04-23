﻿//***************************************************************************************/
//
// File name: M3dblob.cpp
//
// Synopsis:  This program demonstrates how to use the 3d blob module to
//            identify objects in a scene and separate them into categories.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/
#include <mil.h>

// Source file specification.
static const MIL_STRING CONNECTORS_AND_WASHERS_FILE =
   M_IMAGE_PATH MIL_TEXT("ConnectorsAndWashers.mbufc");
static const MIL_STRING CONNECTORS_AND_WASHERS_ILLUSTRATION_FILE =
   M_IMAGE_PATH MIL_TEXT("ConnectorsAndWashers.png");

static const MIL_STRING TWISTY_PUZZLES_FILE = M_IMAGE_PATH MIL_TEXT("TwistyPuzzles.mbufc");

// Segmentation thresholds.
static const MIL_INT    LOCAL_SEGMENTATION_MIN_NB_POINTS = 100;
static const MIL_INT    LOCAL_SEGMENTATION_MAX_NB_POINTS = 10000;
static const MIL_DOUBLE LOCAL_SEGMENTATION_DISTANCE_THRESHOLD = 0.75;   // in mm

static const MIL_INT    PLANAR_SEGMENTATION_MIN_NB_POINTS = 5000;
static const MIL_DOUBLE PLANAR_SEGMENTATION_NORMAL_THRESHOLD = 20;      // in deg

// Function declarations.
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);

void   IdentificationAndSortingExample(MIL_ID SceneDisplay, MIL_ID IllustrationDisplay);
void   PlanarSegmentationExample(MIL_ID SceneDisplay, MIL_ID IllustrationDisplay);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("M3dblob\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program demonstrates how to use the 3d blob analysis module to\n")
      MIL_TEXT("identify objects in a scene and separate them into categories.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: 3D Blob Analysis, 3D Image Processing,\n")
      MIL_TEXT("3D Display, Display, Buffer, and 3D Graphics.\n\n"));

   MIL_ID MilApplication,  // Application identifier.
          MilSystem;       // System identifier.

   // Allocate defaults.
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, M_NULL, M_NULL);

   // Allocate the displays.
   MIL_ID SceneDisplay = Alloc3dDisplayId(MilSystem);
   if(SceneDisplay == M_NULL)
      {
      MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
      return -1;
      }
   M3ddispControl(SceneDisplay, M_TITLE, MIL_TEXT("Scene"));

   MIL_ID IllustrationDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"),
                                           M_WINDOWED, M_NULL);
   MIL_INT IllustrationOffsetX = M3ddispInquire(SceneDisplay, M_SIZE_X, M_NULL);
   MdispControl(IllustrationDisplay, M_TITLE, MIL_TEXT("Objects to inspect"));
   MdispControl(IllustrationDisplay, M_WINDOW_INITIAL_POSITION_X, IllustrationOffsetX);

   // Run the examples.
   IdentificationAndSortingExample(SceneDisplay, IllustrationDisplay);
   PlanarSegmentationExample(SceneDisplay, IllustrationDisplay);

   MdispFree(IllustrationDisplay);
   M3ddispFree(SceneDisplay);

   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }


//*****************************************************************************
// First example.
//*****************************************************************************
void IdentificationAndSortingExample(MIL_ID SceneDisplay, MIL_ID IllustrationDisplay)
   {
   MIL_ID MilSystem = MobjInquire(SceneDisplay, M_OWNER_SYSTEM, M_NULL);
   MIL_ID SceneGraList = (MIL_ID)M3ddispInquire(SceneDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   // Restore the point cloud and display it.
   MIL_ID MilPointCloud = MbufImport(CONNECTORS_AND_WASHERS_FILE, M_DEFAULT,
                                     M_RESTORE, MilSystem, M_NULL);

   M3dgraRemove(SceneGraList, M_ALL, M_DEFAULT);
   M3dgraControl(SceneGraList, M_DEFAULT_SETTINGS, M_THICKNESS, 3);

   M3ddispSelect(SceneDisplay, MilPointCloud, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(SceneDisplay, M_AUTO, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Show an illustration of the objects in the scene.
   MIL_ID IllustrationImage = MbufRestore(CONNECTORS_AND_WASHERS_ILLUSTRATION_FILE,
                                          MilSystem, M_NULL);
   MdispSelect(IllustrationDisplay, IllustrationImage);

   MosPrintf(MIL_TEXT("A 3D point cloud consisting of wire connectors and washers\n")
             MIL_TEXT("is restored from a file and displayed.\n\n")
             MIL_TEXT("Press <Enter> to segment it into separate objects.\n\n"));
   MosGetch();

   // Allocate the segmentation contexts.
   MIL_ID SegmentationContext = M3dblobAlloc(MilSystem, M_SEGMENTATION_CONTEXT,
                                             M_DEFAULT, M_NULL);
   MIL_ID CalculateContext = M3dblobAlloc(MilSystem, M_CALCULATE_CONTEXT, M_DEFAULT, M_NULL);
   MIL_ID Draw3dContext = M3dblobAlloc(MilSystem, M_DRAW_3D_CONTEXT, M_DEFAULT, M_NULL);

   // Allocate the segmentation results. One result is used for each category.
   MIL_ID AllBlobs = M3dblobAllocResult(MilSystem, M_SEGMENTATION_RESULT, M_DEFAULT, M_NULL);
   MIL_ID Connectors = M3dblobAllocResult(MilSystem, M_SEGMENTATION_RESULT, M_DEFAULT, M_NULL);
   MIL_ID Washers = M3dblobAllocResult(MilSystem, M_SEGMENTATION_RESULT, M_DEFAULT, M_NULL);
   MIL_ID UnknownBlobs = M3dblobAllocResult(MilSystem, M_SEGMENTATION_RESULT,
                                            M_DEFAULT, M_NULL);
   
   // Take advantage of the 2d organization.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NEIGHBOR_SEARCH_MODE, M_ORGANIZED);

   // Look for neighbors in a 5x5 square kernel.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NEIGHBORHOOD_ORGANIZED_SIZE, 5);

   // Exclude small isolated clusters.
   M3dblobControl(SegmentationContext, M_DEFAULT,
                  M_NUMBER_OF_POINTS_MIN, LOCAL_SEGMENTATION_MIN_NB_POINTS);

   // Exclude extremely large clusters which make up the background.
   M3dblobControl(SegmentationContext, M_DEFAULT,
                  M_NUMBER_OF_POINTS_MAX, LOCAL_SEGMENTATION_MAX_NB_POINTS);

   // Set the distance between points to be blobbed together.
   M3dblobControl(SegmentationContext, M_DEFAULT,
                  M_MAX_DISTANCE, LOCAL_SEGMENTATION_DISTANCE_THRESHOLD);

   // Segment the point cloud into several blobs.
   M3dblobSegment(SegmentationContext, MilPointCloud, AllBlobs, M_DEFAULT);

   // Draw all blobs in the 3d display.
   M3dblobControlDraw(Draw3dContext, M_DRAW_BLOBS, M_ACTIVE, M_ENABLE);
   M3dblobControlDraw(Draw3dContext, M_DRAW_BLOBS, M_THICKNESS, 3);
   MIL_INT64 AllBlobsLabel = M3dblobDraw3d(Draw3dContext, MilPointCloud, AllBlobs,
                                           M_ALL, SceneGraList, M_ROOT_NODE, M_DEFAULT);

   MosPrintf(MIL_TEXT("The point cloud is segmented based on the distance between points.\n"));
   MosPrintf(MIL_TEXT("Points belonging to the background plane ")
             MIL_TEXT("or small isolated clusters\n"));
   MosPrintf(MIL_TEXT("are excluded.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Calculate features on the blobs and use them to determine
   // the type of object they represent.
   M3dblobControl(CalculateContext, M_DEFAULT, M_PCA_BOX, M_ENABLE);
   M3dblobControl(CalculateContext, M_DEFAULT, M_LINEARITY, M_ENABLE);
   M3dblobControl(CalculateContext, M_DEFAULT, M_PLANARITY, M_ENABLE);

   M3dblobCalculate(CalculateContext, MilPointCloud, AllBlobs, M_ALL, M_DEFAULT);

   // Connectors are more elongated than other blobs.
   // Use the feature M_LINEARITY, which is a value from 0 (perfect sphere/plane)
   // to 1 (perfect line).
   M3dblobSelect(AllBlobs, Connectors, M_LINEARITY, M_GREATER, 0.5, M_NULL, M_DEFAULT);

   // Washers are flat and circular.
   // Use the feature M_PLANARITY, which is a value from 0 (perfect sphere)
   // to 1 (perfect plane).
   M3dblobSelect(AllBlobs, Washers, M_LINEARITY, M_LESS, 0.2, M_NULL, M_DEFAULT);
   M3dblobSelect(Washers, Washers, M_PLANARITY, M_GREATER, 0.8, M_NULL, M_DEFAULT);

   // Blobs that are neither connectors nor washers are unknown objects.
   // Use M3dblobCombine to subtract already identified blobs from AllBlobs.
   M3dblobCombine(AllBlobs, Connectors, UnknownBlobs, M_SUB, M_DEFAULT);
   M3dblobCombine(UnknownBlobs, Washers, UnknownBlobs, M_SUB, M_DEFAULT);

   // Print the number of blobs in each category.
   MIL_INT NbConnectors = (MIL_INT)M3dblobGetResult(Connectors,   M_DEFAULT, M_NUMBER, M_NULL);
   MIL_INT NbWashers    = (MIL_INT)M3dblobGetResult(Washers,      M_DEFAULT, M_NUMBER, M_NULL);
   MIL_INT NbUnknown    = (MIL_INT)M3dblobGetResult(UnknownBlobs, M_DEFAULT, M_NUMBER, M_NULL);

   MosPrintf(MIL_TEXT("Simple 3D features (planarity, linearity) are calculated on the\n"));
   MosPrintf(MIL_TEXT("blobs and used to identify them.\n\n"));
   
   MosPrintf(MIL_TEXT("The relevant objects (connectors and washers) have their\n"));
   MosPrintf(MIL_TEXT("bounding box displayed.\n"));
   MosPrintf(MIL_TEXT("Connectors (in red):     %i\n"), NbConnectors);
   MosPrintf(MIL_TEXT("Washers (in green) :     %i\n"), NbWashers);
   MosPrintf(MIL_TEXT("Unknown (in yellow):     %i\n"), NbUnknown);

   // Draw the blobs in the 3d display.
   M3dgraRemove(SceneGraList, AllBlobsLabel, M_DEFAULT);

   M3dblobControlDraw(Draw3dContext, M_DRAW_BLOBS, M_COLOR, M_COLOR_YELLOW);
   M3dblobDraw3d(Draw3dContext, MilPointCloud, UnknownBlobs, M_ALL, SceneGraList,
                 M_ROOT_NODE, M_DEFAULT);

   M3dblobControlDraw(Draw3dContext, M_DRAW_PCA_BOX, M_ACTIVE, M_ENABLE);
   M3dblobControlDraw(Draw3dContext, M_DRAW_BLOBS, M_COLOR, M_COLOR_RED);
   M3dblobDraw3d(Draw3dContext, MilPointCloud, Connectors, M_ALL, SceneGraList,
                 M_ROOT_NODE, M_DEFAULT);

   M3dblobControlDraw(Draw3dContext, M_DRAW_BLOBS, M_COLOR, M_COLOR_GREEN);
   M3dblobDraw3d(Draw3dContext, MilPointCloud, Washers, M_ALL, SceneGraList,
                 M_ROOT_NODE, M_DEFAULT);

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   M3dblobFree(UnknownBlobs);
   M3dblobFree(Washers);
   M3dblobFree(Connectors);
   M3dblobFree(AllBlobs);
   M3dblobFree(Draw3dContext);
   M3dblobFree(CalculateContext);
   M3dblobFree(SegmentationContext);

   MbufFree(IllustrationImage);
   MbufFree(MilPointCloud);
   }


//*****************************************************************************
// Second example.
//*****************************************************************************
void PlanarSegmentationExample(MIL_ID SceneDisplay, MIL_ID /*IllustrationDisplay*/)
   {
   MIL_ID MilSystem = MobjInquire(SceneDisplay, M_OWNER_SYSTEM, M_NULL);
   MIL_ID SceneGraList = (MIL_ID)M3ddispInquire(SceneDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   // Restore the point cloud and display it.
   MIL_ID MilPointCloud = MbufImport(TWISTY_PUZZLES_FILE, M_DEFAULT,
                                     M_RESTORE, MilSystem, M_NULL);

   M3dgraRemove(SceneGraList, M_ALL, M_DEFAULT);
   M3dgraControl(SceneGraList, M_DEFAULT_SETTINGS, M_THICKNESS, 1);

   M3ddispSelect(SceneDisplay, MilPointCloud, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(SceneDisplay, M_AUTO, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("Another point cloud containing various twisty puzzles is restored.\n\n")
             MIL_TEXT("Press <Enter> to segment it into separate objects.\n\n"));
   MosGetch();

   // Allocate the segmentation objects.
   MIL_ID SegmentationContext = M3dblobAlloc(MilSystem, M_SEGMENTATION_CONTEXT,
                                             M_DEFAULT, M_NULL);
   MIL_ID SegmentationResult = M3dblobAllocResult(MilSystem, M_SEGMENTATION_RESULT,
                                             M_DEFAULT, M_NULL);

   // Take advantage of the 2d organization.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NEIGHBOR_SEARCH_MODE, M_ORGANIZED);

   // Look for neighbors in a 5x5 square kernel.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NEIGHBORHOOD_ORGANIZED_SIZE, 5);

   // Exclude small isolated clusters.
   M3dblobControl(SegmentationContext, M_DEFAULT,
                  M_NUMBER_OF_POINTS_MIN, PLANAR_SEGMENTATION_MIN_NB_POINTS);

   // Use an automatic local distance threshold.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_MAX_DISTANCE_MODE, M_AUTO);

   // Use an automatic local normal threshold.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NORMAL_DISTANCE_MAX_MODE, M_AUTO);

   // Consider flipped normals to be the same.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NORMAL_DISTANCE_MODE, M_ORIENTATION);

   // First segment the point cloud with only local thresholds.
   M3dblobSegment(SegmentationContext, MilPointCloud, SegmentationResult, M_DEFAULT);

   MIL_INT64 AnnotationLabel = M3dblobDraw3d(M_DEFAULT, MilPointCloud, SegmentationResult,
                                             M_ALL, SceneGraList, M_ROOT_NODE, M_DEFAULT);

   MosPrintf(MIL_TEXT("The point cloud is segmented based on local ")
             MIL_TEXT("thresholds (distance, normals).\n\n"));
   MosPrintf(MIL_TEXT("Local thresholds can separate distinct objects due ")
             MIL_TEXT("to camera occlusions,\n"));
   MosPrintf(MIL_TEXT("but are often not enough to segment a single ")
             MIL_TEXT("object into subparts.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to use global thresholds instead.\n\n"));
   MosGetch();

   // Remove the local normal threshold.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NORMAL_DISTANCE_MAX_MODE, M_USER_DEFINED);

   // Use a global normal threshold instead.
   M3dblobControl(SegmentationContext, M_DEFAULT,
                  M_GLOBAL_NORMAL_DISTANCE_MAX, PLANAR_SEGMENTATION_NORMAL_THRESHOLD);  

   // Segment again with global thresholds.
   M3dblobSegment(SegmentationContext, MilPointCloud, SegmentationResult, M_DEFAULT);

   M3dgraRemove(SceneGraList, AnnotationLabel, M_DEFAULT);
   AnnotationLabel = M3dblobDraw3d(M_DEFAULT, MilPointCloud, SegmentationResult,
                                   M_ALL, SceneGraList, M_ROOT_NODE, M_DEFAULT);

   MosPrintf(MIL_TEXT("The puzzles' sides are now separated.\n\n")
             MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   M3dblobFree(SegmentationContext);
   M3dblobFree(SegmentationResult);

   MbufFree(MilPointCloud);
   }

//*****************************************************************************
// Allocates a 3D display if it is supported.
//*****************************************************************************
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_ID MilDisplay = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"),
                                    M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(MilDisplay == M_NULL)
      {
      MosPrintf(MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to exit.\n"));
      MosGetch();
      }

   return MilDisplay;
   }
