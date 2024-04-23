/************************************************************************************/
/*
* File name: ImageFlattening.cpp
*
* Synopsis:  This program demonstrates some image flattening strategies.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

/* Source images file name. */
#define MIL_IMAGE_TEXT          M_IMAGE_PATH  MIL_TEXT("VariousCodeReadings/FlippedDatamatrix2.mim")

/* Utility constant. */
#define SMOOTHNESS_VALUE        90.0

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("Image Flattening\n\n")
      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program demonstrates how to flatten a source image using\n") 
      MIL_TEXT("a combination of linear filtering and arithmetic.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer, image processing.\n\n"));
   }


/* Simple flatten method that subtracts the deviation from the estimated luminance. */
void SimpleFlatten1(MIL_ID MilSystem, MIL_ID Src, MIL_ID Dst, MIL_ID MilFilter, MIL_DOUBLE FilterSmoothness, MIL_DOUBLE Alpha)
   {
   // Allocate temporary buffer.
   MIL_INT Sx = MbufInquire(Src, M_SIZE_X, M_NULL);
   MIL_INT Sy = MbufInquire(Src, M_SIZE_Y, M_NULL);
   MIL_ID Temp = MbufAlloc2d(MilSystem, Sx, Sy, 32 + M_FLOAT, M_IMAGE + M_PROC, M_NULL);
   
   // Luminance deviation set to half the dynamic range.
   MimControl(MilFilter, M_FILTER_SMOOTHNESS, FilterSmoothness);
   MimConvolve(Src, Temp, MilFilter);
   MimArith(Temp, 127, Temp, M_SUB_CONST);
   // Subtract a fraction of the deviation.
   MimArith(Temp, -Alpha, Temp, M_MULT_CONST);
   MimArith(Src, Temp, Dst, M_ADD + M_SATURATION);
   
   // Release temporary buffer.
   MbufFree(Temp);
   }

/* Simple flatten method that divides by the estimated luminance. */
void SimpleFlatten2(MIL_ID MilSystem, MIL_ID Src, MIL_ID Dst, MIL_ID MilFilter, MIL_DOUBLE FilterSmoothness)
   {
   // Allocate temporary buffer.
   MIL_INT Sx = MbufInquire(Src, M_SIZE_X, M_NULL);
   MIL_INT Sy = MbufInquire(Src, M_SIZE_Y, M_NULL);
   MIL_ID Temp = MbufAlloc2d(MilSystem, Sx, Sy, 32 + M_FLOAT, M_IMAGE + M_PROC, M_NULL);

   // Estimate luminance.
   MimControl(MilFilter, M_FILTER_SMOOTHNESS, FilterSmoothness);
   MimConvolve(Src, Temp, MilFilter);
   MimArith(Temp, 1, Temp, M_ADD_CONST + M_SATURATION);
   // Divide by estimated luminance.
   MimArith(Src, Temp, Temp, M_DIV);
   MimArith(Temp, 128, Dst, M_MULT_CONST + M_SATURATION);
   
   // Release temporary buffers.
   MbufFree(Temp);
   }

/* Local normalization operation. */
void LocalNormalization(MIL_ID MilSystem, MIL_ID Src, MIL_ID Dst, MIL_ID MilFilter, MIL_DOUBLE FilterSmoothness)
   {
   // Allocate temporary buffers.
   MIL_INT Sx = MbufInquire(Src, M_SIZE_X, M_NULL);
   MIL_INT Sy = MbufInquire(Src, M_SIZE_Y, M_NULL);
   MIL_ID Temp1 = MbufAlloc2d(MilSystem, Sx, Sy, 32 + M_FLOAT, M_IMAGE + M_PROC, M_NULL);
   MIL_ID Temp2 = MbufAlloc2d(MilSystem, Sx, Sy, 32 + M_FLOAT, M_IMAGE + M_PROC, M_NULL);
   
   // Subtract luminance.
   MimControl(MilFilter, M_FILTER_SMOOTHNESS, FilterSmoothness);
   MimConvolve(Src, Temp1, MilFilter);
   MimArith(Src, Temp1, Temp1, M_SUB);
   // Local standard deviation.
   MimArith(Temp1, M_NULL, Temp2, M_SQUARE);
   MimConvolve(Temp2, Temp2, MilFilter);
   MimArith(Temp2, M_NULL, Temp2, M_SQUARE_ROOT);
   MimArith(Temp2, 1.0, Temp2, M_ADD_CONST);
   // Normalization.
   MimArith(Temp1, Temp2, Temp1, M_DIV);
   MimArith(Temp1, 2.0, Temp1, M_ADD_CONST);
   MimArith(Temp1, 64, Dst, M_MULT_CONST + M_SATURATION);
   
   // Release temporary buffers.
   MbufFree(Temp1);
   MbufFree(Temp2);
   }


