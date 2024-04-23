#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# 
# File name: MdispWindowLeveling.cpp
#
# Synopsis:  This MIL program shows how to display a 10-bit monochrome Medical image
#            and applies a LUT to do interactive Window Leveling.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################

import mil as MIL

# Image file to load.
IMAGE_NAME = "ArmsMono10bit.mim"
IMAGE_FILE = MIL.M_IMAGE_PATH + IMAGE_NAME

# Draw the LUT shape (disable it to reduce CPU usage). 
DRAW_LUT_SHAPE = MIL.M_YES

def MdispWindowLevelingExample():
   MilOriginalImage = None

   # Allocate the MIL Application, System and Display.
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Restore target image from disk.
   MilImage = MIL.MbufRestore(IMAGE_FILE, MilSystem)

   # Dynamically calculates the maximum value of the image using processing. 
   MilExtremeResult = MIL.MimAllocResult(MIL.MbufInquire(MilImage, MIL.M_OWNER_SYSTEM),
                                         1,
                                         MIL.M_EXTREME_LIST)

   MIL.MimFindExtreme(MilImage, MilExtremeResult, MIL.M_MAX_VALUE)

   ImageMaxValue = MIL.MimGetResult(MilExtremeResult, MIL.M_VALUE)[0]

   MIL.MimGetResult(MilExtremeResult, MIL.M_VALUE)
   
   MIL.MimFree(MilExtremeResult)

   # Set the maximum value of the image to indicate to MIL how to initialize 
   # the default display LUT.
   MIL.MbufControl(MilImage, MIL.M_MAX, ImageMaxValue)

   # Display the image (to specify a user-defined window, use MdispSelectWindow()).
   MIL.MdispSelect(MilDisplay, MilImage)

   # Determine the maximum displayable value of the current display. 
   DisplaySizeBit = MIL.MdispInquire(MilDisplay, MIL.M_SIZE_BIT)
   DisplayMaxValue = (1<<DisplaySizeBit)-1

   # Print key information. 
   print("\nINTERACTIVE WINDOW LEVELING:")
   print("----------------------------\n")

   print("Image name : {IMAGE_NAME}".format(IMAGE_NAME=IMAGE_NAME))

   print("Image size : {ImageSizeX} x {ImageSizeY}".format(ImageSizeX=MIL.MbufInquire(MilImage, MIL.M_SIZE_X), ImageSizeY=MIL.MbufInquire(MilImage, MIL.M_SIZE_Y)))
   print("Image max  : {ImageMaxValue}".format(ImageMaxValue=ImageMaxValue))
   print("Display max:  {DisplayMaxValue}\n".format(DisplayMaxValue=DisplayMaxValue))

   # Allocate a LUT buffer according to the image maximum value and
   # display pixel depth.
   MilLut = MIL.MbufAlloc1d(MilSystem, ImageMaxValue + 1, 16 if DisplaySizeBit > 8 else 8 + MIL.M_UNSIGNED, MIL.M_LUT)

   # Generate a LUT with a full range ramp and set its maximum value. 
   MIL.MgenLutRamp(MilLut, 0, 0, ImageMaxValue, DisplayMaxValue)
   MIL.MbufControl(MilLut, MIL.M_MAX, DisplayMaxValue)

   # Set the display LUT. 
   MIL.MdispLut(MilDisplay, MilLut)

   # Interactive Window Leveling using keyboard.
   print("Keys assignment:\n")
   print("Arrow keys :    Left=move Left, Right=move Right, Down=Narrower, Up=Wider.")
   print("Intensity keys: L=Lower,  U=Upper,  R=Reset.")
   print("Press <Enter> to end.\n")

   # Modify LUT shape according to the arrow keys and update it. 
   Ch = 0
   Start = 0
   End = ImageMaxValue
   InflectionLevel = DisplayMaxValue
   Step = int((ImageMaxValue + 1) / 128)
   Step = max(Step, 4)

   while (chr(Ch) != '\r'):
      # Left arrow: Move region left.
      if Ch == 0x4B:
         Start -= Step
         End -= Step

      # Right arrow: Move region right.
      elif Ch == 0x4D:
         Start += Step
         End += Step

      # Down arrow: Narrow region. 
      elif Ch == 0x50:
         Start += Step
         End -= Step
      
      # Up arrow: Widen region. 
      elif Ch == 0x48:
         Start -= Step
         End += Step

      # L key: Lower inflexion point. 
      elif chr(Ch) == "L" or chr(Ch) == "l":
         InflectionLevel -= 1

      # U key: Upper inflexion point.
      elif chr(Ch) == "U" or chr(Ch) == "u":
         InflectionLevel += 1

      # R key: Reset the LUT to full image range.
      elif chr(Ch) == "R" or chr(Ch) == "r":
         Start = 0
         End = ImageMaxValue 
         InflectionLevel = DisplayMaxValue

      # Saturate.
      End = min(End, ImageMaxValue)
      Start = min(Start, End)
      End = max(End, Start)
      Start = max(Start, 0)
      End = max(End, 0)
      InflectionLevel = max(InflectionLevel, 0)
      InflectionLevel = min(InflectionLevel, DisplayMaxValue)
      print("Inflection points: Low=({Start},0), High=({End},{InflectionLevel}).   \r".format(Start=Start, End=End, InflectionLevel=InflectionLevel), end='')

      # Generate a LUT with 3 slopes and saturated at both ends.
      MIL.MgenLutRamp(MilLut, 0, 0, Start, 0)
      MIL.MgenLutRamp(MilLut, Start, 0, End, InflectionLevel)
      MIL.MgenLutRamp(MilLut, End, InflectionLevel, ImageMaxValue, DisplayMaxValue)

      # Update the display LUT.
      MIL.MdispLut(MilDisplay, MilLut)

      if DRAW_LUT_SHAPE:
         if not MilOriginalImage:
            MilOriginalImage = MIL.MbufRestore(IMAGE_FILE, MilSystem)
         DrawLutShape(MilDisplay, MilOriginalImage, MilImage, Start, End, InflectionLevel, ImageMaxValue, DisplayMaxValue)

      # If its an arrow key, get the second code. 
      Ch = MIL.MosGetch()
      if Ch == 0xE0:
            Ch = MIL.MosGetch()
   
   print("\n")

   # Free all allocations.
   MIL.MbufFree(MilLut)
   MIL.MbufFree(MilImage)
   if MilOriginalImage != MIL.M_NULL:
      MIL.MbufFree(MilOriginalImage)
   
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0 


