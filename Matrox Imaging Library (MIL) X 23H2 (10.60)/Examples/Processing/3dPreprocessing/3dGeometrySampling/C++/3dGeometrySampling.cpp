//******************************************************************************
// 
// File name: 3dGeometrySampling.cpp
//
// Synopsis:  Demonstrates how to get samples on 3d geometries.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//******************************************************************************
#include <mil.h>

//******************************************************************************
// Utility structures.
//******************************************************************************
/**
 * 3D Point structure.
 */
template <typename NumType>
struct Point3D
   {
   NumType x;
   NumType y;
   NumType z;
   };

/**
 * 3D Vector structure.
 */
template <typename NumType>
struct Vector3D
   {
   NumType x;
   NumType y;
   NumType z;
   };

/**
 * Display info structure.
 */
struct Display3DInfo
   {
   MIL_INT64 AxisLabel;
   MIL_INT64 GridLabel;
   };

//******************************************************************************
// Functions' declaration.
//******************************************************************************
void PrintHeader();

void WaitForKey();

MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);

void SetGraphicFormat(MIL_ID GraphicsList, MIL_INT64 GraphicsLabel, MIL_INT Color,
                                                                    MIL_INT Thickness,
                                                                    MIL_INT Opacity);

Display3DInfo ShowDisplay(MIL_ID MilSystem, MIL_ID Mil3dDisp, MIL_ID MilGraList,
                          MIL_DOUBLE ViewX, MIL_DOUBLE ViewY, MIL_DOUBLE ViewZ);

void RepositionDisplay(MIL_ID MilSystem, MIL_ID MilGraList, Display3DInfo& DisplayInfo);

void CurveExample(MIL_ID MilSystem, MIL_ID Mil3dDisp);

void SurfaceExample(MIL_ID MilSystem, MIL_ID Mil3dDisp);

void SphereSurfaceExample(MIL_ID MilSystem, MIL_ID Mil3dDisp);

void PlaneSurfaceExample(MIL_ID MilSystem, MIL_ID Mil3dDisp);

void UniformSamplingExample(MIL_ID MilSystem, MIL_ID Mil3dDisp);

//******************************************************************************
// Constants.
//******************************************************************************
static const MIL_DOUBLE AXIS_LENGTH       = 250.0;
static const MIL_DOUBLE GRID_DISPLACEMENT = 0.4 * AXIS_LENGTH;
static const MIL_DOUBLE GRID_SIZE         = 0.8 * AXIS_LENGTH;
static const MIL_DOUBLE GRID_SPACING      = 16.0;

static const MIL_DOUBLE VERT_GRID_SIZE    = 1.6 * AXIS_LENGTH;
static const MIL_DOUBLE VERT_GRID_SPACING = 30.0;

static const MIL_DOUBLE SAMPLING_RESOLUTION = 20;

//*******************************************************************************
// Prints the example's description.
//*******************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("3dgeoSampling\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to get samples from 3D geometries.\n")
             MIL_TEXT("Sparse samples can be obtained using M3dgeoEvalCurve and ")
             MIL_TEXT("M3dgeoEvalSurface,\n")
             MIL_TEXT("which evaluate missing coordinates of points on a given curve or ")
             MIL_TEXT("surface.\n")
             MIL_TEXT("Uniform samples can be obtained using M3dimSample,\n")
             MIL_TEXT("which computes evenly distributed points on a given finite curve or ")
             MIL_TEXT("surface.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Buffer, 3D Display, 3D Graphics, \n")
             MIL_TEXT("3D Geometry and 3D Image Processing\n\n"));
   }

//*******************************************************************************
// Main function.
//*******************************************************************************
int MosMain(void)
   {
   // Print the example's header
   PrintHeader();
   WaitForKey();

   // Allocate a MIL Application, System, and 3D display
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   auto Mil3dDisp = Alloc3dDisplayId(MilSystem);

   // Run the example
   CurveExample(MilSystem, Mil3dDisp);
   SurfaceExample(MilSystem, Mil3dDisp);
   UniformSamplingExample(MilSystem, Mil3dDisp);

   return 0; // No error
   }

