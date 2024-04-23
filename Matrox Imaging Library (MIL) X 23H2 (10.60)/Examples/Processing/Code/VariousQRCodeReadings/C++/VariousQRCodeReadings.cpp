﻿/***************************************************************************************/
/* 
* File name: VariousQRCodeReadings.cpp
*
* Synopsis:  This program contains examples of code reading operations for
*            QR codes under various conditions.
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
             MIL_TEXT("VariousQRCodeReadings\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program reads QR codes under various conditions.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, code.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//**************
// QR code image
//**************
static MIL_CONST_TEXT_PTR CodeAspectRatioAndShearingFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/SampleQRCode.mim");

//*******************
// Code Read function
//*******************
void CodeAspectRatioAndShearing(MIL_CONST_TEXT_PTR SrcFilename,
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

   // Read QR codes.
   CodeAspectRatioAndShearing(CodeAspectRatioAndShearingFilename, MilSystem, MilDisplay);

   // Free other allocations.
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

// Read QR codes
void CodeAspectRatioAndShearing(MIL_CONST_TEXT_PTR SrcFilename,
                                MIL_ID MilSystem,
                                MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("In this example a QrCode is read, even if it has an aspect ratio\n")
            MIL_TEXT("other than 1 or if it has shearing.\n\n"));

   MIL_INT ii;

   const MIL_DOUBLE StepValue = 0.01;
   const MIL_INT    Iterations = 10;

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID   MilDispProcImage,         // Display and destination buffer.
            MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_QRCODE, M_NULL, M_DEFAULT, M_NULL);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a warp matrix and initialize it to identity.
   MIL_ID MilWarpMatrix = MbufAlloc2d(MilSystem, 3, 3, 32 + M_FLOAT, M_ARRAY, M_NULL);
   MgenWarpParameter(M_NULL, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_TRANSLATE, 0, 0);

   for (ii = 0; ii < Iterations; ii++)
      {
      // Increase aspect ratio
      MgenWarpParameter(MilWarpMatrix, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_SCALE, 1.0, 1.0 + StepValue);

      // Disable display update.
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      // Clear the overlay image.
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

      // Apply the transformation.
      MimWarp(MilSrcImage, MilDispProcImage, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_BILINEAR + M_OVERSCAN_CLEAR);

      // Read the code and display the result.
      McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
      RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

      // Enable display update.
      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
      }

   for (ii = 0; ii < Iterations; ii++)
      {
      // Increase the shearing in X.
      MgenWarpParameter(MilWarpMatrix, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_SHEAR_X, StepValue, M_DEFAULT);

      // Disable display update.
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      // Clear the overlay image.
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

      // Apply the transformation.
      MimWarp(MilSrcImage, MilDispProcImage, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_BILINEAR + M_OVERSCAN_CLEAR);

      // Read the code and display the result.
      McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
      RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

      // Enable display update.
      MosSleep(100);
      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects
   MbufFree(MilWarpMatrix);
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   };

//************************************
// Utility sub-functions definitions
//************************************

void RetrieveAndDrawCode(MIL_ID     MilCodeResult,
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
   McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &ReadStatus);

   // Check if the decode operation was successful.
   if (ReadStatus == M_STATUS_READ_OK)
      {
      // Get decoded string.
       MIL_STRING ResultString;
       McodeGetResult(MilCodeResult, 0, M_GENERAL, M_STRING, ResultString);
       
      MIL_INT ECIFlag;
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_IS_ECI + M_TYPE_MIL_INT, &ECIFlag);

      if(ECIFlag == M_FALSE)
         {
         // Replace non printable characters with space.
         MIL_INT ii;
         for(ii = 0; ResultString[ii] != MIL_TEXT('\0'); ii++)
            {
            if((ResultString[ii] < MIL_TEXT('0')) || (ResultString[ii] > MIL_TEXT('z')))
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

      // Draw a box around the code.
      if(DrawBox)
         {
         MgraColor(M_DEFAULT, M_COLOR_GREEN);
         McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_BOX, 0, M_GENERAL, M_DEFAULT);
         }

      if(DrawCode)
         {
         MgraColor(M_DEFAULT, M_COLOR_RED); 
         McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_CODE, 0, M_GENERAL, M_DEFAULT);
         }

      // Retrieve basic results.
      MIL_DOUBLE PositionX, PositionY, SizeX, SizeY;
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_POSITION_X, &PositionX);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_POSITION_X, &PositionY);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_SIZE_X,     &SizeX);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_SIZE_Y,     &SizeY);

      MosPrintf(MIL_TEXT("Reading was successful.\n\n"));
      MosPrintf(MIL_TEXT(" - %s\n"), OutputString.c_str());
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