//***************************************************************************************/
//
// File name: PointCloudSubsamplingModes.cpp
//
// Synopsis:  This example demonstrates the different point cloud subsampling 
//            modes available in MIL. A point cloud scan of a mask is loaded  
//            and subsampled using the 3D image processing module.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>

// Source file specification.
static const MIL_STRING PT_CLD_FILE = M_IMAGE_PATH MIL_TEXT("M3dgra/MaskOrganized.mbufc");

// Function declarations.
bool                 CheckForRequiredMILFile(const MIL_STRING& FileName);
void                 AddComponentNormalsIfMissing(MIL_ID MilContainer);

MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId             (MIL_ID MilSystem);
void                 ConfigureComparison3dDisplays(MIL_ID MilSystem,
                                                   std::vector<MIL_UNIQUE_3DDISP_ID>& MilComparison3dDisplays);
void                 DisplayPointCloud            (MIL_ID MilDisplay, MIL_ID MilPointCloud);

void                 DecimateSubsamplePointCloud (MIL_ID Mil3dDisplay,
                                                  MIL_ID MilSubsampleContext,
                                                  MIL_ID MilPointCloud,
                                                  MIL_ID MilDstPointCloud);
void                 GeometricSubsamplePointCloud(MIL_ID Mil3dDisplay,
                                                  MIL_ID MilSubsampleContext,
                                                  MIL_ID MilPointCloud,
                                                  MIL_ID MilDstPointCloud);
void                 GridSubsamplePointCloud     (MIL_ID Mil3dDisplay,
                                                  MIL_ID MilSubsampleContext,
                                                  MIL_ID MilPointCloud,
                                                  MIL_ID MilDstPointCloud);
void                 NormalSubsamplePointCloud   (MIL_ID Mil3dDisplay,
                                                  MIL_ID MilSubsampleContext,
                                                  MIL_ID MilPointCloud,
                                                  MIL_ID MilDstPointCloud);
void                 RandomSubsamplePointCloud   (MIL_ID Mil3dDisplay,
                                                  MIL_ID MilSubsampleContext,
                                                  MIL_ID MilPointCloud,
                                                  MIL_ID MilDstPointCloud);
void                 SubsampleAndDisplayResult   (MIL_ID Mil3dDisplay,
                                                  MIL_ID MilSubsampleContext,
                                                  MIL_ID MilPointCloud,
                                                  MIL_ID MilDstPointCloud);

