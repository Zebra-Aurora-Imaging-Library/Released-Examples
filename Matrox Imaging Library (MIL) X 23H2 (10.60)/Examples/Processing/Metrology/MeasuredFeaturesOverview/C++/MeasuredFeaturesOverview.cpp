﻿//***************************************************************************************/
//
// File name: MeasuredFeaturesOverview.cpp
//
// Synopsis:  This program demonstrates various MIL Metrology measured features.
//            See the PrintHeader() function below for detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("GeneralMetrology/") MIL_TEXT(x))
static MIL_CONST_TEXT_PTR s_ImageFilename = EX_PATH("MetalSheetWithHoles.mim");

//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("MeasuredFeaturesOverview\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to measure features with metrology.\n"));
   MosPrintf(MIL_TEXT("It also illustrates the regions of the features."));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, system, display, buffer, graphic,\n")
             MIL_TEXT("image processing and metrology.\n\n"));
   }

//****************************************************************************
void WaitForKey()
   {
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();
   }

//****************************************************************************
void FirstFeatureMeasurement(MIL_ID MilMetrolContext,
                             MIL_ID MilImage,
                             MIL_ID MilMetrolResult,
                             MIL_ID MilGraphicList)
   {
   MosPrintf(MIL_TEXT("======================================================================\n"));
   MosPrintf(MIL_TEXT(" Metrology offers many measured features that can be established with\n")
             MIL_TEXT(" a fit operation.\n")
             MIL_TEXT(" For each feature, a region must be set to define the area from which\n")
             MIL_TEXT(" to extract the edgels for the fit.\n\n")
             MIL_TEXT(" - In red (label #1), an example of a fitted segment.\n")
             MIL_TEXT(" - In green, a rectangular region that limits the area from which\n")
             MIL_TEXT("   the edgels are extracted.\n"));

   // Creation of a measured segment, which will be computed according to what can be found on the image.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_SEGMENT, M_FEATURE_LABEL(1), M_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);

   // Definition of its region (image area to extract edgels).
   const MIL_DOUBLE FirstFeatureRegionPositionX = 365;
   const MIL_DOUBLE FirstFeatureRegionPositionY = 169;
   const MIL_DOUBLE FirstFeatureRegionWidth     =  45;
   const MIL_DOUBLE FirstFeatureRegionHeight    =  15;
   const MIL_DOUBLE FirstFeatureRegionAngle     =  50;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(1), M_DEFAULT, M_RECTANGLE,
                 FirstFeatureRegionPositionX, FirstFeatureRegionPositionY,
                 FirstFeatureRegionWidth, FirstFeatureRegionHeight,
                 FirstFeatureRegionAngle, M_NULL);

   // Compute and draw the results.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Draw the computed segment in red.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(1), M_DEFAULT);

   // Display the used region in green.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(1), M_DEFAULT);
   WaitForKey();

   // Clear annotations.
   MgraClear(M_DEFAULT, MilGraphicList);
   }

