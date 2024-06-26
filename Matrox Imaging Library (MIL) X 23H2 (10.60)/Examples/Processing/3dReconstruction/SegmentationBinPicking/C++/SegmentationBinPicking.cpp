﻿//***************************************************************************************/
//
// File name: SegmentationBinPicking.cpp
//
// Synopsis:  This example performs 3d segmentation to identify and pick up objects in a bin.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <math.h>
#include "RobotArmAnimation.h"

// Source file specification.
MIL_INT NB_PT_CLDS = 3;
static const MIL_TEXT_CHAR* const PT_CLD_FILES[] =
    {M_IMAGE_PATH MIL_TEXT("Simple3dBinPicking/BinCloudScene_0.ply"),
     M_IMAGE_PATH MIL_TEXT("Simple3dBinPicking/BinCloudScene_1.ply"), 
     M_IMAGE_PATH MIL_TEXT("Simple3dBinPicking/BinCloudScene_2.ply")};

// Identification thresholds (all in mm).
static const MIL_DOUBLE PLUG_SIZE_X_MIN = 30.0;
static const MIL_DOUBLE PLUG_SIZE_X_MAX = 50.0;
static const MIL_DOUBLE PLUG_SIZE_Y_MIN = 30.0;
static const MIL_DOUBLE PLUG_SIZE_Y_MAX = 50.0;
static const MIL_DOUBLE PLUG_SIZE_Z_MIN = 5.0;
static const MIL_DOUBLE PLUG_SIZE_Z_MAX = 15.0;

// Robot arm animation (all in mm).
static const MIL_INT64 ARM_SECTION_COLOR = M_COLOR_YELLOW;
static const MIL_INT64 ARM_JOINT_COLOR = M_COLOR_GRAY;
static const MIL_INT64 GRID_LINE_COLOR = M_COLOR_BLACK;
static const MIL_INT64 GRID_BACKGROUND_COLOR = M_COLOR_LIGHT_GRAY;

static const MIL_DOUBLE ARM_RADIUS = 10.0;
static const MIL_DOUBLE ARM_LENGTH_A = 90.0;
static const MIL_DOUBLE ARM_LENGTH_B = 80.0;
static const MIL_DOUBLE ARM_LENGTH_C = 40.0;

static const MIL_DOUBLE ARM_BASE_POS_X = 100;
static const MIL_DOUBLE ARM_BASE_POS_Y = 0;
static const MIL_DOUBLE ARM_BASE_POS_Z = -10;

static const MIL_DOUBLE ARM_REST_POS_X = 100;
static const MIL_DOUBLE ARM_REST_POS_Y = 80;
static const MIL_DOUBLE ARM_REST_POS_Z = -10;

static const MIL_DOUBLE GRID_POS_Z = 10;              // Height of the grid (in mm).
static const MIL_DOUBLE GRID_SIZE = 30;               // Size of each grid tile (in mm).
static const MIL_DOUBLE GRID_TILES = 10;              // Number of tiles in the grid.

static const MIL_DOUBLE ARM_SAFETY_HEIGHT = 30;       // Height above the grabbed object to prevent collisions (in mm).
static const MIL_DOUBLE ARM_ANIMATION_SPEED = 200;    // Speed of the grabber (in mm/s).

// Function declarations.
void                 CheckForRequiredMILFile(const MIL_STRING& FileName);
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);
void                 FlipMatrixDownwards(MIL_ID Matrix);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("SegmentationBinPicking\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example performs 3d segmentation to identify\n")
             MIL_TEXT("and pick up objects in a bin. \n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Blob Analysis, 3D Image Processing, 3D Metrology,\n")
             MIL_TEXT("3d Geometry, 3D Display, 3D Graphics, and Buffer.\n\n"));
   }


