//***************************************************************************************/
//
// File name: LaserProfileMetrologyAnalysis.cpp
//
// Synopsis:  This example uses the Metrology module to measure and verify
//            various features along an acquired laser line profile of a part.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

//Macro defining the example's filepath.
#define PART_LASER_PROFILE_FILENAME M_IMAGE_PATH MIL_TEXT("3dProfilometry/PartLaserProfile.mim")

// Util constants
#define PEAK_MIN_CONTRAST   20 // gray level units

// Util text area definition.
static const MIL_INT AnnotationTextChildOX = 500;
static const MIL_INT AnnotationTextChildOY = 10;
static const MIL_INT AnnotationTextChildSX = 600;
static const MIL_INT AnnotationTextChildSY = 40;

// Utility functions.
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("LaserProfileMetrologyAnalysis\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example uses the Metrology module to measure and verify\n"));
   MosPrintf(MIL_TEXT("various features along an acquired laser line profile of a part.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, system, display, buffer, image processing,\n")
             MIL_TEXT("graphic and metrology.\n\n"));
   }

void WaitForKey()
   {
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n\n"));
   MosGetch();
   }

// Utility class to draw tolerance annotations.
class CGraphicalAnnotation
   {
   public:
      CGraphicalAnnotation(MIL_ID GraphicListId)
         {
         m_GraphicalAnnotation = GraphicListId;
         }

      virtual ~CGraphicalAnnotation() { ClearAnnotations(); }

      void ChangeColorAccordingToleranceStatus(MIL_INT Status)
         {
         switch (Status)
            {
            case M_PASS:    { MgraColor(M_DEFAULT, M_COLOR_GREEN);  } break;
            case M_WARNING: { MgraColor(M_DEFAULT, M_COLOR_YELLOW); } break;
            case M_FAIL:    { MgraColor(M_DEFAULT, M_COLOR_RED);    } break;
            default:        {; } break;
            }
         }

      MIL_CONST_TEXT_PTR StatusToText(MIL_INT Status)
         {
         switch (Status)
            {
            case M_PASS: { return MIL_TEXT("pass");    } break;
            case M_WARNING: { return MIL_TEXT("warning"); } break;
            case M_FAIL: { return MIL_TEXT("fail");    } break;
            default:        {; } break;
            }
         return MIL_TEXT("");
         }

      void ClearAnnotations(bool EmptyGraphicList=false)
         {
         for (MIL_INT i = 0; i < (MIL_INT)m_ListLabelsAnnotations.size(); i++)
            MgraControlList(m_GraphicalAnnotation, m_ListLabelsAnnotations[i], M_DEFAULT, M_DELETE, M_DEFAULT);
         
         m_ListLabelsAnnotations.clear();

         if (EmptyGraphicList)
            MgraClear(M_DEFAULT, m_GraphicalAnnotation);
         }

      void PrintToleranceValue(MIL_INT Status,
                               MIL_DOUBLE Value,
                               MIL_CONST_TEXT_PTR pToleranceName,
                               MIL_CONST_TEXT_PTR pToleranceUnits,
                               MIL_INT LineIdx)
         {
         MIL_TEXT_CHAR TextToDisplay[100];
         MosSprintf(TextToDisplay, 100, MIL_TEXT("%s: %-3.2f %s"), pToleranceName, Value, pToleranceUnits);
         ChangeColorAccordingToleranceStatus(Status);
         MgraText(M_DEFAULT, m_GraphicalAnnotation, AnnotationTextChildOX, AnnotationTextChildOY + LineIdx * 20, TextToDisplay);
         MIL_ID TextLabelInGraphicList = MgraInquireList(m_GraphicalAnnotation, M_LIST, M_DEFAULT, M_LAST_LABEL, M_NULL);
         m_ListLabelsAnnotations.push_back(M_GRAPHIC_LABEL(TextLabelInGraphicList));
         MosPrintf(MIL_TEXT("%s (%s).\n"), TextToDisplay, StatusToText(Status));
         }

   private:
      MIL_ID m_GraphicalAnnotation;
      std::vector<MIL_ID> m_ListLabelsAnnotations;
   };

//******************************************************
// Determining specific features' locations on the part.

#define SUBEDGEL_LEFT_REGION_OFFSET_X  10.0
#define SUBEDGEL_LEFT_REGION_OFFSET_Y   5.0
#define SUBEDGEL_LEFT_REGION_SIZE_X   300.0
#define SUBEDGEL_LEFT_REGION_SIZE_Y   400.0
#define RESAMPLING_RADIUS               3.0

