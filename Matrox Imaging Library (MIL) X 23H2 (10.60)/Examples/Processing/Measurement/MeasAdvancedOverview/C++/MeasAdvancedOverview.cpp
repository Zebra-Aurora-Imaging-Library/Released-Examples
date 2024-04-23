//***************************************************************************************/
//
// File name: MeasAdvancedOverview.cpp
//
// Synopsis:  This program illustrates some advanced measurement concepts.
//            See the PrintHeader() function below for a detailed description.
//            
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include "MeasOverviewExample.h"

//***************************************************************************
// Example constants.
//***************************************************************************
static MIL_DOUBLE DISPLAY_ZOOM_FACTOR = 1.5;

//***************************************************************************
// Example description.
//***************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("MeasAdvancedOverview\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program illustrates some advanced concepts\n")
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
static MIL_CONST_TEXT_PTR SCREW_IMAGE_FILE = EXAMPLE_IMAGE_PATH MIL_TEXT("Screw.tif");

//***************************************************************************
// Example setup functions.
//***************************************************************************
static const MIL_DOUBLE NB_SUB_REGIONS = 5;
static const MIL_DOUBLE ANGLE_SUB_REGION_SIZE = 75;
void SetupBasicTransitionAngle(MIL_ID MilMeasMarker)
   {
   MmeasSetMarker(MilMeasMarker, M_SUB_REGIONS_NUMBER, NB_SUB_REGIONS, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_SUB_REGIONS_SIZE, ANGLE_SUB_REGION_SIZE, M_NULL);
   }

void SetupBasicTransitionAngleIntro(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("By analyzing small 1D profiles, a transition angle can be found.\n\n"));
   SetupBasicTransitionAngle(MilMeasMarker);
   }

void SetupBasicTransitionAngleGood(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("To have an accurate transition position and angle, the search region must be\n")
             MIL_TEXT("smaller than the image transition segment.\n\n"));
   SetupBasicTransitionAngle(MilMeasMarker);
   }

void SetupBasicTransitionAngleTooFar(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("A search region partially containing the transition segment could have an\n")
             MIL_TEXT("erroneous position and angle.\n\n"));
   SetupBasicTransitionAngle(MilMeasMarker);
   }

void SetupSubRegion(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("By default, 3 subregions occupying 1/3 of the search region height are used.\n")
             MIL_TEXT("Edges within subregions are considered subedges.\n\n"));
   }

static const MIL_DOUBLE SUB_REGION_SIZE = 50;
void SetupSubRegionSize(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The size of the subregions can be reduced.\n")
             MIL_TEXT("Subregions of %.2f%% size are displayed.\n\n"),
             SUB_REGION_SIZE);
   MmeasSetMarker(MilMeasMarker, M_SUB_REGIONS_SIZE, SUB_REGION_SIZE, M_NULL);
   }

static const MIL_DOUBLE SUB_REGION_OFFSET = 50;
void SetupSubRegionSizeOffset(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The position of the subregions can be offset, relative to their center.\n")
             MIL_TEXT("Subregions offset by %.2f%% are displayed.\n\n"),
             SUB_REGION_OFFSET);
   MmeasSetMarker(MilMeasMarker, M_SUB_REGIONS_SIZE, SUB_REGION_SIZE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_SUB_REGIONS_OFFSET, SUB_REGION_OFFSET, M_NULL);
   }

