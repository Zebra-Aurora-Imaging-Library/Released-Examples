//***************************************************************************************/
//
// File name: MeasProfileFilteringOverview.cpp
//
// Synopsis:  This program illustrates the profile filtering
//            concept of the measurement module.
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
             MIL_TEXT("MeasProfileFilteringOverview\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program illustrates the profile filtering concept\n")
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
static MIL_CONST_TEXT_PTR IMAGE_FILE = EXAMPLE_IMAGE_PATH MIL_TEXT("MetalPieceRotated2.tif");
static MIL_CONST_TEXT_PTR IMAGE_NOISE_FILE = EXAMPLE_IMAGE_PATH MIL_TEXT("MetalPieceRotatedNoisy.tif");
static MIL_CONST_TEXT_PTR IMAGE_EDGE_DISPLACEMENT_FILE = EXAMPLE_IMAGE_PATH MIL_TEXT("MetalPieceRotatedThinned.tif");
static MIL_CONST_TEXT_PTR IMAGE_NO_EDGE_DISPLACEMENT_FILE = EXAMPLE_IMAGE_PATH MIL_TEXT("MetalPiece.tif");

//***************************************************************************
// Examples setup functions.
//***************************************************************************
void SetupNoisyProblem(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Use a Shen filter to find an accurate position in a noisy image or\n")
             MIL_TEXT("noisy contour.\n")
             MIL_TEXT("A transition that cannot be found with an Euler filter is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   }

static const MIL_DOUBLE NOISY_SMOOTHNESS = 100;
void SetupNoisySolution(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Using a Shen filter with its smoothness set to a high value helps to\n")
             MIL_TEXT("distinguish the transition.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_SMOOTHNESS, NOISY_SMOOTHNESS, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   }

static const MIL_DOUBLE NOISY_BEST_SMOOTHNESS = 95;
void SetupNoisyBestSolution(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("For optimal results, specify a taller search region.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_SMOOTHNESS, NOISY_BEST_SMOOTHNESS, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   }

static const MIL_DOUBLE FALSE_EDGEVALUE_MIN = 6;
void SetupFalseProblem(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Use a Shen filter to attenuate (false) transitions coming from thin objects.\n")
             MIL_TEXT("As displayed, the transition coming from the large object of interest cannot\n")
             MIL_TEXT("be distinguished from the (false) transition coming from a thin object,\n")
             MIL_TEXT("which is also present in the search region.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_NUMBER, M_ALL, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_EDGEVALUE_MIN, FALSE_EDGEVALUE_MIN, M_NULL);
   }

static const MIL_DOUBLE FALSE_SMOOTHNESS = 90;
void SetupFalseSolution(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Using a Shen filter with its smoothness set to a high value helps to \n")
             MIL_TEXT("distinguish the transition of interest.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_SMOOTHNESS, FALSE_SMOOTHNESS, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, M_ALL, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_EDGEVALUE_MIN, FALSE_EDGEVALUE_MIN, M_NULL);
   }

void SetupAngleErrorProblem(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Use a Shen filter to reduce the effect of the angular error with a tall region.\n")
             MIL_TEXT("A profile where the transition position is uncertain is displayed.\n")
             MIL_TEXT("The uncertainty results in a plateau, caused by using a filter with a small\n")
             MIL_TEXT("kernel size.\n\n"));
   }

static const MIL_DOUBLE ANGLE_ERROR_SMOOTHNESS = 90;
void SetupAngleErrorSolution(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Using a Shen filter with its smoothness set to a high value stabilizes\n")
             MIL_TEXT("the position.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_SMOOTHNESS, ANGLE_ERROR_SMOOTHNESS, M_NULL);
   }

static const MIL_DOUBLE EDGE_DISPLACEMENT_NB = 2;
void SetupEdgeDisplacementEuler(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Use high smoothness with care. Other strong transitional data present in the\n")
             MIL_TEXT("region supported by the filter, such as an edge with opposite polarity, will\n")
             MIL_TEXT("cause edge displacement.\n")
             MIL_TEXT("An edge position, established by using an Euler filter, is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_EULER, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, EDGE_DISPLACEMENT_NB, M_NULL);
   }

void SetupEdgeDisplacement50(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The edge position, established by using a Shen filter with its smoothness\n")
             MIL_TEXT("set to 50, is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_SMOOTHNESS, 50, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, EDGE_DISPLACEMENT_NB, M_NULL);
   }

void SetupEdgeDisplacement90(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The edge position, established by using a Shen filter with its smoothness\n")
             MIL_TEXT("set to 90, is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_SMOOTHNESS, 90, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, EDGE_DISPLACEMENT_NB, M_NULL);
   }

void SetupNoEdgeDisplacementEuler(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Another edge position, established by using an Euler filter, is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_EULER, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, EDGE_DISPLACEMENT_NB, M_NULL);
   }

void SetupNoEdgeDisplacement90(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The edge position, established by using a Shen filter with its smoothness\n")
             MIL_TEXT("set to 90, is displayed.\n")
             MIL_TEXT("The size of the object is big enough to avoid edge displacement.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_SMOOTHNESS, 90, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, EDGE_DISPLACEMENT_NB, M_NULL);
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the example.
   CMeasOverviewExample MeasExample(true);

   // Print the header.
   PrintHeader();

   // Marker regions used.                             CenterX, CenterY, SizeX, SizeY, Angle
   const SMeasRegion NOISY_TRANSITION_PROBLEM       = {    227,     115,    92,     8,   266};
   const SMeasRegion NOISY_TRANSITION_BEST_SOLUTION = {    227,     115,    92,    92,   266};
   const SMeasRegion FALSE_TRANSITION_PROBLEM       = {    363,     116,   125,     8,   271};
   const SMeasRegion ANGLE_ERROR_PROBLEM            = {    133,     179,    54,    74,     3};
   const SMeasRegion EDGE_DISPLACEMENT              = {    269,     175,    71,    29,     0};

   //                      Source image                    , Marker type, Marker region                  , Setup function              , Measurement list   , Draw list
   MeasExample.RunMeasCase(IMAGE_NOISE_FILE                , M_EDGE     , NOISY_TRANSITION_PROBLEM       , SetupNoisyProblem           , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_NOISE_FILE                , M_EDGE     , NOISY_TRANSITION_PROBLEM       , SetupNoisySolution          , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_NOISE_FILE                , M_EDGE     , NOISY_TRANSITION_BEST_SOLUTION , SetupNoisyBestSolution      , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE                      , M_EDGE     , FALSE_TRANSITION_PROBLEM       , SetupFalseProblem           , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE                      , M_EDGE     , FALSE_TRANSITION_PROBLEM       , SetupFalseSolution          , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE                      , M_EDGE     , ANGLE_ERROR_PROBLEM            , SetupAngleErrorProblem      , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE                      , M_EDGE     , ANGLE_ERROR_PROBLEM            , SetupAngleErrorSolution     , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_EDGE_DISPLACEMENT_FILE    , M_EDGE     , EDGE_DISPLACEMENT              , SetupEdgeDisplacementEuler  , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_EDGE_DISPLACEMENT_FILE    , M_EDGE     , EDGE_DISPLACEMENT              , SetupEdgeDisplacement50     , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_EDGE_DISPLACEMENT_FILE    , M_EDGE     , EDGE_DISPLACEMENT              , SetupEdgeDisplacement90     , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_NO_EDGE_DISPLACEMENT_FILE , M_EDGE     , EDGE_DISPLACEMENT              , SetupNoEdgeDisplacementEuler, M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_NO_EDGE_DISPLACEMENT_FILE , M_EDGE     , EDGE_DISPLACEMENT              , SetupNoEdgeDisplacement90   , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   }
