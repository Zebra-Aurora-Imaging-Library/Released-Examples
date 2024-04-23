//******************************************************************************/
//
// File name: VariousDotMatrixReadings.cpp
//
// Synopsis:  This program uses the Dot Matrix Reader (SureDotOCR®) module
//            to read various strings on products.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <stdlib.h>

#define EXAMPLE_IMAGE_ROOT M_IMAGE_PATH MIL_TEXT("VariousDotMatrixReadings/")

const MIL_INT STRING_MAX_SIZE = 127;

/* Utility functions. */
void InitDisplay(MIL_ID MilSystem, MIL_ID MilImage, MIL_ID MilDisplay);
void GetAndDrawResults(MIL_ID MilDmrResult, MIL_ID MilOverlayImage);

/* Examples. */
void ReadLotAndExp(MIL_ID MilSystem, MIL_ID MilDisplay);
void ReadCanLid(MIL_ID MilSystem, MIL_ID MilDisplay);
void ReadProductDate(MIL_ID MilSystem, MIL_ID MilDisplay);
void ReadBestBy(MIL_ID MilSystem, MIL_ID MilDisplay);
void ReadProductNumber(MIL_ID MilSystem, MIL_ID MilDisplay);
void ReadLotAndBestBy(MIL_ID MilSystem, MIL_ID MilDisplay);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("VariousDotMatrixReadings\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how the MIL Dot Matrix Reader (SureDotOCR) module\n"));
   MosPrintf(MIL_TEXT("can read strings while dealing with various conditions such as distortion,\n"));
   MosPrintf(MIL_TEXT("contrast, texture, rotation, inherent image complexities, skew deformation,\n"));
   MosPrintf(MIL_TEXT("and strong uneven dot spacing.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, buffer, display, dot matrix reader, graphic.\n"));
   }

/*****************************************************************************/
int MosMain(void)
   {
   MIL_ID MilApplication,  /* Application identifier.          */
          MilSystem,       /* System identifier.               */
          MilDisplay,      /* Image buffer identifier.         */
          MilGraphLst;     /* Graphics list for annotations.   */

   PrintHeader();
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Allocate MIL objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphLst);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphLst);

   MosPrintf(MIL_TEXT("Reading a lot number and an expiry date with distortion.\n\n"));
   ReadLotAndExp(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("Reading strings on a can lid with rotation.\n\n"));
   ReadCanLid(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("Reading a product date at fixed angle and with uneven dot spacings.\n\n"));
   ReadProductDate(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("Reading \"best by\" date and product number with\n")
             MIL_TEXT("skew deformation.\n\n"));
   ReadBestBy(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("Reading product number on a non-uniform surface and\n")
             MIL_TEXT("with strong uneven dot spacing.\n\n"));
   ReadProductNumber(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("Reading a lot number and \"best by\" date with different string size.\n"));
   ReadLotAndBestBy(MilSystem, MilDisplay);

   /* Free objects. */
   MdispFree(MilDisplay);
   MgraFree(MilGraphLst);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

/* Initialize the display with a new image. */
void InitDisplay(MIL_ID MilSystem,
                 MIL_ID MilImage,
                 MIL_ID MilDisplay)
   {
   /* Display the image and prepare for overlay annotations. */
   MIL_ID AssociatedGrphLst = M_NULL;
   MdispInquire(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, &AssociatedGrphLst);
   MgraClear(M_DEFAULT, AssociatedGrphLst);

   MdispSelect(MilDisplay, MilImage);
   }

