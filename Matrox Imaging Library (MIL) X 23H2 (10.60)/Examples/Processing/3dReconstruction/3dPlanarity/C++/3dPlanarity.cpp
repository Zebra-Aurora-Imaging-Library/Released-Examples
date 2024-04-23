//***************************************************************************************
// 
// File name: 3dPlanarity.cpp  
//
// Synopsis: Demonstrates object planarity measurements of a
//           scanned 3D mechanical part.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************

#include <mil.h>
#include <math.h>
#include <vector>
using std::vector;

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("3dPlanarity\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to perform planarity measurements\n")
             MIL_TEXT("on a 3D point cloud of a mechanical part."));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: 3D Metrology, Buffer, Calibration,\n")
             MIL_TEXT("Display, Graphics, Geometric Model Finder, 3D Geometry\n")
             MIL_TEXT("3D Image Processing, 3D Display and 3D Graphics.\n\n"));
   }

//****************************************************************************
// Constants.
//****************************************************************************
static const MIL_DOUBLE PLANE_FIT_CENTER_X = 47.88;   // in mm
static const MIL_DOUBLE PLANE_FIT_CENTER_Y = 39.29;   // in mm
static const MIL_DOUBLE PLANE_FIT_RADIUS   = 23.00;   // in mm

static const MIL_INT DISPLAY_SIZE_X   = 700;
static const MIL_INT DISPLAY_SIZE_Y   = 300;
static const MIL_INT DISPLAY_Y_MARGIN =  35;

//****************************************************************************
// File names.
//****************************************************************************
#define DATA_EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("MechanicalPartScan/") MIL_TEXT(x))
static MIL_CONST_TEXT_PTR METAL_PART_CLOUD_CONTAINER = DATA_EX_PATH("MechanicalPart.ply");
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("3dPlanarity/") MIL_TEXT(x))
static MIL_CONST_TEXT_PTR MEASURES_ILLUSTRATIONS[5] =
   {
   EX_PATH("DepthVerticalIllustration.mim"),
   EX_PATH("DepthPlaneVerticalIllustration.mim"),
   EX_PATH("DepthPlaneNormalIllustration.mim"),
   EX_PATH("PointPlaneWorldCylinder.mim"),
   EX_PATH("PointPlaneRaster.mim")
   };

struct SPlanarityMeasure
   {
   SPlanarityMeasure(MIL_CONST_TEXT_PTR Name) : MeasureName(Name){};
   MIL_CONST_TEXT_PTR MeasureName;
   MIL_DOUBLE Planarity;
   };

//****************************************************************************
// Function prototypes.
//****************************************************************************
void AnalyzePlanarity(MIL_ID MilSystem, MIL_ID PointCloudContainer);

void GenerateDepthMap(MIL_ID MilSystem, 
                      MIL_ID PointCloudContainer,
                      MIL_DOUBLE PixelSize,
                      MIL_ID* pOutDepthmap);

bool FixturePart(MIL_ID MilSystem,
                 MIL_ID MilDepthMap,
                 MIL_ID MilDepthMapGraphicList,
                 MIL_ID MilMatrix);

void PrintResultTable(const vector<SPlanarityMeasure>& rPlanarityMeasures);

bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName);

MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   // Allocate the MIL application.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);

   // Check for required example files.
   if (!CheckForRequiredMILFile(METAL_PART_CLOUD_CONTAINER))
      {
      MappFree(MilApplication);
      return -1;
      }

   // Allocate a host system.
   MIL_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   // Restore a 3D point cloud of the object.
   MIL_ID PointCloudContainer = M_NULL;
   MbufRestore(METAL_PART_CLOUD_CONTAINER, MilSystem, &PointCloudContainer);
   
   // Analyze.
   AnalyzePlanarity(MilSystem, PointCloudContainer);

   // Free the MIL 3D point cloud.
   MbufFree(PointCloudContainer);

   // Free the MIL system and application.
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//*******************************************************************************
// AnalyzePlanarity. Planarity analysis of the the scanned object.
//*******************************************************************************
void AnalyzePlanarity(MIL_ID MilSystem, MIL_ID MilPointCloudContainer)
   {
   // Allocates the displays and graphic lists.
   MIL_ID MilIllustrationDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"),
                                              M_WINDOWED, M_NULL);

   MIL_ID MilDisplay3D = Alloc3dDisplayId(MilSystem);
   MIL_ID MilGraphicList = M_NULL;
   if(MilDisplay3D)
      {
      M3ddispInquire(MilDisplay3D, M_3D_GRAPHIC_LIST_ID, &MilGraphicList);

      // Display the 3D point cloud.
      M3ddispSetView(MilDisplay3D, M_AUTO, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      M3ddispControl(MilDisplay3D, M_SIZE_X, DISPLAY_SIZE_X);
      M3ddispControl(MilDisplay3D, M_SIZE_Y, DISPLAY_SIZE_Y);
      M3ddispControl(MilDisplay3D, M_UPDATE, M_DISABLE);
      MIL_INT64 MilContainerGraphics = M3ddispSelect(MilDisplay3D, MilPointCloudContainer, M_SELECT, M_DEFAULT);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_USE_LUT, M_TRUE);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT_BAND, 2);
      M3dgraCopy(M_COLORMAP_TURBO + M_FLIP, M_DEFAULT, MilGraphicList, MilContainerGraphics, M_COLOR_LUT, M_DEFAULT);
      M3ddispSetView(MilDisplay3D, M_ZOOM, 1.5, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      M3ddispSetView(MilDisplay3D, M_TRANSLATE, 60.0, 0.0, 95.0, M_DEFAULT);
      M3ddispControl(MilDisplay3D, M_UPDATE, M_ENABLE);

      M3dgraAxis(MilGraphicList, M_ROOT_NODE, M_DEFAULT, 100, MIL_TEXT(""), M_DEFAULT, M_DEFAULT);

      MosPrintf(MIL_TEXT("A 3D point cloud is restored from a PLY file and displayed with pseudo colors.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }

   // Generate the top view calibrated depth map of the mechanical part.
   MIL_ID MilDepthMap = M_NULL;
   const MIL_DOUBLE PixelSize = 0.3;
   GenerateDepthMap(MilSystem, MilPointCloudContainer, PixelSize, &MilDepthMap);

   MIL_ID MilDepthMapDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MIL_ID MilDepthMapGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MdispControl(MilDepthMapDisplay, M_WINDOW_INITIAL_POSITION_X, DISPLAY_SIZE_X);
   MdispControl(MilDepthMapDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilDepthMapGraphicList);
   MdispZoom(MilDepthMapDisplay, 0.45, 0.45);   
   MdispSelect(MilDepthMapDisplay, MilDepthMap);
   
   MosPrintf(MIL_TEXT("A top view calibrated depth map of the mechanical part was generated.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Locate the part and move the relative coordinate system accordingly.
   MIL_ID MilMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_NULL);
   bool   PartFound = FixturePart(MilSystem, MilDepthMap, MilDepthMapGraphicList, MilMatrix);

   if (PartFound)
      {
      // Fixture the point cloud by applying the matrix transformation.
      M3dimMatrixTransform(MilPointCloudContainer, MilPointCloudContainer, MilMatrix, M_DEFAULT);

      MosPrintf(MIL_TEXT("The mechanical part was located and fixtured using Model Finder in the\ndepth map.\n\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Create a region where we want to measure the planarity.
      MIL_ID MilCylinder = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);
      M3dgeoCylinder(MilCylinder, M_POINT_AND_VECTOR, PLANE_FIT_CENTER_X, PLANE_FIT_CENTER_Y, 0, 0, 0, 1, PLANE_FIT_RADIUS, M_INFINITE,M_DEFAULT);
      MIL_ID MilPtCldRegion = MbufAllocContainer(MilSystem, M_PROC, M_DEFAULT, M_NULL);
      M3dimCrop(MilPointCloudContainer, MilPtCldRegion, MilCylinder, M_NULL, M_UNORGANIZED, M_DEFAULT);

      // Show the region where we want to measure the planarity.
      MIL_DOUBLE MinZ, MaxZ;
      MIL_UNIQUE_3DIM_ID MilResultStatId = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
      M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilPtCldRegion, MilResultStatId,  M_DEFAULT);
      M3dimGetResult(MilResultStatId, M_MAX_Z, &MaxZ);
      M3dimGetResult(MilResultStatId, M_MIN_Z, &MinZ);

      if(MilDisplay3D)
         {
         MIL_INT64 MilGraCylinder =
            M3dgraCylinder(MilGraphicList,
                           M_ROOT_NODE, M_TWO_POINTS,
                           PLANE_FIT_CENTER_X, PLANE_FIT_CENTER_Y, MinZ,
                           PLANE_FIT_CENTER_X, PLANE_FIT_CENTER_Y, MaxZ,
                           PLANE_FIT_RADIUS,
                           M_DEFAULT, M_DEFAULT);
         M3dgraControl(MilGraphicList, MilGraCylinder, M_OPACITY, 70);
         M3dgraControl(MilGraphicList, MilGraCylinder, M_COLOR, M_COLOR_GREEN);
         }

      MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      MgraArcFill(M_DEFAULT, MilDepthMapGraphicList, PLANE_FIT_CENTER_X, PLANE_FIT_CENTER_Y,
                  PLANE_FIT_RADIUS, PLANE_FIT_RADIUS, 0, 360);
      
      MosPrintf(MIL_TEXT("The planarity of the depth data in the green region will be evaluated.\n\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      
      // Measure the vertical planarity. Get the depth map minimum and maximum Z value
      // in the mask.
      SPlanarityMeasure VerticalDepthMeasure(MIL_TEXT("Horizonal plane"));
      VerticalDepthMeasure.Planarity = MaxZ - MinZ;

      // Show the vertical planarity measure.
      MIL_ID MilIllustrationImage = MbufRestore(MEASURES_ILLUSTRATIONS[0], MilSystem, M_NULL);
      MdispControl(MilIllustrationDisplay, M_WINDOW_INITIAL_POSITION_Y, DISPLAY_SIZE_Y + DISPLAY_Y_MARGIN);
      MdispSelect(MilIllustrationDisplay, MilIllustrationImage);
      MosPrintf(MIL_TEXT("First, the planarity of the surface along the Z axis is measured.\n")
                MIL_TEXT("Since the analyzed plane is not coplanar to the XY plane, the planarity\n")
                MIL_TEXT("measure mostly reflects the plane's tilt.\n\n"));

      // Show the plane in the 3D display.
      MIL_INT64 GraPlane = M_NULL;
      if(MilDisplay3D)
         {
         GraPlane = M3dgraPlane(MilGraphicList,
                        M_ROOT_NODE,
                        M_POINT_AND_NORMAL,
                        0.0, 0.0, -(MaxZ + MinZ) / 2.0,
                        0.0, 0.0, 1.0,
                        M_DEFAULT, M_DEFAULT, M_DEFAULT,
                        5,
                        M_DEFAULT);
         }

      vector<SPlanarityMeasure> AllMeasures(1, VerticalDepthMeasure);
      PrintResultTable(AllMeasures);
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      if(MilDisplay3D)
         M3dgraRemove(MilGraphicList, GraPlane, M_DEFAULT);

      // Fit a plane on the data.
      MIL_ID MilFitResult = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_NULL);
      M3dmetFit(M_DEFAULT, MilPtCldRegion, M_PLANE, MilFitResult, M_INFINITE, M_DEFAULT);
      MIL_ID MilPlane = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);
      M3dmetCopyResult(MilFitResult, MilPlane, M_FITTED_GEOMETRY, M_DEFAULT);

      if(MilDisplay3D)
         {
         // Show the fitted plane in the 3D display.
         GraPlane = M3dmetDraw3d(M_DEFAULT, MilFitResult, MilGraphicList, M_NULL, M_DEFAULT);
         M3dgraControl(MilGraphicList, GraPlane, M_OPACITY+M_RECURSIVE, 40);
         M3dgraControl(MilGraphicList, GraPlane, M_COLOR + M_RECURSIVE, M_COLOR_BLUE);
         }

      // Measure the planarity with regards to the fitted plane.
      MIL_DOUBLE PositiveDeviation, NegativeDeviation;
      MIL_ID MilStatContext = M3dmetAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_NULL);
      MIL_ID MilStatResult = M3dmetAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_NULL);
      M3dmetControl(MilStatContext, M_STAT_MAX, M_ENABLE);
      M3dmetControl(MilStatContext, M_STAT_MIN, M_ENABLE);

      SPlanarityMeasure PlaneDepthNormalMeasure(MIL_TEXT("Fitted plane (normal)"));
      M3dmetStat(MilStatContext, MilPtCldRegion, MilPlane, MilStatResult, M_SIGNED_DISTANCE_TO_SURFACE, M_ALL, M_NULL, M_NULL, M_DEFAULT);
      M3dmetGetResult(MilStatResult, M_STAT_MAX, &PositiveDeviation);
      M3dmetGetResult(MilStatResult, M_STAT_MIN, &NegativeDeviation);
      PlaneDepthNormalMeasure.Planarity = PositiveDeviation - NegativeDeviation;

      SPlanarityMeasure PlaneDepthVerticalMeasure(MIL_TEXT("Fitted plane (vertical)"));
      M3dmetStat(MilStatContext, MilPtCldRegion, MilPlane, MilStatResult, M_SIGNED_DISTANCE_Z_TO_SURFACE, M_ALL, M_NULL, M_NULL, M_DEFAULT);
      M3dmetGetResult(MilStatResult, M_STAT_MAX, &PositiveDeviation);
      M3dmetGetResult(MilStatResult, M_STAT_MIN, &NegativeDeviation);
      PlaneDepthVerticalMeasure.Planarity = PositiveDeviation - NegativeDeviation;

      // Show the depth map plane vertical planarity measures.
      MosPrintf(MIL_TEXT("By fitting a plane on the depth map data, the planarity of the surface\n")
                MIL_TEXT("along the Z axis can be measured.\n\n"));

      MbufLoad(MEASURES_ILLUSTRATIONS[1], MilIllustrationImage);
      AllMeasures.push_back(PlaneDepthVerticalMeasure);
      PrintResultTable(AllMeasures);
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Show the depth map plane normal planarity measure.
      MosPrintf(MIL_TEXT("By calculating the tilt from the plane equation, the planarity along\n")
                MIL_TEXT("the fitted plane normal can be deduced.\n\n"));

      MbufLoad(MEASURES_ILLUSTRATIONS[2], MilIllustrationImage);
      AllMeasures.push_back(PlaneDepthNormalMeasure);
      PrintResultTable(AllMeasures);
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();

      M3dmetFree(MilStatResult);
      M3dmetFree(MilStatContext);
      M3dgeoFree(MilCylinder);
      M3dgeoFree(MilPlane);
      M3dmetFree(MilFitResult);
      MbufFree(MilPtCldRegion);
      MbufFree(MilIllustrationImage);
      }
   else
      {
      MosPrintf(MIL_TEXT("Unable to find the part in the corrected depth map.\n")
                MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   // Free the allocated resources.
   MgraFree(MilDepthMapGraphicList);
   MdispFree(MilDepthMapDisplay);
   M3dgeoFree(MilMatrix);
   MbufFree(MilDepthMap);
   MdispFree(MilIllustrationDisplay);
   if(MilDisplay3D)
      { M3ddispFree(MilDisplay3D); }
   }
   
//************************************************************************************
// Finds the model, fixture a destination and draw the occurrence in the graphic list. 
//************************************************************************************
bool FixturePart(MIL_ID MilSystem, 
                 MIL_ID MilDepthMap,
                 MIL_ID MilDepthMapGraphicList,
                 MIL_ID MilMatrix)
   {
   // Restore and setup the model used to fixture the part.
   MIL_CONST_TEXT_PTR MECHANICAL_PART_MODEL = DATA_EX_PATH("ModelFinderContext.mmf");

   MIL_ID ModelCtx = MmodRestore(MECHANICAL_PART_MODEL, MilSystem, M_WITH_CALIBRATION, M_NULL);
   MIL_ID ModelRes = MmodAllocResult(MilSystem, M_DEFAULT, M_NULL);
   
   // Preprocess the model finder context.
   MmodPreprocess(ModelCtx, M_DEFAULT);

   // Create the fixturing offset.
   MIL_ID FixtureOffset = McalAlloc(MilSystem, M_FIXTURING_OFFSET, M_DEFAULT, M_NULL);
   McalFixture(M_NULL, FixtureOffset, M_LEARN_OFFSET, M_MODEL_MOD, ModelCtx, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Find the model.
   MmodFind(ModelCtx, MilDepthMap, ModelRes);

   // Retrieve the information.
   MIL_INT NumOfOccurences = 0;
   MmodGetResult(ModelRes, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumOfOccurences);

   if(NumOfOccurences)
      {
      // Fixture the depth map.
      McalFixture(MilDepthMap, FixtureOffset, M_MOVE_RELATIVE, M_RESULT_MOD, ModelRes, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      // Draw the found occurrence.
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MmodDraw(M_DEFAULT, ModelRes, MilDepthMapGraphicList, M_DRAW_EDGES + M_MODEL, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_DARK_CYAN);
      McalDraw(M_DEFAULT, MilDepthMap, MilDepthMapGraphicList, M_DRAW_RELATIVE_COORDINATE_SYSTEM + M_DRAW_FRAME, M_DEFAULT, M_DEFAULT);
      McalDraw(M_DEFAULT, FixtureOffset, MilDepthMapGraphicList, M_DRAW_FIXTURING_OFFSET, M_DEFAULT, M_DEFAULT);
      }

   MmodControl(ModelRes, M_DEFAULT, M_RESULT_OUTPUT_UNITS, M_WORLD);

   McalGetCoordinateSystem(MilDepthMap, M_RELATIVE_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM, M_HOMOGENEOUS_MATRIX, MilMatrix, M_NULL, M_NULL, M_NULL, M_NULL);
   M3dgeoMatrixSetTransform(MilMatrix, M_INVERSE,(MIL_DOUBLE) MilMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Release the allocated resources.
   MmodFree(ModelCtx);
   MmodFree(ModelRes);
   McalFree(FixtureOffset);

   return (NumOfOccurences > 0);
   }

//****************************************************************************
// Depth map projection from the point cloud container.
//****************************************************************************
void GenerateDepthMap(MIL_ID MilSystem,
                      MIL_ID PointCloudContainer,
                      MIL_DOUBLE PixelSize,
                      MIL_ID* pOutDepthmap)
   {
   // Calculate the size required for the depth map.
   MIL_ID MapSizeContext = M3dimAlloc(MilSystem, M_CALCULATE_MAP_SIZE_CONTEXT, M_DEFAULT, M_NULL);
   M3dimControl(MapSizeContext, M_PIXEL_SIZE_X, PixelSize);
   M3dimControl(MapSizeContext, M_PIXEL_SIZE_Y, PixelSize);
   M3dimControl(MapSizeContext, M_PIXEL_ASPECT_RATIO, M_NULL);
   MIL_INT DepthMapSizeX, DepthMapSizeY;
   M3dimCalculateMapSize(MapSizeContext, PointCloudContainer, M_NULL, M_DEFAULT, &DepthMapSizeX, &DepthMapSizeY);

   // Allocate and calibrate the depth map.
   *pOutDepthmap = MbufAlloc2d(MilSystem, DepthMapSizeX, DepthMapSizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC + M_DISP, M_NULL);
   M3dimCalibrateDepthMap(PointCloudContainer, *pOutDepthmap, M_NULL, M_NULL, M_DEFAULT, M_POSITIVE, M_DEFAULT);

   // Project the point cloud on the depth map.
   M3dimProject(PointCloudContainer, *pOutDepthmap, M_NULL, M_POINT_BASED, M_MAX_Z, M_DEFAULT, M_DEFAULT);

   // Release the allocated resources.
   M3dimFree(MapSizeContext);
   }

//****************************************************************************
// Prints the result table of all the planarity measures.
//****************************************************************************
void PrintResultTable(const vector<SPlanarityMeasure>& rPlanarityMeasures)
   {
   MosPrintf(MIL_TEXT("|--------------------------------------------------------------|-----------|\n")
             MIL_TEXT("|                 Planarity measure description                | Planarity |\n")
             MIL_TEXT("|--------------------------------------------------------------|-----------|\n"));

   for(MIL_UINT m = 0; m < rPlanarityMeasures.size(); m++)
      {
      MIL_INT NameSize = MosStrlen(rPlanarityMeasures[m].MeasureName);
      MIL_INT Left = (62 - NameSize) / 2;
      MIL_INT Right = 62 - NameSize - Left;
      MosPrintf(MIL_TEXT("|%*s%*s|%8.4f mm|\n"),
                Left + NameSize, rPlanarityMeasures[m].MeasureName,
                Right, MIL_TEXT(""),
                rPlanarityMeasures[m].Planarity);
      }

   MosPrintf(MIL_TEXT("|--------------------------------------------------------------|-----------|\n"));

   }

//****************************************************************************
// Check for required files to run the example.    
//****************************************************************************
bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The file needed to run this example is missing. You need \n")
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
                MIL_TEXT("The current system does not support the 3D display.\n"));
      }
   return MilDisplay3D;
   }
