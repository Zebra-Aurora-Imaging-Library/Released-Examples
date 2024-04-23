//***************************************************************************************/
//
// File name: GearInspection.cpp
//
// Synopsis:  This example illustrates how the metrology tool can be used to measure
//            and verify mechanical parts.
//            See the PrintHeader() function below for detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

// Source image paths.
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("GeneralMetrology/") MIL_TEXT(x))
static MIL_CONST_TEXT_PTR SmallGearImageFilename = EX_PATH("SmallGear.mim");
static MIL_CONST_TEXT_PTR LargeGearImageFilename = EX_PATH("LargeGear.mim");

//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("GearInspection\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example illustrates how the metrology tool can be used to measure\n") 
             MIL_TEXT("and verify mechanical parts such as gears."));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, system, display, buffer, graphic and metrology.\n\n"));
   }

//****************************************************************************
void WaitForKey()
   {
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n\n"));
   MosGetch();
   }

//****************************************************************************
// Gear inspection function.
#define POINT_CONSTRUCTION_ANGLE       30.0  // degrees
#define DISTANCE_FROM_OUTER_CLOSE      50.0  // pixels
#define DISTANCE_FROM_OUTER_FAR        30.0  // pixels
#define MAX_CONCENTRICITY              2.0   // pixels
#define COG_REGION_MARGIN              2.0   // pixels
#define NUMBER_OF_COGS                 1000  // 
#define AVERAGE_COG_PERIMETER          150.0 // pixels
#define COG_PERIMETER_VARIATION        1.0   // 1%
#define COG_INTER_ANGLE_MIN            25.0  // degrees
#define COG_INTER_ANGLE_MAX            25.0  // degrees
#define COG_LOW_POINT_DISTANCE_MIN     207.0 // pixels
#define COG_LOW_POINT_DISTANCE_MAX     209.0 // pixels