// Constants.
static const MIL_INT RESULT_WINDOW_OFFSET_X = 800;

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("PointCloudSubsamplingModes\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates the different point cloud subsampling\n")
             MIL_TEXT("modes available in MIL. A point cloud scan of a mask is loaded\n") 
             MIL_TEXT("and subsampled using the 3D image processing module.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Image Processing, 3D Display, and Buffer.\n\n"));
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate MIL application and system.
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem      = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   PrintHeader();
   
   // Check for required example files.
   if(!CheckForRequiredMILFile(PT_CLD_FILE))
      {
      return 0;
      }

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   // Allocate a 3d subsampling context.
   auto MilSubsampleContext = M3dimAlloc(MilSystem, M_SUBSAMPLE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate a statistics result 3D image processing context.
   auto MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate the 3D displays and display the source point cloud.
   std::vector<MIL_UNIQUE_3DDISP_ID> Mil3dDisplays(2);
   for(auto& Mil3dDisplay : Mil3dDisplays)
      {
      Mil3dDisplay = Alloc3dDisplayId(MilSystem);
      M3ddispSetView(Mil3dDisplay, M_AUTO, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      }

   // Restore the source point cloud.
   MosPrintf(MIL_TEXT("A 3D point cloud is restored from a file.\n"));
   auto MilSrcPointCloud = MbufImport(PT_CLD_FILE, M_DEFAULT, M_RESTORE, MilSystem, M_UNIQUE_ID);

   // M_SUBSAMPLE_GEOMETRIC and M_SUBSAMPLE_NORMAL require the existance of M_COMPONENT_NORMALS_MIL in the point cloud.
   MosPrintf(MIL_TEXT("The normals are computed and added to the point cloud if not present,\n"));
   MosPrintf(MIL_TEXT("which are required for the geometric and normal subsampling modes.\n\n"));
   AddComponentNormalsIfMissing(MilSrcPointCloud);

   // Configure the display for the source point cloud.
   M3ddispControl(Mil3dDisplays[0], M_TITLE, MIL_TEXT("Source Point Cloud"));

   // Configure the display for the destination point clouds.
   M3ddispControl(Mil3dDisplays[1], M_TITLE, MIL_TEXT("Subsampled Point Cloud"));
   M3ddispControl(Mil3dDisplays[1], M_WINDOW_INITIAL_POSITION_X, RESULT_WINDOW_OFFSET_X);

   // Display the source point cloud and set the color component to M_COMPONENT_NORMALS_MIL.
   M3ddispSelect(Mil3dDisplays[0], M_NULL, M_OPEN, M_DEFAULT);
   DisplayPointCloud(Mil3dDisplays[0], MilSrcPointCloud);

   // Calculate the amount of points in the source point cloud.
   M3dimStat(M_STAT_CONTEXT_NUMBER_OF_POINTS, MilSrcPointCloud, MilStatResult, M_DEFAULT);
   MIL_INT NumSourcePoints;
   M3dimGetResult(MilStatResult, M_NUMBER_OF_POINTS_TOTAL, &NumSourcePoints);
   MosPrintf(MIL_TEXT("Number of source points          : %7d\n\n"), NumSourcePoints);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();   

   // Allocate destination point clouds and copy the point cloud to be subsampled to them.
   std::vector<MIL_UNIQUE_BUF_ID> MilSubsampledPointClouds(5);
   for(auto& SubsampledPointCloud : MilSubsampledPointClouds)
      SubsampledPointCloud = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);

   MosPrintf(MIL_TEXT("The point cloud is subsampled using the decimate subsampling option.\n"));
   MosPrintf(MIL_TEXT("The decimation subsampling algorithm selects points regularly at set intervals.\n"));

   // Subsample the source point cloud with the decimate subsampling option, and display it.
   DecimateSubsamplePointCloud(Mil3dDisplays[1], MilSubsampleContext, MilSrcPointCloud, MilSubsampledPointClouds[0]);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();


   MosPrintf(MIL_TEXT("The point cloud is subsampled using the geometric subsampling option.\n"));
   MosPrintf(MIL_TEXT("The geometric subsampling algorithm selects points that help 3D registration\n"));
   MosPrintf(MIL_TEXT("operations converge faster and provides greater stability.\n"));

   // Subsample the source point cloud with the geometric subsampling option, and display it.
   GeometricSubsamplePointCloud(Mil3dDisplays[1], MilSubsampleContext, MilSrcPointCloud, MilSubsampledPointClouds[1]);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();


   MosPrintf(MIL_TEXT("The point cloud is subsampled using the grid option. The grid subsampling\n"));
   MosPrintf(MIL_TEXT("algorithm divides the 3D space into cells and selects a single point from\n"));
   MosPrintf(MIL_TEXT("each cell. The grid subsampling operation supports outputting organized\n"));
   MosPrintf(MIL_TEXT("point clouds, which speeds up functions utilizing neighboring points.\n"));

   // Subsample the source point cloud with the grid subsampling option, and display it.
   GridSubsamplePointCloud(Mil3dDisplays[1], MilSubsampleContext, MilSrcPointCloud, MilSubsampledPointClouds[2]);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();


   MosPrintf(MIL_TEXT("The point cloud is subsampled using the normal option. The normal\n"));
   MosPrintf(MIL_TEXT("subsampling algorithm selects points that have distinct surface\n"));
   MosPrintf(MIL_TEXT("normals compared to neighboring points.\n"));

   // Subsample the source point cloud with the normal subsampling option, and display it.
   NormalSubsamplePointCloud(Mil3dDisplays[1], MilSubsampleContext, MilSrcPointCloud, MilSubsampledPointClouds[3]);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();


   MosPrintf(MIL_TEXT("The point cloud is subsampled using the random option. The random subsampling\n"));
   MosPrintf(MIL_TEXT("algorithm randomly selects a specified fraction of points from the source\n"));
   MosPrintf(MIL_TEXT("point cloud.\n"));

   // Subsample the source point cloud with the random subsampling option, and display it.
   RandomSubsamplePointCloud(Mil3dDisplays[1], MilSubsampleContext, MilSrcPointCloud, MilSubsampledPointClouds[4]);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Close the large displays, and open 5 smaller displays for a final comparison.
   MosPrintf(MIL_TEXT("The resulting subsampled point clouds are now displayed for comparison.\n\n"));

   M3ddispSelect(Mil3dDisplays[0], M_NULL, M_CLOSE, M_DEFAULT);
   M3ddispSelect(Mil3dDisplays[1], M_NULL, M_CLOSE, M_DEFAULT);
   std::vector<MIL_UNIQUE_3DDISP_ID> MilComparison3dDisplays(5);
   ConfigureComparison3dDisplays(MilSystem, MilComparison3dDisplays);

   for(MIL_INT i = 0; i < (MIL_INT) MilComparison3dDisplays.size(); i++)
      {
      DisplayPointCloud(MilComparison3dDisplays[i], MilSubsampledPointClouds[i]);
      }

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   return 0;
   }

