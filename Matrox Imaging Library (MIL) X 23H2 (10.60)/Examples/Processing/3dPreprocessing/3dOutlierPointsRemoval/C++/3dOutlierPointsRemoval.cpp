//***************************************************************************************/
//
// File name: 3dOutlierPointsRemoval.cpp
//
// Synopsis:  This program demonstrates how to remove outliers from point clouds.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include "Utilities.h"
#include "DisplayLinker.h"

//******************************************************************************
// Example files.
//******************************************************************************
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("3dOutlierPointsRemoval/") MIL_TEXT(x))

static MIL_STRING LIGHT_CAP_FILE = EX_PATH("LigthCap.mbufc");
static MIL_STRING BOXES_FILE     = EX_PATH("Boxes.mbufc");
static MIL_STRING EARPHONES_FILE = EX_PATH("Earphone.ply");

//******************************************************************************
// Function declarations.
//******************************************************************************
void PrintHeader();
void RunLightCapCase(MIL_ID MilSystem, MIL_ID MilScannedPointCloud, 
                     MIL_ID SrcDisplay, std::vector<DstResult>& DstDisplays);
void RunBoxesCase(MIL_ID MilSystem, MIL_ID MilScannedPointCloud,
                  MIL_ID SrcDisplay, std::vector<DstResult>& DstDisplays);
void RunEarphoneCase(MIL_ID MilSystem, MIL_ID MilScannedPointCloud,
                     MIL_ID SrcDisplay, std::vector<DstResult>& DstDisplays);

//*******************************************************************************
// Prints the Example's description.
//*******************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("3dOutlierPointsRemoval\n\n")
            
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates three different ways of removing outliers\n")
             MIL_TEXT("from a point cloud.\n\n")
            
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Image Processing, 3D Display, 3D Geometry,\n")
             MIL_TEXT("3D Graphics, and Buffer.\n\n"));
   }

