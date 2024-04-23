/*******************************************************************************/
/*
* File name: Mdmr.cpp
*
* Synopsis:  This program uses the Dot Matrix Reader (SureDotOCR®) module to
*            read a product expiry date and lot number printed using a
*            CIJ printer.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

/* MIL file specifications. */
#define IMAGE_FILE_TO_READ    M_IMAGE_PATH   MIL_TEXT("ExpiryDateAndLot.mim")
#define FONT_FILE_TO_IMPORT   M_CONTEXT_PATH MIL_TEXT("ExpiryDateAndLotFont5x7.mdmrf")

/* Dot Matrix Reader settings. */
const MIL_DOUBLE STRING_DOT_DIAMETER = 6.0;
const MIL_DOUBLE TEXT_BLOCK_WIDTH    = 400;
const MIL_DOUBLE TEXT_BLOCK_HEIGHT   = 100;
const MIL_INT    EXPIRY_DATE_LENGTH  = 7;
const MIL_INT    LOT_NUMBER_LENGTH   = 7;

/* Util. */
const MIL_INT TEXT_MAX_SIZE = 128;

/*****************************************************************************/
/* Main. */
int MosMain(void)
   {
   MIL_ID MilApplication,                          /* Application identifier.          */
          MilSystem,                               /* System identifier.               */
          MilDisplay,                              /* Display identifier.              */
          MilImage,                                /* Image buffer identifier.         */
          MilOverlay,                              /* Overlay image.                   */
          MilDmrContext,                           /* Dot Matrix context identifier.   */
          MilDmrResult;                            /* Dot Matrix result identifier.    */

   MIL_INT NumberOfStrings = 0,                    /* Total number of strings to read. */
           StringSize = 0,                         /* Number of strings characters.    */
           StringModelIndex = 0,                   /* String model index.              */
           Index = 0;                              /* Result index.                    */

   MIL_TEXT_CHAR StringResult[TEXT_MAX_SIZE + 1],  /* String of characters read.       */
                 PrintText[TEXT_MAX_SIZE + 1];     /* Util text.                       */

   /* Print module name. */
   MosPrintf(MIL_TEXT("\nDOT MATRIX READER (SureDotOCR) MODULE:\n"));
   MosPrintf(MIL_TEXT("--------------------------------------\n\n"));

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

   /* Restore the font definition image. */
   MbufRestore(IMAGE_FILE_TO_READ, MilSystem, &MilImage);

   /* Display the image and prepare for overlay annotations. */
   MdispSelect(MilDisplay, MilImage);
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlay);

   /* Allocate a new empty Dot Matrix Reader context. */
   MdmrAlloc(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrContext);

   /* Allocate a new empty Dot Matrix Reader result buffer. */
   MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrResult);

   /* Import a Dot Matrix font. */
   MdmrImportFont(FONT_FILE_TO_IMPORT, M_DMR_FONT_FILE, MilDmrContext, M_DEFAULT, M_NULL, M_DEFAULT);

   /* Add a new string model definition for the product lot number. */
   /* ------------------------------------------------------------- */
   MdmrControl(MilDmrContext, M_STRING_ADD, M_DEFAULT);

   /* Set the string model rank and size. */
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(0), M_DEFAULT, M_STRING_RANK,      1, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(0), M_DEFAULT, M_STRING_SIZE_MIN_MAX, LOT_NUMBER_LENGTH, LOT_NUMBER_LENGTH, M_NULL);

   /* Add a new string model definition for the expiry date (YYYY MM DD). */
   /* ------------------------------------------------------------------- */
   MdmrControl(MilDmrContext, M_STRING_ADD, M_DEFAULT);

   /* Set the string model rank and size. */
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(1), M_DEFAULT, M_STRING_RANK,     0, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(1), M_DEFAULT, M_STRING_SIZE_MIN_MAX, EXPIRY_DATE_LENGTH, EXPIRY_DATE_LENGTH, M_NULL);

   /* Set the string model constraint for an expiry date (DDMMMYY). */
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(1), M_POSITION_IN_STRING(0), M_ADD_PERMITTED_CHARS_ENTRY, M_FONT_LABEL(M_ANY), M_DIGITS,            M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(1), M_POSITION_IN_STRING(1), M_ADD_PERMITTED_CHARS_ENTRY, M_FONT_LABEL(M_ANY), M_DIGITS,            M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(1), M_POSITION_IN_STRING(2), M_ADD_PERMITTED_CHARS_ENTRY, M_FONT_LABEL(M_ANY), M_LETTERS_UPPERCASE, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(1), M_POSITION_IN_STRING(3), M_ADD_PERMITTED_CHARS_ENTRY, M_FONT_LABEL(M_ANY), M_LETTERS_UPPERCASE, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(1), M_POSITION_IN_STRING(4), M_ADD_PERMITTED_CHARS_ENTRY, M_FONT_LABEL(M_ANY), M_LETTERS_UPPERCASE, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(1), M_POSITION_IN_STRING(5), M_ADD_PERMITTED_CHARS_ENTRY, M_FONT_LABEL(M_ANY), M_DIGITS,            M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_INDEX(1), M_POSITION_IN_STRING(6), M_ADD_PERMITTED_CHARS_ENTRY, M_FONT_LABEL(M_ANY), M_DIGITS,            M_NULL);

   MosPrintf(MIL_TEXT("A Dot Matrix Reader (SureDotOCR) context was set up to read\n")
             MIL_TEXT("an expiry date and a lot number from a target image.\n\n"));

   /* Set the Dot Matrix dot diameter. */
   MdmrControl(MilDmrContext, M_DOT_DIAMETER, STRING_DOT_DIAMETER);

   /* Set the maximum size of the string box. */
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_WIDTH, TEXT_BLOCK_WIDTH);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_HEIGHT, TEXT_BLOCK_HEIGHT);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Reading the string from a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   /* Get number of strings read and show the result. */
   MdmrGetResult(MilDmrResult, M_GENERAL, M_DEFAULT, M_STRING_NUMBER + M_TYPE_MIL_INT, &NumberOfStrings);

   /* Draw the read result. */
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MdmrDraw(M_DEFAULT, MilDmrResult, MilOverlay, M_DRAW_STRING_CHAR_BOX, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MdmrDraw(M_DEFAULT, MilDmrResult, MilOverlay, M_DRAW_STRING_CHAR_POSITION, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   if( NumberOfStrings > 0)
      {
      MosPrintf(MIL_TEXT("Result: %d strings have been read:\n\n"), NumberOfStrings);

      for (Index = 0; Index < NumberOfStrings; Index++)
         {
         /* Print the read result. */
         MdmrGetResult(MilDmrResult, Index, M_GENERAL, M_STRING_MODEL_INDEX + M_TYPE_MIL_INT, &StringModelIndex);
         MdmrGetResult(MilDmrResult, Index, M_GENERAL, M_STRING + M_STRING_SIZE + M_TYPE_MIL_INT, &StringSize);
         MdmrGetResult(MilDmrResult, Index, M_GENERAL, M_STRING, &StringResult[0]);

         switch (StringModelIndex)
            {
            case 0:
               MosSprintf(PrintText, TEXT_MAX_SIZE, MIL_TEXT(" LOT# : %s "), StringResult);
               MgraText(M_DEFAULT, MilOverlay, 20, 20 + Index * 20, PrintText);
               MosPrintf(MIL_TEXT(" LOT# : %s\n"), StringResult);
               break;
            case 1:
               MosSprintf(PrintText, TEXT_MAX_SIZE, MIL_TEXT(" EXP. : %s "), StringResult);
               MgraText(M_DEFAULT, MilOverlay, 20, 20 + Index * 20, PrintText);
               MosPrintf(MIL_TEXT(" EXPIRY DATE: %s\n"), StringResult);
               break;
            default:
               MosPrintf(MIL_TEXT("Unexpected model index\n"));
               break;
            }
         }
      }
   else
      {
      MosPrintf(MIL_TEXT("Error: no string found.\n"));
      }

   /* Pause to show results. */
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free all allocations. */
   MdmrFree(MilDmrContext);
   MdmrFree(MilDmrResult);
   MbufFree(MilImage);

   /* Free defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }
