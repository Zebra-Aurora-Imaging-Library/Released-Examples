//***************************************************************************************/
//
// File name: MeasBasicOverview.cpp
//
// Synopsis:  This program contains simple measurement examples.
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
             MIL_TEXT("MeasBasicOverview\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program illustrates some basic concepts\n")
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
void SetupBasic(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The purpose of the measurement module is to find 'N' transitions\n")
             MIL_TEXT("in a 1D profile.\n\n"));
   }

void SetupNoValidation(MIL_ID MilMeasMarker)
   {
   MmeasSetMarker(MilMeasMarker, M_NUMBER, M_ALL, M_NULL);
   MosPrintf(MIL_TEXT("The 'N' transitions of a given profile are displayed.\n\n"));
   }

static const MIL_DOUBLE MIN_EDGE_VALUE = 5;
void SetupValidationMinEdgeValue(MIL_ID MilMeasMarker)
   {
   MmeasSetMarker(MilMeasMarker, M_NUMBER, M_ALL, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_EDGEVALUE_MIN, MIN_EDGE_VALUE, M_NULL);
   MosPrintf(MIL_TEXT("The minimum edge value can be modified to exclude transitions\n")
             MIL_TEXT("that are too weak.\n\n"));
   }

void SetupValidationPolarity(MIL_ID MilMeasMarker)
   {
   MmeasSetMarker(MilMeasMarker, M_NUMBER, M_ALL, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_EDGEVALUE_MIN, MIN_EDGE_VALUE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   MosPrintf(MIL_TEXT("The polarity can be constrained to exclude certain transitions.\n")
             MIL_TEXT("Only transitions with a positive polarity are displayed.\n\n"));
   }

static const MIL_DOUBLE NB_TO_FIND = 1;
void SetupValidationSelection(MIL_ID MilMeasMarker)
   {
   MmeasSetMarker(MilMeasMarker, M_NUMBER, M_ALL, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_EDGEVALUE_MIN, MIN_EDGE_VALUE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, NB_TO_FIND, M_NULL);
   MosPrintf(MIL_TEXT("By default, the 'N' strongest transitions are selected.\n")
             MIL_TEXT("The strongest transition is displayed.\n\n"));
   }

int MosMain(void)
   {
   // Allocate the example.
   CMeasOverviewExample MeasExample(true);

   // Print the header.
   PrintHeader();

   // Marker regions used.                          CenterX, CenterY, SizeX, SizeY, Angle
   const SMeasRegion BASIC_REGION                = {     65,     110,   100,     8,     2};
   const SMeasRegion VALIDATION_SELECTION_REGION = {    206,     180,   205,     8,     0};

   //                      Source image  , Marker type, Marker region               , Setup function              , Measurement list  , Draw list
   MeasExample.RunMeasCase(IMAGE_FILE    , M_EDGE     , BASIC_REGION                , SetupBasic                  , M_POSITION        , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE    , M_EDGE     , VALIDATION_SELECTION_REGION , SetupNoValidation           , M_POSITION        , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE    , M_EDGE     , VALIDATION_SELECTION_REGION , SetupValidationMinEdgeValue , M_POSITION        , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE    , M_EDGE     , VALIDATION_SELECTION_REGION , SetupValidationPolarity     , M_POSITION        , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE    , M_EDGE     , VALIDATION_SELECTION_REGION , SetupValidationSelection    , M_POSITION        , EDGE_SIMPLE_DRAW_LIST);
   }
