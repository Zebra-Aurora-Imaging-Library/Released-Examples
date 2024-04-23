//***************************************************************************************
//
// File name: PointCloudsRegistration.h
//
// Synopsis:  Utility header that contains the implementation of functions to perform
//            the 3d registration and combination of point clouds.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************

#pragma once

//****************************************************************************
// Function declaration for alignment and merging.
//****************************************************************************
MIL_DOUBLE EstimatePointCloudOverlap(MIL_ID MilModelPointCloud, MIL_ID MilToAlignPointCloud, MIL_ID MilTransformMatrix, MIL_DOUBLE OverlapPercentage = 90.0, MIL_DOUBLE SubsampleFraction = 1.0);
MIL_UNIQUE_3DREG_ID RegisterPointClouds(MIL_ID MilModelPointCloud, const std::vector<MIL_ID>& MilToAlignPointClouds, Camera3dDataSource DataSource = Camera3dDataSource::Example);
MIL_UNIQUE_BUF_ID MergePointClouds(MIL_ID MilRegResult, const std::vector<MIL_ID>& MilToMergePointClouds);
void SetLocationBasedOnOverlap(MIL_ID Mil3dRegContext, MIL_ID MilTransformMatrix, MIL_ID MilRefPointCloud, MIL_ID MilToAlignPointCloud, MIL_INT Reference, MIL_INT Index);

