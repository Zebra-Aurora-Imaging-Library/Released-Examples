#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#*****************************************************************************
#
# File name: MCal.py
#
# Synopsis:  This program uses the Calibration module to:
#              - Remove distortion and then take measurements in world units using a 2D 
#                calibration.
#              - Perform a 3D calibration to take measurements at several known elevations.
#              - Calibrate a scene using a partial calibration grid that has a 2D code 
#                fiducial.
#
# Printable calibration grids in PDF format can be found in your
# "Matrox Imaging/Images/" directory.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved


import mil as MIL

# Grid offset specifications. 
GRID_OFFSET_X          = 0
GRID_OFFSET_Y          = 0
GRID_OFFSET_Z          = 0

#  Main function. 
def McalExample():
   
   # Allocate defaults. 
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Print module name. 
   print("CALIBRATION MODULE:")
   print("-------------------\n\n")

   LinearInterpolationCalibration(MilSystem, MilDisplay)

   TsaiCalibration(MilSystem, MilDisplay)

   PartialGridCalibration(MilSystem, MilDisplay)

   # Free defaults.     
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0
   

#****************************************************************************
# Linear interpolation example. 
#****************************************************************************

# Source image files specification. 
GRID_IMAGE_FILE        = MIL.M_IMAGE_PATH + "CalGrid.mim"
BOARD_IMAGE_FILE       = MIL.M_IMAGE_PATH + "CalBoard.mim"

# World description of the calibration grid. 
GRID_ROW_SPACING       = 1
GRID_COLUMN_SPACING    = 1
GRID_ROW_NUMBER        = 18
GRID_COLUMN_NUMBER     = 25

# Measurement boxes specification. 
MEAS_BOX_POS_X1        = 55
MEAS_BOX_POS_Y1        = 24
MEAS_BOX_WIDTH1        = 7
MEAS_BOX_HEIGHT1       = 425

MEAS_BOX_POS_X2        = 225
MEAS_BOX_POS_Y2        = 11
MEAS_BOX_WIDTH2        = 7
MEAS_BOX_HEIGHT2       = 450

# Specification of the stripes' constraints. 
WIDTH_APPROXIMATION    = 410
WIDTH_VARIATION        = 25
MIN_EDGE_VALUE         = 5

