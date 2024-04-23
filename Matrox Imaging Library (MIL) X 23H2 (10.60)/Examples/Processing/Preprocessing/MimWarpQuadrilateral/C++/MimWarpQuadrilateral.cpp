﻿//***************************************************************************************/
// 
// File name: MimWarpQuadrilateral.cpp  
//
// Synopsis:  This example shows how to warp an arbitrary quadrilateral region 
//             to another arbitrary quadrilateral region.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <iostream>

#define IMAGE_FILE_PATH M_IMAGE_PATH MIL_TEXT("BaboonMono.mim")

// Coordinates of the quadrilateral in the source image.
static MIL_DOUBLE SrcCornersX[4] = { 120 , 400 , 450 , 70  };
static MIL_DOUBLE SrcCornersY[4] = { 30  , 30  , 480 , 480 };

// Coordinates of the quadrilateral in the destination image.
static MIL_DOUBLE DstCornersX[4] = { 40  , 480 , 350 , 120 };
static MIL_DOUBLE DstCornersY[4] = { 120 , 80  , 480 , 450 };

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("MimWarpQuadrilateral\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example shows how to warp an arbitrary quadrilateral region\n"));
   MosPrintf(MIL_TEXT("to another arbitrary quadrilateral region.\n"));
   MosPrintf(MIL_TEXT("\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: 3dMap, Buffer, Calibration, Display,\n")
             MIL_TEXT("Graphics, Image Processing.\n\n"));

   return;
   }

//****************************************************************************
// Multiply 3*3 array buffers to compose transformations.
//****************************************************************************
void MultiplyArrays(MIL_ID MilFirstBuffer, MIL_ID MilSecBuffer, MIL_ID &MilResultBuffer)
   {
   const MIL_UINT size = 3;

   float MilFirstArray[size][size],
         MilSecondArray[size][size],
         MilResultArray[size][size];

   // Get data form buffers.
   MbufGet2d(MilFirstBuffer, 0, 0, size, size, &MilFirstArray);
   MbufGet2d(MilSecBuffer, 0, 0, size, size, &MilSecondArray);

   for(MIL_UINT i = 0; i < size; i++)
      {
      for(size_t j = 0; j < size; j++)
         {
         MilResultArray[i][j] = 0;
         for(size_t k = 0; k < size; k++)
            {
            MilResultArray[i][j] += MilFirstArray[i][k] * MilSecondArray[k][j];
            }
         }
      }

   // Copy the results into a buffer.
   MbufPut(MilResultBuffer, &MilResultArray);
   return;
   }


//****************************************************************************
// Display the result.
//****************************************************************************
void Display(MIL_ID   MilSystem,
             MIL_ID   MilDisplay,
             MIL_ID   MilSrcImage,
             MIL_ID   MilDstImage,
             MIL_UINT SizeX,
             MIL_UINT SizeY)
   {

   MIL_ID MilImage,
          MilImageChildLeft,
          MilImageChildRight,
          MilOverlayImage;

   // Allocate the display image.
   MbufAlloc2d(MilSystem, 2 * SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilImage);

   // Allocate two children for source and destination images. 
   MbufChild2d(MilImage, 0, 0, SizeX, SizeY, &MilImageChildLeft);
   MbufChild2d(MilImage, SizeX, 0, SizeX, SizeY, &MilImageChildRight);

   // Copy the images into the display image. 
   MbufCopy(MilSrcImage, MilImageChildLeft);
   MbufCopy(MilDstImage, MilImageChildRight);

   // Draw the border of the polygons.
   MgraLines(M_DEFAULT, MilImageChildLeft , 4, SrcCornersX, SrcCornersY, M_NULL, M_NULL, M_POLYGON);
   MgraLines(M_DEFAULT, MilImageChildRight, 4, DstCornersX, DstCornersY, M_NULL, M_NULL, M_POLYGON);

   // Calculate the displacement of the points.
   const MIL_DOUBLE DisplacementX[4] = {
      DstCornersX[0] - SrcCornersX[0] + SizeX,
      DstCornersX[1] - SrcCornersX[1] + SizeX,
      DstCornersX[2] - SrcCornersX[2] + SizeX,
      DstCornersX[3] - SrcCornersX[3] + SizeX};

   const MIL_DOUBLE DisplacementY[4] = {
      DstCornersY[0] - SrcCornersY[0],
      DstCornersY[1] - SrcCornersY[1],
      DstCornersY[2] - SrcCornersY[2],
      DstCornersY[3] - SrcCornersY[3]};

   MdispSelect(MilDisplay, MilImage);
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);

   // Draw the displacement vectors.
   MgraVectors(M_DEFAULT, 
               MilOverlayImage,
               4,
               SrcCornersX, 
               SrcCornersY, 
               DisplacementX, 
               DisplacementY, 
               M_ABSOLUTE, 
               M_DEFAULT, 
               M_DEFAULT);


   MosPrintf(MIL_TEXT("Press <enter> to end...\n"));
   MosGetchar();

   MbufFree(MilImageChildLeft);
   MbufFree(MilImageChildRight);
   MbufFree(MilImage);
   
   return;
   }



