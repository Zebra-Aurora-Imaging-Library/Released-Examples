//******************************************************************************
// 
// File name: 3dMouldingProfileInspection.cpp
//
// Synopsis:  This example demonstrates how to take the profile of meshes,
//            point clouds, and geometries for moulding inspection.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//******************************************************************************
#include <mil.h>
#include <math.h>
#include <cctype>
#include "Utilities.h"
#include "ProfileInspector.h"

//******************************************************************************
// Example files.
//******************************************************************************
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("3dMouldingProfileInspection/") MIL_TEXT(x))

struct InspectionInfo
   {
   MIL_STRING ScannedFile;      // File name of the scanned object to inspect.
   MIL_STRING RefFile;          // File name of the reference object.
   MIL_INT    RefType;          // Either a Mesh or a 3D Geometry object.
   MIL_DOUBLE XProfileLimit;    // Profile limit in the profile plane's x-direction.
   MIL_DOUBLE MaxAreaTolerance; // Maximum acceptable area tolerance (in mm^2).
   };

#define NB_OBJECTS 3
static InspectionInfo INSPECTIONS_INFO[NB_OBJECTS] =
   {
      {EX_PATH("Rod.ply"),       EX_PATH("RefRod.m3dgeo")   , M_3DGEO_GEOMETRY, 15.1, 13.0},
      {EX_PATH("Moulding1.ply"), EX_PATH("RefMoulding1.ply"), M_CONTAINER,      15.0, 30.0},
      {EX_PATH("Moulding2.ply"), EX_PATH("RefMoulding2.ply"), M_CONTAINER,       8.0, 15.0},
   };

//******************************************************************************
// Constants.
//******************************************************************************
static const MIL_DOUBLE THICKNESS_PROFILE  = 0.18 * 3.0;         // Thickness when computing a profile (in mm).
static const MIL_DOUBLE SLICE_LENGTH       = 3.0;                // Distance between consecutive profiles (in mm).
static const MIL_DOUBLE FAILURE_LENGTH     = 3.0 * SLICE_LENGTH; // Distance between a failed profile and the following profile (in mm).
static const MIL_DOUBLE REF_SAMPLING_DIST  = 0.1;                // Profile sampling distance of the reference object (in mm).
static const MIL_DOUBLE SCAN_SAMPLING_DIST = 0.3;                // Profile sampling distance of the scanned object (in mm).

//******************************************************************************
// Function declarations.
//******************************************************************************
void PrintHeader();
void InspectObject(MIL_ID MilSystem, MIL_ID MilRefObject, MIL_ID MilScannedObject,
                   const MIL_DOUBLE XProfileLimit, const MIL_DOUBLE MaxAreaTolerance,
                   const bool isVerbose);

//*******************************************************************************
// Prints the example's description.
//*******************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("3dMouldingProfileInspection\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to take the profile of meshes,\n")
             MIL_TEXT("point clouds, and 3D geometries for moulding inspection.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Buffer, Display, Graphics,\n")
             MIL_TEXT("Metrology, Calibration, 3D Display, 3D Graphics, \n")
             MIL_TEXT("3D Geometry, and 3D Image Processing.\n\n"));
   }

