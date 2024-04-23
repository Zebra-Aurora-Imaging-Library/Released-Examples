#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: Mdmr.py
#
# Synopsis:  This program uses the Dot Matrix Reader (SureDotOCR®) module to
#            read a product expiry date and lot number printed using a
#            CIJ printer.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################

 
import mil as MIL

# MIL file specifications. 
IMAGE_FILE_TO_READ   = MIL.M_IMAGE_PATH   + "ExpiryDateAndLot.mim"
FONT_FILE_TO_IMPORT  = MIL.M_CONTEXT_PATH + "ExpiryDateAndLotFont5x7.mdmrf"

# Dot Matrix Reader settings. 
STRING_DOT_DIAMETER = 6.0
TEXT_BLOCK_WIDTH    = 400
TEXT_BLOCK_HEIGHT   = 100
EXPIRY_DATE_LENGTH  = 7
LOT_NUMBER_LENGTH   = 7

# Util. 
TEXT_MAX_SIZE = 128


def MdmrExample():
   # Print module name.
   print("\nDOT MATRIX READER (SureDotOCR) MODULE:")
   print("--------------------------------------\n")

   # Allocate defaults.
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Restore the font definition image. 
   MilImage = MIL.MbufRestore(IMAGE_FILE_TO_READ, MilSystem)

   # Display the image and prepare for overlay annotations. 
   MIL.MdispSelect(MilDisplay, MilImage)
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MilOverlay = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)

   # Allocate a new empty Dot Matrix Reader context. 
   MilDmrContext = MIL.MdmrAlloc(MilSystem, MIL.M_DOT_MATRIX, MIL.M_DEFAULT)

   # Allocate a new empty Dot Matrix Reader result buffer. 
   MilDmrResult = MIL.MdmrAllocResult(MilSystem, MIL.M_DOT_MATRIX, MIL.M_DEFAULT)

   # Import a Dot Matrix font. 
   MIL.MdmrImportFont(FONT_FILE_TO_IMPORT, MIL.M_DMR_FONT_FILE, MilDmrContext, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_DEFAULT)

   # Add a new string model definition for the product lot number. 
   # ------------------------------------------------------------- 
   MIL.MdmrControl(MilDmrContext, MIL.M_STRING_ADD, MIL.M_DEFAULT)

   # Set the string model rank and size. 
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(0), MIL.M_DEFAULT, MIL.M_STRING_RANK,      1, MIL.M_DEFAULT, MIL.M_NULL)
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(0), MIL.M_DEFAULT, MIL.M_STRING_SIZE_MIN_MAX, LOT_NUMBER_LENGTH, LOT_NUMBER_LENGTH, MIL.M_NULL)

   # Add a new string model definition for the expiry date (YYYY MM DD). 
   # ------------------------------------------------------------------- 
   MIL.MdmrControl(MilDmrContext, MIL.M_STRING_ADD, MIL.M_DEFAULT)

   # Set the string model rank and size. 
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_DEFAULT, MIL.M_STRING_RANK,     0, MIL.M_DEFAULT, MIL.M_NULL)
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_DEFAULT, MIL.M_STRING_SIZE_MIN_MAX, EXPIRY_DATE_LENGTH, EXPIRY_DATE_LENGTH, MIL.M_NULL)

   # Set the string model constraint for an expiry date (DDMMMYY). 
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(0), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_DIGITS,            MIL.M_NULL)
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(1), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_DIGITS,            MIL.M_NULL)
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(2), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_LETTERS_UPPERCASE, MIL.M_NULL)
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(3), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_LETTERS_UPPERCASE, MIL.M_NULL)
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(4), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_LETTERS_UPPERCASE, MIL.M_NULL)
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(5), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_DIGITS,            MIL.M_NULL)
   MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(6), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_DIGITS,            MIL.M_NULL)

   print("A Dot Matrix Reader (SureDotOCR) context was set up to read")
   print("an expiry date and a lot number from a target image.\n")

   # Set the Dot Matrix dot diameter. 
   MIL.MdmrControl(MilDmrContext, MIL.M_DOT_DIAMETER, STRING_DOT_DIAMETER)

   # Set the maximum size of the string box. 
   MIL.MdmrControl(MilDmrContext, MIL.M_TEXT_BLOCK_WIDTH,  TEXT_BLOCK_WIDTH)
   MIL.MdmrControl(MilDmrContext, MIL.M_TEXT_BLOCK_HEIGHT, TEXT_BLOCK_HEIGHT)

   # Preprocess the context. 
   MIL.MdmrPreprocess(MilDmrContext, MIL.M_DEFAULT)

   # Reading the string from a target image. 
   MIL.MdmrRead(MilDmrContext, MilImage, MilDmrResult, MIL.M_DEFAULT)

   # Get number of strings read and show the result. 
   NumberOfStrings = MIL.MdmrGetResult(MilDmrResult, MIL.M_GENERAL, MIL.M_DEFAULT, MIL.M_STRING_NUMBER + MIL.M_TYPE_MIL_INT)

   # Draw the read result. 
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN)
   MIL.MdmrDraw(MIL.M_DEFAULT, MilDmrResult, MilOverlay, MIL.M_DRAW_STRING_CHAR_BOX, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_CYAN)
   MIL.MdmrDraw(MIL.M_DEFAULT, MilDmrResult, MilOverlay, MIL.M_DRAW_STRING_CHAR_POSITION, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   if NumberOfStrings > 0:
      print("Result: {NumberOfStrings} strings have been read:\n".format(NumberOfStrings=NumberOfStrings))

      for Index in range(NumberOfStrings):
         # Print the read result. 
         StringModelIndex = MIL.MdmrGetResult(MilDmrResult, Index, MIL.M_GENERAL, MIL.M_STRING_MODEL_INDEX + MIL.M_TYPE_MIL_INT)
         StringResult = MIL.MdmrGetResult(MilDmrResult, Index, MIL.M_GENERAL, MIL.M_STRING)


         if StringModelIndex == 0:
            PrintText = " LOT# : " + StringResult + " "
            MIL.MgraText(MIL.M_DEFAULT, MilOverlay, 20, 20 + Index * 20, PrintText)
            print(" LOT# : {StringResult}".format(StringResult=StringResult))
         elif StringModelIndex == 1:
            PrintText = " EXP. : " + StringResult + " "
            MIL.MgraText(MIL.M_DEFAULT, MilOverlay, 20, 20 + Index * 20, PrintText)
            print(" EXPIRY DATE: {StringResult}".format(StringResult=StringResult))
         else:
            print("Unexpected model index")
   else:
      print("Error: no string found.")

   # Pause to show results. 
   print("\nPress <Enter> to end.\n")
   MIL.MosGetch()

   # Free all allocations. 
   MIL.MdmrFree(MilDmrContext)
   MIL.MdmrFree(MilDmrResult)
   MIL.MbufFree(MilImage)

   # Free defaults. 
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)
   
   return 0


if __name__ == "__main__":
   MdmrExample()
