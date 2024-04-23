﻿//***************************************************************************************/
//
// File name: 3dMetrologyConstructions.cpp
//
// Synopsis:  This program contains an example where planes are fitted and intersected
//            to reconstruct a 3d pyramid using the 3dmet module.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <math.h>
#include <algorithm>

// Source file specification.
static const MIL_STRING PT_CLD_FILE = M_IMAGE_PATH MIL_TEXT("3dMetrologyConstructions/WoodPyramid.mbufc");
static const MIL_STRING ILLUSTRATION_FILE = M_IMAGE_PATH MIL_TEXT("3dMetrologyConstructions/PyramidIllustration.png");
static const MIL_INT ILLUSTRATION_OFFSET_X = 800;

// Pyramid definition.
static const MIL_INT    NB_SIDES = 4;
static const MIL_DOUBLE PLANE_CROP_DISTANCE_FACTOR = 1.1;  // When cropping points on the planes, use slightly more than the fit distance.

// Annotations.
static const MIL_DOUBLE    TOP_FONT_SIZE = 2.5;          // in mm
static const MIL_DOUBLE    BOTTOM_FONT_SIZE = 3.5;       // in mm
static const MIL_INT64     TOP_COLOR = M_COLOR_CYAN;
static const MIL_INT64     SIDE_COLORS[NB_SIDES] = {M_COLOR_RED, M_COLOR_GREEN, M_COLOR_MAGENTA, M_COLOR_YELLOW};
static MIL_CONST_TEXT_PTR  SIDE_COLOR_NAMES[NB_SIDES] = {MIL_TEXT("Red"), MIL_TEXT("Green"), MIL_TEXT("Magenta"), MIL_TEXT("Yellow")};

// Represents the planes that make up a pyramid.
struct SPyramid
   {
   MIL_ID TopFace;
   MIL_ID Background;
   std::vector<MIL_ID> SideFaces;
   };

// Function declarations.
bool                             CheckForRequiredMILFile(const MIL_STRING& FileName);

std::vector<MIL_UNIQUE_3DGEO_ID> FitPlanes(MIL_ID MilPointCloud, MIL_INT NbPlanes, MIL_DOUBLE* FitDistance);
SPyramid                         SortPlanesIntoPyramid(const std::vector<MIL_UNIQUE_3DGEO_ID>& Planes);
void                             RemoveIntersections(MIL_ID Container, const SPyramid& Pyramid, MIL_DOUBLE OutlierDistance);
void                             InspectPyramid(MIL_ID Display, const SPyramid& Pyramid);

void                             DrawPolygonFromPoints(MIL_ID GraphicList, MIL_INT64 Parent, const std::vector<MIL_ID>& Points);
MIL_DOUBLE                       DrawLengthFromPoints(MIL_ID GraphicList, MIL_INT64 Parent, MIL_ID PointA, MIL_ID PointB, MIL_ID Plane, MIL_DOUBLE FontSize, bool Above);
MIL_DOUBLE                       DrawAngleFromPoints(MIL_ID GraphicList, MIL_INT64 Parent, MIL_ID Center, MIL_ID PointA, MIL_ID PointB, MIL_DOUBLE FontSize);

