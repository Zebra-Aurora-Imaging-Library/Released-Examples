/*****************************************************************************/
/* 
* File name: M3dreg.cpp
*
* Synopsis: This example demonstrates how to use the 3D registration module
*           to stitch several partial point clouds of a 3D object together
*           into a single complete point cloud.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>

/* -------------------------------------------------------------- */
/* Example description.                                           */
/* -------------------------------------------------------------- */
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("M3dreg\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to use the 3D Registration module \n"));
   MosPrintf(MIL_TEXT("to stitch several partial point clouds of a 3D object together \n"));

   MosPrintf(MIL_TEXT("into a single complete point cloud.\n"));
   MosPrintf(MIL_TEXT("\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: 3D Registration, 3D Display, 3D Graphics, and 3D Image\nProcessing.\n\n"));
   }

/* Input scanned point cloud (PLY) files. */
static const MIL_INT NUM_SCANS = 6;
static const MIL_TEXT_CHAR* const FILE_POINT_CLOUD[NUM_SCANS] =
   {
   M_IMAGE_PATH MIL_TEXT("Cloud1.ply"),
   M_IMAGE_PATH MIL_TEXT("Cloud2.ply"),
   M_IMAGE_PATH MIL_TEXT("Cloud3.ply"),
   M_IMAGE_PATH MIL_TEXT("Cloud4.ply"),
   M_IMAGE_PATH MIL_TEXT("Cloud5.ply"),
   M_IMAGE_PATH MIL_TEXT("Cloud6.ply"),
   }; 

/* The colors assigned for each cloud. */
const MIL_INT Color[NUM_SCANS + 1] =
   {
   M_RGB888(0,   159, 255),
   M_RGB888(154,  77,  66),
   M_RGB888(0,   255, 190),
   M_RGB888(120,  63, 193),
   M_RGB888(31,  150, 152),
   M_RGB888(255, 172, 253),
   M_RGB888(177, 204, 113)
   };

/* Utility functions. */
void   ColorCloud(MIL_ID MilPointCloud, MIL_INT Col);
void   AutoRotateDisplay(MIL_ID MilSystem, MIL_ID MilDisplay);
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);
void   DisplayContainer(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilContainer, MIL_ID* pMilDepthMap, MIL_ID* pMilIntensityMap);
void   UpdateDisplay(MIL_ID MilSystem, MIL_ID MilContainer, MIL_ID MilDepthMap, MIL_ID MilIntensityMap);
void   FreeDisplay(MIL_ID MilDisplay);
/* -------------------------------------------------------------- */