//****************************************************************************
// Estimates the overlap between two point clouds.
//****************************************************************************
MIL_DOUBLE EstimatePointCloudOverlap(MIL_ID MilModelPointCloud, MIL_ID MilToAlignPointCloud, MIL_ID MilTransformMatrix, MIL_DOUBLE OverlapPercentage, MIL_DOUBLE SubsampleFraction)
   {
   // Prealign the point cloud using the provided matrix.
   auto MilInitAlignPointCloud = MbufAllocContainer(M_DEFAULT_HOST, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   M3dimMatrixTransform(MilToAlignPointCloud, MilInitAlignPointCloud, MilTransformMatrix, M_DEFAULT);

   // Subsample the point clouds to speed up the distance processing.
   auto MilSubsampleContext = M3dimAlloc(M_DEFAULT_HOST, M_SUBSAMPLE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_RANDOM);
   M3dimControl(MilSubsampleContext, M_FRACTION_OF_POINTS, SubsampleFraction);   
   auto MilSubModelPointCloud = MbufAllocContainer(M_DEFAULT_HOST, M_PROC, M_DEFAULT, M_UNIQUE_ID);
   auto MilSubInitAlignPointCloud = MbufAllocContainer(M_DEFAULT_HOST, M_PROC, M_DEFAULT, M_UNIQUE_ID);
   M3dimSample(MilSubsampleContext, MilModelPointCloud, MilSubModelPointCloud, M_DEFAULT);
   M3dimSample(MilSubsampleContext, MilInitAlignPointCloud, MilSubInitAlignPointCloud, M_DEFAULT);

   // Compute the distance between the range components.
   auto MilSubRangeComponent = MbufInquireContainer(MilSubModelPointCloud, M_COMPONENT_RANGE, M_COMPONENT_ID, M_NULL);
   auto MilSubConfidenceComponent = MbufInquireContainer(MilSubModelPointCloud, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   MIL_INT SubRangeSizeX = MbufInquire(MilSubRangeComponent, M_SIZE_X, M_NULL);
   MIL_INT SubRangeSizeY = MbufInquire(MilSubRangeComponent, M_SIZE_Y, M_NULL);
   auto MilSubDistanceImage = MbufAlloc2d(M_DEFAULT_HOST, SubRangeSizeX, SubRangeSizeY, 32 + M_FLOAT, M_IMAGE + M_PROC, M_UNIQUE_ID);
   M3dmetDistance(MilSubModelPointCloud, MilSubInitAlignPointCloud, MilSubDistanceImage, M_DISTANCE_TO_NEAREST_NEIGHBOR, M_DEFAULT, M_DEFAULT);
   MbufClearCond(MilSubDistanceImage, 0, 0, 0, MilSubDistanceImage, M_EQUAL, MIL_FLOAT_MAX);

   // Compute the distance threshold using the triangle bisection algorithm.
   auto MilSubDistanceThresholdImage = MbufAlloc2d(M_DEFAULT_HOST, SubRangeSizeX, SubRangeSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, M_UNIQUE_ID);
   MimRemap(M_DEFAULT, MilSubDistanceImage, MilSubDistanceThresholdImage, M_FIT_SRC_DATA);
   MbufSetRegion(MilSubDistanceThresholdImage, MilSubConfidenceComponent, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MimBinarize(MilSubDistanceThresholdImage, MilSubDistanceThresholdImage, M_TRIANGLE_BISECTION_BRIGHT + M_LESS, 0, 255);

   // Estimate the overlap based on the distance threshold.
   MIL_INT NbOverlap = MimLocateEvent(MilSubDistanceThresholdImage, M_NULL, M_EQUAL, 255, M_NULL);
   MIL_INT NbValid = MimLocateEvent(MilSubConfidenceComponent, M_NULL, M_NOT_EQUAL, 0, M_NULL);
   MIL_DOUBLE DistanceThresholdOverlap = (MIL_DOUBLE)NbOverlap / NbValid * OverlapPercentage;

   auto MilRangeComponent = MbufInquireContainer(MilModelPointCloud, M_COMPONENT_RANGE, M_COMPONENT_ID, M_NULL);
   auto MilConfidenceComponent = MbufInquireContainer(MilModelPointCloud, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   MIL_INT RangeSizeX = MbufInquire(MilRangeComponent, M_SIZE_X, M_NULL);
   MIL_INT RangeSizeY = MbufInquire(MilRangeComponent, M_SIZE_Y, M_NULL);
   auto MilDistanceImage = MbufAlloc2d(M_DEFAULT_HOST, RangeSizeX, RangeSizeY, 32 + M_FLOAT, M_IMAGE + M_PROC, M_UNIQUE_ID);

   // Generate the depth map of the point cloud to align.
   auto MilInitAlignDepthMap = GenerateDepthMap(MilInitAlignPointCloud);

   // Compute the Z distance with the depth map.
   M3dmetDistance(MilModelPointCloud, MilInitAlignDepthMap, MilDistanceImage, M_ABSOLUTE_DISTANCE_Z_TO_SURFACE, M_DEFAULT, M_DEFAULT);

   // Estimate the overlap based on the number of valid distance measures.
   NbOverlap = MimLocateEvent(MilDistanceImage, M_NULL, M_NOT_EQUAL, MIL_FLOAT_MAX, M_NULL);
   NbValid = MimLocateEvent(MilConfidenceComponent, M_NULL, M_NOT_EQUAL, 0, M_NULL);
   MIL_DOUBLE XYOverlap = (MIL_DOUBLE)NbOverlap / NbValid * OverlapPercentage;

   // Keep the maximum of the two measured overlaps.
   MIL_DOUBLE Overlap = XYOverlap > DistanceThresholdOverlap ? XYOverlap : DistanceThresholdOverlap;

   return Overlap;
   }

//****************************************************************************
// Gets the initial alignment index for the pre-alignment.
// Set to -1 if the source is not the default example sample images.
//****************************************************************************
MIL_INT ExampleAlignIndex(Camera3dDataSource DataSource, MIL_INT i)
   {
   return DataSource == Camera3dDataSource::Example ? i : -1;
   }

//****************************************************************************
// Registers the point clouds.
//****************************************************************************
static const MIL_INT MAX_ITERATION = 200;
static const MIL_INT DECIMATION_STEP = 16;
MIL_UNIQUE_3DREG_ID RegisterPointClouds(MIL_ID MilModelPointCloud, const std::vector<MIL_ID>& MilToAlignPointClouds, Camera3dDataSource DataSource)
   {
   // Allocate context and result for 3D registration (stitching).
   auto MilContext = M3dregAlloc(M_DEFAULT_HOST, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto MilResult = M3dregAllocResult(M_DEFAULT_HOST, M_PAIRWISE_REGISTRATION_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Set up the registration.
   M3dregControl(MilContext, M_DEFAULT, M_NUMBER_OF_REGISTRATION_ELEMENTS, MilToAlignPointClouds.size() + (MilModelPointCloud ? 1 : 0));
   M3dregControl(MilContext, M_DEFAULT, M_MAX_ITERATIONS, MAX_ITERATION);
   M3dregControl(MilContext, M_DEFAULT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_POINT);

   // Use decimation for subsampling.
   MIL_ID MilSubsampleContext = M_NULL;
   M3dregInquire(MilContext, M_DEFAULT, M_SUBSAMPLE_CONTEXT_ID, &MilSubsampleContext);
   M3dregControl(MilContext, M_DEFAULT, M_SUBSAMPLE, M_ENABLE);
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_DECIMATE);
   M3dimControl(MilSubsampleContext, M_STEP_SIZE_X, DECIMATION_STEP);
   M3dimControl(MilSubsampleContext, M_STEP_SIZE_Y, DECIMATION_STEP);

   // Set the rough location of the point clouds based on the user input.
   for(MIL_INT p = 0; p < (MIL_INT)MilToAlignPointClouds.size(); p++)
      {
      // Align with the model. Otherwise align with the previous.
      MosPrintf(MIL_TEXT("Preparing 3D data %i for alignment...\n\n"), p);
      if(MilModelPointCloud)
         {
         auto MilTransformMatrix = AlignDepthMapPair(MilModelPointCloud, MilToAlignPointClouds[p], ExampleAlignIndex(DataSource, 2), ExampleAlignIndex(DataSource, p));
         SetLocationBasedOnOverlap(MilContext, MilTransformMatrix, MilModelPointCloud, MilToAlignPointClouds[p], MilToAlignPointClouds.size(), p);
         }
      else if(p == 0)
         {
         MosPrintf(MIL_TEXT("3D data 0 is set as the global reference.\n"), p);
         M3dregSetLocation(MilContext, p, M_REGISTRATION_GLOBAL, M_IDENTITY_MATRIX, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         }
      else
         {
         auto MilTransformMatrix = AlignDepthMapPair(MilToAlignPointClouds[p - 1], MilToAlignPointClouds[p], ExampleAlignIndex(DataSource, p-1), ExampleAlignIndex(DataSource, p));
         SetLocationBasedOnOverlap(MilContext, MilTransformMatrix, MilToAlignPointClouds[p - 1], MilToAlignPointClouds[p], p - 1, p);
         }
      MosPrintf(MIL_TEXT("\n3D data %d is prepared for alignment.\n"), p);
      if(p < (MIL_INT)MilToAlignPointClouds.size()-1)
         MosPrintf(MIL_TEXT("\n"));
      }

   // Create the array of all the point clouds for the registration process.
   std::vector<MIL_ID> MilAllPointClouds(MilToAlignPointClouds);
   MIL_INT StartReg;
   MIL_INT EndReg = MilAllPointClouds.size();
   if(MilModelPointCloud)
      {
      StartReg = 0;
      MilAllPointClouds.push_back(MilModelPointCloud);
      M3dregSetLocation(MilContext, MilAllPointClouds.size() - 1, M_REGISTRATION_GLOBAL, M_IDENTITY_MATRIX, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      }
   else
      StartReg = 1;

   // Perform the registration.
   MosPrintf(MIL_TEXT("\nRegistering the point clouds...\n"));
   MappTimer(M_TIMER_RESET, M_NULL);
   M3dregCalculate(MilContext, MilAllPointClouds, M_DEFAULT, MilResult, M_DEFAULT);
   MIL_DOUBLE ComputationTime = MappTimer(M_TIMER_READ, M_NULL) * 1000.0;

   MosPrintf(MIL_TEXT("The 3D registration between the data has been completed in %.3f ms.\n\n"), ComputationTime);
   bool AllValid = true;
   for(MIL_INT r = StartReg; r < (MIL_INT)EndReg; r++)
      {
      MIL_INT RegistrationStatus;
      M3dregGetResult(MilResult, r, M_STATUS_REGISTRATION_ELEMENT + M_TYPE_MIL_INT, &RegistrationStatus);

      // Interpret the result status.
      MosPrintf(MIL_TEXT("Registration of 3D data %d "), r);

      switch(RegistrationStatus)
         {
         case M_NOT_INITIALIZED:
            MosPrintf(MIL_TEXT("failed: uninitialized registration result.\n"), r);
            AllValid = false;
            break;
         case M_NOT_ENOUGH_POINT_PAIRS:
            MosPrintf(MIL_TEXT("failed: insufficient overlap.\n"));
            AllValid = false;
            break;
         case M_MAX_ITERATIONS_REACHED:
            MosPrintf(MIL_TEXT("uncertain: the maximum number of iterations was\n"));
            MosPrintf(MIL_TEXT("reached before convergence.\n"));

            break;
         case M_RMS_ERROR_THRESHOLD_REACHED:
         case M_RMS_ERROR_RELATIVE_THRESHOLD_REACHED:
            MIL_DOUBLE RegisterRmsError;
            M3dregGetResult(MilResult, r, M_RMS_ERROR + M_TYPE_MIL_DOUBLE, &RegisterRmsError);
            MosPrintf(MIL_TEXT("succeeded with an RMS error of %f mm.\n"), RegisterRmsError);
            break;
         default:
            MosPrintf(MIL_TEXT("failed: unknown registration status.\n"));
            AllValid = false;
         }
      }
   MosPrintf(MIL_TEXT("\n"));

   if(!AllValid)
      MilResult.reset();
   return MilResult;
   }

//****************************************************************************
// Set the location based on the estimate overlap.
//****************************************************************************
void SetLocationBasedOnOverlap(MIL_ID Mil3dRegContext, MIL_ID MilTransformMatrix, MIL_ID MilRefPointCloud, MIL_ID MilToAlignPointCloud, MIL_INT Reference, MIL_INT Index)
   {
   MosPrintf(MIL_TEXT("Estimating initial overlap...\n"));
   auto Overlap = EstimatePointCloudOverlap(MilRefPointCloud, MilToAlignPointCloud, MilTransformMatrix, 90, 0.1);
   MosPrintf(MIL_TEXT("The estimated overlap is %.2f %%\n"), Overlap);
   M3dregSetLocation(Mil3dRegContext, Index, Reference, MilTransformMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3dregControl(Mil3dRegContext, Index, M_OVERLAP, Overlap);
   }

//****************************************************************************
// Merges the point clouds.
//****************************************************************************
MIL_UNIQUE_BUF_ID MergePointClouds(MIL_ID MilRegResult, const std::vector<MIL_ID>& MilToMergePointClouds)
   {
   // Allocate the point cloud for the final stitched clouds.
   MIL_ID MilSystem = MbufInquire(MilToMergePointClouds[0], M_OWNER_SYSTEM, M_NULL);
   auto MilMergedPointCloud = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);

   // Merge all point clouds.
   if(MilRegResult)
      M3dregMerge(MilRegResult, MilToMergePointClouds, M_DEFAULT, MilMergedPointCloud, M_NULL, M_DEFAULT);
   else
      M3dimMerge(MilToMergePointClouds, MilMergedPointCloud, M_DEFAULT, M_NULL, M_DEFAULT);

   return MilMergedPointCloud;
   }