//*******************************************************************************
// Demonstrates how to use M3dgeoEvalCurve.
//*******************************************************************************
void CurveExample(MIL_ID MilSystem, MIL_ID Mil3dDisp)
   {
   // Allocate a 3D graphic list
   auto GraList = (MIL_ID) M3ddispInquire(Mil3dDisp, M_3D_GRAPHIC_LIST_ID, M_NULL);
   
   // Show the display
   auto DisplayInfo = ShowDisplay(MilSystem, Mil3dDisp, GraList, -3.1, -1.0, -2.0);
 
   // Create and draw a 3D line
   auto MilLineGeo = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   const Point3D<MIL_DOUBLE> LineStart = { 150.0, -10.5, 30.0};
   const Vector3D<MIL_DOUBLE> LineVec  = {-200.0, 135.0, 180.0};
   const MIL_DOUBLE LineLength = M_DEFAULT; // length of line will be inferred 
   M3dgeoLine(MilLineGeo, M_POINT_AND_VECTOR, LineStart.x, LineStart.y, LineStart.z, 
                                              LineVec.x, LineVec.y, LineVec.z, 
                                              LineLength, M_DEFAULT);
   auto LineGraLabel = M3dgeoDraw3d(M_DEFAULT, MilLineGeo, GraList,
                                    M_DEFAULT, M_DEFAULT);
   SetGraphicFormat(GraList, LineGraLabel, M_COLOR_RED, 3, 100);
   
   MosPrintf(MIL_TEXT("Given an input coordinate of a point on the line,\n"));
   MosPrintf(MIL_TEXT("the other 2 coordinates of the point can be found. \n\n"));
   WaitForKey();

   // Find the missing Y- and Z-coordinates of a point on the line
   // and draw the point
   const MIL_DOUBLE PointX = 37.;
   MIL_DOUBLE PointY;
   MIL_DOUBLE PointZ;
   MIL_INT NbValidPoints = M3dgeoEvalCurve(MilLineGeo, M_EVAL_YZ, 1,
                                           &PointX, &PointY, &PointZ, M_DEFAULT);
   if (NbValidPoints == 1)
      {
      const MIL_DOUBLE Xsteps[] = {PointX, PointX, PointX};
      const MIL_DOUBLE Ysteps[] = {0.0   , PointY, PointY};
      const MIL_DOUBLE Zsteps[] = {0.0   , 0.0   , PointZ};
      auto StepDotsGraLabel = M3dgraDots(GraList, M_DEFAULT, sizeof(Xsteps)/sizeof(MIL_DOUBLE),
                                         Xsteps, Ysteps, Zsteps,
                                         M_NULL, M_NULL, M_NULL, M_DEFAULT); 
      SetGraphicFormat(GraList, StepDotsGraLabel, M_COLOR_YELLOW, 5, 100);

      auto StepLine1GraLabel = M3dgraLine(GraList, M_DEFAULT, M_TWO_POINTS, M_DEFAULT,
                                          Xsteps[0], Ysteps[0], Zsteps[0], 
                                          Xsteps[1], Ysteps[1], Zsteps[1],
                                          M_DEFAULT, M_DEFAULT);
      auto StepLine2GraLabel = M3dgraLine(GraList, M_DEFAULT, M_TWO_POINTS, M_DEFAULT, 
                                          Xsteps[1], Ysteps[1], Zsteps[1], 
                                          Xsteps[2], Ysteps[2], Zsteps[2],
                                          M_DEFAULT, M_DEFAULT);
      SetGraphicFormat(GraList, StepLine1GraLabel, M_COLOR_YELLOW, 3, 30);
      SetGraphicFormat(GraList, StepLine2GraLabel, M_COLOR_YELLOW, 3, 30);

      MosPrintf(MIL_TEXT("For instance, given a point on the line whose ")
                MIL_TEXT("X-coordinate is known,\n"));
      MosPrintf(MIL_TEXT("M3dgeoEvalCurve will evaluate its Y- and Z-coordinates.\n"));
      MosPrintf(MIL_TEXT("This point is displayed and tabulated below.\n\n"));
      MosPrintf(MIL_TEXT("   X         Y         Z\n"));
      MosPrintf(MIL_TEXT("%7.3f   %7.3f   %7.3f \n\n"), PointX, PointY, PointZ);

      WaitForKey();

      M3dgraRemove(GraList, StepDotsGraLabel,  M_DEFAULT);
      M3dgraRemove(GraList, StepLine1GraLabel, M_DEFAULT);
      M3dgraRemove(GraList, StepLine2GraLabel, M_DEFAULT);
      }

   // Reposition the display
   RepositionDisplay(MilSystem, GraList, DisplayInfo);

   // Find the missing X- and Y-coordinates of multiple points on the line
   // and draw the points
   const MIL_DOUBLE PointsZ[] = {70.0, 103.5, 143.0, 189.0};
   const MIL_UINT NbPoints = sizeof(PointsZ) / sizeof(MIL_DOUBLE);
   MIL_DOUBLE PointsX[5];
   MIL_DOUBLE PointsY[5];
   NbValidPoints = M3dgeoEvalCurve(MilLineGeo, M_EVAL_XY, NbPoints,
                                   PointsZ, PointsX, PointsY, M_DEFAULT);
   if (NbValidPoints == (MIL_INT) NbPoints)
      {
      const std::vector<MIL_DOUBLE> Zeros(NbPoints, 0.0);
      auto DotsGraLabel = M3dgraDots(GraList, M_DEFAULT, NbPoints, &Zeros[0], &Zeros[0],
                                     PointsZ, M_NULL, M_NULL, M_NULL, M_DEFAULT);
      SetGraphicFormat(GraList, DotsGraLabel, M_COLOR_YELLOW, 5, 100);

      auto LineDotsGraLabel = M3dgraDots(GraList, M_DEFAULT, NbPoints,
                                         PointsX, PointsY, PointsZ,
                                         M_NULL, M_NULL, M_NULL, M_DEFAULT); 
      SetGraphicFormat(GraList, LineDotsGraLabel, M_COLOR_YELLOW, 5, 100);

      for (unsigned int i = 0; i < NbPoints; ++i)
         {
         auto StepLineGraLabel = M3dgraLine(GraList, M_DEFAULT, M_TWO_POINTS, M_DEFAULT, 
                                            0.0, 0.0, PointsZ[i], PointsX[i], 
                                            PointsY[i], PointsZ[i], M_DEFAULT, M_DEFAULT);
         SetGraphicFormat(GraList, StepLineGraLabel, M_COLOR_YELLOW, 3, 30);
         }

      MosPrintf(MIL_TEXT("M3dgeoEvalCurve can also simultaneously evaluate the ")
                MIL_TEXT("missing coordinates\n"));
      MosPrintf(MIL_TEXT("of a list of points.\n"));
      MosPrintf(MIL_TEXT("Given a list of Z-coordinates, the missing X- and Y-coordinates ")
                MIL_TEXT("of the points\n")); 
      MosPrintf(MIL_TEXT("can be found.\n"));
      MosPrintf(MIL_TEXT("These points are displayed and tabulated below.\n\n"));
      MosPrintf(MIL_TEXT("   X         Y         Z\n"));
     
      for (unsigned int i = 0; i < NbPoints; ++i)
         {
         MosPrintf(MIL_TEXT("%7.3f   %7.3f   %7.3f \n"), PointsX[i], PointsY[i], PointsZ[i]);
         }
      MosPrintf(MIL_TEXT("\n"));

      WaitForKey();
      }
   
   // Remove all models from graphics list
   M3dgraRemove(GraList, M_ALL, M_DEFAULT);
   }