/* Retrieves DMR results and draw annotations. */
void GetAndDrawResults(MIL_ID MilDmrResult, MIL_ID MilDisplay)
   {
   MIL_ID DispAnnotations = M_NULL;
   MdispInquire(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, &DispAnnotations);

   MIL_INT NumberOfStringRead;                       /* Total number of strings to read. */
   MIL_TEXT_CHAR StringResult[STRING_MAX_SIZE + 1];  /* String of characters read.       */

   /* Get number of strings read and show the result. */
   MIL_INT ReadOpStatus = M_READ_NOT_PERFORMED;
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &ReadOpStatus);
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STRING_NUMBER + M_TYPE_MIL_INT, &NumberOfStringRead);

   if ((ReadOpStatus == M_COMPLETE) && (NumberOfStringRead >= 1))
      {
      /* Draw read result. */
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      MgraColor(M_DEFAULT, M_COLOR_DARK_BLUE);
      MdmrDraw(M_DEFAULT, MilDmrResult, DispAnnotations, M_DRAW_STRING_CHAR_BOX, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_CYAN);
      MdmrDraw(M_DEFAULT, MilDmrResult, DispAnnotations, M_DRAW_STRING_CHAR_POSITION, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MdmrDraw(M_DEFAULT, MilDmrResult, DispAnnotations, M_DRAW_STRING_BOX, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      MdmrDraw(M_DEFAULT, MilDmrResult, DispAnnotations, M_DRAW_MIL_FONT_STRING, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);

      /* Print the read result. */
      MosPrintf(MIL_TEXT(" String \n"));
      MosPrintf(MIL_TEXT(" -----------------------------------\n"));
      for (MIL_INT ii = 0; ii < NumberOfStringRead; ii++)
         {
         MIL_INT StringSize = 0;
         MdmrGetResult(MilDmrResult, ii, M_GENERAL, M_STRING + M_STRING_SIZE + M_TYPE_MIL_INT, &StringSize);

         MdmrGetResult(MilDmrResult, ii, M_GENERAL, M_FORMATTED_STRING, &StringResult[0]);

         MosPrintf(MIL_TEXT(" %s \n"), StringResult);
         }
      MosPrintf(MIL_TEXT("\n"));
      }
   else
      {
      MosPrintf(MIL_TEXT("Error: the string was not read.\n\n"));
      switch(ReadOpStatus)
         {
         case M_TIMEOUT_REACHED:
            {
            MosPrintf(MIL_TEXT("The read operation reached M_TIMEOUT before its completion.\n\n"));
            MosPrintf(MIL_TEXT("If running the example under Microsoft Visual Studio in 'debugging'\n"));
            MosPrintf(MIL_TEXT("mode, you may consider using the _NO_DEBUG_HEAP=1 environment\n"));
            MosPrintf(MIL_TEXT("variable to accelerate memory allocations for this application.\n"));
            MosPrintf(MIL_TEXT("While useful for debugging applications, 'debug heaps' may cause\n"));
            MosPrintf(MIL_TEXT("the application to run much slower.\n"));
            } break;

         case M_NOT_ENOUGH_MEMORY:
            {
            MosPrintf(MIL_TEXT("Not enough memory to complete the read operation.\n"));
            } break;

         case M_READ_NOT_PERFORMED:
            {
            MosPrintf(MIL_TEXT("No read operation was done on the result.\n"));
            } break;

         default:
            {
            MosPrintf(MIL_TEXT("Unexpected read operation status.\n"));
            } break;
         }
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

/*****************************************************************************/
void ReadLotAndExp(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID MilImage,        /* Source image.                    */
          MilDmrContext,   /* String context identifier.       */
          MilDmrResult;    /* String result buffer identifier. */

   /* Files. */
   MIL_CONST_TEXT_PTR ImageFilename = EXAMPLE_IMAGE_ROOT  MIL_TEXT("LotAndExp.bmp");
   MIL_CONST_TEXT_PTR FontFilename  = EXAMPLE_IMAGE_ROOT  MIL_TEXT("LotAndExp.mdmrf");

   /* Allocate a new empty Dot matrix reader context. */
   MdmrAlloc(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrContext);

   /* Allocate a new empty Dot matrix reader result buffer. */
   MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrResult);

   /* Import a dot matrix font. */
   MdmrImportFont(FontFilename, M_DMR_FONT_FILE, MilDmrContext, M_DEFAULT, M_NULL, M_DEFAULT);

   /* Import the source image. */
   MbufImport(ImageFilename, M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

   /* Basic settings. */
   const MIL_DOUBLE DotDiameter     = 10.0;
   const MIL_DOUBLE TextBlockWidth  = 645.0;
   const MIL_DOUBLE TextBlockHeight = 170.0;
   const MIL_INT    StringSize      = 10;

   /* Initialize display. */
   InitDisplay(MilSystem, MilImage, MilDisplay);

   /* Add string model definition. */
   const MIL_INT StrModLbl1 = 1;
   MdmrControl(MilDmrContext, M_STRING_ADD, StrModLbl1);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_RANK, 0, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_SIZE_MIN_MAX, StringSize, StringSize, M_NULL);

   const MIL_INT StrModLbl2 = 2;
   MdmrControl(MilDmrContext, M_STRING_ADD, StrModLbl2);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl2), M_DEFAULT, M_STRING_RANK, 1, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl2), M_DEFAULT, M_STRING_SIZE_MIN_MAX, StringSize, StringSize, M_NULL);

   MdmrControl(MilDmrContext, M_DOT_DIAMETER, DotDiameter);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_WIDTH, TextBlockWidth);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_HEIGHT, TextBlockHeight);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Reading the string into a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   /* Retrieve the result and draw annotations. */
   GetAndDrawResults(MilDmrResult, MilDisplay);

   /* Free all allocations. */
   MdmrFree(MilDmrContext);
   MdmrFree(MilDmrResult);
   MbufFree(MilImage);
   }

