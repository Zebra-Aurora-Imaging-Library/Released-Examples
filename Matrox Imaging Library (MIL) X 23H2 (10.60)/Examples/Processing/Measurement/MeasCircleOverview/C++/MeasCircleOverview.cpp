﻿//***************************************************************************************/
//
// File name: MeasCircleOverview.cpp
//
// Synopsis:  This program illustrates the circle marker concept
//            of the measurement module.
//            See the PrintHeader() function below for a detailed description.
//            
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include "MeasOverviewExample.h"


//***************************************************************************
// Example description.
//***************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("MeasCircleOverview\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program illustrates the circle marker concept\n")
             MIL_TEXT("of the measurement module.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, image processing, calibration, measurement.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//***************************************************************************
// Example images.
//***************************************************************************
#define EXAMPLE_CIRCLE_IMAGE_PATH M_IMAGE_PATH MIL_TEXT("CircleMeasurement/")
static MIL_CONST_TEXT_PTR IMAGE_FILE = EXAMPLE_CIRCLE_IMAGE_PATH MIL_TEXT("circle3.mim");
static MIL_CONST_TEXT_PTR MAX_ASSOCIATION_IMAGE_FILE = EXAMPLE_CIRCLE_IMAGE_PATH MIL_TEXT("circle2.mim");
static MIL_CONST_TEXT_PTR ACCURACY_IMAGE_FILE = EXAMPLE_CIRCLE_IMAGE_PATH MIL_TEXT("circle0.mim");

//***************************************************************************
// Example setup functions.
//***************************************************************************
static const MIL_DOUBLE NOISY_SMOOTHNESS = 100;
void SetupSmallestRadiusLowAccuracy(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Using multiple radial subregions in a ring search region, it is possible to\n")
             MIL_TEXT("find a circle with the measurement module. The circular boundaries of the ring\n")
             MIL_TEXT("search region are displayed in purple.\n")
             MIL_TEXT("In addition to the strength and contrast score, the required circle can be\n")
             MIL_TEXT("selected according to the radius score.\n")
             MIL_TEXT("The circle with the smallest radius is displayed in red.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_CIRCLE_ACCURACY, M_LOW, M_NULL);
   MmeasSetScore(MilMeasMarker, M_STRENGTH_SCORE, 0, 0, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MmeasSetScore(MilMeasMarker, M_RADIUS_SCORE, 0, 0, 0, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   }

static const MIL_DOUBLE NOISY_BEST_SMOOTHNESS = 95;
void SetupStrongestLowAccuracy(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The strongest circle is displayed in red.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_CIRCLE_ACCURACY, M_LOW, M_NULL);
   }

static const MIL_DOUBLE FALSE_SMOOTHNESS = 90;
void SetupNoMaxAssociationLowAccuracy(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The circle marker with the greatest number of subpositions is always selected\n")
             MIL_TEXT("as the circle marker found, even if it does not have the highest score. This\n")
             MIL_TEXT("can happen when there are subpositions that are outliers to the circle. To\n")
             MIL_TEXT("remove unwanted outlier subpositions, use the max association distance setting.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_CIRCLE_ACCURACY, M_LOW, M_NULL);
   }

static const MIL_DOUBLE MAX_ASSOCIATION_DISTANCE = 5;
void SetupMaxAssociationLowAccuracy(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The strongest circle found using a maximum association distance of %.f\n")
             MIL_TEXT("is displayed.\n\n"), MAX_ASSOCIATION_DISTANCE);
   MmeasSetMarker(MilMeasMarker, M_CIRCLE_ACCURACY, M_LOW, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_MAX_ASSOCIATION_DISTANCE, MAX_ASSOCIATION_DISTANCE, M_NULL);
   }

void SetupLowAccuracy(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("When the ring region is not centered on the circle, the intensity transition in\n")
             MIL_TEXT("each subregion profile is not perpendicular to the subregion which introduces\n")
             MIL_TEXT("position inaccuracy. To improve accuracy, additional refinement steps are\n")
             MIL_TEXT("performed by default to find more precise subpositions.\n")
             MIL_TEXT("The circle found with the circle's accuracy set to low is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_CIRCLE_ACCURACY, M_LOW, M_NULL);
   }

void SetupHighAccuracy(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The circle found with the circle's accuracy set to high(default) is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_CIRCLE_ACCURACY, M_HIGH, M_NULL);
   }

//*****************************************************************************
// Main
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the example.
   CMeasOverviewExample MeasExample(false);

   // Print the header.
   PrintHeader();

   // Marker regions used.                               CenterX, CenterY, InnerRadius, OuterRadius, ChordAngle
   const SMeasRegion CIRCLE_REGION                    = {    270,     240,          50,         200,         10};
   const SMeasRegion MAX_ASSOCIATION_DISTANCE_REGION  = {    260,     230,          20,         200,         10};
   const SMeasRegion ACCURACY_REGION                  = {    248,     262,          15,         150,         10};

   //                      Source image              , Marker type, Marker region                    , Setup function                   , Measurement list   , Draw list
   MeasExample.RunMeasCase(IMAGE_FILE                , M_CIRCLE   , CIRCLE_REGION                    , SetupSmallestRadiusLowAccuracy   , M_DEFAULT          , CIRCLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE                , M_CIRCLE   , CIRCLE_REGION                    , SetupStrongestLowAccuracy        , M_DEFAULT          , CIRCLE_DRAW_LIST);
   MeasExample.RunMeasCase(MAX_ASSOCIATION_IMAGE_FILE, M_CIRCLE   , MAX_ASSOCIATION_DISTANCE_REGION  , SetupNoMaxAssociationLowAccuracy , M_DEFAULT          , CIRCLE_DRAW_LIST);
   MeasExample.RunMeasCase(MAX_ASSOCIATION_IMAGE_FILE, M_CIRCLE   , MAX_ASSOCIATION_DISTANCE_REGION  , SetupMaxAssociationLowAccuracy   , M_DEFAULT          , CIRCLE_DRAW_LIST);
   MeasExample.RunMeasCase(ACCURACY_IMAGE_FILE       , M_CIRCLE   , ACCURACY_REGION                  , SetupLowAccuracy                 , M_DEFAULT          , CIRCLE_DRAW_LIST);
   MeasExample.RunMeasCase(ACCURACY_IMAGE_FILE       , M_CIRCLE   , ACCURACY_REGION                  , SetupHighAccuracy                , M_DEFAULT          , CIRCLE_DRAW_LIST);
   }
