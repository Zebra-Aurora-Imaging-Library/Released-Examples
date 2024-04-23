//***************************************************************************************
//
// File name: MeasOverviewExample.h 
//
// Synopsis:  This file contains the declaration of the classes and structures related
//            to the CMeasOverviewExample class which manages simple measurement 
//            examples.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef MEASOVERVIEWEXAMPLE_H
#define MEASOVERVIEWEXAMPLE_H

//*****************************************************************************
// Image path.
//*****************************************************************************
#define EXAMPLE_IMAGE_PATH M_IMAGE_PATH MIL_TEXT("MeasOverviewBase/")

//*****************************************************************************
// Useful defines.
//*****************************************************************************
#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

//*****************************************************************************
// Example constants.
//*****************************************************************************
static const MIL_INT WINDOWS_OFFSET_Y  = 38;
static const MIL_DOUBLE SUB_ORANGE     = M_RGB888(247,150,70);

//*****************************************************************************
// Declarations of draw operations.
//*****************************************************************************
struct SDrawOp
   {
   MIL_INT Operation;
   MIL_DOUBLE Color;
   bool SubRegionDraw;
   MIL_INT ControlFlag;
   };

struct SDrawList
   {
   void DrawList(MIL_ID MilMeasMarker, MIL_ID MilDest) const
      {
      for(MIL_INT DrawIdx = 0; DrawIdx < NbDrawOp; DrawIdx++)
         {
         const SDrawOp& rDrawOp = pDrawOpList[DrawIdx];
         MgraColor(M_DEFAULT, (MIL_DOUBLE)rDrawOp.Color);
         if(rDrawOp.SubRegionDraw)
            MmeasDraw(M_DEFAULT, MilMeasMarker, MilDest, rDrawOp.Operation, M_RESULT_PER_SUBREGION(M_ALL_SUBREGIONS, M_DEFAULT), rDrawOp.ControlFlag);
         else
            MmeasDraw(M_DEFAULT, MilMeasMarker, MilDest, rDrawOp.Operation, M_DEFAULT, rDrawOp.ControlFlag);
         }
      }

   const SDrawOp* pDrawOpList;
   MIL_INT NbDrawOp;
   };

static const SDrawOp DRAW_REGION                   = {M_DRAW_SEARCH_REGION + M_DRAW_SEARCH_DIRECTION, M_COLOR_MAGENTA, false, M_DEFAULT};
static const SDrawOp DRAW_MARKER_REGION            = {M_DRAW_SEARCH_REGION + M_DRAW_SEARCH_DIRECTION, M_COLOR_MAGENTA, false, M_MARKER};
static const SDrawOp DRAW_CIRCLE_REGION            = {M_DRAW_SEARCH_REGION, M_COLOR_MAGENTA, false, M_MARKER};
static const SDrawOp DRAW_POSITION                 = {M_DRAW_POSITION, M_COLOR_GREEN, false, M_DEFAULT};
static const SDrawOp DRAW_EDGE                     = {M_DRAW_EDGES, M_COLOR_RED, false, M_DEFAULT};
static const SDrawOp DRAW_SUB_POSITIONS            = {M_DRAW_SUB_POSITIONS, SUB_ORANGE, false, M_DEFAULT}; 
static const SDrawOp DRAW_SUB_REGIONS              = {M_DRAW_SEARCH_REGION + M_DRAW_SEARCH_DIRECTION, M_COLOR_BLUE, true, M_DEFAULT};
static const SDrawOp DRAW_STRIPE_WIDTH             = {M_DRAW_WIDTH, M_COLOR_YELLOW, false, M_DEFAULT};
static const SDrawOp DRAW_STRIPE_INCLUSION_POINT   = {M_DRAW_INCLUSION_POINT, M_COLOR_DARK_YELLOW, false, M_MARKER};