void PartFeaturesLocation(MIL_ID MilMetrolContext, 
                          MIL_ID MilMetrolResult, 
                          MIL_ID MilImage,
                          MIL_ID MilGraphicList)
   {
   // Initialisation of the annotation class.
   CGraphicalAnnotation GraphicalAnnotation(MilGraphicList);

   MosPrintf(MIL_TEXT("Determining basic geometric features along the part's profile.\n")
             MIL_TEXT("=====================================================++=======\n\n"));

   std::vector<MIL_INT> BaseFeatureArray(2);

   // Build the extreme right side (0 degree direction) edgel point.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(2), M_CLOSEST_TO_INFINITE_POINT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(2), M_ANGLE, 0.0);
   MosPrintf(MIL_TEXT("- The right most edgel position is constructed (in green).\n"));

   // Extracting a sub-portion of the edgels.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(3), M_CLONE_FEATURE, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(3), M_GLOBAL_FRAME, M_RECTANGLE, 
                 SUBEDGEL_LEFT_REGION_OFFSET_X, 
                 SUBEDGEL_LEFT_REGION_OFFSET_Y, 
                 SUBEDGEL_LEFT_REGION_SIZE_X, 
                 SUBEDGEL_LEFT_REGION_SIZE_Y, 
                 0.0, M_NULL);

   // Resampling of the edgel to have a uniform distribution.
   BaseFeatureArray[0] = M_FEATURE_LABEL(3);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(4), M_COPY_FEATURE_EDGELS, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(4), M_EDGEL_RESAMPLING_MODE,       M_MEAN);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(4), M_EDGEL_RESAMPLING_RADIUS,     RESAMPLING_RADIUS);

   // Constructing a robust fit segment using the resampled edgels.
   BaseFeatureArray[0] = M_FEATURE_LABEL(4);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(5), M_FIT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(5), M_FIT_TYPE, M_ROBUST_FIT); 
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(5), M_FIT_DISTANCE_OUTLIERS,  M_AUTO);

   MosPrintf(MIL_TEXT("- A segment is defined by being fit on the resampled positions (in blue) of\n") 
             MIL_TEXT("  a sub region of the profile to fixture the part (in red).\n"));

   // Constructing segment mid-point.
   BaseFeatureArray[0] = M_FEATURE_LABEL(5);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(6), M_MIDDLE, BaseFeatureArray, M_NULL, 1, M_DEFAULT);

   // Constructing segment right end-point.
   BaseFeatureArray[0] = M_FEATURE_LABEL(5);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(7), M_POSITION_END, BaseFeatureArray, M_NULL, 1, M_DEFAULT);

   // Constructing a local frame centered and aligned according to the segment.
   BaseFeatureArray[0] = M_FEATURE_LABEL(6);
   BaseFeatureArray[1] = M_FEATURE_LABEL(7);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LOCAL_FRAME, M_FEATURE_LABEL(8), M_CONSTRUCTION, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   MosPrintf(MIL_TEXT("- A local frame is defined relative to the mid-point of\n") 
             MIL_TEXT("  the segment (in cyan).\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Metrology annotations.
   MgraClear(M_DEFAULT, MilGraphicList);

   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(1), M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(2), M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_GRAY);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(3), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(4), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(5), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(8), M_DEFAULT);

   MosPrintf(MIL_TEXT("\nZoom and pan the display to see the details.\n"));
   WaitForKey();

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);
   }


//***************************************************************
// Analyzing the slope of the part's neck along its profile.

#define SEGMENT_REGION_SIZE_X         300.0
#define SEGMENT_REGION_SIZE_Y          60.0
#define SEGMENT_RIGHT_REGION_OFFSET_X 400.0
#define SEGMENT_RIGHT_REGION_OFFSET_Y 130.0

#define TOL_ANGULARITY_MIN   65.0
#define TOL_ANGULARITY_MAX   75.0
#define TOL_ROUNDNESS_MAX    20.0

#define SLOPE_REGION_OFFSET_X +20.0 
#define SLOPE_REGION_OFFSET_Y -20.0
#define SLOPE_REGION_SIZE_X    40.0
#define SLOPE_REGION_SIZE_Y    40.0
#define SLOPE_REGION_ANGLE    270.0