//*******************************************************************************
// Main function.
//*******************************************************************************
int MosMain(void)
   {
   PrintHeader();

   // Allocate a MIL application and system.
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   bool isVerbose = true;
   for (unsigned int i = 0; i < NB_OBJECTS; ++i)
      {
      MosPrintf(MIL_TEXT("================================ Object %d of %d ================="),
                i+1, NB_OBJECTS);
      MosPrintf(MIL_TEXT("===============\n\n"));

      // Load files.
      auto MilScannedObject = RestoreFile(MilSystem, INSPECTIONS_INFO[i].ScannedFile);
      if (INSPECTIONS_INFO[i].RefType == M_3DGEO_GEOMETRY)
         {
         // The reference object is a MIL 3D geometry.
         auto MilRefObj = RestoreGeometry(MilSystem, INSPECTIONS_INFO[i].RefFile);
         InspectObject(MilSystem, MilRefObj, MilScannedObject, INSPECTIONS_INFO[i].XProfileLimit,
                       INSPECTIONS_INFO[i].MaxAreaTolerance, isVerbose);
         }
      else
         {
         // The reference object is a meshed point cloud container.
         auto MilRefObj = RestoreFile(MilSystem, INSPECTIONS_INFO[i].RefFile);
         InspectObject(MilSystem, MilRefObj, MilScannedObject, INSPECTIONS_INFO[i].XProfileLimit,
                       INSPECTIONS_INFO[i].MaxAreaTolerance, isVerbose);
         }
      MosPrintf(MIL_TEXT("=================================================================")
                MIL_TEXT("==============\n\n"));

      if (isVerbose) 
         {
         isVerbose = false;
         }
      
      }

   MosPrintf(MIL_TEXT("Completed inspection of all objects.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();
   return 0; // No error.
   }

//*******************************************************************************
// Computes the reference profile using the reference object, 
// and delegates the inspection of the object to the function
// ComputeAndInspectProfiles().
//*******************************************************************************
void InspectObject(MIL_ID MilSystem, MIL_ID MilRefObject, MIL_ID MilScannedObject,
                   const MIL_DOUBLE XProfileLimit, const MIL_DOUBLE MaxAreaTolerance,
                   const bool isVerbose)
   {
   // Display reference and scanned objects.
   auto MilDispRef = Display3dObject(MilSystem, MilRefObject, 0, 0, DISP3D_SIZE_X,
                                     DISP3D_SIZE_Y, MIL_TEXT("Reference object"));

   auto MilDispScanned = Display3dObject(MilSystem, MilScannedObject, DISP3D_SIZE_X,
                                         0, DISP3D_SIZE_X, DISP3D_SIZE_Y,
                                         MIL_TEXT("Scanned object"));

   // Retrieve displays' graphic lists.
   auto MilScannedGraphicList = M3ddispInquire(MilDispScanned, M_3D_GRAPHIC_LIST_ID, M_NULL);
   auto MilRefGraphicList = M3ddispInquire(MilDispRef, M_3D_GRAPHIC_LIST_ID, M_NULL);

   // Colorize scanned object in display.
   std::vector<MIL_INT> DispScannedLabels;
   M3dgraInquire(MilScannedGraphicList, M_ROOT_NODE, M_CHILDREN, DispScannedLabels);
   M3dgraControl(MilScannedGraphicList, DispScannedLabels[0], M_COLOR_USE_LUT, M_TRUE);
   M3dgraControl(MilScannedGraphicList, DispScannedLabels[0], M_COLOR_COMPONENT, M_COMPONENT_RANGE);
   M3dgraControl(MilScannedGraphicList, DispScannedLabels[0], M_COLOR_COMPONENT_BAND, 0);

   if (isVerbose)
      {
      MosPrintf(MIL_TEXT("The reference and scanned objects are shown in")
                MIL_TEXT(" separate displays.\n\n"));
      WaitForKey();
      }

   // Define the profile plane transformation matrix.
   // The XY-plane of the transformation matrix defines the profile plane.
   auto MilInitialSlicingMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX,
                                              M_DEFAULT, M_UNIQUE_ID);
   static Point3D<MIL_DOUBLE> Origin = {0.0, 0.1, 0.0}; // Slight offset in the Y-direction.
   static Vector3D<MIL_DOUBLE> Axis1 = {1.0, 0.0, 0.0};
   static Vector3D<MIL_DOUBLE> Axis2 = {0.0, 0.0, 1.0};
   M3dgeoMatrixSetWithAxes(MilInitialSlicingMatrix, M_XY_AXES + M_COORDINATE_SYSTEM_TRANSFORMATION, 
                           Origin.x, Origin.y, Origin.z,
                           Axis1.x, Axis1.y, Axis1.z,
                           Axis2.x, Axis2.y, Axis2.z,
                           M_DEFAULT);

   auto MilTranslationMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX,
                                           M_DEFAULT, M_UNIQUE_ID);

   auto MilRefObjectBoundingBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilRefObject, MilRefObjectBoundingBox, M_DEFAULT);
   const MIL_DOUBLE RefLength = M3dgeoInquire(MilRefObjectBoundingBox, M_SIZE_Y, M_NULL);

   M3dgeoMatrixSetTransform(MilTranslationMatrix, M_TRANSLATION, 0.0, 0.0, 0.5*RefLength,
                            M_DEFAULT, M_DEFAULT);

   auto MilRefSlicingPlane = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX,
                                         M_DEFAULT, M_UNIQUE_ID);

   M3dgeoMatrixSetTransform(MilRefSlicingPlane, M_COMPOSE_TWO_MATRICES,
                            MilTranslationMatrix,
                            MilInitialSlicingMatrix,
                            M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Allocate the reference profile result.
   auto MilRefProfileResult = M3dimAllocResult(MilSystem, M_PROFILE_RESULT, M_DEFAULT,
                                               M_UNIQUE_ID);

   // Compute the reference profile.
   const auto RefObjectType = MobjInquire(MilRefObject, M_OBJECT_TYPE, M_NULL);
   switch (RefObjectType)
      {
      case M_3DGEO_GEOMETRY:
         {
         const auto ProfileType = M_PROFILE_GEOMETRY;
         const MIL_DOUBLE SamplingMode = M_EUCLIDEAN;
         M3dimProfile(MilRefObject, MilRefProfileResult, ProfileType, MilRefSlicingPlane,
                      REF_SAMPLING_DIST, XProfileLimit, SamplingMode, M_DEFAULT, M_DEFAULT);
         }
         break;
      case M_CONTAINER:
         {
         const auto ProfileType = M_PROFILE_MESH;
         M3dimProfile(MilRefObject, MilRefProfileResult, ProfileType, MilRefSlicingPlane,
                      REF_SAMPLING_DIST, XProfileLimit, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         }
         break;
      default:
         {
         MosPrintf(MIL_TEXT("Only 3D geometry and container object types")
                   MIL_TEXT(" are supported.\n"));
         exit(EXIT_FAILURE);
         }
         break;
      }

   auto Status = M3dimGetResult(MilRefProfileResult, M_STATUS, M_NULL);
   if (Status != M_COMPLETE)
   {
      MosPrintf(MIL_TEXT("Profile of the reference object was not successfully completed.\n"));
      return;
   }

   // Retrieve the profile points in the plane's coordinate system.
   ProfileXY<MIL_DOUBLE> RefProfilePoints;
   M3dimGetResult(MilRefProfileResult, M_PROFILE_PLANE_X, RefProfilePoints.x);
   M3dimGetResult(MilRefProfileResult, M_PROFILE_PLANE_Y, RefProfilePoints.y);

   // Display the profile points.
   auto MilProfileImage = GetProfileImage(MilSystem, RefProfilePoints);
   auto MilDispProfile = Display2dImage(MilSystem, MilProfileImage, 0, DISP3D_SIZE_Y,
                                        MIL_TEXT("Profile inspection"));

   const MIL_DOUBLE RefWidth  = M3dgeoInquire(MilRefObjectBoundingBox, M_SIZE_X, M_NULL);
   const MIL_DOUBLE RefHeight = M3dgeoInquire(MilRefObjectBoundingBox, M_SIZE_Z, M_NULL);
   const MIL_DOUBLE PlaneSize = 2.0 * std::max({ RefWidth, RefHeight });

   // Display the profile plane.
   DrawSlicingPlane(MilSystem, MilRefGraphicList, MilRefProfileResult, PlaneSize);

   if (isVerbose)
      {
      auto MilProfileGraList = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);
      MdispControl(MilDispProfile, M_ASSOCIATED_GRAPHIC_LIST_ID, MilProfileGraList);
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      MgraDots(M_DEFAULT, MilProfileGraList, M_DEFAULT, RefProfilePoints.x, RefProfilePoints.y, M_DEFAULT);

      MosPrintf(MIL_TEXT("The profile of the reference object is taken by specifying \n")
                MIL_TEXT("a sampling distance, a transformation matrix which defines the profile plane, \n")
                MIL_TEXT("and a maximum profile length in the plane's x-direction.\n\n"));
      MosPrintf(MIL_TEXT("The computed profile is shown.\n\n"));
      WaitForKey();
      }

   // The scanned object's profile will be inspected at various positions.
   auto MilScannedBoundingBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilScannedObject, MilScannedBoundingBox, M_DEFAULT);
   const MIL_DOUBLE Length = M3dgeoInquire(MilScannedBoundingBox, M_SIZE_Y, M_NULL);
   const ProfileObject ScannedObjInfo  = {MilScannedObject, Length,
                                          MilInitialSlicingMatrix,
                                          SCAN_SAMPLING_DIST};

   // Perform inspections.
   ProfileInspector Inspector(MilSystem, RefProfilePoints, ScannedObjInfo,
                              MaxAreaTolerance, MilDispScanned,
                              MilDispProfile);
   Inspector.IsVerbose(isVerbose); 
   Inspector.InspectProfiles();
   }

