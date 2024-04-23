﻿//******************************************************************************/
//
// File name: DMRTipsAndTricks.cpp
//
// Synopsis:  This program uses the SureDotOCR® module (Dot Matrix Reader)
//            to read strings on products, demonstrating useful controls that
//            can help you deal with problematic conditions, such as
//            strong dot fusion and strong variation in dot spacing.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <stdlib.h>

#define EXAMPLE_IMAGE_ROOT M_IMAGE_PATH MIL_TEXT("DmrTipsAndTricks/")

/* Utility functions. */
using ReadingInfoPair = std::pair<bool, MIL_DOUBLE>;

void InitDisplay(MIL_ID MilSystem, MIL_ID MilImage, MIL_ID MilDisplay);
bool GetAndDrawResults(MIL_ID MilDmrResult, MIL_ID MilOverlayImage, bool SkipGetChar = false);
void PrintReadInfoPairs(std::vector<ReadingInfoPair> &ReadingInfo);

/* Examples. */
void ReadExpAndLot(MIL_ID MilSystem, MIL_ID MilDisplay);
void ReadProductDate(MIL_ID MilSystem, MIL_ID MilDisplay);
void ReadCanLid(MIL_ID MilSystem, MIL_ID MilDisplay);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("DMRTipsAndTricks\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program uses the SureDotOCR® module (Dot Matrix Reader)\n"));
   MosPrintf(MIL_TEXT("to read strings on products, demonstrating useful controls that\n"));
   MosPrintf(MIL_TEXT("can help you deal with problematic conditions, such as\n"));
   MosPrintf(MIL_TEXT("strong dot fusion and strong variation in dot spacing.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, buffer, display, SureDotOCR (DMR), graphic.\n"));
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

   MosPrintf(MIL_TEXT("=================================================================\n"));
   MosPrintf(MIL_TEXT("Reading an expiry date and a lot number at fixed angle and pitch.\n"));
   MosPrintf(MIL_TEXT("=================================================================\n"));
   ReadExpAndLot(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("============================================================\n"));
   MosPrintf(MIL_TEXT("Reading a product date with strong variation in dot spacing.\n"));
   MosPrintf(MIL_TEXT("============================================================\n"));
   ReadProductDate(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("============================================================\n"));
   MosPrintf(MIL_TEXT("Reading a can lid with strong deformation.\n"));
   MosPrintf(MIL_TEXT("============================================================\n"));
   ReadCanLid(MilSystem, MilDisplay);

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

   MdispZoom(MilDisplay, 0.5, 0.5);
   MdispSelect(MilDisplay, MilImage);
   }

/* Retrieves DMR results and draw annotations. */
bool GetAndDrawResults(MIL_ID MilDmrResult, MIL_ID MilDisplay, bool SkipGetChar /*= false*/)
   {
   bool ReadFlag = false;
   MIL_ID DispAnnotations = M_NULL;
   MdispInquire(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, &DispAnnotations);

   MIL_INT NumberOfStringRead;                       /* Total number of strings to read. */
   MIL_STRING StringResult;                          /* String of characters read.       */

   /* Get number of strings read and show the result. */
   MIL_INT ReadOpStatus = M_READ_NOT_PERFORMED;
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &ReadOpStatus);
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STRING_NUMBER + M_TYPE_MIL_INT, &NumberOfStringRead);

   if ((ReadOpStatus == M_COMPLETE) && (NumberOfStringRead >= 1))
      {
      ReadFlag = true;
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
      MosPrintf(MIL_TEXT(" -------\n"));
      for (MIL_INT ii = 0; ii < NumberOfStringRead; ii++)
         {
         MIL_INT StringSize = 0;
         MdmrGetResult(MilDmrResult, ii, M_GENERAL, M_STRING + M_STRING_SIZE + M_TYPE_MIL_INT, &StringSize);

         MdmrGetResult(MilDmrResult, ii, M_GENERAL, M_FORMATTED_STRING, StringResult);

         MosPrintf(MIL_TEXT(" %s \n"), StringResult.c_str());
         }
      MosPrintf(MIL_TEXT("\n"));
      }
   else
      {
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MgraText(M_DEFAULT, DispAnnotations, 21, 26, MIL_TEXT("No string was read"));

      switch(ReadOpStatus)
         {
         case M_TIMEOUT_REACHED:
            {
            MosPrintf(MIL_TEXT("The read operation reached M_TIMEOUT before its completion.\n\n"));
            MosPrintf(MIL_TEXT("If running the example under Microsoft Visual Studio in 'debugging'\n"));
            MosPrintf(MIL_TEXT("mode, you may consider using the _NO_DEBUG_HEAP=1 environment\n"));
            MosPrintf(MIL_TEXT("variable to accelerate memory allocations for this application.\n"));
            MosPrintf(MIL_TEXT("While useful for debugging applications, 'debug heaps' may cause\n"));
            MosPrintf(MIL_TEXT("the application to run much slower.\n\n"));
            } break;

         case M_NOT_ENOUGH_MEMORY:
            {
            MosPrintf(MIL_TEXT("Not enough memory to complete the read operation.\n\n"));
            } break;

         case M_READ_NOT_PERFORMED:
            {
            MosPrintf(MIL_TEXT("No read operation was done on the result.\n\n"));
            } break;

         default:
            {
            MosPrintf(MIL_TEXT("No string was read.\n\n"));
            } break;
         }
      }

   if(!SkipGetChar)
      {
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }

   return ReadFlag;
   }

