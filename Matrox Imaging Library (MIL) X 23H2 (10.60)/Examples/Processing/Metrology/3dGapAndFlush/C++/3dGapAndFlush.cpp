﻿//***************************************************************************************
// 
// File name: 3DGapAndFlush.cpp  
//
// Synopsis: Demonstrates metrology operations along 3d profiles 
//           of a mechanical part to measure Gaps and Flush.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("3dGapAndFlush\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("Demonstrates metrology operations along 3d profiles of\n")
             MIL_TEXT("a mechanical part to perform Gap and Flush measurement."));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, system, display, buffer, graphic,\n")
             MIL_TEXT("image processing, calibration, and metrology.\n\n"));
   }

// Macro defining the example's file path.
#define IMAGE_FILENAME "LaserMultilineProfiles.mim"
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("3dProfilometry/") MIL_TEXT(x))

const MIL_DOUBLE PeakMinContrast  = 50;
const MIL_DOUBLE PeakWidthNominal = 20;
const MIL_DOUBLE PeakWidthDelta   = 15;
const MIL_INT    NumberOfProfiles =  9;

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   MIL_ID MilApplication,
          MilSystem,
          MilImage,
          MilAnalyse,
          MilDisplayImage,
          MilDisplayAnalyse,
          GraphicListImage,
          GraphicListAnalyse;

   MIL_ID MilPeakContext,
          MilPeakResult;

   MIL_ID MetContext,
          MetResult;

   MIL_INT SizeX,
           SizeY;

   /* Allocate the MIL application. */
   MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);

   /* Allocate the MIL system. */
   MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   
   /* Load the source image of multiple profiles obtained using, 
      for example, Coherent StingRay structured light lasers. */
   MilImage = MbufRestore(EX_PATH(IMAGE_FILENAME), MilSystem, M_NULL);

   /* Display the source image. */
   MilDisplayImage = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   
   GraphicListImage = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   MdispControl(MilDisplayImage, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicListImage);
   
   MdispSelect(MilDisplayImage, MilImage);

   MosPrintf(MIL_TEXT("A source image of multiple laser line profiles\n") 
             MIL_TEXT("has been loaded and is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Smooth the image to reduce speckle noise. */
   MIL_ID MilLinearFilterIIRContext = MimAlloc(MilSystem, M_LINEAR_FILTER_IIR_CONTEXT, M_DEFAULT, M_NULL);
   MimControl(MilLinearFilterIIRContext, M_FILTER_SMOOTHNESS, 70);
   MimConvolve(MilImage, MilImage, MilLinearFilterIIRContext);
   MimFree(MilLinearFilterIIRContext);
   
   /* Allocate context for MimLocatePeak1D. */
   MilPeakContext = MimAlloc(MilSystem, M_LOCATE_PEAK_1D_CONTEXT, M_DEFAULT, M_NULL);

   /* Allocate result for MimLocatePeak1D. */
   MilPeakResult = MimAllocResult(MilSystem, M_DEFAULT, M_LOCATE_PEAK_1D_RESULT, M_NULL);

   /* Set the peak extraction parameters. */
   MimControl(MilPeakContext, M_MINIMUM_CONTRAST,   PeakMinContrast);
   MimControl(MilPeakContext, M_PEAK_WIDTH_NOMINAL, PeakWidthNominal);
   MimControl(MilPeakContext, M_PEAK_WIDTH_DELTA,   PeakWidthDelta);
   MimControl(MilPeakContext, M_NUMBER_OF_PEAKS,    NumberOfProfiles);

   /* Set the result sorting criteria in order of peak position in each lane. */
   MimControl(MilPeakResult, M_SORT_CRITERION, M_PEAK_POSITION);

   /* Locate the peaks in the smoothed image. */
   MimLocatePeak1d(MilPeakContext, MilImage, MilPeakResult, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Draw the located peaks in the source image. */
   MgraColor(M_DEFAULT, M_COLOR_RED);
   for (MIL_INT jj = 0; jj < 9; jj++)
      MimDraw(M_DEFAULT, MilPeakResult, M_NULL, GraphicListImage, M_DRAW_PEAKS + M_DOTS, (MIL_DOUBLE)jj, 1, M_DEFAULT);

   MosPrintf(MIL_TEXT("The source image has been smoothed to reduce speckle noise.\n")
             MIL_TEXT("The peaks have been located and are displayed in red.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Retrieving the source image sizes. */
   SizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   SizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   /* Allocate and display a buffer to draw the result of the gap and flush measurements. */
   MilAnalyse = MbufAlloc2d(MilSystem, SizeX, SizeY, 8, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MbufClear(MilAnalyse, 0);

   MilDisplayAnalyse = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);

   GraphicListAnalyse = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   MdispControl(MilDisplayAnalyse, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicListAnalyse);

   MdispSelect(MilDisplayAnalyse, MilAnalyse);
   
   /* Allocate a Metrology context. */
   MetContext = MmetAlloc(MilSystem, M_DEFAULT, M_NULL);

   /* Allocate a Metrology result. */
   MetResult = MmetAllocResult(MilSystem, M_DEFAULT, M_NULL);

   MIL_DOUBLE* pX = new MIL_DOUBLE[NumberOfProfiles * SizeX];
   MIL_DOUBLE* pY = new MIL_DOUBLE[NumberOfProfiles * SizeX];

   for (MIL_INT ii = 0; ii < NumberOfProfiles; ii++)
      {
      /* Display the peaks in red in the source image. */
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MimDraw(M_DEFAULT, MilPeakResult, M_NULL, GraphicListImage, M_DRAW_PEAKS + M_DOTS, M_ALL, 1, M_DEFAULT);

      /* Display the analyzed profile in green in the source image. */
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      MimDraw(M_DEFAULT, MilPeakResult, M_NULL, GraphicListImage, M_DRAW_PEAKS + M_DOTS, (MIL_DOUBLE)ii, 1, M_DEFAULT);

      /* Retrieve the number of peaks and the positions of the peaks. */
      MIL_INT NumberOfPeaks = 0;
      MimGetResultSingle(MilPeakResult, ii, M_ALL, M_NUMBER + M_TYPE_MIL_INT, &NumberOfPeaks);
      MimGetResultSingle(MilPeakResult, ii, M_ALL, M_PEAK_POSITION_X + M_TYPE_MIL_DOUBLE, pX);
      MimGetResultSingle(MilPeakResult, ii, M_ALL, M_PEAK_POSITION_Y + M_TYPE_MIL_DOUBLE, pY);

      /* Add the analyzed profile to the Metrology context. */
      const MIL_INT CurrentProfileLbl = 100;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_EDGEL, CurrentProfileLbl, M_EXTERNAL_FEATURE, M_NULL, M_NULL, 0, M_DEFAULT);
      MmetPut(MetContext, M_FEATURE_LABEL(CurrentProfileLbl), NumberOfPeaks, M_NULL, pX, pY, M_NULL, M_NULL, M_DEFAULT);
      MmetControl(MetContext, M_FEATURE_LABEL(CurrentProfileLbl), M_EDGEL_DENOISING_MODE, M_MEDIAN);
      MmetControl(MetContext, M_FEATURE_LABEL(CurrentProfileLbl), M_EDGEL_DENOISING_RADIUS, 10);

      /* Define features to measure the gap and flush. */
      /*************************************************/

      /* Add the reference segment. */
      const MIL_INT DatumSegmentLbl = 101;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_SEGMENT, DatumSegmentLbl, M_FIT, &CurrentProfileLbl, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_LABEL(DatumSegmentLbl), M_DEFAULT, M_RECTANGLE, 345, 200, 240, 725, M_NULL, M_NULL);
      MmetControl(MetContext, M_FEATURE_LABEL(DatumSegmentLbl), M_FIT_DISTANCE_MAX, 5.0);
      
      /* Add a constructed local frame at the center of the reference segment. */
      const MIL_INT DatumSegmentSystemLbl = 102;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_LOCAL_FRAME, DatumSegmentSystemLbl, M_CONSTRUCTION, &DatumSegmentLbl, M_NULL, 1, M_DEFAULT);

      /* Add a right segment relative to the reference segment local frame. */
      const MIL_INT RightSegmentLbl = 103;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_SEGMENT, RightSegmentLbl, M_FIT, &CurrentProfileLbl, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_LABEL(RightSegmentLbl), M_FEATURE_LABEL(DatumSegmentSystemLbl), M_RECTANGLE, 320, -250, 100, 100, 0, M_NULL);
      MmetControl(MetContext, M_FEATURE_LABEL(RightSegmentLbl), M_FIT_DISTANCE_MAX, 5.0);
      
      /* Add a left segment relative to the reference segment local frame. */
      const MIL_INT LeftSegmentLbl = 104;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_SEGMENT, LeftSegmentLbl, M_FIT, &CurrentProfileLbl, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_LABEL(LeftSegmentLbl), M_FEATURE_LABEL(DatumSegmentSystemLbl), M_RECTANGLE, -400, -250, 80, 100, 0, M_NULL);
      MmetControl(MetContext, M_FEATURE_LABEL(LeftSegmentLbl), M_FIT_DISTANCE_MAX, 5.0);

      /* Add the right constructed center point of the right segment. */
      const MIL_INT RightCenterLbl = 105;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_POINT, RightCenterLbl, M_CENTER, &RightSegmentLbl, M_NULL, 1, M_DEFAULT);

      /* Add the left constructed center point of the left segment. */
      const MIL_INT LeftCenterLbl = 106;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_POINT, LeftCenterLbl, M_CENTER, &LeftSegmentLbl, M_NULL, 1, M_DEFAULT);

      /* Add a right arc relative to the reference segment local frame. */
      const MIL_INT RightArcLbl = 107;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_ARC, RightArcLbl, M_FIT, &CurrentProfileLbl, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_LABEL(RightArcLbl), M_FEATURE_LABEL(DatumSegmentSystemLbl), M_RING_SECTOR, 310, -60, 130, 180, 90, 155);
      MmetControl(MetContext, M_FEATURE_LABEL(RightArcLbl), M_FIT_DISTANCE_MAX, 10.0);

      /* Add a left arc relative to the reference segment local frame. */
      const MIL_INT LeftArcLbl = 108;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_ARC, LeftArcLbl, M_FIT, &CurrentProfileLbl, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_LABEL(LeftArcLbl), M_FEATURE_LABEL(DatumSegmentSystemLbl), M_RING_SECTOR, -310, -65, 110, 140, 25, 90);
      MmetControl(MetContext, M_FEATURE_LABEL(LeftArcLbl), M_FIT_DISTANCE_MAX, 10.0);

      /* Define tolerance to measure the gap and flush. */
      /**************************************************/

      /* Add the measure of the Y position of the right segment center point 
         relative to the reference segment local frame. */
      const MIL_INT RightHeightLbl = 109;
      const MIL_INT RightHeightFeatures[2] = { DatumSegmentSystemLbl, RightCenterLbl };
      MmetAddTolerance(MetContext, M_POSITION_Y, RightHeightLbl, 10, 100, &RightHeightFeatures[0], M_NULL, 2, M_DEFAULT);

      /* Add the measure of the Y position of the left segment center point
      relative to the reference segment local frame. */
      const MIL_INT LeftHeightLbl = 110;
      const MIL_INT LeftHeightFeatures[2] = { DatumSegmentSystemLbl, LeftCenterLbl };
      MmetAddTolerance(MetContext, M_POSITION_Y, LeftHeightLbl, 10, 100, &LeftHeightFeatures[0], M_NULL, 2, M_DEFAULT);

      /* Add the measure of the gap size as the minimum distance between the two fitted arcs. */
      const MIL_INT GapLbl = 111;
      const MIL_INT GapFeatures[2] = { RightArcLbl, LeftArcLbl};
      MmetAddTolerance(MetContext, M_DISTANCE_MIN, GapLbl, 10, 100, &GapFeatures[0], M_NULL, 2, M_DEFAULT);
      
      /* Perform the gap and flush measurements. */
      MmetCalculate(MetContext, M_NULL, MetResult, M_DEFAULT);

      /* Clear the annotations. */
      MgraClear(M_DEFAULT, GraphicListAnalyse);

      /* Display the Metrology regions and features. */
      // Display the noisy edgels before the edgels smoothing.
      MgraColor(M_DEFAULT, M_COLOR_DARK_YELLOW);
      MmetDraw(M_DEFAULT, MetContext, GraphicListImage, M_DRAW_NOISY_EDGELS, M_FEATURE_LABEL(CurrentProfileLbl), M_DEFAULT);
      // Display the denoised edgels that we will use for fitting
      MgraColor(M_DEFAULT, M_COLOR_BLUE);
      MmetDraw(M_DEFAULT, MetContext, GraphicListImage, M_DRAW_FEATURE, M_FEATURE_LABEL(CurrentProfileLbl), M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_GRAY);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_FEATURE, M_FEATURE_LABEL(CurrentProfileLbl), M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_REGION, M_FEATURE_LABEL(DatumSegmentLbl), M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_DARK_RED);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_REGION, M_FEATURE_LABEL(RightSegmentLbl), M_DEFAULT);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_REGION, M_FEATURE_LABEL(LeftSegmentLbl),  M_DEFAULT);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_REGION, M_FEATURE_LABEL(LeftSegmentLbl),  M_DEFAULT);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_REGION, M_FEATURE_LABEL(RightArcLbl),     M_DEFAULT);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_REGION, M_FEATURE_LABEL(LeftArcLbl),      M_DEFAULT);
      
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_FEATURE, M_FEATURE_LABEL(DatumSegmentLbl), M_DEFAULT);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_FEATURE, M_FEATURE_LABEL(RightSegmentLbl), M_DEFAULT);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_FEATURE, M_FEATURE_LABEL(LeftSegmentLbl),  M_DEFAULT);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_FEATURE, M_FEATURE_LABEL(RightArcLbl),     M_DEFAULT);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_FEATURE, M_FEATURE_LABEL(LeftArcLbl),      M_DEFAULT);
      
      MgraColor(M_DEFAULT, M_COLOR_CYAN);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_FEATURE, M_FEATURE_LABEL(DatumSegmentSystemLbl), M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_RED);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_FEATURE, M_FEATURE_LABEL(RightCenterLbl), M_DEFAULT);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_FEATURE, M_FEATURE_LABEL(LeftCenterLbl), M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_YELLOW);
      MmetDraw(M_DEFAULT, MetResult, GraphicListAnalyse, M_DRAW_TOLERANCE, M_DEFAULT, M_DEFAULT);

      /* Retrieve and output the gap and flush measures. */
      MIL_DOUBLE GapDistanceValue = 0.0;
      MmetGetResult(MetResult, M_TOLERANCE_LABEL(GapLbl), M_TOLERANCE_VALUE + M_TYPE_DOUBLE, &GapDistanceValue);

      MIL_DOUBLE RightHeightValue = 0.0;
      MmetGetResult(MetResult, M_TOLERANCE_LABEL(RightHeightLbl), M_TOLERANCE_VALUE + M_TYPE_DOUBLE, &RightHeightValue);

      MIL_DOUBLE LeftHeightValue = 0.0;
      MmetGetResult(MetResult, M_TOLERANCE_LABEL(LeftHeightLbl), M_TOLERANCE_VALUE + M_TYPE_DOUBLE, &LeftHeightValue);

      MosPrintf(MIL_TEXT("Gap and Flush measures for the profile %d result:\n"), ii);
      MosPrintf(MIL_TEXT("\t- Gap distance : %.2f pixels.\n"), GapDistanceValue);
      MosPrintf(MIL_TEXT("\t- Right side height : %.2f pixels.\n"), -RightHeightValue);
      MosPrintf(MIL_TEXT("\t- Left side height : %.2f pixels.\n\n"), -LeftHeightValue);

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      /* Remove the profile feature and all its dependencies. */
      MmetControl(MetContext, M_FEATURE_LABEL(CurrentProfileLbl), M_DELETE, M_DEFAULT);
      }

   delete[] pX;
   delete[] pY;

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   // Release allocated resources.
   MbufFree(MilImage);
   MbufFree(MilAnalyse);
   MdispFree(MilDisplayImage);
   MdispFree(MilDisplayAnalyse);
   MgraFree(GraphicListImage);
   MgraFree(GraphicListAnalyse);

   MimFree(MilPeakContext);
   MimFree(MilPeakResult);

   MmetFree(MetContext);
   MmetFree(MetResult);

   // Free the MIL system and application.
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }
