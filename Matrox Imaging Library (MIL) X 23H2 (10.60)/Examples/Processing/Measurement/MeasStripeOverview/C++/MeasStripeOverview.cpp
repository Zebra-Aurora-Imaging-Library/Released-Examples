﻿//***************************************************************************************/
//
// File name: MeasStripeOverview.cpp
//
// Synopsis:  This program illustrates the stripe marker concept
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
             MIL_TEXT("MeasStripeOverview\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program illustrates the stripe marker concept\n")
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
static MIL_CONST_TEXT_PTR IMAGE_FILE = EXAMPLE_IMAGE_PATH MIL_TEXT("MetalPieceRotated.tif");

//***************************************************************************
// Example setup functions.
//***************************************************************************
static const MIL_DOUBLE INTRO_STRIPE_NB = 2;
static const MIL_DOUBLE INTRO_STRIPE_MIN_NB = 1;
void SetupStripeIntro(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("By combining two edge transitions, it is possible to get a stripe.\n")
             MIL_TEXT("The distance between a stripe's two exterior edges is its width.\n")
             MIL_TEXT("Stripes have a width and edge-inside selection criteria as well as\n")
             MIL_TEXT("additional polarity constraints.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_NUMBER, INTRO_STRIPE_NB, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER_MIN, INTRO_STRIPE_MIN_NB, M_NULL);
   }

void SetupPolarityPosSame(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("A stripe with a Positive-Same polarity is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_SAME);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, INTRO_STRIPE_NB, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER_MIN, INTRO_STRIPE_MIN_NB, M_NULL);
   }

void SetupPolarityPosOpposite(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Stripes with a Positive-Opposite polarity are displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_OPPOSITE);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, INTRO_STRIPE_NB, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER_MIN, INTRO_STRIPE_MIN_NB, M_NULL);
   }

void SetupPolarityNegSame(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("A stripe with a Negative-Same polarity is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_NEGATIVE, M_SAME);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, INTRO_STRIPE_NB, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER_MIN, INTRO_STRIPE_MIN_NB, M_NULL);
   }

void SetupPolarityNegOpposite(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("A stripe with a Negative-Opposite polarity is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_NEGATIVE, M_OPPOSITE);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, INTRO_STRIPE_NB, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER_MIN, INTRO_STRIPE_MIN_NB, M_NULL);
   }

static const MIL_DOUBLE STRIPE_FIRST_POLARITY = M_ANY;
static const MIL_DOUBLE STRIPE_SECOND_POLARITY = M_OPPOSITE;
static const MIL_DOUBLE INC_POS_X[] = {149, 214, 269};
static const MIL_DOUBLE INC_POS_Y[] = {156, 163, 168};
void SetupInclusionPoint0(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("An inclusion point, which must be inside or outside the stripe, can even\n")
             MIL_TEXT("further constrain the stripe that can be returned.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, STRIPE_FIRST_POLARITY, STRIPE_SECOND_POLARITY);
   MmeasSetMarker(MilMeasMarker, M_INCLUSION_POINT_INPUT_UNITS, M_WORLD, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_INCLUSION_POINT_INSIDE_STRIPE, M_YES, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_INCLUSION_POINT, INC_POS_X[0], INC_POS_Y[0]);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, INTRO_STRIPE_NB, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER_MIN, INTRO_STRIPE_MIN_NB, M_NULL);
   }

void SetupInclusionPoint1(MIL_ID MilMeasMarker)
   {
   MmeasSetMarker(MilMeasMarker, M_POLARITY, STRIPE_FIRST_POLARITY, STRIPE_SECOND_POLARITY);
   MmeasSetMarker(MilMeasMarker, M_INCLUSION_POINT_INPUT_UNITS, M_WORLD, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_INCLUSION_POINT_INSIDE_STRIPE, M_YES, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_INCLUSION_POINT, INC_POS_X[1], INC_POS_Y[1]);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, INTRO_STRIPE_NB, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER_MIN, INTRO_STRIPE_MIN_NB, M_NULL);
   }

void SetupInclusionPoint2(MIL_ID MilMeasMarker)
   {
   MmeasSetMarker(MilMeasMarker, M_POLARITY, STRIPE_FIRST_POLARITY, STRIPE_SECOND_POLARITY);
   MmeasSetMarker(MilMeasMarker, M_INCLUSION_POINT_INPUT_UNITS, M_WORLD, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_INCLUSION_POINT_INSIDE_STRIPE, M_YES, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_INCLUSION_POINT, INC_POS_X[2], INC_POS_Y[2]);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, INTRO_STRIPE_NB, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER_MIN, INTRO_STRIPE_MIN_NB, M_NULL);
   }

