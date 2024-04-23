//*************************************************************************************
//
// File name: BlobDepthMap.cpp
//
// Synopsis:  This program uses the 2D blob module to calculate
//            3D features on a depth map.
//
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

#define EXAMPLE_IMAGE_PATH       M_IMAGE_PATH MIL_TEXT("BlobAnalysis/BlobDepthMap/")
#define IMAGE_FILE               EXAMPLE_IMAGE_PATH MIL_TEXT("RocksDepthMap.mim")

//*****************************************************************************
// Example description.
//*****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("Blob Depth Map Example\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows how depth map blob results can be used on rocks 3D scans.\n")

             MIL_TEXT("Step 1: Restore the rocks depth map and do some preprocessing.\n")
             MIL_TEXT("Step 2: Calculate and display results obtained using the 2D blob module on the 2D depth map.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used : application, system, display, graphics, buffer, blob, \n"
                      "Image Processing, 3D Display, 3D Graphics, 3D Image Processing\n\n"));
   }

void DrawBoxes(MIL_ID BlobResult, MIL_INT BlobIndex, MIL_ID GraList2D, MIL_ID GraList3d, MIL_DOUBLE MinZ, MIL_DOUBLE MaxZ)
   {
   MIL_DOUBLE BoxXMax, BoxXMin, BoxYMax, BoxYMin;

   // Draw box on 2D display
   MblobControl(BlobResult, M_RESULT_OUTPUT_UNITS, M_PIXEL);
   MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_BOX_X_MIN, &BoxXMin);
   MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_BOX_Y_MIN, &BoxYMin);
   MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_BOX_X_MAX, &BoxXMax);
   MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_BOX_Y_MAX, &BoxYMax);

   MblobDraw(M_DEFAULT, BlobResult, GraList2D, M_DRAW_BOX, M_BLOB_INDEX(BlobIndex), M_DEFAULT);
   MgraText(M_DEFAULT, GraList2D, (BoxXMax + BoxXMin) / 2, (BoxYMax + BoxYMin) / 2, MIL_TEXT("ROCK ") + M_TO_STRING(BlobIndex + 1));

   // Draw box on 3D display
   MblobControl(BlobResult, M_RESULT_OUTPUT_UNITS, M_WORLD);
   MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_BOX_X_MIN, &BoxXMin);
   MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_BOX_Y_MIN, &BoxYMin);
   MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_BOX_X_MAX, &BoxXMax);
   MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_BOX_Y_MAX, &BoxYMax);

   M3dgraBox(GraList3d, M_ROOT_NODE, M_BOTH_CORNERS,
             BoxXMin, BoxYMin, MinZ,
             BoxXMax, BoxYMax, MaxZ,
             M_DEFAULT, M_DEFAULT);
   }

