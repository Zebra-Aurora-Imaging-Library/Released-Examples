#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#**************************************************************************************
# 
# File name: Mcol.py  
#
# Synopsis:  This program contains 4 examples using the MIL Color module.
#
#            The first example performs color segmentation of an image
#            by classifying each pixel with one out of 6 color samples.
#            The ratio of each color in the image is then calculated.
#
#            The second example performs color matching of circular regions
#            in objects located with model finder.
#
#            The third example performs color separation in order to
#            separate 2 types of ink on a piece of paper.
#
#            The fourth example performs a color to grayscale conversion
#            using the principal component projection functionality.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
#

import mil as MIL

# Display image margin 
DISPLAY_CENTER_MARGIN_X  = 5
DISPLAY_CENTER_MARGIN_Y  = 5

# Color patch sizes 
COLOR_PATCH_SIZEX        = 30
COLOR_PATCH_SIZEY        = 40

#****************************************************************************
# Main.
#****************************************************************************

def McolExample():
   
   # Allocate defaults. 
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Run the color segmentation example. 
   ColorSegmentationExample(MilSystem, MilDisplay)

   # Run the color matching example. 
   ColorMatchingExample(MilSystem, MilDisplay)

   # Run the color projection example. 
   ColorSeparationExample(MilSystem, MilDisplay)

   # Run the RGB to grayscale conversion example. 
   RGBtoGrayscaleExample(MilSystem, MilDisplay)

   # Free defaults.     
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0
   
#***************************************************************************
# Color Segmentation using color samples. 
#***************************************************************************

# Image filenames 
CANDY_SAMPLE_IMAGE_FILE   = MIL.M_IMAGE_PATH + "CandySamples.mim"
CANDY_TARGET_IMAGE_FILE   = MIL.M_IMAGE_PATH + "Candy.mim"

# Number of samples 
NUM_SAMPLES               = 6

# Draw spacing and offset 
CANDY_SAMPLES_XSPACING    = 35
CANDY_SAMPLES_YOFFSET     = 145

# Match parameters 
MATCH_MODE                = MIL.M_MIN_DIST_VOTE # Minimal distance vote mode.        
DISTANCE_TYPE             = MIL.M_MAHALANOBIS   # Mahalanobis distance.              
TOLERANCE_MODE            = MIL.M_SAMPLE_STDDEV # Standard deviation tolerance mode. 
TOLERANCE_VALUE           = 6.0             # Mahalanobis tolerance value.       
RED_TOLERANCE_VALUE       = 6.0
YELLOW_TOLERANCE_VALUE    = 12.0
PINK_TOLERANCE_VALUE      = 5.0