def LinearInterpolationCalibration(MilSystem, MilDisplay):
                 
   # Clear the display 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)

   # Restore source image into an automatically allocated image buffer. 
   MilImage = MIL.MbufRestore(GRID_IMAGE_FILE, MilSystem)

   # Display the image buffer. 
   MIL.MdispSelect(MilDisplay, MilImage)

   # Prepare for overlay annotation. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MilOverlayImage = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)

   # Pause to show the original image. 
   print("LINEAR INTERPOLATION CALIBRATION:")
   print("---------------------------------\n")
   print("The displayed grid has been grabbed with a high distortion")
   print("camera and will be used to calibrate the camera.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Allocate a camera calibration context. 
   MilCalibration = MIL.McalAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Calibrate the camera with the image of the grid and its world description. 
   MIL.McalGrid(MilCalibration, MilImage,
            GRID_OFFSET_X, GRID_OFFSET_Y, GRID_OFFSET_Z,
            GRID_ROW_NUMBER, GRID_COLUMN_NUMBER,
            GRID_ROW_SPACING, GRID_COLUMN_SPACING,
            MIL.M_DEFAULT, MIL.M_DEFAULT)

   CalibrationStatus = MIL.McalInquire(MilCalibration, MIL.M_CALIBRATION_STATUS + MIL.M_TYPE_MIL_INT)
   if (CalibrationStatus == MIL.M_CALIBRATED):
      
      # Perform a first image transformation with the calibration grid. 
      MIL.McalTransformImage(MilImage, MilImage, MilCalibration,
                         MIL.M_BILINEAR + MIL.M_OVERSCAN_CLEAR, MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Pause to show the corrected image of the grid. 
      print("The camera has been calibrated and the image of the grid")
      print("has been transformed to remove its distortions.")
      print("Press <Enter> to continue.\n")
      MIL.MosGetch()

      # Read the image of the board and associate the calibration to the image. 
      MIL.MbufLoad(BOARD_IMAGE_FILE, MilImage)
      MIL.McalAssociate(MilCalibration, MilImage, MIL.M_DEFAULT)

      # Allocate the measurement markers. 
      MeasMarker1 = MIL.MmeasAllocMarker(MilSystem, MIL.M_STRIPE, MIL.M_DEFAULT)
      MeasMarker2 = MIL.MmeasAllocMarker(MilSystem, MIL.M_STRIPE, MIL.M_DEFAULT)

      # Set the markers' measurement search region. 
      MIL.MmeasSetMarker(MeasMarker1, MIL.M_BOX_ORIGIN, MEAS_BOX_POS_X1, MEAS_BOX_POS_Y1)
      MIL.MmeasSetMarker(MeasMarker1, MIL.M_BOX_SIZE, MEAS_BOX_WIDTH1, MEAS_BOX_HEIGHT1)
      MIL.MmeasSetMarker(MeasMarker2, MIL.M_BOX_ORIGIN, MEAS_BOX_POS_X2, MEAS_BOX_POS_Y2)
      MIL.MmeasSetMarker(MeasMarker2, MIL.M_BOX_SIZE, MEAS_BOX_WIDTH2, MEAS_BOX_HEIGHT2)

      # Set markers' orientation. 
      MIL.MmeasSetMarker(MeasMarker1, MIL.M_ORIENTATION, MIL.M_HORIZONTAL, MIL.M_NULL)
      MIL.MmeasSetMarker(MeasMarker2, MIL.M_ORIENTATION, MIL.M_HORIZONTAL, MIL.M_NULL)

      # Set markers' settings to locate the largest stripe within the range
      #   [WIDTH_APPROXIMATION - WIDTH_VARIATION,
      #    WIDTH_APPROXIMATION + WIDTH_VARIATION],
      #   and with an edge strength over MIN_EDGE_VALUE. 
      MIL.MmeasSetMarker(MeasMarker1, MIL.M_EDGEVALUE_MIN, MIN_EDGE_VALUE, MIL.M_NULL)

      # Remove the default strength characteristic score MIL.Mapping. 
      MIL.MmeasSetScore(MeasMarker1, MIL.M_STRENGTH_SCORE,
                                 0.0,
                                 0.0,
                                 MIL.M_MAX_POSSIBLE_VALUE,
                                 MIL.M_MAX_POSSIBLE_VALUE,
                                 MIL.M_DEFAULT,
                                 MIL.M_DEFAULT,
                                 MIL.M_DEFAULT)

      # Add a width characteristic score MIL.Mapping (increasing ramp)
      # to find the largest stripe within a given range.

      # Width score MIL.Mapping to find the largest stripe within a given
      # width range ]Wmin, Wmax]:

      #    Score
      #       ^
      #       |         /|
      #       |       /  |
      #       |     /    |
      #       +---------------> Width
      #           Wmin  Wmax
      
      MIL.MmeasSetScore(MeasMarker1, MIL.M_STRIPE_WIDTH_SCORE,
                                 WIDTH_APPROXIMATION - WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 MIL.M_DEFAULT,
                                 MIL.M_PIXEL,
                                 MIL.M_DEFAULT)

      # Set the same settings for the second marker. 
      MIL.MmeasSetMarker(MeasMarker2, MIL.M_EDGEVALUE_MIN, MIN_EDGE_VALUE, MIL.M_NULL)

      MIL.MmeasSetScore(MeasMarker2, MIL.M_STRENGTH_SCORE,
                                 0.0,
                                 0.0,
                                 MIL.M_MAX_POSSIBLE_VALUE,
                                 MIL.M_MAX_POSSIBLE_VALUE,
                                 MIL.M_DEFAULT,
                                 MIL.M_DEFAULT,
                                 MIL.M_DEFAULT)

      MIL.MmeasSetScore(MeasMarker2, MIL.M_STRIPE_WIDTH_SCORE,
                                 WIDTH_APPROXIMATION - WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 MIL.M_DEFAULT,
                                 MIL.M_PIXEL,
                                 MIL.M_DEFAULT)

      # Find and measure the position and width of the board. 
      MIL.MmeasFindMarker(MIL.M_DEFAULT, MilImage, MeasMarker1, MIL.M_STRIPE_WIDTH+MIL.M_POSITION)
      MIL.MmeasFindMarker(MIL.M_DEFAULT, MilImage, MeasMarker2, MIL.M_STRIPE_WIDTH+MIL.M_POSITION)

      # Get the world width of the two markers. 
      WorldDistance1 = MIL.MmeasGetResult(MeasMarker1, MIL.M_STRIPE_WIDTH, None, MIL.M_NULL)
      WorldDistance2 = MIL.MmeasGetResult(MeasMarker2, MIL.M_STRIPE_WIDTH, None, MIL.M_NULL)

      # Get the pixel width of the two markers. 
      MIL.MmeasSetMarker(MeasMarker1, MIL.M_RESULT_OUTPUT_UNITS, MIL.M_PIXEL, MIL.M_NULL)
      MIL.MmeasSetMarker(MeasMarker2, MIL.M_RESULT_OUTPUT_UNITS, MIL.M_PIXEL, MIL.M_NULL)
      PixelDistance1 = MIL.MmeasGetResult(MeasMarker1, MIL.M_STRIPE_WIDTH, None, MIL.M_NULL)
      PixelDistance2 = MIL.MmeasGetResult(MeasMarker2, MIL.M_STRIPE_WIDTH, None, MIL.M_NULL)

      # Get the edges position in pixel to draw the annotations. 
      PosX1, PosY1 = MIL.MmeasGetResult(MeasMarker1, MIL.M_POSITION+MIL.M_EDGE_FIRST)
      PosX2, PosY2 = MIL.MmeasGetResult(MeasMarker1, MIL.M_POSITION+MIL.M_EDGE_SECOND)
      PosX3, PosY3 = MIL.MmeasGetResult(MeasMarker2, MIL.M_POSITION+MIL.M_EDGE_FIRST)
      PosX4, PosY4 = MIL.MmeasGetResult(MeasMarker2, MIL.M_POSITION+MIL.M_EDGE_SECOND)

      # Draw the measurement indicators on the image.  
      MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_YELLOW)
      MIL.MmeasDraw(MIL.M_DEFAULT, MeasMarker1, MilOverlayImage, MIL.M_DRAW_WIDTH, MIL.M_DEFAULT, MIL.M_RESULT)
      MIL.MmeasDraw(MIL.M_DEFAULT, MeasMarker2, MilOverlayImage, MIL.M_DRAW_WIDTH, MIL.M_DEFAULT, MIL.M_RESULT)

      MIL.MgraBackColor(MIL.M_DEFAULT, MIL.M_COLOR_BLACK)
      MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, int(PosX1[0]+0.5-40), int((PosY1[0]+0.5)+((PosY2[0] - PosY1[0])/2.0)), " Distance 1 ")
      MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, int(PosX3[0]+0.5-40), int((PosY3[0]+0.5)+((PosY4[0] - PosY3[0])/2.0)), " Distance 2 ")

      # Pause to show the original image and the measurement results. 
      print("A distorted image grabbed with the same camera was loaded and")
      print("calibrated measurements were done to evaluate the board dimensions.\n")
      print("========================================================")
      print("                      Distance 1          Distance 2 ")
      print("--------------------------------------------------------")
      print(" Calibrated unit:   {:8.2f} cm           {:6.2f} cm    ".format(WorldDistance1[0], WorldDistance2[0]))
      print(" Uncalibrated unit: {:8.2f} pixels       {:6.2f} pixels".format(PixelDistance1[0], PixelDistance2[0]))
      print("========================================================\n")
      print("Press <Enter> to continue.\n")
      MIL.MosGetch()

      # Clear the display overlay. 
      MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)

      # Read the image of the PCB. 
      MIL.MbufLoad(BOARD_IMAGE_FILE, MilImage)

      # Transform the image of the board. 
      MIL.McalTransformImage(MilImage, MilImage, MilCalibration, MIL.M_BILINEAR+MIL.M_OVERSCAN_CLEAR, MIL.M_DEFAULT, MIL.M_DEFAULT)

      # show the transformed image of the board. 
      print("The image was corrected to remove its distortions.")

      # Free measurement markers. 
      MIL.MmeasFree(MeasMarker1)
      MIL.MmeasFree(MeasMarker2)
      
   else:
      
      print("Calibration generated an exception.")
      print("See User Guide to resolve the situation.\n")
      

   # Wait for a key to be pressed.  
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Free all allocations. 
   MIL.McalFree(MilCalibration)
   MIL.MbufFree(MilImage)
   