//*******************************************************************************
// Main function.
//*******************************************************************************
int MosMain(void)
   {
   PrintHeader();

   // Allocate a MIL Application and System.
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   WaitForKey();

   // Allocate displays and destination buffers.
   auto MilSrcDisplay = Alloc3dDisplayId(MilSystem, SRC_DISPLAY_INFO.PositionX,
                                         SRC_DISPLAY_INFO.PositionY,
                                         SRC_DISPLAY_INFO.Size,
                                         SRC_DISPLAY_INFO.Size,
                                         SRC_DISPLAY_INFO.Title);

   const MIL_INT NbModes = 3;
   std::vector<DstResult> DstResults(NbModes);
   std::vector<MIL_ID> Displays(NbModes + 1);
   Displays[0] = MilSrcDisplay;
   for (int i = 0; i < NbModes; ++i)
      {
      const auto DispInfo = DST_DISPLAY_INFO[i];
      auto MilPC = MbufAllocContainer(MilSystem, M_PROC | M_DISP, M_DEFAULT, M_UNIQUE_ID);
      auto MilOutlierPoints = MbufAllocContainer(MilSystem, M_PROC | M_DISP, M_DEFAULT, M_UNIQUE_ID);
      auto MilDisplay = Alloc3dDisplayId(MilSystem, DispInfo.PositionX, DispInfo.PositionY,
                                         DispInfo.Size, DispInfo.Size, DispInfo.Title);
      auto GraList = M3ddispInquire(MilDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
      DstResults[i] = DstResult(MilPC.release(), MilOutlierPoints.release(), MilDisplay.release(), GraList);

      Displays[i + 1] = DstResults[i].Display;
      }

   // Synchronize displays.
   CDisplayLinker DisplayLinker(Displays);

   // Run first case.
   auto MilLightCapPC = RestoreFile(MilSystem, LIGHT_CAP_FILE);
   MbufConvert3d(MilLightCapPC, MilLightCapPC, M_NULL, M_DEFAULT, M_DEFAULT);
   RunLightCapCase(MilSystem, MilLightCapPC, MilSrcDisplay, DstResults);

   // Run second case.
   auto MilBoxesPC = RestoreFile(MilSystem, BOXES_FILE);
   MbufConvert3d(MilBoxesPC, MilBoxesPC, M_NULL, M_DEFAULT, M_DEFAULT);
   RunBoxesCase(MilSystem, MilBoxesPC, MilSrcDisplay, DstResults);
   
   // Run third case.
   auto MilHeadsetPC = RestoreFile(MilSystem, EARPHONES_FILE);
   MbufConvert3d(MilHeadsetPC, MilHeadsetPC, M_NULL, M_DEFAULT, M_DEFAULT);
   DstResults[1].SetDisplayTitle(MIL_TEXT("M_STD_DEVIATION threshold mode"));
   DstResults[2].SetDisplayTitle(MIL_TEXT("M_ROBUST_STD_DEVIATION threshold mode"));
   Displays = { MilSrcDisplay, DstResults[1].Display, DstResults[2].Display };
   DisplayLinker.SetDisplays(Displays);
   DstResults.erase(DstResults.begin());
   RunEarphoneCase(MilSystem, MilHeadsetPC, MilSrcDisplay, DstResults);

   return EXIT_SUCCESS; // No error.
   }

//*******************************************************************************
// Remove outliers from the light cap point cloud using various modes.
//*******************************************************************************
void RunLightCapCase(MIL_ID MilSystem, MIL_ID MilScannedPC, MIL_ID SrcDisplay,
                     std::vector<DstResult>& DstResults)
   {
   // Different outlier removal modes.
   const auto NbModes = DstResults.size();

   // Outlier removal parameters.
   const auto MinNbNeighbors = 25;
   const auto OrganizedSize = 7;
   const auto DistanceFactor = 9.0;
   const auto StdDevFactor = 4.5;
   const auto ProbabilityThresholdFactor = 1.2;

   // Display scanned point cloud.
   M3ddispSelect(SrcDisplay, MilScannedPC, M_SELECT, M_DEFAULT);
   for (int i = 0; i < DstResults.size(); ++i)
      {
      M3ddispSelect(DstResults[i].Display, MilScannedPC, M_SELECT, M_DEFAULT);
      }
   M3ddispSetView(SrcDisplay, M_VIEW_BOX, M_WHOLE_SCENE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   auto GraSrcList = M3ddispInquire(SrcDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   const auto NbSrcPoints = M3dimGet(MilScannedPC, M_COMPONENT_CONFIDENCE, M_NULL, M_DEFAULT,
                                     M_NULL, M_NULL, M_NULL);
   MosPrintf(MIL_TEXT("A scanned light cap with %i points is shown.\n\n"), NbSrcPoints);
   WaitForKey();

   MosPrintf(MIL_TEXT("The outliers are removed using: \n")
             MIL_TEXT(" 1) M_NUMBER_WITHIN_DISTANCE outlier mode\n")
             MIL_TEXT(" 2) M_LOCAL_DISTANCE outlier mode + M_ROBUST_STD_DEVIATION threshold mode\n")
             MIL_TEXT(" 3) M_LOCAL_DENSITY_PROBABILITY outlier mode\n\n"));

   MosPrintf(MIL_TEXT("The outliers are shown in red.\n\n"));

   // Allocate the outlier mask buffer.
   const auto SizeX = MbufInquireContainer(MilScannedPC, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   const auto SizeY = MbufInquireContainer(MilScannedPC, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   auto MilOutlierMask = MbufAlloc2d(MilSystem, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE | M_PROC, M_UNIQUE_ID);

   // Define the outlier removal context using the M_NUMBER_WITHIN_DISTANCE outlier mode.
   DstResults[0].Context = M3dimAlloc(MilSystem, M_OUTLIERS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(DstResults[0].Context, M_OUTLIER_MODE, M_NUMBER_WITHIN_DISTANCE);
   M3dimControl(DstResults[0].Context, M_MINIMUM_NUMBER_NEIGHBORS, MinNbNeighbors);
   M3dimControl(DstResults[0].Context, M_NEIGHBOR_SEARCH_MODE, M_ORGANIZED);
   M3dimControl(DstResults[0].Context, M_NEIGHBORHOOD_ORGANIZED_SIZE, OrganizedSize);
   auto MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_DISTANCE_TO_NEAREST_NEIGHBOR, MilScannedPC, MilStatResult, M_DEFAULT);
   const auto AveDistance = M3dimGetResult(MilStatResult, M_DISTANCE_TO_NEAREST_NEIGHBOR_AVERAGE, M_NULL);
   M3dimControl(DstResults[0].Context, M_NEIGHBORHOOD_DISTANCE_MODE, M_USER_DEFINED);
   M3dimControl(DstResults[0].Context, M_NEIGHBORHOOD_DISTANCE, DistanceFactor * AveDistance);

   // Define the outlier removal context using the M_LOCAL_DISTANCE outlier mode +
   // M_ROBUST_STD_DEVIATION threshold mode.
   DstResults[1].Context = M3dimAlloc(MilSystem, M_OUTLIERS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(DstResults[1].Context, M_NEIGHBOR_SEARCH_MODE, M_ORGANIZED);
   M3dimControl(DstResults[1].Context, M_NEIGHBORHOOD_ORGANIZED_SIZE, OrganizedSize);
   M3dimControl(DstResults[1].Context, M_STD_DEVIATION_FACTOR, StdDevFactor);

   // Define the outlier removal context using the M_LOCAL_DENSITY_PROBABILITY outlier mode.
   DstResults[2].Context = M3dimAlloc(MilSystem, M_OUTLIERS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(DstResults[2].Context, M_NEIGHBOR_SEARCH_MODE, M_ORGANIZED);
   M3dimControl(DstResults[2].Context, M_NEIGHBORHOOD_ORGANIZED_SIZE, OrganizedSize);
   M3dimControl(DstResults[2].Context, M_OUTLIER_MODE, M_LOCAL_DENSITY_PROBABILITY);
   M3dimControl(DstResults[2].Context, M_PROBABILITY_THRESHOLD_FACTOR, ProbabilityThresholdFactor);

   // Remove outliers and compute the processing time.
   std::vector<MIL_INT> NbOutliers(NbModes);
   std::vector<MIL_INT> CompTimes(NbModes); // in ms.
   for (int i = 0; i < NbModes; ++i)
      {
      M3dimOutliers(DstResults[i].Context, MilScannedPC, DstResults[i].PC, MilOutlierMask, M_DEFAULT);

      CompTimes[i] = TimeComputation(MilScannedPC, DstResults[i]);

      auto NbInliers = M3dimGet(DstResults[i].PC, M_COMPONENT_CONFIDENCE, M_NULL, M_DEFAULT, M_NULL, M_NULL, M_NULL);
      NbOutliers[i] = NbSrcPoints - NbInliers;
      M3dimCrop(MilScannedPC, DstResults[i].OutlierPoints, MilOutlierMask, M_NULL, M_SAME, M_DEFAULT);
      DrawOutlierPoints(DstResults[i]);
      }

   // Print the results.
   MosPrintf(MIL_TEXT("Outlier mode                  Nb outliers      Processing time (in ms)\n"));
   MosPrintf(MIL_TEXT("M_NUMBER_WITHIN_DISTANCE          %d                %d \n"),   NbOutliers[0], CompTimes[0]);
   MosPrintf(MIL_TEXT("M_LOCAL_DISTANCE                  %d                %d \n"),   NbOutliers[1], CompTimes[1]);
   MosPrintf(MIL_TEXT("M_LOCAL_DENSITY_PROBABILITY       %d                %d \n\n"), NbOutliers[2], CompTimes[2]);

   MosPrintf(MIL_TEXT("For this point cloud, all three approaches produce similar results.\n")
             MIL_TEXT("For such cases, the M_NUMBER_WITHIN_DISTANCE outlier mode is usually preferred\n")
             MIL_TEXT("as it is the most computationally efficient mode.\n\n"));

   M3ddispControl(SrcDisplay, M_AUTO_ROTATE, M_ENABLE);
   WaitForKey();
   M3ddispControl(SrcDisplay, M_AUTO_ROTATE, M_DISABLE);
   M3ddispSetView(SrcDisplay, M_AZIM_ELEV_ROLL, 270, 0, 0, M_DEFAULT);
   for (auto& Result : DstResults)
      {
      M3dgraRemove(Result.GraList, M_ALL, M_DEFAULT);
      M3ddispSelect(Result.Display, Result.PC, M_ADD, M_DEFAULT);
      }

   // Calculate and draw the semi-oriented bounding box.
   M3dimStat(M_STAT_CONTEXT_SEMI_ORIENTED_BOX, MilScannedPC, MilStatResult, M_DEFAULT);
   const auto SrcHeight = M3dimGetResult(MilStatResult, M_SIZE_Z, M_NULL);
   DrawBoundingBox(MilSystem, MilStatResult, GraSrcList);

   auto DstHeightSum = 0.0;
   for (auto& Result : DstResults)
      {
      M3dimStat(M_STAT_CONTEXT_SEMI_ORIENTED_BOX, Result.PC, MilStatResult, M_DEFAULT);
      DstHeightSum += M3dimGetResult(MilStatResult, M_SIZE_Z, M_NULL);
      DrawBoundingBox(MilSystem, MilStatResult, Result.GraList);
      }

   auto const DstHeight = DstHeightSum / (MIL_DOUBLE)NbModes;
   auto const HeightRatio = SrcHeight / DstHeight;
   MosPrintf(MIL_TEXT("The outliers are removed from the displays and the point clouds' bounding boxes\n")
             MIL_TEXT("are shown. The original scanned point cloud's bounding box is %3.1f times \n")
             MIL_TEXT("taller than necessary due to the outliers.\n\n"), HeightRatio);
   WaitForKey();

   // Remove all graphics from displays.
   M3dgraRemove(GraSrcList, M_ALL, M_DEFAULT);
   for (auto& Result : DstResults)
      {
      M3dgraRemove(Result.GraList, M_ALL, M_DEFAULT);
      }
   }

//*******************************************************************************
// Remove outliers from the boxes point cloud using various modes.
//*******************************************************************************
void RunBoxesCase(MIL_ID MilSystem, MIL_ID MilScannedPC, MIL_ID SrcDisplay, std::vector<DstResult>& DstResults)
   {
   // Different outlier removal modes.
   const auto NbModes = DstResults.size();

   // Outlier removal parameters.
   const auto MinNbNeighbors = 25;
   const auto DistanceFactor = 12.0;
   const auto StdDevFactor = 4.0;
   const auto ProbabilityThresholdFactor = 2.2;

   // Display scanned point cloud.
   M3ddispSelect(SrcDisplay, MilScannedPC, M_SELECT, M_DEFAULT);
   for (auto& Result : DstResults)
      {
      M3ddispSelect(Result.Display, MilScannedPC, M_SELECT, M_DEFAULT);
      }
   // Set view.
   M3ddispSetView(SrcDisplay, M_VIEWPOINT, -450, -6035, 575, M_DEFAULT);
   M3ddispSetView(SrcDisplay, M_UP_VECTOR, 0.5, 0, -1, M_DEFAULT);
   M3ddispSetView(SrcDisplay, M_INTEREST_POINT, 740, -120, 1445, M_DEFAULT);

   const auto NbSrcPoints = M3dimGet(MilScannedPC, M_COMPONENT_CONFIDENCE, M_NULL, M_DEFAULT, M_NULL, M_NULL, M_NULL);
   MosPrintf(MIL_TEXT("A point cloud of 2 boxes, which has %i points, is shown.\n\n"), NbSrcPoints);
   WaitForKey();

   MosPrintf(MIL_TEXT("The outliers are removed using: \n")
             MIL_TEXT(" 1) M_NUMBER_WITHIN_DISTANCE outlier mode\n")
             MIL_TEXT(" 2) M_LOCAL_DISTANCE outlier mode + M_ROBUST_STD_DEVIATION threshold mode\n")
             MIL_TEXT(" 3) M_LOCAL_DENSITY_PROBABILITY outlier mode\n\n"));

   MosPrintf(MIL_TEXT("The outliers are shown in red.\n\n"));

   // Allocate the outlier mask buffer.
   const auto SizeX = MbufInquireContainer(MilScannedPC, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   const auto SizeY = MbufInquireContainer(MilScannedPC, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   auto MilOutlierMask = MbufAlloc2d(MilSystem, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE | M_PROC, M_UNIQUE_ID);

   // Define the outlier removal context using the M_NUMBER_WITHIN_DISTANCE outlier mode.
   DstResults[0].Context = M3dimAlloc(MilSystem, M_OUTLIERS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(DstResults[0].Context, M_OUTLIER_MODE, M_NUMBER_WITHIN_DISTANCE);
   M3dimControl(DstResults[0].Context, M_MINIMUM_NUMBER_NEIGHBORS, MinNbNeighbors);
   auto MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_DISTANCE_TO_NEAREST_NEIGHBOR, MilScannedPC, MilStatResult, M_DEFAULT);
   const auto AveDistance = M3dimGetResult(MilStatResult, M_DISTANCE_TO_NEAREST_NEIGHBOR_AVERAGE, M_NULL);
   M3dimControl(DstResults[0].Context, M_NEIGHBORHOOD_DISTANCE_MODE, M_USER_DEFINED);
   M3dimControl(DstResults[0].Context, M_NEIGHBORHOOD_DISTANCE, DistanceFactor * AveDistance);

   // Define the outlier removal context using the M_LOCAL_DISTANCE outlier mode + M_ROBUST_STD_DEVIATION threshold mode.
   DstResults[1].Context = M3dimAlloc(MilSystem, M_OUTLIERS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(DstResults[1].Context, M_STD_DEVIATION_FACTOR, StdDevFactor);

   // Define the outlier removal context using the M_LOCAL_DENSITY_PROBABILITY outlier mode.
   DstResults[2].Context = M3dimAlloc(MilSystem, M_OUTLIERS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(DstResults[2].Context, M_OUTLIER_MODE, M_LOCAL_DENSITY_PROBABILITY);
   M3dimControl(DstResults[2].Context, M_PROBABILITY_THRESHOLD_FACTOR, ProbabilityThresholdFactor);

   // Remove outliers.
   std::vector<MIL_INT> NbOutliers(NbModes);
   for (int i = 0; i < NbModes; ++i)
      {
      M3dimOutliers(DstResults[i].Context, MilScannedPC, DstResults[i].PC, MilOutlierMask, M_DEFAULT);
      auto NbInliers = M3dimGet(DstResults[i].PC, M_COMPONENT_CONFIDENCE, M_NULL, M_DEFAULT, M_NULL, M_NULL, M_NULL);
      NbOutliers[i] = NbSrcPoints - NbInliers;
      M3dimCrop(MilScannedPC, DstResults[i].OutlierPoints, MilOutlierMask, M_NULL, M_SAME, M_DEFAULT);
      DrawOutlierPoints(DstResults[i]);
      }

   // Print the results.
   MosPrintf(MIL_TEXT("Outlier mode                  Nb outliers \n"));
   MosPrintf(MIL_TEXT("M_NUMBER_WITHIN_DISTANCE          %d      \n"),  NbOutliers[0]);
   MosPrintf(MIL_TEXT("M_LOCAL_DISTANCE                  %d      \n"),  NbOutliers[1]);
   MosPrintf(MIL_TEXT("M_LOCAL_DENSITY_PROBABILITY       %d      \n\n"), NbOutliers[2]);

   MosPrintf(MIL_TEXT("For this case, the M_NUMBER_WITHIN_DISTANCE and M_LOCAL_DISTANCE outlier modes\n")
             MIL_TEXT("falsely classify sparse regions as outliers. The M_LOCAL_DENSITY_PROBABILITY\n")
             MIL_TEXT("outlier mode does a better job and should generally be favored for \n")
             MIL_TEXT("point clouds with non-uniform density.\n\n"));
   WaitForKey();

   // Remove all graphics from displays.
   auto GraSrcList = M3ddispInquire(SrcDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraRemove(GraSrcList, M_ALL, M_DEFAULT);
   for (auto& Result : DstResults)
      {
      M3dgraRemove(Result.GraList, M_ALL, M_DEFAULT);
      }
}

//*******************************************************************************
// Remove outliers from the earphone point cloud using various modes.
//*******************************************************************************
void RunEarphoneCase(MIL_ID MilSystem, MIL_ID MilScannedPC, MIL_ID SrcDisplay, std::vector<DstResult>& DstResults)
   {
   // Different outlier removal modes.
   const auto NbModes = DstResults.size();

   // Outlier removal parameters.
   const auto StdDevFactor = 2.5;

   // Display scanned point cloud.
   M3ddispSelect(SrcDisplay, MilScannedPC, M_SELECT, M_DEFAULT);
   for (auto& Result : DstResults)
      {
      M3ddispSelect(Result.Display, MilScannedPC, M_SELECT, M_DEFAULT);
      }
   // Set view.
   M3ddispSetView(SrcDisplay, M_VIEWPOINT, 140, 35, 1095, M_DEFAULT);
   M3ddispSetView(SrcDisplay, M_UP_VECTOR, 0.5, 0.5, 0, M_DEFAULT);
   M3ddispSetView(SrcDisplay, M_INTEREST_POINT, 95, -45, 400, M_DEFAULT);

   const auto NbSrcPoints = M3dimGet(MilScannedPC, M_COMPONENT_CONFIDENCE, M_NULL, M_DEFAULT, M_NULL, M_NULL, M_NULL);
   MosPrintf(MIL_TEXT("A point cloud of an earphone case, which has %i points, is shown.\n"), NbSrcPoints);
   MosPrintf(MIL_TEXT("The point cloud is polluted by some outliers far from the earphone\n")
             MIL_TEXT("case (left middle of the display).\n\n"));
   WaitForKey();

   MosPrintf(MIL_TEXT("The outliers are removed using: \n")
             MIL_TEXT(" 1) M_LOCAL_DISTANCE outlier mode + M_STD_DEVIATION threshold mode\n")
             MIL_TEXT(" 2) M_LOCAL_DISTANCE outlier mode + M_ROBUST_STD_DEVIATION threshold mode\n\n"));

   MosPrintf(MIL_TEXT("Generally, these 2 approaches produce similar results, except for scenarios\n")
             MIL_TEXT("where outliers exist far away from the main scene.\n\n"));

   MosPrintf(MIL_TEXT("The outliers are shown in red.\n\n"));

   // Allocate the outlier mask buffer.
   const auto SizeX = MbufInquireContainer(MilScannedPC, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   const auto SizeY = MbufInquireContainer(MilScannedPC, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   auto MilOutlierMask = MbufAlloc2d(MilSystem, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE | M_PROC, M_UNIQUE_ID);

   // Define the outlier removal context using the M_LOCAL_DISTANCE outlier mode + M_STD_DEVIATION threshold mode.
   DstResults[0].Context = M3dimAlloc(MilSystem, M_OUTLIERS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(DstResults[0].Context, M_DISTANCE_THRESHOLD_MODE, M_STD_DEVIATION);
   M3dimControl(DstResults[0].Context, M_STD_DEVIATION_FACTOR, StdDevFactor);

   // Define the outlier removal context using the M_LOCAL_DISTANCE outlier mode + M_ROBUST_STD_DEVIATION threshold mode.
   DstResults[1].Context = M3dimAlloc(MilSystem, M_OUTLIERS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(DstResults[1].Context, M_STD_DEVIATION_FACTOR, StdDevFactor);

   // Remove outliers.
   std::vector<MIL_INT> NbOutliers(NbModes);
   for (int i = 0; i < NbModes; ++i)
      {
      M3dimOutliers(DstResults[i].Context, MilScannedPC, DstResults[i].PC, MilOutlierMask, M_DEFAULT);
      auto NbInliers = M3dimGet(DstResults[i].PC, M_COMPONENT_CONFIDENCE, M_NULL, M_DEFAULT, M_NULL, M_NULL, M_NULL);
      NbOutliers[i] = NbSrcPoints - NbInliers;
      M3dimCrop(MilScannedPC, DstResults[i].OutlierPoints, MilOutlierMask, M_NULL, M_SAME, M_DEFAULT);
      DrawOutlierPoints(DstResults[i]);
      }

   // Print the results.
   MosPrintf(MIL_TEXT("Threshold mode                  Nb outliers \n"));
   MosPrintf(MIL_TEXT("M_STD_DEVIATION                     %d      \n"), NbOutliers[0]);
   MosPrintf(MIL_TEXT("M_ROBUST_STD_DEVIATION              %d      \n\n"), NbOutliers[1]);

   MosPrintf(MIL_TEXT("The far-lying outliers skew the local average distance distribution of the\n")
             MIL_TEXT("point cloud. The M_ROBUST_STD_DEVIATION threshold mode uses robust statistics\n")
             MIL_TEXT("to accurately identify outliers in the main scene, unlike the M_STD_DEVIATION\n")
             MIL_TEXT("threshold mode.\n\n"));
   WaitForKey();

   MosPrintf(MIL_TEXT("A zoomed-in view of the earphone case is shown.\n\n"));
   M3ddispSetView(SrcDisplay, M_VIEWPOINT, 270, -45, 950, M_DEFAULT);
   M3ddispSetView(SrcDisplay, M_UP_VECTOR, 0, 1, 0, M_DEFAULT);
   M3ddispSetView(SrcDisplay, M_INTEREST_POINT, -10, -20, 600, M_DEFAULT);
    
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   auto GraSrcList = M3ddispInquire(SrcDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraRemove(GraSrcList, M_ALL, M_DEFAULT);
   for (auto& Result: DstResults)
      {
      M3dgraRemove(Result.GraList, M_ALL, M_DEFAULT);
      }
   }