MIL_UNIQUE_3DDISP_ID             Alloc3dDisplayId(MIL_ID MilSystem);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("3dMetrologyConstructions\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to reconstruct features from fitted geometries.\n")
             MIL_TEXT("A plane is fit on each face of the pyramid. The edges are then reconstructed \n")
             MIL_TEXT("and the lengths and angles are computed.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Geometry, 3D Metrology, 3D Image Processing,\n")
             MIL_TEXT("3D Display, Display, Buffer, Graphics, and 3D Graphics.\n\n"));
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Check for required example files.
   if(!CheckForRequiredMILFile(PT_CLD_FILE))
      {
      return 0;
      }
     
   auto MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Show illustration of the object to reconstruct.
   auto IllustrationDispId = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);
   auto IllustrationImageId = MbufRestore(ILLUSTRATION_FILE, MilSystem, M_UNIQUE_ID);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Object to inspect."));
   MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_X, ILLUSTRATION_OFFSET_X);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   // Allocate the display.
   auto Mil3dDisplay = Alloc3dDisplayId(MilSystem);

   // Use the M_FAST transparency sort mode so that metrology annotations
   // which are drawn at the end appear on top.
   M3ddispControl(Mil3dDisplay, M_TRANSPARENCY_SORT_MODE, M_FAST);

   // Restore the point cloud and display it.
   MosPrintf(MIL_TEXT("A 3D point cloud is restored from a ply file and displayed.\n\n"));
   auto MilPointCloud = MbufImport(PT_CLD_FILE, M_DEFAULT, M_RESTORE, MilSystem, M_UNIQUE_ID);
   MbufConvert3d(MilPointCloud, MilPointCloud, M_NULL, M_DEFAULT, M_DEFAULT); // TODO Update the mbufc instead.

   M3ddispSetView(Mil3dDisplay, M_AUTO, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSelect(Mil3dDisplay, MilPointCloud, M_SELECT, M_DEFAULT);

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   // Perform preliminary fits on the 5 visible pyramid faces and the background.
   MIL_DOUBLE FitDistance = M_AUTO_VALUE;
   auto Planes = FitPlanes(MilPointCloud, NB_SIDES + 2, &FitDistance);

   // From the 6 planes, identify the background, top face, and side faces.
   auto Pyramid = SortPlanesIntoPyramid(Planes);

   // Remove the intersections from the point cloud.
   RemoveIntersections(MilPointCloud, Pyramid, FitDistance * PLANE_CROP_DISTANCE_FACTOR);
   MosPrintf(MIL_TEXT("%i preliminary planes are fit on the point cloud.\n"), NB_SIDES + 2);
   MosPrintf(MIL_TEXT("Their intersections are removed to reduce noise in the final fit.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Perform the actual fit without the noise from the intersections.
   Planes = FitPlanes(MilPointCloud, NB_SIDES + 2, &FitDistance);
   Pyramid = SortPlanesIntoPyramid(Planes);

   // Inspect the pyramid and draw the measurements in the 3d display.
   InspectPyramid(Mil3dDisplay, Pyramid);

   return 0;
   }


//*****************************************************************************
// Fit N planes.
//*****************************************************************************
std::vector<MIL_UNIQUE_3DGEO_ID> FitPlanes(MIL_ID MilPointCloud, MIL_INT NbPlanes, MIL_DOUBLE* FitDistance)
   {
   MIL_ID MilSystem = MobjInquire(MilPointCloud, M_OWNER_SYSTEM, M_NULL);

   // Create the fit objects.
   auto FitContext = M3dmetAlloc(MilSystem, M_FIT_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto FitResult = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dmetControl(FitContext, M_EXPECTED_OUTLIER_PERCENTAGE, 70);  // High outlier ratio because the scene isn't just 1 big plane.

   // Create a copy of the point cloud so we can crop it without affecting the display.
   auto FitPointCloud = MbufAllocContainer(MilSystem, M_PROC, M_DEFAULT, M_UNIQUE_ID);
   MbufCopy(MilPointCloud, FitPointCloud);
   MIL_ID FitConfidence = MbufInquireContainer(FitPointCloud, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);

   std::vector<MIL_UNIQUE_3DGEO_ID> Planes(NbPlanes);
   MIL_DOUBLE MaxFitDistance = 0;
   for(auto& Plane : Planes)
      {
      // Fit the planes.
      Plane = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      M3dmetFit(FitContext, FitPointCloud, M_PLANE, FitResult, *FitDistance, M_DEFAULT);
      M3dmetCopyResult(FitResult, Plane, M_FITTED_GEOMETRY, M_DEFAULT);
      MIL_DOUBLE CurrentFitDistance = M3dmetGetResult(FitResult, M_OUTLIER_DISTANCE, M_NULL);
      if(CurrentFitDistance > MaxFitDistance)
         MaxFitDistance = CurrentFitDistance;

      // Exclude the points on the plane from subsequent fits using the outlier mask.
      M3dmetCopyResult(FitResult, FitConfidence, M_OUTLIER_MASK, M_DEFAULT);
      }

   *FitDistance = MaxFitDistance;
   return Planes;
   }

//*****************************************************************************
// Find the top and background planes, and sort the remaining planes counterclockwise.
//*****************************************************************************
SPyramid SortPlanesIntoPyramid(const std::vector<MIL_UNIQUE_3DGEO_ID>& Planes)
   {
   MIL_ID MilSystem = MobjInquire(Planes[0], M_OWNER_SYSTEM, M_NULL);
   SPyramid Pyramid;

   // Find the two most parallel planes. The truncated pyramid is flat, so these are the top and background (potentially swapped).
   MIL_DOUBLE MinAngle = INFINITY;
   for(MIL_UINT i = 0; i < Planes.size(); i++)
      {
      for(MIL_UINT j = i + 1; j < Planes.size(); j++)
         {
         MIL_DOUBLE Angle = M3dmetFeature(Planes[i], Planes[j], M_PARALLELISM, M_DEFAULT, M_NULL);
         if(Angle < MinAngle)
            {
            MinAngle = Angle;
            Pyramid.Background = Planes[i];
            Pyramid.TopFace = Planes[j];
            }
         }
      }

   // The other planes are on the sides.
   for(const auto& Plane : Planes)
      {
      if(Plane != Pyramid.Background && Plane != Pyramid.TopFace)
         Pyramid.SideFaces.push_back(Plane);
      }

   // Orient all the planes so their normal points upwards relative to the background.
   for(const auto& Plane : Planes)
      {
      if(M3dmetFeature(Plane, Pyramid.Background, M_ANGLE, M_DEFAULT, M_NULL) > 90)
         M3dgeoConstruct(Plane, M_NULL, Plane, M_PLANE, M_FLIP, M_DEFAULT, M_DEFAULT); // Flip the plane on itself.
      }

   // Sort the sides counterclockwise.
   auto FixturedPlane = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   auto FixturingMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(FixturingMatrix, M_FIXTURE_TO_PLANE, Pyramid.Background, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   std::sort(Pyramid.SideFaces.begin(), Pyramid.SideFaces.end(), [&FixturingMatrix, &FixturedPlane](MIL_ID PlaneA, MIL_ID PlaneB)
      {
      // Fixture both planes with respect to the background, then compare the angle formed by their normals in x/y.
      M3dimMatrixTransform(PlaneA, FixturedPlane, FixturingMatrix, M_DEFAULT);
      MIL_DOUBLE AngleA = atan2(M3dgeoInquire(FixturedPlane, M_NORMAL_Y, M_NULL), M3dgeoInquire(FixturedPlane, M_NORMAL_X, M_NULL));
      M3dimMatrixTransform(PlaneB, FixturedPlane, FixturingMatrix, M_DEFAULT);
      MIL_DOUBLE AngleB = atan2(M3dgeoInquire(FixturedPlane, M_NORMAL_Y, M_NULL), M3dgeoInquire(FixturedPlane, M_NORMAL_X, M_NULL));
      return AngleA < AngleB;
      });

   // Measure the top and bottom side lengths. The top should be smaller, if it isn't swap it with the background.
   auto CornerA = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   auto CornerB = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);

   M3dmetFeatureEx(M_DEFAULT, Pyramid.TopFace, Pyramid.SideFaces[0], Pyramid.SideFaces[1], CornerA, M_INTERSECTION, M_DEFAULT, M_DEFAULT);
   M3dmetFeatureEx(M_DEFAULT, Pyramid.TopFace, Pyramid.SideFaces[2], Pyramid.SideFaces[3], CornerB, M_INTERSECTION, M_DEFAULT, M_DEFAULT);
   MIL_DOUBLE TopDiag = M3dmetFeature(CornerA, CornerB, M_DISTANCE, M_DEFAULT, M_NULL);

   M3dmetFeatureEx(M_DEFAULT, Pyramid.Background, Pyramid.SideFaces[0], Pyramid.SideFaces[1], CornerA, M_INTERSECTION, M_DEFAULT, M_DEFAULT);
   M3dmetFeatureEx(M_DEFAULT, Pyramid.Background, Pyramid.SideFaces[2], Pyramid.SideFaces[3], CornerB, M_INTERSECTION, M_DEFAULT, M_DEFAULT);
   MIL_DOUBLE BottomDiag = M3dmetFeature(CornerA, CornerB, M_DISTANCE, M_DEFAULT, M_NULL);

   if(TopDiag > BottomDiag)
      std::swap(Pyramid.TopFace, Pyramid.Background);

   // Orient the side planes so they point towards the center of the pyramid.
   auto Center = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dmetFeatureEx(M_DEFAULT, CornerA, CornerB, M_NULL, Center, M_INTERPOLATION, 0.5, M_DEFAULT);
   for(const auto& Face : Pyramid.SideFaces)
      {
      if(M3dmetFeature(Center, Face, M_IS_INSIDE, M_DEFAULT, M_NULL) == M_OUTSIDE)
         M3dgeoConstruct(Face, M_NULL, Face, M_PLANE, M_FLIP, M_DEFAULT, M_DEFAULT); // Flip the plane on itself.
      }

   return Pyramid;
   }


//*****************************************************************************
// Remove points that are part of more than one plane.
//*****************************************************************************
void RemoveIntersections(MIL_ID Container, const SPyramid& Pyramid, MIL_DOUBLE OutlierDistance)
   {
   MIL_ID MilSystem = MobjInquire(Container, M_OWNER_SYSTEM, M_NULL);
   std::vector<MIL_ID> Planes = Pyramid.SideFaces;
   Planes.push_back(Pyramid.Background);
   Planes.push_back(Pyramid.TopFace);

   // Create the necessary buffers.
   MIL_INT SizeX = MbufInquireContainer(Container, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(Container, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   auto Distances = MbufAlloc2d(MilSystem, SizeX, SizeY, M_FLOAT + 32, M_IMAGE + M_PROC, M_UNIQUE_ID);         // Distance from each point to the plane.
   auto OutsidePoints = MbufAlloc2d(MilSystem, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC, M_UNIQUE_ID);   // Determines which points are outside the pyramid.
   auto PlaneCount = MbufAlloc2d(MilSystem, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC, M_UNIQUE_ID);      // Counts the number of planes that are close to each point.
   auto TempBuffer = MbufAlloc2d(MilSystem, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC, M_UNIQUE_ID);
   MbufClear(PlaneCount, 0);
   MbufClear(OutsidePoints, 0);

   for(const auto& Plane : Planes)
      {
      // Get the distances to the plane.
      M3dmetDistance(Container, Plane, Distances, M_SIGNED_DISTANCE_TO_SURFACE, M_DEFAULT, M_DEFAULT);

      // Add the inliers to the plane count.
      MimBinarize(Distances, TempBuffer, M_FIXED + M_IN_RANGE, -OutlierDistance, OutlierDistance);
      MbufClearCond(TempBuffer, 1, M_NULL, M_NULL, TempBuffer, M_NOT_EQUAL, 0);
      MimArith(PlaneCount, TempBuffer, PlaneCount, M_ADD);

      // Check if the points are outside.
      if(Plane != Pyramid.Background && Plane != Pyramid.TopFace)
         {
         MimBinarize(Distances, TempBuffer, M_FIXED + M_LESS, -OutlierDistance, M_NULL);
         MimArith(OutsidePoints, TempBuffer, OutsidePoints, M_OR + M_LOGICAL);
         }
      }

   // Keep all points outside the pyramid, and points inside the pyramid which are part of a single plane.
   MIL_ID Confidence = MbufInquireContainer(Container, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   MimBinarize(PlaneCount, TempBuffer, M_FIXED + M_EQUAL, 1, M_NULL);
   MimArith(OutsidePoints, TempBuffer, Confidence, M_OR + M_LOGICAL);
   }


//*****************************************************************************
// Given the planes that make up the pyramid, display the reconstructed pyramid and calculate it's length and angles.
//*****************************************************************************
void InspectPyramid(MIL_ID Display, const SPyramid& Pyramid)
   {
   MIL_ID GraphicList = (MIL_ID)M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL);
   MIL_ID MilSystem = MobjInquire(GraphicList, M_OWNER_SYSTEM, M_NULL);
   const MIL_INT NbSides = Pyramid.SideFaces.size();

   // Compute the 8 corners of the pyramid by intersecting the planes.
   std::vector<MIL_UNIQUE_3DGEO_ID> TopCorners(NbSides), BottomCorners(NbSides);
   for(MIL_INT i = 0; i < NbSides; i++)
      {
      TopCorners[i] = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      BottomCorners[i] = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      M3dmetFeatureEx(M_DEFAULT, Pyramid.SideFaces[i], Pyramid.SideFaces[(i + 1) % NbSides], Pyramid.TopFace, TopCorners[i], M_INTERSECTION, M_DEFAULT, M_DEFAULT);
      M3dmetFeatureEx(M_DEFAULT, Pyramid.SideFaces[i], Pyramid.SideFaces[(i + 1) % NbSides], Pyramid.Background, BottomCorners[i], M_INTERSECTION, M_DEFAULT, M_DEFAULT);
      }

   // Display the reconstructed pyramid in the 3d display.
   M3ddispControl(Display, M_UPDATE, M_DISABLE);
   M3dgraControl(GraphicList, M_ROOT_NODE, M_OPACITY + M_RECURSIVE, 99); // Add a tiny bit of transparency so we can see the recustruction underneath.
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_OPACITY, 50);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_APPEARANCE, M_SOLID_WITH_WIREFRAME);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_COLOR, M_COLOR_BLACK);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_THICKNESS, 5);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_FILL_COLOR, TOP_COLOR);
   DrawPolygonFromPoints(GraphicList, M_ROOT_NODE, {TopCorners[0], TopCorners[1], TopCorners[2], TopCorners[3]});
   for(MIL_INT i = 0; i < NbSides; i++)
      {
      M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_FILL_COLOR, SIDE_COLORS[i]);
      DrawPolygonFromPoints(GraphicList, M_ROOT_NODE, {TopCorners[i], TopCorners[(i + 1) % NbSides], BottomCorners[(i + 1) % NbSides], BottomCorners[i]});
      }
   M3ddispControl(Display, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("The planes are fit again without the noise from the intersections.\n"));
   MosPrintf(MIL_TEXT("The new intersections are used to reconstruct the pyramid.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // For each side, draw some measurements.
   std::vector<MIL_DOUBLE> TopLengths(NbSides);
   std::vector<MIL_DOUBLE> BottomLengths(NbSides);
   std::vector<MIL_DOUBLE> TopAngles(NbSides);
   std::vector<MIL_DOUBLE> BottomAngles(NbSides);
   std::vector<MIL_DOUBLE> SideAngles(NbSides);

   M3ddispControl(Display, M_UPDATE, M_DISABLE);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_OPACITY, 99);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_THICKNESS, 3);
   for(MIL_INT i = 0; i < NbSides; i++)
      {
      // Draw the length of each top and bottom side.
      TopLengths[i] = DrawLengthFromPoints(GraphicList, M_ROOT_NODE, TopCorners[i], TopCorners[(i + 1) % NbSides], Pyramid.TopFace, TOP_FONT_SIZE, true);
      BottomLengths[i] = DrawLengthFromPoints(GraphicList, M_ROOT_NODE, BottomCorners[i], BottomCorners[(i + 1) % NbSides], Pyramid.Background, BOTTOM_FONT_SIZE, false);

      // Draw the 4 angles on the top and bottom faces of the pyramid.
      TopAngles[i] = DrawAngleFromPoints(GraphicList, M_ROOT_NODE, TopCorners[i], TopCorners[(i + 1) % NbSides], TopCorners[(i + NbSides - 1) % NbSides], TOP_FONT_SIZE);
      BottomAngles[i] = DrawAngleFromPoints(GraphicList, M_ROOT_NODE, BottomCorners[i], BottomCorners[(i + 1) % NbSides], BottomCorners[(i + NbSides - 1) % NbSides], BOTTOM_FONT_SIZE);

      // Draw the angle each side makes with the background.
      // For just computing the angle, M3dmetFeatureEx(M_DEFAULT, M_ANGLE) is sufficient.
      // However, displaying the angle requires 3 points, so do it using the bottom midpoint, the top midpoint, and the top midpoint's projection.
      // This isn't the exact angle like M3dmetFeatureEx(M_DEFAULT, M_ANGLE), but as long as the pyramid is not too skewed it's a close enough approximation.
      auto TopMidPoint = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      M3dmetFeatureEx(M_DEFAULT, TopCorners[i], TopCorners[(i + 1) % NbSides], M_NULL, TopMidPoint, M_INTERPOLATION, 0.5, M_DEFAULT);

      auto BottomMidPoint = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      M3dmetFeatureEx(M_DEFAULT, BottomCorners[i], BottomCorners[(i + 1) % NbSides], M_NULL, BottomMidPoint, M_INTERPOLATION, 0.5, M_DEFAULT);

      auto Projection = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      M3dmetFeatureEx(M_DEFAULT, TopMidPoint, Pyramid.Background, M_NULL, Projection, M_PROJECTION, M_DEFAULT, M_DEFAULT);

      SideAngles[i] = DrawAngleFromPoints(GraphicList, M_ROOT_NODE, BottomMidPoint, Projection, TopMidPoint, BOTTOM_FONT_SIZE);
      }

   // Find the center of the top face by averaging the 4 top corners.
   auto TopCenter = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoCopy(TopCorners[0], TopCenter, M_GEOMETRY, M_DEFAULT);
   for(MIL_INT i = 1; i < NbSides; i++)
      {
      M3dmetFeatureEx(M_DEFAULT, TopCenter, TopCorners[i], M_NULL, TopCenter, M_INTERPOLATION, 1.0 / (i + 1), M_DEFAULT);
      }

   // Project the top center on the background to get the pyramid's height.
   auto Projection = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dmetFeatureEx(M_DEFAULT, TopCenter, Pyramid.Background, M_NULL, Projection, M_PROJECTION, M_DEFAULT, M_DEFAULT);
   MIL_DOUBLE Height = DrawLengthFromPoints(GraphicList, M_ROOT_NODE, TopCenter, Projection, M_NULL, BOTTOM_FONT_SIZE, true);

   // Calculate the angle between the top and the background.
   MIL_DOUBLE BackroundAngle = M3dmetFeature(Pyramid.Background, Pyramid.TopFace, M_PARALLELISM, M_DEFAULT, M_NULL);

   M3ddispControl(Display, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("The pyramid's angles and dimensions are measured:\n"));

   MosPrintf(MIL_TEXT("\nPyramid height (mm):  \t%.2f"), Height);
   MosPrintf(MIL_TEXT("\nTop angle (deg):      \t%.2f\n"), BackroundAngle);

   MosPrintf(MIL_TEXT("\nSide:                 \t"));
   for(const auto& Color : SIDE_COLOR_NAMES)
      MosPrintf(MIL_TEXT("%s\t"), Color);

   MosPrintf(MIL_TEXT("\nTop lengths (mm):     \t"));
   for(const auto& Length : TopLengths)                                   
      MosPrintf(MIL_TEXT("%.2f\t"), Length);                              
                                                                          
   MosPrintf(MIL_TEXT("\nBottom lengths (mm):  \t"));
   for(const auto& Length : BottomLengths)                                
      MosPrintf(MIL_TEXT("%.2f\t"), Length);                              
                                                                          
   MosPrintf(MIL_TEXT("\nTop angles (deg):     \t"));
   for(const auto& Angle : TopAngles)
      MosPrintf(MIL_TEXT("%.1f\t"), Angle);

   MosPrintf(MIL_TEXT("\nBottom angles (deg):  \t"));
   for(const auto& Angle : BottomAngles)
      MosPrintf(MIL_TEXT("%.1f\t"), Angle);

   MosPrintf(MIL_TEXT("\nSide angles (deg):    \t"));
   for(const auto& Angle : SideAngles)
      MosPrintf(MIL_TEXT("%.1f\t"), Angle);

   MosPrintf(MIL_TEXT("\n\nPress <Enter> to end.\n\n"));
   MosGetch();

   }