void NeckAnalysis(MIL_ID MilSystem, 
                  MIL_ID MilDisplay,
                  MIL_ID MilMetrolContext, 
                  MIL_ID MilMetrolResult, 
                  MIL_ID MilImage, 
                  MIL_ID MilGraphicList)
   {
   // Initialization of the annotation utility class.
   CGraphicalAnnotation GraphicalAnnotation(MilGraphicList);

   std::vector<MIL_INT> BaseFeatureArray(2);
   MIL_INT ToleranceStatus;
   MIL_DOUBLE ToleranceValue;

   MosPrintf(MIL_TEXT("Determining and verifying the slope and roundness of the neck of the part's\n")
             MIL_TEXT("neck.\n")
             MIL_TEXT("============================================================================\n\n"));

   // Construct a segment on the right flank of the neck relative to the local frame.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(10), M_FIT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(10), M_FEATURE_LABEL(8), M_RECTANGLE, 
                 0, -SEGMENT_REGION_SIZE_Y / 2, 
                 SEGMENT_REGION_SIZE_X, 
                 SEGMENT_REGION_SIZE_Y, 
                 0.0, M_NULL);
   // Use a robust fit method to handle the laser profile noise (speckle).
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_FIT_TYPE, M_ROBUST_FIT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_FIT_DISTANCE_OUTLIERS, M_USER_DEFINED);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_FIT_DISTANCE_OUTLIERS_VALUE, 1.0);

   // Construct a segment on the left flank of the neck relative to the local frame.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(11), M_FIT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(11), M_FEATURE_LABEL(8), M_RECTANGLE, 
                 SEGMENT_RIGHT_REGION_OFFSET_X - SEGMENT_REGION_SIZE_X, 
                 SEGMENT_RIGHT_REGION_OFFSET_Y - SEGMENT_REGION_SIZE_Y / 2, 
                 SEGMENT_REGION_SIZE_X, 
                 SEGMENT_REGION_SIZE_Y, 
                 0.0, M_NULL);
   // Use a robust fit method to handle the laser profile noise (speckle).
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(11), M_FIT_TYPE, M_ROBUST_FIT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(11), M_FIT_DISTANCE_OUTLIERS, M_USER_DEFINED);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(11), M_FIT_DISTANCE_OUTLIERS_VALUE, 1.0);
   
   MosPrintf(MIL_TEXT("- Segments (in green) to fit the flanks from either side of the neck are built.\n")
             MIL_TEXT("  End points of the segments correspond to a user defined maximum\n")
             MIL_TEXT("  deviation of the part profile from the segments.\n"));

   // Construct the mid-point between the two segments' end-point from either side of the neck.
   BaseFeatureArray[0] = M_FEATURE_LABEL(10);
   BaseFeatureArray[1] = M_FEATURE_LABEL(11);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(12), M_CLOSEST, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   BaseFeatureArray[0] = M_FEATURE_LABEL(11);
   BaseFeatureArray[1] = M_FEATURE_LABEL(10);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(13), M_CLOSEST, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   BaseFeatureArray[0] = M_FEATURE_LABEL(12);
   BaseFeatureArray[1] = M_FEATURE_LABEL(13);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(14), M_CENTER, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   MosPrintf(MIL_TEXT("- The mid-point (in cyan) between the segment end-points (in blue) is\n  constructed.\n"));

   // Construct the intersection point between the part profile and a line going trough the mid point and parallel to the left flank.
   BaseFeatureArray[0] = M_FEATURE_LABEL(14);
   BaseFeatureArray[1] = M_FEATURE_LABEL(10);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LINE, M_FEATURE_LABEL(15), M_PARALLEL, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   BaseFeatureArray[1] = M_FEATURE_LABEL(15);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(16), M_INTERSECTION, BaseFeatureArray, M_NULL, 2, M_DEFAULT);
   
   MosPrintf(MIL_TEXT("- The intersection point (in red) between the profile and a line parallel\n") 
             MIL_TEXT("  to the left flank (in blue) is constructed.\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Metrology annotations.
   MgraClear(M_DEFAULT, MilGraphicList);

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(8), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(10), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(10), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(11), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(11), M_DEFAULT);
 
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(12), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(13), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(15), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(14), M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(16), M_DEFAULT);
 
   MdispZoom(MilDisplay, 2.0, 2.0);
   MdispPan(MilDisplay, 105, 110);
   WaitForKey();

   // Construct a local frame attached to the neck's mid-point.
   BaseFeatureArray[0] = M_FEATURE_LABEL(16);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LOCAL_FRAME, M_FEATURE_LABEL(17), M_CONSTRUCTION, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(17), M_REFERENCE_FRAME, M_FEATURE_LABEL(8));
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(17), M_ANGLE, 0.0);
   
   // Fit a segment along the slope of the neck.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(18), M_FIT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(18), M_FEATURE_LABEL(17), M_RECTANGLE, 
                 SLOPE_REGION_OFFSET_X, 
                 SLOPE_REGION_OFFSET_Y, 
                 SLOPE_REGION_SIZE_X, 
                 SLOPE_REGION_SIZE_Y, 
                 SLOPE_REGION_ANGLE, 
                 M_NULL);

   MosPrintf(MIL_TEXT("- A segment that fits the slope of the neck at the location of the intersection\n  point is defined (in red).\n"));

   // Define the angular tolerance of the slope of the neck relative to the right flank of the neck.
   BaseFeatureArray[0] = M_FEATURE_LABEL(18);
   BaseFeatureArray[1] = M_FEATURE_LABEL(11);
   MmetAddTolerance(MilMetrolContext, M_ANGULARITY, M_TOLERANCE_LABEL(19), TOL_ANGULARITY_MIN, TOL_ANGULARITY_MAX, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   MosPrintf(MIL_TEXT("- An angular tolerance (in yellow) is defined between the slope and the right\n  flank (in red).\n\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Retrieving the tolerance value.
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(19), M_STATUS + M_TYPE_MIL_INT, &ToleranceStatus);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(19), M_TOLERANCE_VALUE, &ToleranceValue);

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(16), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(17), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(18), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(11), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(18), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(19), M_DEFAULT);
   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus, ToleranceValue, MIL_TEXT("Angle between the slope and the flank"), MIL_TEXT("degrees"), 0);

   MdispZoom(MilDisplay, 4.0, 4.0);
   MdispPan(MilDisplay, 280, 215);
   WaitForKey();

   // Intersection between the segment of the left flank and segment of the neck's slope is constructed.
   BaseFeatureArray[0] = M_FEATURE_LABEL(10);
   BaseFeatureArray[1] = M_FEATURE_LABEL(18);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(20), M_EXTENDED_INTERSECTION, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   // A derived ring region centered on the intersection is built.
   MIL_ID DerivedRingRegion = MmetAlloc(MilSystem, M_DERIVED_GEOMETRY_REGION, M_NULL);
   MmetControl(DerivedRingRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_GEOMETRY, M_RING);
   MmetControl(DerivedRingRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION_TYPE, M_LABEL_VALUE);
   MmetControl(DerivedRingRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION, M_FEATURE_LABEL(20));
   MmetControl(DerivedRingRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START_TYPE, M_PARAMETRIC);
   MmetControl(DerivedRingRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START, 0.0);
   MmetControl(DerivedRingRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END_TYPE, M_LABEL_VALUE);
   MmetControl(DerivedRingRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END, M_FEATURE_LABEL(16));

   // Copy of the edgels from the ring region.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(21), M_CLONE_FEATURE, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(21), M_DEFAULT, M_FROM_DERIVED_GEOMETRY_REGION, DerivedRingRegion, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL);
   MmetFree(DerivedRingRegion);

   MosPrintf(MIL_TEXT("- Edgels of the transition (in red) from the right flank to the slope are\n  extracted\n") 
             MIL_TEXT("  using a derived a ring region (in blue).\n"));

   // Define the roundness tolerance of the neck left flank to the slope transition.
   BaseFeatureArray[0] = M_FEATURE_LABEL(21);
   MmetAddTolerance(MilMetrolContext, M_ROUNDNESS, M_TOLERANCE_LABEL(22), 0, TOL_ROUNDNESS_MAX, BaseFeatureArray, M_NULL, 1, M_DEFAULT);

   MosPrintf(MIL_TEXT("- The roundness of the transition is measured (in green).\n\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Retrieving the tolerance value.
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(22), M_STATUS + M_TYPE_MIL_INT, &ToleranceStatus);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(22), M_TOLERANCE_VALUE, &ToleranceValue);

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(16), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(20), M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(21), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(21), M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(22), M_DEFAULT);
   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus, ToleranceValue, MIL_TEXT("Roundness of the left transition"), MIL_TEXT("pixels"), 0);

   MdispZoom(MilDisplay, 2.0, 2.0);
   MdispPan(MilDisplay, 85, 60);
   WaitForKey();

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);
   }

