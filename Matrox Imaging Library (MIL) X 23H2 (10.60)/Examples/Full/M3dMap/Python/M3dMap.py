#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: m3dmap.py 
#
# Synopsis: This program inspects a wood surface using 
#           sheet-of-light profiling (laser) to find any depth defects.
#
# Printable calibration grids in PDF format can be found in your
# "Matrox Imaging/Images/" directory.
#
# When considering a laser-based 3D reconstruction system, the file "3D Setup Helper.xls"
# can be used to accelerate prototyping by choosing an adequate hardware configuration
# (angle, distance, lens, camera, ...). The file is located in your
# "Matrox Imaging/Tools/" directory.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################

 
import mil as MIL
import math

def M3dMapExample():
   # Allocate defaults.
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Run the depth correction example.
   DepthCorrectionExample(MilSystem, MilDisplay)

   # Run the calibrated camera example.
   CalibratedCameraExample(MilSystem, MilDisplay)

   # Free defaults.    
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)
   
   return 0

#****************************************************************************
# Depth correction example.
#****************************************************************************

# Input sequence specifications.
REFERENCE_PLANES_SEQUENCE_FILE = MIL.M_IMAGE_PATH + "ReferencePlanes.avi"
OBJECT_SEQUENCE_FILE           = MIL.M_IMAGE_PATH + "ScannedObject.avi"

# Peak detection parameters.
PEAK_WIDTH_NOMINAL        =  10
PEAK_WIDTH_DELTA          =   8
MIN_CONTRAST              = 140

# Calibration heights in mm.
CORRECTED_DEPTHS = [1.25, 2.50, 3.75, 5.00]

SCALE_FACTOR = 10000.0 # (depth in world units) * SCALE_FACTOR gives gray levels

# Annotation position.
CALIB_TEXT_POS_X        = 400   
CALIB_TEXT_POS_Y        =  15

