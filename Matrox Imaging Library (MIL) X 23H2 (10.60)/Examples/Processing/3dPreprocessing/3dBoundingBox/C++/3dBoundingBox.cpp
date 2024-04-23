﻿//******************************************************************************
//
// File name: BoundingBox.cpp
//
// Synopsis:  This example demonstrates how to get various bounding boxes
//            of a point cloud. The normalization from the bounding box and
//            the standardization from moments is also shown.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//******************************************************************************
#include <mil.h>

//******************************************************************************
// Functions' declaration.
//******************************************************************************
void PrintHeader();

void WaitForKey();

MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);

void SetGraphicFormat(MIL_ID GraphicsList, MIL_INT64 GraphicsLabel, MIL_INT Color,
                      MIL_INT Thickness, MIL_INT Opacity);

void SemiOrientedBoxAndRotation(MIL_ID MilSystem, MIL_ID MilGraList, MIL_ID MilContainer);
void RobustBoxAndCrop(MIL_ID MilSystem, MIL_ID MilGraList, MIL_ID MilContainer);
void Normalization(MIL_ID MilSystem, MIL_ID MilContainer);
void Standardization(MIL_ID MilSystem, MIL_ID MilContainer);
void SetupDisplayView(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilContainer);
MIL_UNIQUE_BUF_ID GenerateDepthMap(MIL_ID MilSystem, MIL_ID MilContainer);
//******************************************************************************
// Constants.
//******************************************************************************
static MIL_CONST_TEXT_PTR POINT_CLOUD_FILE_NAME = M_IMAGE_PATH MIL_TEXT("3dBoundingBox/BlisterPack.ply");

static const MIL_DOUBLE AXIS_LENGTH = 15;
static const MIL_DOUBLE SAMPLING_RESOLUTION = 20;
static const MIL_INT WINDOW_SIZE_X = 750;

//*******************************************************************************
// Prints the example's description.
//*******************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("3dBoundingBox\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to get various bounding boxes\n")
             MIL_TEXT("of a point cloud. The normalization from the bounding box and\n")
             MIL_TEXT("the standardization from the moments is also shown.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Buffer, 3D Display, 3D Graphics, \n")
             MIL_TEXT("3D Geometry and 3D Image Processing\n\n"));
   }

