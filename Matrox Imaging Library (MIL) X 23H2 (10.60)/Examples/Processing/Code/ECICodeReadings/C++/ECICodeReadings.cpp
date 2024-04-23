﻿/***************************************************************************************/
/* 
* File name: ECICodeReadings.cpp
*
* Synopsis:  This program contains examples of code reading operations for 2D code types
*            with an Extended Channel Interpretation (ECI) encoding.
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
             MIL_TEXT("ECICodeReadings\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program corrects examples of code reading operations for 2D\n")
             MIL_TEXT("code types with an Extended Channel Interpretation (ECI) encoding.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, code.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }



//****************************************
//* CODE Character Set ECIs declarations
//****************************************

const MIL_INT CodeECINumber = 2;

static MIL_CONST_TEXT_PTR CodeECIFilename[CodeECINumber] =
   {
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/ECIQRCode.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/ECIAztecCode.mim")
   };

void CodeECI(MIL_CONST_TEXT_PTR SrcFilename,
             MIL_INT CodeType,
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

void RetrieveAndDrawCode(MIL_ID     MilCodeResult,
                         MIL_ID     MilDisplay,
                         MIL_ID     MilOverlayImage,
                         MIL_DOUBLE DrawPosX,
                         MIL_DOUBLE DrawPosY,
                         bool       DrawBox,
                         bool       DrawCode);

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

   //**************************
   // CODE Character Set ECIs
   //**************************
   MgraFont(M_DEFAULT, MIL_FONT_NAME(M_FONT_DEFAULT_TTF));
   MgraControl(M_DEFAULT, M_FONT_SIZE, 10);

   for (MIL_INT ii = 0; ii < CodeECINumber; ii++)
      {
      CodeECI(CodeECIFilename[ii], (ii == 0) ? M_QRCODE : M_AZTEC, MilSystem, MilDisplay);
      }

   // Free other allocations.
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }


void CodeECI(MIL_CONST_TEXT_PTR SrcFilename,
             MIL_INT CodeType,
             MIL_ID MilSystem,
             MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("In this example, a bar code with an Extended Channel Interpretation (ECI) is read.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_IMPROVED_RECOGNITION, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, CodeType, M_NULL, M_DEFAULT, M_NULL);

   MosPrintf(MIL_TEXT("The string result is displayed in ECI format.\n\n"));

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Enable the raw data string format.
   McodeControl(MilCodeResult, M_STRING_FORMAT, M_RAW_DATA);

   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   MosPrintf(MIL_TEXT("This is the same string result, but displayed in raw data format.\n\n"));

   // Display the result.
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

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

void RetrieveAndDrawCode(  MIL_ID     MilCodeResult,
                           MIL_ID     MilDisplay,
                           MIL_ID     MilOverlayImage,
                           MIL_DOUBLE DrawPosX,
                           MIL_DOUBLE DrawPosY,
                           bool       DrawBox,
                           bool       DrawCode)
   {
   const MIL_INT DispOffsetY = 30;

   // Get decoding status.
   MIL_INT ReadStatus;
   McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &ReadStatus);

   // Check if the decode operation was successful.
   if (ReadStatus == M_STATUS_READ_OK)
      {
      // Get decoded string.
      MIL_STRING ResultString;
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_STRING, ResultString);

      MIL_INT ECIFlag;
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_IS_ECI + M_TYPE_MIL_INT, &ECIFlag);

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

      // Add prefix to the string.
      MIL_STRING PrefixString = MIL_TEXT("Read code: ");
      MIL_STRING OutputString = PrefixString + ResultString;

      // Draw read string.
      MgraColor(M_DEFAULT, M_COLOR_CYAN);
      MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
      MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
      MgraText(M_DEFAULT, MilOverlayImage, DrawPosX, DrawPosY, OutputString);

      if (McodeInquire(MilCodeResult, M_STRING_FORMAT, M_NULL) == M_RAW_DATA)
         OutputString = MIL_TEXT("(Format: RAW)");
      else
         OutputString = MIL_TEXT("(Format: ECI)");

      MgraText(M_DEFAULT, MilOverlayImage, DrawPosX, DrawPosY + 20, OutputString);

      // Draw a box around the code.
      if (DrawBox)
         {
         MgraColor(M_DEFAULT, M_COLOR_GREEN);
         McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_BOX, 0, M_GENERAL, M_DEFAULT);
         }

      if (DrawCode)
         {
         MgraColor(M_DEFAULT, M_COLOR_RED);
         McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_CODE, 0, M_GENERAL, M_DEFAULT);
         }

      // Retrieve basic results.
      MIL_DOUBLE PositionX, PositionY, SizeX, SizeY;
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_POSITION_X, &PositionX);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_POSITION_X, &PositionY);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_SIZE_X, &SizeX);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_SIZE_Y, &SizeY);

      MosPrintf(MIL_TEXT("Reading was successful.\n\n"));
      MosPrintf(MIL_TEXT(" - Position: (%.2f, %.2f)\n"), PositionX, PositionY);
      MosPrintf(MIL_TEXT(" - Dimensions: (%.2f x %.2f)\n\n"), SizeX, SizeY);
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