//****************************************************************************
// Illustrations basic properties of regions use.
void PropertiesOfRegions(MIL_ID MilMetrolContext,
                         MIL_ID MilImage,
                         MIL_ID MilMetrolResult,
                         MIL_ID MilGraphicList,
                         MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("======================================================================\n"));
   MosPrintf(MIL_TEXT(" The region orientation defines the relative angle criteria used to\n")
             MIL_TEXT(" select a subset of edgels on which to fit the feature.\n")
             MIL_TEXT(" - In yellow, the fitted segment feature that is extracted using only\n")
             MIL_TEXT("   the edgels that follow the orientation of the region.\n")
             MIL_TEXT(" - In magenta, the fitted segment feature that is extracted using only\n")
             MIL_TEXT("   the edgels that follow the opposite orientation of the region.\n\n")
             MIL_TEXT(" Recall that the angle of an edgel is perpendicular to the edge and\n")
             MIL_TEXT(" points in the direction going from dark to bright pixels.\n"));

   // Create segments from a different edgels orientation.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_SEGMENT, M_FEATURE_LABEL(2), M_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);

   const MIL_DOUBLE SecondFeatureRegionPositionX = 394;
   const MIL_DOUBLE SecondFeatureRegionPositionY = 302;
   const MIL_DOUBLE SecondFeatureRegionWidth     =  28;
   const MIL_DOUBLE SecondFeatureRegionHeight    =  21;
   const MIL_DOUBLE SecondFeatureRegionAngle     = 318;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(2), M_DEFAULT, M_RECTANGLE,
                 SecondFeatureRegionPositionX, SecondFeatureRegionPositionY,
                 SecondFeatureRegionWidth, SecondFeatureRegionHeight,
                 SecondFeatureRegionAngle, M_NULL);
   
   // Extraction of edgels having the same orientation than the region (default behavior).
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(2), M_EDGEL_RELATIVE_ANGLE, M_SAME);
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_SEGMENT, M_FEATURE_LABEL(3), M_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(3), M_DEFAULT, M_RECTANGLE,
                 SecondFeatureRegionPositionX, SecondFeatureRegionPositionY,
                 SecondFeatureRegionWidth, SecondFeatureRegionHeight,
                 SecondFeatureRegionAngle, M_NULL);
   
   // Extraction of edgels having the opposite orientation than the region.
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(3), M_EDGEL_RELATIVE_ANGLE, M_REVERSE);

   // Calculate and draw the features.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(2), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(2), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(3), M_DEFAULT);
   WaitForKey();

   MosPrintf(MIL_TEXT("======================================================================\n"));
   MosPrintf(MIL_TEXT(" Zoom into the display to see the region and the fitted segments.\n"));

   MdispPan(MilDisplay, 325.0, 275.0);
   MdispZoom(MilDisplay, 6.0, 6.0);
   WaitForKey();

   MosPrintf(MIL_TEXT("======================================================================\n"));
   MosPrintf(MIL_TEXT(" The 'active' edgels, which are considered for the fit, are extracted\n")
             MIL_TEXT(" inside the region.\n")
             MIL_TEXT(" - In yellow, the active edgels that follow the orientation of the\n")
             MIL_TEXT("   region (same).\n")
             MIL_TEXT(" - In magenta, the active edgels that follow the opposite orientation\n")
             MIL_TEXT("   of the region (reverse).\n")
             MIL_TEXT(" Edgel orientation determines the behavior of the fit operation.\n"));

   // Clear annotations.
   MgraClear(M_DEFAULT, MilGraphicList);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(2), M_DEFAULT);
   // Draw the fitted edgels with the different orientations, using the same color legend than before.
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_ACTIVE_EDGELS, M_FEATURE_LABEL(2), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_ACTIVE_EDGELS, M_FEATURE_LABEL(3), M_DEFAULT);

   WaitForKey();

   // Resetting the zoom.
   MdispPan(MilDisplay, 0.0, 0.0);
   MdispZoom(MilDisplay, 1.0, 1.0);
   }

