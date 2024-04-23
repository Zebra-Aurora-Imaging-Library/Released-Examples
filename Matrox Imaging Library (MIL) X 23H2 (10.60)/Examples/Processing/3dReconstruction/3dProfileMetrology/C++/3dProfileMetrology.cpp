//***************************************************************************************
// 
// File name: 3dProfileMetrology.cpp  
//
// Synopsis: Demonstrates metrology operations along the 3D profile of a 
//           the scanned 3D mechanical part.
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
   MosPrintf(MIL_TEXT("3dProfileMetrology\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the validation of metrology measurements\n")
             MIL_TEXT("along a 3D profile of the 3d point cloud of a mechanical part."));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, Display, Buffer, Graphics,\n")
             MIL_TEXT("Calibration, 3D Image Processing, Geometric Model Finder,\n")
             MIL_TEXT("3D Display, 3D Graphics, and Metrology.\n\n"));
   }

static const MIL_INT DISPLAY_SIZE_X = 900;
static const MIL_INT DISPLAY_SIZE_Y = 720;

// Macro defining the example's filepath.
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("MechanicalPartScan/") MIL_TEXT(x))
#define METAL_PART_CLOUD_CONTAINER EX_PATH("MechanicalPart.ply")//

struct SProjectionBox
   {
   MIL_DOUBLE OffsetX;
   MIL_DOUBLE OffsetY;
   MIL_DOUBLE OffsetZ;
   MIL_DOUBLE Length;
   MIL_DOUBLE Thickness;
   MIL_DOUBLE Height;
   };

struct SRectRegion
   {
   MIL_DOUBLE OffsetX;
   MIL_DOUBLE OffsetY;
   MIL_DOUBLE Width;
   MIL_DOUBLE Height;
   MIL_DOUBLE Angle;
   };

struct SArcRegion
   {
   MIL_DOUBLE OffsetX;
   MIL_DOUBLE OffsetY;
   MIL_DOUBLE StartRadius;
   MIL_DOUBLE EndRadius;
   MIL_DOUBLE StartAngle;
   MIL_DOUBLE EndAngle;
   };

//****************************************************************************
// Functions' declarations.
//****************************************************************************
void GenerateDepthMap(MIL_ID MilSystem,
                      MIL_ID PointCloudContainer,
                      MIL_DOUBLE PixelSize,
                      MIL_ID& OutDepthmap);

void Analyze3DProfile(MIL_ID MilSystem,
                      MIL_ID PointCloudContainer);

bool FixturePart(MIL_ID MilSystem, 
                 MIL_ID MilDepthMap,
                 MIL_ID MilDepthMapGraphicList,
                 MIL_ID MilFixtureDestination);

bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName);

MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);

bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   // Allocate the MIL application.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);

   //Check for required example files.
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
   Analyze3DProfile(MilSystem, PointCloudContainer);

   // Free the MIL 3D point cloud.
   MbufFree(PointCloudContainer);

   // Free the MIL system and application.
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//*******************************************************************************
// 3D profile analysis of the the scanned object.
//*******************************************************************************
void Analyze3DProfile (MIL_ID MilSystem, MIL_ID PointCloudContainer)
   {
   // Allocates the displays and graphic lists.
   MIL_ID MilGraphicList = M_NULL;
   MIL_ID MilDisplay3D   = Alloc3dDisplayId(MilSystem);
   if(MilDisplay3D)
      { M3ddispInquire(MilDisplay3D, M_3D_GRAPHIC_LIST_ID, &MilGraphicList); }

   MIL_ID MilDisplayProjection = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID MilProjectionGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MdispControl(MilDisplayProjection, M_ASSOCIATED_GRAPHIC_LIST_ID, MilProjectionGraphicList);

   // Display the point cloud.
   if(MilDisplay3D)
      {
      M3ddispControl(MilDisplay3D, M_SIZE_X, DISPLAY_SIZE_X);
      M3ddispControl(MilDisplay3D, M_SIZE_Y, DISPLAY_SIZE_Y);
      M3ddispSetView(MilDisplay3D, M_AUTO  , M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      M3ddispControl(MilDisplay3D, M_UPDATE, M_DISABLE);
      MIL_INT64 MilContainerGraphics = M3ddispSelect(MilDisplay3D, PointCloudContainer, M_SELECT, M_DEFAULT);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_USE_LUT       , M_TRUE);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT     , M_COMPONENT_RANGE);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT_BAND, 2);

      // Generate a grayscale flipped LUT.
      MIL_ID GrayFlipLUT = MbufAlloc1d(MilSystem, 256, 8 + M_UNSIGNED, M_LUT, M_NULL);
      MgenLutRamp(GrayFlipLUT, 0, 255.0, 255, 0.0);
      M3dgraCopy(GrayFlipLUT, M_DEFAULT, MilGraphicList, MilContainerGraphics, M_COLOR_LUT, M_DEFAULT);
      MbufFree(GrayFlipLUT); 

      M3ddispControl(MilDisplay3D, M_UPDATE, M_ENABLE);
      M3dgraAxis(MilGraphicList, M_ROOT_NODE, M_DEFAULT, 100, MIL_TEXT(""), M_DEFAULT, M_DEFAULT);
      MosPrintf(MIL_TEXT("A scan of a mechanical part was restored and displayed.\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }
   
   // Set the extraction box definition in world units and relative to the part fixture.
   // The extraction box is defined to retrieve a slice of 3D positions from the scanned object.
   SProjectionBox ProjBox;
   ProjBox.OffsetX   = 41.0;
   ProjBox.OffsetY   = 70.0;
   ProjBox.OffsetZ   = -20.0;
   ProjBox.Length    = 90.0;
   ProjBox.Height    = 40.0;
   ProjBox.Thickness = 0.1;

   MIL_ID MilDepthMap = M_NULL;
   const MIL_DOUBLE PixelSize = 0.3;
   GenerateDepthMap(MilSystem, PointCloudContainer, PixelSize, MilDepthMap);

   // Allocate the necessary buffers for processing and display.
   MIL_ID MilDepthMapDisplay     = MdispAlloc   (MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MIL_ID MilDepthMapGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MdispControl(MilDepthMapDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilDepthMapGraphicList);
   MdispControl(MilDepthMapDisplay, M_WINDOW_INITIAL_POSITION_X ,  DISPLAY_SIZE_X);

   // Display the depth map and its calibration.
   MgraClear(M_DEFAULT, MilDepthMapGraphicList);
   MgraColor(M_DEFAULT, M_COLOR_LIGHT_BLUE);   
   McalDraw(M_DEFAULT, MilDepthMap, MilDepthMapGraphicList, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);
  
   MdispSelect(MilDepthMapDisplay, MilDepthMap);

   MosPrintf(MIL_TEXT("A top view calibrated depth map of the mechanical part was generated.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Locate the part and generate a fixturing matrix.
   MIL_ID MilMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_NULL);
   bool PartFound   = FixturePart(MilSystem, MilDepthMap, MilDepthMapGraphicList, MilMatrix);

   if (PartFound)
      {
      // Fixture the point cloud with the model finding result.
      M3dimMatrixTransform(PointCloudContainer, PointCloudContainer, MilMatrix, M_DEFAULT);

      MosPrintf(MIL_TEXT("The mechanical part was located and fixtured using Model Finder in the\ndepth map.\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));

      //Draw where the profile is considered.
      MgraColor  (M_DEFAULT, M_COLOR_YELLOW);
      MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
      MgraLine(M_DEFAULT, MilDepthMapGraphicList,
               ProjBox.OffsetX, ProjBox.OffsetY, ProjBox.OffsetX, ProjBox.OffsetY + ProjBox.Length);
      if(MilDisplay3D)
         {
         // Draw a clipped plane equivalent to the yellow line drawn on the depth map.
         MIL_UNIQUE_3DGEO_ID MilBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
         M3dgeoBox (MilBox, M_CENTER_AND_DIMENSION, ProjBox.OffsetX, ProjBox.OffsetY + 0.5*ProjBox.Length,
                    ProjBox.OffsetZ, ProjBox.Thickness , ProjBox.Length, ProjBox.Height, M_DEFAULT);
         M3dgraCopy(MilBox, M_DEFAULT, MilGraphicList, M_LIST, M_CLIPPING_BOX, M_DEFAULT);

         MIL_INT64 GraPlane = M3dgraPlane(MilGraphicList, M_DEFAULT, M_POINT_AND_NORMAL, ProjBox.OffsetX,
                                          ProjBox.OffsetY, ProjBox.OffsetZ, 1.0, 0.0, 0.0, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         M3dgraControl(MilGraphicList, GraPlane, M_OPACITY, 80);
         M3dgraControl(MilGraphicList, GraPlane, M_COLOR  , M_COLOR_YELLOW);
         MosPrintf(MIL_TEXT("The profile plane is set relative to the fixture in yellow.\n")
                   MIL_TEXT("Press <Enter> to continue.\n\n"));
         MosGetch();
         }

      MIL_UNIQUE_3DGEO_ID FixtureMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
    
      M3dgeoMatrixSetWithAxes(FixtureMatrix, M_ZX_AXES + M_COORDINATE_SYSTEM_TRANSFORMATION, ProjBox.OffsetX , ProjBox.OffsetY, 0.0,
                              ProjBox.Thickness, 0.0, 0.0, 0.0,  ProjBox.Length, 0.0 , M_DEFAULT);
      MIL_UNIQUE_3DIM_ID MilProfile = M3dimAllocResult(MilSystem, M_PROFILE_RESULT, M_DEFAULT, M_UNIQUE_ID);
      M3dimProfile(PointCloudContainer, MilProfile, M_PROFILE_POINT_CLOUD, FixtureMatrix, PixelSize, PixelSize, ProjBox.Thickness, ProjBox.Length, M_DEFAULT);

      // Get the profile points.
      MIL_INT NumberOfExtractedPoints;
      M3dimGetResult(MilProfile, M_NUMBER_OF_POINTS_VALID, &NumberOfExtractedPoints);
      std::vector<MIL_DOUBLE> XDouble;
      std::vector<MIL_DOUBLE> YDouble;
      M3dimGetResult(MilProfile, M_PROFILE_PLANE_X, XDouble);
      M3dimGetResult(MilProfile, M_PROFILE_PLANE_Y, YDouble);

      // Allocate the Metrology tool to perform measures along the 3D profile.
      MIL_ID MetContext = MmetAlloc      (MilSystem, M_DEFAULT, M_NULL);
      MIL_ID MetResult  = MmetAllocResult(MilSystem, M_DEFAULT, M_NULL);

      const SRectRegion UpperRect = {40, -25,  8, 15,   0};
      const SRectRegion LowerRect = {55, -15, 20, 10,   0};
      const SArcRegion  UpperArc  = {12, -30,  3,  8, 180, 270};

      // Set the Metrology features and tolerances.
      const MIL_INT EdgelLabel[1] = { M_FEATURE_INDEX(1) };
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_EDGEL, M_DEFAULT, M_EXTERNAL_FEATURE, M_NULL, M_NULL, 0, M_DEFAULT);
      MmetPut(MetContext, M_FEATURE_INDEX(1), NumberOfExtractedPoints, M_NULL, XDouble, YDouble, M_NULL, M_NULL, M_DEFAULT);

      // Denoising of the previously entered edgels.
      MmetControl(MetContext, M_FEATURE_LABEL(1), M_EDGEL_DENOISING_MODE, M_GAUSSIAN);
      MmetControl(MetContext, M_FEATURE_LABEL(1), M_EDGEL_DENOISING_RADIUS, 1.5);

      MmetAddFeature(MetContext, M_CONSTRUCTED, M_SEGMENT, M_DEFAULT, M_FIT, EdgelLabel, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_INDEX(2), M_DEFAULT, M_RECTANGLE, UpperRect.OffsetX, UpperRect.OffsetY, UpperRect.Width, UpperRect.Height, UpperRect.Angle, M_NULL);

      MmetAddFeature(MetContext, M_CONSTRUCTED, M_SEGMENT, M_DEFAULT, M_FIT, EdgelLabel, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_INDEX(3), M_DEFAULT, M_RECTANGLE, LowerRect.OffsetX, LowerRect.OffsetY, LowerRect.Width, LowerRect.Height, LowerRect.Angle, M_NULL);

      const MIL_INT SegLabels[2] = { M_FEATURE_INDEX(2), M_FEATURE_INDEX(3) };
      MmetAddTolerance(MetContext, M_PARALLELISM, M_DEFAULT, 0, 2, SegLabels, M_NULL, 2, M_DEFAULT);

      MmetAddFeature(MetContext, M_CONSTRUCTED, M_ARC, M_DEFAULT, M_FIT, EdgelLabel, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_INDEX(4), M_DEFAULT, M_RING_SECTOR, UpperArc.OffsetX, UpperArc.OffsetY, UpperArc.StartRadius, UpperArc.EndRadius, UpperArc.StartAngle, UpperArc.EndAngle);

      const MIL_INT ArcLabel[1] = { M_FEATURE_INDEX(4)};
      MmetAddTolerance(MetContext, M_RADIUS, M_DEFAULT, 0, 2, ArcLabel, M_NULL, 1, M_DEFAULT);

      // Calculate the features and tolerances.
      MmetCalculate(MetContext, M_NULL, MetResult, M_DEFAULT);

      // Allocate a buffer to display the extracted 3D slice.
      const MIL_DOUBLE ZoomFactor = 3.0;
      MIL_ID SliceDispImage =
         MbufAlloc2d(MilSystem,
                     (MIL_INT) (ZoomFactor * ProjBox.Length / PixelSize),
                     (MIL_INT) (ZoomFactor * 2.0 * ProjBox.Height / PixelSize),
                     8 + M_UNSIGNED,
                     M_IMAGE + M_PROC + M_DISP,
                     M_NULL);

      MbufClear(SliceDispImage, 0.0);
      McalUniform(SliceDispImage, 0.0, ProjBox.OffsetZ - ProjBox.Height, PixelSize / ZoomFactor, PixelSize / ZoomFactor, 0.0, M_DEFAULT);
      MgraClear(M_DEFAULT, MilProjectionGraphicList);

      // Display the calibration system.
      MgraColor(M_DEFAULT, M_COLOR_LIGHT_GRAY);
      McalDraw(M_DEFAULT, SliceDispImage, MilProjectionGraphicList, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);

      // Populate the slice with the extracted points.
      MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
      MgraColor(M_DEFAULT, 96.0); // dark gray.
      MgraDots (M_DEFAULT, SliceDispImage, NumberOfExtractedPoints, XDouble, YDouble, M_DEFAULT);

      // Display the Metrology results.
      MgraColor(M_DEFAULT, M_RGB888(64, 240, 128)); // soft green.
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_REGION, M_DEFAULT, M_DEFAULT);

      MgraColor(M_DEFAULT, M_RGB888(164, 164, 0)); // dark yellow
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_NOISY_EDGELS, M_FEATURE_INDEX(1), M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_YELLOW);
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(1), M_DEFAULT);

      MgraColor(M_DEFAULT, M_RGB888(32, 164, 240)); // light blue
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_ACTIVE_EDGELS, M_FEATURE_INDEX(2), M_DEFAULT);
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_ACTIVE_EDGELS, M_FEATURE_INDEX(3), M_DEFAULT);
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_ACTIVE_EDGELS, M_FEATURE_INDEX(4), M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_RED);
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_FEATURE, M_FEATURE_INDEX(2), M_DEFAULT);
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_FEATURE, M_FEATURE_INDEX(3), M_DEFAULT);
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_FEATURE, M_FEATURE_INDEX(4), M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
      MmetDraw (M_DEFAULT, MetResult, MilProjectionGraphicList, M_DRAW_TOLERANCE, M_ALL, M_DEFAULT);

      // Select the buffer to the display.
      MdispSelect(MilDisplayProjection, SliceDispImage);

      MosPrintf(MIL_TEXT("Metrology measurements and tolerances were calculated along the 3D profile.\n")
                MIL_TEXT("   - Profile positions are displayed in yellow (before denoising \n")
                MIL_TEXT("     positions are darker).\n")
                MIL_TEXT("   - Regions are displayed in green.\n")
                MIL_TEXT("   - Active edgels are displayed in blue.\n")
                MIL_TEXT("   - Fitted features are displayed in red.\n")
                MIL_TEXT("   - Tolerances are displayed in magenta.\n\n"));

      MosPrintf(MIL_TEXT("The display can be zoomed to observe the subpixel annotations.\n\n")
                MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();

      // Free the allocated resources.
      M3dgeoFree(MilMatrix);
      MmetFree(MetContext);
      MmetFree(MetResult);
      MbufFree(SliceDispImage);
      }
   else
      {
      MosPrintf(MIL_TEXT("Unable to find the part in the corrected depth map.\n")
                MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   // Free the allocated resources.
   if(MilDisplay3D)
      { M3ddispFree(MilDisplay3D); }
   MgraFree(MilDepthMapGraphicList);
   MdispFree(MilDepthMapDisplay);
   MbufFree(MilDepthMap);
   MdispFree(MilDisplayProjection);
   MgraFree(MilProjectionGraphicList);
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
   MIL_CONST_TEXT_PTR MECHANICAL_PART_MODEL = EX_PATH("ModelFinderContext.mmf");

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
      // Fixture the depth map for display purposes.
      McalFixture(MilDepthMap, FixtureOffset, M_MOVE_RELATIVE, M_RESULT_MOD, ModelRes, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      // Draw the found occurrence.
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MmodDraw(M_DEFAULT, ModelRes, MilDepthMapGraphicList, M_DRAW_EDGES + M_MODEL, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_BLUE);
      MgraBackColor(M_DEFAULT, M_COLOR_WHITE);
      McalDraw(M_DEFAULT, MilDepthMap  , MilDepthMapGraphicList, M_DRAW_RELATIVE_COORDINATE_SYSTEM + M_DRAW_FRAME, M_DEFAULT, M_DEFAULT);
      McalDraw(M_DEFAULT, FixtureOffset, MilDepthMapGraphicList, M_DRAW_FIXTURING_OFFSET, M_DEFAULT, M_DEFAULT);
      }

   // Get back the fixturing matrix from the model finder result.
   McalFixture(MilMatrix, FixtureOffset, M_MOVE_RELATIVE, M_RESULT_MOD, ModelRes, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // The returned matrix go from model to object. Inverse it to go from object to model.
   M3dgeoMatrixSetTransform(MilMatrix, M_INVERSE, MilMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Release the allocated resources.
   MmodFree(ModelCtx);
   MmodFree(ModelRes);
   McalFree(FixtureOffset);

   return (NumOfOccurences > 0);
   }

//*****************************************************************************
// Check for required files for running the example.   
//*****************************************************************************
bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The files needed to run this example are missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }

//*****************************************************************************
// Projects the point cloud into a depth map.
//*****************************************************************************
void GenerateDepthMap(MIL_ID     MilSystem,
                      MIL_ID     PointCloudContainer,
                      MIL_DOUBLE PixelSize,
                      MIL_ID&    OutDepthmap)
   {
   // Set the volume information.
   SProjectionBox Box;
   Box.OffsetX   =    5.00;
   Box.OffsetY   = -160.00;
   Box.OffsetZ   =   -4.00;
   Box.Length    =  120.00;
   Box.Height    =  410.00;
   Box.Thickness =  -30.00;

   MIL_UNIQUE_3DGEO_ID MilBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoBox(MilBox, M_CORNER_AND_DIMENSION, Box.OffsetX, Box.OffsetY, Box.OffsetZ, Box.Length, Box.Height, Box.Thickness, M_DEFAULT);
   M3dimCrop(PointCloudContainer, PointCloudContainer, MilBox, M_NULL, M_UNORGANIZED, M_DEFAULT);

   // Calculate the size required for the depth map.
   MIL_ID MapSizeContext = M3dimAlloc(MilSystem, M_CALCULATE_MAP_SIZE_CONTEXT, M_DEFAULT, M_NULL);
   M3dimControl(MapSizeContext, M_PIXEL_SIZE_X, PixelSize);
   M3dimControl(MapSizeContext, M_PIXEL_SIZE_Y, PixelSize);
   M3dimControl(MapSizeContext, M_PIXEL_ASPECT_RATIO, M_NULL);
   MIL_INT DepthMapSizeX, DepthMapSizeY;
   M3dimCalculateMapSize(MapSizeContext, PointCloudContainer, M_NULL, M_DEFAULT, &DepthMapSizeX, &DepthMapSizeY);

   // Allocate and calibrate the depth map.
   OutDepthmap = MbufAlloc2d(MilSystem, DepthMapSizeX, DepthMapSizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC + M_DISP, M_NULL);
   M3dimCalibrateDepthMap(PointCloudContainer, OutDepthmap, M_NULL, M_NULL, M_DEFAULT, M_NEGATIVE, M_DEFAULT);

   // Project the point cloud on the depth map.
   M3dimProject(PointCloudContainer, OutDepthmap, M_NULL, M_POINT_BASED, M_MAX_Z, M_DEFAULT, M_DEFAULT);

   // Fill gaps if there is any.
   MIL_UNIQUE_3DIM_ID FillGapsContext = M3dimAlloc(MilSystem, M_FILL_GAPS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(FillGapsContext, M_FILL_MODE           , M_X_THEN_Y);
   M3dimControl(FillGapsContext, M_FILL_SHARP_ELEVATION, M_DISABLE);
   M3dimControl(FillGapsContext, M_FILL_THRESHOLD_X    , 1.0);
   M3dimControl(FillGapsContext, M_FILL_THRESHOLD_Y    , 1.0);

   M3dimFillGaps(FillGapsContext, OutDepthmap, M_NULL, M_DEFAULT);

   // Release the allocated resources.
   M3dimFree(MapSizeContext);
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