//*****************************************************************************
// Small wrapper around M3dgraPolyon to interface with lists of 3dgeo points.
//*****************************************************************************
void DrawPolygonFromPoints(MIL_ID GraphicList, MIL_INT64 Parent, const std::vector<MIL_ID>& Points)
   {
   std::vector<MIL_DOUBLE> x(Points.size()), y(Points.size()), z(Points.size());

   for(MIL_UINT i = 0; i < Points.size(); i++)
      {
      x[i] = M3dgeoInquire(Points[i], M_POSITION_X, M_NULL);
      y[i] = M3dgeoInquire(Points[i], M_POSITION_Y, M_NULL);
      z[i] = M3dgeoInquire(Points[i], M_POSITION_Z, M_NULL);
      }
   M3dgraPolygon(GraphicList, Parent, M_DEFAULT, M_DEFAULT, x, y, z, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   }

//*****************************************************************************
// Draws the line from point A to B, as well as its length.
// The text is drawn in the provided plane if there is one.
// Returns the length.
//*****************************************************************************
MIL_DOUBLE DrawLengthFromPoints(MIL_ID GraphicList, MIL_INT64 Parent, MIL_ID A, MIL_ID B, MIL_ID Plane, MIL_DOUBLE FontSize, bool Above)
   {
   MIL_ID MilSystem = MobjInquire(GraphicList, M_OWNER_SYSTEM, M_NULL);

   // Draw the line.
   auto Line = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoConstruct(A, B, Line, M_LINE, M_TWO_POINTS, M_DEFAULT, M_DEFAULT);
   M3dgeoDraw3d(M_DEFAULT, Line, GraphicList, Parent, M_DEFAULT);

   // Plane is the plane in which the text will be drawn.
   // If one isn't provided, choose a random plane that contains the line.
   MIL_UNIQUE_3DGEO_ID RandomPlane;
   if(Plane == M_NULL)
      {
      auto Origin = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      RandomPlane = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      M3dgeoPoint(Origin, 0, 0, 0, M_DEFAULT);
      M3dgeoConstruct(Line, Origin, RandomPlane, M_PLANE, M_LINE_AND_POINT, M_DEFAULT, M_DEFAULT);
      Plane = RandomPlane;
      }

   // Create the text graphic.
   MIL_TEXT_CHAR Text[20];
   MIL_DOUBLE Length = M3dgeoInquire(Line, M_LENGTH, M_NULL);
   MosSprintf(Text, 20, MIL_TEXT("%.2f mm"), Length);

   auto Mat = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   auto Translation = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetWithAxes(Mat,
                           M_ZX_AXES,
                           M3dgeoInquire(Line, M_CENTER_X, M_NULL),
                           M3dgeoInquire(Line, M_CENTER_Y, M_NULL),
                           M3dgeoInquire(Line, M_CENTER_Z, M_NULL),
                           M3dgeoInquire(Plane, M_NORMAL_X, M_NULL),
                           M3dgeoInquire(Plane, M_NORMAL_Y, M_NULL),
                           M3dgeoInquire(Plane, M_NORMAL_Z, M_NULL),
                           M3dgeoInquire(B, M_POSITION_X, M_NULL) - M3dgeoInquire(A, M_POSITION_X, M_NULL),
                           M3dgeoInquire(B, M_POSITION_Y, M_NULL) - M3dgeoInquire(A, M_POSITION_Y, M_NULL),
                           M3dgeoInquire(B, M_POSITION_Z, M_NULL) - M3dgeoInquire(A, M_POSITION_Z, M_NULL),
                           M_DEFAULT);
   M3dgeoMatrixSetTransform(Translation, M_TRANSLATION, 0, FontSize * 0.3 * (Above ? 1 : -1), 0, M_DEFAULT, M_DEFAULT);
   M3dgeoMatrixSetTransform(Mat, M_COMPOSE_TWO_MATRICES, Mat, Translation, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_FONT_SIZE, FontSize);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_TEXT_ALIGN_VERTICAL, Above ? M_BOTTOM : M_TOP);
   M3dgraText(GraphicList, Parent, Text, Mat, M_DEFAULT, M_DEFAULT);

   return Length;
   }

