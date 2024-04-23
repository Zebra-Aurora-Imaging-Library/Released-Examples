#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#*****************************************************************************
#
# File name: MIL.Mblob.py
#
# Synopsis:  This program loads an image of some nuts, bolts and washers, 
#             determines the number of each of these, finds and marks
#             their center of gravity using the Blob analysis module.
# 
#  Copyright © Matrox Electronic Systems Ltd., 1992-2023.
#  All Rights Reserved
#

import mil as MIL
 
# Target MIL image file specifications.
IMAGE_FILE            = MIL.M_IMAGE_PATH + "BoltsNutsWashers.mim"
IMAGE_THRESHOLD_VALUE = 26 

# Minimum and maximum area of blobs.
MIN_BLOB_AREA         = 50 
MAX_BLOB_AREA         = 50000

# Radius of the smallest particles to keep. 
MIN_BLOB_RADIUS       = 3

# Minimum hole compactness corresponding to a washer. 
MIN_COMPACTNESS       = 1.5


def MblobExample():

   # Allocate defaults. 
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Restore source image into image buffer.  
   MilImage = MIL.MbufRestore(IMAGE_FILE, MilSystem)

   # Allocate a graphic list to hold the subpixel annotations to draw. 
   MilGraphicList = MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT)

   # Associate the graphic list to the display. 
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList)

   # Display the buffer. 
   MIL.MdispSelect(MilDisplay, MilImage)

   # Allocate a binary image buffer for fast processing. 
   SizeX = MIL.MbufInquire(MilImage, MIL.M_SIZE_X)
   SizeY = MIL.MbufInquire(MilImage, MIL.M_SIZE_Y)
   MilBinImage = MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 1+MIL.M_UNSIGNED, MIL.M_IMAGE+MIL.M_PROC)

   # Pause to show the original image.  
   print("BLOB ANALYSIS:")
   print("--------------\n")
   print("This program determines the number of bolts, nuts and washers")
   print("in the image and finds their center of gravity.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()
 
   # Binarize image. 
   MIL.MimBinarize(MilImage, MilBinImage, MIL.M_FIXED+MIL.M_GREATER_OR_EQUAL, IMAGE_THRESHOLD_VALUE, MIL.M_NULL)

   # Remove small particles and then remove small holes. 
   MIL.MimOpen(MilBinImage, MilBinImage, MIN_BLOB_RADIUS, MIL.M_BINARY)
   MIL.MimClose(MilBinImage, MilBinImage, MIN_BLOB_RADIUS, MIL.M_BINARY)
 
   # Allocate a context.  
   MilBlobContext = MIL.MblobAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT)
  
   # Enable the Center Of Gravity feature calculation.  
   MIL.MblobControl(MilBlobContext, MIL.M_CENTER_OF_GRAVITY + MIL.M_BINARY, MIL.M_ENABLE)
 
   # Allocate a blob result buffer. 
   MilBlobResult = MIL.MblobAllocResult(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT) 
 
   # Calculate selected features for each blob.  
   MIL.MblobCalculate(MilBlobContext, MilBinImage, MIL.M_NULL, MilBlobResult)
 
   # Exclude blobs whose area is too small.  
   MIL.MblobSelect(MilBlobResult, MIL.M_EXCLUDE, MIL.M_AREA, MIL.M_LESS_OR_EQUAL, MIN_BLOB_AREA, MIL.M_NULL) 
 
   # Get the total number of selected blobs.  
   TotalBlobs = MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT)
   print("There are {} objects and their centers of gravity are:".format(TotalBlobs)) 
  
   # Get the resuls. 
   CogX = MIL.MblobGetResult(MilBlobResult, MIL.M_INCLUDED_BLOBS, MIL.M_CENTER_OF_GRAVITY_X + MIL.M_BINARY)
   CogY = MIL.MblobGetResult(MilBlobResult, MIL.M_INCLUDED_BLOBS, MIL.M_CENTER_OF_GRAVITY_Y + MIL.M_BINARY)
          
   for n in range (0, TotalBlobs):
      print("Blob #{}: X={:5.1f}, Y={:5.1f}".format(int(n), CogX[n], CogY[n]))
  
   # Draw a cross at the center of gravity of each blob.   
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED)
   MIL.MblobDraw(MIL.M_DEFAULT, MilBlobResult, MilGraphicList, MIL.M_DRAW_CENTER_OF_GRAVITY, MIL.M_INCLUDED_BLOBS, MIL.M_DEFAULT)

   # Reverse what is considered to be the background so that holes are seen as being blobs. 
    
   MIL.MblobControl(MilBlobContext, MIL.M_FOREGROUND_VALUE, MIL.M_ZERO)

   # Add a feature to distinguish between types of holes. Since area
   # has already been added to the context, and the processing 
   # mode has been changed, all blobs will be re-included and the area 
   # of holes will be calculated automatically.
    
   MIL.MblobControl(MilBlobContext, MIL.M_COMPACTNESS, MIL.M_ENABLE)

   # Calculate selected features for each blob. 
   MIL.MblobCalculate(MilBlobContext, MilBinImage, MIL.M_NULL, MilBlobResult)

   # Exclude small holes and large (the area around objects) holes. 
   MIL.MblobSelect(MilBlobResult, MIL.M_EXCLUDE, MIL.M_AREA, MIL.M_OUT_RANGE, MIN_BLOB_AREA, MAX_BLOB_AREA)

   # Get the number of blobs with holes. 
   BlobsWithHoles = MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT)

   # Exclude blobs whose holes are compact (i.e. nuts). 
   MIL.MblobSelect(MilBlobResult, MIL.M_EXCLUDE, MIL.M_COMPACTNESS, MIL.M_LESS_OR_EQUAL, MIN_COMPACTNESS, MIL.M_NULL)

   # Get the number of blobs with holes that are NOT compact. 
   BlobsWithRoughHoles = MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT)

   # Print results. 
   print("\nIdentified objects:")
   print("{} bolts".format(int(TotalBlobs-BlobsWithHoles)))
   print("{} nuts".format(int(BlobsWithHoles - BlobsWithRoughHoles)))
   print("{} washers\n".format(int(BlobsWithRoughHoles)))
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free all allocations. 
   MIL.MgraFree(MilGraphicList)
   MIL.MblobFree(MilBlobResult) 
   MIL.MblobFree(MilBlobContext)
   MIL.MbufFree(MilBinImage)
   MIL.MbufFree(MilImage)
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0

if __name__ == "__main__":
   MblobExample()