//*******************************************************************************
// Main function.
//*******************************************************************************
int MosMain(void)
   {
   PrintHeader();
   WaitForKey();

   // Allocate a MIL Application, System, and 3D display.
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   auto Mil3dDisp = Alloc3dDisplayId(MilSystem);

   // Getting the graphic list from the display.
   auto MilGraList = (MIL_ID)M3ddispInquire(Mil3dDisp, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraControl(MilGraList, M_DEFAULT_SETTINGS, M_APPEARANCE, M_SOLID_WITH_WIREFRAME);

   // Load and display the point cloud along with the axis.
   auto MilContainer = MbufRestore(POINT_CLOUD_FILE_NAME, MilSystem, M_UNIQUE_ID);
   M3dgraAdd(MilGraList, M_ROOT_NODE, MilContainer, M_DEFAULT);
   M3dgraAxis(MilGraList, M_ROOT_NODE, M_DEFAULT, AXIS_LENGTH, M_NULL, M_DEFAULT, M_DEFAULT);
   SetupDisplayView(MilSystem, Mil3dDisp, MilContainer);
   M3ddispSelect(Mil3dDisp, M_NULL, M_OPEN, M_DEFAULT);

   // Semi-oriented bounding box example.
   SemiOrientedBoxAndRotation(MilSystem, MilGraList, MilContainer);

   // Robust bounding box example.
   RobustBoxAndCrop(MilSystem, MilGraList, MilContainer);

   // Normalization example.
   Normalization(MilSystem, MilContainer);

   // Standardization example.
   Standardization(MilSystem, MilContainer);

   return 0;
   }

//*******************************************************************************
// Example that shows how to use the semi-oriented box to align the data in the
// X-Y plane.
//*******************************************************************************
void SemiOrientedBoxAndRotation(MIL_ID MilSystem, MIL_ID MilGraList, MIL_ID MilContainer)
   {

   MosPrintf(MIL_TEXT("The axis aligned bounding box of the point cloud\n")
             MIL_TEXT("is displayed in green.\n"));

   // Allocate a 3D image processing result buffer with statistics parameter.
   auto MilStats = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Process the bounding box data.
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilContainer, MilStats, M_DEFAULT);

   // Allocating a geometry box for the bounding box.
   auto MilBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);

   // Getting the bounding box.
   M3dimCopyResult(MilStats, MilBox, M_BOUNDING_BOX, M_DEFAULT);

   // Drawing the geometry box in the graphic list.
   auto MilBoxLabel = M3dgeoDraw3d(M_DEFAULT, MilBox, MilGraList, M_ROOT_NODE, M_DEFAULT);
   SetGraphicFormat(MilGraList, MilBoxLabel, M_COLOR_GREEN, 3, 20);

   WaitForKey();

   MosPrintf(MIL_TEXT("The semi-oriented bounding box, whose orientation in the X-Y\n")
             MIL_TEXT("plane is chosen to minimize the box volume, is displayed in blue.\n"));

   // To hold the data of the semi-oriented bounding box.
   MIL_DOUBLE BoxOrientation = 0;
   MIL_DOUBLE BoxCenter[3] = {0, 0, 0};
   MIL_DOUBLE BoxSize[3] = {0, 0, 0};

   // Process the semi-oriented bounding box data.
   M3dimStat(M_STAT_CONTEXT_SEMI_ORIENTED_BOX, MilContainer, MilStats, M_DEFAULT);

   // Get the box orientation.
   M3dimGetResult(MilStats, M_SEMI_ORIENTED_BOX_ANGLE, &BoxOrientation);

   // Getting the center coordinates of the box from stats result.
   M3dimGetResult(MilStats, M_BOX_CENTER_X, &BoxCenter[0]);
   M3dimGetResult(MilStats, M_BOX_CENTER_Y, &BoxCenter[1]);
   M3dimGetResult(MilStats, M_BOX_CENTER_Z, &BoxCenter[2]);

   // Getting the size of the box from stats result.
   M3dimGetResult(MilStats, M_SIZE_X, &BoxSize[0]);
   M3dimGetResult(MilStats, M_SIZE_Y, &BoxSize[1]);
   M3dimGetResult(MilStats, M_SIZE_Z, &BoxSize[2]);

   // Allocating and creating a geometry box with the semi-oriented bounding box data.
   auto MilSemiOrientedBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoBox(MilSemiOrientedBox, M_CENTER_AND_DIMENSION, BoxCenter[0], BoxCenter[1], BoxCenter[2], BoxSize[0], BoxSize[1], BoxSize[2], M_DEFAULT);

   // Rotating the geometry box to match the semi-oriented box.
   M3dimRotate(MilSemiOrientedBox, MilSemiOrientedBox, M_ROTATION_Z, BoxOrientation, M_DEFAULT, M_DEFAULT, M_DEFAULT, BoxCenter[0], BoxCenter[1], BoxCenter[2], M_DEFAULT);

   // Drawing the geometry box in the graphic list.
   auto MilSemiOrientedBoxLabel = M3dgeoDraw3d(M_DEFAULT, MilSemiOrientedBox, MilGraList, M_ROOT_NODE, M_DEFAULT);
   SetGraphicFormat(MilGraList, MilSemiOrientedBoxLabel, M_COLOR_BLUE, 3, 20);

   WaitForKey();

   MosPrintf(MIL_TEXT("The data can then be rotated so the axis-aligned\n")
             MIL_TEXT("bounding box is now optimal.\n"));

   // Removing the semi-oriented box fromt he graphic list.
   M3dgraRemove(MilGraList, MilSemiOrientedBoxLabel, M_DEFAULT);

   // Rotating the point cloud to align with the axis.
   M3dimRotate(MilContainer, MilContainer, M_ROTATION_Z, 90 - BoxOrientation, M_DEFAULT, M_DEFAULT, M_DEFAULT, BoxCenter[0], BoxCenter[1], BoxCenter[2], M_DEFAULT);

   // Updating the bounding box of the point cloud because of the rotation.
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilContainer, MilStats, M_DEFAULT);
   M3dimCopyResult(MilStats, MilBox, M_BOUNDING_BOX, M_DEFAULT);

   auto MilBetterBoxLabel = M3dgeoDraw3d(M_DEFAULT, MilBox, MilGraList, M_ROOT_NODE, M_DEFAULT);
   SetGraphicFormat(MilGraList, MilBetterBoxLabel, M_COLOR_BRIGHT_GRAY, 3, 20);

   WaitForKey();

   // Removing the boxes from the graphic list.
   M3dgraRemove(MilGraList, MilBoxLabel, M_DEFAULT);
   M3dgraRemove(MilGraList, MilBetterBoxLabel, M_DEFAULT);
   }