#****************************************************************************
#Tsai example. 
#****************************************************************************

# Source image files specification. 
GRID_ORIGINAL_IMAGE_FILE     = MIL.M_IMAGE_PATH + "CalGridOriginal.mim"
OBJECT_ORIGINAL_IMAGE_FILE   = MIL.M_IMAGE_PATH + "CalObjOriginal.mim"
OBJECT_MOVED_IMAGE_FILE      = MIL.M_IMAGE_PATH + "CalObjMoved.mim"

# World description of the calibration grid. 
GRID_ORG_ROW_SPACING        = 1.5
GRID_ORG_COLUMN_SPACING     = 1.5
GRID_ORG_ROW_NUMBER         = 12
GRID_ORG_COLUMN_NUMBER      = 13
GRID_ORG_OFFSET_X           = 0
GRID_ORG_OFFSET_Y           = 0 
GRID_ORG_OFFSET_Z           = 0

# Region parameters for metrology 
MEASURED_CIRCLE_LABEL      = 1
RING1_POS_X                = 2.3
RING1_POS_Y                = 3.9
RING2_POS_X                = 10.7
RING2_POS_Y                = 11.1

RING_START_RADIUS          = 1.25
RING_END_RADIUS            = 2.3

# measured plane position 
RING_THICKNESS             = 0.175
STEP_THICKNESS             = 4.0

