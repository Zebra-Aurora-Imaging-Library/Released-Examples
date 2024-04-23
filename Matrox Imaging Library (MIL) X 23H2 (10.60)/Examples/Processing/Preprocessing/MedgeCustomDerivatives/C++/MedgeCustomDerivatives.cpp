﻿/*****************************************************************************/
/* 
 * File name: MedgeCustomDerivatives.cpp
 *
 * Synopsis:  This program uses the MIL Edge Finder module to extract
 *            contours using user-defined custom partial derivatives.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/* Source MIL image file specifications. */
#define CONTOUR_IMAGE               M_IMAGE_PATH MIL_TEXT("Wafer.mim")

/* Utility function to blur an image in the horizontal direction. */
void   HorizontalBlurImage(MIL_ID MilSystem, MIL_ID MilImage, MIL_INT Size);

/* Utility function to create a custom derivative convolution kernel. */
#define DERIVATIVE_X 0
#define DERIVATIVE_Y 1
MIL_ID CreateDerivativeKernel(MIL_ID MilSystem, MIL_INT DimX, MIL_INT DimY, MIL_INT KernelType);

/* Utility function to calculate the image derivative in an ROI. */
void   CalculateChildDerivative(MIL_ID MilImage, 
                                MIL_ID Derivative, 
                                MIL_ID Kernel, 
                                MIL_INT OffsetX, 
                                MIL_INT OffsetY, 
                                MIL_INT SizeX, 
                                MIL_INT SizeY);

/* Util ROI definitions. */
#define ROI1_OFFSET_X 280
#define ROI1_OFFSET_Y  55
#define ROI1_SIZE_X   100
#define ROI1_SIZE_Y   120

#define ROI2_OFFSET_X 235
#define ROI2_OFFSET_Y 325
#define ROI2_SIZE_X    60
#define ROI2_SIZE_Y    60

#define ROI3_OFFSET_X  50
#define ROI3_OFFSET_Y  50
#define ROI3_SIZE_X   135
#define ROI3_SIZE_Y    65