def DepthCorrectionExample(MilSystem, MilDisplay):
   # Inquire characteristics of the input sequences.
   SizeX = MIL.MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_SIZE_X)
   SizeY = MIL.MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_SIZE_Y)
   NbReferencePlanes = MIL.MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_NUMBER_OF_IMAGES)
   FrameRate = MIL.MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_FRAME_RATE)
   NbObjectImages = MIL.MbufDiskInquire(OBJECT_SEQUENCE_FILE, MIL.M_NUMBER_OF_IMAGES)

   # Allocate buffer to hold images. 
   MilImage = MIL.MbufAlloc2d(MilSystem, SizeX, SizeY,  8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_DISP + MIL.M_PROC)
   MIL.MbufClear(MilImage, 0.0)

   print("\nDEPTH ANALYSIS:")
   print("---------------\n")
   print("This program performs a surface inspection to detect depth defects ")
   print("on a wood surface using a laser (sheet-of-light) profiling system.\n")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Select display. 
   MIL.MdispSelect(MilDisplay, MilImage)

   # Prepare for overlay annotations. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MilOverlayImage = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)
   MIL.MgraControl(MIL.M_DEFAULT, MIL.M_BACKGROUND_MODE, MIL.M_TRANSPARENT)
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_WHITE)

   # Allocate 3dmap objects. 
   MilLaser = MIL.M3dmapAlloc(MilSystem, MIL.M_LASER, MIL.M_DEPTH_CORRECTION)
   MilCalibScan = MIL.M3dmapAllocResult(MilSystem, MIL.M_LASER_CALIBRATION_DATA, MIL.M_DEFAULT)

   # Set laser line extraction options.
   MilPeakLocator = MIL.M3dmapInquire(MilLaser, MIL.M_DEFAULT, MIL.M_LOCATE_PEAK_1D_CONTEXT_ID + MIL.M_TYPE_MIL_ID)
   MIL.MimControl(MilPeakLocator, MIL.M_PEAK_WIDTH_NOMINAL, PEAK_WIDTH_NOMINAL)
   MIL.MimControl(MilPeakLocator, MIL.M_PEAK_WIDTH_DELTA  , PEAK_WIDTH_DELTA  )
   MIL.MimControl(MilPeakLocator, MIL.M_MINIMUM_CONTRAST  , MIN_CONTRAST      )

   # Open the calibration sequence file for reading.
   MIL.MbufImportSequence(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                   MIL.M_NULL, MIL.M_NULL, MIL.M_OPEN)

   # Read and process all images in the input sequence.
   StartTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)

   for n in range(NbReferencePlanes):

      # Read image from sequence. 
      MIL.MbufImportSequence(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_LOAD, MIL.M_NULL,
                                                             MilImage, MIL.M_DEFAULT, 1, MIL.M_READ)

      # Annotate the image with the calibration height. 
      MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)
      CalibString = "Reference plane {idx}: {height:.2f} mm".format(idx=(n+1), height=round(CORRECTED_DEPTHS[n], 2))
      MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, CALIB_TEXT_POS_X, CALIB_TEXT_POS_Y, CalibString)

      # Set desired corrected depth of next reference plane. 
      MIL.M3dmapControl(MilLaser, MIL.M_DEFAULT, MIL.M_CORRECTED_DEPTH, CORRECTED_DEPTHS[n] * SCALE_FACTOR)

      # Analyze the image to extract laser line.
      MIL.M3dmapAddScan(MilLaser, MilCalibScan, MilImage, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Wait to have a proper frame rate, if necessary. 
      EndTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)
      WaitTime = (1.0 / FrameRate) - (EndTime - StartTime)
      if WaitTime > 0:
         MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_WAIT, WaitTime)
      StartTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)
   
   # Close the calibration sequence file.
   MIL.MbufImportSequence(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                  MIL.M_NULL, MIL.M_NULL, MIL.M_CLOSE)

   # Calibrate the laser profiling context using reference planes of known heights.
   MIL.M3dmapCalibrate(MilLaser, MilCalibScan, MIL.M_NULL, MIL.M_DEFAULT)

   print("The laser profiling system has been calibrated using 4 reference")
   print("planes of known heights.\n")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   print("The wood surface is being scanned.\n")

   # Free the result buffer used for calibration because it will not be used anymore.
   MIL.M3dmapFree(MilCalibScan)
   MilCalibScan = MIL.M_NULL

   # Allocate the result buffer for the scanned depth corrected data.
   MilScan = MIL.M3dmapAllocResult(MilSystem, MIL.M_DEPTH_CORRECTED_DATA, MIL.M_DEFAULT)

   # Open the object sequence file for reading.
   FrameRate = MIL.MbufDiskInquire(OBJECT_SEQUENCE_FILE, MIL.M_FRAME_RATE)
   MIL.MbufImportSequence(OBJECT_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                           MIL.M_NULL, MIL.M_OPEN)

   # Read and process all images in the input sequence.
   StartTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)

   for n in range(NbObjectImages):
      # Read image from sequence.
      MIL.MbufImportSequence(OBJECT_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_LOAD, MIL.M_NULL, MilImage,
                                                                     MIL.M_DEFAULT, 1, MIL.M_READ)

      # Analyze the image to extract laser line and correct its depth. 
      MIL.M3dmapAddScan(MilLaser, MilScan, MilImage, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Wait to have a proper frame rate, if necessary. 
      EndTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)
      WaitTime = (1.0/FrameRate) - (EndTime - StartTime)
      if WaitTime > 0:
         MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_WAIT, WaitTime)
      StartTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)

   # Close the object sequence file.
   MIL.MbufImportSequence(OBJECT_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                    MIL.M_NULL, MIL.M_CLOSE)

   # Allocate the image for a partially corrected depth map.
   MilDepthMap = MIL.MbufAlloc2d(MilSystem, SizeX, NbObjectImages, 16 + MIL.M_UNSIGNED,
                                                       MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP)

   # Get partially corrected depth map from accumulated information in the result buffer.
   MIL.M3dmapCopyResult(MilScan, MIL.M_DEFAULT, MilDepthMap, MIL.M_PARTIALLY_CORRECTED_DEPTH_MAP, MIL.M_DEFAULT)

   # Disable display updates.
   MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_DISABLE);

   # Show partially corrected depth map and find defects.
   SetupColorDisplay(MilSystem, MilDisplay, MIL.MbufInquire(MilDepthMap, MIL.M_SIZE_BIT, MIL.M_NULL))
   
   # Display partially corrected depth map.
   MIL.MdispSelect(MilDisplay, MilDepthMap)
   MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_ENABLE);
   MilOverlayImage = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)

   print("The pseudo-color depth map of the surface is displayed.\n")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   PerformBlobAnalysis(MilSystem, MilDisplay, MilOverlayImage, MilDepthMap)

   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Disassociates display LUT and clear overlay.
   MIL.MdispSelect(MilDisplay, MIL.M_NULL)
   MIL.MdispLut(MilDisplay, MIL.M_DEFAULT)
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)

   # Free all allocations.
   MIL.M3dmapFree(MilScan)
   MIL.M3dmapFree(MilLaser)
   MIL.MbufFree(MilDepthMap)
   MIL.MbufFree(MilImage)