//*******************************************************************************
// Demonstrates how to use M3dgeoEvalSurface.
//*******************************************************************************
void SurfaceExample(MIL_ID MilSystem, MIL_ID Mil3dDisp)
   {
   SphereSurfaceExample(MilSystem, Mil3dDisp);

   PlaneSurfaceExample(MilSystem, Mil3dDisp);
   }

//*******************************************************************************
// Demonstrates how to use M3dgeoEvalSurface with spheres.
//*******************************************************************************
void SphereSurfaceExample(MIL_ID MilSystem, MIL_ID Mil3dDisp)
   {
   // Allocate a 3D graphic list
   auto GraList = (MIL_ID) M3ddispInquire(Mil3dDisp, M_3D_GRAPHIC_LIST_ID, M_NULL);
   
   // Show display 
   auto DisplayInfo = ShowDisplay(MilSystem, Mil3dDisp, GraList, -4.0, -5.0, -3.0);

   // Create and draw sphere
   auto MilSphereGeo = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   const Point3D<MIL_DOUBLE> Center = {75.0, 115.0, 165.0};
   const MIL_DOUBLE Radius = 47.0;
   M3dgeoSphere(MilSphereGeo, Center.x, Center.y, Center.z, Radius, M_DEFAULT);
   auto SphereGraLabel = M3dgeoDraw3d(M_DEFAULT, MilSphereGeo, GraList,
                                      M_DEFAULT, M_DEFAULT);
   SetGraphicFormat(GraList, SphereGraLabel, M_COLOR_RED, 3, 30);

   MosPrintf(MIL_TEXT("Given partial coordinates of points on the sphere's surface,\n"));
   MosPrintf(MIL_TEXT("the missing ones can be evaluated.\n\n"));
   WaitForKey();

   // Find the missing Y- and Z-coordinates of a point on the sphere
   // and draw the point
   const MIL_DOUBLE PointX = 80.0;
   const MIL_DOUBLE PointY = 130.0;
   MIL_DOUBLE PointZMax;
   MIL_INT NbValidPoints = M3dgeoEvalSurface(MilSphereGeo, M_EVAL_Z + M_MAX_VALUE, 1,
                                             &PointX, &PointY, &PointZMax, M_DEFAULT);
   MIL_DOUBLE PointZMin;
   NbValidPoints += M3dgeoEvalSurface(MilSphereGeo, M_EVAL_Z + M_MIN_VALUE, 1,
                                      &PointX, &PointY, &PointZMin, M_DEFAULT);

   if (NbValidPoints == 2)
      {
      const MIL_DOUBLE Xsteps[] = {PointX, PointX};
      const MIL_DOUBLE Ysteps[] = {PointY, PointY};
      const MIL_DOUBLE Zsteps[] = {0.0   , PointZMax};
      auto StepDots1GraLabel = M3dgraDots(GraList, M_DEFAULT,
                                          sizeof(Xsteps)/sizeof(MIL_DOUBLE),
                                          Xsteps, Ysteps, Zsteps,
                                          M_NULL, M_NULL, M_NULL, M_DEFAULT); 
      SetGraphicFormat(GraList, StepDots1GraLabel, M_COLOR_YELLOW, 5, 100);

      auto StepLineGraLabel = M3dgraLine(GraList, M_DEFAULT, M_TWO_POINTS, M_DEFAULT, 
                                         Xsteps[0], Ysteps[0], Zsteps[0], 
                                         Xsteps[1], Ysteps[1], Zsteps[1], 
                                         M_DEFAULT, M_DEFAULT);
      SetGraphicFormat(GraList, StepLineGraLabel, M_COLOR_YELLOW, 3, 30);

 
      auto StepDots2GraLabel = M3dgraDots(GraList, M_DEFAULT, 1,
                                          &PointX, &PointY, &PointZMin,
                                         M_NULL, M_NULL, M_NULL, M_DEFAULT);
      SetGraphicFormat(GraList, StepDots2GraLabel, M_COLOR_YELLOW, 5, 100);

      MosPrintf(MIL_TEXT("For instance, given X- and Y-coordinates, ")
                MIL_TEXT("M3dgeoEvalSurface will evaluate the\n")); 
      MosPrintf(MIL_TEXT("Z-coordinates of both points on the sphere's surface.\n"));
      MosPrintf(MIL_TEXT("Both points are displayed and tabulated below.\n\n"));

      MosPrintf(MIL_TEXT("   X         Y         Z\n"));
      MosPrintf(MIL_TEXT("%7.3f   %7.3f   %7.3f \n")  , PointX, PointY, PointZMin);
      MosPrintf(MIL_TEXT("%7.3f   %7.3f   %7.3f \n\n"), PointX, PointY, PointZMax);

      WaitForKey();

      M3dgraRemove(GraList, StepDots1GraLabel,  M_DEFAULT);
      M3dgraRemove(GraList, StepDots2GraLabel,  M_DEFAULT);
      M3dgraRemove(GraList, StepLineGraLabel, M_DEFAULT);
      }

   // Reposition display
   RepositionDisplay(MilSystem, GraList, DisplayInfo);

   // Find the missing X- and Y-coordinates of multiple points on the sphere
   // and draw the points
   const MIL_DOUBLE PointsX[] = {80.0, 101.0, 70.0};
   const MIL_DOUBLE PointsZ[] = {138.0, 163.0, 191.0};
   const MIL_UINT NbPoints = sizeof(PointsX) / sizeof(MIL_DOUBLE);
   MIL_DOUBLE PointsY[NbPoints];
   NbValidPoints = M3dgeoEvalSurface(MilSphereGeo, M_EVAL_Y + M_MAX_VALUE, NbPoints,
                                     PointsX, PointsZ, PointsY, M_DEFAULT);
   if (NbValidPoints == (MIL_INT) NbPoints)
      {
      const std::vector<MIL_DOUBLE> Zeros(NbPoints, 0.0);
      auto DotsGraLabel = M3dgraDots(GraList, M_DEFAULT, NbPoints,
                                     PointsX, &Zeros[0], PointsZ,
                                     M_NULL, M_NULL, M_NULL, M_DEFAULT);
      SetGraphicFormat(GraList, DotsGraLabel, M_COLOR_YELLOW, 5, 100);
      auto LineDotsGraLabel = M3dgraDots(GraList, M_DEFAULT, NbPoints,
                                         PointsX, PointsY, PointsZ,
                                         M_NULL, M_NULL, M_NULL, M_DEFAULT); 
      SetGraphicFormat(GraList, LineDotsGraLabel, M_COLOR_YELLOW, 5, 100);

      for (unsigned int i = 0; i < NbPoints; ++i)
         {
         auto StepLineGraLabel = M3dgraLine(GraList, M_DEFAULT, M_TWO_POINTS, M_DEFAULT, 
                                            PointsX[i], 0.0, PointsZ[i],
                                            PointsX[i], PointsY[i], PointsZ[i], 
                                            M_DEFAULT, M_DEFAULT);
         SetGraphicFormat(GraList, StepLineGraLabel, M_COLOR_YELLOW, 3, 30);
         }

      MosPrintf(MIL_TEXT("Similar to the line example, M3dgeoEvalSurface can evaluate ")
                MIL_TEXT("the missing\n"));
      MosPrintf(MIL_TEXT("coordinates of a list of points at once. ")
                MIL_TEXT("Given a list of points with\n"));
      MosPrintf(MIL_TEXT("known X- and Z-coordinates, the missing ")
                MIL_TEXT("Y-coordinates can be found. \n"));
      MosPrintf(MIL_TEXT("These points are displayed and tabulated below.\n"));
      MosPrintf(MIL_TEXT("Note that only the points with the largest Y-coordinate ")
                MIL_TEXT("are shown in this case.\n\n"));
      MosPrintf(MIL_TEXT("   X         Y         Z\n"));

      for (unsigned int i = 0; i < NbPoints; ++i)
         {
         MosPrintf(MIL_TEXT("%7.3f   %7.3f   %7.3f \n"), PointsX[i], PointsY[i], PointsZ[i]);
         }
      MosPrintf(MIL_TEXT("\n"));

      WaitForKey();
      }

   // Remove all models from graphics list
   M3dgraRemove(GraList, M_ALL, M_DEFAULT);
   }