//****************************************************************************
// An illustration of the diversity of available measured features
// in MIL Metrology.
void DiversityOfMeasuredFeatures(MIL_ID MilMetrolContext,
                                 MIL_ID MilImage,
                                 MIL_ID MilMetrolResult,
                                 MIL_ID MilGraphicList,
                                 MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("======================================================================\n"));
   MosPrintf(MIL_TEXT(" Beside segments, several other features can be established with\n a fit operation.\n"));
   MosPrintf(MIL_TEXT(" - Circles, arcs, points and edgels.\n"));

   // Clear annotations.
   MgraClear(M_DEFAULT, MilGraphicList);

   // Add a measured circle.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_CIRCLE, M_FEATURE_LABEL(4), M_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE CircleFeatureRingRegionPositionX   = 190;
   const MIL_DOUBLE CircleFeatureRingRegionPositionY   = 250;
   const MIL_DOUBLE CircleFeatureRingRegionInnerRadius =  17;
   const MIL_DOUBLE CircleFeatureRingRegionOuterRadius =  27;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(4), M_DEFAULT, M_RING,
                 CircleFeatureRingRegionPositionX, CircleFeatureRingRegionPositionY,
                 CircleFeatureRingRegionInnerRadius, CircleFeatureRingRegionOuterRadius,
                 M_NULL, M_NULL);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(4), M_EDGEL_RELATIVE_ANGLE, M_REVERSE);
   
   // Add a measured arc.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_ARC, M_FEATURE_LABEL(5), M_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE ArcFeatureRingSectorRegionPositionX   = 377;
   const MIL_DOUBLE ArcFeatureRingSectorRegionPositionY   = 320;
   const MIL_DOUBLE ArcFeatureRingSectorRegionInnerRadius =  15;
   const MIL_DOUBLE ArcFeatureRingSectorRegionOuterRadius =  25;
   const MIL_DOUBLE ArcFeatureRingSectorRegionStartAngle  =  60;
   const MIL_DOUBLE ArcFeatureRingSectorRegionEndAngle    = 220;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(5), M_DEFAULT, M_RING_SECTOR,
                 ArcFeatureRingSectorRegionPositionX, ArcFeatureRingSectorRegionPositionY,
                 ArcFeatureRingSectorRegionInnerRadius, ArcFeatureRingSectorRegionOuterRadius,
                 ArcFeatureRingSectorRegionStartAngle, ArcFeatureRingSectorRegionEndAngle);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(5), M_EDGEL_RELATIVE_ANGLE, M_REVERSE);

   // Another measured arc.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_ARC, M_FEATURE_LABEL(6), M_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE SecondArcFeatureRingSectorRegionPositionX   = 295;
   const MIL_DOUBLE SecondArcFeatureRingSectorRegionPositionY   = 250;
   const MIL_DOUBLE SecondArcFeatureRingSectorRegionInnerRadius = 122;
   const MIL_DOUBLE SecondArcFeatureRingSectorRegionOuterRadius = 147;
   const MIL_DOUBLE SecondArcFeatureRingSectorRegionStartAngle  = 240;
   const MIL_DOUBLE SecondArcFeatureRingSectorRegionEndAngle    = 305;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(6), M_DEFAULT, M_RING_SECTOR,
                 SecondArcFeatureRingSectorRegionPositionX, SecondArcFeatureRingSectorRegionPositionY,
                 SecondArcFeatureRingSectorRegionInnerRadius, SecondArcFeatureRingSectorRegionOuterRadius,
                 SecondArcFeatureRingSectorRegionStartAngle, SecondArcFeatureRingSectorRegionEndAngle);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(6), M_EDGEL_ANGLE_RANGE, 10.0);

   // Add measured edgels.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_EDGEL, M_FEATURE_LABEL(7), M_DEFAULT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE EdgelFeatureRectangleRegionPositionX = 352;
   const MIL_DOUBLE EdgelFeatureRectangleRegionPositionY = 302;
   const MIL_DOUBLE EdgelFeatureRectangleRegionWidth     =  30;
   const MIL_DOUBLE EdgelFeatureRectangleRegionHeight    =  70;
   const MIL_DOUBLE EdgelFeatureRectangleRegionAngle     = 180;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(7), M_DEFAULT, M_RECTANGLE,
                 EdgelFeatureRectangleRegionPositionX, EdgelFeatureRectangleRegionPositionY,
                 EdgelFeatureRectangleRegionWidth, EdgelFeatureRectangleRegionHeight,
                 EdgelFeatureRectangleRegionAngle, M_NULL);
   
   // Sets the angular range for the extraction of the edgels.
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(7), M_EDGEL_RELATIVE_ANGLE, M_SAME_OR_REVERSE);
   const MIL_DOUBLE EdgelsAppertureAngle = 180.0;
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(7), M_EDGEL_ANGLE_RANGE, EdgelsAppertureAngle);

   // Add a measured point.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_POINT, M_FEATURE_LABEL(8), M_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE PointFeatureSegmentRegionStartPositionX = 460;
   const MIL_DOUBLE PointFeatureSegmentRegionStartPositionY = 245;
   const MIL_DOUBLE PointFeatureSegmentRegionEndPositionX   = 405;
   const MIL_DOUBLE PointFeatureSegmentRegionEndPositionY   = 245;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(8), M_DEFAULT, M_SEGMENT,
                 PointFeatureSegmentRegionStartPositionX, PointFeatureSegmentRegionStartPositionY,
                 PointFeatureSegmentRegionEndPositionX, PointFeatureSegmentRegionEndPositionY,
                 M_NULL, M_NULL);

   // Calculate and show results.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(1), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(4), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(5), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(6), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE,                M_FEATURE_LABEL(7), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(8), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(1), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(4), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(5), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(6), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(7), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(8), M_DEFAULT);
   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
   WaitForKey();
   }

