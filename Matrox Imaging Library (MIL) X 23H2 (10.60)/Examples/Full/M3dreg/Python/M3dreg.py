#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: M3dreg.py
#
# Synopsis: This example demonstrates how to use the 3D registration module
#           to stitch several partial point clouds of a 3D object together
#           into a single complete point cloud.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################

 
import mil as MIL

# --------------------------------------------------------------
# Example description.                                          
# --------------------------------------------------------------
def PrintHeader():
   print("[EXAMPLE NAME]")
   print("M3dreg\n")

   print("[SYNOPSIS]")
   print("This example demonstrates how to use the 3D Registration module ")
   print("to stitch several partial point clouds of a 3D object together ")

   print("into a single complete point cloud.\n")

   print("[MODULES USED]")
   print("Modules used: 3D Registration, 3D Display, 3D Graphics, and 3D Image\nProcessing.\n")

# Input scanned point cloud (PLY) files. 
NUM_SCANS = 6
FILE_POINT_CLOUD = [
   MIL.M_IMAGE_PATH + "Cloud1.ply",
   MIL.M_IMAGE_PATH + "Cloud2.ply",
   MIL.M_IMAGE_PATH + "Cloud3.ply",
   MIL.M_IMAGE_PATH + "Cloud4.ply",
   MIL.M_IMAGE_PATH + "Cloud5.ply",
   MIL.M_IMAGE_PATH + "Cloud6.ply",
]

# The colors assigned for each cloud. 
Color = [
   MIL.M_RGB888(0,   159, 255),
   MIL.M_RGB888(154,  77,  66),
   MIL.M_RGB888(0,   255, 190),
   MIL.M_RGB888(120,  63, 193),
   MIL.M_RGB888(31,  150, 152),
   MIL.M_RGB888(255, 172, 253),
   MIL.M_RGB888(177, 204, 113)
]

