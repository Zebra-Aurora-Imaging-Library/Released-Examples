#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#*************************************************************************************
#
# File name: Mclass.py
#
# Synopsis:  This example identifies the type of pastas using a 
# pre-trained classification module. 
#
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved

import mil as MIL

# Path definitions.
EXAMPLE_IMAGE_DIR_PATH   = MIL.M_IMAGE_PATH + "/Classification/Pasta/"
EXAMPLE_CLASS_CTX_PATH   = EXAMPLE_IMAGE_DIR_PATH + "MatroxNet_PastaEx.mclass"
TARGET_IMAGE_DIR_PATH    = EXAMPLE_IMAGE_DIR_PATH + "Products"

DIG_IMAGE_FOLDER         = TARGET_IMAGE_DIR_PATH
DIG_REMOTE_IMAGE_FOLDER  = "remote:///" + TARGET_IMAGE_DIR_PATH

# Util constant.
BUFFERING_SIZE_MAX = 10

#/****************************************************************************
#    Main.
#/****************************************************************************

# User's processing function hook data structure.
class HookDataStruct():
   def __init__(self, ClassCtx, ClassRes, MilDisplay, MilDispChild, NumberOfCategories, MilOverlayImage, SourceSizeX, SourceSizeY, NumberOfFrames):      
      self.ClassCtx = ClassCtx
      self.ClassRes = ClassRes
      self.MilDisplay = MilDisplay
      self.MilDispChild = MilDispChild
      self.NbCategories = NumberOfCategories
      self.MilOverlayImage = MilOverlayImage
      self.SourceSizeX = SourceSizeX
      self.SourceSizeY = SourceSizeY
      self.NbOfFrames = NumberOfFrames

