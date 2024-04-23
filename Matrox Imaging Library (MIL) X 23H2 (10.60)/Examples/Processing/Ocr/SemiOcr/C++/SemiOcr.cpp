//***************************************************************************************
// 
// File name: SemiOcr.cpp  
//
// Synopsis: This example first performs an automatic background subtraction using
//           morphological operations before reading a string composed of SEMI fonts.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

///****************************************************************************
// Constants.
//*****************************************************************************
#define NUMBER_OF_CLOSE_ITERATIONS                10
#define NB_CHAR                                   13
#define EXAMPLE_OCR_PATH                          M_IMAGE_PATH MIL_TEXT("SemiOcr/")

// Source image files. 
const MIL_INT NUMBER_OF_IMAGES = 2;
MIL_CONST_TEXT_PTR IMAGE_LIST[NUMBER_OF_IMAGES] =
   {
   EXAMPLE_OCR_PATH MIL_TEXT("SemiOcr1.mim"),
   EXAMPLE_OCR_PATH MIL_TEXT("SemiOcr2.mim")
   };

MIL_CONST_TEXT_PTR CONTEXT_PATH = EXAMPLE_OCR_PATH MIL_TEXT("SEMI_M12-92_01.mfo");

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("SemiOcr\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example performs an automatic background subtraction using morphological\n"));
   MosPrintf(MIL_TEXT("operations before reading a string composed of SEMI fonts.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, graphic, \n"));
   MosPrintf(MIL_TEXT("image processing, OCR.\n\n"));
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_ID     MilApplication = M_NULL,   // Application identifier.
              MilSystem      = M_NULL,   // System identifier.
              MilDisplay     = M_NULL,   // Display identifier.
              MilGraphicList = M_NULL,   // Graphic List buffer identifier.
              MilOcrContext  = M_NULL,   // OCR context identifier.
              MilStructElem  = M_NULL,   // Structuring element buffer identifier.
              MilOcrResult   = M_NULL,   // OCR result buffer identifier.
              MilImage       = M_NULL,   // Image buffer identifier.
              MilTmp8Image   = M_NULL,   // Image buffer identifier.
              MilTmp16Image  = M_NULL;   // Image buffer identifier.
   MIL_INT    NumStringRead = 0;         // Number of string read.
   MIL_DOUBLE PositionX[NB_CHAR],        // Position of the characters read.
              PositionY[NB_CHAR];        // Position of the characters read. 
   MIL_TEXT_CHAR  ResultString[NB_CHAR]; // Array of characters to read.

   // Print Header. 
   PrintHeader();
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate MIL objects.
   MappAlloc(MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Allocate a linear structuring element and set its elements to 0.
   MbufAlloc1d(MilSystem, 7, 32 + M_UNSIGNED, M_STRUCT_ELEMENT, &MilStructElem);
   MbufClear(MilStructElem, 0);

   // Restore the OCR context.
   MocrRestoreFont(CONTEXT_PATH, M_RESTORE, MilSystem, &MilOcrContext);
   MocrPreprocess(MilOcrContext, M_DEFAULT);

   // Allocate an OCR result buffer.
   MocrAllocResult(MilSystem, M_DEFAULT, &MilOcrResult);

   for (MIL_INT ImageIndex = 0; ImageIndex < NUMBER_OF_IMAGES; ImageIndex++)
      {
      // Clear the graphic list.
      MgraClear(M_DEFAULT, MilGraphicList);

      // Restore the buffer.
      MIL_INT SizeX = MbufDiskInquire(IMAGE_LIST[ImageIndex], M_SIZE_X, M_NULL);
      MIL_INT SizeY = MbufDiskInquire(IMAGE_LIST[ImageIndex], M_SIZE_Y, M_NULL);

      MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilImage);
      MbufLoad(IMAGE_LIST[ImageIndex], MilImage);

      // Allocate temporary buffers.
      MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilTmp8Image);
      MbufAlloc2d(MilSystem, SizeX, SizeY, 16 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilTmp16Image);

      MdispSelect(MilDisplay, MilImage);
      MosPrintf(MIL_TEXT("A new image is loaded.\nPress <Enter> to continue.\n\n"));
      MosGetch();

      // Create an estimate of the background.
      MimMorphic(MilImage, MilTmp8Image, MilStructElem, M_CLOSE, NUMBER_OF_CLOSE_ITERATIONS, M_GRAYSCALE);

      MdispSelect(MilDisplay, MilTmp8Image);
      MosPrintf(MIL_TEXT("The background estimation is displayed.\nPress <Enter> to continue.\n\n"));
      MosGetch();

      // Divide the source image by the background image.
      MimArith(MilImage, 255, MilTmp16Image, M_MULT_CONST);
      MimArith(MilTmp16Image, MilTmp8Image, MilTmp8Image, M_DIV);

      // Read the string in the preprocessed image.
      MocrReadString(MilTmp8Image, MilOcrContext, MilOcrResult);
      MocrGetResult(MilOcrResult, M_NB_STRING + M_TYPE_MIL_INT, &NumStringRead);

      if (NumStringRead == 1)
         {
         MIL_INT StringSize = 0;

         MocrGetResult(MilOcrResult, M_STRING, ResultString);
         MocrGetResult(MilOcrResult, M_CHAR_POSITION_X, PositionX);
         MocrGetResult(MilOcrResult, M_CHAR_POSITION_Y, PositionY);

         // Print the results in the console.
         MosPrintf(MIL_TEXT("The background estimation is subtracted from the\n"));
         MosPrintf(MIL_TEXT("original image. The OCR operation is then performed.\n"));
         MosPrintf(MIL_TEXT("The string read is: \"%s\".\n\n"), ResultString);

         MgraColor(M_DEFAULT, M_COLOR_BLUE);
         MocrDraw(M_DEFAULT, MilOcrResult, MilGraphicList, M_DRAW_STRING_CHAR_BOX, M_DEFAULT, M_NULL, M_DEFAULT);
         MocrDraw(M_DEFAULT, MilOcrResult, MilGraphicList, M_DRAW_STRING_CHAR_POSITION, M_DEFAULT, M_NULL, M_DEFAULT);

         MgraColor(M_DEFAULT, M_COLOR_GREEN);
         MgraFont(M_DEFAULT, M_FONT_DEFAULT_MEDIUM);
         MgraText(M_DEFAULT, MilGraphicList, PositionX[0], PositionY[0] + 50, ResultString);

         MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
         MosGetch();
         }

      MbufFree(MilTmp8Image);
      MbufFree(MilTmp16Image);
      MbufFree(MilImage);
      }

   // Release the allocated MIL objects.
   MocrFree(MilOcrResult);
   MocrFree(MilOcrContext);
   MbufFree(MilStructElem);
   MgraFree(MilGraphicList);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   return 0;
   }