//*******************************************************************************
// Class that performs the profile inspection of a scanned object.
//*******************************************************************************
ProfileInspector::ProfileInspector(MIL_ID MilSystem,
                                   const ProfileXY<MIL_DOUBLE>& RefProfilePoints,
                                   const ProfileObject ScannedObject,
                                   const MIL_DOUBLE MaxAreaTolerance,
                                   MIL_ID MilDispScanned,
                                   MIL_ID MilDispProfile)
   : m_MilSystem(MilSystem),
     m_RefProfilePoints(RefProfilePoints),
     m_ScannedObject(ScannedObject),
     m_MaxAreaTolerance(MaxAreaTolerance),
     m_MilDispScanned(MilDispScanned),
     m_MilDispProfile(MilDispProfile),
     m_IsVerbose(false)
   {
   // Number of slices to inspect.
   const unsigned int NbSlices = (unsigned int)std::floor(m_ScannedObject.Length / SLICE_LENGTH);
   m_FailedResults.reserve(NbSlices);
   
   // Allocate a profile result.
   m_MilScannedProfileResult = M3dimAllocResult(m_MilSystem, M_PROFILE_RESULT, M_DEFAULT,
                                                M_UNIQUE_ID);

   // Allocate a metrology context and result.
   m_MilMetContext = MmetAlloc(m_MilSystem, M_DEFAULT, M_UNIQUE_ID);
   m_MilMetResult = MmetAllocResult(m_MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Add reference profile as a metrology feature.
   MmetAddFeature(m_MilMetContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(1),
                  M_EXTERNAL_FEATURE, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetPut(m_MilMetContext, M_FEATURE_LABEL(1), M_DEFAULT, M_NULL, m_RefProfilePoints.x,
           m_RefProfilePoints.y, M_NULL, M_NULL, M_DEFAULT);

   // Allocate a display for the profiles that fail the inspection.
   auto ImageBuf = MdispInquire(m_MilDispProfile, M_SELECTED, M_NULL);
   m_MilFailedDisplay = Display2dImage(m_MilSystem, ImageBuf, DISP2D_SIZE_X, DISP3D_SIZE_Y,
                                       MIL_TEXT("Last failed inspection"));
   }

//*******************************************************************************
// Performs profile inspection of a scanned object at various sections.
//*******************************************************************************
void ProfileInspector::InspectProfiles()
   {
   auto MilProfileGraList = MgraAllocList(m_MilSystem, M_DEFAULT, M_UNIQUE_ID);
   MdispControl(m_MilDispProfile, M_ASSOCIATED_GRAPHIC_LIST_ID, MilProfileGraList);

   auto MilFailedGraList = MgraAllocList(m_MilSystem, M_DEFAULT, M_UNIQUE_ID);
   MdispControl(m_MilFailedDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilFailedGraList);
   
   if (IsVerbose())
      {
      MosPrintf(MIL_TEXT("The profile of the reference and scanned objects are \n")
                MIL_TEXT("shown in green and red, respectively.\n\n"));
   
      MosPrintf(MIL_TEXT("The profile inspection is said to pass if the area between \n")
                MIL_TEXT("the reference and the scanned objects' profiles \n")
                MIL_TEXT("are within the specified tolerance.\n\n"));
      }
   
   // Setup the display for the profile plane.
   auto MilScannedObjectBoundingBox = M3dgeoAlloc(m_MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, m_ScannedObject.Id, MilScannedObjectBoundingBox, M_DEFAULT);
   const MIL_DOUBLE Width  = M3dgeoInquire(MilScannedObjectBoundingBox, M_SIZE_X, M_NULL);
   const MIL_DOUBLE Height = M3dgeoInquire(MilScannedObjectBoundingBox, M_SIZE_Z, M_NULL);
   const MIL_DOUBLE PlaneSize = 2.0 * std::max({ Width, Height });
   
   MosPrintf(MIL_TEXT("Processing profiles...\r"));
   
   // Compute and inspect the profile at different locations.
   MIL_DOUBLE CurrentPosition = 0.0;
   while (CurrentPosition <= m_ScannedObject.Length)
      {
      const auto Inspection = SliceAndInspectProfile(CurrentPosition, PlaneSize);
      if (Inspection.Status == M_COMPLETE)
         {
         // Store the information of the failed inspection.
         if ((!Inspection.Passed))
            {
            m_FailedResults.push_back({ CurrentPosition, Inspection.Area });
   
            // Display the profile that failed the inspection.
            MgraClear(M_DEFAULT, MilFailedGraList);
   
            MgraCopy(MilProfileGraList, MilFailedGraList, M_COPY, M_DEFAULT,
                     M_ALL, M_NULL, M_NULL, M_DEFAULT);
   
            // Refresh the display.
            MdispControl(m_MilFailedDisplay, M_UPDATE, M_ENABLE);
            MdispControl(m_MilFailedDisplay, M_UPDATE, M_DISABLE);
   
            // Update the position for the next profile inspection.
            CurrentPosition += FAILURE_LENGTH;
            }
         else
            {
            // Inspection passed.
            // Update the position for the next profile inspection.
            CurrentPosition += SLICE_LENGTH;
            }
   
         // Clear the profile display.
         MgraClear(M_DEFAULT, MilProfileGraList);
         }
      else
         {
         // Profile computation did not complete successfully, continue.
         CurrentPosition += SLICE_LENGTH;
         }
      }
   
   // Print failed results.
   PrintFailedResults(); 

   InteractivelyDisplayFailures();
   }

//*******************************************************************************
// Computes the profile at a specified position.
//*******************************************************************************
InspectionResult ProfileInspector::SliceAndInspectProfile(MIL_DOUBLE Position,
                                                          MIL_DOUBLE PlaneSize)
   {
   InspectionResult Inspection;
   
   auto MilScannedGraphicList = M3ddispInquire(m_MilDispScanned, M_3D_GRAPHIC_LIST_ID, M_NULL);
   
   // Allocate the transformation matrices to define the profile plane.
   auto MilTranslationMatrix = M3dgeoAlloc(m_MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT,
                                           M_UNIQUE_ID);
   auto MilScannedSlicingPlane = M3dgeoAlloc(m_MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT,
                                             M_UNIQUE_ID);
   
   M3dgeoMatrixSetTransform(MilTranslationMatrix, M_TRANSLATION, 0.0, 0.0, Position,
                            M_DEFAULT, M_DEFAULT);
   
   M3dgeoMatrixSetTransform(MilScannedSlicingPlane, M_COMPOSE_TWO_MATRICES,
                            MilTranslationMatrix,
                            m_ScannedObject.SlicingPlaneTransformationMatrix,
                            M_DEFAULT, M_DEFAULT, M_DEFAULT);
   
   // Compute the profile.
   M3dimProfile(m_ScannedObject.Id, m_MilScannedProfileResult, M_PROFILE_POINT_CLOUD,
                MilScannedSlicingPlane, m_ScannedObject.SamplingDistance,
                m_ScannedObject.SamplingDistance, THICKNESS_PROFILE,
                M_DEFAULT, M_DEFAULT);
   
   Inspection.Status = (MIL_INT)M3dimGetResult(m_MilScannedProfileResult,
                                               M_STATUS, M_NULL);
   if (Inspection.Status == M_COMPLETE)
      {
      // Retrieve the profile points in the profile plane's coordinate system.
      M3dimGetResult(m_MilScannedProfileResult, M_PROFILE_PLANE_X, m_ScannedProfilePoints.x);
      M3dimGetResult(m_MilScannedProfileResult, M_PROFILE_PLANE_Y, m_ScannedProfilePoints.y);
   
      // Display the profile plane.
      auto ScannedPlaneLabel = DrawSlicingPlane(m_MilSystem, MilScannedGraphicList,
                                                m_MilScannedProfileResult, PlaneSize);
   
      // Perform the inspection.
      Inspection = InspectProfile();
   
      // Refresh the displays.
      M3ddispControl(m_MilDispScanned, M_UPDATE, M_ENABLE);
      MdispControl(m_MilDispProfile, M_UPDATE, M_ENABLE);
   
      M3ddispControl(m_MilDispScanned, M_UPDATE, M_DISABLE);
      MdispControl(m_MilDispProfile, M_UPDATE, M_DISABLE);
   
      if (!Inspection.Passed)
         {
         // If inspection failed, display the profile's plane in red.
         M3dgraControl(MilScannedGraphicList, ScannedPlaneLabel, M_COLOR, M_COLOR_RED);
         }
      else
         {
         // Remove the profile plane (since inspection has finished and passed).
         M3dgraRemove(MilScannedGraphicList, ScannedPlaneLabel, M_DEFAULT);
         }
      }
   
   return Inspection;
   }

//*******************************************************************************
// Performs a profile inspection.
//*******************************************************************************
InspectionResult ProfileInspector::InspectProfile()
   {
   InspectionResult Result;
   
   auto MilProfileGraList = MdispInquire(m_MilDispProfile, M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL);
   
   // Add scanned profile as another edgel feature.
   MmetAddFeature(m_MilMetContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(2), M_EXTERNAL_FEATURE,
                  M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(m_MilMetContext, M_FEATURE_LABEL(2), M_EDGEL_PROVIDED_ORDER, M_NONE);
   MmetPut(m_MilMetContext, M_FEATURE_LABEL(2), M_DEFAULT, M_NULL,
           m_ScannedProfilePoints.x, m_ScannedProfilePoints.y,
           M_NULL, M_NULL, M_DEFAULT);
   
   // Define an 'area between the curves' metrology tolerance.
   std::vector<MIL_ID> FeatureLabels = { M_FEATURE_LABEL(1), M_FEATURE_LABEL(2) };
   MmetAddTolerance(m_MilMetContext, M_AREA_BETWEEN_CURVES, M_TOLERANCE_LABEL(1),
                    0.0, m_MaxAreaTolerance,
                    FeatureLabels, M_NULL, M_DEFAULT, M_DEFAULT);
   
   MmetControl(m_MilMetContext, M_FEATURE_LABEL(2), M_EDGEL_DENOISING_MODE, M_MEAN);
   const auto DenoisingRadius = 0.5;
   MmetControl(m_MilMetContext, M_FEATURE_LABEL(2), M_EDGEL_DENOISING_RADIUS, DenoisingRadius);

   MmetCalculate(m_MilMetContext, M_NULL, m_MilMetResult, M_DEFAULT);
   
   // Verify if all tolerances are satisfied.
   MIL_INT NbToleranceFails;
   MmetGetResult(m_MilMetResult, M_GENERAL, M_NUMBER_OF_TOLERANCES_FAIL + M_TYPE_MIL_INT,
                 &NbToleranceFails);
   Result.Passed = (NbToleranceFails == 0);
   
   // Retrieve area between the curve value.
   MmetGetResult(m_MilMetResult, M_TOLERANCE_LABEL(1), M_TOLERANCE_VALUE, &(Result.Area));
   
   // Draw profiles.
   std::vector<MIL_DOUBLE> Colors = { M_COLOR_GREEN, M_COLOR_RED };
   std::vector<MIL_INT64> Thicknesses = { 1, 3 };
   for (unsigned int i = 0; i < FeatureLabels.size(); ++i)
      {
      MgraColor(M_DEFAULT, Colors[i]);
      MgraControl(M_DEFAULT, M_LINE_THICKNESS, Thicknesses[i]);
      MmetDraw(M_DEFAULT, m_MilMetResult, MilProfileGraList, M_DRAW_FEATURE,
               M_FEATURE_LABEL(i + 1), M_DEFAULT);
      }
   
   // Indicate in display whether inspection passed or failed.
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_PIXEL);
   MIL_STRING StatusString;
   if (Result.Passed)
      {
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      StatusString = MIL_TEXT("PASS");
      }
   else
      {
      MgraColor(M_DEFAULT, M_COLOR_RED);
      StatusString = MIL_TEXT("FAIL");
      }
   MgraText(M_DEFAULT, MilProfileGraList, 0.9 * DISP2D_SIZE_X, 0.1 * DISP2D_SIZE_Y, StatusString);
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
   
   MmetControl(m_MilMetContext, M_FEATURE_LABEL(2), M_DELETE, M_DEFAULT);
   return Result;
   }

//*******************************************************************************
// Outputs the failed results in a table. 
//*******************************************************************************
void ProfileInspector::PrintFailedResults() const
   {
   MosPrintf(MIL_TEXT("The maximum acceptable area between the reference \n")
      MIL_TEXT("and scanned profiles is %3.2f mm^2.\n\n"), m_MaxAreaTolerance);
   if (m_FailedResults.size() == 0)
      {
      MosPrintf(MIL_TEXT("Inspection passed.\n\n"));
      }
   else
      {
      MosPrintf(MIL_TEXT("The inspection failed at the following locations. \n"));
      MosPrintf(MIL_TEXT("Profile's location (in mm)      Area between the curves (in mm^2)\n"));
      for (auto& Failure : m_FailedResults)
         {
         MosPrintf(MIL_TEXT("%9c %7.2f %23c %6.2f\n"), ' ', Failure.Position, ' ', Failure.Area);
         }
      MosPrintf(MIL_TEXT("\n"));
      }
   WaitForKey();
   }

//*******************************************************************************
// Interactively display profiles of failed inspections.
//*******************************************************************************
void ProfileInspector::InteractivelyDisplayFailures()
   {
   auto MilProfileGraList = MdispInquire(m_MilDispProfile, M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL);
   auto MilScannedGraList = M3ddispInquire(m_MilDispScanned, M_3D_GRAPHIC_LIST_ID, M_NULL);

   M3ddispControl(m_MilDispScanned, M_UPDATE, M_ENABLE);
   const MIL_INT NbFailures = (MIL_INT)m_FailedResults.size();
   MIL_INT iFailure = NbFailures - 1;
   MIL_INT Key;

   auto MilScannedObjectBoundingBox = M3dgeoAlloc(m_MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, m_ScannedObject.Id, MilScannedObjectBoundingBox, M_DEFAULT);
   const MIL_DOUBLE Width = M3dgeoInquire(MilScannedObjectBoundingBox, M_SIZE_X, M_NULL);
   const MIL_DOUBLE Height = M3dgeoInquire(MilScannedObjectBoundingBox, M_SIZE_Z, M_NULL);
   const MIL_DOUBLE PlaneSize = 2.0 * std::max({ Width, Height });

   MosPrintf(MIL_TEXT("We can interactively see the profiles where an inspection failure occurred.\n"));
   MosPrintf(MIL_TEXT("Press 'a' to move to the left, 's' to the right, or 'q' to continue.\n\n"));

   while (true)
      {
      Key = (MIL_INT)std::tolower((int)MosGetch());
      if (Key == 'q')
      {
         MgraClear(M_DEFAULT, MilProfileGraList);
         M3dgraRemove(MilScannedGraList, M_ALL, M_DEFAULT);

         break;
      }
      else if (Key == 'a') // left.
      {
         iFailure = std::min(iFailure + 1, NbFailures - 1);
      }
      else if (Key == 's') // right.
      {
         iFailure = std::max(iFailure - 1, (MIL_INT)0);
      }
      else
      {
         continue;
      }

      M3dgraRemove(MilScannedGraList, M_ALL, M_DEFAULT);
      auto PCLabel = M3ddispSelect(m_MilDispScanned, m_ScannedObject.Id, M_ADD, M_DEFAULT);
      M3dgraControl(MilScannedGraList, PCLabel, M_COLOR_USE_LUT, M_TRUE);
      M3dgraControl(MilScannedGraList, PCLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
      M3dgraControl(MilScannedGraList, PCLabel, M_COLOR_COMPONENT_BAND, 0);
      MgraClear(M_DEFAULT, MilProfileGraList);
      MdispSelect(m_MilFailedDisplay, M_NULL);
      SliceAndInspectProfile(m_FailedResults[iFailure].Position, PlaneSize);
      MosPrintf(MIL_TEXT("Failure #%d/%d occurred at position %.2f mm.     \r"), iFailure + 1,
                NbFailures, m_FailedResults[iFailure].Position);
      }
   }