//****************************************************************************
// And some extra regions available for these features.
void RemainingPossibleRegionsForFeatures(MIL_ID MilMetrolContext,
                                         MIL_ID MilImage,
                                         MIL_ID MilMetrolResult,
                                         MIL_ID MilGraphicList)
   {
   // Clear annotations.
   MgraClear(M_DEFAULT, MilGraphicList);

   MosPrintf(MIL_TEXT("======================================================================\n"));
   MosPrintf(MIL_TEXT(" Different types of regions are available for a given feature:\n"));
   MosPrintf(MIL_TEXT(" - Rectangles, rings, ring sectors and arcs.\n"));

   // Show measured edgels with different regions.
   // First, a rectangle region.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_EDGEL, M_FEATURE_LABEL(9), M_DEFAULT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE SecondEdgelFeatureRectangleRegionPositionX = 251;
   const MIL_DOUBLE SecondEdgelFeatureRectangleRegionPositionY = 204;
   const MIL_DOUBLE SecondEdgelFeatureRectangleRegionWidth     =   7;
   const MIL_DOUBLE SecondEdgelFeatureRectangleRegionHeight    =  40;
   const MIL_DOUBLE SecondEdgelFeatureRectangleRegionAngle     =   0;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(9), M_DEFAULT, M_RECTANGLE,
                 SecondEdgelFeatureRectangleRegionPositionX, SecondEdgelFeatureRectangleRegionPositionY,
                 SecondEdgelFeatureRectangleRegionWidth, SecondEdgelFeatureRectangleRegionHeight,
                 SecondEdgelFeatureRectangleRegionAngle, M_NULL);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(9), M_EDGEL_RELATIVE_ANGLE, M_SAME_OR_REVERSE);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(9), M_EDGEL_ANGLE_RANGE, 180.0);

   // Second, a ring sector region.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_EDGEL, M_FEATURE_LABEL(10), M_DEFAULT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE ThirdEdgelFeatureRingSectorRegionPositionX   = 313;
   const MIL_DOUBLE ThirdEdgelFeatureRingSectorRegionPositionY   = 225;
   const MIL_DOUBLE ThirdEdgelFeatureRingSectorRegionInnerRadius =  15;
   const MIL_DOUBLE ThirdEdgelFeatureRingSectorRegionOuterRadius =  27;
   const MIL_DOUBLE ThirdEdgelFeatureRingSectorRegionStartAngle  =  20;
   const MIL_DOUBLE ThirdEdgelFeatureRingSectorRegionEndAngle    = 160;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(10), M_DEFAULT, M_RING_SECTOR,
                 ThirdEdgelFeatureRingSectorRegionPositionX, ThirdEdgelFeatureRingSectorRegionPositionY,
                 ThirdEdgelFeatureRingSectorRegionInnerRadius, ThirdEdgelFeatureRingSectorRegionOuterRadius,
                 ThirdEdgelFeatureRingSectorRegionStartAngle, ThirdEdgelFeatureRingSectorRegionEndAngle);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_EDGEL_RELATIVE_ANGLE, M_SAME_OR_REVERSE);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_EDGEL_ANGLE_RANGE, 180.0);

   // Third, a ring region.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_EDGEL, M_FEATURE_LABEL(11), M_DEFAULT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE FourthEdgelFeatureRingRegionPositionX   = 296;
   const MIL_DOUBLE FourthEdgelFeatureRingRegionPositionY   = 247;
   const MIL_DOUBLE FourthEdgelFeatureRingRegionInnerRadius =  63;
   const MIL_DOUBLE FourthEdgelFeatureRingRegionOuterRadius =  72;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(11), M_DEFAULT, M_RING,
                 FourthEdgelFeatureRingRegionPositionX, FourthEdgelFeatureRingRegionPositionY,
                 FourthEdgelFeatureRingRegionInnerRadius, FourthEdgelFeatureRingRegionOuterRadius,
                 M_NULL, M_NULL);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(11), M_EDGEL_RELATIVE_ANGLE, M_SAME_OR_REVERSE);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(11), M_EDGEL_ANGLE_RANGE, 180.0);

   // The segment feature with a ring sector region (the previous one was with a rectangle region).
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_SEGMENT, M_FEATURE_LABEL(12), M_DEFAULT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE SegmentFeatureRingSectorRegionPositionX   = 295;
   const MIL_DOUBLE SegmentFeatureRingSectorRegionPositionY   = 247;
   const MIL_DOUBLE SegmentFeatureRingSectorRegionInnerRadius = 105;
   const MIL_DOUBLE SegmentFeatureRingSectorRegionOuterRadius = 135;
   const MIL_DOUBLE SegmentFeatureRingSectorRegionStartAngle  = 215;
   const MIL_DOUBLE SegmentFeatureRingSectorRegionEndAngle    = 230;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(12), M_DEFAULT, M_RING_SECTOR,
                 SegmentFeatureRingSectorRegionPositionX, SegmentFeatureRingSectorRegionPositionY,
                 SegmentFeatureRingSectorRegionInnerRadius, SegmentFeatureRingSectorRegionOuterRadius,
                 SegmentFeatureRingSectorRegionStartAngle, SegmentFeatureRingSectorRegionEndAngle);
   
   // The constructed point with the arc sector region.
   // Shows how to fetch two points at the same time.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_POINT, M_FEATURE_LABEL(13), M_DEFAULT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE PointFeatureArcRegionPositionX  = 297;
   const MIL_DOUBLE PointFeatureArcRegionPositionY  = 247;
   const MIL_DOUBLE PointFeatureArcRegionRadius     = 132;
   const MIL_DOUBLE PointFeatureArcRegionStartAngle = 125;
   const MIL_DOUBLE PointFeatureArcRegionEndAngle   = 155;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(13), M_DEFAULT, M_ARC,
                 PointFeatureArcRegionPositionX, PointFeatureArcRegionPositionY,
                 PointFeatureArcRegionRadius, PointFeatureArcRegionStartAngle, PointFeatureArcRegionEndAngle, M_NULL);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(13), M_NUMBER_MAX, 2);

   // Compute and draw results.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(9),  M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(10), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(11), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(12), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(13), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(9),  M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(10), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(11), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(12), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(13), M_DEFAULT);

   WaitForKey();
   }