# Color definitions 
ABSOLUTE_COLOR   = MIL.M_RGB888(255,0,0)
RELATIVE_COLOR   = MIL.M_RGB888(0,255,0)
REGION_COLOR     = MIL.M_RGB888(0,100,255)
FEATURE_COLOR    = MIL.M_RGB888(255,0,255)


def TsaiCalibration(MilSystem, MilDisplay):        

   # Restore source image into an automatically allocated image buffer. 
   MilImage = MIL.MbufRestore(GRID_ORIGINAL_IMAGE_FILE, MilSystem)

   # Display the image buffer. 
   MIL.MdispSelect(MilDisplay, MilImage)
   # Prepare for overlay annotation. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MilOverlayImage = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)

   # Pause to show the original image. 
   print("TSAI BASED CALIBRATION:")
   print("-----------------------\n")
   print("The displayed grid has been grabbed with a high perspective")
   print("camera and will be used to calibrate the camera.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Allocate a camera calibration context. 
   MilCalibration = MIL.McalAlloc(MilSystem, MIL.M_TSAI_BASED, MIL.M_DEFAULT)

   # Calibrate the camera with the image of the grid and its world description. 
   MIL.McalGrid(MilCalibration, MilImage,
            GRID_ORG_OFFSET_X, GRID_ORG_OFFSET_Y, GRID_ORG_OFFSET_Z,
            GRID_ORG_ROW_NUMBER, GRID_ORG_COLUMN_NUMBER,
            GRID_ORG_ROW_SPACING, GRID_ORG_COLUMN_SPACING,
            MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Verify if the camera calibration is successful. 
   CalibrationStatus = MIL.McalInquire(MilCalibration, MIL.M_CALIBRATION_STATUS + MIL.M_TYPE_MIL_INT)
   if (CalibrationStatus == MIL.M_CALIBRATED):
      
      # Display the world absolute coordinate system 
      MIL.MgraColor(MIL.M_DEFAULT, ABSOLUTE_COLOR)
      MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilOverlayImage, MIL.M_DRAW_ABSOLUTE_COORDINATE_SYSTEM+MIL.M_DRAW_AXES, MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Print camera information 
      print("The camera has been calibrated.")
      print("The world absolute coordinate system is shown in red.\n")
      ShowCameraInformation(MilCalibration)

      # Load source image into an image buffer. 
      MIL.MbufLoad(OBJECT_ORIGINAL_IMAGE_FILE, MilImage)

      # Associate the calibration to the image 
      MIL.McalAssociate(MilCalibration, MilImage, MIL.M_DEFAULT)
      
      # Set the offset of the camera calibration plane. 
      # This moves the relative origin at the top of the first metallic part 
      MIL.McalSetCoordinateSystem(MilImage,
         MIL.M_RELATIVE_COORDINATE_SYSTEM,
         MIL.M_ABSOLUTE_COORDINATE_SYSTEM,
         MIL.M_TRANSLATION + MIL.M_ASSIGN,
         MIL.M_NULL,
         0, 0, -RING_THICKNESS,
         MIL.M_DEFAULT)

      # Display the world relative coordinate system 
      MIL.MgraColor(MIL.M_DEFAULT, RELATIVE_COLOR)
      MIL.McalDraw(MIL.M_DEFAULT, MilImage, MilOverlayImage, MIL.M_DRAW_RELATIVE_COORDINATE_SYSTEM + MIL.M_DRAW_FRAME, MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Measure the first circle. 
      print("The relative coordinate system (shown in green) was translated by {:.3f} cm".format(-RING_THICKNESS))
      print("in z to align it with the top of the first metallic part.")
      MeasureRing(MilSystem, MilOverlayImage, MilImage, RING1_POS_X, RING1_POS_Y )
      print("Press <Enter> to continue.\n")
      MIL.MosGetch()

      # Modify the offset of the camera calibration plane 
      # This moves the relative origin at the top of the second metallic part  
      MIL.McalSetCoordinateSystem(MilImage,
         MIL.M_RELATIVE_COORDINATE_SYSTEM,
         MIL.M_ABSOLUTE_COORDINATE_SYSTEM,
         MIL.M_TRANSLATION + MIL.M_COMPOSE_WITH_CURRENT,
         MIL.M_NULL,
         0, 0, -STEP_THICKNESS,
         MIL.M_DEFAULT)

      # Clear the overlay 
      MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)
      # Display the world absolute coordinate system 
      MIL.MgraColor(MIL.M_DEFAULT, ABSOLUTE_COLOR)
      MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilOverlayImage, MIL.M_DRAW_ABSOLUTE_COORDINATE_SYSTEM + MIL.M_DRAW_AXES, MIL.M_DEFAULT, MIL.M_DEFAULT)
      # Display the world relative coordinate system 
      MIL.MgraColor(MIL.M_DEFAULT, RELATIVE_COLOR)
      MIL.McalDraw(MIL.M_DEFAULT, MilImage, MilOverlayImage, MIL.M_DRAW_RELATIVE_COORDINATE_SYSTEM + MIL.M_DRAW_FRAME, MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Measure the second circle. 
      print("The relative coordinate system was translated by another {:.3f} cm".format(-STEP_THICKNESS))
      print("in z to align it with the top of the second metallic part.")
      MeasureRing(MilSystem, MilOverlayImage, MilImage, RING2_POS_X, RING2_POS_Y )
      print("Press <Enter> to continue.\n")
      MIL.MosGetch()
      
   else:
      
      print("Calibration generated an exception.")
      print("See User Guide to resolve the situation.\n")
      

   # Free all allocations. 
   MIL.McalFree(MilCalibration)
   MIL.MbufFree(MilImage)
   

# Measuring function with MilMetrology module 
def MeasureRing(MilSystem, MilOverlayImage, MilImage, MeasureRingX, MeasureRingY):

   # Allocate metrology context and result. 
   MilMetrolContext = MIL.MmetAlloc(MilSystem, MIL.M_DEFAULT)
   MilMetrolResult = MIL.MmetAllocResult(MilSystem, MIL.M_DEFAULT)

   # Add a first measured segment feature to context and set its search region 
   MIL.MmetAddFeature(MilMetrolContext, MIL.M_MEASURED, MIL.M_CIRCLE, MEASURED_CIRCLE_LABEL,
                  MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, 0, MIL.M_DEFAULT)

   MIL.MmetSetRegion(MilMetrolContext, MIL.M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), 
                 MIL.M_DEFAULT, MIL.M_RING,
                 MeasureRingX, MeasureRingY,
                 RING_START_RADIUS, RING_END_RADIUS,
                 MIL.M_NULL, MIL.M_NULL )

   # Calculate 
   MIL.MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, MIL.M_DEFAULT)

   # Draw region 
   MIL.MgraColor(MIL.M_DEFAULT, REGION_COLOR)
   MIL.MmetDraw(MIL.M_DEFAULT, MilMetrolResult, MilOverlayImage, MIL.M_DRAW_REGION, MIL.M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), MIL.M_DEFAULT)

   # Draw the circle 
   MIL.MgraColor(MIL.M_DEFAULT, FEATURE_COLOR)
   MIL.MmetDraw(MIL.M_DEFAULT, MilMetrolResult, MilOverlayImage, MIL.M_DRAW_FEATURE, MIL.M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), MIL.M_DEFAULT)

   Value = MIL.MmetGetResult(MilMetrolResult, MIL.M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), MIL.M_RADIUS)
   print("The large circle's radius was measured: {:.3f} cm.".format(Value))

   # Free all allocations. 
   MIL.MmetFree(MilMetrolResult)
   MIL.MmetFree(MilMetrolContext)
   