//***************************************
// Analysis of the gap along the profile.

#define LEFT_SEGMENT_REGION_OFFSET_X 400.0
#define LEFT_SEGMENT_REGION_OFFSET_Y   0.0 
#define LEFT_SEGMENT_REGION_SIZE_X   250.0 
#define LEFT_SEGMENT_REGION_SIZE_Y   150.0

#define RIGHT_SEGMENT_REGION_OFFSET_X 400.0
#define RIGHT_SEGMENT_REGION_OFFSET_Y 200.0 
#define RIGHT_SEGMENT_REGION_SIZE_X   250.0 
#define RIGHT_SEGMENT_REGION_SIZE_Y   100.0

#define GAP_REGION_OFFSET_X  LEFT_SEGMENT_REGION_OFFSET_X 
#define GAP_REGION_OFFSET_Y                          80.0
#define GAP_REGION_SIZE_X      LEFT_SEGMENT_REGION_SIZE_X 
#define GAP_REGION_SIZE_Y                           200.0

#define END_TO_END_DISTANCE_TOL_MIN     100.0
#define END_TO_END_DISTANCE_TOL_MAX     120.0
#define DEG00_DISTANCE_TOL_MIN          10.0
#define DEG00_DISTANCE_TOL_MAX          20.0
#define DEG90_DISTANCE_TOL_MIN          110.0
#define DEG90_DISTANCE_TOL_MAX          120.0

void GapAnalysis(MIL_ID MilDisplay,
                 MIL_ID MilMetrolContext, 
                 MIL_ID MilMetrolResult, 
                 MIL_ID MilImage, 
                 MIL_ID MilGraphicList)
   {
   // Initialization of the annotation utility class.
   CGraphicalAnnotation GraphicalAnnotation(MilGraphicList);

   std::vector<MIL_INT> BaseFeatureArray(2);
   MIL_INT    ToleranceStatus[2];
   MIL_DOUBLE ToleranceValue[2];

   MosPrintf(MIL_TEXT("Analysing the gap along the part's profile.\n"));
   MosPrintf(MIL_TEXT("===========================================\n\n"));

   // Constructing a segment on the left side of the gap.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(30), M_FIT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(30), M_FEATURE_LABEL(8), M_RECTANGLE, 
                 LEFT_SEGMENT_REGION_OFFSET_X, 
                 LEFT_SEGMENT_REGION_OFFSET_Y, 
                 LEFT_SEGMENT_REGION_SIZE_X, 
                 LEFT_SEGMENT_REGION_SIZE_Y, 
                 0.0, M_NULL);
   // Use a robust fit method.
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(30), M_FIT_TYPE, M_ROBUST_FIT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(30), M_FIT_DISTANCE_OUTLIERS, M_USER_DEFINED);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(30), M_FIT_DISTANCE_OUTLIERS_VALUE, 10.0);

   // Constructing a segment on the right side of the gap.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(31), M_FIT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(31), M_FEATURE_LABEL(8), M_RECTANGLE, 
                 RIGHT_SEGMENT_REGION_OFFSET_X, 
                 RIGHT_SEGMENT_REGION_OFFSET_Y, 
                 RIGHT_SEGMENT_REGION_SIZE_X, 
                 RIGHT_SEGMENT_REGION_SIZE_Y, 
                 0.0, M_NULL);
   // Use a robust fit method.
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(31), M_FIT_TYPE, M_ROBUST_FIT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(31), M_FIT_DISTANCE_OUTLIERS, M_USER_DEFINED);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(31), M_FIT_DISTANCE_OUTLIERS_VALUE, 10.0);
   
   MosPrintf(MIL_TEXT("- Best-fit segments from either side of the gap are constructed (in green).\n"));

   // Tolerance distance between segment end-points.
   BaseFeatureArray[0] = M_FEATURE_LABEL(30);
   BaseFeatureArray[1] = M_FEATURE_LABEL(31);
   MmetAddTolerance(MilMetrolContext, M_DISTANCE_MIN, M_TOLERANCE_LABEL(32), END_TO_END_DISTANCE_TOL_MIN, END_TO_END_DISTANCE_TOL_MAX, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   MosPrintf(MIL_TEXT("- The minimum distance tolerance between the segments is defined.\n\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Retrieving the tolerance value.
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(32), M_STATUS + M_TYPE_MIL_INT, ToleranceStatus);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(32), M_TOLERANCE_VALUE, ToleranceValue);

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION,                 M_FEATURE_LABEL(30),  M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(30),  M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION,                 M_FEATURE_LABEL(31),  M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(31),  M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(32), M_DEFAULT);
   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus[0], ToleranceValue[0], MIL_TEXT("End-to-end gap distance value"), MIL_TEXT("pixels"), 0);
   
   MdispZoom(MilDisplay, 1.0, 1.0);
   MdispPan(MilDisplay, 0, 0);
   WaitForKey();

   // Extracting edgels in a region around the gap.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(33), M_CLONE_FEATURE, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(33), M_FEATURE_LABEL(8), M_RECTANGLE, 
                 GAP_REGION_OFFSET_X, 
                 GAP_REGION_OFFSET_Y, 
                 GAP_REGION_SIZE_X, 
                 GAP_REGION_SIZE_Y, 
                 0.0, M_NULL);

   // Determining the top-right edgel end point in the local frame of the part.
   BaseFeatureArray[0] = M_FEATURE_LABEL(33);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(34), M_CLOSEST_TO_INFINITE_POINT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(34), M_REFERENCE_FRAME, M_FEATURE_LABEL(8));
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(34), M_ANGLE, 70);

   // Determining the bottom-left edgel end point in the local frame of the part.
   BaseFeatureArray[0] = M_FEATURE_LABEL(33);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(35), M_CLOSEST_TO_INFINITE_POINT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(35), M_REFERENCE_FRAME, M_FEATURE_LABEL(8));
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(35), M_ANGLE, 250);

   MosPrintf(MIL_TEXT("- The end points of the edgels from either side of the gap are found (in blue).\n"));

   // Calculating the minimum distance between the end point along the x-axis direction of the feature reference frame.
   BaseFeatureArray[0] = M_FEATURE_LABEL(34);
   BaseFeatureArray[1] = M_FEATURE_LABEL(35);
   MmetAddTolerance(MilMetrolContext, M_DISTANCE_MIN, M_TOLERANCE_LABEL(36), DEG00_DISTANCE_TOL_MIN, DEG00_DISTANCE_TOL_MAX, BaseFeatureArray, M_NULL, 2, M_DEFAULT);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(36), M_DISTANCE_MODE, M_GAP_AT_ANGLE);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(36), M_ANGLE, 0.0);

   MosPrintf(MIL_TEXT("- The gap minimum distance along the local frame's x-axis is calculated\n  (in yellow).\n"));

   // Calculating the minimum distance between the end point along the y-axis direction of the feature reference frame.
   BaseFeatureArray[0] = M_FEATURE_LABEL(34);
   BaseFeatureArray[1] = M_FEATURE_LABEL(35);
   MmetAddTolerance(MilMetrolContext, M_DISTANCE_MIN, M_TOLERANCE_LABEL(37), DEG90_DISTANCE_TOL_MIN, DEG90_DISTANCE_TOL_MAX, BaseFeatureArray, M_NULL, 2, M_DEFAULT);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(37), M_DISTANCE_MODE, M_GAP_AT_ANGLE);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(37), M_ANGLE, 90.0);

   MosPrintf(MIL_TEXT("- The gap minimum distance along the local frame's y-axis is calculated\n  (in yellow).\n\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);
   
   // Retrieving the tolerance values.
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(36), M_STATUS + M_TYPE_MIL_INT, &ToleranceStatus[0]);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(36), M_TOLERANCE_VALUE, &ToleranceValue[0]);
   
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(37), M_STATUS + M_TYPE_MIL_INT, &ToleranceStatus[1]);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(37), M_TOLERANCE_VALUE, &ToleranceValue[1]);

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(8), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION,                 M_FEATURE_LABEL(33),  M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(33),  M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(34),  M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(35),  M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(36), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(37), M_DEFAULT);
   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus[0], ToleranceValue[0], MIL_TEXT("Horizontal gap value"), MIL_TEXT("pixels"), 0);
   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus[1], ToleranceValue[1], MIL_TEXT("Vertical gap value"), MIL_TEXT("pixels"), 1);

   MdispZoom(MilDisplay, 2.0, 2.0);
   MdispPan(MilDisplay, 425, 230);
   WaitForKey();

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);
   }