def ColorSegmentationExample(MilSystem, MilDisplay):

   # Blank spaces to align the samples names evenly. 
   Spaces = ["", " ", "  ", "   "] 

   # Color samples names. 
   SampleNames = ["Green", "Red", "Yellow", "Purple", "Blue", "Pink"]

   # Color samples position: OffsetX, OffsetY  
   SamplesROI = [[58, 143], [136, 148], [217, 144], [295, 142], [367, 143], [442, 147]]

   # Color samples size. 
   SampleSizeX = 36
   SampleSizeY = 32

   print("\nCOLOR SEGMENTATION:")
   print("-------------------")

   # Allocate the parent display image.   
   SourceSizeX = MIL.MbufDiskInquire(CANDY_SAMPLE_IMAGE_FILE, MIL.M_SIZE_X)
   SourceSizeY = MIL.MbufDiskInquire(CANDY_SAMPLE_IMAGE_FILE, MIL.M_SIZE_Y)
   DisplayImage = MIL.MbufAllocColor(MilSystem, 3, 2*SourceSizeX + DISPLAY_CENTER_MARGIN_X, SourceSizeY, 8+MIL.M_UNSIGNED, MIL.M_IMAGE+MIL.M_DISP+MIL.M_PROC)
   MIL.MbufClear(DisplayImage, MIL.M_COLOR_BLACK)

   # Create a source and dest child in the display image. 
   SourceChild = MIL.MbufChild2d(DisplayImage, 0, 0, SourceSizeX, SourceSizeY)
   DestChild = MIL.MbufChild2d(DisplayImage, SourceSizeX + DISPLAY_CENTER_MARGIN_X, 0, SourceSizeX, SourceSizeY)

   # Load the source image into the source child. 
   MIL.MbufLoad(CANDY_SAMPLE_IMAGE_FILE, SourceChild)
  
   # Allocate a color matching context. 
   MatchContext = MIL.McolAlloc(MilSystem, MIL.M_COLOR_MATCHING, MIL.M_RGB, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Define each color sample in the context. 
   for i in range(0, NUM_SAMPLES):
      MIL.McolDefine(MatchContext, SourceChild, MIL.M_SAMPLE_LABEL(i+1), MIL.M_IMAGE, SamplesROI[i][0], SamplesROI[i][1], SampleSizeX, SampleSizeY)

   # Set the color matching parameters. 
   MIL.McolSetMethod(MatchContext, MATCH_MODE, DISTANCE_TYPE, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MIL.McolControl(MatchContext, MIL.M_CONTEXT, MIL.M_DISTANCE_TOLERANCE_MODE, TOLERANCE_MODE)
   MIL.McolControl(MatchContext, MIL.M_ALL, MIL.M_DISTANCE_TOLERANCE, TOLERANCE_VALUE)

   # Adjust tolerances for the red, yellow and pink samples. 
   MIL.McolControl(MatchContext, MIL.M_SAMPLE_INDEX(1), MIL.M_DISTANCE_TOLERANCE, RED_TOLERANCE_VALUE)
   MIL.McolControl(MatchContext, MIL.M_SAMPLE_INDEX(2), MIL.M_DISTANCE_TOLERANCE, YELLOW_TOLERANCE_VALUE)
   MIL.McolControl(MatchContext, MIL.M_SAMPLE_INDEX(5), MIL.M_DISTANCE_TOLERANCE, PINK_TOLERANCE_VALUE)

   # Preprocess the context. 
   MIL.McolPreprocess(MatchContext, MIL.M_DEFAULT) 

   # Fill the samples colors array. 
   SampleMatchColor = []
   for i in range(0, NUM_SAMPLES):
      SampleColor = 3*[0]
      SampleColor[0] = MIL.McolInquire(MatchContext, MIL.M_SAMPLE_LABEL(i+1), MIL.M_MATCH_SAMPLE_COLOR_BAND_0 + MIL.M_TYPE_MIL_INT)
      SampleColor[1] = MIL.McolInquire(MatchContext, MIL.M_SAMPLE_LABEL(i+1), MIL.M_MATCH_SAMPLE_COLOR_BAND_1 + MIL.M_TYPE_MIL_INT)
      SampleColor[2] = MIL.McolInquire(MatchContext, MIL.M_SAMPLE_LABEL(i+1), MIL.M_MATCH_SAMPLE_COLOR_BAND_2 + MIL.M_TYPE_MIL_INT)
      SampleMatchColor.append(SampleColor)
      

   # Draw the samples. 
   DrawSampleColors(DestChild, SampleMatchColor, SampleNames, NUM_SAMPLES, CANDY_SAMPLES_XSPACING, CANDY_SAMPLES_YOFFSET)

   # Select the image buffer for display. 
   MIL.MdispSelect(MilDisplay, DisplayImage)

   # Pause to show the original image. 
   print("Color samples are defined for each possible candy color.")
   print("Press <Enter> to do color matching.\n")
   MIL.MosGetch()

   # Load the target image.
   MIL.MbufClear(DisplayImage, MIL.M_COLOR_BLACK)
   MIL.MbufLoad(CANDY_TARGET_IMAGE_FILE, SourceChild)

   # Allocate a color matching result buffer. 
   MatchResult = MIL.McolAllocResult(MilSystem, MIL.M_COLOR_MATCHING_RESULT, MIL.M_DEFAULT)

   # Enable controls to draw the labeled color image. 
   MIL.McolControl(MatchContext, MIL.M_CONTEXT, MIL.M_GENERATE_PIXEL_MATCH, MIL.M_ENABLE)
   MIL.McolControl(MatchContext, MIL.M_CONTEXT, MIL.M_GENERATE_SAMPLE_COLOR_LUT, MIL.M_ENABLE)

   # Match with target image. 
   MIL.McolMatch(MatchContext, SourceChild, MIL.M_DEFAULT, MIL.M_NULL, MatchResult, MIL.M_DEFAULT)

   # Retrieve and display results.    
   print("Each pixel of the mixture is matched with one of the color samples.\n")
   print("Color segmentation results:")
   print("---------------------------")

   for SampleIndex in range(0, NUM_SAMPLES):      
      MatchScore = MIL.McolGetResult(MatchResult, MIL.M_DEFAULT, MIL.M_SAMPLE_INDEX(SampleIndex), MIL.M_SCORE)
      SpacesIndex = 6 - len(SampleNames[SampleIndex])
      print("Ratio of {:s}{:s} sample = {:5.2f}%".format(SampleNames[SampleIndex], Spaces[SpacesIndex], MatchScore[0]))
      
   print("\nResults reveal the low proportion of Blue candy.\n")

   # Draw the colored label image in the destination child. 
   MIL.McolDraw(MIL.M_DEFAULT, MatchResult, DestChild, MIL.M_DRAW_PIXEL_MATCH_USING_COLOR, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   # Pause to show the result image. 
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free all allocations. 
   MIL.MbufFree(DestChild)
   MIL.MbufFree(SourceChild)
   MIL.MbufFree(DisplayImage)
   MIL.McolFree(MatchResult)
   MIL.McolFree(MatchContext)
   

#****************************************************************************
# Color matching in labeled regions.
#****************************************************************************
# Image filenames 
FUSE_SAMPLES_IMAGE       = MIL.M_IMAGE_PATH + "FuseSamples.mim"
FUSE_TARGET_IMAGE        = MIL.M_IMAGE_PATH + "Fuse.mim"

# Model Finder context filename 
FINDER_CONTEXT           = MIL.M_IMAGE_PATH + "FuseModel.mmf"

# Number of fuse sample objects 
NUM_FUSES                = 4

# Draw spacing and offset 
FUSE_SAMPLES_XSPACING    = 40
FUSE_SAMPLES_YOFFSET     = 145

def ColorMatchingExample(MilSystem, MilDisplay):
   
   # Color sample names 
   SampleNames = ["Green", " Blue", " Red", "Yellow"]

   # Sample ROIs coordinates: OffsetX, OffsetY, SizeX, SizeY 
   SampleROIs =  [[54, 139, 28, 14],
                 [172, 137, 30, 23],
                 [296, 135, 31, 23],
                 [417, 134, 27, 22]]

   # Array of match sample colors. 
   SampleMatchColor = NUM_FUSES*3*[0]

   print("\nCOLOR IDENTIFICATION:")
   print("---------------------")

   # Allocate the parent display image. 
   SizeX = MIL.MbufDiskInquire(FUSE_TARGET_IMAGE, MIL.M_SIZE_X)
   SizeY = MIL.MbufDiskInquire(FUSE_TARGET_IMAGE, MIL.M_SIZE_Y)   
   DisplayImage = MIL.MbufAllocColor(MilSystem, 3, 2*SizeX + DISPLAY_CENTER_MARGIN_X, SizeY, 8+MIL.M_UNSIGNED, MIL.M_IMAGE+MIL.M_DISP+MIL.M_PROC)
   MIL.MbufClear(DisplayImage, MIL.M_COLOR_BLACK)

   # Allocate the model, area and label images. 
   ModelImage = MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 8+MIL.M_UNSIGNED, MIL.M_IMAGE+MIL.M_PROC+MIL.M_DISP)
   AreaImage = MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 8+MIL.M_UNSIGNED, MIL.M_IMAGE+MIL.M_PROC+MIL.M_DISP)

   # Create a source and destination child in the display image. 
   SourceChild = MIL.MbufChild2d(DisplayImage, 0, 0, SizeX, SizeY)
   DestChild = MIL.MbufChild2d(DisplayImage, SizeX + DISPLAY_CENTER_MARGIN_X, 0, SizeX, SizeY)

   # Load the sample source image. 
   MIL.MbufLoad(FUSE_SAMPLES_IMAGE, SourceChild)

   # Display the image buffer. 
   MIL.MdispSelect(MilDisplay, DisplayImage)

   # Prepare the overlay. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)
   OverlayID = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)
   OverlaySourceChild = MIL.MbufChild2d(OverlayID, 0, 0, SizeX, SizeY)
   OverlayDestChild = MIL.MbufChild2d(OverlayID, SizeX + DISPLAY_CENTER_MARGIN_X, 0, SizeX, SizeY)

   # Prepare the model finder context and result. 
   FuseFinderCtx = MIL.MmodRestore(FINDER_CONTEXT, MilSystem, MIL.M_DEFAULT)   
   MIL.MmodPreprocess(FuseFinderCtx, MIL.M_DEFAULT)
   FuseFinderRes = MIL.MmodAllocResult(MilSystem, MIL.M_DEFAULT)

   # Allocate a color match context and result. 
   ColMatchContext = MIL.McolAlloc(MilSystem, MIL.M_COLOR_MATCHING, MIL.M_RGB, MIL.M_DEFAULT, MIL.M_DEFAULT)
   ColMatchResult = MIL.McolAllocResult(MilSystem, MIL.M_COLOR_MATCHING_RESULT, MIL.M_DEFAULT)

   # Define the color samples in the context. 
   for i in range(0, NUM_FUSES):
      
      MIL.McolDefine(ColMatchContext, SourceChild, MIL.M_SAMPLE_LABEL(i+1), MIL.M_IMAGE, 
                  SampleROIs[i][0], 
                  SampleROIs[i][1], 
                  SampleROIs[i][2],
                  SampleROIs[i][3])
      

   # Preprocess the context. 
   MIL.McolPreprocess(ColMatchContext, MIL.M_DEFAULT)
   
   # Fill the samples colors array. 
   SampleMatchColor = []
   for i in range(0, NUM_FUSES):
      SampleColor = 3*[0]
      SampleColor[0] = MIL.McolInquire(ColMatchContext, MIL.M_SAMPLE_LABEL(i+1), MIL.M_MATCH_SAMPLE_COLOR_BAND_0 + MIL.M_TYPE_MIL_INT)
      SampleColor[1] = MIL.McolInquire(ColMatchContext, MIL.M_SAMPLE_LABEL(i+1), MIL.M_MATCH_SAMPLE_COLOR_BAND_1 + MIL.M_TYPE_MIL_INT)
      SampleColor[2] = MIL.McolInquire(ColMatchContext, MIL.M_SAMPLE_LABEL(i+1), MIL.M_MATCH_SAMPLE_COLOR_BAND_2 + MIL.M_TYPE_MIL_INT)
      SampleMatchColor.append(SampleColor)
      

   # Draw the color samples. 
   DrawSampleColors(DestChild, SampleMatchColor, SampleNames, 
                    NUM_FUSES, FUSE_SAMPLES_XSPACING, FUSE_SAMPLES_YOFFSET)
  
   # Draw the sample ROIs in the source image overlay. 
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED)
   for SampleIndex in range(0, NUM_FUSES):
      
      XEnd = SampleROIs[SampleIndex][0] + SampleROIs[SampleIndex][2] - 1
      YEnd = SampleROIs[SampleIndex][1] + SampleROIs[SampleIndex][3] - 1
      MIL.MgraRect(MIL.M_DEFAULT, OverlaySourceChild, SampleROIs[SampleIndex][0], 
                                              SampleROIs[SampleIndex][1], 
                                              XEnd, YEnd)
      

   # Pause to show the source image. 
   print("Colors are defined using one color sample region per fuse.")
   print("Press <Enter> to process the target image.\n")
   MIL.MosGetch()

   # Clear the overlay. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)
   
   # Disable the display update. 
   MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_DISABLE)

   # Load the target image into the source child. 
   MIL.MbufLoad(FUSE_TARGET_IMAGE, SourceChild)

   # Get the grayscale model image and copy it into the display dest child. 
   MIL.MimConvert(SourceChild, ModelImage, MIL.M_RGB_TO_L)
   MIL.MbufCopy(ModelImage, DestChild)

   # Find the Model. 
   MIL.MmodFind(FuseFinderCtx, ModelImage, FuseFinderRes)   

   # Draw the blob image: labeled circular areas centered at each found fuse occurrence. 
   Number = MIL.MmodGetResult(FuseFinderRes, MIL.M_DEFAULT, MIL.M_NUMBER+MIL.M_TYPE_MIL_INT)
   MIL.MbufClear(AreaImage, 0)
   for i in range(0, Number):
      # Get the position 
      X = MIL.MmodGetResult(FuseFinderRes, i, MIL.M_POSITION_X)
      Y = MIL.MmodGetResult(FuseFinderRes, i, MIL.M_POSITION_Y)
      # Set the label color 
      MIL.MgraColor(MIL.M_DEFAULT, float(i+1))
      # Draw the filled circle 
      MIL.MgraArcFill(MIL.M_DEFAULT, AreaImage, X, Y, 20, 20, 0, 360)
      

   # Enable controls to draw the labeled color image. 
   MIL.McolControl(ColMatchContext, MIL.M_CONTEXT, MIL.M_SAVE_AREA_IMAGE, MIL.M_ENABLE)
   MIL.McolControl(ColMatchContext, MIL.M_CONTEXT, MIL.M_GENERATE_SAMPLE_COLOR_LUT, MIL.M_ENABLE)

   # Perform the color matching. 
   MIL.McolMatch(ColMatchContext, SourceChild, MIL.M_DEFAULT, AreaImage, ColMatchResult, MIL.M_DEFAULT)

   # Draw the label image into the overlay child. 
   MIL.McolDraw(MIL.M_DEFAULT, ColMatchResult, OverlayDestChild, 
                   MIL.M_DRAW_AREA_MATCH_USING_COLOR, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   # Draw the model position over the colored areas. 
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_BLUE)
   MIL.MmodDraw(MIL.M_DEFAULT, FuseFinderRes, OverlayDestChild, MIL.M_DRAW_BOX+MIL.M_DRAW_POSITION, 
            MIL.M_ALL, MIL.M_DEFAULT)

   # Enable the display update. 
   MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_ENABLE)
   
   # Pause to show the resulting image. 
   print("Fuses are located using the Model Finder tool.")
   print("The color of each target area is identified.")
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free all allocations. 
   MIL.MmodFree(FuseFinderRes)
   MIL.MmodFree(FuseFinderCtx)
   MIL.MbufFree(AreaImage)
   MIL.MbufFree(ModelImage)
   MIL.MbufFree(SourceChild)
   MIL.MbufFree(DestChild)
   MIL.MbufFree(OverlaySourceChild)
   MIL.MbufFree(OverlayDestChild)
   MIL.MbufFree(DisplayImage)   
   MIL.McolFree(ColMatchContext)
   MIL.McolFree(ColMatchResult)
   