# Values used for binarization. 
EXPECTED_HEIGHT    = 3.4   # Inspected surface should be at this height (in mm)   
DEFECT_THRESHOLD   = 0.2   # Max acceptable deviation from expected height (mm)   
SATURATED_DEFECT   = 1.0   # Deviation at which defect will appear red (in mm)    

# Radius of the smallest particles to keep. 
MIN_BLOB_RADIUS            =  3

# Pixel offset for drawing text. 
TEXT_H_OFFSET_1           = -50
TEXT_V_OFFSET_1           =  -6
TEXT_H_OFFSET_2           = -30
TEXT_V_OFFSET_2           =   6

def PerformBlobAnalysis(MilSystem, MilDisplay, MilOverlayImage, MilDepthMap):
   # Get size of depth map. 
   SizeX = MIL.MbufInquire(MilDepthMap, MIL.M_SIZE_X)
   SizeY = MIL.MbufInquire(MilDepthMap, MIL.M_SIZE_Y)

   # Allocate a binary image buffer for fast processing. 
   MilBinImage = MIL.MbufAlloc2d(MilSystem, SizeX, SizeY,  1 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC)

   # Binarize image. 
   DefectThreshold = (EXPECTED_HEIGHT - DEFECT_THRESHOLD) * SCALE_FACTOR
   MIL.MimBinarize(MilDepthMap, MilBinImage, MIL.M_FIXED + MIL.M_LESS_OR_EQUAL, DefectThreshold, MIL.M_NULL)

   # Remove small particles. 
   MIL.MimOpen(MilBinImage, MilBinImage, MIN_BLOB_RADIUS, MIL.M_BINARY)

   # Allocate a blob context. *
   MilBlobContext = MIL.MblobAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT)
  
   # Enable the Center Of Gravity and Min Pixel features calculation. *
   MIL.MblobControl(MilBlobContext, MIL.M_CENTER_OF_GRAVITY + MIL.M_GRAYSCALE, MIL.M_ENABLE)
   MIL.MblobControl(MilBlobContext, MIL.M_MIN_PIXEL, MIL.M_ENABLE)
 
   # Allocate a blob result buffer. 
   MilBlobResult = MIL.MblobAllocResult(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT)
 
   # Calculate selected features for each blob. *
   MIL.MblobCalculate(MilBlobContext, MilBinImage, MilDepthMap, MilBlobResult)
 
   # Get the total number of selected blobs. *

   TotalBlobs = MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT)

   print("Number of defects: {TotalBlobs}".format(TotalBlobs=TotalBlobs))

   # Read and print the blob characteristics. *
   # Get the results. 
   CogX = MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_CENTER_OF_GRAVITY_X + MIL.M_GRAYSCALE)
   CogY = MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_CENTER_OF_GRAVITY_Y + MIL.M_GRAYSCALE)
   MinPixels = MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_MIN_PIXEL + MIL.M_TYPE_MIL_INT)

   # Draw the defects. 
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED)
   MIL.MblobDraw(MIL.M_DEFAULT, MilBlobResult, MilOverlayImage,
               MIL.M_DRAW_BLOBS, MIL.M_INCLUDED_BLOBS, MIL.M_DEFAULT)
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_WHITE)

   # Print the depth of each blob. 
   if TotalBlobs > 1:
      for n in range(TotalBlobs):
         # Write the depth of the defect in the overlay. 
         DepthOfDefect = EXPECTED_HEIGHT - (MinPixels[n]/SCALE_FACTOR)
         DepthString = "{DepthOfDefect:.2f} mm".format(DepthOfDefect=DepthOfDefect)
         print("Defect #{n}: depth = {DepthOfDefect:.2f} mm\n".format(n=n, DepthOfDefect=DepthOfDefect))
         MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, CogX[n] + TEXT_H_OFFSET_1,
                                          CogY[n] + TEXT_V_OFFSET_1, "Defect depth")
         MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, CogX[n] + TEXT_H_OFFSET_2,
                                                      CogY[n] + TEXT_V_OFFSET_2, DepthString)
   else:
      DepthOfDefect = EXPECTED_HEIGHT - (MinPixels/SCALE_FACTOR)
      DepthString = "{DepthOfDefect:.2f} mm".format(DepthOfDefect=DepthOfDefect)
      print("Defect #0: depth = {DepthOfDefect:.2f} mm\n".format(DepthOfDefect=DepthOfDefect))
      MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, CogX + TEXT_H_OFFSET_1,
                                       CogY + TEXT_V_OFFSET_1, "Defect depth")
      MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, CogX + TEXT_H_OFFSET_2,
                                                   CogY + TEXT_V_OFFSET_2, DepthString)

   # Free all allocations. 
   MIL.MblobFree(MilBlobResult)
   MIL.MblobFree(MilBlobContext)
   MIL.MbufFree(MilBinImage)