int MosMain()
   {
   /* Print example information in console. */
   PrintHeader();

   MIL_ID MilApplication,    /* MIL application identifier  */
          MilSystem,         /* MIL system identifier       */
          MilDisplay;        /* MIL display identifier      */
   
   /* Allocate MIL objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_DEFAULT, M_DEFAULT, M_DEFAULT, &MilSystem);

   MIL_ID MilContainerIds[NUM_SCANS];
   MosPrintf(MIL_TEXT("Reading the PLY files of 6 partial point clouds"));
   for(MIL_INT i = 0; i < NUM_SCANS; ++i)
      {
      MosPrintf(MIL_TEXT("."));
      MilContainerIds[i] = MbufImport(FILE_POINT_CLOUD[i], M_DEFAULT, M_RESTORE, MilSystem, M_NULL);
      ColorCloud(MilContainerIds[i], Color[i]);
      }
   MosPrintf(MIL_TEXT("\n\n"));

   MilDisplay = Alloc3dDisplayId(MilSystem);

   MIL_ID MilDisplayImage = M_NULL; /* Used for 2D display if needed. */
   MIL_ID MilDepthMap = M_NULL;     /* Used for 2D display if needed. */

   /* Display the first point cloud container. */
   DisplayContainer(MilSystem, MilDisplay, MilContainerIds[0], &MilDepthMap, &MilDisplayImage);
   AutoRotateDisplay(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("Showing the first partial point cloud of the object.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();
  
   /* Allocate context and result for 3D registration (stitching). */
   MIL_ID MilContext = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_NULL);
   MIL_ID MilResult  = M3dregAllocResult(MilSystem, M_PAIRWISE_REGISTRATION_RESULT, M_DEFAULT, M_NULL);

   M3dregControl(MilContext, M_DEFAULT, M_NUMBER_OF_REGISTRATION_ELEMENTS, NUM_SCANS);
   M3dregControl(MilContext, M_DEFAULT, M_MAX_ITERATIONS, 40);

   /* Pairwise registration context controls.                                 */
   /* Use normal subsampling to preserve edges and yield faster registration. */
   MIL_ID MilSubsampleContext = M_NULL;
   M3dregInquire(MilContext, M_DEFAULT, M_SUBSAMPLE_CONTEXT_ID, &MilSubsampleContext);
   M3dregControl(MilContext, M_DEFAULT, M_SUBSAMPLE, M_ENABLE);

   /* Keep edge points. */
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_NORMAL);
   M3dimControl(MilSubsampleContext, M_NEIGHBORHOOD_DISTANCE, 10);
 
   /* Chain of set location, i==0 is referencing to the GLOBAL. */
   for(MIL_INT i = 1; i < NUM_SCANS; ++i)
      { M3dregSetLocation(MilContext, i, i-1, M_IDENTITY_MATRIX, M_DEFAULT, M_DEFAULT, M_DEFAULT); }

   MosPrintf(MIL_TEXT("The 3D stitching between partial point clouds has been performed with\n")
             MIL_TEXT("the help of the points within the expected common overlap regions.\n\n"));

   /* Calculate the time to perform the registration. */
   MIL_DOUBLE ComputationTimeMS;
   MappTimer(M_TIMER_RESET, M_NULL);

   /* Perform the registration (stitching). */
   M3dregCalculate(MilContext, MilContainerIds, NUM_SCANS, MilResult, M_DEFAULT);

   ComputationTimeMS = MappTimer(M_TIMER_READ, M_NULL) * 1000.0;

   MosPrintf(MIL_TEXT("The registration of the 6 partial point clouds succeeded in %.2f ms.\n\n"),
             ComputationTimeMS);

   /* Merging overlapping point clouds will result into unneeded large number of points at */
   /* the overlapping regions.                                                             */
   /* During merging, subsampling is used to ensure that the density of points             */
   /* is fairly dense, but without replications.                                           */
   MIL_DOUBLE GridSize;
   MIL_ID StatResultId = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_NULL);
   M3dimStat(M_STAT_CONTEXT_DISTANCE_TO_NEAREST_NEIGHBOR, MilContainerIds[0], StatResultId, M_DEFAULT);

   /* Nearest neighbour distances gives a measure of the point cloud density. */
   M3dimGetResult(StatResultId, M_DISTANCE_TO_NEAREST_NEIGHBOR_MIN, &GridSize);

   /* Use the measured point cloud density as guide for the subsampling. */
   MIL_ID MilMergeSubsampleContext = M3dimAlloc(MilSystem , M_SUBSAMPLE_CONTEXT, M_DEFAULT, M_NULL);
   M3dimControl(MilMergeSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GRID);
   M3dimControl(MilMergeSubsampleContext, M_GRID_SIZE_X, GridSize);
   M3dimControl(MilMergeSubsampleContext, M_GRID_SIZE_Y, GridSize);
   M3dimControl(MilMergeSubsampleContext, M_GRID_SIZE_Z, GridSize);

   /* Allocate the point cloud for the final stitched clouds. */
   MIL_ID MilStitchedId = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_NULL);

   MosPrintf(MIL_TEXT("The merging of point clouds will be shown incrementally.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to show 2 merged point clouds of 6.\n\n"));
   MosGetch();

   /* Merge can merge all clouds at once, but here it is done incrementally for the display. */
   for(MIL_INT i = 2; i <= NUM_SCANS; ++i)
      {
      M3dregMerge(MilResult, MilContainerIds, i, MilStitchedId, MilMergeSubsampleContext, M_DEFAULT);

      if(i == 2)
         { DisplayContainer(MilSystem, MilDisplay, MilStitchedId, &MilDepthMap, &MilDisplayImage); }
      else
         { UpdateDisplay(MilSystem, MilStitchedId, MilDepthMap, MilDisplayImage); }

      if(i < NUM_SCANS)
         {
         MosPrintf(MIL_TEXT("Press <Enter> to show %d merged point clouds of 6.\n\n"), i + 1);
         }
      else
         {
         MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
         }

      AutoRotateDisplay(MilSystem, MilDisplay);
      MosGetch();
      }
    
   /* Free Objects. */
   for(MIL_INT i = 0; i < NUM_SCANS; ++i)
      { MbufFree(MilContainerIds[i]); }

   MbufFree(MilStitchedId);
   M3dimFree(StatResultId);
   M3dimFree(MilMergeSubsampleContext);
   M3dregFree(MilContext);
   M3dregFree(MilResult);
   FreeDisplay(MilDisplay);
   if(MilDisplayImage)
      MbufFree(MilDisplayImage);
   if(MilDepthMap)
      MbufFree(MilDepthMap);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

/* -------------------------------------------------------------- */
/* Color the container.                                           */
/* -------------------------------------------------------------- */
void ColorCloud(MIL_ID MilPointCloud, MIL_INT Col)
   {
   MIL_INT SizeX = MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   MIL_ID ReflectanceId = MbufAllocComponent(MilPointCloud, 3, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE, M_COMPONENT_REFLECTANCE, M_NULL);
   MbufClear(ReflectanceId, static_cast<MIL_DOUBLE>(Col));
   }