static const MIL_DOUBLE SCORE_STRIPE_FIRST_POLARITY = M_POSITIVE;
static const MIL_DOUBLE SCORE_STRIPE_SECOND_POLARITY = M_OPPOSITE;
static const MIL_DOUBLE THINNEST_STRIPE_NB = 1;
void SetupThinnest(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The width score can be used to select the required stripe.\n")
             MIL_TEXT("The thinnest stripe is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, SCORE_STRIPE_FIRST_POLARITY, SCORE_STRIPE_SECOND_POLARITY);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, THINNEST_STRIPE_NB, M_NULL);
   MmeasSetScore(MilMeasMarker, M_STRENGTH_SCORE, 0, 0, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MmeasSetScore(MilMeasMarker, M_STRIPE_WIDTH_SCORE, 0, 0, 0, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   }

void SetupWidestNoEdge(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The edge-inside score can be used to select the required stripe.\n")
             MIL_TEXT("The widest stripe, without any edge inside it, is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, SCORE_STRIPE_FIRST_POLARITY, SCORE_STRIPE_SECOND_POLARITY);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, THINNEST_STRIPE_NB, M_NULL);
   MmeasSetScore(MilMeasMarker, M_STRENGTH_SCORE, 0, 0, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MmeasSetScore(MilMeasMarker, M_STRIPE_WIDTH_SCORE, 0, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MmeasSetScore(MilMeasMarker, M_EDGE_INSIDE_SCORE, 0, 0, 0, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   }

void SetupThinnest2EdgesMin(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The thinnest stripe, with at least two edges inside it, is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, SCORE_STRIPE_FIRST_POLARITY, SCORE_STRIPE_SECOND_POLARITY);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, THINNEST_STRIPE_NB, M_NULL);
   MmeasSetScore(MilMeasMarker, M_STRENGTH_SCORE, 0, 0, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MmeasSetScore(MilMeasMarker, M_STRIPE_WIDTH_SCORE, 0, 0, 0, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MmeasSetScore(MilMeasMarker, M_EDGE_INSIDE_SCORE, 2, 2, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   }

//*****************************************************************************
// Main
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the example.
   CMeasOverviewExample MeasExample(true);

   // Print the header.
   PrintHeader();

   // Marker regions used.             CenterX, CenterY, SizeX, SizeY, Angle
   const SMeasRegion STRIPE_REGION = {     210,     173,   182,    34,   356};

   //                      Source image     , Marker type, MarkerRegion  , Setup function              , Measurement list    , Draw list
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupStripeIntro            , M_POSITION          , STRIPE_SIMPLE_DRAW_LIST   );
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupPolarityPosSame        , M_POSITION          , STRIPE_SIMPLE_DRAW_LIST   );
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupPolarityPosOpposite    , M_POSITION          , STRIPE_SIMPLE_DRAW_LIST   );
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupPolarityNegSame        , M_POSITION          , STRIPE_SIMPLE_DRAW_LIST   );
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupPolarityNegOpposite    , M_POSITION          , STRIPE_SIMPLE_DRAW_LIST   );
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupInclusionPoint0        , M_POSITION          , STRIPE_INCLUSION_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupInclusionPoint1        , M_POSITION          , STRIPE_INCLUSION_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupInclusionPoint2        , M_POSITION          , STRIPE_INCLUSION_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupThinnest               , M_POSITION          , STRIPE_SIMPLE_DRAW_LIST   );
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupWidestNoEdge           , M_POSITION          , STRIPE_SIMPLE_DRAW_LIST   );
   MeasExample.RunMeasCase(IMAGE_FILE       , M_STRIPE   , STRIPE_REGION , SetupThinnest2EdgesMin      , M_POSITION          , STRIPE_SIMPLE_DRAW_LIST   );
   }
