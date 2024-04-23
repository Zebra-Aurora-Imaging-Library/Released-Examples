#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#****************************************************************************
# 
# File name: MIL.Mbead.py
#
# Synopsis:  This program uses the Bead module to train a bead template
#            and then to inspect a defective bead in a target image.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
#

import mil as MIL

#***************************************************************************
# Main. 
#*****************************************************************************

def MbeadExample():
   
   # Allocate defaults. 
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)    

   # Run fixtured bead example. 
   FixturedBeadExample(MilSystem, MilDisplay)

   # Run predefined bead example. 
   PredefinedBeadExample(MilSystem, MilDisplay)

   # Free defaults.     
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0

# Utility definitions.
USER_POSITION_COLOR      = MIL.M_COLOR_RED
USER_TEMPLATE_COLOR      = MIL.M_COLOR_CYAN
TRAINED_BEAD_WIDTH_COLOR = MIL.M_RGB888(255, 128, 0)
MODEL_FINDER_COLOR       = MIL.M_COLOR_GREEN
COORDINATE_SYSTEM_COLOR  = MIL.M_RGB888(164, 164, 0)
RESULT_SEARCH_BOX_COLOR  = MIL.M_COLOR_CYAN
PASS_BEAD_WIDTH_COLOR    = MIL.M_COLOR_GREEN
PASS_BEAD_POSITION_COLOR = MIL.M_COLOR_GREEN
FAIL_NOT_FOUND_COLOR     = MIL.M_COLOR_RED
FAIL_SMALL_WIDTH_COLOR   = MIL.M_RGB888(255, 128, 0)
FAIL_EDGE_OFFSET_COLOR   = MIL.M_COLOR_RED
   
#***************************************************************************
# Fixtured 'stripe' bead example. 

# Target MIL image specifications. 
IMAGE_FILE_TRAINING = MIL.M_IMAGE_PATH + "BeadTraining.mim"
IMAGE_FILE_TARGET   = MIL.M_IMAGE_PATH + "BeadTarget.mim"

# Bead stripe training data definition. 
NUMBER_OF_TRAINING_POINTS = 13

TrainingPointsX = [180, 280, 400, 430, 455, 415, 370, 275, 185, 125, 105, 130, 180]
TrainingPointsY = [190, 215, 190, 200, 260, 330, 345, 310, 340, 305, 265, 200, 190]

# Max angle correction. 
MAX_ANGLE_CORRECTION_VALUE = 20.0

# Max offset deviation. 
MAX_DEVIATION_OFFSET_VALUE = 25.0

# Maximum negative width variation. 
WIDTH_DELTA_NEG_VALUE = 2.0

# Model region  definition. 
MODEL_ORIGIN_X = 145
MODEL_ORIGIN_Y = 115
MODEL_SIZE_X   = 275
MODEL_SIZE_Y   = 60

#***************************************************************************
# Predefined 'edge' bead example. 

# Target MIL image specifications. 
CAP_FILE_TARGET = MIL.M_IMAGE_PATH + "Cap.mim"

# Template attributes definition. 
CIRCLE_CENTER_X = 330.0
CIRCLE_CENTER_Y = 230.0
CIRCLE_RADIUS   = 120.0

# Edge threshold value. 
EDGE_THRESHOLD_VALUE = 25.0

# Max offset found and deviation tolerance. 
MAX_CONTOUR_DEVIATION_OFFSET = 5.0
MAX_CONTOUR_FOUND_OFFSET     = 25.0

