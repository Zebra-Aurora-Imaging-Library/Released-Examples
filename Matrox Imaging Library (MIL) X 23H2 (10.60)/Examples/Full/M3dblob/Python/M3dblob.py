#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MIL.M3dblob.cpp
#
# Synopsis:  This program demonstrates how to use the 3d blob analysis module to
#            identify objects in a scene and separate them into categories.
#            See the PrintHeader() function below for a detailed description.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################

 
import mil as MIL

# Source file specification.
CONNECTORS_AND_WASHERS_FILE = MIL.M_IMAGE_PATH + "ConnectorsAndWashers.mbufc"
CONNECTORS_AND_WASHERS_ILLUSTRATION_FILE = MIL.M_IMAGE_PATH + "ConnectorsAndWashers.png"

TWISTY_PUZZLES_FILE = MIL.M_IMAGE_PATH + "TwistyPuzzles.mbufc"

# Segmentation thresholds.
LOCAL_SEGMENTATION_MIN_NB_POINTS = 100
LOCAL_SEGMENTATION_MAX_NB_POINTS = 10000
LOCAL_SEGMENTATION_DISTANCE_THRESHOLD = 0.75 # in mm

PLANAR_SEGMENTATION_MIN_NB_POINTS = 5000
PLANAR_SEGMENTATION_NORMAL_THRESHOLD = 20    # in deg

# --------------------------------------------------------------
# First example.
# --------------------------------------------------------------
def IdentificationAndSortingExample(SceneDisplay, IllustrationDisplay):

   MilSystem = MIL.MobjInquire(SceneDisplay, MIL.M_OWNER_SYSTEM)
   SceneGraphicList = MIL.M3ddispInquire(SceneDisplay, MIL.M_3D_GRAPHIC_LIST_ID)

   # Restore the point cloud and display it.
   MilPointCloud = MIL.MbufImport(CONNECTORS_AND_WASHERS_FILE, MIL.M_DEFAULT, MIL.M_RESTORE, MilSystem)

   MIL.M3dgraRemove(SceneGraphicList, MIL.M_ALL, MIL.M_DEFAULT)
   MIL.M3dgraControl(SceneGraphicList, MIL.M_DEFAULT_SETTINGS, MIL.M_THICKNESS, 3)

   MIL.M3ddispSelect(SceneDisplay, MilPointCloud, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MIL.M3ddispSetView(SceneDisplay, MIL.M_AUTO, MIL.M_TOP_TILTED, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Show an illustration of the objects in the scene.
   IllustrationImage = MIL.MbufRestore(CONNECTORS_AND_WASHERS_ILLUSTRATION_FILE, MilSystem)
   MIL.MdispSelect(IllustrationDisplay, IllustrationImage)

   print("A 3D point cloud consisting of wire connectors and washers")
   print("is restored from a file and displayed.")
   print()
   print("Press <Enter> to segment it into separate objects.")
   print()
   MIL.MosGetch()

   # Allocate the segmentation contexts.
   SegmentationContext = MIL.M3dblobAlloc(MilSystem, MIL.M_SEGMENTATION_CONTEXT, MIL.M_DEFAULT)
   CalculateContext = MIL.M3dblobAlloc(MilSystem, MIL.M_CALCULATE_CONTEXT, MIL.M_DEFAULT)
   Draw3dContext = MIL.M3dblobAlloc(MilSystem, MIL.M_DRAW_3D_CONTEXT, MIL.M_DEFAULT)

   # Allocate the segmentation results. One result is used for each category.
   AllBlobs = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT)
   Connectors = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT)
   Washers = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT)
   UnknownBlobs = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT)

   # Segment the point cloud into several blobs.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NEIGHBOR_SEARCH_MODE, MIL.M_ORGANIZED)                  # Take advantage of the 2d organization.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NEIGHBORHOOD_ORGANIZED_SIZE, 5)                         # Look for neighbors in a 5x5 square kernel.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NUMBER_OF_POINTS_MIN, LOCAL_SEGMENTATION_MIN_NB_POINTS) # Exclude small isolated clusters.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NUMBER_OF_POINTS_MAX, LOCAL_SEGMENTATION_MAX_NB_POINTS) # Exclude extremely large clusters which make up the background.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_MAX_DISTANCE, LOCAL_SEGMENTATION_DISTANCE_THRESHOLD)    # Set the distance between points to be blobbed together.

   MIL.M3dblobSegment(SegmentationContext, MilPointCloud, AllBlobs, MIL.M_DEFAULT)

   # Draw all blobs in the 3d display.
   MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_ACTIVE, MIL.M_ENABLE)
   MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_THICKNESS, 3)
   AllBlobsLabel = MIL.M3dblobDraw3d(Draw3dContext, MilPointCloud, AllBlobs, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT)

   print("The point cloud is segmented based on the distance between points.")
   print("Points belonging to the background plane or small isolated clusters")
   print("are excluded.")
   print()
   print("Press <Enter> to continue.")
   print()
   MIL.MosGetch()

   # Calculate features on the blobs and use them to determine the type of object they represent.
   MIL.M3dblobControl(CalculateContext, MIL.M_DEFAULT, MIL.M_PCA_BOX, MIL.M_ENABLE)
   MIL.M3dblobControl(CalculateContext, MIL.M_DEFAULT, MIL.M_LINEARITY, MIL.M_ENABLE)
   MIL.M3dblobControl(CalculateContext, MIL.M_DEFAULT, MIL.M_PLANARITY, MIL.M_ENABLE)

   MIL.M3dblobCalculate(CalculateContext, MilPointCloud, AllBlobs, MIL.M_ALL, MIL.M_DEFAULT)

   # Connectors are more elongated than other blobs.
   # Use the feature M_LINEARITY, which is a value from 0 (perfect sphere/plane) to 1 (perfect line).
   MIL.M3dblobSelect(AllBlobs, Connectors, MIL.M_LINEARITY, MIL.M_GREATER, 0.5, MIL.M_NULL, MIL.M_DEFAULT)

   # Washers are flat and circular.
   # Use the feature M_PLANARITY, which is a value from 0 (perfect sphere) to 1 (perfect plane).
   MIL.M3dblobSelect(AllBlobs, Washers, MIL.M_LINEARITY, MIL.M_LESS, 0.2, MIL.M_NULL, MIL.M_DEFAULT)
   MIL.M3dblobSelect(Washers, Washers, MIL.M_PLANARITY, MIL.M_GREATER, 0.8, MIL.M_NULL, MIL.M_DEFAULT)

   # Blobs that are neither connectors nor washers are unknown objects.
   # Use M3dblobCombine to subtract already identified blobs from AllBlobs.
   MIL.M3dblobCombine(AllBlobs, Connectors, UnknownBlobs, MIL.M_SUB, MIL.M_DEFAULT)
   MIL.M3dblobCombine(UnknownBlobs, Washers, UnknownBlobs, MIL.M_SUB, MIL.M_DEFAULT)

   # Print the number of blobs in each category.
   NbConnectors = MIL.M3dblobGetResult(Connectors,   MIL.M_DEFAULT, MIL.M_NUMBER)
   NbWashers    = MIL.M3dblobGetResult(Washers,      MIL.M_DEFAULT, MIL.M_NUMBER)
   NbUnknown    = MIL.M3dblobGetResult(UnknownBlobs, MIL.M_DEFAULT, MIL.M_NUMBER)

   print("Simple 3D features (planarity, linearity) are calculated on the")
   print("blobs and used to identify them.")
   print()
   print("The relevant objects (connectors and washers) have their")
   print("bounding box displayed.")
   print("Connectors (in red):     " + str(NbConnectors))
   print("Washers (in green) :     " + str(NbWashers))
   print("Unknown (in yellow):     " + str(NbUnknown))
   print()

   # Draw the blobs in the 3d display.
   MIL.M3dgraRemove(SceneGraphicList, AllBlobsLabel, MIL.M_DEFAULT)

   MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_COLOR, MIL.M_COLOR_YELLOW)
   MIL.M3dblobDraw3d(Draw3dContext, MilPointCloud, UnknownBlobs, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT)

   MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_PCA_BOX, MIL.M_ACTIVE, MIL.M_ENABLE)
   MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_COLOR, MIL.M_COLOR_RED)
   MIL.M3dblobDraw3d(Draw3dContext, MilPointCloud, Connectors, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT)

   MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_COLOR, MIL.M_COLOR_GREEN)
   MIL.M3dblobDraw3d(Draw3dContext, MilPointCloud, Washers, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT)

   print("Press <Enter> to continue.")
   print()
   MIL.MosGetch()

   MIL.M3dblobFree(UnknownBlobs)
   MIL.M3dblobFree(Washers)
   MIL.M3dblobFree(Connectors)
   MIL.M3dblobFree(AllBlobs)
   MIL.M3dblobFree(Draw3dContext)
   MIL.M3dblobFree(CalculateContext)
   MIL.M3dblobFree(SegmentationContext)

   MIL.MbufFree(IllustrationImage)
   MIL.MbufFree(MilPointCloud)