MIL_DOUBLE EstimateRealVolume(MIL_ID BlobContext, MIL_ID BlobResult, MIL_ID BlobClipResult,
                              MIL_INT BlobIndex, MIL_ID DepthMapProc, MIL_ID DepthMapClip,
                              MIL_DOUBLE Volume, MIL_DOUBLE MinZ, MIL_DOUBLE MaxZ)
   {
   MIL_DOUBLE ClippedVolume, BelowVolume;
   MblobControl(BlobClipResult, M_INPUT_SELECT_UNITS, M_WORLD);
   // The following condition allows to handle the case when the symmetry of the rock
   // goes below the elevation 0. We assume that the rock can not be below 0.
   if(MinZ < 0.5 * MaxZ)
      {
      // Get minimum elevation in pixel
      MIL_DOUBLE MinPixel;
      MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_MIN_PIXEL, &MinPixel);

      // Clip depth map buffer to keep only the volume below the box of the rock (below MinZ)
      MimArith(DepthMapProc, MinPixel, DepthMapClip, M_MIN_CONST);

      // Delete invalid pixels
      MbufSetRegion(DepthMapClip, M_NULL, M_DEFAULT, M_RASTERIZE_DEPTH_MAP_VALID_PIXELS, M_DEFAULT);

      // Calculate and select blobs
      MblobCalculate(BlobContext, M_NULL, DepthMapClip, BlobClipResult);
      MblobSelect(BlobClipResult, M_EXCLUDE, M_AREA, M_LESS, 100, 0);

      // Get the below volume
      MblobGetResult(BlobClipResult, M_BLOB_INDEX(BlobIndex), M_DEPTH_MAP_VOLUME, &BelowVolume);

      // Clear clip buffer
      MbufClear(DepthMapClip, 0);

      // Clip depth map buffer to keep only pixels below 2 * MinZ (MinZ is the axis of symmetry)
      MimArith(DepthMapProc, 2.0*MinPixel, DepthMapClip, M_MIN_CONST);
      MbufSetRegion(DepthMapClip, M_NULL, M_DEFAULT, M_RASTERIZE_DEPTH_MAP_VALID_PIXELS, M_DEFAULT);

      // Calculate and select blobs
      MblobCalculate(BlobContext, M_NULL, DepthMapClip, BlobClipResult);
      MblobSelect(BlobClipResult, M_EXCLUDE, M_AREA, M_LESS, 100, 0);

      // Get the clipped volume
      MblobGetResult(BlobClipResult, M_BLOB_INDEX(BlobIndex), M_DEPTH_MAP_VOLUME, &ClippedVolume);

      // Estimate the volume
      return (Volume + ClippedVolume - 2.0 * BelowVolume);
      }
   // Here, we assume that the rock can not fly in the air : it must be on the ground.
   else
      {
      // Get maximum elevation in pixel
      MIL_DOUBLE MaxPixel;
      MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_MAX_PIXEL, &MaxPixel);

      // Clip depth map buffer to keep only pixels below 0.5 * MaxZ (0.5*MaxZ is the axis of symmetry)
      MimArith(DepthMapProc, 0.5*MaxPixel, DepthMapClip, M_MIN_CONST);
      MbufSetRegion(DepthMapClip, M_NULL, M_DEFAULT, M_RASTERIZE_DEPTH_MAP_VALID_PIXELS, M_DEFAULT);

      // Calculate and select blobs
      MblobCalculate(BlobContext, M_NULL, DepthMapClip, BlobClipResult);
      MblobSelect(BlobClipResult, M_EXCLUDE, M_AREA, M_LESS, 100, 0);

      // Get the clip volume
      MblobGetResult(BlobClipResult, M_BLOB_INDEX(BlobIndex), M_DEPTH_MAP_VOLUME, &ClippedVolume);

      return (2.0 * (Volume - ClippedVolume));
      }
   }