def MclassExample():

   NumberOfCategories = 0

   # Allocate MIL objects.
   MilApplication = MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT)
   MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   SystemType = MIL.MsysInquire(MilSystem, MIL.M_SYSTEM_TYPE, MIL.M_NULL)
   if SystemType != MIL.M_SYSTEM_HOST_TYPE:
      MIL.MsysFree(MilSystem)
      MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_HOST, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_NULL)
                
   MilDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, ("M_DEFAULT"), MIL.M_DEFAULT)

   MilSystemLocation = MIL.MsysInquire(MilSystem, MIL.M_LOCATION, MIL.M_NULL);
   DigImageFolder = DIG_IMAGE_FOLDER
   if MilSystemLocation == MIL.M_REMOTE:
      DigImageFolder = DIG_REMOTE_IMAGE_FOLDER
   MilDigitizer = MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, DigImageFolder, MIL.M_DEFAULT)

   # Print the example synopsis.
   print("[EXAMPLE NAME]")
   print("Mclass\n")
   print("[SYNOPSIS]")
   print("This programs shows the use of a pre-trained classification")
   print("tool to recognize product categories.\n")
   print("[MODULES USED]")
   print("Classification, Buffer, Display, Graphics, Image Processing.\n")

   # Wait for user.
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()
   
   print("Restoring the classification context from file..", end="")
   ClassCtx = MIL.MclassRestore(EXAMPLE_CLASS_CTX_PATH, MilSystem, MIL.M_DEFAULT)
   print(".", end="")

   # Preprocess the context.
   MIL.MclassPreprocess(ClassCtx, MIL.M_DEFAULT)
   print(".ready.")

   NumberOfCategories = MIL.MclassInquire(ClassCtx, MIL.M_CONTEXT, MIL.M_NUMBER_OF_CLASSES + MIL.M_TYPE_MIL_INT)
   InputSizeX = MIL.MclassInquire(ClassCtx, MIL.M_DEFAULT_SOURCE_LAYER, MIL.M_SIZE_X + MIL.M_TYPE_MIL_INT)
   InputSizeY = MIL.MclassInquire(ClassCtx, MIL.M_DEFAULT_SOURCE_LAYER, MIL.M_SIZE_Y + MIL.M_TYPE_MIL_INT)

   if(NumberOfCategories > 0):
      # Inquire and print source layer information.
      print(" - The classifier was trained to recognize {:d} categories".format(NumberOfCategories))
      print(" - The classifier was trained for {:d}x{:d} source images\n".format(InputSizeX, InputSizeY))

      # Allocate a classification result buffer.
      ClassRes = MIL.MclassAllocResult(MilSystem, MIL.M_PREDICT_CNN_RESULT, MIL.M_DEFAULT)

      # Inquire the size of the source image.
      SourceSizeX = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X)
      SourceSizeY = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y)

      # Setup the example display.
      MilDispImage, MilDispChild, MilOverlayImage = SetupDisplay(MilSystem, MilDisplay, SourceSizeX, SourceSizeY, ClassCtx, NumberOfCategories)

      # Retrieve the number of frame in the source directory.
      NumberOfFrames = MIL.MdigInquire(MilDigitizer, MIL.M_SOURCE_NUMBER_OF_FRAMES)

      MilGrabBufferList = [0]*BUFFERING_SIZE_MAX   # MIL image identifier
      MilChildBufferList = [0]*BUFFERING_SIZE_MAX  # MIL child identifier

      # Allocate the grab buffers.
      for BufIndex in range(0, BUFFERING_SIZE_MAX):
         MilGrabBufferList[BufIndex] = MIL.MbufAlloc2d(MilSystem, SourceSizeX, SourceSizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_GRAB + MIL.M_PROC)
         MilChildBufferList[BufIndex] = MIL.MbufChild2d(MilGrabBufferList[BufIndex], int((SourceSizeX - InputSizeX) / 2), int((SourceSizeY - InputSizeY) / 2), InputSizeX, InputSizeY)
         MIL.MobjControl(MilGrabBufferList[BufIndex], MIL.M_OBJECT_USER_DATA_PTR, MilChildBufferList[BufIndex])
         
      
      # Initialize the user's processing function data structure.
      ClassificationData = HookDataStruct(ClassCtx, ClassRes, MilDisplay, MilDispChild, NumberOfCategories, MilOverlayImage, SourceSizeX, SourceSizeY, NumberOfFrames)
   
      # Start the processing. The processing function is called with every frame grabbed.
      ClassificationFuncPtr = MIL.MIL_DIG_HOOK_FUNCTION_PTR(ClassificationFunc)
      
      # Start the grab.
      if(NumberOfFrames != MIL.M_INFINITE):
         ClassificationData = MIL.MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE_MAX, MIL.M_SEQUENCE + MIL.M_COUNT(NumberOfFrames), MIL.M_SYNCHRONOUS, ClassificationFuncPtr, ClassificationData)
      else:
         ClassificationData = MIL.MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE_MAX, MIL.M_START, MIL.M_DEFAULT, ClassificationFuncPtr, ClassificationData)

      # Ready to exit.
      print("\nPress <Enter> to exit.")
      MIL.MosGetch()

      # Stop the digitizer.
      MIL.MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE_MAX, MIL.M_STOP, MIL.M_DEFAULT, ClassificationFuncPtr, ClassificationData)

      MIL.MbufFree(MilDispChild)
      MIL.MbufFree(MilDispImage)

      for BufIndex in range(0, BUFFERING_SIZE_MAX):
        MIL.MbufFree(MilChildBufferList[BufIndex])
        MIL.MbufFree(MilGrabBufferList[BufIndex])
         
      MIL.MclassFree(ClassRes)
      MIL.MclassFree(ClassCtx)
      

   # Free the allocated resources.
   MIL.MdigFree(MilDigitizer)
   MIL.MdispFree(MilDisplay)
   MIL.MsysFree(MilSystem)
   MIL.MappFree(MilApplication)

   return 0
   