/* Print cumulative reading info table. */
void PrintReadInfoPairs(std::vector<ReadingInfoPair> &ReadingInfo)
   {
   MIL_INT NbSuccessfulRead = 0;
   MIL_INT ImageIndex = 0;
   MosPrintf(MIL_TEXT("------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Image Index    |  Status    |  Time in ms \n"));
   MosPrintf(MIL_TEXT("------------------------------------------\n"));

   for(const ReadingInfoPair& CurrentPair : ReadingInfo)
      {
      if(CurrentPair.first)
         {
         NbSuccessfulRead++;
         MosPrintf(MIL_TEXT("      %2d          Read         %.1f\n"), ImageIndex, CurrentPair.second * 1000);
         }
      else
         MosPrintf(MIL_TEXT("      %2d          No Read      %.1f\n"), ImageIndex, CurrentPair.second * 1000);

      ImageIndex++;
      }

   MosPrintf(MIL_TEXT("------------------------------------------\n\n"));

   MosPrintf(MIL_TEXT("Successful Read : %d out of %d images.\n\n"), NbSuccessfulRead, ReadingInfo.size());

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

/*****************************************************************************/
void ReadExpAndLot(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_UNIQUE_BUF_ID MilImage;

   MIL_DOUBLE Time = 0.0;  /* Bench variable.                  */

   /* Files. */
   const MIL_STRING ImageFilename   = EXAMPLE_IMAGE_ROOT  MIL_TEXT("ExpAndLot.bmp");
   const MIL_STRING ContextFilename = EXAMPLE_IMAGE_ROOT  MIL_TEXT("ExpAndLot.mdmr");

   /* Allocate a new empty SureDotOCR result buffer. */
   MIL_UNIQUE_DMR_ID MilDmrResult = MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   /* Restoring a SureDotOCR context. */
   MIL_UNIQUE_DMR_ID MilDmrContext = MdmrRestore(ContextFilename, MilSystem, M_DEFAULT, M_UNIQUE_ID);

   /* Import the source image. */
   MbufImport(ImageFilename, M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

   MosPrintf(MIL_TEXT("\nApplications where the string and italic angles and pitches are consistant from one image\n"));
   MosPrintf(MIL_TEXT("to the other may benefit from setting these controls. This can be done by getting these results\n"));
   MosPrintf(MIL_TEXT("from a read of a typical image. This can help improving the spead and robustness especially \n"));
   MosPrintf(MIL_TEXT("when there are some dot fusion and/or variation in the spacing between dots.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Initialize display. */
   InitDisplay(MilSystem, MilImage, MilDisplay);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Reading the string into a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   /* Read the reading time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

   MosPrintf(MIL_TEXT("\nThe reading time is %.1f ms with the context where string, italic angles and\n"), Time*1000.0);
   MosPrintf(MIL_TEXT("pitches mode are set to M_AUTO.\n\n"));

   /* Retrieve the result and draw annotations. */
   GetAndDrawResults(MilDmrResult, MilDisplay);

   MIL_INT ReadOpStatus = M_READ_NOT_PERFORMED;
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &ReadOpStatus);
   MIL_INT NumberOfStringRead = 0;
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STRING_NUMBER + M_TYPE_MIL_INT, &NumberOfStringRead);

   /* Retrieve a specific char angle and pitch for string and italic. */
   MIL_DOUBLE StringAngle = 0.0;
   MIL_DOUBLE ItalicAngle = 0.0;
   MIL_DOUBLE StringPitch = 0.0;
   MIL_DOUBLE ItalicPitch = 0.0;

   if((ReadOpStatus == M_COMPLETE) && (NumberOfStringRead >= 1))
      {
      MdmrGetResult(MilDmrResult, 0, M_DEFAULT, M_STRING_CHAR_ANGLE, &StringAngle);
      MdmrGetResult(MilDmrResult, 0, M_DEFAULT, M_ITALIC_CHAR_ANGLE, &ItalicAngle);
      MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STRING_PITCH, &StringPitch);
      MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_ITALIC_PITCH, &ItalicPitch);
      }
   else
      {
      /* Use constant values. */
      StringAngle = 164.65;
      ItalicAngle = -1.08;
      StringPitch = 3.53;
      ItalicPitch = 5.21;
      }

   /* Set a specific string angle. */
   MdmrControl(MilDmrContext, M_STRING_ANGLE_MODE, M_ANGLE);
   MdmrControl(MilDmrContext, M_STRING_ANGLE, StringAngle);

   /* Set a specific italic angle. */
   MdmrControl(MilDmrContext, M_ITALIC_ANGLE_MODE, M_ANGLE);
   MdmrControl(MilDmrContext, M_ITALIC_ANGLE, ItalicAngle);

   /* Set a specific string pitch. */
   MdmrControl(MilDmrContext, M_STRING_PITCH_MODE, M_USER_DEFINED);
   MdmrControl(MilDmrContext, M_STRING_PITCH, StringPitch);

   /* Set a specific italic pitch. */
   MdmrControl(MilDmrContext, M_ITALIC_PITCH_MODE, M_USER_DEFINED);
   MdmrControl(MilDmrContext, M_ITALIC_PITCH, ItalicPitch);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Reading the string in a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   /* Read the reading time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

   MosPrintf(MIL_TEXT("\nThe reading time is %.1f ms after setting string, italic angles and\n"), Time*1000.0);
   MosPrintf(MIL_TEXT("pitches. Values used for context settings were retrieved from previous read\n"));
   MosPrintf(MIL_TEXT("results.\n\n"));

   /* Retrieve the result and draw annotations. */
   GetAndDrawResults(MilDmrResult, MilDisplay);

   }

