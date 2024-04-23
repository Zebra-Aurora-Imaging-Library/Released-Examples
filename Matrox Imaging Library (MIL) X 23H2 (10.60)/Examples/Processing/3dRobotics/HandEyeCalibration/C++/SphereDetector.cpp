//***************************************************************************************/
// 
// File name: SphereDetector.cpp  
//
// Synopsis:  This class detects and localize spheres in a calibration model.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//**************************************************************************************/

#include"SphereDetector.h"


//*****************************************************************************
// SphereDetector Constructor
// Input: aNumSpheres    -  Number of spheres in the calibration model
//        aRadiusClasses -  Array constaining the radiuses of the spheres in the
//                          model. Each sphere must have unique model.
//        aRadiusTolerance- Radius Tolerance 
//*****************************************************************************
SphereDetector::SphereDetector(MIL_INT aNumSpheres,
                               const MIL_DOUBLE* aRadiusClasses,
                               MIL_DOUBLE aRadiusTolerance)
   : mRadiusTolerance(aRadiusTolerance)
   {
   for(MIL_INT i = 0; i < aNumSpheres; i++)
      mSphereRadii.push_back(aRadiusClasses[i]);
   }

//*****************************************************************************
// SphereDetector::DetectSpheres
// Desctiption:
//        Detects spheres in point cloud. Will look for spheres in a given 
//        radius range.
// Input: MilSystem      -  MilSystem ID
//        MilCloud  - Cloud ID
//        MinRadius - Radius range min
//        MaxRadius - Radius range max
//        Context   - 3D Mod context Id
//        Result    - 3D Mod result
//        NumberOfSpheres - Number of expected spheres. -1 if unknown.ma
//  Output: vector containing Stats of detected spheres
//*****************************************************************************
std::vector<SphereStats> SphereDetector::DetectSpheres(MIL_ID MilCloud,
                                                       const MIL_DOUBLE MinRadius,
                                                       const MIL_DOUBLE MaxRadius,
                                                       MIL_ID Context,
                                                       MIL_ID Result,
                                                       MIL_INT NumberOfSpheres)
   {
   // Define the sphere model.
   M3dmodDefine(Context, M_ADD, M_SPHERE_RANGE, MinRadius - mRadiusTolerance, MaxRadius + mRadiusTolerance, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Configure 3dMod.
   M3dmodControl(Context, 0, M_NUMBER, NumberOfSpheres);
   M3dmodControl(Context, 0, M_CERTAINTY, 100);
   M3dmodControl(Context, M_CONTEXT, M_PERSEVERANCE, 100);
   M3dmodControl(Context, M_CONTEXT, M_FIT_ITERATIONS_MAX, 2);

   // Preprocess the context.
   M3dmodPreprocess(Context, M_DEFAULT);

   // The Sphere Finder requires the existance of M_COMPONENT_NORMALS_MIL in the point cloud.
   AddComponentNormalsIfMissing(MilCloud);

   // Find the model.
   MIL_INT Status;
   M3dmodFind(Context, MilCloud, Result, M_DEFAULT);
   M3dmodGetResult(Result, M_DEFAULT, M_STATUS, &Status);
   MIL_INT NumResults = 0;
   if(Status == M_COMPLETE)
      {
      M3dmodGetResult(Result, M_DEFAULT, M_NUMBER, &NumResults);
      }

   // Organize results into vector of SphereStats.
   std::vector<SphereStats> DetectedSpheres(NumResults);

   if(NumResults > 0)
      {
      std::vector<MIL_INT64> Labels(NumResults);
      for(MIL_INT i = 0; i < NumResults; ++i)
         {
         // Get results.
         DetectedSpheres[i].mSphere.Radius = M3dmodGetResult(Result, i, M_RADIUS, M_NULL);
         DetectedSpheres[i].mSphere.Center.X = M3dmodGetResult(Result, i, M_CENTER_X, M_NULL);
         DetectedSpheres[i].mSphere.Center.Y = M3dmodGetResult(Result, i, M_CENTER_Y, M_NULL);
         DetectedSpheres[i].mSphere.Center.Z = M3dmodGetResult(Result, i, M_CENTER_Z, M_NULL);
         DetectedSpheres[i].mScore = M3dmodGetResult(Result, i, M_SCORE, M_NULL);
         DetectedSpheres[i].SetRadiusID(mSphereRadii);
         }
      }

   return DetectedSpheres;
   }

//*****************************************************************************
// SphereDetector::DetectSpheresAllRadius
// Desctiption:
//        Detects all spheres in the model using the retained list of possible
//        radii
// Input: MilSystem -  MilSystem ID
//        MilCloud  -  Point Cloud ID
//  Output: vector containing Stats of detected spheres
//*****************************************************************************
SphereDetectorResult SphereDetector::DetectSpheresAllRadius(MIL_ID MilSystem, MIL_ID MilCloud)
   {
   std::vector<SphereStats> Results;

   // Fetch radii range.
   auto minmax = std::minmax_element(std::begin(mSphereRadii), std::end(mSphereRadii));
   MIL_DOUBLE RangeMin = (*minmax.first) - mRadiusTolerance; //min - tolerance
   MIL_DOUBLE RangeMax = (*minmax.second) + mRadiusTolerance; //max + tolerance

   // Allocate a Sphere Finder context.
   auto Context = M3dmodAlloc(MilSystem, M_FIND_SPHERE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate a Sphere Finder result.
   SphereDetectorResult Result;
   Result.MilModResult = M3dmodAllocResult(MilSystem, M_FIND_SPHERE_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Detect all spheres in range.
   Result.DetectedSpheres = DetectSpheres(MilCloud, RangeMin, RangeMax, Context, Result.MilModResult, (long)mSphereRadii.size());

   return Result;
   }

//*****************************************************************************
// SphereDetector::RetrieveModelSpheres
// Desctiption:
//        Retrieve and display spheres from point cloud. The spheres are 
//        associated  to sphere classes
// Input: MilSystem -  MilSystem ID
//        MilCloud - Point Cloud ID
//        ExpectedAngleAssociations - Expected angle associations for each sphere
//        MilCommonDisplay - 3d Display unique Id to display point cloud relative
//                           to camera
// Output: True if successful
//*****************************************************************************
std::vector<SphereStats> SphereDetector::RetrieveModelSpheres(const MIL_ID &MilSystem,
                                                              const MIL_ID MilCloud,
                                                              MIL_ID MilSpheresDisplay,
                                                              MIL_ID MilPoseDisplay)
   {
   // Prepare Timer.
   MIL_DOUBLE ComputationTime = 0.0;
   MappTimer(M_TIMER_RESET, M_NULL);
   
   // Detect spheres.
   auto SphereResult = DetectSpheresAllRadius(MilSystem, MilCloud);
   auto& detectedSpheres = SphereResult.DetectedSpheres;

   // Sort the sphere according to the id.
   std::sort(detectedSpheres.begin(), detectedSpheres.end(), [](const SphereStats& s1, const SphereStats& s2) -> bool { return s1.mBRadiusID > s2.mBRadiusID; });

   // Read timer.
   MappTimer(M_TIMER_READ, &ComputationTime);
   MosPrintf(MIL_TEXT("Localized %i spheres in %.2f s.\n\n"), detectedSpheres.size(), ComputationTime);

   MosPrintf(MIL_TEXT("RadiusID         Center          Radius  Score  Color \n"));
   MosPrintf(MIL_TEXT("----------------------------------------------------------\n"));

   // Retrieve Spheres display graphics list
   MIL_ID MilGraphicsList = (MIL_ID)M3ddispInquire(MilSpheresDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   // Clear the Spheres display.
   M3dgraRemove(MilGraphicsList, M_ALL, M_DEFAULT);

   // Populate Common Display view.
   if(MilPoseDisplay)
      {
      MIL_ID MilPoseGraphicsList = (MIL_ID)M3ddispInquire(MilPoseDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

      // Clear display.
      M3dgraRemove(MilPoseGraphicsList, M_ALL, M_DEFAULT);

      // Draw a coordinate system at the origin.
      M3dgraAxis(MilPoseGraphicsList, M_DEFAULT, M_DEFAULT, 100, MIL_TEXT("Camera"), M_DEFAULT, M_DEFAULT);

      // Display point cloud.
      MIL_INT64 MilLabel = M3ddispSelect(MilPoseDisplay, MilCloud, M_SELECT, M_DEFAULT);
      M3dgraControl(MilPoseGraphicsList, MilLabel, M_COLOR_USE_LUT, M_TRUE);
      M3dgraControl(MilPoseGraphicsList, MilLabel, M_COLOR_COMPONENT_BAND, 2);
      M3dgraControl(MilPoseGraphicsList, MilLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
      }

   // Draw sphere detection results.
   if(MilSpheresDisplay)
      {
      if(MilPoseDisplay)
         {
         // Inherit azimuth, elevation and roll from common display if available.
         MIL_DOUBLE Azimuth=0, Elevation=0, Roll=0;
         M3ddispGetView(MilPoseDisplay, M_AZIM_ELEV_ROLL, &Azimuth, &Elevation, &Roll, M_DEFAULT);
         M3ddispSetView(MilSpheresDisplay, M_AZIM_ELEV_ROLL, Azimuth, Elevation, Roll, M_DEFAULT);
         }

      // Disable display update prior to draw spheres
      M3ddispControl(MilSpheresDisplay, M_UPDATE, M_DISABLE);

      // Display point cloud
      MIL_INT64 MilLabel = M3ddispSelect(MilSpheresDisplay, MilCloud, M_SELECT, M_DEFAULT);
      M3dgraControl(MilGraphicsList, MilLabel, M_COLOR_USE_LUT, M_TRUE);
      M3dgraControl(MilGraphicsList, MilLabel, M_COLOR_COMPONENT_BAND, 2);
      M3dgraControl(MilGraphicsList, MilLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
      M3ddispSetView(MilSpheresDisplay, M_VIEW_BOX, M_WHOLE_SCENE, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      for(size_t i = 0; i < detectedSpheres.size(); i++)
         {
         // Fetch unique color for sphere.
         MIL_INT32 ColorValue = 0;
         MIL_STRING ColorString;
         IndexToColor(i, ColorValue, ColorString);

         // Configure drawing
         auto MilDrawContext = M3dmodAlloc(MilSystem, M_DRAW_3D_GEOMETRIC_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
         M3dmodControlDraw(MilDrawContext, M_DRAW_MODEL, M_ACTIVE, M_ENABLE);
         M3dmodControlDraw(MilDrawContext, M_DRAW_BOX, M_COLOR, ColorValue);
         M3dmodControlDraw(MilDrawContext, M_DRAW_BOX, M_THICKNESS, 3);
         M3dmodControlDraw(MilDrawContext, M_DRAW_RESERVED_POINTS, M_ACTIVE, M_ENABLE);
         M3dmodControlDraw(MilDrawContext, M_DRAW_RESERVED_POINTS, M_THICKNESS, 1);
         // Draw all the detected spheres.
         M3dmodDraw3d(MilDrawContext, SphereResult.MilModResult, i, MilGraphicsList, M_DEFAULT, M_DEFAULT);

         // Print sphere info.         
         MosPrintf(MIL_TEXT("    %i     (%5.1f, %5.1f, %4.1f)    %4.1f   %4.1f  %-7s\n"),
                   detectedSpheres[i].mBRadiusID,
                   detectedSpheres[i].mSphere.Center.X,
                   detectedSpheres[i].mSphere.Center.Y,
                   detectedSpheres[i].mSphere.Center.Z,
                   detectedSpheres[i].mSphere.Radius,
                   detectedSpheres[i].mScore,
                   ColorString.c_str());
         }

      // Enable display update.
      M3ddispControl(MilSpheresDisplay, M_UPDATE, M_ENABLE);
      }

   MosPrintf(MIL_TEXT("\n"));

   return std::move(SphereResult.DetectedSpheres);
   }
