//******************************************************************************* /
/*
* File name: MBlobTransform.cpp
*
* Synopsis:  This program loads an image and illustrates 
*            blob tranformation operations.
*
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include "mil.h"
#include <vector>

// Util constants
static const MIL_INT MAX_STRING_LEN = 512;
#define MIN_BLOB_AREA         50L 
#define  EXAMPLE_IMAGE_PATH   M_IMAGE_PATH MIL_TEXT("BlobAnalysis/ManyBlobs.mim")

int MosMain(void)
   {
   MIL_ID MilApplication = M_NULL,
          MilSystem      = M_NULL,
          MilImage       = M_NULL,
          MilOverlay     = M_NULL,
          MilLut         = M_NULL,
          MilBlobCtx     = M_NULL,
          MilBlobRes     = M_NULL,
          MilDisplay     = M_NULL;

   MIL_INT    BlobCount, MaxLabel;
   MIL_INT    SizeX, SizeY;
   MIL_TEXT_CHAR LabelString[MAX_STRING_LEN];

   // Allocating MIL application, system, and display.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, MIL_TEXT("M_SYSTEM_HOST"), M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);
   MdispControl(MilDisplay, M_TITLE, MIL_TEXT("Display"));
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);

   /* Print header. */
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("MblobTransform\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program loads an image and illustrates\n")
             MIL_TEXT("blob tranformation operations.\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Buffer, Display, Graphics, Blob\n\n")
             );

   // Allocate a color LUT.
   MbufAllocColor(MilSystem, 3, 256, 1, 8 + M_UNSIGNED, M_LUT, &MilLut);
   MgenLutFunction(MilLut, M_COLORMAP_DISTINCT_256, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
  
   // Restore the source buffer.
   MbufRestore(EXAMPLE_IMAGE_PATH, MilSystem, &MilImage);

   // Retrieving source sizes.
   MbufInquire(MilImage, M_SIZE_X, &SizeX);
   MbufInquire(MilImage, M_SIZE_Y, &SizeY);

   // Allocate blob objects.
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobCtx);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobRes);

   // Enable blob context features.
   MblobControl(MilBlobCtx, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);
  
   // Display the source image.
   MdispSelect(MilDisplay, MilImage);
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlay);

   MosPrintf(MIL_TEXT("The image of blobs is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <ENTER> to continue\n"));
   MosGetch();

   // Calculate and delete small blobs.
   MblobCalculate(MilBlobCtx, MilImage, M_NULL, MilBlobRes);
   MblobSelect(MilBlobRes, M_DELETE, M_AREA, M_LESS_OR_EQUAL, MIN_BLOB_AREA, M_NULL);

   // Retrieve the number of blobs.
   MblobGetResult(MilBlobRes, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &BlobCount);

   // Retrieve blobs' positions and labels
   std::vector<MIL_DOUBLE> CgX(BlobCount);
   std::vector<MIL_DOUBLE> CgY(BlobCount);
   std::vector<MIL_INT>    Label(BlobCount);
   MblobGetResult(MilBlobRes, M_DEFAULT, M_CENTER_OF_GRAVITY_X + M_BINARY, CgX.data());
   MblobGetResult(MilBlobRes, M_DEFAULT, M_CENTER_OF_GRAVITY_Y + M_BINARY, CgY.data());
   MblobGetResult(MilBlobRes, M_DEFAULT, M_LABEL_VALUE + M_TYPE_MIL_INT, Label.data());

   // Display the labeled blobs.
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   MblobLabel(MilBlobRes, MilImage, M_CLEAR);
   MdispLut(MilDisplay, MilLut);

   for(MIL_INT i = 0; i < BlobCount; ++i)
      {
      MosSprintf(LabelString, MAX_STRING_LEN, MIL_TEXT("%d"), Label[i]);
      MgraText(M_DEFAULT, MilOverlay, CgX[i], CgY[i], LabelString);
      }

   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);

   MblobGetResult(MilBlobRes, M_GENERAL, M_MAX_LABEL_VALUE + M_TYPE_MIL_INT, &MaxLabel);
   MosPrintf(MIL_TEXT("The number of remaining blobs is: %d.\n"), BlobCount);
   MosPrintf(MIL_TEXT("The largest blob label value is %d.\n"), MaxLabel);
   MosPrintf(MIL_TEXT("After blob deletion, the blobs' labels are not continuous.\n\n"));
   MosPrintf(MIL_TEXT("Press <ENTER> to continue\n"));
   MosGetch();

   //////////////////////////////
   // Relabel the blob labels. //
   //////////////////////////////
   MblobTransform(MilBlobRes, MilBlobRes, M_RELABEL_CONSECUTIVE, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   
   // Display the newly labeled blobs.
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   MblobLabel(MilBlobRes, MilImage, M_CLEAR);
   MdispLut(MilDisplay, MilLut);

   MblobGetResult(MilBlobRes, M_DEFAULT, M_LABEL_VALUE + M_TYPE_MIL_INT, Label.data());

   for (MIL_INT i = 0; i < BlobCount; ++i)
      {
      MosSprintf(LabelString, MAX_STRING_LEN, MIL_TEXT("%d"), Label[i]);
      MgraText(M_DEFAULT, MilOverlay, CgX[i], CgY[i], LabelString);
      }

   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);

   MblobGetResult(MilBlobRes, M_GENERAL, M_MAX_LABEL_VALUE + M_TYPE_MIL_INT, &MaxLabel);
   MosPrintf(MIL_TEXT("The number of remaining blobs is: %d.\n"), BlobCount);
   MosPrintf(MIL_TEXT("The largest blob label value is %d.\n"), MaxLabel);
   MosPrintf(MIL_TEXT("The remaining blobs have been relabeled.\n\n"));
   MosPrintf(MIL_TEXT("Press <ENTER> to end the program\n"));
   MosGetch();

   // Release allocated objects.
   MblobFree(MilBlobCtx);
   MblobFree(MilBlobRes);
   MbufFree(MilLut);
   MbufFree(MilImage);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }
