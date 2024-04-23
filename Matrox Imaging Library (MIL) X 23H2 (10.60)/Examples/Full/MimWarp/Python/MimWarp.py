#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MimWarp.py 
#
# Synopsis:  : This program performs three types of warp transformations. 
#              First the image is stretched according to four specified 
#              reference points. Then it is warped on a sinusoid, and 
#              finally, the program loops while warping the image on a 
#              sphere.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################
 
import mil as MIL
import math
import sys

# Target image specifications. 
IMAGE_FILE             =  MIL.M_IMAGE_PATH + "BaboonMono.mim"
INTERPOLATION_MODE     =  MIL.M_NEAREST_NEIGHBOR
FIXED_POINT_PRECISION = lambda   : MIL.M_FIXED_POINT + 0 if INTERPOLATION_MODE == MIL.M_NEAREST_NEIGHBOR else MIL.M_FIXED_POINT + 6
FLOAT_TO_FIXED_POINT  = lambda x : 1 * x if INTERPOLATION_MODE == MIL.M_NEAREST_NEIGHBOR else 64 * x
ROTATION_STEP         = 1


def MimWarpExample():
   FourCornerMatrix = [
            0.0,             # X coordinate of quadrilateral's 1st corner 
            0.0,             # Y coordinate of quadrilateral's 1st corner 
            456.0,           # X coordinate of quadrilateral's 2nd corner 
            62.0,            # Y coordinate of quadrilateral's 2nd corner 
            333.0,           # X coordinate of quadrilateral's 3rd corner 
            333.0,           # Y coordinate of quadrilateral's 3rd corner 
            100.0,           # X coordinate of quadrilateral's 4th corner 
            500.0,           # Y coordinate of quadrilateral's 4th corner 
            0.0,           # X coordinate of rectangle's top-left corner 
            0.0,           # Y coordinate of rectangle's top-left corner 
            511.0,           # X coordinate of rectangle's bottom-right corner 
            511.0 ]          # Y coordinate of rectangle's bottom-right corner 
   Precision     = FIXED_POINT_PRECISION()
   Interpolation = INTERPOLATION_MODE
   OffsetX       = 0
   ImageWidth = 0
   ImageHeight = 0
   ImageType = MIL.M_NULL
   i = 0
   j = 0
   FramesPerSecond = 0
   Time = 0
   NbLoop = 0

   # Allocate defaults. 
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Restore the source image. 
   MilSourceImage = MIL.MbufRestore(IMAGE_FILE, MilSystem)

   # Allocate a display buffers and show the source image. 
   ImageWidth = MIL.MbufInquire(MilSourceImage, MIL.M_SIZE_X)
   ImageHeight = MIL.MbufInquire(MilSourceImage, MIL.M_SIZE_Y)
   ImageType = MIL.MbufInquire(MilSourceImage, MIL.M_TYPE)

   MilDisplayImage = MIL.MbufAlloc2d(MilSystem, ImageWidth, ImageHeight, ImageType, MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP)
   MIL.MbufCopy(MilSourceImage, MilDisplayImage)
   MIL.MdispSelect(MilDisplay, MilDisplayImage)

   # Print a message. 
   print("\nWARPING:")
   print("--------\n")
   print("This image will be warped using different methods.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Four-corner LUT warping 
   #-------------------------
   
   # Allocate 2 LUT buffers. 
   MilLutX = MIL.MbufAlloc2d(MilSystem, ImageWidth, ImageHeight, 16 + MIL.M_SIGNED, MIL.M_LUT)
   MilLutY = MIL.MbufAlloc2d(MilSystem, ImageWidth, ImageHeight, 16 + MIL.M_SIGNED, MIL.M_LUT)

   # Allocate the coefficient buffer. 
   Mil4CornerArray = MIL.MbufAlloc2d(MilSystem, 12, 1, 32 + MIL.M_FLOAT, MIL.M_ARRAY)

   # Put warp values into the coefficient buffer. 
   MIL.MbufPut1d(Mil4CornerArray, 0, 12, FourCornerMatrix)

   # Generate LUT buffers. 
   MIL.MgenWarpParameter(Mil4CornerArray, MilLutX, MilLutY, MIL.M_WARP_4_CORNER + Precision, MIL.M_DEFAULT, 0.0, 0.0)

   # Clear the destination. 
   MIL.MbufClear(MilDisplayImage, 0)

   # Warp the image. 
   MIL.MimWarp(MilSourceImage, MilDisplayImage, MilLutX, MilLutY, MIL.M_WARP_LUT + Precision,
           Interpolation)

   # Print a message. 
   print("The image was warped from an arbitrary quadrilateral to a square.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Sinusoidal LUT warping 
   #------------------------

   # Allocate user-defined LUTs. 
   MilLutXPtr = [None] *ImageHeight*ImageWidth
   MilLutYPtr = [None] *ImageHeight*ImageWidth
  
   # Fill the LUT with a sinusoidal waveforms with a 6-bit precision.
   for j in range(ImageHeight):
      for i in range(ImageWidth):
         MilLutYPtr[i + (j * ImageWidth)] = FLOAT_TO_FIXED_POINT(j + int(20 * math.sin(0.03 * i)))
         MilLutXPtr[i + (j * ImageWidth)] = FLOAT_TO_FIXED_POINT(i + int(20 * math.sin(0.03 * j)))
   
   # Put the values into the LUT buffers.
   MIL.MbufPut2d(MilLutX, 0, 0, ImageWidth, ImageHeight, MilLutXPtr)
   MIL.MbufPut2d(MilLutY, 0, 0, ImageWidth, ImageHeight, MilLutYPtr)

   # Clear the destination. 
   MIL.MbufClear(MilDisplayImage,0)

   # Warp the image. 
   MIL.MimWarp(MilSourceImage,MilDisplayImage,MilLutX,MilLutY,MIL.M_WARP_LUT + Precision,
           Interpolation)
   
   # wait for a key 
   print("The image was warped on two sinusoidal waveforms.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Continuous spherical LUT warping 
   #--------------------------------

   # Allocate temporary buffer. 
   MIL.MbufFree(MilSourceImage)
   MilSourceImage = MIL.MbufAlloc2d(MilSystem, ImageWidth*2, ImageHeight, ImageType, 
                                        MIL.M_IMAGE + MIL.M_PROC)
                            
   # Reload the image. 
   MIL.MbufLoad(IMAGE_FILE, MilSourceImage)
   
   # Fill the LUTs with a sphere pattern with a 6-bit precision.
   GenerateSphericLUT(ImageWidth, ImageHeight, MilLutXPtr,MilLutYPtr)
   MIL.MbufPut2d(MilLutX, 0, 0, ImageWidth, ImageHeight, MilLutXPtr)
   MIL.MbufPut2d(MilLutY, 0, 0, ImageWidth, ImageHeight, MilLutYPtr)

   # Duplicate the buffer to allow wrap around in the warping. 
   MIL.MbufCopy(MilSourceImage, MilDisplayImage)
   ChildWindow = MIL.MbufChild2d(MilSourceImage, ImageWidth, 0, ImageWidth, ImageHeight)
   MIL.MbufCopy(MilDisplayImage, ChildWindow)
   MIL.MbufFree(ChildWindow)
  
   # Clear the destination. 
   MIL.MbufClear(MilDisplayImage,0)

   # Print a message and start the timer. 
   print("The image is continuously warped on a sphere.")
   print("Press <Enter> to stop.\n")
   MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL)

   # Create a child in the buffer containing the two images. 
   ChildWindow = MIL.MbufChild2d(MilSourceImage, OffsetX, 0, ImageWidth, ImageHeight)

   # Warp the image continuously. 
   while ((not MIL.MosKbhit()) or (OffsetX != (ImageWidth/4))):
      # Move the child to the new position 
      MIL.MbufChildMove(ChildWindow, OffsetX, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
      
      # Warp the child in the window. 
      MIL.MimWarp(ChildWindow, MilDisplayImage, MilLutX, MilLutY, 
                           MIL.M_WARP_LUT+Precision, Interpolation)

      # Update the offset (shift the window to the right). 
      OffsetX += ROTATION_STEP
      
      # Reset the offset if the child is outside the buffer. 
      if OffsetX > ImageWidth - 1:
         OffsetX = 0

      NbLoop += 1

      # Calculate and print the number of frames per second processed. 
      Time = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)
      FramesPerSecond = NbLoop/Time
      print("Processing speed: {FramesPerSecond:.0f} Images/Sec.".format(FramesPerSecond=FramesPerSecond), end="\r")
   
   MIL.MosGetch()
   print("\nPress <Enter> to end.")
   MIL.MosGetch()

   # Free objects. 

   MIL.MbufFree(ChildWindow)
   MIL.MbufFree(MilLutX)
   MIL.MbufFree(MilLutY)
   MIL.MbufFree(Mil4CornerArray)
   MIL.MbufFree(MilSourceImage)
   MIL.MbufFree(MilDisplayImage)
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0

# Generate two custom LUTs used to map the image on a sphere. 
# ----------------------------------------------------------- 
def GenerateSphericLUT(ImageWidth, ImageHeight, MilLutXPtr, MilLutYPtr):     
   # Set the radius of the sphere 
   Radius = 200.0         

   # Generate the X and Y buffers 
   for j in range(ImageHeight):
      k = j * ImageWidth

      vtmp = (j - (ImageHeight/2)) / Radius

      # Check that still in the sphere (in the Y axis). 
      if abs(vtmp) < 1.0:
         # We scan from top to bottom, so reverse the value obtained above
         # and obtain the angle. 
         vtmp = math.acos(-vtmp)
         if vtmp == 0.0:
            vtmp=0.0000001
      
         # Compute the position to fetch in the source. 
         v = ((vtmp/3.1415926) * (ImageHeight - 1) + 0.5)

         # Compute the Y coordinate of the sphere. 
         tmp = Radius * math.sin(vtmp)

         for i in range(ImageWidth):
            # Check that still in the sphere. 
            utmp = float((i - (ImageWidth/2)) / tmp)
            if abs(utmp) < 1.0:
               utmp = math.acos(-utmp)
            
               # Compute the position to fetch (fold the image in four).  
               MilLutXPtr[i + k] = int(FLOAT_TO_FIXED_POINT(((utmp/3.1415926) * float((ImageWidth/2) - 1) + 0.5)))
               MilLutYPtr[i + k] = int(FLOAT_TO_FIXED_POINT(v))
            else:
               # Default position (fetch outside the buffer to 
               # activate the clear overscan). 
               MilLutXPtr[i + k] = int(FLOAT_TO_FIXED_POINT(ImageWidth))
               MilLutYPtr[i + k] = int(FLOAT_TO_FIXED_POINT(ImageHeight)) 
      else:
         for  i in range(ImageWidth):
            # Default position (fetch outside the buffer for clear overscan). 
            MilLutXPtr[i + k] = int(FLOAT_TO_FIXED_POINT(ImageWidth))
            MilLutYPtr[i + k] = int(FLOAT_TO_FIXED_POINT(ImageHeight))
           
if __name__ == "__main__":
   MimWarpExample()