# Print the current camera position and orientation  
def ShowCameraInformation(MilCalibration):

   CameraPosX, CameraPosY,CameraPosZ = MIL.McalGetCoordinateSystem(MilCalibration,
                                             MIL.M_CAMERA_COORDINATE_SYSTEM,
                                             MIL.M_ABSOLUTE_COORDINATE_SYSTEM,
                                             MIL.M_TRANSLATION,
                                             MIL.M_NULL,
                                             None, None, None, 
                                             MIL.M_NULL)

   CameraYaw, CameraPitch, CameraRoll = MIL.McalGetCoordinateSystem(MilCalibration,
                                             MIL.M_CAMERA_COORDINATE_SYSTEM,
                                             MIL.M_ABSOLUTE_COORDINATE_SYSTEM,
                                             MIL.M_ROTATION_YXZ,
                                             MIL.M_NULL,
                                             None, None, None, 
                                             MIL.M_NULL)

   # Pause to show the camera position and orientation. 
   print("Camera position in cm:          (x, y, z)           " + "({:.2f}, {:.2f}, {:.2f})".format(CameraPosX, CameraPosY, CameraPosZ))
   print("Camera orientation in degrees:  (yaw, pitch, roll)  " + "({:.2f}, {:.2f}, {:.2f})".format(CameraYaw, CameraPitch, CameraRoll))
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()
   