//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Print example information.
   PrintHeader();

   // Allocate MIL objects. 
   MIL_ID MilApplication, 
          MilSystem,          
          MilSrcImage,
          MilDstImage,
          MilDisplay,
          MilWarpArrayBuffer,
          MilReversWarpArrayBuffer,
          MilDirTrans,
          MilRvsTrans,
          MilTransformation;

   MIL_UINT SizeX,
            SizeY;

   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   // Load the image and allocate objects.
   MbufRestore(IMAGE_FILE_PATH, MilSystem, &MilSrcImage);

   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SizeY);

   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDstImage);
   MbufClear(MilDstImage, M_COLOR_BLACK);

   MbufAlloc1d(MilSystem, 12, 32 + M_FLOAT, M_ARRAY, &MilWarpArrayBuffer);
   MbufAlloc1d(MilSystem, 12, 32 + M_FLOAT, M_ARRAY, &MilReversWarpArrayBuffer);
   
   MbufAlloc2d(MilSystem, 3, 3, 32 + M_FLOAT, M_ARRAY, &MilDirTrans);
   MbufAlloc2d(MilSystem, 3, 3, 32 + M_FLOAT, M_ARRAY, &MilRvsTrans);
   MbufAlloc2d(MilSystem, 3, 3, 32 + M_FLOAT, M_ARRAY, &MilTransformation);
   
   float SrcArrayWarp[12] = {
      (float) SrcCornersX[0], (float) SrcCornersY[0],
      (float) SrcCornersX[1], (float) SrcCornersY[1],
      (float) SrcCornersX[2], (float) SrcCornersY[2],
      (float) SrcCornersX[3], (float) SrcCornersY[3],
      (float) 0.0          , (float) 0.0,
      (float) SizeX        , (float) SizeY};

   float SrcArrayWarpReverse[12] = {
      (float) DstCornersX[0], (float) DstCornersY[0],
      (float) DstCornersX[1], (float) DstCornersY[1],
      (float) DstCornersX[2], (float) DstCornersY[2],
      (float) DstCornersX[3], (float) DstCornersY[3],
      (float) 0.0          , (float) 0.0,
      (float)SizeX         , (float)SizeY};

   // Put data into array buffers.
   MbufPut1d(MilWarpArrayBuffer, 0, 12, SrcArrayWarp);
   MbufPut1d(MilReversWarpArrayBuffer, 0, 12, SrcArrayWarpReverse);

   // Generate the transformation matrices.
   MgenWarpParameter(MilWarpArrayBuffer, MilDirTrans, M_NULL, M_WARP_4_CORNER, M_DEFAULT, M_NULL, M_NULL);
   MgenWarpParameter(MilReversWarpArrayBuffer, MilRvsTrans, M_NULL, M_WARP_4_CORNER_REVERSE, M_DEFAULT, M_NULL, M_NULL);

   // Combine the transformations.
   MultiplyArrays(MilDirTrans, MilRvsTrans, MilTransformation);
   
   MimWarp(MilSrcImage, MilDstImage, MilTransformation, M_NULL, M_WARP_POLYNOMIAL, M_BICUBIC);
   
   // Display the result.
   Display(MilSystem,
           MilDisplay, 
           MilSrcImage, 
           MilDstImage, 
           SizeX, 
           SizeY);

   // Free the buffers.
   MbufFree(MilTransformation);
   MbufFree(MilRvsTrans);
   MbufFree(MilDirTrans);
   MbufFree(MilReversWarpArrayBuffer);
   MbufFree(MilWarpArrayBuffer);
   MbufFree(MilDstImage);
   MbufFree(MilSrcImage);

   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
    
   return 0;
   }