# Function to draw the current LUT's shape in the image.

# Note: This simple annotation method requires significant update
#       and CPU time since it repaints the entire image every time.

def DrawLutShape(MilDisplay, MilOriginalImage, MilImage, Start, End, InflexionIntensity, ImageMaxValue, DisplayMaxValue):
   # Inquire image dimensions. 
   ImageSizeX = MIL.MbufInquire(MilImage, MIL.M_SIZE_X)
   ImageSizeY = MIL.MbufInquire(MilImage, MIL.M_SIZE_Y)

   # Calculate the drawing parameters. 
   Xstep  = ImageSizeX / ImageMaxValue
   Xstart = Start * Xstep
   Xend   = End * Xstep
   Ystep  = (ImageSizeY / 4.0) / DisplayMaxValue
   Ymin   = ImageSizeY - 2
   Yinf   = Ymin - (InflexionIntensity * Ystep)
   Ymax   = Ymin - (DisplayMaxValue * Ystep)

   # To increase speed, disable display update until all annotations are done. 
   MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_DISABLE)

   # Restore the original image.
   MIL.MbufCopy(MilOriginalImage, MilImage)
   
   # Draw axis max and min values. 
   MIL.MgraColor(MIL.M_DEFAULT, float(ImageMaxValue))
   MIL.MgraText(MIL.M_DEFAULT, MilImage, 4, int(Ymin - 22), "0")
   MIL.MgraText(MIL.M_DEFAULT, MilImage, 4, int(Ymax - 16), str(DisplayMaxValue))
   MIL.MgraText(MIL.M_DEFAULT, MilImage, ImageSizeX - 38 , Ymin - 22, str(ImageMaxValue))

   # Draw LUT Shape (X axis is display values and Y is image values). 
   MIL.MgraLine(MIL.M_DEFAULT, MilImage, 0, int(Ymin), int(Xstart), int(Ymin))
   MIL.MgraLine(MIL.M_DEFAULT, MilImage, int(Xstart), int(Ymin), int(Xend), int(Yinf))
   MIL.MgraLine(MIL.M_DEFAULT, MilImage, int(Xend), int(Yinf), ImageSizeX - 1, int(Ymax))

   # Enable display update to show the result. 
   MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_ENABLE)


   
if __name__ == "__main__":
   MdispWindowLevelingExample()