# Color constants for display LUT. 
BLUE_HUE = 171.0           # Expected depths will be blue.   
RED_HUE  = 0.0             # Worst defects will be red.      
FULL_SATURATION = 255      # All colors are fully saturated. 
HALF_LUMINANCE  = 128      # All colors have half luminance. 

# Creates a color display LUT to show defects in red. 
def SetupColorDisplay(MilSystem, MilDisplay, SizeBit):
   # Number of possible gray levels in corrected depth map. 
   NbGrayLevels = 1 << SizeBit

   # Allocate 1-band LUT that will contain hue values. 
   MilRampLut1Band = MIL.MbufAlloc1d(MilSystem, NbGrayLevels, 8 + MIL.M_UNSIGNED, MIL.M_LUT)

   # Compute limit gray values. 
   DefectGrayLevel   = int((EXPECTED_HEIGHT - SATURATED_DEFECT) * SCALE_FACTOR)
   ExpectedGrayLevel = int(EXPECTED_HEIGHT * SCALE_FACTOR)

   # Create hue values for each possible gray level. 
   MIL.MgenLutRamp(MilRampLut1Band, 0, RED_HUE, DefectGrayLevel, RED_HUE)
   MIL.MgenLutRamp(MilRampLut1Band, DefectGrayLevel, RED_HUE, ExpectedGrayLevel, BLUE_HUE)
   MIL.MgenLutRamp(MilRampLut1Band, ExpectedGrayLevel, BLUE_HUE, NbGrayLevels-1, BLUE_HUE)

   # Create a HSL image buffer. 
   MilColorImage = MIL.MbufAllocColor(MilSystem, 3, NbGrayLevels, 1, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE)
   MIL.MbufClear(MilColorImage, MIL.M_RGB888(0, FULL_SATURATION, HALF_LUMINANCE))

   # Set its H band (hue) to the LUT contents and convert the image to RGB. 
   MIL.MbufCopyColor2d(MilRampLut1Band, MilColorImage, 0, 0, 0, 0, 0, 0, NbGrayLevels, 1)
   MIL.MimConvert(MilColorImage, MilColorImage, MIL.M_HSL_TO_RGB)

   # Create RGB LUT to give to display and copy image contents. 
   MilRampLut3Band = MIL.MbufAllocColor(MilSystem, 3, NbGrayLevels, 1, 8 + MIL.M_UNSIGNED, MIL.M_LUT)
   MIL.MbufCopy(MilColorImage, MilRampLut3Band)

   # Associates LUT to display. 
   MIL.MdispLut(MilDisplay, MilRampLut3Band)

   # Free all allocations. 
   MIL.MbufFree(MilRampLut1Band)
   MIL.MbufFree(MilRampLut3Band)
   MIL.MbufFree(MilColorImage)
   