//*****************************************************************************
// Draws the arc from A to B around Center, as well as its angle.
// Returns the angle.
//*****************************************************************************
MIL_DOUBLE DrawAngleFromPoints(MIL_ID GraphicList, MIL_INT64 Parent, MIL_ID Center, MIL_ID A, MIL_ID B, MIL_DOUBLE FontSize)
   {
   MIL_ID MilSystem = MobjInquire(GraphicList, M_OWNER_SYSTEM, M_NULL);

   // Create the line graphics.
   auto LineA = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   auto LineB = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoConstruct(Center, A, LineA, M_LINE, M_TWO_POINTS, M_DEFAULT, M_DEFAULT);
   M3dgeoConstruct(Center, B, LineB, M_LINE, M_TWO_POINTS, M_DEFAULT, M_DEFAULT);
   M3dgeoLine(LineA, M_POINT_AND_VECTOR, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, FontSize * 1.2, M_DEFAULT);
   M3dgeoLine(LineB, M_POINT_AND_VECTOR, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, FontSize * 1.2, M_DEFAULT);
   M3dgeoDraw3d(M_DEFAULT, LineA, GraphicList, Parent, M_DEFAULT);
   M3dgeoDraw3d(M_DEFAULT, LineB, GraphicList, Parent, M_DEFAULT);

   // Create the arc graphic.
   MIL_INT64 Arc = M3dgraArc(GraphicList,
                             Parent,
                             M_CENTER_AND_TWO_POINTS,
                             M_DEFAULT,
                             M3dgeoInquire(Center, M_POSITION_X, M_NULL),
                             M3dgeoInquire(Center, M_POSITION_Y, M_NULL),
                             M3dgeoInquire(Center, M_POSITION_Z, M_NULL),
                             M3dgeoInquire(A, M_POSITION_X, M_NULL),
                             M3dgeoInquire(A, M_POSITION_Y, M_NULL),
                             M3dgeoInquire(A, M_POSITION_Z, M_NULL),
                             M3dgeoInquire(B, M_POSITION_X, M_NULL),
                             M3dgeoInquire(B, M_POSITION_Y, M_NULL),
                             M3dgeoInquire(B, M_POSITION_Z, M_NULL),
                             M_SMALLEST_ANGLE,
                             M_DEFAULT);
   M3dgraControl(GraphicList, Arc, M_RADIUS, FontSize * 1.2);

   // Create the text graphic.
   MIL_DOUBLE Angle;
   MIL_TEXT_CHAR Text[20];
   M3dgraInquire(GraphicList, Arc, M_ANGLE, &Angle);
   MosSprintf(Text, 20, MIL_TEXT("%.1f°"), Angle);

   auto Mat = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(Mat, M_TRANSLATION, 0, FontSize * 1.5, 0, M_DEFAULT, M_DEFAULT);
   M3dgeoMatrixSetTransform(Mat, M_ROTATION_Z, Angle / 2 - 90, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_FONT_SIZE, FontSize);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
   M3dgraControl(GraphicList, M_DEFAULT_SETTINGS, M_TEXT_ALIGN_VERTICAL, M_BOTTOM);
   M3dgraText(GraphicList, Arc, Text, Mat, M_DEFAULT, M_DEFAULT);

   return Angle;
   }

//****************************************************************************
// Check for required files to run the example.    
//****************************************************************************
bool CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.  
//*****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to exit.\n"));
      MosGetch();
      exit(0);
      }
   return MilDisplay3D;
   }