/*****************************************************************************/
void ReadProductDate(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_DOUBLE Time = 0.0;  /* Bench variable.                  */

   /* Files. */
   std::vector<MIL_STRING> ImageFilename = {EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductDate_0.mim"),
                                            EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductDate_1.mim"),
                                            EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductDate_2.mim"),
                                            EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductDate_3.mim"),
                                            EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductDate_4.mim"),
                                            EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductDate_5.mim")
                                           };

   const MIL_STRING ContextFilename = EXAMPLE_IMAGE_ROOT  MIL_TEXT("ProductDate.mdmr");

   /* Allocate a new empty SureDotOCR result buffer. */
   MIL_UNIQUE_DMR_ID MilDmrResult = MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   /* Restoring a SureDotOCR context. */
   MIL_UNIQUE_DMR_ID MilDmrContext = MdmrRestore(ContextFilename, MilSystem, M_DEFAULT, M_UNIQUE_ID);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   MosPrintf(MIL_TEXT("\nApplications with dot diameter variation and/or with strong dots fusion may benefit\n"));
   MosPrintf(MIL_TEXT("from enabling the dot diameter spread control. This can help improve robustness.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("\nReading different images with a context where M_DOT_DIAMETER_SPREAD_MODE is\n"));
   MosPrintf(MIL_TEXT("set to M_DISABLE.\n\n"));

   /* Get number of images to read. */
   MIL_INT NbImages = (MIL_INT)ImageFilename.size();

   /* Allocate a vector to hold reading info. */
   std::vector<ReadingInfoPair> ReadingInfo(NbImages);

   for(MIL_INT Index = 0; Index < NbImages; Index++)
      {
      MosPrintf(MIL_TEXT("\nReading image %d out of %d\n"), Index + 1, NbImages);
      MosPrintf(MIL_TEXT("--------------------------\n"));

      MIL_UNIQUE_BUF_ID MilImage;

      /* Import the source image. */
      MbufImport(ImageFilename[Index], M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

      /* Initialize display. */
      InitDisplay(MilSystem, MilImage, MilDisplay);

      /* Reset the timer. */
      MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

      /* Reading the string into a target image. */
      MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

      /* Read the reading time. */
      MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

      MosPrintf(MIL_TEXT("\nThe reading time is %.1f ms.\n\n"), Time*1000.0);

      /* Retrieve the result and draw annotations. */
      ReadingInfo[Index].first = GetAndDrawResults(MilDmrResult, MilDisplay);
      ReadingInfo[Index].second = Time;
      }

   /* Print cumulative reading info table. */
   PrintReadInfoPairs(ReadingInfo);

   MosPrintf(MIL_TEXT("\nReading the same images after setting a tolerance for size of the dot diameter.\n"));

   /* Set a tolerance for size of the dot diameter. */
   MdmrControl(MilDmrContext, M_DOT_DIAMETER_SPREAD_MODE, M_ENABLE);
   MdmrControl(MilDmrContext, M_DOT_DIAMETER_SPREAD, 1.0);
   MdmrControl(MilDmrContext, M_DOT_DIAMETER_STEP, 0.1);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Allocate a vector to hold reading info. */
   std::vector<ReadingInfoPair> NewReadingInfo(NbImages);

   for(MIL_INT Index = 0; Index < NbImages; Index++)
      {
      MosPrintf(MIL_TEXT("\nReading image %d out of %d\n"), Index+1, NbImages);
      MosPrintf(MIL_TEXT("--------------------------\n"));

      MIL_UNIQUE_BUF_ID MilImage;

      /* Import the source image. */
      MbufImport(ImageFilename[Index], M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

      /* Initialize display. */
      InitDisplay(MilSystem, MilImage, MilDisplay);

      /* Reset the timer. */
      MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

      /* Reading the string in a target image. */
      MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

      /* Read the reading time. */
      MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

      MosPrintf(MIL_TEXT("\nThe reading time is %.1f ms.\n\n"), Time*1000.0);

      /* Retrieve the result and draw annotations. */
      NewReadingInfo[Index].first = GetAndDrawResults(MilDmrResult, MilDisplay);
      NewReadingInfo[Index].second = Time;
      }

   /* Print cumulative reading info table. */
   PrintReadInfoPairs(NewReadingInfo);
   }

/*****************************************************************************/
void ReadCanLid(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_UNIQUE_BUF_ID MilImage;

   /* Files. */
   const MIL_STRING ImageFilename   = EXAMPLE_IMAGE_ROOT  MIL_TEXT("CanLid.mim");
   const MIL_STRING ContextFilename = EXAMPLE_IMAGE_ROOT  MIL_TEXT("CanLid.mdmr");

   /* Allocate a new empty SureDotOCR result buffer. */
   MIL_UNIQUE_DMR_ID MilDmrResult = MdmrAllocResult(MilSystem, M_DOT_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   /* Restoring a SureDotOCR context. */
   MIL_UNIQUE_DMR_ID MilDmrContext = MdmrRestore(ContextFilename, MilSystem, M_DEFAULT, M_UNIQUE_ID);

   /* Import the source image. */
   MbufImport(ImageFilename, M_DEFAULT, M_RESTORE + M_NO_GRAB, MilSystem, &MilImage);

   MosPrintf(MIL_TEXT("\nWhen a read operation is not successful using SureDotOCR, enabling the string partial mode\n"));
   MosPrintf(MIL_TEXT("can help find the reason why. For example, the character might be badly print or wrongly\n"));
   MosPrintf(MIL_TEXT("defined in the font.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();


   MosPrintf(MIL_TEXT("\nFirst a read is performed with partial mode set to disable.\n\n"));

   /* Initialize display. */
   InitDisplay(MilSystem, MilImage, MilDisplay);

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   /* Reading the string in a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   MIL_INT ReadOpStatus = M_READ_NOT_PERFORMED;
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &ReadOpStatus);
   MIL_INT NumberOfStringRead = 0;
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STRING_NUMBER + M_TYPE_MIL_INT, &NumberOfStringRead);

   if((ReadOpStatus == M_COMPLETE) && (NumberOfStringRead == 0))
      {
      MosPrintf(MIL_TEXT("No string was read.\n\n"));

      MosPrintf(MIL_TEXT("This image cannot be read. We enable the partial string mode to help find the reason. \n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }
   else
      /* Retrieve the result and draw annotations. */
      GetAndDrawResults(MilDmrResult, MilDisplay);

   /* Set that the best match for a partial string will be returned; unrecognized characters in the string */
   /* will be replaced with M_STRING_PARTIAL_CHAR_INVALID. */
   MdmrControl(MilDmrContext, M_STRING_PARTIAL_MODE, M_ENABLE);

   /* Set the character to replace invalid or unrecognized characters. */
   /* Note: This value is validated during the preprocess operation to confirm that none of the selected characters */
   /*       appear in any font in this context. */
   MdmrControl(MilDmrContext, M_STRING_PARTIAL_CHAR_INVALID, MIL_TEXT("#"));

   /* Preprocess the context. */
   MdmrPreprocess(MilDmrContext, M_DEFAULT);

   MosPrintf(MIL_TEXT("\nPerform a read after enabling to return a partial string and setting the invalid character.\n"));
   MosPrintf(MIL_TEXT("The invalid or unrecognized characters are returned as \'#\'. They are drawn in yellow\n"));
   MosPrintf(MIL_TEXT("to highlight them.\n\n"));

   /* Reading the string in a target image. */
   MdmrRead(MilDmrContext, MilImage, MilDmrResult, M_DEFAULT);

   /* Retrieve the result and draw annotations. */
   GetAndDrawResults(MilDmrResult, MilDisplay, true);

   ReadOpStatus = M_READ_NOT_PERFORMED;
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &ReadOpStatus);
   NumberOfStringRead = 0;
   MdmrGetResult(MilDmrResult, M_GENERAL, M_GENERAL, M_STRING_NUMBER + M_TYPE_MIL_INT, &NumberOfStringRead);

   if((ReadOpStatus == M_COMPLETE) && (NumberOfStringRead >= 1))
      {
      MIL_ID DispAnnotations = M_NULL;
      MdispInquire(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, &DispAnnotations);

      for(MIL_INT StringIndex = 0; StringIndex < NumberOfStringRead; StringIndex++)
         {
         std::vector<MIL_INT> CharInvalidIndexes;
         MdmrGetResult(MilDmrResult, StringIndex, M_DEFAULT, M_STRING_CHAR_INVALID_INDICES, CharInvalidIndexes);

         if(CharInvalidIndexes.size() > 0)
            {
            MgraColor(M_DEFAULT, M_COLOR_YELLOW);
            MgraControl(M_DEFAULT, M_LINE_THICKNESS, 3);

            MosPrintf(MIL_TEXT("Char Invalid for string index %d was found at position: "), StringIndex);

            for(MIL_INT index = 0; index < (MIL_INT)CharInvalidIndexes.size(); index++)
               {
               /* Draw char invalid positions and boxes. */
               MdmrDraw(M_DEFAULT, MilDmrResult, DispAnnotations, M_DRAW_STRING_CHAR_BOX, StringIndex, M_INDEX_IN_STRING(CharInvalidIndexes[index]), M_DEFAULT);
               MdmrDraw(M_DEFAULT, MilDmrResult, DispAnnotations, M_DRAW_STRING_CHAR_POSITION, StringIndex, M_INDEX_IN_STRING(CharInvalidIndexes[index]), M_DEFAULT);

               if(index < (MIL_INT)(CharInvalidIndexes.size() - 1))
                  MosPrintf(MIL_TEXT(" %d,"), CharInvalidIndexes[index]);
               else
                  MosPrintf(MIL_TEXT(" %d."), CharInvalidIndexes[index]);
               }

            MosPrintf(MIL_TEXT("\n"), StringIndex);
            }
         }
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   }