def M3dregExample():
   # Print example information in console. 
   PrintHeader()
   
   # Allocate MIL objects. 
   MilApplication = MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT)
   MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   MilContainerIds = [None] * NUM_SCANS
   print("Reading the PLY files of 6 partial point clouds", end="")
   for i in range(NUM_SCANS):
      print(".", end="")
      MilContainerIds[i] = MIL.MbufImport(FILE_POINT_CLOUD[i], MIL.M_DEFAULT, MIL.M_RESTORE, MilSystem, MIL.M_NULL)
      ColorCloud(MilContainerIds[i], Color[i])

   print("\n")

   MilDisplay = Alloc3dDisplayId(MilSystem)

   MilDisplayImage = MIL.M_NULL # Used for 2D display if needed. 
   MilDepthMap = MIL.M_NULL     # Used for 2D display if needed. 

   # Display the first point cloud container. 
   MilDepthMap, MilDisplayImage = DisplayContainer(MilSystem, MilDisplay, MilContainerIds[0], MilDepthMap, MilDisplayImage)
   AutoRotateDisplay(MilSystem, MilDisplay)

   print("Showing the first partial point cloud of the object.")
   print("Press <Enter> to start.\n")
   MIL.MosGetch()
  
   # Allocate context and result for 3D registration (stitching). 
   MilContext = MIL.M3dregAlloc(MilSystem, MIL.M_PAIRWISE_REGISTRATION_CONTEXT, MIL.M_DEFAULT, MIL.M_NULL)
   MilResult  = MIL.M3dregAllocResult(MilSystem, MIL.M_PAIRWISE_REGISTRATION_RESULT, MIL.M_DEFAULT, MIL.M_NULL)

   MIL.M3dregControl(MilContext, MIL.M_DEFAULT, MIL.M_NUMBER_OF_REGISTRATION_ELEMENTS, NUM_SCANS)
   MIL.M3dregControl(MilContext, MIL.M_DEFAULT, MIL.M_MAX_ITERATIONS, 40)

   # Pairwise registration context controls.                                 
   # Use normal subsampling to preserve edges and yield faster registration. 
   MilSubsampleContext = MIL.M_NULL
   MilSubsampleContext = MIL.M3dregInquire(MilContext, MIL.M_DEFAULT, MIL.M_SUBSAMPLE_CONTEXT_ID)
   MIL.M3dregControl(MilContext, MIL.M_DEFAULT, MIL.M_SUBSAMPLE, MIL.M_ENABLE)

   # Keep edge points. 
   MIL.M3dimControl(MilSubsampleContext, MIL.M_SUBSAMPLE_MODE, MIL.M_SUBSAMPLE_NORMAL)
   MIL.M3dimControl(MilSubsampleContext, MIL.M_NEIGHBORHOOD_DISTANCE, 10)
 
   # Chain of set location, i==0 is referencing to the GLOBAL. 
   for i in range(1, NUM_SCANS):
      MIL.M3dregSetLocation(MilContext, i, i-1, MIL.M_IDENTITY_MATRIX, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   print("The 3D stitching between partial point clouds has been performed with")
   print("the help of the points within the expected common overlap regions.\n")

   # Calculate the time to perform the registration. 
   MIL.MappTimer(MilApplication, MIL.M_TIMER_RESET, MIL.M_NULL)

   # Perform the registration (stitching). 
   MIL.M3dregCalculate(MilContext, MilContainerIds, NUM_SCANS, MilResult, MIL.M_DEFAULT)

   ComputationTimeMS = MIL.MappTimer(MilApplication, MIL.M_TIMER_READ, MIL.M_NULL) * 1000.0

   print("The registration of the 6 partial point clouds succeeded in {ComputationTimeMS:.2f} ms.\n".format(ComputationTimeMS=ComputationTimeMS))

   # Merging overlapping point clouds will result into unneeded large number of points at 
   # the overlapping regions.                                                             
   # During merging, subsampling is used to ensure that the density of points             
   # is fairly dense, but without replications.                                           
   StatResultId = MIL.M3dimAllocResult(MilSystem, MIL.M_STATISTICS_RESULT, MIL.M_DEFAULT)
   MIL.M3dimStat(MIL.M_STAT_CONTEXT_DISTANCE_TO_NEAREST_NEIGHBOR, MilContainerIds[0], StatResultId, MIL.M_DEFAULT)

   # Nearest neighbour distances gives a measure of the point cloud density. 
   GridSize = MIL.M3dimGetResult(StatResultId, MIL.M_MIN_DISTANCE_TO_NEAREST_NEIGHBOR)

   # Use the measured point cloud density as guide for the subsampling. 
   MilMergeSubsampleContext = MIL.M3dimAlloc(MilSystem , MIL.M_SUBSAMPLE_CONTEXT, MIL.M_DEFAULT)
   MIL.M3dimControl(MilMergeSubsampleContext, MIL.M_SUBSAMPLE_MODE, MIL.M_SUBSAMPLE_GRID)
   MIL.M3dimControl(MilMergeSubsampleContext, MIL.M_GRID_SIZE_X, GridSize)
   MIL.M3dimControl(MilMergeSubsampleContext, MIL.M_GRID_SIZE_Y, GridSize)
   MIL.M3dimControl(MilMergeSubsampleContext, MIL.M_GRID_SIZE_Z, GridSize)

   # Allocate the point cloud for the final stitched clouds. 
   MilStitchedId = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC + MIL.M_DISP, MIL.M_DEFAULT)

   print("The merging of point clouds will be shown incrementally.")
   print("Press <Enter> to show 2 merged point clouds of 6.\n")
   MIL.MosGetch()

   # Merge can merge all clouds at once, but here it is done incrementally for the display. 
   i = 2
   while i <= NUM_SCANS:
      MIL.M3dregMerge(MilResult, MilContainerIds, i, MilStitchedId, MilMergeSubsampleContext, MIL.M_DEFAULT)

      if i == 2:
         DisplayContainer(MilSystem, MilDisplay, MilStitchedId, MilDepthMap, MilDisplayImage)
      else:
         UpdateDisplay(MilSystem, MilStitchedId, MilDepthMap, MilDisplayImage) 

      if i < NUM_SCANS:
         print("Press <Enter> to show {i} merged point clouds of 6.\n".format(i=(i+1)))
      else:
         print("Press <Enter> to end.")
      i += 1

      AutoRotateDisplay(MilSystem, MilDisplay)
      MIL.MosGetch()
    
   # Free Objects. 
   for i in range(NUM_SCANS):
      MIL.MbufFree(MilContainerIds[i])

   MIL.MbufFree(MilStitchedId)
   MIL.M3dimFree(StatResultId)
   MIL.M3dimFree(MilMergeSubsampleContext)
   MIL.M3dregFree(MilContext)
   MIL.M3dregFree(MilResult)
   FreeDisplay(MilDisplay)
   if MilDisplayImage:
      MIL.MbufFree(MilDisplayImage)
   if MilDepthMap:
      MIL.MbufFree(MilDepthMap)
   MIL.MsysFree(MilSystem)
   MIL.MappFree(MilApplication)
   
   return 0

# -------------------------------------------------------------- 
# Color the container.                                           
# -------------------------------------------------------------- 
def ColorCloud(MilPointCloud, Col):
   SizeX = MIL.MbufInquireContainer(MilPointCloud, MIL.M_COMPONENT_RANGE, MIL.M_SIZE_X, MIL.M_NULL)
   SizeY = MIL.MbufInquireContainer(MilPointCloud, MIL.M_COMPONENT_RANGE, MIL.M_SIZE_Y, MIL.M_NULL)
   ReflectanceId = MIL.MbufAllocComponent(MilPointCloud, 3, SizeX, SizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE, MIL.M_COMPONENT_REFLECTANCE)
   MIL.MbufClear(ReflectanceId, Col)

# -------------------------------------------------------------- 
# Auto rotate the 3D object.                                     
# -------------------------------------------------------------- 
def AutoRotateDisplay(MilSystem, MilDisplay):
   DisplayType = MIL.MobjInquire(MilDisplay, MIL.M_OBJECT_TYPE)

   # AutoRotate available only for the 3D display. 
   if DisplayType != MIL.M_3D_DISPLAY:
      return

   # By default the display rotates around the Z axis, but the robot is oriented along the Y axis. 
   Geometry = MIL.M3dgeoAlloc(MilSystem, MIL.M_GEOMETRY, MIL.M_DEFAULT)
   MIL.M3ddispCopy(MilDisplay, Geometry, MIL.M_ROTATION_AXIS, MIL.M_DEFAULT)
   CenterX = MIL.M3dgeoInquire(Geometry, MIL.M_START_POINT_X)
   CenterY = MIL.M3dgeoInquire(Geometry, MIL.M_START_POINT_Y)
   CenterZ = MIL.M3dgeoInquire(Geometry, MIL.M_START_POINT_Z)
   MIL.M3dgeoLine(Geometry, MIL.M_POINT_AND_VECTOR, MIL.M_UNCHANGED, MIL.M_UNCHANGED, MIL.M_UNCHANGED, 0, 1, 0, MIL.M_UNCHANGED, MIL.M_DEFAULT)
   MIL.M3ddispCopy(Geometry, MilDisplay, MIL.M_ROTATION_AXIS, MIL.M_DEFAULT)
   MIL.M3ddispControl(MilDisplay, MIL.M_AUTO_ROTATE, MIL.M_ENABLE)
   MIL.M3dgeoFree(Geometry)