static const SDrawOp EDGE_SIMPLE_DRAW_OP[]         = {DRAW_REGION, DRAW_POSITION};
static const SDrawOp EDGE_DRAW_OP[]                = {DRAW_REGION, DRAW_EDGE, DRAW_SUB_POSITIONS, DRAW_POSITION};
static const SDrawOp EDGE_DRAW_WITH_MARKER_OP[]    = {DRAW_MARKER_REGION, DRAW_REGION, DRAW_EDGE, DRAW_SUB_POSITIONS, DRAW_POSITION};
static const SDrawOp EDGE_COMPLETE_DRAW_OP[]       = {DRAW_SUB_REGIONS, DRAW_REGION, DRAW_EDGE, DRAW_SUB_POSITIONS, DRAW_POSITION};
static const SDrawOp STRIPE_SIMPLE_DRAW_OP[]       = {DRAW_REGION, DRAW_POSITION, DRAW_STRIPE_WIDTH};
static const SDrawOp STRIPE_INCLUSION_DRAW_OP[]    = {DRAW_REGION, DRAW_POSITION, DRAW_STRIPE_WIDTH, DRAW_STRIPE_INCLUSION_POINT};
static const SDrawOp CIRCLE_DRAW_OP[]              = {DRAW_CIRCLE_REGION, DRAW_EDGE, DRAW_SUB_POSITIONS, DRAW_POSITION};

#define CREATE_LIST(x) {x, ARRAY_COUNT(x)}
static const SDrawList EDGE_SIMPLE_DRAW_LIST       = CREATE_LIST(EDGE_SIMPLE_DRAW_OP);
static const SDrawList EDGE_DRAW_LIST              = CREATE_LIST(EDGE_DRAW_OP);
static const SDrawList EDGE_DRAW_WITH_MARKER_LIST  = CREATE_LIST(EDGE_DRAW_WITH_MARKER_OP);
static const SDrawList EDGE_COMPLETE_DRAW_LIST     = CREATE_LIST(EDGE_COMPLETE_DRAW_OP);
static const SDrawList STRIPE_SIMPLE_DRAW_LIST     = CREATE_LIST(STRIPE_SIMPLE_DRAW_OP);
static const SDrawList STRIPE_INCLUSION_DRAW_LIST  = CREATE_LIST(STRIPE_INCLUSION_DRAW_OP);
static const SDrawList CIRCLE_DRAW_LIST            = CREATE_LIST(CIRCLE_DRAW_OP);

//*****************************************************************************
// SMeasRegion. A useful structure that contains all the information of a measurement
//              region.
//*****************************************************************************
struct SMeasRegion
   {
   void SetMarkerRegion(MIL_ID MilMeasMarker, MIL_INT MarkerType) const
      {
      if(MarkerType == M_CIRCLE)
         {
         MmeasSetMarker(MilMeasMarker, M_RING_CENTER, CenterX, CenterY);
         MmeasSetMarker(MilMeasMarker, M_RING_RADII, SizeXOrInnerRadius, SizeYOrOuterRadius);
         MmeasSetMarker(MilMeasMarker, M_SUB_REGIONS_CHORD_ANGLE, Angle, M_NULL);
         }
      else
         {
         MmeasSetMarker(MilMeasMarker, M_BOX_CENTER, CenterX, CenterY);
         MmeasSetMarker(MilMeasMarker, M_BOX_SIZE, SizeXOrInnerRadius, SizeYOrOuterRadius);
         MmeasSetMarker(MilMeasMarker, M_BOX_ANGLE, Angle, M_NULL);
         }
      MmeasSetMarker(MilMeasMarker, M_SEARCH_REGION_INPUT_UNITS, M_WORLD, M_NULL);
      }

   MIL_DOUBLE CenterX;
   MIL_DOUBLE CenterY;
   MIL_DOUBLE SizeXOrInnerRadius;
   MIL_DOUBLE SizeYOrOuterRadius;
   MIL_DOUBLE Angle;
   };

// Forward declaration of the CProfileDisplay.
class CProfileDisplay;

typedef void (*SetupFunc)(MIL_ID);

//*****************************************************************************
// CMeasOverviewExample. Declaration of the CMeasOverview class.
//*****************************************************************************
class CMeasOverviewExample
   {
   public:
      // Constructor.
      CMeasOverviewExample(bool UseProfileDisplay);

      // Destructor.
      virtual ~CMeasOverviewExample();
      
      // Case running function.
      void RunMeasCase(MIL_CONST_TEXT_PTR ImageFile, MIL_INT MarkerType, const SMeasRegion& rMeasBox, SetupFunc pSetupFunc, MIL_INT MeasurementList, const SDrawList& rDrawList);

      // Function to set the display zoom.
      void SetDisplayZoom( MIL_DOUBLE DisplayZoom);

   private:

      MIL_ID m_MilApplication;
      MIL_ID m_MilSystem;
      MIL_ID m_MilDisplay;
      MIL_ID m_MilGraList;

      CProfileDisplay* m_pProfileDisplay;
   };

#endif //MEASOVERVIEWEXAMPLE_H