//*******************************************************************************
// Example that shows how to use the robust bounding box to remove outliers.
//*******************************************************************************
void RobustBoxAndCrop(MIL_ID MilSystem, MIL_ID MilGraList, MIL_ID MilContainer)
   {

   MosPrintf(MIL_TEXT("The robust bounding box, which excludes some outlier points,\n")
             MIL_TEXT("is displayed in red.\n"));

   // Allocating a 3D image processing result buffer with statistics parameter.
   auto MilStats = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Process the bounding box data.
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilContainer, MilStats, M_DEFAULT);

   // Allocating a geometry box for the bounding box.
   auto MilBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);

   // Getting the bounding box.
   M3dimCopyResult(MilStats, MilBox, M_BOUNDING_BOX, M_DEFAULT);

   // Drawing the geometry box in the graphic list.
   auto MilBoxLabel = M3dgeoDraw3d(M_DEFAULT, MilBox, MilGraList, M_ROOT_NODE, M_DEFAULT);
   SetGraphicFormat(MilGraList, MilBoxLabel, M_COLOR_GREEN, 3, 20);

   // Allocating and creating a context to process the robust bounding box.
   auto CustomContext = M3dimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(CustomContext, M_BOUNDING_BOX_ALGORITHM, M_ROBUST);
   M3dimControl(CustomContext, M_BOUNDING_BOX, M_ENABLE);

   // Process the robust bounding box data.
   M3dimStat(CustomContext, MilContainer, MilStats, M_DEFAULT);

   // Allocating and creating a geometry box with the robust bounding box data.
   auto MilRobustBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dimCopyResult(MilStats, MilRobustBox, M_BOUNDING_BOX, M_DEFAULT);

   // Drawing the robust geometry box in the graphic list.
   auto MilRobustBoxLabel = M3dgeoDraw3d(M_DEFAULT, MilRobustBox, MilGraList, M_ROOT_NODE, M_DEFAULT);
   SetGraphicFormat(MilGraList, MilRobustBoxLabel, M_COLOR_RED, 3, 20);

   // Generating the depth map from the point cloud.
   auto MilBaseDepthMap = GenerateDepthMap(MilSystem, MilContainer);

   WaitForKey();

   // Cropping the point cloud with the robust bounding box.
   M3dimCrop(MilContainer, MilContainer, MilRobustBox, M_NULL, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The data can then be cropped using the robust box to remove outlier points.\n")
             MIL_TEXT("Since the box Z dimension is much smaller, the grayscale\n")
             MIL_TEXT("depth resolution of the depth map projected from the cropped\n")
             MIL_TEXT("data is much better.\n"));

   // Generating the robust depth map.
   auto MilRobustDepthMap = GenerateDepthMap(MilSystem, MilContainer);

   // Display the depth map generated from the complete point cloud.
   auto MilBaseDepthMapDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("Original point cloud"), M_DEFAULT, M_UNIQUE_ID);
   MdispControl(MilBaseDepthMapDisplay, M_TITLE, MIL_TEXT("Original point cloud"));
   MdispControl(MilBaseDepthMapDisplay, M_WINDOW_INITIAL_POSITION_X, WINDOW_SIZE_X);
   MdispSelect(MilBaseDepthMapDisplay, MilBaseDepthMap);

   // Display the depth map generated from the cropped point cloud.
   // The display is positioned next to the complete point cloud depth map.
   auto MilRobustDepthMapDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("Cropped point cloud"), M_DEFAULT, M_UNIQUE_ID);
   MdispControl(MilRobustDepthMapDisplay, M_TITLE, MIL_TEXT("Cropped point cloud"));
   MIL_INT DepthMapDisplaySizeX = 0;
   MdispInquire(MilBaseDepthMapDisplay, M_SIZE_X, &DepthMapDisplaySizeX);
   MdispControl(MilRobustDepthMapDisplay, M_WINDOW_INITIAL_POSITION_X, WINDOW_SIZE_X + DepthMapDisplaySizeX);
   MdispSelect(MilRobustDepthMapDisplay, MilRobustDepthMap);

   WaitForKey();

   // Removing the original bounding box from the graphic list.
   M3dgraRemove(MilGraList, MilBoxLabel, M_DEFAULT);

   // Removing the robust bounding box from the graphic list.
   M3dgraRemove(MilGraList, MilRobustBoxLabel, M_DEFAULT);

   }

