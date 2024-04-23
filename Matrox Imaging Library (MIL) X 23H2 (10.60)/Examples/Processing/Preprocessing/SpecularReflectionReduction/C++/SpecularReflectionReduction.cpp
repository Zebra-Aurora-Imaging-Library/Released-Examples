﻿//////////////////////////////////////////////////////////////////////////////////////////
// 
// File name: SpecularReflectionReduction.cpp
// 
// Synopsis:  This example shows how to combine images taken under multiple 
//            directional illuminations to reduce specular reflections.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <mil.h>
#include <vector>

// Util definitions.
#define IMAGE_DIR M_IMAGE_PATH MIL_TEXT("SpecularReflectionReduction/")
static MIL_CONST_TEXT_PTR ILLUSTRATION_PATH = IMAGE_DIR MIL_TEXT("LightOrientations.png");
static const MIL_INT      ILLUSTRATION_DISPLAY_OFFSET_X = 660;
const MIL_INT NB_IMG = 4;

// Example description.                                                     
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("SpecularReflectionReduction\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows how to combine images taken under multiple directional\n")
             MIL_TEXT("illuminations to reduce the presence of specular reflections.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, buffer, display,       \n")
             MIL_TEXT("image processing, code reader, system.            \n\n"));

   }

//////////////////////////////////////////////////////////////////////////////////////////
int MosMain()
   {
   // Util.
   MIL_TEXT_CHAR ImgName[256];

   // Allocate MIL objects.
   MIL_ID MilApplicationId = MappAlloc (M_DEFAULT, M_NULL);
   MIL_ID MilSystemId      = MsysAlloc (M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplayId     = MdispAlloc(MilSystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MdispControl(MilDisplayId, M_VIEW_MODE, M_AUTO_SCALE);

   // Show illustration of light orientations.
   MIL_ID IllustrationDispId  = MdispAlloc(MilSystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID IllustrationImageId = MbufRestore(ILLUSTRATION_PATH, MilSystemId, M_NULL);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Light orientations"));
   MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_X, ILLUSTRATION_DISPLAY_OFFSET_X);
   MdispSelect(IllustrationDispId, IllustrationImageId);
   
   PrintHeader();

   // Restoring the images.
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////
   std::vector<MIL_ID> SourceImageVect(NB_IMG, M_NULL);
   for (MIL_INT i = 0; i < NB_IMG; ++i)
      {
      MosSprintf(ImgName, 256, MIL_TEXT("%sFrame%d.mim"), IMAGE_DIR, i);
      MbufRestore(ImgName, MilSystemId, &SourceImageVect[i]);
      MdispSelect(MilDisplayId, SourceImageVect[i]);
      MosPrintf(MIL_TEXT("The next image, taken with a directional illumination, is displayed.\n")
                MIL_TEXT("Press any key to continue...\n\n")); 
      MosGetch();
      }

   // Reducing the specular reflections using cumulative statistics.
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////

   /* Allocate a destination image. */
   MIL_ID DestId;
   MbufClone(SourceImageVect[0], M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, &DestId);

   /* Allocate cumulative statistic buffers. */
   MIL_ID MilStatCumulativeContext = MimAlloc(MilSystemId, M_STATISTICS_CUMULATIVE_CONTEXT, M_DEFAULT, M_NULL);
   MIL_ID MilStatCumulativeResult = MimAllocResult(MilSystemId, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);
   MimControl(MilStatCumulativeContext, M_STAT_MIN, M_ENABLE);

   /* Accumulate statistics. */
   for (MIL_INT i = 0; i<NB_IMG; i++)
      MimStatCalculate(MilStatCumulativeContext, SourceImageVect[i], MilStatCumulativeResult, M_DEFAULT);

   /* Retrieve the min image statistic.*/
   MimDraw(M_DEFAULT, MilStatCumulativeResult, M_NULL, DestId, M_DRAW_STAT_RESULT, M_STAT_MIN, M_NULL, M_DEFAULT);

   MdispSelect(MilDisplayId, DestId);
   MosPrintf(MIL_TEXT("The combined image with reduced specular reflections is displayed.\n")
             MIL_TEXT("Press any key to continue...\n\n"));
   MosGetch();

   // Reading the linear code.
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////
   
   // Allocate a code context and result.
   MIL_ID MilCodeContext = McodeAlloc(MilSystemId, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilCodeResult  = McodeAllocResult(MilSystemId, M_DEFAULT, M_NULL);
   MIL_ID MilCodeModel   = McodeModel(MilCodeContext, M_ADD, M_UPC_A, M_NULL, M_DEFAULT, M_NULL);

   // Read the code and display the result.
   McodeRead(MilCodeContext, DestId, MilCodeResult);

   // Get decoding status.
   MIL_INT ReadStatus;
   McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &ReadStatus);

   if (ReadStatus == M_STATUS_READ_OK)
      {
      // Get decoded string.
      MIL_INT ResultStringSize;
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_STRING + M_STRING_SIZE + M_TYPE_MIL_INT, &ResultStringSize);
      MIL_TEXT_CHAR* ResultString = new MIL_TEXT_CHAR[ResultStringSize];
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_STRING, ResultString);

      // Add prefix to the string.
      MIL_CONST_TEXT_PTR PrefixString = MIL_TEXT("Read code: ");
      MIL_INT OutputStringSize = ResultStringSize + MosStrlen(PrefixString);
      MIL_TEXT_CHAR* OutputString = new MIL_TEXT_CHAR[OutputStringSize];
      MosSprintf(OutputString, OutputStringSize, MIL_TEXT("%s%s"), PrefixString, ResultString);

      MIL_ID MilOverlayImage;
      MdispControl(MilDisplayId, M_OVERLAY, M_ENABLE);
      MdispInquire(MilDisplayId, M_OVERLAY_ID, &MilOverlayImage);

      // Draw read string.
      MgraColor(M_DEFAULT, M_COLOR_CYAN);
      MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
      MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
      MgraText(M_DEFAULT, MilOverlayImage, 100, 100, OutputString);

      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_BOX, 0, M_GENERAL, M_DEFAULT);

      MgraColor(M_DEFAULT, M_COLOR_RED);
      McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_POSITION, 0, M_GENERAL, M_DEFAULT);
      
      MosPrintf(MIL_TEXT("The linear code is read in the combined image.\n"));
      }

   MosPrintf(MIL_TEXT("Press any key to end.\n\n"));
   MosGetch();

   // Free allocated objects.
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   
   MimFree(MilStatCumulativeContext);
   MimFree(MilStatCumulativeResult);

   for (MIL_INT i = 0; i < NB_IMG; ++i)
      MbufFree(SourceImageVect[i]);

   MbufFree(DestId);
   MdispFree(IllustrationDispId);
   MbufFree(IllustrationImageId);

   MdispFree(MilDisplayId);
   MsysFree (MilSystemId);
   MappFree (MilApplicationId);
   }