/* Main function. */
int MosMain(void)
   {
   MIL_ID      MilApplication,                         /* Application identifier.       */
               MilSystem,                              /* System Identifier.            */
               MilDisplay,                             /* Display identifier.           */
               MilImage,                               /* Image buffer identifier.      */
               GraphicList,                            /* Graphic list identifier.      */
               MilEdgeContext,                         /* Edge context.                 */ 
               MilEdgeResult;                          /* Edge result identifier.       */   

   MIL_ID      DerivativeKernel,                       /* Derivative convolution kernel */
               DerivativeX,                            /* Partial derivative in X       */
               DerivativeY;                            /* Partial derivative in Y       */

   MIL_INT     SizeX,
               SizeY;

   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Restore the image and display it. */
   MbufRestore(CONTOUR_IMAGE, MilSystem, &MilImage);

   /* Blur the source image in the X direction. */
   HorizontalBlurImage(MilSystem, MilImage, 10);

   /* Display the blurred source image. */
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Set graphic context properties */
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);

   /* Pause to show the original image. */ 
   MosPrintf(MIL_TEXT("\nEDGE MODULE:\n"));  
   MosPrintf(MIL_TEXT("------------\n\n"));  
   MosPrintf(MIL_TEXT("This program extracts image contours in ROIs using\n")
             MIL_TEXT("user-defined custom partial derivatives.\n"));

   /* Retrieving the size of the source buffer */
   MbufInquire(MilImage, M_SIZE_X, &SizeX);
   MbufInquire(MilImage, M_SIZE_Y, &SizeY);

   /* Allocating and initializing the partial derivative buffers */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 16 + M_SIGNED, M_IMAGE + M_PROC, &DerivativeX);
   MbufClear(DerivativeX, 0);

   MbufAlloc2d(MilSystem, SizeX, SizeY, 16 + M_SIGNED, M_IMAGE + M_PROC, &DerivativeY);
   MbufClear(DerivativeY, 0);

   /* Calculating the Y derivative only in the region (1) containing horizontal features using a 1x5 derivative kernel */
   /*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
   MgraText(M_DEFAULT, GraphicList, ROI1_OFFSET_X + 1, ROI1_OFFSET_Y + 1, MIL_TEXT("1"));
   MgraRect(M_DEFAULT, GraphicList, ROI1_OFFSET_X, ROI1_OFFSET_Y, ROI1_OFFSET_X + ROI1_SIZE_X, ROI1_OFFSET_Y + ROI1_SIZE_Y);
   
   DerivativeKernel = CreateDerivativeKernel(MilSystem, 2, 5, DERIVATIVE_Y);
   CalculateChildDerivative(MilImage, DerivativeY, DerivativeKernel, ROI1_OFFSET_X, ROI1_OFFSET_Y, ROI1_SIZE_X, ROI1_SIZE_Y);
   MbufFree(DerivativeKernel);

   /* Calculating the X derivative only in the region (2) containing horizontal features using a 15x2 derivative kernel */
   /*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
   MgraText(M_DEFAULT, GraphicList, ROI2_OFFSET_X + 1, ROI2_OFFSET_Y + 1, MIL_TEXT("2"));
   MgraRect(M_DEFAULT, GraphicList, ROI2_OFFSET_X, ROI2_OFFSET_Y, ROI2_OFFSET_X + ROI2_SIZE_X, ROI2_OFFSET_Y + ROI2_SIZE_Y);
   
   DerivativeKernel = CreateDerivativeKernel(MilSystem, 15, 2, DERIVATIVE_X);
   CalculateChildDerivative(MilImage, DerivativeX, DerivativeKernel, ROI2_OFFSET_X, ROI2_OFFSET_Y, ROI2_SIZE_X, ROI2_SIZE_Y);
   MbufFree(DerivativeKernel);

   /* Calculating both X and Y derivative components in the region (3) containing features of various orientations */
   /*//////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
   MgraText(M_DEFAULT, GraphicList, ROI3_OFFSET_X + 1, ROI3_OFFSET_Y + 1, MIL_TEXT("3"));
   MgraRect(M_DEFAULT, GraphicList, ROI3_OFFSET_X, ROI3_OFFSET_Y, ROI3_OFFSET_X + ROI3_SIZE_X, ROI3_OFFSET_Y + ROI3_SIZE_Y);
   
   DerivativeKernel = CreateDerivativeKernel(MilSystem, 15, 5, DERIVATIVE_X);
   CalculateChildDerivative(MilImage, DerivativeX, DerivativeKernel, ROI3_OFFSET_X, ROI3_OFFSET_Y, ROI3_SIZE_X, ROI3_SIZE_Y);
   MbufFree(DerivativeKernel);

   DerivativeKernel = CreateDerivativeKernel(MilSystem, 5, 5, DERIVATIVE_Y);
   CalculateChildDerivative(MilImage, DerivativeY, DerivativeKernel, ROI3_OFFSET_X, ROI3_OFFSET_Y, ROI3_SIZE_X, ROI3_SIZE_Y);
   MbufFree(DerivativeKernel);

   /* Allocate a Edge Finder context. */
   MedgeAlloc(MilSystem, M_CONTOUR, M_DEFAULT, &MilEdgeContext); 

   /* Allocate a result buffer. */
   MedgeAllocResult(MilSystem, M_DEFAULT, &MilEdgeResult);

   /* Calculate the edges using the custom partial derivative. */
   MedgeCalculate(MilEdgeContext, M_NULL, DerivativeX, DerivativeY, M_NULL, MilEdgeResult, M_DEFAULT);

   /* Draw edges in the source image to show the result. */
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MedgeDraw(M_DEFAULT, MilEdgeResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);

   /* Wait for a key press. */ 
   MosPrintf(MIL_TEXT("\nThe extracted edges are displayed.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MbufFree(DerivativeX);
   MbufFree(DerivativeY);
   MedgeFree(MilEdgeContext);
   MedgeFree(MilEdgeResult);
   MdispFree(MilDisplay);

   /* Free defaults. */    
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }


/* Utility function to blur the source image horizontally. */
void   HorizontalBlurImage(MIL_ID MilSystem, MIL_ID MilImage, MIL_INT Size)
   {
   MIL_ID Kernel;
   
   /* Create a 1-D average filter. */
   MbufAlloc1d(MilSystem, Size, 8 + M_UNSIGNED, M_KERNEL, &Kernel);
   MbufClear(Kernel, 1);

   /* Set the normalization factor to obtain an 8-bit output result. */
   MbufControl(Kernel, M_NORMALIZATION_FACTOR, Size);
   
   /* Apply the convolution to simulate an horizontal blur. */
   MimConvolve(MilImage, MilImage, Kernel);
   
   /* Free the allocated kernel. */
   MbufFree(Kernel);
   }

/* Utility function to allocate convolution kernels to calculate partial derivatives. */
MIL_ID CreateDerivativeKernel(MIL_ID MilSystem, MIL_INT DimX, MIL_INT DimY, MIL_INT KernelType)
   {
   MIL_ID    Kernel;
   MIL_ID    KernelChild;
   MIL_INT   KernelSizeX = (DimX / 2) * 2 + 1; // upper odd size.
   MIL_INT   KernelSizeY = (DimY / 2) * 2 + 1; // upper odd size.

   MbufAlloc2d(MilSystem, KernelSizeX, KernelSizeY, 8 + M_SIGNED, M_KERNEL, &Kernel);   
   MbufClear(Kernel, 0);

   if (KernelType == DERIVATIVE_X)
      {
      /* Create a [KernelSizeX x KernelSizeY] X-axis derivative kernel.
      *  -1 -1 -1 0 +1 +1 +1 
      *  -1 -1 -1 0 +1 +1 +1
      *  -1 -1 -1 0 +1 +1 +1
      */
      MbufChild2d(Kernel, 0, 0, KernelSizeX / 2, KernelSizeY, &KernelChild);
      MbufClear(KernelChild, -1);
      MbufChildMove(KernelChild, KernelSizeX / 2 + 1, 0, KernelSizeX / 2, KernelSizeY, M_DEFAULT);
      MbufClear(KernelChild, +1);
      MbufFree(KernelChild);
      }
   else if (KernelType == DERIVATIVE_Y)
      {
      /* Create a [KernelSizeX x KernelSizeY] Y-axis derivative kernel.
      /*  -1 -1 -1 -1 -1
      /*  -1 -1 -1 -1 -1
      /*   0  0  0  0  0
      /*  +1 +1 +1 +1 +1
      /*  +1 +1 +1 +1 +1
      */
      MbufChild2d(Kernel, 0, 0, KernelSizeX, KernelSizeY / 2, &KernelChild);
      MbufClear(KernelChild, -1);
      MbufChildMove(KernelChild, 0, KernelSizeY / 2 + 1, KernelSizeX, KernelSizeY / 2, M_DEFAULT);
      MbufClear(KernelChild, +1);
      MbufFree(KernelChild);
      }
   else
      {
      MosPrintf(MIL_TEXT("\n WARNING: Invalid function 'KernelType' value.\n"));
      }
   
   /* Apply normalization factor to obtain a 10-bits 
   *  signed ouptput derivative as required by the 
   *  MedgeCalculate function.
   */
   MIL_INT NormFactor = (KernelSizeX * KernelSizeY) / 2;
   MbufControl(Kernel, M_NORMALIZATION_FACTOR, NormFactor);

   /* Set the overscan mode to transparent. */
   MbufControl(Kernel, M_OVERSCAN, M_TRANSPARENT);

   /* Return the kernel identifier. */
   return Kernel;
   }

/* Utility function to calculate the partial derivative within a specified ROI. */
void CalculateChildDerivative(MIL_ID MilImage, 
                              MIL_ID Derivative, 
                              MIL_ID Kernel, 
                              MIL_INT OffsetX, 
                              MIL_INT OffsetY, 
                              MIL_INT SizeX, 
                              MIL_INT SizeY)
   {
   MIL_ID MilImageChild, DerivativeChild;

   /* Allocate the source and destination child buffers. */
   MbufChild2d(MilImage, OffsetX, OffsetY, SizeX, SizeY, &MilImageChild);
   MbufChild2d(Derivative, OffsetX, OffsetY, SizeX, SizeY, &DerivativeChild);

   /* Calculate the derivative. */
   MimConvolve(MilImageChild, DerivativeChild, Kernel);

   /* Release the local resources. */
   MbufFree(MilImageChild);
   MbufFree(DerivativeChild);
   }