//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Check for required example files.
   for(const auto& File : PT_CLD_FILES)
      CheckForRequiredMILFile(File);

   // Allocate the 3d display.
   auto Display = Alloc3dDisplayId(MilSystem);
   MIL_ID GraList = (MIL_ID)M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3ddispSetView(Display, M_VIEW_ORIENTATION, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Restore and display the first point cloud.
   // The point cloud's up direction is -Z, so color it with a flipped height LUT.
   auto Container = MbufImport(PT_CLD_FILES[0], M_DEFAULT, M_RESTORE, MilSystem, M_UNIQUE_ID);
   M3dgraCopy(M_COLORMAP_TURBO + M_FLIP, M_DEFAULT, GraList, M_DEFAULT_SETTINGS, M_COLOR_LUT, M_DEFAULT);
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_COLOR_COMPONENT_BAND, 2);
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_COLOR_USE_LUT, M_TRUE);
   M3ddispSelect(Display, Container, M_DEFAULT, M_DEFAULT);

   // Create the grid graphics.
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_COLOR, GRID_LINE_COLOR);
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_FILL_COLOR, GRID_BACKGROUND_COLOR);
   auto GridMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(GridMatrix, M_TRANSLATION, 0, 0, GRID_POS_Z, M_DEFAULT, M_DEFAULT);
   M3dgraGrid(GraList, M_ROOT_NODE, M_TILES_AND_SPACING, GridMatrix, GRID_TILES, GRID_TILES, GRID_SIZE, GRID_SIZE, M_DEFAULT);
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_FILL_COLOR, M_SAME_AS_COLOR);

   // Create the robot arm graphics.
   MIL_INT64 AllBlobsNode = M_INVALID;                // Node used to hold all blob annotations.
   MIL_INT64 PreviousSelectedPlugNode = M_INVALID;    // Node representing the last plug that was picked.
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_COLOR, ARM_JOINT_COLOR);
   M3dgraBox(GraList, M_ROOT_NODE, M_BOTH_CORNERS,
             ARM_BASE_POS_X - ARM_RADIUS * 2, ARM_BASE_POS_Y - ARM_RADIUS * 2, GRID_POS_Z,
             ARM_BASE_POS_X + ARM_RADIUS * 2, ARM_BASE_POS_Y + ARM_RADIUS * 2, ARM_BASE_POS_Z,
             M_DEFAULT, M_DEFAULT);
   CRobotArmAnimation RobotArm(Display,
                               ARM_BASE_POS_X,
                               ARM_BASE_POS_Y,
                               ARM_BASE_POS_Z,
                               ARM_RADIUS,
                               ARM_LENGTH_A,
                               ARM_LENGTH_B,
                               ARM_LENGTH_C,
                               ARM_ANIMATION_SPEED,
                               ARM_SECTION_COLOR,
                               ARM_JOINT_COLOR,
                               EOrientation::eZDown);

   // Move the robot arm to its rest position. This is where the plugs will be dropped.
   auto RestMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(RestMatrix, M_SCALE, 1, -1, -1, M_DEFAULT, M_DEFAULT);
   M3dgeoMatrixSetTransform(RestMatrix, M_TRANSLATION, ARM_REST_POS_X, ARM_REST_POS_Y, ARM_REST_POS_Z, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
   RobotArm.MoveInstant(RestMatrix);

   // Calculate the median nearest neighbor distance for M3dimNormals. M_INFINITE would work too, but giving an explicit distance is faster.
   auto StatContext = M3dimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto StatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(StatContext, M_DISTANCE_TO_NEAREST_NEIGHBOR, M_ENABLE);
   M3dimControl(StatContext, M_NUMBER_OF_POINTS, M_ENABLE);
   M3dimControl(StatContext, M_CALCULATE_MEDIAN, M_ENABLE);
   M3dimControl(StatContext, M_NUMBER_OF_SAMPLES, 999);
   M3dimStat(StatContext, Container, StatResult, M_DEFAULT);
   MIL_DOUBLE MedianDistanceToNearestNeighbor = M3dimGetResult(StatResult, M_DISTANCE_TO_NEAREST_NEIGHBOR_MEDIAN, M_NULL);
   MIL_INT TotalNbPoints = (MIL_INT)M3dimGetResult(StatResult, M_NUMBER_OF_POINTS_VALID, M_NULL);

   // Allocate the segmentation objects.
   auto NormalsContext = M3dimAlloc(MilSystem, M_NORMALS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto SegmentationContext = M3dblobAlloc(MilSystem, M_SEGMENTATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto CalculateContext = M3dblobAlloc(MilSystem, M_CALCULATE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto DrawContext = M3dblobAlloc(MilSystem, M_DRAW_3D_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   auto AllBlobs = M3dblobAllocResult(MilSystem, M_SEGMENTATION_RESULT, M_DEFAULT, M_UNIQUE_ID);         // Result which holds all blobs that were found.
   auto UnknownBlobs = M3dblobAllocResult(MilSystem, M_SEGMENTATION_RESULT, M_DEFAULT, M_UNIQUE_ID);     // Result which holds blobs that are not plugs.
   auto PlugBlobs = M3dblobAllocResult(MilSystem, M_SEGMENTATION_RESULT, M_DEFAULT, M_UNIQUE_ID);        // Result which holds only plugs.

   // Set up the segmentation objects.
   M3dimControl(NormalsContext, M_MAXIMUM_NUMBER_NEIGHBORS, 9);                                          // Use a small kernel because it is faster.
   M3dimControl(NormalsContext, M_NEIGHBORHOOD_DISTANCE, MedianDistanceToNearestNeighbor * 2);           // Use a maximum distance because it is faster.

   M3dblobControl(SegmentationContext, M_DEFAULT, M_MAX_DISTANCE_MODE, M_AUTO);                          // Automatic distance threshold.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NORMAL_DISTANCE_MAX_MODE, M_AUTO);                   // Automatic normal threshold.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NORMAL_DISTANCE_MODE, M_ORIENTATION);                // M3dimNormals can flip normals, so ignore the orientation.
   M3dblobControl(SegmentationContext, M_DEFAULT, M_NUMBER_OF_POINTS_MIN, TotalNbPoints * 0.02);         // Reject blobs that make up less than 2% of the points.

   M3dblobControl(CalculateContext, M_DEFAULT, M_PCA_BOX, M_ENABLE);
   M3dblobControl(CalculateContext, M_DEFAULT, M_CENTROID, M_ENABLE);

   M3dblobControlDraw(DrawContext, M_DRAW_PCA_BOX, M_ACTIVE, M_ENABLE);
   M3dblobControlDraw(DrawContext, M_DRAW_PCA_BOX, M_COLOR, M_COLOR_YELLOW);

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   for(MIL_INT i = 0; ; i++)
      {
      // Remove previous annotations.
      if(AllBlobsNode != M_INVALID)
         M3dgraRemove(GraList, AllBlobsNode, M_DEFAULT);

      // Load a snapshot of the bin if we have one. If not, reuse the previous point cloud.
      if(i < NB_PT_CLDS)
         MbufImport(PT_CLD_FILES[i], M_DEFAULT, M_LOAD, MilSystem, &Container);

      // Calculate the normals.
      M3dimNormals(NormalsContext, Container, Container, M_DEFAULT);

      // Perform 3d segmentation.
      M3dblobSegment(SegmentationContext, Container, AllBlobs, M_DEFAULT);

      // Calculate features on the blobs so we can identify the plugs.
      M3dblobCalculate(CalculateContext, Container, AllBlobs, M_ALL, M_DEFAULT);

      // Select the plugs.
      M3dblobSelect(AllBlobs, PlugBlobs, M_PCA_BOX + M_SIZE_X, M_IN_RANGE, PLUG_SIZE_X_MIN, PLUG_SIZE_X_MAX, M_DEFAULT);
      M3dblobSelect(PlugBlobs, PlugBlobs, M_PCA_BOX + M_SIZE_Y, M_IN_RANGE, PLUG_SIZE_Y_MIN, PLUG_SIZE_Y_MAX, M_DEFAULT);
      M3dblobSelect(PlugBlobs, PlugBlobs, M_PCA_BOX + M_SIZE_Z, M_IN_RANGE, PLUG_SIZE_Z_MIN, PLUG_SIZE_Z_MAX, M_DEFAULT);
      M3dblobCombine(AllBlobs, PlugBlobs, UnknownBlobs, M_SUB, M_DEFAULT);

      // Draw the unknown blobs.
      AllBlobsNode = M3dgraNode(GraList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);
      M3dblobDraw3d(M_DEFAULT, Container, UnknownBlobs, M_ALL, GraList, AllBlobsNode, M_DEFAULT);

      if(i == 0)
         MosPrintf(MIL_TEXT("Large blobs are identified in the point cloud.\n"));

      if(M3dblobGetResult(PlugBlobs, M_DEFAULT, M_NUMBER, M_NULL) > 0)
         {
         // Sort the plugs by height. The first plug (smallest Z) will be selected by the robot.
         M3dblobSort(PlugBlobs, PlugBlobs, M_CENTROID_Z, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

         // Get the selected plug's position with the PCA matrix. Potentially flip it so the robot grabs the plug from above and not below.
         auto PCAMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
         M3dblobCopyResult(PlugBlobs, M_BLOB_INDEX(0), PCAMatrix, M_PCA_MATRIX, M_DEFAULT);
         FlipMatrixDownwards(PCAMatrix);

         // Draw a yellow bounding box around the plugs.
         M3dblobControlDraw(DrawContext, M_DRAW_PCA_BOX, M_THICKNESS, 1);
         MIL_INT64 AllPlugsNode = M3dblobDraw3d(DrawContext, Container, PlugBlobs, M_ALL, GraList, AllBlobsNode, M_DEFAULT);

         // Identify the node corresponding to the selected plug. The nodes are drawn in the same order as the blobs, so this is the first node.
         // This can be used to change blob annotations after they were drawn. In this case, the selected plug's graphics are thickened.
         std::vector<MIL_INT64> PlugNodes;
         M3dgraInquire(GraList, AllPlugsNode, M_CHILDREN, PlugNodes);
         MIL_INT64 SelectedPlugNode = PlugNodes[0];

         // Draw the picking position and thicken the selected plug's graphics.
         M3dgraAxis(GraList, SelectedPlugNode, PCAMatrix, ARM_RADIUS * 2, M_NULL, M_DEFAULT, M_DEFAULT);
         M3dgraControl(GraList, SelectedPlugNode, M_THICKNESS + M_RECURSIVE, 3);

         if(i == 0)
            {
            MosPrintf(MIL_TEXT("The shape and size of the blobs is used to identify the plugs.\n"));
            MosPrintf(MIL_TEXT("Potential picks are highlighted in yellow and \n"));
            MosPrintf(MIL_TEXT("the next picking position is shown in blue.\n\n"));
            MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
            MosGetch();
            MosPrintf(MIL_TEXT("The highest plug is picked and moved to the side.\n\n"));
            }

         // Remove the previous picked plug and move the robot arm to the new plug.
         if(PreviousSelectedPlugNode != M_INVALID)
            M3dgraRemove(GraList, PreviousSelectedPlugNode, M_DEFAULT);
         RobotArm.Move(PCAMatrix, ARM_SAFETY_HEIGHT);

         // Remove points that are part of the plug.
         auto PCABox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
         M3dblobCopyResult(PlugBlobs, M_BLOB_INDEX(0), PCABox, M_PCA_BOX, M_DEFAULT);
         M3dimCrop(Container, Container, PCABox, M_NULL, M_SAME, M_INVERSE);

         // Make the highest plug follow the arm around by making it a child in the graphic hierarchy.
         M3ddispControl(Display, M_UPDATE, M_DISABLE);
         PreviousSelectedPlugNode = M3dgraCopy(GraList, SelectedPlugNode, GraList, RobotArm.m_SectionC, M_GRAPHIC + M_RECURSIVE, M_DEFAULT);
         M3dgraCopy(M_IDENTITY_MATRIX, M_DEFAULT, GraList, PreviousSelectedPlugNode, M_TRANSFORMATION_MATRIX + M_RELATIVE_TO_ROOT, M_DEFAULT);
         M3dgraRemove(GraList, SelectedPlugNode, M_DEFAULT);
         M3ddispControl(Display, M_UPDATE, M_ENABLE);

         // Move the robot arm back to its normal position.
         RobotArm.Move(RestMatrix, ARM_SAFETY_HEIGHT);


         if(i == NB_PT_CLDS - 1)
            MosPrintf(MIL_TEXT("It is possible to reuse the same point cloud and pick all the visible plugs.\n\n"));

         if(i < NB_PT_CLDS - 1)
            MosPrintf(MIL_TEXT("Press <Enter> to acquire a new point cloud and pick a new plug.\n\n"));
         else
            MosPrintf(MIL_TEXT("Press <Enter> to pick a new plug.\n\n"));

         MosGetch();
         }
      else
         {
         // No plugs were found.
         MosPrintf(MIL_TEXT("There are no remaining visible plugs in the point cloud.\n\n"));
         break;
         }
      }

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   return 0;
   }

//****************************************************************************
// Potentially rotate the matrix 180deg in-place so Z always points downwards.
//****************************************************************************
void FlipMatrixDownwards(MIL_ID Matrix)
   {
   MIL_DOUBLE MatrixValues[16];
   M3dgeoMatrixGet(Matrix, M_DEFAULT, MatrixValues);
   if(MatrixValues[10] > 0)
      {
      MIL_ID MilSystem = MobjInquire(Matrix, M_OWNER_SYSTEM, M_NULL);
      auto FlipMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
      M3dgeoMatrixSetTransform(FlipMatrix, M_SCALE, 1, -1, -1, M_DEFAULT, M_DEFAULT);
      M3dgeoMatrixSetTransform(Matrix, M_COMPOSE_TWO_MATRICES, Matrix, FlipMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      }
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
                MIL_TEXT("Press any key to exit.\n"));
      MosGetch();
      exit(0);
      }
   return MilDisplay3D;
   }