//****************************************************************************
// Introduction to derived regions.
void DerivedMetrologyRegion(MIL_ID MilMetrolContext, MIL_ID  MilImage, MIL_ID  MilMetrolResult, MIL_ID  MilGraphicList)
   {
   MosPrintf(MIL_TEXT("======================================================================\n"));
   MosPrintf(MIL_TEXT(" A region can be computed relatively to one or many\n")
             MIL_TEXT(" calculated feature(s) (in magenta).\n")
             MIL_TEXT(" This region is then known as a derived region (in green).\n"));

   // Add a measured point
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_POINT, M_FEATURE_LABEL(20), M_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE PointFeatureFirstSegmentRegionStartPositionX = 190;
   const MIL_DOUBLE PointFeatureFirstSegmentRegionStartPositionY = 215;
   const MIL_DOUBLE PointFeatureFirstSegmentRegionEndPositionX   = 190;
   const MIL_DOUBLE PointFeatureFirstSegmentRegionEndPositionY   = 250;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(20), M_DEFAULT, M_SEGMENT,
                 PointFeatureFirstSegmentRegionStartPositionX, PointFeatureFirstSegmentRegionStartPositionY,
                 PointFeatureFirstSegmentRegionEndPositionX, PointFeatureFirstSegmentRegionEndPositionY,
                 M_NULL, M_NULL);

   // Add a measured point
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_POINT, M_FEATURE_LABEL(21), M_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   const MIL_DOUBLE PointFeatureSecondSegmentRegionStartPositionX = 112;
   const MIL_DOUBLE PointFeatureSecondSegmentRegionStartPositionY = 280;
   const MIL_DOUBLE PointFeatureSecondSegmentRegionEndPositionX   = 112;
   const MIL_DOUBLE PointFeatureSecondSegmentRegionEndPositionY   = 250;
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(21), M_DEFAULT, M_SEGMENT,
                 PointFeatureSecondSegmentRegionStartPositionX, PointFeatureSecondSegmentRegionStartPositionY,
                 PointFeatureSecondSegmentRegionEndPositionX, PointFeatureSecondSegmentRegionEndPositionY,
                 M_NULL, M_NULL);

   // Allocate the derived geometry region.
   MIL_ID derivedGeometryId = MmetAlloc(M_DEFAULT_HOST, M_DERIVED_GEOMETRY_REGION, M_NULL);
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_EDGEL, M_FEATURE_LABEL(22), M_DEFAULT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   
   // Modify the derived geometry region.
   // Choose the region type.
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_GEOMETRY, M_RING_SECTOR);
   
   // Provide its position based on a point.
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION_TYPE, M_FEATURE_LABEL_VALUE);
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION, M_FEATURE_LABEL(8));
   
   // Provide the start and end radii based on points
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START_TYPE, M_FEATURE_LABEL_VALUE);
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START, M_FEATURE_LABEL(20));
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END_TYPE, M_FEATURE_LABEL_VALUE);
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END, M_FEATURE_LABEL(21));

   // The remaining controls are set parametrically.
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_START_TYPE,   M_PARAMETRIC);
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_START,        170);
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_END_TYPE,     M_PARAMETRIC);
   MmetControl(derivedGeometryId, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_END,          190);

   // Associate the derived region geometry of the figure.
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(22), M_DEFAULT, M_FROM_DERIVED_GEOMETRY_REGION, derivedGeometryId, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL);

   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Draw results of interest.
   MgraClear(M_DEFAULT, MilGraphicList);
   MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(8), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(20), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(21), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(22), M_DEFAULT);

   MmetFree(derivedGeometryId);
   }