#***************************************************************************
# Calibrated camera example.
#***************************************************************************

# Input sequence specifications. 
GRID_FILENAME              =  MIL.M_IMAGE_PATH + "GridForLaser.mim"
LASERLINE_FILENAME         =  MIL.M_IMAGE_PATH + "LaserLine.mim"
OBJECT2_SEQUENCE_FILE      =  MIL.M_IMAGE_PATH + "Cookie.avi"

# Camera calibration grid parameters. 
GRID_NB_ROWS           =  13
GRID_NB_COLS           =  12
GRID_ROW_SPACING       =  5.0     # in mm                
GRID_COL_SPACING       =  5.0     # in mm                

# Laser device setup parameters. 
CONVEYOR_SPEED         =  -0.2     # in mm/frame          

# Fully corrected depth map generation parameters. 
DEPTH_MAP_SIZE_X       =  480      # in pixels            
DEPTH_MAP_SIZE_Y       =  480      # in pixels            
GAP_DEPTH              =  1.5      # in mm                

# Peak detection parameters. 
PEAK_WIDTH_NOMINAL_2      =  9
PEAK_WIDTH_DELTA_2        =  7
MIN_CONTRAST_2            = 75

# Everything below this is considered as noise. 
MIN_HEIGHT_THRESHOLD = 1.0 # in mm 