/*****************************************************************************/
void ReadCanLid(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID MilImage,        /* Source image.                    */
          MilDmrContext,   /* String context identifier.       */
          MilDmrResult;    /* String result buffer identifier. */

   /* Files. */
   MIL_CONST_TEXT_PTR ImageFilename = EXAMPLE_IMAGE_ROOT  MIL_TEXT("CanLidString.bmp");
   MIL_CONST_TEXT_PTR FontFilename  = EXAMPLE_IMAGE_ROOT  MIL_TEXT("CanLidString.mdmrf");

   /* Allocate a new empty Dot matrix reader context. */
   MdmrAlloc(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrContext);

   /* Allocate a new empty Dot matrix reader result buffer. */
   MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrResult);

   /* Import a dot matrix font. */
   MdmrImportFont(FontFilename, M_DMR_FONT_FILE, MilDmrContext, M_DEFAULT, M_NULL, M_DEFAULT);

   /* Import the source image. */
   MbufImport(ImageFilename, M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

   /* Basic settings. */
   const MIL_DOUBLE DotDiameter     = 8.0;
   const MIL_DOUBLE TextBlockWidth  = 840.0;
   const MIL_DOUBLE TextBlockHeight = 200.0;
   const MIL_INT    StringSize_1    = 4;
   const MIL_INT    StringSize_2    = 12;

   /* Initialize display. */
   InitDisplay(MilSystem, MilImage, MilDisplay);

   /* Add string model definition. */
   const MIL_INT StrModLbl1 = 1;
   MdmrControl(MilDmrContext, M_STRING_ADD, StrModLbl1);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_RANK, 0, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_SIZE_MIN_MAX, StringSize_1, StringSize_1, M_NULL);

   const MIL_INT StrModLbl2 = 2;
   MdmrControl(MilDmrContext, M_STRING_ADD, StrModLbl2);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl2), M_DEFAULT, M_STRING_RANK, 1, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl2), M_DEFAULT, M_STRING_SIZE_MIN_MAX, StringSize_2, StringSize_2, M_NULL);

   MdmrControl(MilDmrContext, M_DOT_DIAMETER, DotDiameter);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_WIDTH, TextBlockWidth);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_HEIGHT, TextBlockHeight);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Reading the string into a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   /* Retrieve the result and draw annotations. */
   GetAndDrawResults(MilDmrResult, MilDisplay);

   /* Free all allocations. */
   MdmrFree(MilDmrContext);
   MdmrFree(MilDmrResult);
   MbufFree(MilImage);
   }