def SetupDisplay(MilSystem, MilDisplay, SourceSizeX, SourceSizeY, ClassCtx, NbCategories):
          
   # Allocate a color buffer.
   IconSize = int(SourceSizeY / NbCategories)
   MilDispImage = MIL.MbufAllocColor(MilSystem, 3, SourceSizeX + IconSize, SourceSizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP, MIL.M_NULL)
   MIL.MbufClear(MilDispImage, MIL.M_COLOR_BLACK)
   MilDispChild = MIL.MbufChild2d(MilDispImage, 0, 0, SourceSizeX, SourceSizeY, MIL.M_NULL)

   # Set annotation color.
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED)

   # Setup the display.
   for iter in range(0, NbCategories):

      # Allocate a child buffer per product categorie.   
      MilChildSample = MIL.MbufChild2d(MilDispImage, SourceSizeX, iter * IconSize, IconSize, IconSize)

      # Load the sample image.
      MilImageLoader = MIL.MclassInquire(ClassCtx, MIL.M_CLASS_INDEX(iter), MIL.M_CLASS_ICON_ID + MIL.M_TYPE_MIL_ID)
      
      if (MilImageLoader != MIL.M_NULL):
         MIL.MimResize(MilImageLoader, MilChildSample, MIL.M_FILL_DESTINATION, MIL.M_FILL_DESTINATION, MIL.M_BICUBIC + MIL.M_OVERSCAN_FAST) 

      # Draw an initial red rectangle around the buffer.
      MIL.MgraRect(MIL.M_DEFAULT, MilChildSample, 0, 1, IconSize - 1, IconSize - 2)

      # Free the allocated buffers.
      MIL.MbufFree(MilChildSample)
      

   # Display the window with black color.
   MIL.MdispSelect(MilDisplay, MilDispImage)

   # Prepare for overlay annotations.
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MilOverlay = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID, MIL.M_NULL)

   return MilDispImage, MilDispChild, MilOverlay
   

def ClassificationFunc(HookType, EventId, DataPtr):

   MilImage = MIL.MdigGetHookInfo(EventId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID)

   data = DataPtr
   MIL.MdispControl(data.MilDisplay, MIL.M_UPDATE, MIL.M_DISABLE)
   pMilInputImage = MIL.MobjInquire(MilImage, MIL.M_OBJECT_USER_DATA_PTR)
   
   # Display the new target image.
   MIL.MbufCopy(MilImage, data.MilDispChild)

   # Perform product recognition using the classification module.
   MIL.MclassPredict(data.ClassCtx, pMilInputImage, data.ClassRes, MIL.M_DEFAULT)

   # Retrieve best classification score and class index.
   BestScore = MIL.MclassGetResult(data.ClassRes, MIL.M_GENERAL, MIL.M_BEST_CLASS_SCORE + MIL.M_TYPE_MIL_DOUBLE)[0]

   BestIndex = MIL.MclassGetResult(data.ClassRes, MIL.M_GENERAL, MIL.M_BEST_CLASS_INDEX + MIL.M_TYPE_MIL_INT)[0]

   # Clear the overlay buffer.
   MIL.MdispControl(data.MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_TRANSPARENT_COLOR)
   
   # Draw a green rectangle around the winning sample.
   IconSize = int(data.SourceSizeY / data.NbCategories)
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN)
   MIL.MgraRect(MIL.M_DEFAULT, data.MilOverlayImage, data.SourceSizeX, (BestIndex*IconSize)+1, data.SourceSizeX + IconSize - 1, (BestIndex + 1)*IconSize - 2)

   # Print the classification accuracy in the sample buffer.
   Accuracy_text = "{:.1f}%".format(BestScore) + " score"
   MIL.MgraControl(MIL.M_DEFAULT, MIL.M_BACKGROUND_MODE, MIL.M_TRANSPARENT)
   MIL.MgraFont(MIL.M_DEFAULT, MIL.M_FONT_DEFAULT_SMALL)
   MIL.MgraText(MIL.M_DEFAULT, data.MilOverlayImage, data.SourceSizeX+ 2, BestIndex*IconSize + 4, Accuracy_text)

   # Update the display.
   MIL.MdispControl(data.MilDisplay, MIL.M_UPDATE, MIL.M_ENABLE)

   # Wait for the user.
   if (data.NbOfFrames != MIL.M_INFINITE):
      print("Press <Enter> to continue.", end="\r")
      MIL.MosGetch()
      
   return 0
   
if __name__ == "__main__":
   MclassExample()