int MosMain()
   {
   PrintHeader();

   MIL_ID MilApplication,    /* Application identifier.    */
          MilSystem,         /* System identifier.         */
          MilImage,          /* Image buffer identifier.   */
          MilIIRFilterId,    /* Filter context identifier. */
          MilSrcChild,       /* Child buffer identifier.   */
          MilFlattenChild,   /* Child buffer identifier.   */
          MilDisplay,        /* Display identifier.        */
          MilDispOvrId;      /* Display overlay identifier.*/

   MIL_INT SizeX, SizeY;
   
   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);
   MgraColor(M_DEFAULT, 255);

   /* Print a message. */
   MosPrintf(MIL_TEXT("An image is loaded and flattened using several strategies.\n"));

   /* Inquire the image size and type. */
   MbufDiskInquire(MIL_IMAGE_TEXT, M_SIZE_X, &SizeX);
   MbufDiskInquire(MIL_IMAGE_TEXT, M_SIZE_Y, &SizeY);

   /* Allocate the display and child buffers. */
   MbufAlloc2d(MilSystem, 2 * SizeX, 2 * SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilImage);
   MbufChild2d(MilImage, 0, 0, SizeX, SizeY, &MilSrcChild);
   MbufClear(MilImage, 0);

   // Allocate an IIR low-pass filter context object.
   MimAlloc(MilSystem, M_LINEAR_FILTER_IIR_CONTEXT, M_DEFAULT, &MilIIRFilterId);
   MimControl(MilIIRFilterId, M_FILTER_TYPE, M_SHEN);
   MimControl(MilIIRFilterId, M_FILTER_OPERATION, M_SMOOTH);

   /* Load an image. */
   MbufLoad(MIL_IMAGE_TEXT, MilSrcChild);

   /* Method 1*/
   MosPrintf(MIL_TEXT("\nMethod 1: A fraction of the deviation to the estimated image luminance is\n") 
               MIL_TEXT("          subtracted from the source image. Locally, the contrast information\n") 
               MIL_TEXT("          is similar to the source image's local contrast.\n"));

   MbufChild2d(MilImage, SizeX, 0, SizeX, SizeY, &MilFlattenChild);   
   SimpleFlatten1(MilSystem, MilSrcChild, MilFlattenChild, MilIIRFilterId, SMOOTHNESS_VALUE, 0.8);

   /* Method 2*/
   MosPrintf(MIL_TEXT("\nMethod 2: The source image is divided by the estimation of the image's\n")
               MIL_TEXT("          luminance. The resulting image has enhanced local contrast\n")
               MIL_TEXT("          information.\n"));

   MbufChildMove(MilFlattenChild, 0, SizeY, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   SimpleFlatten2(MilSystem, MilSrcChild, MilFlattenChild, MilIIRFilterId, SMOOTHNESS_VALUE);

   /* Method 3*/
   MosPrintf(MIL_TEXT("\nMethod 3: A local contrast image, obtained by subtracting the\n")
               MIL_TEXT("          estimated luminance, is normalized by the local intensity\n")
               MIL_TEXT("          variation. In the resulting image, the amplitude of all\n")
               MIL_TEXT("          the local contrast information is similar.\n"));

   MbufChildMove(MilFlattenChild, SizeX, SizeY, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   LocalNormalization(MilSystem, MilSrcChild, MilFlattenChild, MilIIRFilterId, SMOOTHNESS_VALUE);

   /* Display the image buffer. */
   MdispSelect(MilDisplay, MilImage);

   /* Identify the methods in the display overlay. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilDispOvrId);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraText(M_DEFAULT, MilDispOvrId, SizeX + 10,         10, MIL_TEXT("Method 1"));
   MgraText(M_DEFAULT, MilDispOvrId,         10, SizeY + 10, MIL_TEXT("Method 2"));
   MgraText(M_DEFAULT, MilDispOvrId, SizeX + 10, SizeY + 10, MIL_TEXT("Method 3"));

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free objects. */
   MimFree(MilIIRFilterId);
   MbufFree(MilSrcChild);
   MbufFree(MilFlattenChild);
   MbufFree(MilImage); 
   MdispFree(MilDisplay);

   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
   return 0;
   }