//*******************************************************************************
// Example that shows the normalization of the data based on the bounding box.
//*******************************************************************************
void Normalization(MIL_ID MilSystem, MIL_ID MilContainer)
   {

   MosPrintf(MIL_TEXT("The data can be normalized (scaled) so the bounding box fits in\n")
             MIL_TEXT("a unit cube.\n"));

   // Allocating a display for the normalized data.
   auto Mil3dDisp = Alloc3dDisplayId(MilSystem);

   // Getting the graphic list from the new display.
   auto MilDiffGraList = (MIL_ID)M3ddispInquire(Mil3dDisp, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraControl(MilDiffGraList, M_DEFAULT_SETTINGS, M_FONT_SIZE, 0.2);

   // Adding axis label of length 1.
   auto AxisLabel = M3dgraAxis(MilDiffGraList, M_ROOT_NODE, M_DEFAULT, 1.0, MIL_TEXT("1 Unit"), M_FLIP, M_DEFAULT);
   M3dgraControl(MilDiffGraList, AxisLabel, M_THICKNESS, 3);

   // Setting up the new display.
   M3ddispControl(Mil3dDisp, M_WINDOW_INITIAL_POSITION_X, WINDOW_SIZE_X);
   M3ddispControl(Mil3dDisp, M_SIZE_X, WINDOW_SIZE_X);
   M3ddispControl(Mil3dDisp, M_TITLE, MIL_TEXT("Normalized data"));

   // Allocating a 3D image processing result buffer with statistics parameter.
   auto MilStats = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocating and creating a context to normalize the data.
   auto CustomContext = M3dimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(CustomContext, M_BOUNDING_BOX, M_ENABLE);
   M3dimControl(CustomContext, M_NORMALIZATION_MODE, M_NORMALIZE_UNSIGNED);
   M3dimControl(CustomContext, M_NORMALIZATION_SCALE, M_UNIFORM);
   M3dimControl(CustomContext, M_MOMENTS, M_ENABLE);
   M3dimControl(CustomContext, M_CENTROID, M_ENABLE);

   // Allocating a container for the normalized data.
   auto MilNormalizedContainer = MbufAllocContainer(MilSystem, M_DISP + M_PROC, M_DEFAULT, M_UNIQUE_ID);

   // Allocating a transformation matrix to normalize the data.
   MIL_UNIQUE_3DGEO_ID TransformMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   // Process the normalization matrix.
   M3dimStat(CustomContext, MilContainer, MilStats, M_DEFAULT);

   // Get the normalization matrix.
   M3dimCopyResult(MilStats, TransformMatrix, M_NORMALIZATION_MATRIX, M_DEFAULT);

   // Apply the matrix to the point cloud.
   M3dimMatrixTransform(MilContainer, MilNormalizedContainer, TransformMatrix, M_DEFAULT);

   // Adding the normalized point cloud into the new display.
   M3dgraAdd(MilDiffGraList, M_ROOT_NODE, MilNormalizedContainer, M_DEFAULT);

   SetupDisplayView(MilSystem, Mil3dDisp, MilNormalizedContainer);
   M3ddispSelect(Mil3dDisp, M_NULL, M_OPEN, M_DEFAULT);

   WaitForKey();

   M3ddispSelect(Mil3dDisp, M_NULL, M_CLOSE, M_DEFAULT);
   }

//*******************************************************************************
// Example that shows the standardization of the data based on the moments.
//*******************************************************************************
void Standardization(MIL_ID MilSystem, MIL_ID MilContainer)
   {

   MosPrintf(MIL_TEXT("The data can be standardized so the resulting point cloud\n")
             MIL_TEXT("is centered at the origin and scaled to have unit variance\n")
             MIL_TEXT("along each axis.\n"));

   // Allocating a display for the normalized data.
   auto Mil3dDisp = Alloc3dDisplayId(MilSystem);

   // Getting the graphic list from the new display.
   auto MilDiffGraList = (MIL_ID)M3ddispInquire(Mil3dDisp, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraControl(MilDiffGraList, M_DEFAULT_SETTINGS, M_FONT_SIZE, 0.2);

   // Adding axis label of length 1.
   auto AxisLabel = M3dgraAxis(MilDiffGraList, M_ROOT_NODE, M_DEFAULT, 1.0, MIL_TEXT("Length 1"), M_FLIP, M_DEFAULT);
   M3dgraControl(MilDiffGraList, AxisLabel, M_THICKNESS, 3);

   // Setting up the new display.
   M3ddispControl(Mil3dDisp, M_WINDOW_INITIAL_POSITION_X, WINDOW_SIZE_X);
   M3ddispControl(Mil3dDisp, M_SIZE_X, WINDOW_SIZE_X);
   M3ddispControl(Mil3dDisp, M_TITLE, MIL_TEXT("Standardized data"));

   // Allocating a 3D image processing result buffer with statistics parameter.
   auto MilStats = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocating and creating a context to normalize the data.
   auto CustomContext = M3dimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(CustomContext, M_BOUNDING_BOX, M_ENABLE);
   M3dimControl(CustomContext, M_NORMALIZATION_MODE, M_NORMALIZE_UNSIGNED);
   M3dimControl(CustomContext, M_NORMALIZATION_SCALE, M_UNIFORM);
   M3dimControl(CustomContext, M_MOMENTS, M_ENABLE);
   M3dimControl(CustomContext, M_CENTROID, M_ENABLE);

   // Allocating a container for the normalized data.
   auto MilStandardizedContainer = MbufAllocContainer(MilSystem, M_DISP + M_PROC, M_DEFAULT, M_UNIQUE_ID);

   // Allocating a transformation matrix to normalize the data.
   MIL_UNIQUE_3DGEO_ID TransformMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   // Process the normalization matrix.
   M3dimStat(CustomContext, MilContainer, MilStats, M_DEFAULT);

   // Get the standardized matrix.
   M3dimCopyResult(MilStats, TransformMatrix, M_STANDARDIZATION_MATRIX, M_DEFAULT);

   // Apply the matrix to the point cloud.
   M3dimMatrixTransform(MilContainer, MilStandardizedContainer, TransformMatrix, M_DEFAULT);

   // Adding the standardized point cloud into the new display.
   M3dgraAdd(MilDiffGraList, M_ROOT_NODE, MilStandardizedContainer, M_DEFAULT);

   SetupDisplayView(MilSystem, Mil3dDisp, MilStandardizedContainer);
   M3ddispSelect(Mil3dDisp, M_NULL, M_OPEN, M_DEFAULT);

   WaitForKey();

   M3ddispSelect(Mil3dDisp, M_NULL, M_CLOSE, M_DEFAULT);
   }

//*******************************************************************************
// Setup the view of the display.
//*******************************************************************************
void SetupDisplayView(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilContainer)
   {
   auto MilBoundingBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilContainer, MilBoundingBox, M_DEFAULT);
   M3ddispSetView(MilDisplay, M_VIEW_ORIENTATION, 2, -1, 1, M_DEFAULT);
   M3ddispSetView(MilDisplay, M_UP_VECTOR, 0.0, 0.0, -1.0, M_DEFAULT);
   M3ddispSetView(MilDisplay, M_VIEW_BOX, MilBoundingBox, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   }

//*******************************************************************************
// Generates the depth map using a fixed pixel size.
//*******************************************************************************
MIL_UNIQUE_BUF_ID GenerateDepthMap(MIL_ID MilSystem, MIL_ID MilContainer)
   {
   const MIL_DOUBLE PixelSize = 0.3;
   // Calculate the size required for the depth map.
   auto MapSizeContext = M3dimAlloc(MilSystem, M_CALCULATE_MAP_SIZE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MapSizeContext, M_PIXEL_SIZE_X, PixelSize);
   M3dimControl(MapSizeContext, M_PIXEL_SIZE_Y, PixelSize);
   M3dimControl(MapSizeContext, M_PIXEL_ASPECT_RATIO, M_NULL);
   MIL_INT DepthMapSizeX, DepthMapSizeY;
   M3dimCalculateMapSize(MapSizeContext, MilContainer, M_NULL, M_DEFAULT, &DepthMapSizeX, &DepthMapSizeY);

   // Allocate and calibrate the depth map.
   auto MilDepthmap = MbufAlloc2d(MilSystem, DepthMapSizeX, DepthMapSizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   M3dimCalibrateDepthMap(MilContainer, MilDepthmap, M_NULL, M_NULL, M_DEFAULT, M_NEGATIVE, M_DEFAULT);

   // Project the point cloud on the depth map.
   M3dimProject(MilContainer, MilDepthmap, M_NULL, M_POINT_BASED, M_MAX_Z, M_DEFAULT, M_DEFAULT);

   return MilDepthmap;
   }

//*******************************************************************************
// Pauses the execution until a key is pressed.
//*******************************************************************************
void WaitForKey()
   {
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*******************************************************************************
// Allocates a 3D display if possible.  
//*******************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto Mil3dDisp = M3ddispAlloc(MilSystem, M_DEFAULT,
                                 MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!Mil3dDisp)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n"));
      WaitForKey();
      exit(EXIT_FAILURE);
      }
   M3ddispControl(Mil3dDisp, M_SIZE_X, WINDOW_SIZE_X);
   return Mil3dDisp;
   }

//*******************************************************************************
// Sets color, thickness, and opacity of the specified model.
//*******************************************************************************
void SetGraphicFormat(MIL_ID GraphicsList, MIL_INT64 ModelLabel, MIL_INT Color,
                      MIL_INT Thickness,
                      MIL_INT Opacity)
   {
   M3dgraControl(GraphicsList, ModelLabel, M_COLOR, Color);
   M3dgraControl(GraphicsList, ModelLabel, M_THICKNESS, Thickness);
   M3dgraControl(GraphicsList, ModelLabel, M_OPACITY, Opacity);
   }