//*******************************************************************************
// Demonstrates how to use M3dgeoEvalSurface with planes.
//*******************************************************************************
void PlaneSurfaceExample(MIL_ID MilSystem, MIL_ID Mil3dDisp)
   {
   // Allocate a 3D graphic list
   auto GraList = (MIL_ID) M3ddispInquire(Mil3dDisp, M_3D_GRAPHIC_LIST_ID, M_NULL);

   // Show display
   auto DisplayInfo = ShowDisplay(MilSystem, Mil3dDisp, GraList, -4.0, -4.0, -3.0);

   // Create and draw plane
   auto MilPlaneGeo = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   const Point3D<MIL_DOUBLE> Point = {60.0, -100.0, 23.0};
   const Vector3D<MIL_DOUBLE> Normal = {-20.0, -15.0, 30.0};
   M3dgeoPlane(MilPlaneGeo, M_POINT_AND_NORMAL, Point.x, Point.y, Point.z,
               Normal.x, Normal.y, Normal.z, 
               M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   auto PlaneGraLabel = M3dgeoDraw3d(M_DEFAULT, MilPlaneGeo, GraList,
                                     M_DEFAULT, M_DEFAULT);
   SetGraphicFormat(GraList, PlaneGraLabel, M_COLOR_RED, 3, 30);

   MosPrintf(MIL_TEXT("Similar to spheres, given 2 input coordinates of points ")
             MIL_TEXT("on a plane,\n"));
   MosPrintf(MIL_TEXT("the other coordinate of the points can be found. \n\n"));
   WaitForKey();

   // Find the missing X- and Y-coordinates of multiple points on the plane
   // and draw the points
   const MIL_DOUBLE PointsX[] = {33.0 ,  85.0, 130.0, 183.0};
   const MIL_DOUBLE PointsY[] = {180.0, 133.0,  53.0, 89.0};
   const MIL_UINT NbPoints = sizeof(PointsX) / sizeof(MIL_DOUBLE);
   MIL_DOUBLE PointsZ[NbPoints];
   MIL_INT NbValidPoints = M3dgeoEvalSurface(MilPlaneGeo, M_EVAL_Z, NbPoints,
                                             PointsX, PointsY, PointsZ, M_DEFAULT);
   if (NbValidPoints == (MIL_INT) NbPoints)
      {
      const std::vector<MIL_DOUBLE> Zeros(NbPoints, 0.0);
      auto DotsGraLabel = M3dgraDots(GraList, M_DEFAULT, NbPoints,
                                     PointsX, PointsY, &Zeros[0],
                                     M_NULL, M_NULL, M_NULL, M_DEFAULT);
      SetGraphicFormat(GraList, DotsGraLabel, M_COLOR_YELLOW, 5, 100);

      auto LineDotsGraLabel = M3dgraDots(GraList, M_DEFAULT, NbPoints,
                                         PointsX, PointsY, PointsZ,
                                         M_NULL, M_NULL, M_NULL, M_DEFAULT); 
      SetGraphicFormat(GraList, LineDotsGraLabel, M_COLOR_YELLOW, 5, 100);

      for (unsigned int i = 0; i < NbPoints; ++i)
         {
         auto StepLineGraLabel = M3dgraLine(GraList, M_DEFAULT, M_TWO_POINTS,
                                            M_DEFAULT, 
                                            PointsX[i], PointsY[i], 0.0,
                                            PointsX[i], PointsY[i], PointsZ[i], 
                                            M_DEFAULT, M_DEFAULT);
         SetGraphicFormat(GraList, StepLineGraLabel, M_COLOR_YELLOW, 3, 30);
         }
      MosPrintf(MIL_TEXT("For instance, given the X- and Y-coordinates ")
                MIL_TEXT("of a list of points,\n"));
      MosPrintf(MIL_TEXT("the missing Z-coordinates can be evaluated.\n"));
      MosPrintf(MIL_TEXT("These points are displayed and tabulated below.\n\n"));
      MosPrintf(MIL_TEXT("   X         Y         Z\n"));

      for (unsigned int i = 0; i < NbPoints; ++i)
         {
         MosPrintf(MIL_TEXT("%7.3f   %7.3f   %7.3f \n"), PointsX[i], PointsY[i], PointsZ[i]);
         }
      MosPrintf(MIL_TEXT("\n"));

      WaitForKey();
      }

   // Remove all models from graphics list
   M3dgraRemove(GraList, M_ALL, M_DEFAULT);
   }

//*******************************************************************************
// Demonstrates how sample a surface using M3dimSample.
//*******************************************************************************
void UniformSamplingExample(MIL_ID MilSystem, MIL_ID Mil3dDisp)
   {
   // Allocate a 3D graphic list
   auto MilGraList = (MIL_ID) M3ddispInquire(Mil3dDisp, M_3D_GRAPHIC_LIST_ID, M_NULL);
   
   // Show display
   ShowDisplay(MilSystem, Mil3dDisp, MilGraList, -4.0, -5.0, -3.0);

   // Create and draw the geometries
   M3ddispControl(Mil3dDisp, M_UPDATE, M_DISABLE);
   std::vector<MIL_UNIQUE_3DGEO_ID> MilGeometries;

   // Create sphere
   MilGeometries.emplace_back(M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID));
   M3dgeoSphere(MilGeometries.back(),
                0.25 * GRID_SIZE, 0.25 * GRID_SIZE, 0.25 * GRID_SIZE,
                0.2 * GRID_SIZE, M_DEFAULT);
   auto Label = M3dgeoDraw3d(M_DEFAULT, MilGeometries.back(),
                             MilGraList, M_DEFAULT, M_DEFAULT);
   SetGraphicFormat(MilGraList, Label, M_COLOR_RED, 3, 30);

   // Create line
   MilGeometries.emplace_back(M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID));
   M3dgeoLine(MilGeometries.back(), M_TWO_POINTS,
              0.1  * GRID_SIZE, 0.75 * GRID_SIZE, 0.3  * GRID_SIZE ,
              0.75 * GRID_SIZE, 0.75 * GRID_SIZE, 0.9  * GRID_SIZE,
              M_DEFAULT, M_DEFAULT);
   Label = M3dgeoDraw3d(M_DEFAULT, MilGeometries.back(), MilGraList, M_DEFAULT, M_DEFAULT);
   SetGraphicFormat(MilGraList, Label, M_COLOR_GREEN, 3, 30);

   // Create cylinder
   MilGeometries.emplace_back(M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID));
   M3dgeoCylinder(MilGeometries.back(), M_TWO_POINTS,
                  0.75 * GRID_SIZE, 0.1 * GRID_SIZE, 0.2 * GRID_SIZE,
                  0.75 * GRID_SIZE, 0.9 * GRID_SIZE, 0.2 * GRID_SIZE,
                  0.2 * GRID_SIZE, M_DEFAULT, M_DEFAULT);
   Label = M3dgeoDraw3d(M_DEFAULT, MilGeometries.back(), MilGraList, M_DEFAULT, M_DEFAULT);
   SetGraphicFormat(MilGraList, Label, M_COLOR_BLUE, 3, 30);
   M3ddispControl(Mil3dDisp, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("Given finite geometries, it is possible to sample\n"));
   MosPrintf(MIL_TEXT("each surface or curve at a given resolution.\n\n"));
   WaitForKey();   

   // Sample the geometries at a resolution of X mm into a point cloud container
   M3ddispControl(Mil3dDisp, M_UPDATE, M_DISABLE);
   std::vector<MIL_UNIQUE_BUF_ID> MilPointClouds;
   auto MilSurfaceSampleContext = M3dimAlloc(MilSystem, M_SURFACE_SAMPLE_CONTEXT,
                                             M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MilSurfaceSampleContext, M_RESOLUTION, SAMPLING_RESOLUTION);
   for(const auto& geo : MilGeometries)
      {
      MilPointClouds.emplace_back(MbufAllocContainer(MilSystem, M_PROC + M_DISP,
                                                     M_DEFAULT, M_UNIQUE_ID));
      M3dimSample(MilSurfaceSampleContext, geo, MilPointClouds.back(), M_DEFAULT);

      // Display the result
      auto PointCloudLabel = M3ddispSelect(Mil3dDisp, MilPointClouds.back(), M_ADD, M_DEFAULT);
      SetGraphicFormat(MilGraList, PointCloudLabel, M_COLOR_YELLOW, 3, 100);
      }

   // Enable auto rotation
   M3ddispControl(Mil3dDisp, M_AUTO_ROTATE, M_ENABLE);
   M3ddispControl(Mil3dDisp, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("The point clouds resulting from the sampling are displayed.\n\n"));
   WaitForKey();

   // Draw the normals as lines
   std::vector<MIL_DOUBLE> Nx;
   std::vector<MIL_DOUBLE> Ny;
   std::vector<MIL_DOUBLE> Nz;
   std::vector<MIL_DOUBLE> X;
   std::vector<MIL_DOUBLE> Y;
   std::vector<MIL_DOUBLE> Z;
   for(const auto& pointCloud : MilPointClouds)
      {
      if(MbufInquireContainer(pointCloud, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL))
         {
         // Get the points and normal directions
         M3dimGet(pointCloud, M_COMPONENT_RANGE, M_DEFAULT, M_PLANAR, X, Y, Z);
         M3dimGet(pointCloud, M_COMPONENT_NORMALS_MIL, M_DEFAULT, M_PLANAR, Nx, Ny, Nz);

         // Draw the lines.
         M3ddispControl(Mil3dDisp, M_UPDATE, M_DISABLE);
         M3dgraControl(MilGraList, M_DEFAULT_SETTINGS, M_COLOR, M_COLOR_YELLOW);
         for(MIL_INT p = 0; p < (MIL_INT)X.size(); p++)
            {
            M3dgraLine(MilGraList, M_DEFAULT, M_POINT_AND_VECTOR, M_DEFAULT,
                       X[p], Y[p], Z[p], Nx[p], Ny[p], Nz[p],
                       SAMPLING_RESOLUTION, M_DEFAULT);
            }
         }
      M3ddispControl(Mil3dDisp, M_UPDATE, M_ENABLE);
      }

   MosPrintf(MIL_TEXT("The normals of the surface samples are also added as another\n")
             MIL_TEXT("component of the point clouds.\n\n"));
   WaitForKey();

   // Remove all graphics from graphics list
   M3dgraRemove(MilGraList, M_ALL, M_DEFAULT);

   // Disable auto rotation
   M3ddispControl(Mil3dDisp, M_AUTO_ROTATE, M_DISABLE);
   }

