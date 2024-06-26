#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#########################################################################################
#
#
# File name: MBufColor.py
#
# Synopsis:  This program demonstrates color buffer manipulations. It allocates 
#            a displayable color image buffer, displays it, and loads a color   
#            image into the left part. After that, color annotations are done 
#            in each band of the color buffer. Finally, to increase the image
#            luminance, the image is converted to Hue, Saturation and Luminance
#            (HSL), a certain offset is added to the luminance component and 
#            the image is converted back to Red, Green, Blue (RGB) into the 
#            right part to display the result. 
#
#            The example also demonstrates how to display multiple images 
#            in a single display using child buffers.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
#########################################################################################

import mil as MIL

# Source MIL image file specifications. 
IMAGE_FILE             = MIL.M_IMAGE_PATH + "Bird.mim"

# Luminance offset to add to the image. 
IMAGE_LUMINANCE_OFFSET = 40

# Main function.
def MbufColorExample():

   # Allocate defaults.
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Allocate a color display buffer twice the size of the source image and display it. 
   SizeX = MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_X)
   SizeY = MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_Y) 
   MilImage = MIL.MbufAllocColor(MilSystem,
                  MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_BAND),
                  SizeX * 2,
                  SizeY,
                  MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_TYPE),
                  MIL.M_IMAGE + MIL.M_DISP + MIL.M_PROC)
   MIL.MbufClear(MilImage, 0)
   MIL.MdispSelect(MilDisplay, MilImage)

   # Define 2 child buffers that maps to the left and right part of the display 
   # buffer, to put the source and destination color images.
   MilLeftSubImage = MIL.MbufChild2d(MilImage, 0, 0, SizeX, SizeY)
   MilRightSubImage = MIL.MbufChild2d(MilImage, SizeX, 0, SizeX, SizeY)

   # Load the color source image on the left.
   MIL.MbufLoad(IMAGE_FILE, MilLeftSubImage)

   # Define child buffers that map to the red, green and blue components
   # of the source image.
   MilRedBandSubImage = MIL.MbufChildColor(MilLeftSubImage, MIL.M_RED)
   MilGreenBandSubImage = MIL.MbufChildColor(MilLeftSubImage, MIL.M_GREEN)
   MilBlueBandSubImage = MIL.MbufChildColor(MilLeftSubImage, MIL.M_BLUE)

   # Write color text annotations to show access in each individual band of the image.
   
   # Note that this is typically done more simply by using:
   # MIL.MgraColor(MIL.M_DEFAULT, MIL.M_RGB888(0xFF,0x90,0x00))
   # MIL.MgraText(MIL.M_DEFAULT, MilLeftSubImage, ...)
   MIL.MgraColor(MIL.M_DEFAULT, 0xFF)
   MIL.MgraText(MIL.M_DEFAULT, MilRedBandSubImage,   SizeX/16, SizeY/8, " TOUCAN ")
   MIL.MgraColor(MIL.M_DEFAULT, 0x90)
   MIL.MgraText(MIL.M_DEFAULT, MilGreenBandSubImage, SizeX/16, SizeY/8, " TOUCAN ")
   MIL.MgraColor(MIL.M_DEFAULT, 0x00)
   MIL.MgraText(MIL.M_DEFAULT, MilBlueBandSubImage,  SizeX/16, SizeY/8, " TOUCAN ")
   
   # Print a message. 
   print("\nCOLOR OPERATIONS:")
   print("-----------------\n")
   print("A color source image was loaded on the left and color text")
   print("annotations were written in it.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Convert image to Hue, Saturation, Luminance color space (HSL). 
   MIL.MimConvert(MilLeftSubImage, MilRightSubImage, MIL.M_RGB_TO_HSL)
  
   # Create a child buffer that maps to the luminance component. 
   MilLumSubImage = MIL.MbufChildColor(MilRightSubImage, MIL.M_LUMINANCE)
     
   # Add an offset to the luminance component. 
   MIL.MimArith(MilLumSubImage, IMAGE_LUMINANCE_OFFSET, MilLumSubImage, 
                                                      MIL.M_ADD_CONST + MIL.M_SATURATION)
  
   # Convert image back to Red, Green, Blue color space (RGB) for display. 
   MIL.MimConvert(MilRightSubImage, MilRightSubImage, MIL.M_HSL_TO_RGB) 

   # Print a message. 
   print("Luminance was increased using color image processing.")
  
   # Print a message. 
   print("Press <Enter> to end.")
   MIL.MosGetch()

   # Release sub-images and color image buffer. 
   MIL.MbufFree(MilLumSubImage)
   MIL.MbufFree(MilRedBandSubImage)
   MIL.MbufFree(MilGreenBandSubImage)
   MIL.MbufFree(MilBlueBandSubImage)
   MIL.MbufFree(MilRightSubImage)
   MIL.MbufFree(MilLeftSubImage)
   MIL.MbufFree(MilImage)

   # Release defaults. 
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0


if __name__ == "__main__":
   MbufColorExample()