/*****************************************************************************/
void ReadProductDate(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID MilImage,        /* Source image.                    */
          MilDmrContext,   /* String context identifier.       */
          MilDmrResult;    /* String result buffer identifier. */

   /* Files. */
   MIL_CONST_TEXT_PTR ImageFilename = EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductDate.bmp");
   MIL_CONST_TEXT_PTR FontFilename  = EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductDate.mdmrf");

   /* Allocate a new empty Dot matrix reader context. */
   MdmrAlloc(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrContext);

   /* Allocate a new empty Dot matrix reader result buffer. */
   MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrResult);

   /* Import a dot matrix font. */
   MdmrImportFont(FontFilename, M_DMR_FONT_FILE, MilDmrContext, M_DEFAULT, M_NULL, M_DEFAULT);

   /* Import the source image. */
   MbufImport(ImageFilename, M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

   /* Basic settings. */
   const MIL_DOUBLE DotDiameter     = 4.0;
   const MIL_DOUBLE TextBlockWidth  = 250.0;
   const MIL_DOUBLE TextBlockHeight = 60.0;
   const MIL_INT    StringSize      = 11;

   /* String angle settings. */
   const MIL_INT    StringAngleMode = M_ANGLE;
   const MIL_DOUBLE StringAngle     = 0.0;

   /* Initialize display. */
   InitDisplay(MilSystem, MilImage, MilDisplay);

   /* Add string model definition. */
   const MIL_INT StrModLbl1 = 1;
   MdmrControl(MilDmrContext, M_STRING_ADD, StrModLbl1);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_RANK, 0, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_SIZE_MIN_MAX, StringSize, StringSize, M_NULL);

   MdmrControl(MilDmrContext, M_DOT_DIAMETER, DotDiameter);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_WIDTH, TextBlockWidth);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_HEIGHT, TextBlockHeight);

   /* Set a specific string angle. */
   MdmrControl(MilDmrContext, M_STRING_ANGLE_MODE, StringAngleMode);
   MdmrControl(MilDmrContext, M_STRING_ANGLE, StringAngle);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Reading the string into a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   /* Retrieve the result and draw annotations. */
   GetAndDrawResults(MilDmrResult, MilDisplay);

   /* Free all allocations. */
   MdmrFree(MilDmrContext);
   MdmrFree(MilDmrResult);
   MbufFree(MilImage);
   }

/*****************************************************************************/
void ReadBestBy(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID MilImage,        /* Source image.                    */
          MilDmrContext,   /* String context identifier.       */
          MilDmrResult;    /* String result buffer identifier. */

   /* Files. */
   MIL_CONST_TEXT_PTR ImageFilename = EXAMPLE_IMAGE_ROOT  MIL_TEXT("BestBy.bmp");
   MIL_CONST_TEXT_PTR FontFilename  = EXAMPLE_IMAGE_ROOT  MIL_TEXT("BestBy.mdmrf");

   /* Allocate a new empty Dot matrix reader context. */
   MdmrAlloc(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrContext);

   /* Allocate a new empty Dot matrix reader result buffer. */
   MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrResult);

   /* Import a dot matrix font. */
   MdmrImportFont(FontFilename, M_DMR_FONT_FILE, MilDmrContext, M_DEFAULT, M_NULL, M_DEFAULT);

   /* Import the source image. */
   MbufImport(ImageFilename, M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

   /* Basic settings. */
   const MIL_DOUBLE DotDiameter     = 5.0;
   const MIL_DOUBLE TextBlockWidth  = 430.0;
   const MIL_DOUBLE TextBlockHeight = 130.0;
   const MIL_INT    StringSize_1    = 17;
   const MIL_INT    StringSize_2    = 15;

   /* Initialize display. */
   InitDisplay(MilSystem, MilImage, MilDisplay);

   /* Add string model definition. */
   const MIL_INT StrModLbl1 = 1;
   MdmrControl(MilDmrContext, M_STRING_ADD, StrModLbl1);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_RANK, 0, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_SIZE_MIN_MAX, StringSize_1, StringSize_1, M_NULL);

   const MIL_INT StrModLbl2 = 2;
   MdmrControl(MilDmrContext, M_STRING_ADD, StrModLbl2);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl2), M_DEFAULT, M_STRING_RANK, 1, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl2), M_DEFAULT, M_STRING_SIZE_MIN_MAX, StringSize_2, StringSize_2, M_NULL);

   MdmrControl(MilDmrContext, M_DOT_DIAMETER, DotDiameter);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_WIDTH, TextBlockWidth);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_HEIGHT, TextBlockHeight);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Reading the string into a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   /* Retrieve the result and draw annotations. */
   GetAndDrawResults(MilDmrResult, MilDisplay);

   /* Free all allocations. */
   MdmrFree(MilDmrContext);
   MdmrFree(MilDmrResult);
   MbufFree(MilImage);
   }

