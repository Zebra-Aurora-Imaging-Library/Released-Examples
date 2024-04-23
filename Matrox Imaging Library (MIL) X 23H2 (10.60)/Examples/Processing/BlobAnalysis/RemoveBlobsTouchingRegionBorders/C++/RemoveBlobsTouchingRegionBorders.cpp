﻿/////////////////////////////////////////////////////////////////////////////////////////////
//
// File name:  RemoveBlobsTouchingRegionBorders.cpp
//
// Synopsis:   This example shows how to use a mask image to
//             remove the blobs touching region borders.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//////////////////////////////////////////////////////////////////////////////////////////////

#include <mil.h>

// Source MIL image file specifications.
#define EXAMPLE_IMAGE               M_IMAGE_PATH MIL_TEXT("Seals.mim")

// Number of vertice of the polygon region
#define VERTICES_NUMBER  4

// Vertex of the polygon
static const MIL_INT VerticeXArray[VERTICES_NUMBER] = { 130, 250, 460, 150 };
static const MIL_INT VerticeYArray[VERTICES_NUMBER] = { 130, 30,  200, 440 };

//******************************************************************************************
// Example description.
//******************************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
               MIL_TEXT("RemoveBlobsTouchingRegionBorders\n\n")
               MIL_TEXT("[SYNOPSIS]\n")
               MIL_TEXT("This example shows how to use a mask image to\n")
               MIL_TEXT("remove the blobs touching region borders.\n\n")
               MIL_TEXT("[MODULES USED]\n")
               MIL_TEXT("Modules used: application, system, display, buffer,\n")
               MIL_TEXT("blob, graphics, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//********************************************************************************************
// Main.
//********************************************************************************************
int MosMain(void)
   {
   PrintHeader();

   // Allocate MIL objects.
   MIL_UNIQUE_APP_ID    MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID    MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_DISP_ID   MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);
   MIL_UNIQUE_GRA_ID MilGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Associate the graphic list to the display.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Restore and display the original image.
   MIL_UNIQUE_BUF_ID MilOrgImage = MbufRestore(EXAMPLE_IMAGE, MilSystem, M_UNIQUE_ID);
   MdispSelect(MilDisplay, MilOrgImage);

   MosPrintf(MIL_TEXT("Original image is displayed.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a blob identifier image.
   MIL_UNIQUE_BUF_ID BlobIdentImage = MbufAlloc2d(MilSystem, MbufInquire(MilOrgImage, M_SIZE_X, M_NULL), MbufInquire(MilOrgImage, M_SIZE_Y, M_NULL),
      M_UNSIGNED + 8, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);

   //Binarize the original image.
   MimBinarize(MilOrgImage, BlobIdentImage, M_FIXED + M_LESS, 45.0, M_NULL);

   MdispSelect(MilDisplay, BlobIdentImage);
   MosPrintf(MIL_TEXT("Binarize the original image to produce the blob identifier image.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a mask image and clear it to white.
   MIL_UNIQUE_BUF_ID MilMaskImage = MbufClone(MilOrgImage, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MbufClear(MilMaskImage, M_COLOR_WHITE);

   // Draw a desired polygon in black.
   MgraColor(M_DEFAULT, M_COLOR_BLACK);
   MgraLines(M_DEFAULT, MilMaskImage, VERTICES_NUMBER, VerticeXArray, VerticeYArray, M_NULL, M_NULL, M_POLYGON + M_FILLED);

   MdispSelect(MilDisplay, MilMaskImage);
   MosPrintf(MIL_TEXT("Draw a desired black polygon region in a white mask image.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Add the mask image onto the identifer image, to make the region outside the polygon white.
   MimArith(BlobIdentImage, MilMaskImage, BlobIdentImage, M_ADD +M_SATURATION);

   MosPrintf(MIL_TEXT("Add the mask image to the identifier image to make the region outside the polygon white.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MdispSelect(MilDisplay, BlobIdentImage);
   MosGetch();

   // Allocate a blob context.
   MIL_UNIQUE_BLOB_ID MilBlobContext = MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   // Enable the bounding box features to calculate.
   MblobControl(MilBlobContext, M_BOX, M_ENABLE);

   // Allocate a blob result.
   MIL_UNIQUE_BLOB_ID MilBlobResult = MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Calculate the blobs.
   MblobCalculate(MilBlobContext, BlobIdentImage, M_NULL, MilBlobResult);

   // Draw the included blobs.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MblobDraw(M_DEFAULT, MilBlobResult, MilGraphicList, M_DRAW_BLOBS, M_INCLUDED_BLOBS, M_DEFAULT);

   MosPrintf(MIL_TEXT("Calculate the blobs.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Exclude the blobs touching the image borders.
   MblobSelect(MilBlobResult, M_EXCLUDE, M_BLOB_TOUCHING_IMAGE_BORDERS, M_NULL, M_NULL, M_NULL);

   // Draw the blobs inside the polygon region.
   MgraClear(M_DEFAULT, MilGraphicList);
   MblobDraw(M_DEFAULT, MilBlobResult, MilGraphicList, M_DRAW_BLOBS, M_INCLUDED_BLOBS, M_DEFAULT);
   
   MosPrintf(MIL_TEXT("Exclude the blobs touching the borders.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   return 0;
   }