void GearInspections(MIL_ID MilSystem, 
                     MIL_ID MilMetrolContext, 
                     MIL_ID MilMetrolResult, 
                     MIL_ID MilImage,
                     MIL_ID MilDisplay,
                     MIL_ID MilGraphicList)
   {
   std::vector<MIL_INT> BaseFeatures(3);
   MIL_ID               DerivedRegion;
   MIL_DOUBLE           ToleranceValue;

   // Clear any previous annotation.
   MgraClear(M_DEFAULT, MilGraphicList);
   
   MosPrintf(MIL_TEXT("The concentricity of the gear is measured and verified:\n\n"));

   // Outer circle fit of the gear contour.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_CIRCLE, M_FEATURE_LABEL(1), M_OUTER_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(1), M_EDGEL_RELATIVE_ANGLE, M_SAME_OR_REVERSE);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(1), M_EDGEL_ANGLE_RANGE,    180.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(1), M_THRESHOLD_MODE,       M_VERY_HIGH);
   MosPrintf(MIL_TEXT("- The outer circular shape of the gear (in red) is fitted using the whole \n  image.\n"));

   // Construct a center point of the gear.
   BaseFeatures[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(2), M_CENTER, BaseFeatures, M_NULL, 1, M_DEFAULT);

   // Construct a point along the outer fitted circle.
   BaseFeatures[0] = M_FEATURE_LABEL(1);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(3), M_ANGLE_ABSOLUTE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(3), M_ANGLE, POINT_CONSTRUCTION_ANGLE);

   // Construct points at a fixed distance from the outer circle, inside the gear.
   BaseFeatures[0] = M_FEATURE_LABEL(3);
   BaseFeatures[1] = M_FEATURE_LABEL(2);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(4), M_CONSTRUCTION, BaseFeatures, M_NULL, 2, M_DEFAULT);

   BaseFeatures[0] = M_FEATURE_LABEL(4);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(5), M_POSITION_ABSOLUTE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(5), M_POSITION, DISTANCE_FROM_OUTER_CLOSE);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(6), M_POSITION_ABSOLUTE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(6), M_POSITION, DISTANCE_FROM_OUTER_FAR);

   // Define a centered ring region arounf the inner contour of the gear.
   DerivedRegion = MmetAlloc(MilSystem, M_DERIVED_GEOMETRY_REGION, M_NULL);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_GEOMETRY,          M_RING);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION_TYPE,     M_LABEL_VALUE);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION,          M_FEATURE_LABEL(2));
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START_TYPE, M_LABEL_VALUE);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START,      M_FEATURE_LABEL(5));
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END_TYPE,   M_LABEL_VALUE);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END,        M_FEATURE_LABEL(6));

   // Build the inner fitted circle.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_CIRCLE, M_FEATURE_LABEL(7), M_INNER_FIT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(7), M_DEFAULT, M_FROM_DERIVED_GEOMETRY_REGION, DerivedRegion, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(7), M_EDGEL_RELATIVE_ANGLE, M_SAME);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(7), M_EDGEL_ANGLE_RANGE, 90.0);
   MmetFree(DerivedRegion);

   MosPrintf(MIL_TEXT("- The inner circular shape of the gear (in red) is fitted in a region\n") 
             MIL_TEXT("  (in blue) relative to the outer fit.\n"));

   // Verify the cocncentricity between the inner and the outer circles.
   BaseFeatures[0] = M_FEATURE_LABEL(1);
   BaseFeatures[1] = M_FEATURE_LABEL(7);
   MmetAddTolerance(MilMetrolContext, M_CONCENTRICITY, M_TOLERANCE_LABEL(1), 0.0, MAX_CONCENTRICITY, BaseFeatures, M_NULL, 2, M_DEFAULT);
   MosPrintf(MIL_TEXT("- The concentricity (in cyan) between the two circular shapes is verified.\n"));

   // Calculating and retrieving the results.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(1), M_TOLERANCE_VALUE, &ToleranceValue);
   MosPrintf(MIL_TEXT("- The concentricity value is: %.3f pixels\n"), ToleranceValue);
   
   // Annotations.
   MgraClear(M_DEFAULT, MilGraphicList);
   MgraColor(M_DEFAULT, M_COLOR_GRAY);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(2), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(3), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(4), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(5), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(7), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_ACTIVE_EDGELS, M_FEATURE_LABEL(7), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(1), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(7), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(1), M_DEFAULT);

   WaitForKey();

   MosPrintf(MIL_TEXT("The cogs of the gear are established and located:\n\n"));

   // Build a point along the inner fitted circle.
   BaseFeatures[0] = M_FEATURE_LABEL(7);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(8), M_ANGLE_ABSOLUTE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(8), M_ANGLE, POINT_CONSTRUCTION_ANGLE);

   // Build an arc region at about half way height of the cogs.
   BaseFeatures[0] = M_FEATURE_LABEL(3);
   BaseFeatures[1] = M_FEATURE_LABEL(8);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(9), M_CENTER, BaseFeatures, M_NULL, 2, M_DEFAULT);

   DerivedRegion = MmetAlloc(MilSystem, M_DERIVED_GEOMETRY_REGION, M_NULL);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_GEOMETRY,            M_ARC);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION_TYPE,       M_LABEL_VALUE);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION,            M_FEATURE_LABEL(2));
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_TYPE,         M_LABEL_VALUE);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS,              M_FEATURE_LABEL(9));
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_START_TYPE,    M_PARAMETRIC);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_START,         0.0);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_END_TYPE,      M_PARAMETRIC);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_END,           360.0);

   MosPrintf(MIL_TEXT("- An oriented circular region (in blue) crossing the cogs is\n  defined.\n"));

   // Build cogs' points along the 1D arc region.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_POINT, M_FEATURE_LABEL(10), M_DEFAULT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(10), M_DEFAULT, M_FROM_DERIVED_GEOMETRY_REGION, DerivedRegion, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_EDGEL_RELATIVE_ANGLE, M_SAME);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_NUMBER_MAX, NUMBER_OF_COGS);
   MmetFree(DerivedRegion);

   MosPrintf(MIL_TEXT("- Intersection points (in red) along the circular region with the\n")
             MIL_TEXT("  contour of the gear's cogs are found.\n"));

   // Calculate and retrieve the results.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   // Annotations.
   MgraClear(M_DEFAULT, MilGraphicList);
   MgraColor(M_DEFAULT, M_COLOR_GRAY);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(3), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(8), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(9), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(10), M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(10), M_DEFAULT);

   MIL_INT NumberOfCogs;
   MmetGetResult(MilMetrolResult, M_FEATURE_LABEL(10), M_NUMBER + M_TYPE_MIL_INT, &NumberOfCogs);
   MosPrintf(MIL_TEXT("- The number of cogs is: %d.\n"), NumberOfCogs);

   WaitForKey();

   MosPrintf(MIL_TEXT("The cogs' inter-angles and minimum distances to center are measured:\n\n"));

   // Construct two points slitghly outside/inside the outer/inner circles.
   BaseFeatures[0] = M_FEATURE_LABEL(3);
   BaseFeatures[1] = M_FEATURE_LABEL(8);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(11), M_CONSTRUCTION, BaseFeatures, M_NULL, 2, M_DEFAULT);
   BaseFeatures[0] = M_FEATURE_LABEL(11);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(12), M_POSITION_ABSOLUTE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(12), M_POSITION, -COG_REGION_MARGIN);

   BaseFeatures[0] = M_FEATURE_LABEL(8);
   BaseFeatures[1] = M_FEATURE_LABEL(3);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(13), M_CONSTRUCTION, BaseFeatures, M_NULL, 2, M_DEFAULT);
   BaseFeatures[0] = M_FEATURE_LABEL(13);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(14), M_POSITION_ABSOLUTE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(14), M_POSITION, -COG_REGION_MARGIN);

   // Build a region containing the gear cogs contour.
   DerivedRegion = MmetAlloc(MilSystem, M_DERIVED_GEOMETRY_REGION, M_NULL);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_GEOMETRY,            M_RING);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION_TYPE,       M_LABEL_VALUE);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION,            M_FEATURE_LABEL(2));
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START_TYPE,   M_LABEL_VALUE);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START,        M_FEATURE_LABEL(14));
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END_TYPE,     M_LABEL_VALUE);
   MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END,          M_FEATURE_LABEL(12));

   // Extract edgels along the cogs.
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_EDGEL, M_FEATURE_LABEL(15), M_DEFAULT, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(15), M_DEFAULT, M_FROM_DERIVED_GEOMETRY_REGION, DerivedRegion, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(15), M_EDGEL_RELATIVE_ANGLE, M_SAME);
   MmetFree(DerivedRegion);

   MosPrintf(MIL_TEXT("- A ring region (in blue) containing the contour of the gears' cogs is\n  defined.\n"));
   
   // Calculate and construct the invidual gear cog intersection points.
   std::vector<MIL_DOUBLE> IntersectionsX;
   std::vector<MIL_DOUBLE> IntersectionsY;
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);
   MmetGetResult(MilMetrolResult, M_FEATURE_LABEL(10), M_POSITION_X, IntersectionsX);
   MmetGetResult(MilMetrolResult, M_FEATURE_LABEL(10), M_POSITION_Y, IntersectionsY);

   for (MIL_INT i = 0; i < NumberOfCogs; ++i)
      {
      MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(100 + i), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
      MmetControl(MilMetrolContext, M_FEATURE_LABEL(100 + i), M_POSITION_X, IntersectionsX[i]);
      MmetControl(MilMetrolContext, M_FEATURE_LABEL(100 + i), M_POSITION_Y, IntersectionsY[i]);
      }
      
   for (MIL_INT i = 0; i < NumberOfCogs; ++i)
      {
      MIL_INT CurLabelOffset  = i;
      MIL_INT NextLabelOffset = ((i + 1) % NumberOfCogs);

      // Adding angularity tolerance between consecutive cogs.
      BaseFeatures[0] = M_FEATURE_LABEL(2);
      BaseFeatures[1] = M_FEATURE_LABEL(100 + CurLabelOffset);
      BaseFeatures[2] = M_FEATURE_LABEL(100 + NextLabelOffset);
      MmetAddTolerance(MilMetrolContext, M_ANGULARITY, M_TOLERANCE_LABEL(100 + CurLabelOffset), COG_INTER_ANGLE_MIN, COG_INTER_ANGLE_MAX, BaseFeatures, M_NULL, M_DEFAULT, M_DEFAULT);

      // Build a region from edge to edge cog.
      DerivedRegion = MmetAlloc(MilSystem, M_DERIVED_GEOMETRY_REGION, M_NULL);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_GEOMETRY,            M_RING_SECTOR);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION_TYPE,       M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION,            M_FEATURE_LABEL(2));
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START_TYPE,   M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START,        M_FEATURE_LABEL(14));
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END_TYPE,     M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END,          M_FEATURE_LABEL(12));
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_START_TYPE,    M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_START,         M_FEATURE_LABEL(100 + CurLabelOffset));
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_END_TYPE,      M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_END,           M_FEATURE_LABEL(100 + NextLabelOffset));

      BaseFeatures[0] = M_FEATURE_LABEL(15);
      MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(200 + CurLabelOffset), M_CLONE_FEATURE, BaseFeatures, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(200 + CurLabelOffset), M_DEFAULT, M_FROM_DERIVED_GEOMETRY_REGION, DerivedRegion, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL);
      MmetFree(DerivedRegion);

      // For each cog section, find the lowest position.
      BaseFeatures[1] = M_FEATURE_LABEL(2);
      BaseFeatures[0] = M_FEATURE_LABEL(200 + i);
      MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(300 + CurLabelOffset), M_CLOSEST, BaseFeatures, M_NULL, 2, M_DEFAULT);

      // Ste tolerance distance between the lowest cog section position aand the center of the gear.
      BaseFeatures[0] = M_FEATURE_LABEL(2);
      BaseFeatures[1] = M_FEATURE_LABEL(300 + CurLabelOffset);
      MmetAddTolerance(MilMetrolContext, M_DISTANCE_MIN, M_TOLERANCE_LABEL(200 + CurLabelOffset), COG_LOW_POINT_DISTANCE_MIN, COG_LOW_POINT_DISTANCE_MAX, BaseFeatures, M_NULL, 2, M_DEFAULT);
      }

   MosPrintf(MIL_TEXT("- Angularity tolerances (in magenta) are set for consecutive pairs of cogs.\n"));
   MosPrintf(MIL_TEXT("- The contours for each individual cog (in red) are isolated.\n"));
   MosPrintf(MIL_TEXT("- The lowest points (in cyan) for each cog are identified.\n"));
   MosPrintf(MIL_TEXT("- The distance from the lowest point to the center (in green) are verified.\n\n"));

   // Calculate and retrieve results.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);
   
   for (MIL_INT i = 0; i < NumberOfCogs; ++i)
      {
      MosPrintf(MIL_TEXT("Cog %2d:\t"), i);
      MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(100 + i), M_TOLERANCE_VALUE + M_TYPE_MIL_DOUBLE, &ToleranceValue);
      MosPrintf(MIL_TEXT("angle: %.3f_deg\t"), ToleranceValue);
      MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(200 + i), M_TOLERANCE_VALUE + M_TYPE_MIL_DOUBLE, &ToleranceValue);
      MosPrintf(MIL_TEXT("distance: %.3f_pix\n"), ToleranceValue);
      }
   
   // Annotations.
   MgraClear(M_DEFAULT, MilGraphicList);
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   MgraColor(M_DEFAULT, M_COLOR_GRAY);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(12), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(13), M_DEFAULT);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(14), M_DEFAULT);

   for (MIL_INT i = 0; i < NumberOfCogs; ++i)
      {
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(200 + i), M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_BLUE);
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(200 + i), M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(100 + i), M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_CYAN);
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE, M_FEATURE_LABEL(300 + i), M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(200 + i), M_DEFAULT);
      }
   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);

   WaitForKey();

   MosPrintf(MIL_TEXT("Determine and validate the area of each cog:\n"));

   for (MIL_INT i = 0; i < NumberOfCogs; ++i)
      {
      MIL_INT CurLabelOffset  = i;
      MIL_INT NextLabelOffset = ((i + 1) % NumberOfCogs);

      // Build a cog's baseline passing through the lowest positions from each sides.
      BaseFeatures[0] = M_FEATURE_LABEL(300 + CurLabelOffset);
      BaseFeatures[1] = M_FEATURE_LABEL(300 + NextLabelOffset);
      MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LINE, M_FEATURE_LABEL(500 + CurLabelOffset), M_CONSTRUCTION, BaseFeatures, M_NULL, 2, M_DEFAULT);


      // Build a region around the cog and limited by the lowest positions from each sides.
      DerivedRegion = MmetAlloc(MilSystem, M_DERIVED_GEOMETRY_REGION, M_NULL);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_GEOMETRY,            M_RING_SECTOR);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION_TYPE,       M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_POSITION,            M_FEATURE_LABEL(2));
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START_TYPE,   M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_START,        M_FEATURE_LABEL(14));
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END_TYPE,     M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_RADIUS_END,          M_FEATURE_LABEL(12));
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_START_TYPE,    M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_START,         M_FEATURE_LABEL(300 + CurLabelOffset));
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_END_TYPE,      M_LABEL_VALUE);
      MmetControl(DerivedRegion, M_DERIVED_GEOMETRY_REGION, M_REGION_ANGLE_END,           M_FEATURE_LABEL(300 + NextLabelOffset));
      
      BaseFeatures[0] = M_FEATURE_LABEL(15);
      MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(400 + CurLabelOffset), M_CLONE_FEATURE, BaseFeatures, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(400 + CurLabelOffset), M_DEFAULT, M_FROM_DERIVED_GEOMETRY_REGION, DerivedRegion, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL);
      MmetFree(DerivedRegion);

      // Verify the cog area above its baseline.
      BaseFeatures[0] = M_FEATURE_LABEL(400 + CurLabelOffset);
      BaseFeatures[1] = M_FEATURE_LABEL(500 + CurLabelOffset);
      MmetAddTolerance(MilMetrolContext, M_AREA_UNDER_CURVE_MIN, M_TOLERANCE_LABEL(300 + CurLabelOffset), 0, 0, BaseFeatures, M_NULL, 2, M_DEFAULT);
      MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(300 + CurLabelOffset), M_AREA_UNDER_CURVE_ALLOW_NEGATIVE, M_ENABLE);
      }

   MosPrintf(MIL_TEXT("- The cog baselines (dashed magenta) are constructed.\n"));
   MosPrintf(MIL_TEXT("- The contours around each individual cog (in blue) are isolated.\n"));
   MosPrintf(MIL_TEXT("- The cog areas (in magenta) above the baselines are measured.\n"));

   // Calculate and retrieve the results.
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   for (MIL_INT i = 0; i < NumberOfCogs; ++i)
      {
      MosPrintf(MIL_TEXT("Cog %2d:\t"), i);
      MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(300 + i), M_TYPE_MIL_DOUBLE, &ToleranceValue);
      MosPrintf(MIL_TEXT("area: %.3f_pix^2\n"), ToleranceValue);
      }

   // Annotations.
   MgraClear(M_DEFAULT, MilGraphicList);
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   for (MIL_INT i = 0; i < NumberOfCogs; ++i)
      {
      MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(300 + i), M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_BLUE);
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_REGION, M_FEATURE_LABEL(400 + i), M_DEFAULT);
      }
   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);

   WaitForKey();
   }


