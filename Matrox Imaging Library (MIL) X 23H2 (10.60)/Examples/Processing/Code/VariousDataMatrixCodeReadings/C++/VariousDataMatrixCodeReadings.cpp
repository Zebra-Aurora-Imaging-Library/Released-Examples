﻿/*****************************************************************************/
/*
* File name: VariousDataMatrixCodeReadings.cpp
*
* Synopsis:  This program contains examples of code reading operations for
*            Data Matrix codes under various conditions.
*            See the PrintHeader() function below for detailed description.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>


///***************************************************************************
// Example description.
///***************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("VariousDataMatrixCodeReadings\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program reads Data Matrix codes,\n")
             MIL_TEXT("under various conditions.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, code.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*********************
// Black and white code
//*********************
static MIL_CONST_TEXT_PTR CodeForegroundColorFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/BlackAndWhiteDatamatrix.mim");

//*************
// Flipped code
//*************
static MIL_CONST_TEXT_PTR CodeFlippedDatamatrixFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/FlippedDatamatrix1.mim");

//*****************
// Uneven grid code
//*****************
static MIL_CONST_TEXT_PTR CodeUnevenGridFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix1.mim");

//*************
// Rotated code
//*************
static MIL_CONST_TEXT_PTR CodeRotatedFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/RotatedDatamatrix1.mim");

//*************************
// Extened rectangular code
//*************************
static MIL_CONST_TEXT_PTR CodeDMREFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/DMRE1.mim");

//*******************
// Code Read function
//*******************
void ReadDatamatixCode(MIL_CONST_TEXT_PTR SrcFilename,
                       MIL_ID MilSystem,
                       MIL_ID MilDisplay);

//************************************
// Utility sub-functions declarations
//************************************
void AllocDisplayImage(MIL_ID MilSystem,
                       MIL_ID MilSrcImage,
                       MIL_ID MilDisplay,
                       MIL_ID& MilDispProcImage,
                       MIL_ID& MilOverlayImage);

void RetrieveAndDrawCode(MIL_ID MilCodeResult,
                         MIL_ID MilDisplay,
                         MIL_ID MilOverlayImage,
                         bool   DrawBox,
                         bool   DrawCode);

//*****************************************************************************
// Main
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the MIL objects.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_ID MilSystem      = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplay     = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);

   // Print Header.
   PrintHeader();

   //************************
   // CODE FOREGROUND COLOR
   //************************
   MosPrintf(MIL_TEXT("[READING BLACK AND WHITE FOREGROUND COLOR CODES]\n\n")
      MIL_TEXT("In this example, two codes with opposite colors are read at once.\n\n"));

   ReadDatamatixCode(CodeForegroundColorFilename, MilSystem, MilDisplay);

   //*************************
   // CODE FLIPPED DATAMTRIX
   //*************************
   MosPrintf(MIL_TEXT("[READING FLIPPED DATAMATRIX]\n\n")
      MIL_TEXT("In this example, a flipped Data Matrix is read.\n\n"));

   ReadDatamatixCode(CodeFlippedDatamatrixFilename, MilSystem, MilDisplay);

   //****************************
   // CODE UNVEN GRID DISTORTION
   //****************************
   MosPrintf(MIL_TEXT("[READING UNEVEN GRID STEP DISTORTED DATAMATRIX]\n\n")
      MIL_TEXT("In this example, an uneven grid step distorted Data Matrix is read.\n\n"));

   ReadDatamatixCode(CodeUnevenGridFilename, MilSystem, MilDisplay);

   //****************************
   // CODE ROTATED
   //****************************
   MosPrintf(MIL_TEXT("[READING ROTATED DATAMATRIX]\n\n")
      MIL_TEXT("In this example, a rotated Data Matrix is read.\n\n"));

   ReadDatamatixCode(CodeRotatedFilename, MilSystem, MilDisplay);

   //**************************************
   // CODE EXTENDED RECTANGULAR DATAMTRIX
   //**************************************
   MosPrintf(MIL_TEXT("[READING EXTENDED RECTANGULAR DATAMATRIX]\n\n")
      MIL_TEXT("In this example, an extended rectangular Data Matrix is read.\n\n"));

   ReadDatamatixCode(CodeDMREFilename, MilSystem, MilDisplay);

   // Free other allocations.
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }


//************************
// CODE FOREGROUND COLOR
//************************
void ReadDatamatixCode(MIL_CONST_TEXT_PTR SrcFilename,
                       MIL_ID MilSystem,
                       MIL_ID MilDisplay)
   {
   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a Improved Recognition context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_IMPROVED_RECOGNITION, M_NULL);

   // Add a Datamatrix model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, M_NULL);

   // Setup to decode all the datamatrix
   McodeControl(MilCodeModel, M_NUMBER, M_ALL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Read the code(s).
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);

   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
}

//************************************
// Utility sub-functions definitions
//************************************
void RetrieveAndDrawCode(MIL_ID     MilCodeResult,
                         MIL_ID     MilDisplay,
                         MIL_ID     MilOverlayImage,
                         bool       DrawBox, 
                         bool       DrawCode)
   {
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   const MIL_INT DispOffsetY = 30;

   // Get decoding status.
   MIL_INT ReadStatus, ReadNumber, i;
   MIL_DOUBLE PositionX, PositionY, SizeX, SizeY;

   McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &ReadStatus);
   McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &ReadNumber);

   // Check if the decode operation was successful.
   if (ReadStatus == M_STATUS_READ_OK)
      {
      MIL_STRING PrefixString = MIL_TEXT("Read code : ");
 
      MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
      MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);

      for (i=0; i < ReadNumber; i++)
         {
         // Get decoded string.
         MIL_STRING ResultString;
         McodeGetResult(MilCodeResult, i, M_GENERAL, M_STRING, ResultString);

         MIL_INT ECIFlag;
         McodeGetResult(MilCodeResult, i, M_GENERAL, M_IS_ECI + M_TYPE_MIL_INT, &ECIFlag);

         if (ECIFlag == M_FALSE)
            {
            // Replace non printable characters with space.
            MIL_INT ii;
            for (ii = 0; ResultString[ii] != MIL_TEXT('\0'); ii++)
               {
               if ((ResultString[ii] < MIL_TEXT('0')) || (ResultString[ii] > MIL_TEXT('z')))
                  ResultString[ii] = MIL_TEXT(' ');
               }
            }

         // Draw a box around the code.
         if (DrawBox)
            {
            MgraColor(M_DEFAULT, M_COLOR_GREEN);
            McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_BOX, i, M_GENERAL, M_DEFAULT);
            }

         if (DrawCode)
            {
            MgraColor(M_DEFAULT, M_COLOR_RED);
            McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_CODE, i, M_GENERAL, M_DEFAULT);
            }

         // Retrieve basic results.
         McodeGetResult(MilCodeResult, i, M_GENERAL, M_POSITION_X, &PositionX);
         McodeGetResult(MilCodeResult, i, M_GENERAL, M_POSITION_X, &PositionY);
         McodeGetResult(MilCodeResult, i, M_GENERAL, M_SIZE_X, &SizeX);
         McodeGetResult(MilCodeResult, i, M_GENERAL, M_SIZE_Y, &SizeY);

         MIL_STRING OutputString = PrefixString + ResultString;
         MosPrintf(MIL_TEXT("Reading was successful.\n\n"));
         MosPrintf(MIL_TEXT(" - %s\n"), OutputString.c_str());
         MosPrintf(MIL_TEXT(" - Position: (%.2f, %.2f)\n"), PositionX, PositionY);
         MosPrintf(MIL_TEXT(" - Dimensions: (%.2f x %.2f)\n\n"), SizeX, SizeY);

         // Draw read string.
         MgraColor(M_DEFAULT, M_COLOR_CYAN);
         MgraText(M_DEFAULT, MilOverlayImage, MbufInquire(MilOverlayImage, M_SIZE_X, M_NULL) / 2, 15 * (i + 1), OutputString);
         }
      }
   else
      {
      MosPrintf(MIL_TEXT("Code read operation failed.\n\n"));
      }
   }

void AllocDisplayImage(MIL_ID MilSystem,
                       MIL_ID MilSrcImage,
                       MIL_ID MilDisplay,
                       MIL_ID& MilDispProcImage,
                       MIL_ID& MilOverlayImage)
   {
   // Retrieve the source image size.
   MIL_INT SrcSizeX, SrcSizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SrcSizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SrcSizeY);

   // Allocate the display image.
   MbufAlloc2d(MilSystem,
               SrcSizeX,
               SrcSizeY,
               8L+M_UNSIGNED,
               M_IMAGE+M_PROC+M_DISP,
               &MilDispProcImage);

   MbufCopy(MilSrcImage, MilDispProcImage);

   // Display the image buffer.
   MdispSelect(MilDisplay, MilDispProcImage);

   // Prepare for overlay annotations.
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   }