int MosMain()
   {
   // Allocate defaults.
   auto ApplicationId = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto SystemId = M_DEFAULT_HOST;

   MIL_INT NumberOfRocks;
   MIL_DOUBLE Volume, MinZ, MaxZ, MeanZ, SizeZ;
   MIL_DOUBLE EstimatedVolume;

   // Allocate blob context and result
   auto BlobContext    = MblobAlloc(SystemId, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   auto BlobResult     = MblobAllocResult(SystemId, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   auto BlobClipResult = MblobAllocResult(SystemId, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Alloc 3D display and control it
   auto Display3DId       = M3ddispAlloc(SystemId, M_DEFAULT, MIL_TEXT(""), M_DEFAULT, M_UNIQUE_ID);
   auto MilMapSizeContext = M3dimAlloc(SystemId, M_CALCULATE_MAP_SIZE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto Context3d         = M3dimAlloc(SystemId, M_FILL_GAPS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto GraList3d         = (MIL_ID) M3ddispInquire(Display3DId, M_3D_GRAPHIC_LIST_ID + M_TYPE_MIL_ID, M_NULL);
   M3dgraControl(GraList3d, M_DEFAULT_SETTINGS, M_COLOR_USE_LUT, M_TRUE);

   // Get the graphic list to draw annotations
   M3dgraControl(GraList3d, M_DEFAULT_SETTINGS, M_APPEARANCE, M_WIREFRAME);

   // Alloc 2D display and control it
   auto Display2DId = MdispAlloc(SystemId, M_DEFAULT, MIL_TEXT(""), M_DEFAULT, M_UNIQUE_ID);
   MdispControl(Display2DId, M_SCALE_DISPLAY, M_ENABLE);
   MdispControl(Display2DId, M_WINDOW_INITIAL_POSITION_X, 800);
   MdispZoom(Display2DId, 0.5, 0.5);

   // Allocate a graphic list to hold the subpixel annotations to draw. 
   MIL_UNIQUE_GRA_ID GraList2D = MgraAllocList(SystemId, M_DEFAULT, M_UNIQUE_ID);
   // Associate the graphic list to the 2D display. 
   MdispControl(Display2DId, M_ASSOCIATED_GRAPHIC_LIST_ID, GraList2D);

   // Set colors which will be used to display blob boxes
   std::vector<MIL_DOUBLE> Colors {M_COLOR_GREEN, M_COLOR_RED, M_COLOR_BLUE,
                                   M_COLOR_YELLOW, M_COLOR_MAGENTA, M_COLOR_GRAY,
                                   M_COLOR_WHITE, M_COLOR_CYAN};

   PrintHeader();

   // Import the depth map
   auto DepthMapBuf = MbufImport(IMAGE_FILE, M_MIL_TIFF + M_WITH_CALIBRATION, M_RESTORE, SystemId, M_UNIQUE_ID);
   MIL_INT SizeX = MbufInquire(DepthMapBuf, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquire(DepthMapBuf, M_SIZE_Y, M_NULL);

   // Allocation of needed buffers to process the depth map
   auto DepthMapProc = MbufAlloc2d(SystemId, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   auto DepthMapIdent = MbufAlloc2d(SystemId, SizeX, SizeY, M_UNSIGNED + 1, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   auto DepthMapClip = MbufAlloc2d(SystemId, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);

   // Clear buffers
   MbufClear(DepthMapProc, 0);
   MbufClear(DepthMapIdent, 0);
   MbufClear(DepthMapClip, 0);

   // Copy processed depth map buffer from original one
   MbufCopy(DepthMapBuf, DepthMapProc);

   // Set threshold for gaps filling
   M3dimControl(Context3d, M_FILL_THRESHOLD_X, 0.28);
   M3dimControl(Context3d, M_FILL_THRESHOLD_Y, 0.86);

   // Avoid holes by filling gaps in processed depth map
   M3dimFillGaps(Context3d, DepthMapProc, M_NULL, M_DEFAULT);

   // Remove background from processed depth map
   MimBinarize(DepthMapProc, DepthMapIdent, M_FIXED + M_GREATER, 45.0, M_NULL);

   // Delete invalid pixels in processed depth map, they must not be considered in the calculations.
   MbufSetRegion(DepthMapProc, DepthMapIdent, M_DEFAULT, M_RASTERIZE_DEPTH_MAP_VALID_PIXELS, M_DEFAULT);

   // Display points cloud
   M3ddispControl(Display3DId, M_TITLE, MIL_TEXT("Rocks Point Cloud"));
   M3ddispSelect(Display3DId, DepthMapBuf, M_DEFAULT, M_DEFAULT);

   // Display 2D depth map
   MdispControl(Display2DId, M_TITLE, MIL_TEXT("2D Depth Map"));
   MdispSelect(Display2DId, DepthMapProc);

   MosPrintf(MIL_TEXT("3D features are going to be calculated using 2D blob and the 2D depth map buffer.\n\n"));
   MosPrintf(MIL_TEXT("Gaps have been filled in the depth map in order to calculate a volume \nas close as possible from the real one.\n\n"));
   MosPrintf(MIL_TEXT("Invalid pixels (maximum label value) have been excluded from the region.\n"));
   MosPrintf(MIL_TEXT("Therefore, they will not be considered in the calculations.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue."));
   MosGetch();

   // Enable needed features
   MblobControl(BlobContext, M_DEPTH_MAP_VOLUME, M_ENABLE);
   MblobControl(BlobContext, M_DEPTH_MAP_MIN_ELEVATION, M_ENABLE);
   MblobControl(BlobContext, M_DEPTH_MAP_MAX_ELEVATION, M_ENABLE);
   MblobControl(BlobContext, M_DEPTH_MAP_MEAN_ELEVATION, M_ENABLE);
   MblobControl(BlobContext, M_DEPTH_MAP_SIZE_Z, M_ENABLE);

   MblobControl(BlobContext, M_BOX, M_ENABLE);

   // Set input units on result
   MblobControl(BlobResult, M_INPUT_SELECT_UNITS, M_WORLD);

   // Calculate and select blobs (exclude small blobs)
   MblobCalculate(BlobContext, M_NULL, DepthMapProc, BlobResult);
   MblobSelect(BlobResult, M_EXCLUDE, M_AREA, M_LESS, 100, 0);

   // Get results
   MosPrintf(MIL_TEXT("\n\n---------- RESULTS ----------\n\n"));
   MosPrintf(MIL_TEXT("Here, the volume computed is the volume of the rock added to the volume between \nthe rock and the floor.\n"));
   MosPrintf(MIL_TEXT("Min elevation is the difference between the lowest rock elevation and the floor.\n"));
   MosPrintf(MIL_TEXT("Max elevation is the difference between the highest rock elevation and the floor.\n"));
   MosPrintf(MIL_TEXT("Mean elevation is the mean elevation of the rock.\n"));
   MosPrintf(MIL_TEXT("Z size is the difference between maximum and minimum rock elevation.\n\n"));
   MosPrintf(MIL_TEXT("We can estimate the real volume of a rock with several methods.\n"));
   MosPrintf(MIL_TEXT("In this example, we assume that the shape of the rock is symmetrical to estimate \nits real volume.\n"));

   // Get number of rocks
   MblobGetResult(BlobResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NumberOfRocks);

   for(int BlobIndex = 0; BlobIndex < NumberOfRocks; BlobIndex++)
      {
      // Control draw aspect
      MgraColor(M_DEFAULT, Colors[BlobIndex]);
      M3dgraControl(GraList3d, M_DEFAULT_SETTINGS, M_COLOR, Colors[BlobIndex]);

      // Get the results
      MblobControl(BlobResult, M_RESULT_OUTPUT_UNITS, M_WORLD);
      MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_DEPTH_MAP_VOLUME, &Volume);
      MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_DEPTH_MAP_MIN_ELEVATION, &MinZ);
      MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_DEPTH_MAP_MAX_ELEVATION, &MaxZ);
      MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_DEPTH_MAP_MEAN_ELEVATION, &MeanZ);
      MblobGetResult(BlobResult, M_BLOB_INDEX(BlobIndex), M_DEPTH_MAP_SIZE_Z, &SizeZ);

      // Print the results
      MosPrintf(MIL_TEXT("\n\nRock %d :\n"), BlobIndex + 1);
      MosPrintf(MIL_TEXT("\tVolume : %0.2f\n"), Volume);
      MosPrintf(MIL_TEXT("\tMin elevation : %0.2f\n"), MinZ);
      MosPrintf(MIL_TEXT("\tMax elevation : %0.2f\n"), MaxZ);
      MosPrintf(MIL_TEXT("\tMean elevation : %0.2f\n"), MeanZ);
      MosPrintf(MIL_TEXT("\tZ Size : %0.2f\n\n"), SizeZ);

      // Draw boxes on 2D and 3D displays
      DrawBoxes(BlobResult, BlobIndex, GraList2D, GraList3d, MinZ, MaxZ);

      // Real volume estimation
      EstimatedVolume = EstimateRealVolume(BlobContext, BlobResult, BlobClipResult, BlobIndex,
                                           DepthMapProc, DepthMapClip, Volume, MinZ, MaxZ);

      MosPrintf(MIL_TEXT("\tEstimated volume : %0.2f\n\n"), EstimatedVolume);
      MosPrintf(MIL_TEXT("Press <Enter> to continue."));
      MosGetch();
      }      

   MosPrintf(MIL_TEXT("\n\nPress <Enter> to quit."));
   MosGetch();
   }