//*******************************************************************************
// Pauses the execution until a key is pressed.
//*******************************************************************************
void WaitForKey()
   {
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*******************************************************************************
// Allocates a 3D display if possible.  
//*******************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto Mil3dDisp = M3ddispAlloc(MilSystem, M_DEFAULT,
                                 MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!Mil3dDisp)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n"));
      WaitForKey();
      exit(EXIT_FAILURE);
      }

   return Mil3dDisp;
   }

//*******************************************************************************
// Sets color, thickness, and opacity of the specified model.
//*******************************************************************************
void SetGraphicFormat(MIL_ID GraphicsList, MIL_INT64 ModelLabel, MIL_INT Color,
                                                                 MIL_INT Thickness,
                                                                 MIL_INT Opacity)
   {
   M3dgraControl(GraphicsList, ModelLabel, M_COLOR, Color);
   M3dgraControl(GraphicsList, ModelLabel, M_THICKNESS, Thickness);
   M3dgraControl(GraphicsList, ModelLabel, M_OPACITY, Opacity);
   }

//*******************************************************************************
// Shows the display.
//*******************************************************************************
Display3DInfo ShowDisplay(MIL_ID MilSystem, MIL_ID Mil3dDisp, MIL_ID MilGraList,
                          MIL_DOUBLE ViewX, MIL_DOUBLE ViewY, MIL_DOUBLE ViewZ)
   {
   // Set the view point
   M3ddispSetView(Mil3dDisp, M_VIEW_ORIENTATION, ViewX, ViewY, ViewZ, M_DEFAULT);
   M3ddispSetView(Mil3dDisp, M_UP_VECTOR, 0.0, 0.0, 1.0, M_DEFAULT);
   M3ddispSelect(Mil3dDisp, M_NULL, M_OPEN, M_DEFAULT);

   // Set the size of the window
   M3ddispControl(Mil3dDisp, M_SIZE_X, 500);
   M3ddispControl(Mil3dDisp, M_SIZE_Y, 375);

   // Draw the axis and grid
   Display3DInfo DisplayInfo;
   DisplayInfo.AxisLabel = M3dgraAxis(MilGraList, M_ROOT_NODE, M_DEFAULT,
                                      AXIS_LENGTH, M_NULL, M_DEFAULT, M_DEFAULT);
   auto MilGridMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX,
                                    M_DEFAULT, M_UNIQUE_ID);
   
   M3dgeoMatrixSetTransform(MilGridMatrix, M_TRANSLATION, GRID_DISPLACEMENT, GRID_DISPLACEMENT,
                            0.0, M_DEFAULT, M_DEFAULT);

   DisplayInfo.GridLabel = M3dgraGrid(MilGraList, DisplayInfo.AxisLabel,
                                      M_SIZE_AND_SPACING, MilGridMatrix,
                                      GRID_SIZE, GRID_SIZE, GRID_SPACING, GRID_SPACING,
                                      M_DEFAULT);
   M3dgraControl(MilGraList, DisplayInfo.GridLabel, M_FILL_COLOR, M_COLOR_WHITE);
   SetGraphicFormat(MilGraList, DisplayInfo.GridLabel, M_COLOR_BLACK, 1, 30);

   return DisplayInfo;
   }

