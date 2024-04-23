#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MCode.py
#
# Synopsis:  This program decodes a linear Code 39 and a DataMatrix code.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################

 
import mil as MIL

# Target image character specifications. 
IMAGE_FILE                   = MIL.M_IMAGE_PATH + "Code.mim"

# Regions around 1D code. 
BARCODE_REGION_TOP_LEFT_X    = 256
BARCODE_REGION_TOP_LEFT_Y    =  80
BARCODE_REGION_SIZE_X        = 290
BARCODE_REGION_SIZE_Y        =  60

# Regions around 2D code. 
DATAMATRIX_REGION_TOP_LEFT_X =   8
DATAMATRIX_REGION_TOP_LEFT_Y = 312
DATAMATRIX_REGION_SIZE_X     = 118
DATAMATRIX_REGION_SIZE_Y     = 105

# Maximum length of the string to read. 
STRING_LENGTH_MAX            = 64


def McodeExample():
   AnnotationColor = MIL.M_COLOR_GREEN
   AnnotationBackColor = MIL.M_COLOR_GRAY

   # Allocate defaults.
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Restore source image into an automatically allocated image buffer. 
   MilImage = MIL.MbufRestore(IMAGE_FILE, MilSystem)

   # Display the image buffer. 
   MIL.MdispSelect(MilDisplay, MilImage)
       
   # Prepare for overlay annotations. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MilOverlayImage = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)

   # Prepare bar code results buffer 
   CodeResults = MIL.McodeAllocResult(MilSystem, MIL.M_DEFAULT)

   # Pause to show the original image. 
   print("\n1D and 2D CODE READING:")
   print("-----------------------\n")
   print("This program will decode a linear Code 39 and a DataMatrix code.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()


   # 1D BARCODE READING: 
   
   # Create a read region around the code to speedup reading. 
   BarCodeRegion = MIL.MbufChild2d(MilImage, BARCODE_REGION_TOP_LEFT_X, BARCODE_REGION_TOP_LEFT_Y,
               BARCODE_REGION_SIZE_X, BARCODE_REGION_SIZE_Y)
   
   # Allocate CODE objects. 
   Barcode = MIL.McodeAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MIL.McodeModel(Barcode, MIL.M_ADD, MIL.M_CODE39, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_NULL)

   # Read codes from image. 
   MIL.McodeRead(Barcode, BarCodeRegion, CodeResults)
    
   # Get decoding status. 
   BarcodeStatus = MIL.McodeGetResult(CodeResults, MIL.M_GENERAL, MIL.M_GENERAL, MIL.M_STATUS + MIL.M_TYPE_MIL_INT)

   # Check if decoding was successful. 
   if BarcodeStatus == MIL.M_STATUS_READ_OK:
      # Get decoded string. 
      BarcodeString = MIL.McodeGetResult(CodeResults, 0, MIL.M_GENERAL, MIL.M_STRING)

      # Draw the decoded strings and read region in the overlay image. 
      MIL.MgraColor(MIL.M_DEFAULT, AnnotationColor) 
      MIL.MgraBackColor(MIL.M_DEFAULT, AnnotationBackColor) 
      OutputString = "\"{BarcodeString}\" ".format(BarcodeString=BarcodeString)
      MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, BARCODE_REGION_TOP_LEFT_X + 10,  BARCODE_REGION_TOP_LEFT_Y + 80, " 1D linear 39 bar code:           ")
      MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, BARCODE_REGION_TOP_LEFT_X + 200, BARCODE_REGION_TOP_LEFT_Y + 80, OutputString)
      MIL.MgraRect(MIL.M_DEFAULT, MilOverlayImage,
                   BARCODE_REGION_TOP_LEFT_X, BARCODE_REGION_TOP_LEFT_Y,
                   BARCODE_REGION_TOP_LEFT_X + BARCODE_REGION_SIZE_X, 
                   BARCODE_REGION_TOP_LEFT_Y + BARCODE_REGION_SIZE_Y)

   # Free objects.
   MIL.McodeFree(Barcode)
   MIL.MbufFree(BarCodeRegion)

   # 2D CODE READING:
  
   # Create a read region around the code to speedup reading.
   DataMatrixRegion = MIL.MbufChild2d(MilImage, DATAMATRIX_REGION_TOP_LEFT_X, DATAMATRIX_REGION_TOP_LEFT_Y,
               DATAMATRIX_REGION_SIZE_X, DATAMATRIX_REGION_SIZE_Y)
  
   # Allocate CODE objects.
   DataMatrixCode = MIL.McodeAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MIL.McodeModel(DataMatrixCode, MIL.M_ADD, MIL.M_DATAMATRIX, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_NULL)


   # Set the foreground value for the DataMatrix since it is different
   # from the default value. 
   MIL.McodeControl(DataMatrixCode, MIL.M_FOREGROUND_VALUE, MIL.M_FOREGROUND_WHITE) 
    
   # Read codes from image.
   MIL.McodeRead(DataMatrixCode, DataMatrixRegion, CodeResults)
    
   # Get decoding status.
   DataMatrixStatus = MIL.McodeGetResult(CodeResults, MIL.M_GENERAL, MIL.M_GENERAL, MIL.M_STATUS + MIL.M_TYPE_MIL_INT)

   # Check if decoding was successful. 
   if DataMatrixStatus == MIL.M_STATUS_READ_OK:
      # Get decoded string. 
      DataMatrixString = MIL.McodeGetResult(CodeResults, 0, MIL.M_GENERAL, MIL.M_STRING)

      # Draw the decoded strings and read region in the overlay image. 
      MIL.MgraColor(MIL.M_DEFAULT, AnnotationColor) 
      MIL.MgraBackColor(MIL.M_DEFAULT, AnnotationBackColor) 
      # Replace non printable characters with space.
      for n in range(len(DataMatrixString)):
         if DataMatrixString[n] < '0' or DataMatrixString[n] > 'Z':
            DataMatrixString = list(DataMatrixString)
            DataMatrixString[n] = ' '
            DataMatrixString = "".join(DataMatrixString)
      OutputString = "\"{DataMatrixString}\" ".format(DataMatrixString=DataMatrixString)
      MIL.MgraText(MIL.M_DEFAULT, 
               MilOverlayImage, 
               DATAMATRIX_REGION_TOP_LEFT_X,  
               DATAMATRIX_REGION_TOP_LEFT_Y+120, 
               " 2D Data Matrix code:                  ")
      MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, DATAMATRIX_REGION_TOP_LEFT_X + 200, 
               DATAMATRIX_REGION_TOP_LEFT_Y + 120, OutputString)
      MIL.MgraRect(MIL.M_DEFAULT, MilOverlayImage,
               DATAMATRIX_REGION_TOP_LEFT_X, DATAMATRIX_REGION_TOP_LEFT_Y,
               DATAMATRIX_REGION_TOP_LEFT_X + DATAMATRIX_REGION_SIZE_X, 
               DATAMATRIX_REGION_TOP_LEFT_Y + DATAMATRIX_REGION_SIZE_Y)

   # Free objects. 
   MIL.McodeFree(DataMatrixCode)
   MIL.MbufFree(DataMatrixRegion)

   # Free results buffer. 
   MIL.McodeFree(CodeResults)

   # Pause to show the results. 
   if DataMatrixStatus == MIL.M_STATUS_READ_OK and BarcodeStatus == MIL.M_STATUS_READ_OK:
      print("Decoding was successful and the strings were written under each code.")
   else:
      print("Decoding error found.")
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free other allocations. 
   MIL.MbufFree(MilImage)
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)
   
   return 0

if __name__ == "__main__":
   McodeExample()