//**********************************
// Verifying the part's thread.

#define THREAD_REGION_OFFSET_X   600.0 
#define THREAD_REGION_OFFSET_Y   150.0 
#define THREAD_REGION_SIZE_X     300.0 
#define THREAD_REGION_SIZE_Y     150.0 
#define CROSS_SECTION_DISTANCE    25.0
#define CROSS_SECTION_LENGTH     100.0

#define DENT_EDGE_POSITION_MIN    34.0
#define DENT_EDGE_POSITION_MAX    36.0
#define DENT_ANGULARITY_MIN       70.0
#define DENT_ANGULARITY_MAX       75.0
#define DENT_AREA_MIN            980.0
#define DENT_AREA_MAX            990.0
#define DENT_PERIMETER_MIN       135.0
#define DENT_PERIMETER_MAX       145.0
#define DENT_RESIDUAL_AREA_MIN     0.0
#define DENT_RESIDUAL_AREA_MAX   120.0

const MIL_INT    NbRefDentPositions = 50;
const MIL_DOUBLE DentHeight = 35.0;
std::vector<MIL_DOUBLE> RefDentArrayPosX(NbRefDentPositions);
std::vector<MIL_DOUBLE> RefDentArrayPosY(NbRefDentPositions);

// Util theoretical dent profile construction.
void BuildTheoreticalDentProfile()
   {
   MIL_DOUBLE DentSlope = (2.0*DentHeight / NbRefDentPositions);
   for (MIL_INT ii = 0; ii < NbRefDentPositions/2; ii++)
      {
      RefDentArrayPosX[ii] = (MIL_DOUBLE)ii;
      RefDentArrayPosY[ii] = -ii*DentSlope;
      }
   for (MIL_INT ii = NbRefDentPositions / 2; ii < NbRefDentPositions; ii++)
      {
      RefDentArrayPosX[ii] = (MIL_DOUBLE)ii;
      RefDentArrayPosY[ii] = -DentHeight + (ii - NbRefDentPositions / 2 )* DentSlope;
      }
   }