/*****************************************************************************/
void ReadProductNumber(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID MilImage,        /* Source image.                    */
          MilDmrContext,   /* String context identifier.       */
          MilDmrResult;    /* String result buffer identifier. */

   /* Files. */
   MIL_CONST_TEXT_PTR ImageFilename = EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductNumber.bmp");
   MIL_CONST_TEXT_PTR FontFilename  = EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductNumber.mdmrf");

   /* Allocate a new empty Dot matrix reader context. */
   MdmrAlloc(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrContext);

   /* Allocate a new empty Dot matrix reader result buffer. */
   MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrResult);

   /* Import a dot matrix font. */
   MdmrImportFont(FontFilename, M_DMR_FONT_FILE, MilDmrContext, M_DEFAULT, M_NULL, M_DEFAULT);

   /* Import the source image. */
   MbufImport(ImageFilename, M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

   /* Basic settings. */
   const MIL_DOUBLE DotDiameter     = 5.0;
   const MIL_DOUBLE TextBlockWidth  = 320.0;
   const MIL_DOUBLE TextBlockHeight = 60.0;
   const MIL_INT    StringSize      = 12;

   /* Initialize display. */
   InitDisplay(MilSystem, MilImage, MilDisplay);

   /* Add string model definition. */
   const MIL_INT StrModLbl1 = 1;
   MdmrControl(MilDmrContext, M_STRING_ADD, StrModLbl1);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_RANK, 0, M_DEFAULT, M_NULL);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_SIZE_MIN_MAX, StringSize, StringSize, M_NULL);

   /* Add positional constraints. */
   MIL_INT PositionEntryTypes[StringSize] = {
      //  0         1         2         3
      M_DIGITS, M_DIGITS, M_DIGITS, M_DIGITS,
      //  4
      M_DIGITS,
      //  5                6
      M_LETTERS_UPPERCASE, M_DIGITS,
      //  7         8       9       10        11
      M_DIGITS, M_DIGITS, M_ANY, M_DIGITS, M_DIGITS
      };

   for (MIL_INT p = 0; p < StringSize; p++)
      {
      MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_POSITION_IN_STRING(p), M_ADD_PERMITTED_CHARS_ENTRY, M_FONT_LABEL(M_ANY), PositionEntryTypes[p], M_NULL);
      }

   MdmrControl(MilDmrContext, M_DOT_DIAMETER, DotDiameter);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_WIDTH, TextBlockWidth);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_HEIGHT, TextBlockHeight);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Reading the string into a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   /* Retrieve the result and draw annotations. */
   GetAndDrawResults(MilDmrResult, MilDisplay);

   /* Free all allocations. */
   MdmrFree(MilDmrContext);
   MdmrFree(MilDmrResult);
   MbufFree(MilImage);
   }


