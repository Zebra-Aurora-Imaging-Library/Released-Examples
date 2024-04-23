﻿/***************************************************************************************/
/*
* File name: Various1DBarcodeReadings.cpp
*
* Synopsis:  This program contains examples of code reading operations for different
*            types of 1D barcodes under various conditions.
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
             MIL_TEXT("Various1DBarcodeReadings\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program reads different types of 1D barcodes,\n")
             MIL_TEXT("under various conditions.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, code.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//******************************
// CODE ROTATION declarations
//******************************

static MIL_CONST_TEXT_PTR CodeRotationFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/GS1Databar.mim");

void CodeRotation(MIL_CONST_TEXT_PTR SrcFilename,
                  MIL_ID MilSystem,
                  MIL_ID MilDisplay);


//******************************************
// LINEAR CODE SCANLINE SCORES declarations
//******************************************

static MIL_CONST_TEXT_PTR CodeScanLineScoresFileName = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/Code128_ScanScore.mim");

void LinearCodeScanLineScores(MIL_CONST_TEXT_PTR SrcFilename,
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

   //*****************
   // CODE ROTATION
   //*****************
   CodeRotation(CodeRotationFilename, MilSystem, MilDisplay);

   //****************************
   // LINEAR CODE SCANLINE SCORES
   //****************************
   LinearCodeScanLineScores(CodeScanLineScoresFileName, MilSystem, MilDisplay);
  
   // Free other allocations.
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//*****************
// CODE ROTATION
//*****************
void CodeRotation(MIL_CONST_TEXT_PTR SrcFilename,
                  MIL_ID MilSystem,
                  MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING A ROTATED CODE]\n\n")
      MIL_TEXT("In this example, a linear code is read at any angle.\n\n"));

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
   McodeModel(MilCodeContext, M_ADD, M_GS1_DATABAR, M_NULL, M_DEFAULT, M_NULL);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, false);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Rotate the image, then read and display the result.
   for(MIL_INT ii=5; ii<=360; ii+=5)
      {
      // Disable display update.
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      // Clear overlay.
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

      // Rotate the image.
      MimRotate(MilSrcImage, MilDispProcImage, (MIL_DOUBLE)(ii), M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_BILINEAR);

      // Read the code and display the result.
      McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
      RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, false);

      // Disable display update.
      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   };

//****************************
// LINEAR CODE SCANLINE SCORES
//****************************
void LinearCodeScanLineScores(MIL_CONST_TEXT_PTR SrcFilename,
                              MIL_ID MilSystem,
                              MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING A LINEAR CODE AND DISPLAYING SCANLINES]\n\n")

             MIL_TEXT("In this example, a linear code is read. We then\n")
             MIL_TEXT("display the ScanLines that were decoded from it,\n")
             MIL_TEXT("along with their scores, to help understand the\n")
             MIL_TEXT("quality of the code.\n\n"));
             
   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage;        // Display and destination buffer.
   MIL_ID MilOverlayImage;         // Overlay buffer.

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
   McodeModel(MilCodeContext, M_ADD, M_CODE128, M_NULL, M_DEFAULT, M_NULL);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, false);

   //Display Read Score.
   MIL_DOUBLE ReadScore = 0;
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_SCORE, &ReadScore);

   const MIL_INT TEXT_SIZE = 256;
   MIL_TEXT_CHAR OutputText[TEXT_SIZE];
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("Read Score: %.2f"), ReadScore);
   MosPrintf(MIL_TEXT("Code 128 was decoded with a read score of %.2f.\n"), ReadScore);

   // Draw read string.
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
   MgraText(M_DEFAULT, MilOverlayImage, 0.5*SizeX, 25, OutputText);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //Draw Decoded Scan Lines.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_DECODED_SCANS, 0, M_ALL, M_DEFAULT);
   //Retrieve decoded scan scores.
   std::vector<MIL_DOUBLE> DecodedScanScores;
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_SCORE, DecodedScanScores);
   //Retrieve decoded scan Start positions.
   std::vector<MIL_INT> DecodedScansStartX;
   std::vector<MIL_INT> DecodedScansStartY;
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_START_X, DecodedScansStartX);
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_START_Y, DecodedScansStartY);
   //Retrieve decoded scan End positions.
   std::vector<MIL_INT> DecodedScansEndX;
   std::vector<MIL_INT> DecodedScansEndY;
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_END_X, DecodedScansEndX);
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_END_Y, DecodedScansEndY);

   //Display ScanLine Index and scores.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MgraBackColor(M_DEFAULT, M_COLOR_WHITE);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_VERTICAL, M_CENTER);

   //Display column titles.
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("ScanLine"));
   MgraText(M_DEFAULT, MilOverlayImage, 10, 70, OutputText);
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("Index"));
   MgraText(M_DEFAULT, MilOverlayImage, 10, 85, OutputText);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_RIGHT);
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("ScanLine"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX - 10, 70, OutputText);
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("Scores"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX - 10, 85, OutputText);

   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);
   MgraFontScale(M_DEFAULT, 0.8, 0.8);

   MosPrintf(MIL_TEXT("%i ScanLines were decoded, with the following scores:\n"), DecodedScanScores.size());

   for(MIL_UINT i = 0; i < DecodedScanScores.size(); i++)
      {
      //Display ScanLine Index.
      MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("%i"), i);
      MgraText(M_DEFAULT, MilOverlayImage, DecodedScansStartX[i]-10, DecodedScansStartY[i], OutputText);

      //Display ScanLine Score.
      MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("%.2f"), DecodedScanScores[i]);
      MgraText(M_DEFAULT, MilOverlayImage, DecodedScansEndX[i] + 10, DecodedScansEndY[i], OutputText);

      MosPrintf(MIL_TEXT("ScanLine[%i] Score = %.2f\n"), i, DecodedScanScores[i]);
      }
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