def CalibratedCameraExample(MilSystem, MilDisplay):
   print("\n3D PROFILING AND VOLUME ANALYSIS:")
   print("---------------------------------\n")
   print("This program generates fully corrected 3D data of a")
   print("scanned cookie and computes its volume.")
   print("The laser (sheet-of-light) profiling system uses a")
   print("3d-calibrated camera.\n")

   # Load grid image for camera calibration.
   MilImage = MIL.MbufRestore(GRID_FILENAME, MilSystem)

   # Select display.
   MIL.MdispSelect(MilDisplay, MilImage)

   print("Calibrating the camera...\n")

   SizeX = MIL.MbufInquire(MilImage, MIL.M_SIZE_X)
   SizeY = MIL.MbufInquire(MilImage, MIL.M_SIZE_Y)

   # Allocate calibration context in 3D mode.
   MilCalibration = MIL.McalAlloc(MilSystem, MIL.M_TSAI_BASED, MIL.M_DEFAULT)

   # Calibrate the camera.
   MIL.McalGrid(MilCalibration, MilImage, 0.0, 0.0, 0.0, GRID_NB_ROWS, GRID_NB_COLS,
            GRID_ROW_SPACING, GRID_COL_SPACING, MIL.M_DEFAULT, MIL.M_CHESSBOARD_GRID)

   CalibrationStatus = MIL.McalInquire(MilCalibration, MIL.M_CALIBRATION_STATUS + MIL.M_TYPE_MIL_INT)
   
   if CalibrationStatus != MIL.M_CALIBRATED:
      MIL.McalFree(MilCalibration)
      MIL.MbufFree(MilImage)
      print("Camera calibration failed.")
      print("Press <Enter> to end.\n")
      MIL.MosGetch()
      return

   # Prepare for overlay annotations.
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MilOverlayImage = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN)

   # Draw camera calibration points.
   MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilOverlayImage, MIL.M_DRAW_IMAGE_POINTS,
                                                                     MIL.M_DEFAULT, MIL.M_DEFAULT)

   print("The camera was calibrated using a chessboard grid.\n")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Disable overlay.
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_DISABLE)

   # Load laser line image.
   MIL.MbufLoad(LASERLINE_FILENAME, MilImage)

   # Allocate 3dmap objects.
   MilLaser = MIL.M3dmapAlloc(MilSystem, MIL.M_LASER, MIL.M_CALIBRATED_CAMERA_LINEAR_MOTION)
   MilCalibScan = MIL.M3dmapAllocResult(MilSystem, MIL.M_LASER_CALIBRATION_DATA, MIL.M_DEFAULT)

   # Set laser line extraction options.
   MilPeakLocator = MIL.M3dmapInquire(MilLaser, MIL.M_DEFAULT, MIL.M_LOCATE_PEAK_1D_CONTEXT_ID + MIL.M_TYPE_MIL_ID)
   MIL.MimControl(MilPeakLocator, MIL.M_PEAK_WIDTH_NOMINAL, PEAK_WIDTH_NOMINAL_2)
   MIL.MimControl(MilPeakLocator, MIL.M_PEAK_WIDTH_DELTA  , PEAK_WIDTH_DELTA_2  )
   MIL.MimControl(MilPeakLocator, MIL.M_MINIMUM_CONTRAST  , MIN_CONTRAST_2      )

   # Calibrate laser profiling context.
   MIL.M3dmapAddScan(MilLaser, MilCalibScan, MilImage, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MIL.M3dmapCalibrate(MilLaser, MilCalibScan, MilCalibration, MIL.M_DEFAULT)

   print("The laser profiling system has been calibrated using the image")
   print("of one laser line.\n")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Free the result buffer use for calibration as it will not be used anymore.
   MIL.M3dmapFree(MilCalibScan)
   MilCalibScan = MIL.M_NULL

   # Allocate the result buffer to hold the scanned 3D points.
   MilScan = MIL.M3dmapAllocResult(MilSystem, MIL.M_POINT_CLOUD_RESULT, MIL.M_DEFAULT)

   # Set speed of scanned object (speed in mm/frame is constant).
   MIL.M3dmapControl(MilLaser, MIL.M_DEFAULT, MIL.M_SCAN_SPEED, CONVEYOR_SPEED)

   # Inquire characteristics of the input sequence.
   NumberOfImages = MIL.MbufDiskInquire(OBJECT2_SEQUENCE_FILE, MIL.M_NUMBER_OF_IMAGES)
   FrameRate = MIL.MbufDiskInquire(OBJECT2_SEQUENCE_FILE, MIL.M_FRAME_RATE)

   # Open the object sequence file for reading.
   MIL.MbufImportSequence(OBJECT2_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                           MIL.M_NULL, MIL.M_OPEN)

   print("The cookie is being scanned to generate 3D data.\n")

   # Read and process all images in the input sequence.
   StartTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)

   for n in range(NumberOfImages):
      # Read image from sequence. 
      MIL.MbufImportSequence(OBJECT2_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_LOAD, MIL.M_NULL, MilImage,
                                                                     MIL.M_DEFAULT, 1, MIL.M_READ)

      # Analyze the image to extract laser line and correct its depth. 
      MIL.M3dmapAddScan(MilLaser, MilScan, MilImage, MIL.M_NULL, MIL.M_NULL, MIL.M_POINT_CLOUD_LABEL(1), MIL.M_DEFAULT)

      # Wait to have a proper frame rate, if necessary. 
      EndTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)
      WaitTime = (1.0/FrameRate) - (EndTime - StartTime)
      if WaitTime > 0:
         MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_WAIT, WaitTime)
      StartTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)

   # Close the object sequence file. 
   MIL.MbufImportSequence(OBJECT2_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                          MIL.M_NULL, MIL.M_CLOSE)

   # Convert to M_CONTAINER for 3D processing. 
   MilContainerId = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC | MIL.M_DISP, MIL.M_DEFAULT)
   MIL.M3dmapCopyResult(MilScan, MIL.M_ALL, MilContainerId, MIL.M_POINT_CLOUD_UNORGANIZED, MIL.M_DEFAULT)

   # The container's reflectance is 16bits, but only uses the bottom 8.Set the maximum value to display it properly.
   MIL.MbufControlContainer(MilContainerId, MIL.M_COMPONENT_REFLECTANCE, MIL.M_MAX, 255)

   # Allocate image for the fully corrected depth map. 
   MilDepthMap = MIL.MbufAlloc2d(MilSystem, DEPTH_MAP_SIZE_X, DEPTH_MAP_SIZE_Y, 16 + MIL.M_UNSIGNED,
               MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP)

   # Include all points during depth map generation. 
   MIL.M3dimCalibrateDepthMap(MilContainerId, MilDepthMap, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_NEGATIVE, MIL.M_DEFAULT)

   # Remove noise in the container close to the Z = 0. 
   MilPlane = MIL.M3dgeoAlloc(MilSystem, MIL.M_GEOMETRY, MIL.M_DEFAULT)
   MIL.M3dgeoPlane(MilPlane, MIL.M_COEFFICIENTS, 0.0, 0.0, 1.0, MIN_HEIGHT_THRESHOLD, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # M_INVERSE remove what is above the plane. 
   MIL.M3dimCrop(MilContainerId, MilContainerId, MilPlane, MIL.M_NULL, MIL.M_SAME, MIL.M_INVERSE)
   MIL.M3dgeoFree(MilPlane)

   print("Fully corrected 3D data of the cookie is displayed.\n")

   M3dDisplay = Alloc3dDisplayId(MilSystem)
   if M3dDisplay:
      print("Press <R> on the display window to stop/start the rotation.\n")
      MIL.M3ddispSelect(M3dDisplay, MilContainerId, MIL.M_SELECT, MIL.M_DEFAULT)
      MIL.M3ddispSetView(M3dDisplay, MIL.M_AUTO, MIL.M_BOTTOM_TILTED, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
      MIL.M3ddispControl(M3dDisplay, MIL.M_AUTO_ROTATE, MIL.M_ENABLE)

   # Get fully corrected depth map from accumulated information in the result buffer. 
   MIL.M3dimProject(MilContainerId, MilDepthMap, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_MIN_Z, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Set fill gaps parameters. 
   FillGapsContext = MIL.M3dimAlloc(MilSystem, MIL.M_FILL_GAPS_CONTEXT, MIL.M_DEFAULT)
   MIL.M3dimControl(FillGapsContext, MIL.M_FILL_MODE,                  MIL.M_X_THEN_Y)
   MIL.M3dimControl(FillGapsContext, MIL.M_FILL_SHARP_ELEVATION,       MIL.M_MIN)
   MIL.M3dimControl(FillGapsContext, MIL.M_FILL_SHARP_ELEVATION_DEPTH, GAP_DEPTH)
   MIL.M3dimControl(FillGapsContext, MIL.M_FILL_BORDER,                MIL.M_DISABLE)

   MIL.M3dimFillGaps(FillGapsContext, MilDepthMap, MIL.M_NULL, MIL.M_DEFAULT)

   # Compute the volume of the depth map. 
   Volume, StatusPtr = MIL.M3dmetVolume(MilDepthMap, MIL.M_XY_PLANE, MIL.M_TOTAL,  MIL.M_DEFAULT)

   print("Volume of the cookie is {Volume:.1f} cm^3.\n".format(Volume=(Volume / 1000.0)))
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free all allocations. 
   if (M3dDisplay):
      MIL.M3ddispFree(M3dDisplay)
   MIL.M3dimFree(FillGapsContext)
   MIL.MbufFree(MilContainerId)
   MIL.M3dmapFree(MilScan)
   MIL.M3dmapFree(MilLaser)
   MIL.McalFree(MilCalibration)
   MIL.MbufFree(MilDepthMap)
   MIL.MbufFree(MilImage)

#*****************************************************************************
# Allocates a 3D display and returns its MIL identifier.  
#*****************************************************************************
def Alloc3dDisplayId(MilSystem):
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
   MilDisplay3D = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, MIL.M_NULL)
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)

   if not MilDisplay3D:
      print("\nThe current system does not support the 3D display.\n")
   return MilDisplay3D


if __name__ == "__main__":
   M3dMapExample()