# --------------------------------------------------------------
# Second example.
# --------------------------------------------------------------
def PlanarSegmentationExample(SceneDisplay, IllustrationDisplay):

   MilSystem = MIL.MobjInquire(SceneDisplay, MIL.M_OWNER_SYSTEM)
   SceneGraphicList = MIL.M3ddispInquire(SceneDisplay, MIL.M_3D_GRAPHIC_LIST_ID)

   # Restore the point cloud and display it.
   MilPointCloud = MIL.MbufImport(TWISTY_PUZZLES_FILE, MIL.M_DEFAULT, MIL.M_RESTORE, MilSystem)

   MIL.M3dgraRemove(SceneGraphicList, MIL.M_ALL, MIL.M_DEFAULT)
   MIL.M3dgraControl(SceneGraphicList, MIL.M_DEFAULT_SETTINGS, MIL.M_THICKNESS, 1)

   MIL.M3ddispSelect(SceneDisplay, MilPointCloud, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MIL.M3ddispSetView(SceneDisplay, MIL.M_AUTO, MIL.M_TOP_TILTED, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   print("Another point cloud containing various twisty puzzles is restored.")
   print()
   print("Press <Enter> to segment it into separate objects.")
   print()
   MIL.MosGetch()

   # Allocate the segmentation objects.
   SegmentationContext = MIL.M3dblobAlloc(MilSystem, MIL.M_SEGMENTATION_CONTEXT, MIL.M_DEFAULT)
   SegmentationResult = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT)

   # First segment the point cloud with only local thresholds.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NEIGHBOR_SEARCH_MODE, MIL.M_ORGANIZED)                    # Take advantage of the 2d organization.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NEIGHBORHOOD_ORGANIZED_SIZE, 5)                           # Look for neighbors in a 5x5 square kernel.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NUMBER_OF_POINTS_MIN, PLANAR_SEGMENTATION_MIN_NB_POINTS)  # Exclude small isolated clusters.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_MAX_DISTANCE_MODE, MIL.M_AUTO)                            # Use an automatic local distance threshold.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NORMAL_DISTANCE_MAX_MODE, MIL.M_AUTO)                     # Use an automatic local normal threshold.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NORMAL_DISTANCE_MODE, MIL.M_ORIENTATION)                  # Consider flipped normals to be the same.

   MIL.M3dblobSegment(SegmentationContext, MilPointCloud, SegmentationResult, MIL.M_DEFAULT)

   AnnotationLabel = MIL.M3dblobDraw3d(MIL.M_DEFAULT, MilPointCloud, SegmentationResult, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT)

   print("The point cloud is segmented based on local thresholds (distance, normals).")
   print()
   print("Local thresholds can separate distinct objects due to camera occlusions,")
   print("but are often not enough to segment a single object into subparts.")
   print()
   print("Press <Enter> to use global thresholds instead.")
   print()
   MIL.MosGetch()

   # Then segment again with global thresholds.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NORMAL_DISTANCE_MAX_MODE, MIL.M_USER_DEFINED);                      # Remove the local normal threshold.
   MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_GLOBAL_NORMAL_DISTANCE_MAX, PLANAR_SEGMENTATION_NORMAL_THRESHOLD);  # Use a global normal threshold instead.

   MIL.M3dblobSegment(SegmentationContext, MilPointCloud, SegmentationResult, MIL.M_DEFAULT)

   MIL.M3dgraRemove(SceneGraphicList, AnnotationLabel, MIL.M_DEFAULT)
   AnnotationLabel = MIL.M3dblobDraw3d(MIL.M_DEFAULT, MilPointCloud, SegmentationResult, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT)

   print("The puzzles' sides are now separated.")
   print()
   print("Press <Enter> to end.")
   print()
   MIL.MosGetch()

   MIL.M3dblobFree(SegmentationContext)
   MIL.M3dblobFree(SegmentationResult)

   MIL.MbufFree(MilPointCloud)


# --------------------------------------------------------------
# Allocates a 3D display if it is supported.
# --------------------------------------------------------------
def Alloc3dDisplayId(MilSystem):
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
   MilDisplay = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)

   if MilDisplay == MIL.M_NULL:
      print("The current system does not support the 3D display.")
      print("Press any key to exit.")
      MIL.MosGetch()

   return MilDisplay

# --------------------------------------------------------------
# Main.                                          
# --------------------------------------------------------------
def M3dblobExample():
   print("[EXAMPLE NAME]")
   print("M3dblob")
   print()
   print("[SYNOPSIS]")
   print("This program demonstrates how to use the 3d blob analysis module to")
   print("identify objects in a scene and separate them into categories.")
   print()
   print("[MODULES USED]")
   print("Modules used: 3D Blob Analysis, 3D Image Processing,")
   print("3D Display, Display, Buffer, and 3D Graphics.")
   print()

   # Allocate defaults. 
   MilApplication, MilSystem = MIL.MappAllocDefault(MIL.M_DEFAULT, DispIdPtr=MIL.M_NULL, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Allocate the displays.
   SceneDisplay = Alloc3dDisplayId(MilSystem)
   if SceneDisplay == MIL.M_NULL:
      MIL.MappFreeDefault(MilApplication, MilSystem, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL)
      return -1
   MIL.M3ddispControl(SceneDisplay, MIL.M_TITLE, "Scene")

   IllustrationDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_WINDOWED)
   IllustrationOffsetX = MIL.M3ddispInquire(SceneDisplay, MIL.M_SIZE_X)
   MIL.MdispControl(IllustrationDisplay, MIL.M_TITLE, "Objects to inspect")
   MIL.MdispControl(IllustrationDisplay, MIL.M_WINDOW_INITIAL_POSITION_X, IllustrationOffsetX)
   
   # Run the examples.
   IdentificationAndSortingExample(SceneDisplay, IllustrationDisplay)
   PlanarSegmentationExample(SceneDisplay, IllustrationDisplay)

   MIL.MdispFree(IllustrationDisplay)
   MIL.M3ddispFree(SceneDisplay)

   MIL.MappFreeDefault(MilApplication, MilSystem, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL)

   return 0

if __name__ == "__main__":
   M3dblobExample()