//****************************************************************************
// Subsample a point cloud using the M_SUBSAMPLE_DECIMATE subsampling mode.    
//****************************************************************************
void DecimateSubsamplePointCloud(MIL_ID Mil3dDisplay,
                                 MIL_ID MilSubsampleContext,
                                 MIL_ID MilPointCloud,
                                 MIL_ID MilDstPointCloud)
   {
   constexpr MIL_INT DECIMATE_STEP_SIZE = 5;

   // Set the subsample mode of the 3D image processing context to decimate.
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_DECIMATE);
   M3dimControl(MilSubsampleContext, M_STEP_SIZE_X, DECIMATE_STEP_SIZE);
   M3dimControl(MilSubsampleContext, M_STEP_SIZE_Y, DECIMATE_STEP_SIZE);

   // Subsample and display the point cloud.
   SubsampleAndDisplayResult(Mil3dDisplay, MilSubsampleContext, MilPointCloud, MilDstPointCloud);

   MosPrintf(MIL_TEXT("Decimate step size (X and Y)     : %6d\n\n"), DECIMATE_STEP_SIZE);
   }

//****************************************************************************
// Subsample a point cloud using the M_SUBSAMPLE_GEOMETRIC subsampling mode.    
//****************************************************************************
void GeometricSubsamplePointCloud(MIL_ID Mil3dDisplay,
                                  MIL_ID MilSubsampleContext,
                                  MIL_ID MilPointCloud,
                                  MIL_ID MilDstPointCloud)
   {
   constexpr MIL_DOUBLE GEOMETRIC_FRACTION_OF_POINTS = 0.1;

   // Set the subsample mode of the 3D image processing context to geometric and subsample the point cloud.
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GEOMETRIC);
   M3dimControl(MilSubsampleContext, M_FRACTION_OF_POINTS, GEOMETRIC_FRACTION_OF_POINTS);

   // Subsample and display the point cloud.
   SubsampleAndDisplayResult(Mil3dDisplay, MilSubsampleContext, MilPointCloud, MilDstPointCloud);

   MosPrintf(MIL_TEXT("Fraction of points               : %.4f\n\n"), GEOMETRIC_FRACTION_OF_POINTS);
   }