#****************************************************************************
# Perform color separation of colored inks on a piece of paper.
#****************************************************************************
# Source image 
WRITING_IMAGE_FILE    = MIL.M_IMAGE_PATH + "stamp.mim"

# Color triplets 
BACKGROUND_COLOR = [245, 234, 206]         
WRITING_COLOR    = [141, 174, 174]         
STAMP_COLOR      = [226, 150, 118]         

# Drawing spacing 
PATCHES_XSPACING   = 70

def ColorSeparationExample(MilSystem, MilDisplay):

   # Color samples' names 
   ColorNames = ["BACKGROUND", "WRITING", "STAMP"] 

   # Array with color patches to draw. 
   Colors =  [BACKGROUND_COLOR, WRITING_COLOR, STAMP_COLOR]

   # Samples' color coordinates 
   BackgroundColor = BACKGROUND_COLOR
   SelectedColor   = WRITING_COLOR
   RejectedColor   = STAMP_COLOR

   print("\nCOLOR SEPARATION:")
   print("-----------------")

   # Allocate an array buffer and fill it with the color coordinates. 
   ColorsArray = MIL.MbufAlloc2d(MilSystem, 3, 3, 8+MIL.M_UNSIGNED, MIL.M_ARRAY)
   MIL.MbufPut2d(ColorsArray, 0, 0, 3, 1, BackgroundColor)
   MIL.MbufPut2d(ColorsArray, 0, 1, 3, 1, SelectedColor)
   MIL.MbufPut2d(ColorsArray, 0, 2, 3, 1, RejectedColor)

   # Allocate the parent display image.    
   SourceSizeX = MIL.MbufDiskInquire(WRITING_IMAGE_FILE, MIL.M_SIZE_X)
   SourceSizeY = MIL.MbufDiskInquire(WRITING_IMAGE_FILE, MIL.M_SIZE_Y)
   DisplayImage = MIL.MbufAllocColor(MilSystem, 
                  3, 
                  2*SourceSizeX + DISPLAY_CENTER_MARGIN_X, 
                  SourceSizeY, 
                  8+MIL.M_UNSIGNED, 
                  MIL.M_IMAGE+MIL.M_DISP+MIL.M_PROC)
   MIL.MbufClear(DisplayImage, MIL.M_COLOR_BLACK)

   # Clear the overlay. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)   

   # Create a source and dest child in the display image 
   SourceChild = MIL.MbufChild2d(DisplayImage, 0, 0, SourceSizeX, SourceSizeY)
   DestChild = MIL.MbufChild2d(DisplayImage, SourceSizeX + DISPLAY_CENTER_MARGIN_X, 0, SourceSizeX, SourceSizeY)

   # Load the source image into the display image source child. 
   MIL.MbufLoad(WRITING_IMAGE_FILE, SourceChild)
     
   # Draw the color patches. 
   DrawSampleColors(DestChild, Colors, ColorNames, 3, PATCHES_XSPACING, -1)

   # Display the image. 
   MIL.MdispSelect(MilDisplay, DisplayImage)

   # Pause to show the source image and color patches. 
   print("The writing will be separated from the stamp using the following triplets:")
   print("the background color: beige [{}, {}, {}],".format(int(BackgroundColor[0]), int(BackgroundColor[1]), int(BackgroundColor[2])))
   print("the writing color   : green [{}, {}, {}],".format(int(SelectedColor[0]), int(SelectedColor[1]), int(SelectedColor[2])))
   print("the stamp color     : red   [{}, {}, {}].\n".format(int(RejectedColor[0]), int(RejectedColor[1]), int(RejectedColor[2])))
   print("Press <Enter> to extract the writing.\n")
   MIL.MosGetch()

   # Perform the color projection. 
   MIL.McolProject(SourceChild, ColorsArray, DestChild, MIL.M_NULL, MIL.M_COLOR_SEPARATION, MIL.M_DEFAULT, MIL.M_NULL)

   # Wait for a key. 
   print("Press <Enter> to extract the stamp.\n")
   MIL.MosGetch()

   # Switch the order of the selected vs rejected colors in the color array. 
   MIL.MbufPut2d(ColorsArray, 0, 2, 3, 1, SelectedColor)
   MIL.MbufPut2d(ColorsArray, 0, 1, 3, 1, RejectedColor)   

   # Perform the color projection. 
   MIL.McolProject(SourceChild, ColorsArray, DestChild, MIL.M_NULL, MIL.M_COLOR_SEPARATION, MIL.M_DEFAULT, MIL.M_NULL)

   # Wait for a key. 
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free all allocations. 
   MIL.MbufFree(ColorsArray)
   MIL.MbufFree(SourceChild)
   MIL.MbufFree(DestChild)
   MIL.MbufFree(DisplayImage)   
   