def FixturedBeadExample(MilSystem, MilDisplay):
                                      
   # Restore source images into automatically allocated image buffers. 
   MilImageTraining = MIL.MbufRestore(IMAGE_FILE_TRAINING, MilSystem)
   MilImageTarget = MIL.MbufRestore(IMAGE_FILE_TARGET, MilSystem)

   # Display the training image buffer. 
   MIL.MdispSelect(MilDisplay, MilImageTraining)

   # Allocate a graphic list to hold the subpixel annotations to draw. 
   MilGraList = MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT)

   # Associate the graphic list to the display for annotations. 
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList)

   # Original template image. 
   print("\nFIXTURED BEAD INSPECTION:")
   print("-------------------------\n")
   print("This program performs a bead inspection on a mechanical part.")
   print("In the first step, a bead template context is trained using an image.")
   print("In the second step, a mechanical part, at an arbitrary angle and with")
   print("a defective bead, is inspected.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Allocate a MIL bead context. 
   MilBeadContext = MIL.MbeadAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Allocate a MIL bead result. 
   MilBeadResult = MIL.MbeadAllocResult(MilSystem, MIL.M_DEFAULT)

   # Add a bead template. 
   MIL.MbeadTemplate(MilBeadContext, MIL.M_ADD, MIL.M_DEFAULT, MIL.M_TEMPLATE_LABEL(1), NUMBER_OF_TRAINING_POINTS, TrainingPointsX, TrainingPointsY, MIL.M_NULL, MIL.M_DEFAULT)

   # Set template input units to world units.    
   MIL.MbeadControl(MilBeadContext, MIL.M_TEMPLATE_LABEL(1), MIL.M_TEMPLATE_INPUT_UNITS, MIL.M_WORLD)

   # Set the bead 'edge type' search properties. 
   MIL.MbeadControl(MilBeadContext, MIL.M_TEMPLATE_LABEL(1), MIL.M_ANGLE_ACCURACY_MAX_DEVIATION, MAX_ANGLE_CORRECTION_VALUE)

   # Set the maximum valid bead deformation. 
   MIL.MbeadControl(MilBeadContext, MIL.M_TEMPLATE_LABEL(1), MIL.M_OFFSET_MAX, MAX_DEVIATION_OFFSET_VALUE)

   # Set the valid bead minimum width criterion. 
   MIL.MbeadControl(MilBeadContext, MIL.M_TEMPLATE_LABEL(1), MIL.M_WIDTH_DELTA_NEG, WIDTH_DELTA_NEG_VALUE)

   # Display the bead polyline. 
   MIL.MgraColor(MIL.M_DEFAULT, USER_TEMPLATE_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadContext, MilGraList, MIL.M_DRAW_POSITION_POLYLINE, MIL.M_USER, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   # Display the bead training points. 
   MIL.MgraColor(MIL.M_DEFAULT, USER_POSITION_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadContext, MilGraList, MIL.M_DRAW_POSITION, MIL.M_USER, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   # Pause to show the template image and user points. 
   print("The initial points specified by the user (in red) are")
   print("used to train the bead information from an image.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Set a 1:1 calibration to the training image. 
   MIL.McalUniform(MilImageTraining, 0, 0, 1, 1, 0, MIL.M_DEFAULT)

   # Train the bead context. 
   MIL.MbeadTrain(MilBeadContext, MilImageTraining, MIL.M_DEFAULT)

   # Display the trained bead.    
   MIL.MgraColor(MIL.M_DEFAULT, TRAINED_BEAD_WIDTH_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadContext, MilGraList, MIL.M_DRAW_WIDTH, MIL.M_TRAINED, MIL.M_TEMPLATE_LABEL(1), MIL.M_ALL, MIL.M_DEFAULT)

   # Retrieve the trained nominal width. 
   NominalWidth = MIL.MbeadInquire(MilBeadContext,MIL.M_TEMPLATE_LABEL(1), MIL.M_TRAINED_WIDTH_NOMINAL)

   print("The template has been trained and is displayed in orange.")
   print("Its nominal trained width is {:.2f} pixels.\n".format(NominalWidth))

   # model to further fixture the bead template. 
   MilModelFinderContext = MIL.MmodAlloc(MilSystem, MIL.M_GEOMETRIC, MIL.M_DEFAULT)

   MilModelFinderResult = MIL.MmodAllocResult(MilSystem, MIL.M_DEFAULT)

   MIL.MmodDefine(MilModelFinderContext, MIL.M_IMAGE, MilImageTraining, MODEL_ORIGIN_X, MODEL_ORIGIN_Y, MODEL_SIZE_X, MODEL_SIZE_Y)

   # Preprocess the model. 
   MIL.MmodPreprocess(MilModelFinderContext, MIL.M_DEFAULT)   

   # Allocate a fixture object. 
   MilFixturingOffset = MIL.McalAlloc(MilSystem, MIL.M_FIXTURING_OFFSET, MIL.M_DEFAULT)

   # Learn the relative offset of the model. 
   MIL.McalFixture(MIL.M_NULL, MilFixturingOffset, MIL.M_LEARN_OFFSET, MIL.M_MODEL_MOD, MilModelFinderContext, 0, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Display the model. 
   MIL.MgraColor(MIL.M_DEFAULT, MODEL_FINDER_COLOR)
   MIL.MmodDraw(MIL.M_DEFAULT, MilModelFinderContext, MilGraList, MIL.M_DRAW_BOX+MIL.M_DRAW_POSITION, MIL.M_DEFAULT, MIL.M_ORIGINAL) 

   print("A Model Finder model (in green) is also defined to")
   print("further fixture the bead verification operation.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Clear the overlay annotation. 
   MIL.MgraClear(MIL.M_DEFAULT, MilGraList)

   # Display the target image buffer. 
   MIL.MdispSelect(MilDisplay, MilImageTarget)

   # Find the location of the fixture using Model Finder. 
   MIL.MmodFind(MilModelFinderContext, MilImageTarget, MilModelFinderResult)

   # Display the found model occurrence. 
   MIL.MgraColor(MIL.M_DEFAULT, MODEL_FINDER_COLOR)
   MIL.MmodDraw(MIL.M_DEFAULT, MilModelFinderResult, MilGraList, MIL.M_DRAW_BOX+MIL.M_DRAW_POSITION, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Apply fixture offset to the target image. 
   MIL.McalFixture(MilImageTarget, MilFixturingOffset, MIL.M_MOVE_RELATIVE, MIL.M_RESULT_MOD, MilModelFinderResult, 0, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Display the relative coordinate system. 
   MIL.MgraColor(MIL.M_DEFAULT, COORDINATE_SYSTEM_COLOR)
   MIL.McalDraw(MIL.M_DEFAULT, MIL.M_NULL, MilGraList, MIL.M_DRAW_RELATIVE_COORDINATE_SYSTEM, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Perform the inspection of the bead in the fixtured target image. 
   MIL.MbeadVerify(MilBeadContext, MilImageTarget, MilBeadResult, MIL.M_DEFAULT)

   # Display the result search boxes. 
   MIL.MgraColor(MIL.M_DEFAULT, RESULT_SEARCH_BOX_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadResult, MilGraList, MIL.M_DRAW_SEARCH_BOX, MIL.M_ALL, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   print("The mechanical part's position and angle (in green) were located")
   print("using Model Finder, and the bead's search boxes (in cyan) were")
   print("positioned accordingly.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Clear the overlay annotation. 
   MIL.MgraClear(MIL.M_DEFAULT, MilGraList)

   # Display the moved relative coordinate system. 
   MIL.MgraColor(MIL.M_DEFAULT, COORDINATE_SYSTEM_COLOR)
   MIL.McalDraw(MIL.M_DEFAULT, MIL.M_NULL, MilGraList, MIL.M_DRAW_RELATIVE_COORDINATE_SYSTEM, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Display the pass bead sections. 
   MIL.MgraColor(MIL.M_DEFAULT, PASS_BEAD_WIDTH_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadResult, MilGraList, MIL.M_DRAW_WIDTH, MIL.M_PASS, MIL.M_TEMPLATE_LABEL(1), MIL.M_ALL, MIL.M_DEFAULT)

   # Display the missing bead sections. 
   MIL.MgraColor(MIL.M_DEFAULT, FAIL_NOT_FOUND_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadResult, MilGraList, MIL.M_DRAW_SEARCH_BOX, MIL.M_FAIL_NOT_FOUND, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   # Display bead sections which do not meet the minimum width criteria. 
   MIL.MgraColor(MIL.M_DEFAULT, FAIL_SMALL_WIDTH_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadResult, MilGraList, MIL.M_DRAW_SEARCH_BOX, MIL.M_FAIL_WIDTH_MIN, MIL.M_TEMPLATE_LABEL(1), MIL.M_ALL, MIL.M_DEFAULT)

   # Retrieve and display general bead results. 
   Score = MIL.MbeadGetResult(MilBeadResult, MIL.M_TEMPLATE_LABEL(1), MIL.M_GENERAL, MIL.M_SCORE)
   GapCov = MIL.MbeadGetResult(MilBeadResult, MIL.M_TEMPLATE_LABEL(1), MIL.M_GENERAL, MIL.M_GAP_COVERAGE)
   AvWidth = MIL.MbeadGetResult(MilBeadResult, MIL.M_TEMPLATE_LABEL(1), MIL.M_GENERAL, MIL.M_WIDTH_AVERAGE)
   MaxGap = MIL.MbeadGetResult(MilBeadResult, MIL.M_TEMPLATE_LABEL(1), MIL.M_GENERAL, MIL.M_GAP_MAX_LENGTH)

   print("The bead has been inspected:")
   print(" -Passing bead sections (green) cover {:.2f}% of the bead".format(Score))
   print(" -Missing bead sections (red) cover {:.2f}% of the bead".format(GapCov))
   print(" -Sections outside the specified tolerances are drawn in orange")
   print(" -The bead's average width is {:.2f} pixels".format(AvWidth))
   print(" -The bead's longest gap section is {:.2f} pixels".format(MaxGap))

   # Pause to show the result. 
   print("Press <Enter> to continue.")
   MIL.MosGetch()

   # Free all allocations. 
   MIL.MmodFree(MilModelFinderContext)
   MIL.MmodFree(MilModelFinderResult)
   MIL.MbeadFree(MilBeadContext)
   MIL.MbeadFree(MilBeadResult)
   MIL.McalFree(MilFixturingOffset)
   MIL.MbufFree(MilImageTraining)
   MIL.MbufFree(MilImageTarget)
   MIL.MgraFree(MilGraList)
   
def PredefinedBeadExample(MilSystem, MilDisplay):    

   # Restore target image into an automatically allocated image buffers. 
   MilImageTarget = MIL.MbufRestore(CAP_FILE_TARGET, MilSystem)

   # Display the training image buffer. 
   MIL.MdispSelect(MilDisplay, MilImageTarget)

   # Prepare the overlay for annotations. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MilOverlayImage = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_TRANSPARENT_COLOR)

   # Original template image. 
   print("\nPREDEFINED BEAD INSPECTION:")
   print("---------------------------\n")
   print("This program performs a bead inspection of a bottle")
   print("cap's contour using a predefined circular bead.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Allocate a MIL bead context. 
   MilBeadContext = MIL.MbeadAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Allocate a MIL bead result. 
   MilBeadResult = MIL.MbeadAllocResult(MilSystem, MIL.M_DEFAULT)

   # Add the bead templates. 
   MIL.MbeadTemplate(MilBeadContext, MIL.M_ADD, MIL.M_BEAD_EDGE, MIL.M_TEMPLATE_LABEL(1), 0, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT)

   # Set the bead shape properties. 
   MIL.MbeadControl(MilBeadContext, MIL.M_TEMPLATE_LABEL(1), MIL.M_TRAINING_PATH, MIL.M_CIRCLE)

   MIL.MbeadControl(MilBeadContext, MIL.M_TEMPLATE_LABEL(1), MIL.M_TEMPLATE_CIRCLE_CENTER_X, CIRCLE_CENTER_X)
   MIL.MbeadControl(MilBeadContext, MIL.M_TEMPLATE_LABEL(1), MIL.M_TEMPLATE_CIRCLE_CENTER_Y, CIRCLE_CENTER_Y)
   MIL.MbeadControl(MilBeadContext, MIL.M_TEMPLATE_LABEL(1), MIL.M_TEMPLATE_CIRCLE_RADIUS, CIRCLE_RADIUS)

   # Set the edge threshold value to extract the object shape. 
   MIL.MbeadControl(MilBeadContext, MIL.M_TEMPLATE_LABEL(1), MIL.M_THRESHOLD_VALUE, EDGE_THRESHOLD_VALUE)

   # Using the default fixed user defined nominal edge width. 
   MIL.MbeadControl(MilBeadContext, MIL.M_ALL, MIL.M_WIDTH_NOMINAL_MODE, MIL.M_USER_DEFINED)

   # Set the maximal expected contour deformation. 
   MIL.MbeadControl(MilBeadContext, MIL.M_ALL, MIL.M_FAIL_WARNING_OFFSET, MAX_CONTOUR_FOUND_OFFSET)

   # Set the maximum valid bead deformation. 
   MIL.MbeadControl(MilBeadContext, MIL.M_ALL, MIL.M_OFFSET_MAX, MAX_CONTOUR_DEVIATION_OFFSET)      

   # Display the bead in the overlay image. 
   MIL.MgraColor(MIL.M_DEFAULT, USER_TEMPLATE_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadContext, MilOverlayImage, MIL.M_DRAW_POSITION, MIL.M_USER, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   # The bead template is entirely defined and is trained without sample image. 
   MIL.MbeadTrain(MilBeadContext, MIL.M_NULL, MIL.M_DEFAULT)

   # Display the trained bead. 
   MIL.MgraColor(MIL.M_DEFAULT, TRAINED_BEAD_WIDTH_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadContext, MilOverlayImage, MIL.M_DRAW_SEARCH_BOX, MIL.M_TRAINED, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   # Pause to show the template image and user points. 
   print("A circular template that was parametrically defined by the")
   print("user is displayed (in cyan). The template has been trained") 
   print("and the resulting search is displayed (in orange).")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Perform the inspection of the bead in the fixtured target image. 
   MIL.MbeadVerify(MilBeadContext, MilImageTarget, MilBeadResult, MIL.M_DEFAULT)

   # Clear the overlay annotation. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_TRANSPARENT_COLOR)

   # Display the pass bead sections. 
   MIL.MgraColor(MIL.M_DEFAULT, PASS_BEAD_POSITION_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadResult, MilOverlayImage, MIL.M_DRAW_POSITION, MIL.M_PASS, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   # Display the offset bead sections. 
   MIL.MgraColor(MIL.M_DEFAULT, FAIL_EDGE_OFFSET_COLOR)
   MIL.MbeadDraw(MIL.M_DEFAULT, MilBeadResult, MilOverlayImage, MIL.M_DRAW_POSITION, MIL.M_FAIL_OFFSET, MIL.M_ALL, MIL.M_ALL, MIL.M_DEFAULT)

   # Retrieve and display general bead results. 
   MaximumOffset = MIL.MbeadGetResult(MilBeadResult, MIL.M_TEMPLATE_LABEL(1), MIL.M_GENERAL, MIL.M_OFFSET_MAX)

   print("The bottle cap shape has been inspected:")
   print(" -Sections outside the specified offset tolerance are drawn in red")
   print(" -The maximum offset value is {:.2f} pixels.\n".format(MaximumOffset))

   # Pause to show the result. 
   print("Press <Enter> to terminate.\n")
   MIL.MosGetch()

   # Free all allocations. 
   MIL.MbeadFree(MilBeadContext)
   MIL.MbeadFree(MilBeadResult)
   MIL.MbufFree(MilImageTarget)

if __name__ == "__main__":
   MbeadExample()