//****************************************************************************
// Subsample a point cloud using the M_SUBSAMPLE_GRID subsampling mode.    
//****************************************************************************
void GridSubsamplePointCloud(MIL_ID Mil3dDisplay,
                             MIL_ID MilSubsampleContext,
                             MIL_ID MilPointCloud,
                             MIL_ID MilDstPointCloud)
   {
   constexpr MIL_DOUBLE GRID_SIZE = 1.5;

   // Set the subsample mode of the 3D image processing context to grid and subsample the point cloud.
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GRID);
   M3dimControl(MilSubsampleContext, M_ORGANIZATION_TYPE, M_ORGANIZED);
   M3dimControl(MilSubsampleContext, M_GRID_SIZE_X, GRID_SIZE);
   M3dimControl(MilSubsampleContext, M_GRID_SIZE_Y, GRID_SIZE);
   M3dimControl(MilSubsampleContext, M_GRID_SIZE_Z, M_INFINITE);

   // Subsample and display the point cloud.
   SubsampleAndDisplayResult(Mil3dDisplay, MilSubsampleContext, MilPointCloud, MilDstPointCloud);

   MosPrintf(MIL_TEXT("Grid size (X and Y)              : %.4f\n\n"), GRID_SIZE);
   }

//****************************************************************************
// Subsample a point cloud using the M_SUBSAMPLE_NORMAL subsampling mode.    
//****************************************************************************
void NormalSubsamplePointCloud(MIL_ID Mil3dDisplay,
                               MIL_ID MilSubsampleContext,
                               MIL_ID MilPointCloud,
                               MIL_ID MilDstPointCloud)
   {
   constexpr MIL_DOUBLE NEIGHBORHOOD_DISTANCE = 3;
   constexpr MIL_INT DISTINCT_ANGLE_DIFFERENCE = 8;

   // Set the subsample mode of the 3D image processing context to normal and subsample the point cloud.
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_NORMAL);
   M3dimControl(MilSubsampleContext, M_NEIGHBORHOOD_DISTANCE, NEIGHBORHOOD_DISTANCE);
   M3dimControl(MilSubsampleContext, M_DISTINCT_ANGLE_DIFFERENCE, DISTINCT_ANGLE_DIFFERENCE);
   M3dimControl(MilSubsampleContext, M_ORGANIZATION_TYPE, M_DEFAULT);

   // Subsample and display the point cloud.
   SubsampleAndDisplayResult(Mil3dDisplay, MilSubsampleContext, MilPointCloud, MilDstPointCloud);

   MosPrintf(MIL_TEXT("Neighborhood point distance      : %.4f\n"), NEIGHBORHOOD_DISTANCE);
   MosPrintf(MIL_TEXT("Distinct angle difference        : %6d\n\n"), DISTINCT_ANGLE_DIFFERENCE);
   }

//****************************************************************************
// Subsample a point cloud using the M_SUBSAMPLE_RANDOM subsampling mode.    
//****************************************************************************
void RandomSubsamplePointCloud(MIL_ID Mil3dDisplay,
                               MIL_ID MilSubsampleContext,
                               MIL_ID MilPointCloud,
                               MIL_ID MilDstPointCloud)
   {
   constexpr MIL_DOUBLE RANDOM_FRACTION_OF_POINTS = 0.035;

   // Set the subsample mode of the 3D image processing context to random and subsample the point cloud.
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_RANDOM);
   M3dimControl(MilSubsampleContext, M_FRACTION_OF_POINTS, RANDOM_FRACTION_OF_POINTS);

   // Subsample and display the point cloud.
   SubsampleAndDisplayResult(Mil3dDisplay, MilSubsampleContext, MilPointCloud, MilDstPointCloud);

   MosPrintf(MIL_TEXT("Fraction of points               : %.4f\n\n"), RANDOM_FRACTION_OF_POINTS);
   }