#****************************************************************************
# Perform RGB to grayscale conversion
#****************************************************************************
# Source image 
RGB_IMAGE_FILE  = MIL.M_IMAGE_PATH + "BinderClip.mim"

def RGBtoGrayscaleExample(MilSystem, MilDisplay):
   
   print("\nCONVERSION FROM RGB TO GRAYSCALE:")
   print("---------------------------------")
   print("The example compares principal component projection to luminance based\nconversion.\n")

   # Inquire size and type of the source image. 
   SourceSizeX = MIL.MbufDiskInquire(RGB_IMAGE_FILE, MIL.M_SIZE_X, MIL.M_NULL)
   SourceSizeY = MIL.MbufDiskInquire(RGB_IMAGE_FILE, MIL.M_SIZE_Y, MIL.M_NULL)
   Type        = MIL.MbufDiskInquire(RGB_IMAGE_FILE, MIL.M_TYPE, MIL.M_NULL)

   # Allocate buffer to display the input image and the results. 
   DisplayImage = MIL.MbufAllocColor(MilSystem, 3, SourceSizeX, 3 * SourceSizeY + 2 * DISPLAY_CENTER_MARGIN_Y, Type, MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP)
   MIL.MbufClear(DisplayImage, 0)

   # Create a source child in the display image. 
   SourceChild = MIL.MbufChildColor2d(DisplayImage, MIL.M_ALL_BANDS, 0, 0, SourceSizeX, SourceSizeY)

   # Create a destination child in the display image for luminance based conversion. 
   LuminanceChild = MIL.MbufChildColor2d(DisplayImage, MIL.M_ALL_BANDS, 0, SourceSizeY + DISPLAY_CENTER_MARGIN_Y,
                    SourceSizeX, SourceSizeY)

   # Create a destination child in the display image for principal component projection. 
   PCPchild = MIL.MbufChildColor2d(DisplayImage, MIL.M_ALL_BANDS, 0, 2 * SourceSizeY + 2 * DISPLAY_CENTER_MARGIN_Y,
                    SourceSizeX, SourceSizeY)

   # Load the source image into the display image source child. 
   MIL.MbufLoad(RGB_IMAGE_FILE, SourceChild)
   MIL.MdispSelect(MilDisplay, DisplayImage)

   # Extract luminance channel from source image and copy the result in the display image. 
   LuminanceResult = MIL.MbufAlloc2d(MilSystem, SourceSizeX, SourceSizeY, Type, MIL.M_IMAGE + MIL.M_PROC)
   MIL.MimConvert(SourceChild, LuminanceResult, MIL.M_RGB_TO_L)
   MIL.MbufCopy(LuminanceResult, LuminanceChild)

   print("Color image converted to grayscale using luminance based conversion\ntechnique.\n")
   print("Press <Enter> to convert the image using principal component projection\ntechnique.\n")
   MIL.MosGetch()

   # Create a mask to identify important colors in the source image. 
   Mask = MIL.MbufAlloc2d(MIL.M_DEFAULT_HOST, SourceSizeX, SourceSizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC)
   MIL.MbufClear(Mask, 0)

   # Define regions of interest (pixel colors with which to perform the color projection). 
   OffsetX   = 105
   OffsetY   = 45
   MaskSizeX = 60
   MaskSizeY = 20
   ChildMask = MIL.MbufChild2d(Mask, OffsetX, OffsetY, MaskSizeX, MaskSizeY)
   MIL.MbufClear(ChildMask, MIL.M_SOURCE_LABEL)
   MIL.MbufFree(ChildMask)

   OffsetX = 220
   ChildMask = MIL.MbufChild2d(Mask, OffsetX, OffsetY, MaskSizeX, MaskSizeY)
   MIL.MbufClear(ChildMask, MIL.M_SOURCE_LABEL)
   MIL.MbufFree(ChildMask)

   OffsetX = 335
   ChildMask = MIL.MbufChild2d(Mask, OffsetX, OffsetY, MaskSizeX, MaskSizeY)
   MIL.MbufClear(ChildMask, MIL.M_SOURCE_LABEL)

   # Perform principal component projection and copy the result in the display image. 
   PCPresult = MIL.MbufAlloc2d(MilSystem, SourceSizeX, SourceSizeY, Type, MIL.M_IMAGE + MIL.M_PROC)
   MIL.McolProject(SourceChild, Mask, PCPresult, MIL.M_NULL, MIL.M_PRINCIPAL_COMPONENT_PROJECTION, MIL.M_DEFAULT, MIL.M_NULL)
   MIL.MbufCopy(PCPresult, PCPchild)

   print("Color image converted to grayscale using principal component projection\ntechnique.")
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free all allocations. 
   MIL.MbufFree(ChildMask)
   MIL.MbufFree(Mask)
   MIL.MbufFree(PCPresult)
   MIL.MbufFree(LuminanceResult)
   MIL.MbufFree(PCPchild)
   MIL.MbufFree(LuminanceChild)
   MIL.MbufFree(SourceChild)
   MIL.MbufFree(DisplayImage)
   