/*****************************************************************************/
void ReadLotAndBestBy(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID MilImage,        /* Source image.                    */
          MilDmrContext,   /* String context identifier.       */
          MilDmrResult;    /* String result buffer identifier. */

   /* Files. */
   const MIL_INT ImageNumber = 4;
   MIL_CONST_TEXT_PTR ImageFilename[ImageNumber] = { EXAMPLE_IMAGE_ROOT  MIL_TEXT("LotAndBestBy_0.png"),
                                                     EXAMPLE_IMAGE_ROOT  MIL_TEXT("LotAndBestBy_1.png"),
                                                     EXAMPLE_IMAGE_ROOT  MIL_TEXT("LotAndBestBy_2.png"),
                                                     EXAMPLE_IMAGE_ROOT  MIL_TEXT("LotAndBestBy_3.png")
                                                   };

   MIL_CONST_TEXT_PTR FontFilename = EXAMPLE_IMAGE_ROOT  MIL_TEXT("LotAndBestBy.mdmrf");

   /* Allocate a new empty SureDotOCR context. */
   MdmrAlloc(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrContext);

   /* Allocate a new empty SureDotOCR result buffer. */
   MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, &MilDmrResult);

   /* Basic settings. */
   const MIL_DOUBLE DotDiameter = 4.0;
   const MIL_DOUBLE TextBlockWidth = 656.0;
   const MIL_DOUBLE TextBlockHeight = 124.0;

   MdmrControl(MilDmrContext, M_DOT_DIAMETER, DotDiameter);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_WIDTH, TextBlockWidth);
   MdmrControl(MilDmrContext, M_TEXT_BLOCK_HEIGHT, TextBlockHeight);

   /* Import a dot matrix font. */
   MdmrImportFont(FontFilename, M_DMR_FONT_FILE, MilDmrContext, M_DEFAULT, M_NULL, M_DEFAULT);

   /* Add string model definition. */
   const MIL_INT StrModLbl1 = 1;
   MdmrControl(MilDmrContext, M_STRING_ADD, StrModLbl1);
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_RANK, 0, M_DEFAULT, M_NULL);

   /* Adjust string size min and max to read strings with sizes between 18 and 21. */
   const MIL_INT    StringSizeMin = 18;
   const MIL_INT    StringSizeMax = 21;
   MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_DEFAULT, M_STRING_SIZE_MIN_MAX, StringSizeMin, StringSizeMax, M_NULL);

   /* Add positional constraints. */
   MIL_INT PositionEntryTypes[StringSizeMax] = {
      //  0         1          2    
      M_DIGITS, M_LETTERS, M_DIGITS,
      //  3         4          5           6
      M_LETTERS, M_LETTERS, M_LETTERS, M_LETTERS,
      //  7         8
      M_LETTERS, M_LETTERS,
      //  9         10         11
      M_LETTERS, M_LETTERS, M_LETTERS,
      //  12        13      
      M_DIGITS, M_DIGITS,
      //  14        15       16        17
      M_DIGITS, M_DIGITS, M_DIGITS, M_DIGITS,
      //  18        19        20
      M_LETTERS, M_LETTERS, M_LETTERS,
      };

   for(MIL_INT p = 0; p < StringSizeMax; p++)
      {
      MdmrControlStringModel(MilDmrContext, M_STRING_LABEL(StrModLbl1), M_POSITION_IN_STRING(p), M_ADD_PERMITTED_CHARS_ENTRY, M_FONT_LABEL(M_ANY), PositionEntryTypes[p], M_NULL);
      }
   
   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   MosPrintf(MIL_TEXT("\nThe following images with cropped strings will be read using only one\n"));
   MosPrintf(MIL_TEXT("string model with string size min and max set to 18 and 21.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Get number of images to read. */
   for(MIL_INT Index = 0; Index < ImageNumber; Index++)
      {
      /* Import the source image. */
      MbufImport(ImageFilename[Index], M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

      /* Initialize display. */
      InitDisplay(MilSystem, MilImage, MilDisplay);

      /* Reading the string in a target image. */
      MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

      /* Retrieve the result and draw annotations. */
      GetAndDrawResults(MilDmrResult, MilDisplay);

      MbufFree(MilImage);
      }

   /* Free all allocations. */
   MdmrFree(MilDmrContext);
   MdmrFree(MilDmrResult);
   }