/* -------------------------------------------------------------- */
/* Auto rotate the 3D object.                                     */
/* -------------------------------------------------------------- */
void AutoRotateDisplay(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_INT64 DisplayType;
   MobjInquire(MilDisplay, M_OBJECT_TYPE, &DisplayType);

   /* AutoRotate available only for the 3D display. */
   if(DisplayType != M_3D_DISPLAY)
      return;

   /* By default the display rotates around the Z axis, but the robot is oriented along the Y axis. */
   MIL_ID Geometry = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);
   M3ddispCopy(MilDisplay, Geometry, M_ROTATION_AXIS, M_DEFAULT);
   MIL_DOUBLE CenterX = M3dgeoInquire(Geometry, M_START_POINT_X, M_NULL);
   MIL_DOUBLE CenterY = M3dgeoInquire(Geometry, M_START_POINT_Y, M_NULL);
   MIL_DOUBLE CenterZ = M3dgeoInquire(Geometry, M_START_POINT_Z, M_NULL);
   M3dgeoLine(Geometry, M_POINT_AND_VECTOR, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED, 0, 1, 0, M_UNCHANGED, M_DEFAULT);
   M3ddispCopy(Geometry, MilDisplay, M_ROTATION_AXIS, M_DEFAULT);
   M3ddispControl(MilDisplay, M_AUTO_ROTATE, M_ENABLE);
   M3dgeoFree(Geometry);
   }

/* -------------------------------------------------------------- */
/* Allocates a 3D display and returns its MIL identifier.           */
/* -------------------------------------------------------------- */
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_ID MilDisplay = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("A 2D display will be used instead.\n")
                MIL_TEXT("Press any key to continue.\n"));
      MosGetch();

      /* Allocate a 2D Display instead. */
      MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
      }
   else
      {
      /* Adjust the viewpoint of the 3D display. */
      M3ddispSetView(MilDisplay, M_AUTO, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MosPrintf(MIL_TEXT("Press <R> on the display window to stop/start the rotation.\n\n"));
      }

   return MilDisplay;
   }

/* -------------------------------------------------------------- */
/* Display the received container.                                */
/* -------------------------------------------------------------- */
void DisplayContainer(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilContainer, MIL_ID* pMilDepthMap, MIL_ID* pMilIntensityMap)
   {
   MIL_INT64 DisplayType;
   MobjInquire(MilDisplay, M_OBJECT_TYPE, &DisplayType);

   bool Use3D = (DisplayType == M_3D_DISPLAY);
   if(Use3D)
      {
      M3ddispSelect(MilDisplay, MilContainer, M_ADD, M_DEFAULT);
      M3ddispSelect(MilDisplay, M_NULL, M_OPEN, M_DEFAULT);
      }
   else
      {
      if(*pMilDepthMap == M_NULL)
         {
         MIL_INT SizeX = 0; /* Image size X for 2d display. */
         MIL_INT SizeY = 0; /* Image size Y for 2d display. */

         M3dimCalculateMapSize(M_DEFAULT, MilContainer, M_NULL, M_DEFAULT, &SizeX, &SizeY);

         *pMilIntensityMap = MbufAllocColor(MilSystem, 3, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE | M_PROC | M_DISP, M_NULL);
         *pMilDepthMap     = MbufAlloc2d   (MilSystem,    SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE | M_PROC | M_DISP, M_NULL);

         M3dimCalibrateDepthMap(MilContainer, *pMilDepthMap, *pMilIntensityMap, M_NULL, M_DEFAULT, M_DEFAULT, M_CENTER);
         }

      /* Rotate the point cloud container to be in the xy plane before projecting. */
      MIL_ID RotatedContainer = MbufAllocContainer(MilSystem, M_PROC, M_DEFAULT, M_NULL);

      M3dimRotate(MilContainer, RotatedContainer, M_ROTATION_XYZ, 90, 60, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);     
      M3dimProject(MilContainer, *pMilDepthMap, *pMilIntensityMap, M_DEFAULT, M_MIN_Z, M_DEFAULT, M_DEFAULT);

      /* Display the projected point cloud container. */
      MdispSelect(MilDisplay, *pMilIntensityMap);
      MbufFree(RotatedContainer);
      }
   }

/* -------------------------------------------------------------- */
/* Updated the displayed image.                                   */
/* -------------------------------------------------------------- */
void UpdateDisplay(MIL_ID MilSystem, MIL_ID MilContainer, MIL_ID MilDepthMap, MIL_ID MilIntensityMap)
   {
   if(!MilDepthMap)
      { return; }

   MIL_ID RotatedContainer = MbufAllocContainer(MilSystem, M_PROC, M_DEFAULT, M_NULL);

   M3dimRotate(MilContainer, RotatedContainer, M_ROTATION_XYZ, 90, 70, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3dimProject(MilContainer, MilDepthMap, MilIntensityMap, M_DEFAULT, M_MIN_Z, M_DEFAULT, M_DEFAULT);

   MbufFree(RotatedContainer);
   }

/* -------------------------------------------------------------- */
/* Free the display.                                              */
/* -------------------------------------------------------------- */
void FreeDisplay(MIL_ID MilDisplay)
   {
   MIL_INT64 DisplayType;
   MobjInquire(MilDisplay, M_OBJECT_TYPE, &DisplayType);

   if(DisplayType == M_DISPLAY)
      { MdispFree(MilDisplay); }
   else
      { M3ddispFree(MilDisplay); }
   }