# -------------------------------------------------------------- 
# Allocates a 3D display and returns its MIL identifier.           
# -------------------------------------------------------------- 
def Alloc3dDisplayId(MilSystem):
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
   MilDisplay = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)

   if not MilDisplay:
      print("\nThe current system does not support the 3D display.")
      print("A 2D display will be used instead.")
      print("Press any key to continue.")
      MIL.MosGetch()

      # Allocate a 2D Display instead. 
      MilDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_WINDOWED)
   else:
      # Adjust the viewpoint of the 3D display. 
      MIL.M3ddispSetView(MilDisplay, MIL.M_AUTO, MIL.M_BOTTOM_VIEW, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
      print("Press <R> on the display window to stop/start the rotation.\n")

   return MilDisplay

# -------------------------------------------------------------- 
# Display the received container.                                
# -------------------------------------------------------------- 
def DisplayContainer(MilSystem, MilDisplay, MilContainer, MilDepthMap, MilIntensityMap):
   DisplayType = MIL.MobjInquire(MilDisplay, MIL.M_OBJECT_TYPE)

   Use3D = (DisplayType == MIL.M_3D_DISPLAY)
   if Use3D:
      MIL.M3ddispSelect(MilDisplay, MilContainer, MIL.M_ADD, MIL.M_DEFAULT)
      MIL.M3ddispSelect(MilDisplay, MIL.M_NULL, MIL.M_OPEN, MIL.M_DEFAULT)
   else:
      if MilDepthMap == MIL.M_NULL:
         SizeX = 0 # Image size X for 2d display. 
         SizeY = 0 # Image size Y for 2d display. 

         SizeX, SizeY = MIL.M3dimCalculateMapSize(MIL.M_DEFAULT, MilContainer, MIL.M_NULL, MIL.M_DEFAULT)

         MilIntensityMap = MIL.MbufAllocColor(MilSystem, 3, SizeX, SizeY, MIL.M_UNSIGNED + 8, MIL.M_IMAGE | MIL.M_PROC | MIL.M_DISP)
         MilDepthMap     = MIL.MbufAlloc2d   (MilSystem,    SizeX, SizeY, MIL.M_UNSIGNED + 8, MIL.M_IMAGE | MIL.M_PROC | MIL.M_DISP)

         MIL.M3dimCalibrateDepthMap(MilContainer, MilDepthMap, MilIntensityMap, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_CENTER)

      # Rotate the point cloud container to be in the xy plane before projecting. 
      RotatedContainer = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC, MIL.M_DEFAULT)

      MIL.M3dimRotate(MilContainer, RotatedContainer, MIL.M_ROTATION_XYZ, 90, 60, 0, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)     
      MIL.M3dimProject(MilContainer, MilDepthMap, MilIntensityMap, MIL.M_DEFAULT, MIL.M_MIN_Z, MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Display the projected point cloud container. 
      MIL.MdispSelect(MilDisplay, MilIntensityMap)
      MIL.MbufFree(RotatedContainer)

   return MilDepthMap, MilIntensityMap

# -------------------------------------------------------------- 
# Updated the displayed image.                                   
# -------------------------------------------------------------- 
def UpdateDisplay(MilSystem, MilContainer, MilDepthMap, MilIntensityMap):
   if not MilDepthMap:
      return 

   RotatedContainer = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC, MIL.M_DEFAULT)

   MIL.M3dimRotate(MilContainer, RotatedContainer, MIL.M_ROTATION_XYZ, 90, 70, 0, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MIL.M3dimProject(MilContainer, MilDepthMap, MilIntensityMap, MIL.M_DEFAULT, MIL.M_MIN_Z, MIL.M_DEFAULT, MIL.M_DEFAULT)

   MIL.MbufFree(RotatedContainer)

# -------------------------------------------------------------- 
# Free the display.                                              
# -------------------------------------------------------------- 
def FreeDisplay(MilDisplay):
   DisplayType = MIL.MobjInquire(MilDisplay, MIL.M_OBJECT_TYPE)

   if DisplayType == MIL.M_DISPLAY:
      MIL.MdispFree(MilDisplay) 
   else:
      MIL.M3ddispFree(MilDisplay) 

if __name__ == "__main__":
   M3dregExample()