#****************************************************************************
#Partial grid example. 
#****************************************************************************
# Source image files specification. 
PARTIAL_GRID_IMAGE_FILE        = MIL.M_IMAGE_PATH + "PartialGrid.mim"

# Definition of the region to correct. 
CORRECTED_SIZE_X          = 60.0
CORRECTED_SIZE_Y          = 50.0
CORRECTED_OFFSET_X        = -35.0
CORRECTED_OFFSET_Y        = -5.0
CORRECTED_IMAGE_SIZE_X    = 512

def PartialGridCalibration(MilSystem, MilDisplay):
            
   # Clear the display 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)

   # Allocate a graphic list and associate it to the display. 
   MilGraList = MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT)
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList)

   # Restore source image into an automatically allocated image buffer. 
   MilImage = MIL.MbufRestore(PARTIAL_GRID_IMAGE_FILE, MilSystem)
   ImageType = MIL.MbufInquire(MilImage, MIL.M_TYPE)

   # Display the image buffer. 
   MIL.MdispSelect(MilDisplay, MilImage)

   # Pause to show the partial grid image. 
   print("PARTIAL GRID CALIBRATION:")
   print("-------------------------\n")
   print("A camera will be calibrated using a rectangular grid that")
   print("is only partially visible in the camera's field of view.")
   print("The 2D code in the center is used as a fiducial to retrieve")
   print("the characteristics of the calibration grid.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Allocate the calibration object. 
   MilCalibration = MIL.McalAlloc(MilSystem, MIL.M_TSAI_BASED, MIL.M_DEFAULT)

   # Set the calibration to calibrate a partial grid with fiducial. 
   MIL.McalControl(MilCalibration, MIL.M_GRID_PARTIAL, MIL.M_ENABLE)
   MIL.McalControl(MilCalibration, MIL.M_GRID_FIDUCIAL, MIL.M_DATAMATRIX)

   # Calibrate the camera with the partial grid with fiducial. 
   MIL.McalGrid(MilCalibration, MilImage,
            GRID_OFFSET_X, GRID_OFFSET_Y, GRID_OFFSET_Z,
            MIL.M_UNKNOWN, MIL.M_UNKNOWN, MIL.M_FROM_FIDUCIAL, MIL.M_FROM_FIDUCIAL,
            MIL.M_DEFAULT, MIL.M_CHESSBOARD_GRID)

   CalibrationStatus = MIL.McalInquire(MilCalibration, MIL.M_CALIBRATION_STATUS + MIL.M_TYPE_MIL_INT)
   if(CalibrationStatus == MIL.M_CALIBRATED):
      
      # Draw the absolute coordinate system. 
      MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED)
      MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilGraList, MIL.M_DRAW_ABSOLUTE_COORDINATE_SYSTEM,
         MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Draw a box around the fiducial. 
      MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_CYAN)
      MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilGraList, MIL.M_DRAW_FIDUCIAL_BOX,
         MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Get the information of the grid read from the fiducial. 
      RowSpacing = MIL.McalInquire(MilCalibration, MIL.M_ROW_SPACING)
      ColumnSpacing = MIL.McalInquire(MilCalibration, MIL.M_COLUMN_SPACING)
      UnitName = MIL.McalInquire(MilCalibration, MIL.M_GRID_UNIT_SHORT_NAME)

      # Draw the information of the grid read from the fiducial. 
      MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED)
      MIL.MgraControl(MIL.M_DEFAULT, MIL.M_INPUT_UNITS, MIL.M_DISPLAY)
      DrawGridInfo(MilGraList, "Row spacing", RowSpacing, 0, UnitName)
      DrawGridInfo(MilGraList, "Col spacing", ColumnSpacing, 1, UnitName)

      # Pause to show the calibration result. 
      print("The camera has been calibrated.\n")
      print("The grid information read is displayed.")
      print("Press <Enter> to continue.\n")
      MIL.MosGetch()

      # Calculate the pixel size and size Y of the corrected image. 
      CorrectedPixelSize = CORRECTED_SIZE_X / CORRECTED_IMAGE_SIZE_X
      CorrectedImageSizeY = int(CORRECTED_SIZE_Y / CorrectedPixelSize)

      # Allocate the corrected image. 
      MilCorrectedImage = MIL.MbufAlloc2d(MilSystem, CORRECTED_IMAGE_SIZE_X, CorrectedImageSizeY, ImageType,
         MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP)

      # Calibrate the corrected image. 
      MIL.McalUniform(MilCorrectedImage, CORRECTED_OFFSET_X, CORRECTED_OFFSET_Y,
         CorrectedPixelSize, CorrectedPixelSize, 0.0, MIL.M_DEFAULT)

      # Correct the calibrated image. 
      MIL.McalTransformImage(MilImage, MilCorrectedImage, MilCalibration,
         MIL.M_BILINEAR + MIL.M_OVERSCAN_CLEAR, MIL.M_DEFAULT,
         MIL.M_WARP_IMAGE + MIL.M_USE_DESTINATION_CALIBRATION)

      # Select the corrected image on the display. 
      MIL.MgraClear(MIL.M_DEFAULT, MilGraList)
      MIL.MdispSelect(MilDisplay, MilCorrectedImage)

      # Pause to show the corrected image. 
      print("A sub-region of the grid was selected and transformed")
      print("to remove the distortions.")
      print("The sub-region dimensions and position are:")
      print("   Size X  : {:3.3g} {:s}".format(CORRECTED_SIZE_X, UnitName))
      print("   Size Y  : {:3.3g} {:s}".format(CORRECTED_SIZE_Y, UnitName))
      print("   Offset X: {:3.3g} {:s}".format(CORRECTED_OFFSET_X, UnitName))
      print("   Offset Y: {:3.3g} {:s}".format(CORRECTED_OFFSET_Y, UnitName))

      # Wait for a key to be pressed. 
      print("Press <Enter> to quit.\n")
      MIL.MosGetch()

      MIL.MbufFree(MilCorrectedImage)
      
   else:
      
      print("Calibration generated an exception.")
      print("See User Guide to resolve the situation.\n")
      print("Press <Enter> to quit.\n")
      MIL.MosGetch()
      

   # Free all allocations. 
   MIL.McalFree(MilCalibration)
   MIL.MbufFree(MilImage)
   MIL.MgraFree(MilGraList)
   

# Definition of the parameters for the drawing of the grid info 
LINE_HEIGHT      = 16
MAX_INFO_SIZE    = 64

# Draw an information of the grid. 
def DrawGridInfo(MilGraList, InfoTag, Value, LineOffsetY, Units):
   Info = "{}: {:.3g} {}".format(InfoTag, Value, Units)
   MIL.MgraText(MIL.M_DEFAULT, MilGraList, 0, LineOffsetY * LINE_HEIGHT, Info)

if __name__ == "__main__":
   McalExample()
   