void SetupSubRegionSizeOffsetNb(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The number of subregions can be changed.\n")
             MIL_TEXT("An edge marker with 5 subregions is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_SUB_REGIONS_NUMBER, NB_SUB_REGIONS, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_SUB_REGIONS_SIZE, SUB_REGION_SIZE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_SUB_REGIONS_OFFSET, SUB_REGION_OFFSET, M_NULL);
   }

void SetupNoMaxAssociationDistance(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Outlier subedges can affect the transition angle found.\n\n"));
   }

static const MIL_DOUBLE MAX_ASSOCIATION_DISTANCE = 3;
void SetupMaxAssociationDistance(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("A maximum association distance can be set to exclude outliers that are\n")
             MIL_TEXT("too far from the global position.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_MAX_ASSOCIATION_DISTANCE, MAX_ASSOCIATION_DISTANCE, M_NULL);
   }

static const MIL_DOUBLE NB_TO_FIND = 1;
void SetupScoreStrongestPositive(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The score function can be used to select the required edge.\n")
             MIL_TEXT("The position resulting from using the score function to find the\n")
             MIL_TEXT("strongest positive edge is displayed.\n\n")); 

   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, NB_TO_FIND, M_NULL);
   }

void SetupScoreLastPositive(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The position resulting from using the score function to find the\n")
             MIL_TEXT("last positive edge is displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_POLARITY, M_POSITIVE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER, NB_TO_FIND, M_NULL);
   MmeasSetScore(MilMeasMarker, M_STRENGTH_SCORE, 0, 0, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MmeasSetScore(MilMeasMarker, M_DISTANCE_FROM_BOX_ORIGIN_SCORE, 0, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   }

static const MIL_DOUBLE FILTER_SMOOTHNESS = 50;
void SetupNoRotationAngleMode(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("Multiple searches at discrete angular steps can be used to find the angle\n")
             MIL_TEXT("at which the marker returns the highest score.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_SMOOTHNESS, FILTER_SMOOTHNESS, M_NULL);
   }

static const MIL_DOUBLE DELTA_ANGLE = 40;
static const MIL_DOUBLE TOLERANCE_ANGLE = 10;
static const MIL_DOUBLE ACCURACY_ANGLE= 5;
void SetupRotationAngleMode(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The search region found to be at the best angle and its transition\n")
             MIL_TEXT("are displayed.\n\n"));
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_SMOOTHNESS, FILTER_SMOOTHNESS, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_BOX_ANGLE_MODE, M_ENABLE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_BOX_ANGLE_DELTA_NEG, DELTA_ANGLE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_BOX_ANGLE_DELTA_POS, DELTA_ANGLE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_BOX_ANGLE_TOLERANCE, TOLERANCE_ANGLE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_BOX_ANGLE_ACCURACY, ACCURACY_ANGLE, M_NULL);
   }

static const MIL_DOUBLE MIN_EDGEVALUE_VAR_NB = 4;
static const MIL_DOUBLE MIN_EDGEVALUE_VAR_NB_MIN = 2;
static const MIL_DOUBLE MIN_EDGEVALUE = 4;
static const MIL_DOUBLE MIN_EDGEVALUE_VAR_FILTER_TYPE = M_PREWITT;
void SetupGeneralEdgeValueVar(MIL_ID MilMeasMarker)
   {
   MmeasSetMarker(MilMeasMarker, M_NUMBER, MIN_EDGEVALUE_VAR_NB, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_NUMBER_MIN, MIN_EDGEVALUE_VAR_NB_MIN, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_EDGEVALUE_MIN, MIN_EDGEVALUE, M_NULL);
   MmeasSetMarker(MilMeasMarker, M_FILTER_TYPE, MIN_EDGEVALUE_VAR_FILTER_TYPE, M_NULL);
   }

void SetupNoMinEdgeValueVar(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("In some cases, strong transitions that do not return to zero might not be\n")
             MIL_TEXT("detected without increasing the minimum edgevalue. However, increasing the\n")
             MIL_TEXT("minimum edgevalue can compromise the extraction of other weaker transitions.\n")
             MIL_TEXT("The minimum edgevalue variation can be used to solve such issues.\n\n"));
   SetupGeneralEdgeValueVar(MilMeasMarker);   
   }

static const MIL_DOUBLE MIN_EDGEVALUE_VAR = 2;
void SetupMinEdgeValueVar(MIL_ID MilMeasMarker)
   {
   MosPrintf(MIL_TEXT("The transitions extracted using an edgevalue variation of %.2f are displayed.\n\n"), MIN_EDGEVALUE_VAR);
   SetupGeneralEdgeValueVar(MilMeasMarker);
   MmeasSetMarker(MilMeasMarker, M_EDGEVALUE_VAR_MIN, MIN_EDGEVALUE_VAR, M_NULL);
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the example.
   CMeasOverviewExample MeasExample(true);

   // Zoom in the display to show more details.
   MeasExample.SetDisplayZoom(DISPLAY_ZOOM_FACTOR);

   // Print the header.
   PrintHeader();

   // Marker regions used.                               CenterX, CenterY, SizeX, SizeY, Angle
   const SMeasRegion BASIC_ANGLE_REGION               = {    149,     191,   106,    49,     0};
   const SMeasRegion ANGLE_GOOD_REGION                = {    164,     204,    50,    49,     0};
   const SMeasRegion ANGLE_TOO_FAR_REGION             = {    162,     228,    50,    49,     0};
   const SMeasRegion SUB_REGION_CONTROL_REGION        = {    133,     186,    28,    60,     0};
   const SMeasRegion MAX_ASSOCIATION_DISTANCE_REGION  = {    335,     149,    24,    57,   266};
   const SMeasRegion SCORE_REGION                     = {    206,     180,   205,     8,     0};
   const SMeasRegion ANGLE_STEP_REGION                = {    242,     108,   104,   286,   270};
   const SMeasRegion MIN_EDGE_VALUE_VAR_REGION        = {    315,     190,   170,    73,   270};

   //                      Source image     , Marker type, Marker region                   , Setup function                 , Measurement list   , Draw list
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , BASIC_ANGLE_REGION              , SetupBasicTransitionAngleIntro , M_DEFAULT          , EDGE_COMPLETE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , ANGLE_GOOD_REGION               , SetupBasicTransitionAngleGood  , M_DEFAULT          , EDGE_COMPLETE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , ANGLE_TOO_FAR_REGION            , SetupBasicTransitionAngleTooFar, M_DEFAULT          , EDGE_COMPLETE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , SUB_REGION_CONTROL_REGION       , SetupSubRegion                 , M_DEFAULT          , EDGE_COMPLETE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , SUB_REGION_CONTROL_REGION       , SetupSubRegionSize             , M_DEFAULT          , EDGE_COMPLETE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , SUB_REGION_CONTROL_REGION       , SetupSubRegionSizeOffset       , M_DEFAULT          , EDGE_COMPLETE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , SUB_REGION_CONTROL_REGION       , SetupSubRegionSizeOffsetNb     , M_DEFAULT          , EDGE_COMPLETE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , MAX_ASSOCIATION_DISTANCE_REGION , SetupNoMaxAssociationDistance  , M_DEFAULT          , EDGE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , MAX_ASSOCIATION_DISTANCE_REGION , SetupMaxAssociationDistance    , M_DEFAULT          , EDGE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , SCORE_REGION                    , SetupScoreStrongestPositive    , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , SCORE_REGION                    , SetupScoreLastPositive         , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , ANGLE_STEP_REGION               , SetupNoRotationAngleMode       , M_DEFAULT          , EDGE_DRAW_WITH_MARKER_LIST);
   MeasExample.RunMeasCase(IMAGE_FILE       , M_EDGE     , ANGLE_STEP_REGION               , SetupRotationAngleMode         , M_DEFAULT          , EDGE_DRAW_WITH_MARKER_LIST);
   MeasExample.RunMeasCase(SCREW_IMAGE_FILE , M_EDGE     , MIN_EDGE_VALUE_VAR_REGION       , SetupNoMinEdgeValueVar         , M_POSITION         , EDGE_SIMPLE_DRAW_LIST);
   MeasExample.RunMeasCase(SCREW_IMAGE_FILE , M_EDGE     , MIN_EDGE_VALUE_VAR_REGION       , SetupMinEdgeValueVar           , M_DEFAULT          , EDGE_SIMPLE_DRAW_LIST);
   }