//*******************************************************************************
// Repositions the display.
//*******************************************************************************
void RepositionDisplay(MIL_ID MilSystem, MIL_ID MilGraList, Display3DInfo& DisplayInfo)
   {
   M3dgraRemove(MilGraList, DisplayInfo.GridLabel, M_DEFAULT);
   const MIL_DOUBLE Angle = 90.0;
   auto MilGridMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX,
                                    M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MilGridMatrix, M_TRANSLATION, GRID_DISPLACEMENT, GRID_DISPLACEMENT,
                            0.0, M_DEFAULT, M_DEFAULT);
   M3dgeoMatrixSetTransform(MilGridMatrix, M_ROTATION_XYZ,
                            Angle, 0.0, 0.0, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
   DisplayInfo.GridLabel = M3dgraGrid(MilGraList, DisplayInfo.AxisLabel,
                                      M_SIZE_AND_SPACING, MilGridMatrix,
                                      VERT_GRID_SIZE, VERT_GRID_SIZE,
                                      VERT_GRID_SPACING, VERT_GRID_SPACING,
                                      M_DEFAULT);
   M3dgraControl(MilGraList, DisplayInfo.GridLabel, M_FILL_COLOR, M_COLOR_WHITE);
   SetGraphicFormat(MilGraList, DisplayInfo.GridLabel, M_COLOR_BLACK, 1, 30);
   }
