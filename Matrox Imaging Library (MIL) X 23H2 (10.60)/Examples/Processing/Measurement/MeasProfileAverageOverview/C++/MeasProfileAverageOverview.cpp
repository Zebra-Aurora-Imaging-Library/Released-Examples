//***************************************************************************************/
//
// File name: MeasProfileAverageOverview.cpp
//
// Synopsis:  This program illustrates the profile averaging 
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
             MIL_TEXT("MeasProfileAverageOverview\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program illustrates the projection concept\n")
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
static MIL_CONST_TEXT_PTR IMAGE_NOISE_FILE = EXAMPLE_IMAGE_PATH MIL_TEXT("MetalPieceRotatedNoisy.tif");

//***************************************************************************
// Example setup functions.
//***************************************************************************
void SetupNoiseProblem(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Profile averaging helps to find an accurate position in a noisy image\n")
             MIL_TEXT("or noisy contour.\n")
             MIL_TEXT("A transition that cannot be found within a small region is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   }
void SetupNoiseSolution(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("By increasing the size of the region, you get a more average profile.\n")
             MIL_TEXT("The transition can now be extracted from the average profile.\n\n"));
   }

void SetupFalseTransitionProblem(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("A small, slightly misplaced, search region containing little objetcs\n")
             MIL_TEXT("can lead to finding an incorrect transition.\n")
             MIL_TEXT("Profile averaging helps to find the correct position by reducing the \n")
             MIL_TEXT("influence of minor (false) transitions.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   }

void SetupFalseTransitionSolution(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("By increasing the size of the region, the required transition is now\n")
             MIL_TEXT("the only valid transition.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   }

void SetupAngleErrorProblem(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("A taller search region is more sensitive to angular error.\n")
             MIL_TEXT("This results in peak strength attenuation and positional uncertainty.\n\n"));
   }

void SetupAngleErrorSolution(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("A better alignement is then necessary to ensure an accurate measurement.\n\n"));
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

   // Marker regions used.                         CenterX, CenterY, SizeX, SizeY, Angle
   const SMeasRegion NOISY_TRANSITION_PROBLEM   = {    227,     115,    92,     8,   266};
   const SMeasRegion NOISY_TRANSITION_SOLUTION  = {    227,     115,    92,    92,   266};
   const SMeasRegion FALSE_TRANSITION_PROBLEM   = {    345,     115,   125,     8,   266};
   const SMeasRegion FALSE_TRANSITION_SOLUTION  = {    371,     118,   125,    58,   266};
   const SMeasRegion ANGLE_ERROR_PROBLEM        = {    133,     179,    54,    74,     3};
   const SMeasRegion ANGLE_ERROR_SOLUTION       = {    133,     179,    54,    74,   356};

   //                      Source image     , Marker type , Marker region             , Setup function              , Measurement list   , Draw list
   MeasExample.RunMeasCase(IMAGE_NOISE_FILE , M_EDGE      , NOISY_TRANSITION_PROBLEM  , SetupNoiseProblem           , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_NOISE_FILE , M_EDGE      , NOISY_TRANSITION_SOLUTION , SetupNoiseSolution          , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE      , FALSE_TRANSITION_PROBLEM  , SetupFalseTransitionProblem , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE      , FALSE_TRANSITION_SOLUTION , SetupFalseTransitionSolution, M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE      , ANGLE_ERROR_PROBLEM       , SetupAngleErrorProblem      , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE      , ANGLE_ERROR_SOLUTION      , SetupAngleErrorSolution     , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   }