#***************************************************************************
# Draw the samples as color patches.                                        
def DrawSampleColors(DestImage, pSamplesColors, pSampleNames, NumSamples, XSpacing, YOffset):
   
   DestSizeX = MIL.MbufInquire(DestImage, MIL.M_SIZE_X, MIL.M_NULL)
   DestSizeY = MIL.MbufInquire(DestImage, MIL.M_SIZE_Y, MIL.M_NULL)
   OffsetX = (DestSizeX - (NumSamples * COLOR_PATCH_SIZEX) - ((NumSamples - 1) * XSpacing)) /2.0
   OffsetY =  YOffset if YOffset > 0 else (DestSizeY - COLOR_PATCH_SIZEY)/2.0
   MIL.MgraFont(MIL.M_DEFAULT, MIL.M_FONT_DEFAULT_SMALL)

   for SampleIndex in range(0, NumSamples):
      
      MIL.MgraColor(MIL.M_DEFAULT, MIL.M_RGB888(pSamplesColors[SampleIndex][0], pSamplesColors[SampleIndex][1], pSamplesColors[SampleIndex][2]))
      MIL.MgraRectFill(MIL.M_DEFAULT, DestImage, OffsetX, OffsetY, OffsetX + COLOR_PATCH_SIZEX, OffsetY + COLOR_PATCH_SIZEY)
      MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_YELLOW)
      TextOffsetX = OffsetX + COLOR_PATCH_SIZEX / 2.0 - 4.0 * len(pSampleNames[SampleIndex]) + 0.5
      MIL.MgraText(MIL.M_DEFAULT, DestImage, TextOffsetX, OffsetY-20, pSampleNames[SampleIndex])
      OffsetX += (COLOR_PATCH_SIZEX + XSpacing)
      
   
if __name__ == "__main__":
   McolExample()