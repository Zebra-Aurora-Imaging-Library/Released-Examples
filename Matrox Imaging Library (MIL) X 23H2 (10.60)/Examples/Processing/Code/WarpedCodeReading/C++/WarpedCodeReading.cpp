﻿/***************************************************************************************/
/*
* File name: WarpedCodeReading.cpp
*
* Synopsis:  This program corrects an image that printed on a non planar surface
*            using calibration then read it.
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
             MIL_TEXT("WarpedCodeReading\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program corrects the image of a QrCode that was printed on a non-planar\n")
             MIL_TEXT("surface using calibration, and reads it.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, code.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }


//*********************************
// CODE DEFORMATION declarations
//*********************************

const MIL_INT NUMBER_GRID_ROWS    = 19;
const MIL_INT NUMBER_GRID_COLUMNS = 19;

static MIL_CONST_TEXT_PTR CalDeformationFilename  = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/CalibrationQRCode.mim");
static MIL_CONST_TEXT_PTR CodeDeformationFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/DeformedQRCode.mim");

void CodeDeformation(MIL_CONST_TEXT_PTR SrcFilename,
                     MIL_CONST_TEXT_PTR GridFilename,
                     MIL_INT RowNumber, 
                     MIL_INT ColumNumber, 
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
   MIL_UNIQUE_APP_ID    MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID    MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_DISP_ID   MilDisplay= MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);

   // Print Header.
   PrintHeader();

   // Calibrate the code image then read.
   CodeDeformation(CodeDeformationFilename, CalDeformationFilename, 19, 19, MilSystem, MilDisplay);

   return 0;
   }

//******************
// CODE DEFORMATION
//******************
void CodeDeformation(MIL_CONST_TEXT_PTR SrcFilename,
                     MIL_CONST_TEXT_PTR GridFilename,
                     MIL_INT RowNumber,
                     MIL_INT ColumNumber,
                     MIL_ID MilSystem,
                     MIL_ID MilDisplay)
   {
   // Restore the grid image.
   MIL_ID MilSrcImage = MbufRestore(GridFilename, MilSystem, M_NULL);

   // Allocate a calibration.
   MIL_UNIQUE_CAL_ID MilCalContext = McalAlloc(MilSystem, M_LINEAR_INTERPOLATION, M_DEFAULT, M_UNIQUE_ID);

   // Calibrate from the grid image.
   McalGrid(MilCalContext, MilSrcImage, 0, 0, 0, NUMBER_GRID_ROWS, NUMBER_GRID_COLUMNS, 1, 1, M_DEFAULT, M_CHESSBOARD_GRID);

   // Allocate a display image.
   MIL_ID   MilDispProcImage,         // Display and destination buffer.
            MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   // Display the calibration result.
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   McalDraw(M_DEFAULT, MilSrcImage, MilOverlayImage, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The image of the surface is calibrated using a chessboard grid.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Free the calibration image and the display image.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);

   // Restore the image.
   MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Associate the calibration.
   McalAssociate(MilCalContext, MilSrcImage, M_DEFAULT);

   // Allocate a display image.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   // Retrieve image info.
   MIL_INT SizeX, SizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SizeY);

   // Display the calibration result.
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   McalDraw(M_DEFAULT, MilSrcImage, MilOverlayImage, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The image of the distorted code is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Clear the overlay image.
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   // Transform the image.
   McalTransformImage(MilSrcImage, MilDispProcImage, MilCalContext, M_BILINEAR, M_DEFAULT, M_WARP_IMAGE + M_CLIP);

   // Display the calibration result.
   McalDraw(M_DEFAULT, MilDispProcImage, MilOverlayImage, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The transformed image of the code is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a code context 
   MIL_UNIQUE_CODE_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate a code result
   MIL_UNIQUE_CODE_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_QRCODE, M_NULL, M_DEFAULT, M_NULL);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 0.5*SizeY, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to finish.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   }

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