//****************************************************************************
int MosMain()
   {
   PrintHeader();

   MIL_ID MilApplication = MappAlloc(M_DEFAULT, M_NULL);
   MIL_ID MilSystem = M_DEFAULT_HOST;
   MIL_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MIL_ID MilGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   MIL_ID MilImage = M_NULL;
   MbufImport(s_ImageFilename, M_MIL_TIFF, M_RESTORE, M_DEFAULT_HOST, &MilImage);
   MdispSelect(MilDisplay, MilImage);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   MosPrintf(MIL_TEXT("======================================================================\n"));
   MosPrintf(MIL_TEXT(" To measure features, the first step is to create a context.\n")      
             MIL_TEXT(" All features and their measuring properties will be defined \n")
             MIL_TEXT(" in this context.\n")
             MIL_TEXT(" The context can be saved and restored from disk or memory.\n"));

   // Allocation of the context object to contain the features definition.
   MIL_ID MilMetrolContext;
   MmetAlloc(MilSystem, M_CONTEXT, &MilMetrolContext);

   // Allocation of the result object to contain all the computed features.
   MIL_ID MilMetrolResult;
   MmetAllocResult(MilSystem, M_DEFAULT, &MilMetrolResult);
   
   WaitForKey();

   FirstFeatureMeasurement            (MilMetrolContext, MilImage, MilMetrolResult, MilGraphicList);
   PropertiesOfRegions                (MilMetrolContext, MilImage, MilMetrolResult, MilGraphicList, MilDisplay);
   DiversityOfMeasuredFeatures        (MilMetrolContext, MilImage, MilMetrolResult, MilGraphicList, MilDisplay);
   RemainingPossibleRegionsForFeatures(MilMetrolContext, MilImage, MilMetrolResult, MilGraphicList);
   DerivedMetrologyRegion             (MilMetrolContext, MilImage, MilMetrolResult, MilGraphicList);

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n"));
   MosGetch();

   MmetFree(MilMetrolContext);
   MmetFree(MilMetrolResult);
   MgraFree(MilGraphicList);
   MbufFree(MilImage);
   MdispFree(MilDisplay);
   if (MilSystem != M_DEFAULT_HOST)
      { MsysFree(MilSystem); }

   MappFree(MilApplication);

   return 0;
   }