//****************************************************************************
// Subsample a point cloud based on the controls set in the subsample context.    
//****************************************************************************
void SubsampleAndDisplayResult(MIL_ID Mil3dDisplay,
                               MIL_ID MilSubsampleContext,
                               MIL_ID MilPointCloud,
                               MIL_ID MilDstPointCloud)
   {
   // Time the operation and subsample the point cloud.
   auto StartTime = MappTimer(M_TIMER_READ, M_NULL); // In s.
   M3dimSample(MilSubsampleContext, MilPointCloud, MilDstPointCloud, M_DEFAULT);
   auto EndTime = MappTimer(M_TIMER_READ, M_NULL); // In s.
   auto TimeTaken = (MIL_INT) ((EndTime - StartTime) * 1000);

   // Display the subsampled point cloud.
   DisplayPointCloud(Mil3dDisplay, MilDstPointCloud);

   // Calculate the amount of points in the subsampled point cloud.
   auto MilSystem = MobjInquire(MilPointCloud, M_OWNER_SYSTEM, M_NULL);
   auto MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_NUMBER_OF_POINTS, MilDstPointCloud, MilStatResult, M_DEFAULT);
   MIL_INT NumSubsampledPoints;

   // Display the number of points in the subsampled point cloud, and the time the operation took.
   M3dimGetResult(MilStatResult, M_NUMBER_OF_POINTS_TOTAL, &NumSubsampledPoints);
   MosPrintf(MIL_TEXT("Number of points post-subsampling: %6d\n"), NumSubsampledPoints);
   MosPrintf(MIL_TEXT("Processing time                  : %3d ms\n"), TimeTaken);
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

//*****************************************************************************
// Configure comparison 3D displays.  
//*****************************************************************************
void ConfigureComparison3dDisplays(MIL_ID MilSystem, std::vector<MIL_UNIQUE_3DDISP_ID>& MilComparison3dDisplays)
   {
   constexpr MIL_INT WINDOW_SIZE_X = 320;

   MIL_STRING SUBSAMPLE_MODES[] = {MIL_TEXT("M_SUBSAMPLE_DECIMATE" ),
                                   MIL_TEXT("M_SUBSAMPLE_GEOMETRIC"),
                                   MIL_TEXT("M_SUBSAMPLE_GRID"     ),
                                   MIL_TEXT("M_SUBSAMPLE_NORMAL"   ),
                                   MIL_TEXT("M_SUBSAMPLE_RANDOM"   )
                                  };

   for(MIL_INT i = 0; i < (MIL_INT) MilComparison3dDisplays.size(); i++)
      {
      MilComparison3dDisplays[i] = Alloc3dDisplayId(MilSystem);
      M3ddispSetView(MilComparison3dDisplays[i], M_AUTO, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      M3ddispControl(MilComparison3dDisplays[i], M_SIZE_X, WINDOW_SIZE_X);

      M3ddispControl(MilComparison3dDisplays[i], M_TITLE, SUBSAMPLE_MODES[i]);
      M3ddispControl(MilComparison3dDisplays[i], M_WINDOW_INITIAL_POSITION_X, WINDOW_SIZE_X*i);
      }
   }

//*****************************************************************************
// Small wrapper function to display point clouds with color.
//*****************************************************************************
void DisplayPointCloud(MIL_ID MilDisplay, MIL_ID MilPointCloud)
   {
   auto MilSrcPointCloudLabel = M3ddispSelect(MilDisplay, MilPointCloud, M_SELECT, M_DEFAULT);
   auto MilSrcGraList = M3ddispInquire(MilDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraControl(MilSrcGraList, MilSrcPointCloudLabel, M_COLOR_COMPONENT, M_COMPONENT_NORMALS_MIL);
   }


//*****************************************************************************
// Adds the component M_COMPONENT_NORMALS_MIL if it's not found.
//*****************************************************************************
void AddComponentNormalsIfMissing(MIL_ID MilContainer)
   {
   MIL_ID MilNormals =
      MbufInquireContainer(MilContainer, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL);

   if(MilNormals != M_NULL)
      return;
   MIL_INT SizeX = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   if(SizeX < 50 || SizeY < 50)
      M3dimNormals(M_NORMALS_CONTEXT_TREE, MilContainer, MilContainer, M_DEFAULT);
   else
      M3dimNormals(M_NORMALS_CONTEXT_ORGANIZED, MilContainer, MilContainer, M_DEFAULT);
   }