//****************************************************************************
int MosMain()
   {
   PrintHeader();

   MIL_ID MilApplication = MappAlloc(M_DEFAULT, M_NULL);
   MIL_ID MilSystem      = M_DEFAULT_HOST;
   MIL_ID MilDisplay     = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MIL_ID MilGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MIL_ID MilMetrolContext,
          MilMetrolResult,
          MilImage;

   MmetAllocResult(MilSystem, M_DEFAULT, &MilMetrolResult);

   // Measure a small gear.
   MosPrintf(MIL_TEXT("\nMeasuring a small gear.\n")
             MIL_TEXT("=========================\n\n"));
   MmetAlloc(MilSystem, M_CONTEXT, &MilMetrolContext);
   MgraClear(M_DEFAULT, MilGraphicList);
   MbufImport(SmallGearImageFilename, M_MIL_TIFF, M_RESTORE, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);
   GearInspections(MilSystem, MilMetrolContext, MilMetrolResult, MilImage, MilDisplay, MilGraphicList);
   MmetFree(MilMetrolContext);
   MbufFree(MilImage);

   // Measure a large gear.
   MosPrintf(MIL_TEXT("\nMeasuring a large gear.\n")
             MIL_TEXT("=========================\n\n"));
   MmetAlloc(MilSystem, M_CONTEXT, &MilMetrolContext);
   MgraClear(M_DEFAULT, MilGraphicList);
   MbufImport(LargeGearImageFilename, M_MIL_TIFF, M_RESTORE, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);
   GearInspections(MilSystem, MilMetrolContext, MilMetrolResult, MilImage, MilDisplay, MilGraphicList);
   MmetFree(MilMetrolContext);
   MbufFree(MilImage);
   
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n"));
   MosGetch();

   MmetFree(MilMetrolResult);
   MgraFree(MilGraphicList);
   MdispFree(MilDisplay);
   if (MilSystem != M_DEFAULT_HOST)
      { MsysFree(MilSystem); }
   MappFree(MilApplication);

   return 0;
   }