void ThreadAnalysis(MIL_ID MilSystem, 
                    MIL_ID MilDisplay,
                    MIL_ID MilMetrolContext, 
                    MIL_ID MilMetrolResult, 
                    MIL_ID MilImage, 
                    MIL_ID MilGraphicList)
   {
   // Initialization of the annotation utility class.
   CGraphicalAnnotation GraphicalAnnotation(MilGraphicList);

   // Build dent theoretical profile.
   BuildTheoreticalDentProfile();

   std::vector<MIL_INT> BaseFeatureArray(3);
   MIL_INT ToleranceStatus[2];
   MIL_DOUBLE ToleranceValue[2];

   MosPrintf(MIL_TEXT("Verifying the part's thread.\n"));
   MosPrintf(MIL_TEXT("============================\n\n"));

   // Extracting edgels in a region around the thread.
   BaseFeatureArray[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(40), M_CLONE_FEATURE, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(40), M_FEATURE_LABEL(8), M_RECTANGLE, 
                 THREAD_REGION_OFFSET_X, 
                 THREAD_REGION_OFFSET_Y, 
                 THREAD_REGION_SIZE_X, 
                 THREAD_REGION_SIZE_Y, 
                 0.0, M_NULL);

   // Fit a segment on the top of the thread profile.
   BaseFeatureArray[0] = M_FEATURE_LABEL(40);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(41), M_INNER_FIT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);

   MosPrintf(MIL_TEXT("- A segment is defined by being fit on the top position of the thread\n  (in blue).\n"));

   // Find the tip of the first dent.
   BaseFeatureArray[0] = M_FEATURE_LABEL(40);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(42), M_CLOSEST_TO_INFINITE_POINT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(42), M_ANGLE, 100.0);

   // Build a local frame at the location of the first tip.
   BaseFeatureArray[0] = M_FEATURE_LABEL(41);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(43), M_POSITION_END, BaseFeatureArray, M_NULL, 1, M_DEFAULT);

   BaseFeatureArray[0] = M_FEATURE_LABEL(42);
   BaseFeatureArray[1] = M_FEATURE_LABEL(43);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LOCAL_FRAME, M_FEATURE_LABEL(44), M_CONSTRUCTION, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   MosPrintf(MIL_TEXT("- A local frame (in cyan), aligned with the top segment, is constructed on\n  the first tip of the thread.\n"));

   // Define a segment parallel to the top segment at a fixed distance.
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(45), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(45), M_POSITION_START_X,    0.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(45), M_POSITION_START_Y,   CROSS_SECTION_DISTANCE);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(45), M_POSITION_END_X,     CROSS_SECTION_LENGTH);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(45), M_POSITION_END_Y,     CROSS_SECTION_DISTANCE);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(45), M_REFERENCE_FRAME,    M_FEATURE_LABEL(44));

   BaseFeatureArray[0] = M_FEATURE_LABEL(45);
   BaseFeatureArray[1] = M_FEATURE_LABEL(40);

   // Build the first four points of measurement around the second dent. 
   const MIL_INT NbDentItersections = 4;
   for (MIL_INT ii = 0; ii < NbDentItersections; ii++)
      {
      MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(100+ii), M_INTERSECTION, BaseFeatureArray, M_NULL, 2, M_DEFAULT);
      MmetControl(MilMetrolContext, M_FEATURE_LABEL(100+ii), M_OCCURRENCE, ii);
      }

   MosPrintf(MIL_TEXT("- Cross section points (in red) of the second dent are built at a fixed\n  distance from the top segments.\n"));

   // Define a positional tolerance along the x-axis of the local frame.
   BaseFeatureArray[0] = M_FEATURE_LABEL(44);
   BaseFeatureArray[1] = M_FEATURE_LABEL(101);
   MmetAddTolerance(MilMetrolContext, M_POSITION_X, M_TOLERANCE_LABEL(46), DENT_EDGE_POSITION_MIN, DENT_EDGE_POSITION_MAX, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   MosPrintf(MIL_TEXT("- An x-axis positional tolerance (in yellow) is defined for the point of\n  the rising edge of the dent.\n\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Retrieving the tolerance values.
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(46), M_STATUS + M_TYPE_MIL_INT, &ToleranceStatus[0]);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(46), M_TOLERANCE_VALUE, &ToleranceValue[0]);

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(40), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(40), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(41), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(43), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(44), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(45), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   for (MIL_INT ii = 0; ii < 4; ii++)
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(100+ii), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(46), M_DEFAULT);
   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus[0], ToleranceValue[0], MIL_TEXT("Tolerance position X value"), MIL_TEXT("pixels"), 0);

   MdispZoom(MilDisplay, 2.0, 2.0);
   MdispPan(MilDisplay, 560, 270);
   WaitForKey();

   // Build points at the extremes of the crests and ridges' dents positions.
   for (MIL_INT i = 0; i < NbDentItersections - 1; i++)
      {
      MIL_BOOL RisingEdge = ((i % 2) == 0);

      // Build crest or ridge region.
      MIL_ID DentRegion = MmetAlloc(MilSystem, M_DERIVED_GEOMETRY_REGION, M_NULL);
      MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_GEOMETRY,       M_RECTANGLE);
      MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION_TYPE,  M_LABEL_VALUE);
      MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION,       M_FEATURE_LABEL(RisingEdge ? 100 + i : 100 + i + 1));
      MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_WIDTH_TYPE,     M_LABEL_VALUE);
      MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_WIDTH,          M_FEATURE_LABEL(RisingEdge ? 100 + i + 1 : 100 + i));
      MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_TYPE,     M_LABEL_VALUE);
      MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE,          M_FEATURE_LABEL(RisingEdge ? 100 + i + 1 : 100 + i));
      MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_HEIGHT_TYPE,    M_PARAMETRIC);
      MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_HEIGHT,         50.0);

      // Extract the egdels of the region.
      BaseFeatureArray[0] = M_FEATURE_LABEL(40);
      MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(200 + i), M_CLONE_FEATURE, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(200 + i), M_DEFAULT, M_FROM_DERIVED_GEOMETRY_REGION, DentRegion, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL);
      MmetControl(MilMetrolContext, M_FEATURE_LABEL(200 + i), M_REFERENCE_FRAME, M_FEATURE_LABEL(44));
      MmetFree(DentRegion);

      // Construct the extreme point position.
      BaseFeatureArray[0] = M_FEATURE_LABEL(200 + i);
      MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(300 + i), M_CLOSEST_TO_INFINITE_POINT, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
      MmetControl(MilMetrolContext, M_FEATURE_LABEL(300 + i), M_ANGLE,  (RisingEdge ? 270.0 : 90.0));
      }

   MosPrintf(MIL_TEXT("- Positions at the extremes of the dent ridges and crest are built (in red).\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);
   
   MgraColor(M_DEFAULT, M_COLOR_GRAY);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(40), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(45), M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   for (MIL_INT i = 0; i < NbDentItersections - 1; i++)
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(200 + i), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   for (MIL_INT i = 0; i < NbDentItersections - 1; i++)
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(300 + i), M_DEFAULT);

   MdispZoom(MilDisplay, 4.0, 4.0);
   MdispPan(MilDisplay, 750, 375);
   WaitForKey();

   // Angularity tolerance of the dent is defined.
   BaseFeatureArray[0] = M_FEATURE_LABEL(301);
   BaseFeatureArray[1] = M_FEATURE_LABEL(101);
   BaseFeatureArray[2] = M_FEATURE_LABEL(102);
   MmetAddTolerance(MilMetrolContext, M_ANGULARITY, M_TOLERANCE_LABEL(47), DENT_ANGULARITY_MIN, DENT_ANGULARITY_MAX, BaseFeatureArray, M_NULL, 3, M_DEFAULT);

   MosPrintf(MIL_TEXT("- Angularity of the dent is verified (in yellow).\n\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Retrieving the tolerance value.
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(47), M_STATUS + M_TYPE_MIL_INT, &ToleranceStatus[0]);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(47), M_TOLERANCE_VALUE,         &ToleranceValue[0]);

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(301), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(101), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(102), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(47), M_DEFAULT);
   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus[0], ToleranceValue[0], MIL_TEXT("Angularity value"), MIL_TEXT("degrees"), 0);
   
   MdispZoom(MilDisplay, 4.0, 4.0);
   MdispPan(MilDisplay, 750, 375);
   WaitForKey();
   
   // Define a tight region around the second dent.
   MIL_ID DentRegion = MmetAlloc(MilSystem, M_DERIVED_GEOMETRY_REGION, M_NULL);
   MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_GEOMETRY,      M_RECTANGLE);
   MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION_TYPE, M_LABEL_VALUE);
   MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION,      M_FEATURE_LABEL(302));
   MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_WIDTH_TYPE,    M_LABEL_VALUE);
   MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_WIDTH,         M_FEATURE_LABEL(300));
   MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_TYPE,    M_LABEL_VALUE);
   MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE,         M_FEATURE_LABEL(300));
   MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_HEIGHT_TYPE,   M_PARAMETRIC);
   MmetControl(DentRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_HEIGHT,        50.0);

   // Extract the dent's edgels.
   BaseFeatureArray[0] = M_FEATURE_LABEL(40);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(48), M_CLONE_FEATURE, BaseFeatureArray, M_NULL, 1, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(48), M_DEFAULT, M_FROM_DERIVED_GEOMETRY_REGION, DentRegion, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL);
   MmetFree(DentRegion);

   MosPrintf(MIL_TEXT("- Edgels of the dent are isolated (in red).\n"));

   // Construct the dent base line.
   BaseFeatureArray[0] = M_FEATURE_LABEL(300);
   BaseFeatureArray[1] = M_FEATURE_LABEL(302);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LINE, M_FEATURE_LABEL(49), M_CONSTRUCTION, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   MosPrintf(MIL_TEXT("- The baseline of the dent is constructed (in blue).\n"));

   // Define the tolerance of the area of the dent.
   BaseFeatureArray[0] = M_FEATURE_LABEL(48);
   BaseFeatureArray[1] = M_FEATURE_LABEL(49);
   MmetAddTolerance(MilMetrolContext, M_AREA_UNDER_CURVE_MIN, M_TOLERANCE_LABEL(50), DENT_AREA_MIN, DENT_AREA_MAX, BaseFeatureArray, M_NULL, 2, M_DEFAULT);
   // Accept potential residual positions below the baseline.
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(50), M_AREA_UNDER_CURVE_ALLOW_NEGATIVE, M_ENABLE);

   // Define the tolerance of the perimeter of the dent.
   BaseFeatureArray[0] = M_FEATURE_LABEL(48);
   MmetAddTolerance(MilMetrolContext, M_PERIMETER_SIMPLE, M_TOLERANCE_LABEL(51), DENT_PERIMETER_MIN, DENT_PERIMETER_MAX, BaseFeatureArray, M_NULL, 1, M_DEFAULT);

   MosPrintf(MIL_TEXT("- The area and the perimeter of the dent above the baseline are verified\n(in yellow).\n\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Retrieving the tolerance value.
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(50), M_STATUS + M_TYPE_MIL_INT, &ToleranceStatus[0]);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(50), M_TOLERANCE_VALUE, &ToleranceValue[0]);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(51), M_STATUS + M_TYPE_MIL_INT, &ToleranceStatus[1]);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(51), M_TOLERANCE_VALUE, &ToleranceValue[1]);

   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);
       
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(49), M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(50), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(51), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(48), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(48), M_DEFAULT);

   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus[0], ToleranceValue[0], MIL_TEXT("Area of the dent"), MIL_TEXT("pixels^2"), 0);
   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus[1], ToleranceValue[1], MIL_TEXT("Perimeter of the dent"), MIL_TEXT("pixels"), 1);

   MdispZoom(MilDisplay, 4.0, 4.0);
   MdispPan(MilDisplay, 750, 375);
   WaitForKey();

   // Build a local frame attached to the base of the dent.
   BaseFeatureArray[0] = M_FEATURE_LABEL(300);
   BaseFeatureArray[1] = M_FEATURE_LABEL(302);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LOCAL_FRAME, M_FEATURE_LABEL(53), M_CONSTRUCTION, BaseFeatureArray, M_NULL, 2, M_DEFAULT);

   // Put the reference theoretical dent profile.
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(54), M_EXTERNAL_FEATURE, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(54), M_REFERENCE_FRAME, M_FEATURE_LABEL(53));
   MmetPut(MilMetrolContext, M_FEATURE_LABEL(54), M_DEFAULT, M_NULL, RefDentArrayPosX, RefDentArrayPosY, M_NULL, M_NULL, M_DEFAULT);

   MosPrintf(MIL_TEXT("- The theoretical dent profile aligned at the location of the dent is\nimported (in red).\n"));

   // Verify the area between the the dent profile and the theoretical profile.
   BaseFeatureArray[0] = M_FEATURE_LABEL(48);
   BaseFeatureArray[1] = M_FEATURE_LABEL(54);
   MmetAddTolerance(MilMetrolContext, M_AREA_BETWEEN_CURVES, M_TOLERANCE_LABEL(55), DENT_RESIDUAL_AREA_MIN, DENT_RESIDUAL_AREA_MAX, BaseFeatureArray, M_NULL, 2, M_DEFAULT);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(55), M_CURVE_INFO, M_FEATURE_LABEL(53));

   MosPrintf(MIL_TEXT("- The area residual between the dent (in green) and the theoretical profile\nis verified (in yellow).\n\n"));

   // Metrology calculation.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);
   
   // Retrieving the tolerance value.
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(55), M_STATUS + M_TYPE_MIL_INT, &ToleranceStatus[0]);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(55), M_TOLERANCE_VALUE, &ToleranceValue[0]);
   
   // Clear annotations.
   GraphicalAnnotation.ClearAnnotations(true);

   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(55), M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(48), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(48), M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(53), M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(54), M_DEFAULT);
   
   GraphicalAnnotation.PrintToleranceValue(ToleranceStatus[0], ToleranceValue[0], MIL_TEXT("Residual area between the measured dent and the theoretical dent\nprofile"), MIL_TEXT("pixels^2"), 0);

   MdispZoom(MilDisplay, 4.0, 4.0);
   MdispPan(MilDisplay, 750, 375);
   WaitForKey();
   }


//****************************************************************************
int MosMain()
   {
   PrintHeader();

   // Allocate MIL application main objects.
   MIL_ID MilApplication = MappAlloc(M_DEFAULT, M_NULL);
   MIL_ID MilSystem      = M_DEFAULT_HOST;
   MIL_ID MilDisplay     = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MIL_ID MilGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   
   // Associate the graphic list to the display object.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Allocate Metrology context and result objects.
   MIL_ID MilMetrolContext = MmetAlloc(MilSystem, M_CONTEXT, M_NULL);
   MIL_ID MilMetrolResult  = MmetAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Import then display the source image.
   MIL_ID MilImage = MbufImport(PART_LASER_PROFILE_FILENAME, M_MIL_TIFF, M_RESTORE, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);
   
   //////////////////////////////////////////////
   //// Extracting the laser profile sample. ////
   //////////////////////////////////////////////

   // Allocate a locate peak context and result.
   MIL_ID ImageContext = MimAlloc(MilSystem, M_LOCATE_PEAK_1D_CONTEXT, M_DEFAULT, M_NULL);
   MIL_ID ImageResult  = MimAllocResult(MilSystem, M_DEFAULT, M_LOCATE_PEAK_1D_RESULT, M_NULL);

   // Perform the laser profile extraction.
   MimLocatePeak1d(ImageContext, MilImage, ImageResult, M_DEFAULT, M_DEFAULT, PEAK_MIN_CONTRAST, M_DEFAULT, M_DEFAULT);

   // Retrieve the profile positions 
   std::vector<MIL_DOUBLE> EdgelPositionX;
   std::vector<MIL_DOUBLE> EdgelPositionY;
   MimGetResultSingle(ImageResult, M_ALL, M_ALL, M_PEAK_POSITION_X, EdgelPositionX);
   MimGetResultSingle(ImageResult, M_ALL, M_ALL, M_PEAK_POSITION_Y, EdgelPositionY);

   // Add the laser profile positions in a Metrology measured feature. 
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(1), M_EXTERNAL_FEATURE, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetPut(MilMetrolContext, M_FEATURE_LABEL(1), M_DEFAULT, M_NULL, EdgelPositionX, EdgelPositionY, M_NULL, M_NULL, M_DEFAULT);

   // Release locate peak objects.
   MimFree(ImageContext);
   MimFree(ImageResult);

   ////////////////////////////////////////
   //// Building the Metrology context. ////
   ////////////////////////////////////////

   // Building the localization of the piece and creation of a local frame accordingly.
   PartFeaturesLocation(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList);
   
   // Building the analysis of the slope of the part's neck.
   NeckAnalysis(MilSystem, MilDisplay, MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList);

   // Building the analysis of the gap.
   GapAnalysis(MilDisplay, MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList);
   
   // Building the analysis of the thread.
   ThreadAnalysis(MilSystem, MilDisplay, MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList);

   // Display the complete built context.
   MdispZoom(MilDisplay, 1.0, 1.0);
   MdispPan(MilDisplay, 0, 0);

   MosPrintf(MIL_TEXT("Overview of the complete context.\n"));
   MosPrintf(MIL_TEXT("=================================\n\n"));
   MgraClear(M_DEFAULT, MilGraphicList);
   MgraColor(M_DEFAULT, M_COLOR_DARK_GREEN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_ALL_FEATURES, M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_DARK_BLUE);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_ALL_FEATURES, M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_DARK_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_ALL_TOLERANCES, M_DEFAULT);

   MosPrintf(MIL_TEXT("The context's regions, features, and tolerances are displayed.\n"));

   MosPrintf(MIL_TEXT("\nPress enter to end.\n"));
   MosGetch();

   // Release Metrology objects.
   MmetFree(MilMetrolContext);
   MmetFree(MilMetrolResult);

   // Free objects.
   MbufFree(MilImage);
   MgraFree(MilGraphicList);
   MdispFree(MilDisplay);
   MappFree(MilApplication);

   return 0;
   }
