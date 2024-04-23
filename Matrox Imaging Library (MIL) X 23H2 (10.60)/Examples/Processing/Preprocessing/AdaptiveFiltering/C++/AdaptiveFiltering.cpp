﻿/************************************************************************************/
/*
 * File name: AdaptiveFiltering.cpp 
 *
 * Synopsis:  This program demonstrates various methods to filter an image.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/* Example function prototype. */

void Filter (MIL_ID MilSystem,
             MIL_ID MilDisplay,
             MIL_CONST_TEXT_PTR SourceFile);

void SaltAndPepperFilter (MIL_ID MilSystem,
                          MIL_ID MilDisplay);

/* Source images filename. */
MIL_CONST_TEXT_PTR IMAGE_FOR_FILTER = M_IMAGE_PATH MIL_TEXT("MultipleTarget.mim");

/* Timing loop iterations. */
#define NB_LOOP 10


/*****************************************************************************/
/* Example description.                                                      */
/*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("AdaptiveFiltering\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program demonstrates how to filter an image\n")
             MIL_TEXT("using linear filtering and edge preserving methods.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("image processing.\n\n"));

   }

/****************************************************************************/
int MosMain(void)
   {
   PrintHeader();

   MIL_ID MilApplication,    /* Application identifier. */
          MilSystem,         /* System identifier.      */
          MilDisplay;        /* Display identifier.     */

   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Set display properties. */
   MdispControl(MilDisplay, M_OVERLAY,       M_ENABLE);
   MdispControl(MilDisplay, M_SCALE_DISPLAY, M_ENABLE);

   /* Run the filtering example. */
   Filter(MilSystem, MilDisplay, IMAGE_FOR_FILTER);

   /* Run the salt and pepper filtering example. */
   SaltAndPepperFilter(MilSystem, MilDisplay);

   /* Free defaults. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }


/*****************************************************************************/
/*    Perform filtering.                                                     */
/*****************************************************************************/
void Filter (MIL_ID MilSystem,
             MIL_ID MilDisplay,
             MIL_CONST_TEXT_PTR SourceFile)
   {
   MIL_ID MilImageId,                     /* Image buffer identifier.        */
          MilSubImage00,                  /* Child buffer identifier.        */
          MilSubImage01,                  /* Child buffer identifier.        */ 
          MilSubImage10,                  /* Child buffer identifier.        */
          MilSubImage11,                  /* Child buffer identifier.        */
          MilDstImage,                    /* Image buffer identifier.        */
          MilOverlayId;                   /* Overlay image buffer identifier.*/
          
   MIL_INT SizeX, SizeY, SizeBand, Type;
   MIL_DOUBLE Time;

   /* Inquire the images' size and type. */
   MbufDiskInquire(SourceFile, M_SIZE_X   , &SizeX   );
   MbufDiskInquire(SourceFile, M_SIZE_Y   , &SizeY   );
   MbufDiskInquire(SourceFile, M_SIZE_BAND, &SizeBand);
   MbufDiskInquire(SourceFile, M_TYPE     , &Type    );

   /* Allocate image buffers. */
   MbufAlloc2d(MilSystem, 2 * SizeX, 2 * SizeY, Type, M_IMAGE + M_PROC + M_DISP, &MilImageId);
   MbufChild2d(MilImageId,     0,     0, SizeX, SizeY, &MilSubImage00);
   MbufChild2d(MilImageId, SizeX,     0, SizeX, SizeY, &MilSubImage01);
   MbufChild2d(MilImageId,     0, SizeY, SizeX, SizeY, &MilSubImage10);
   MbufChild2d(MilImageId, SizeX, SizeY, SizeX, SizeY, &MilSubImage11);
   MbufAlloc2d(MilSystem, SizeX, SizeY, Type, M_IMAGE + M_PROC + M_DISP, &MilDstImage);

   /* Load an image. */
   MbufLoad(SourceFile, MilSubImage00);

   MosPrintf(MIL_TEXT("*****************\n"));
   MosPrintf(MIL_TEXT("General filtering\n"));
   MosPrintf(MIL_TEXT("*****************\n\n"));

   /* We will time each filtering method:
      A destination image that is not displayed is used to have the real 
      operation time. Also the function must be called once before the 
      timing loop for more accurate time (dll load, ...).
   */

   /* Perform filtering using a Deriche filter. */
   MIL_ID MilLinearFilterIIRContext = MimAlloc(MilSystem, M_LINEAR_FILTER_IIR_CONTEXT, M_DEFAULT, M_NULL);
   MimControl(MilLinearFilterIIRContext, M_FILTER_SMOOTHNESS, 40);
   MimConvolve(MilSubImage00, MilSubImage01, MilLinearFilterIIRContext);

   MimConvolve(MilSubImage00, MilDstImage, MilLinearFilterIIRContext);
   MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);
   for (MIL_INT it = 0; it < NB_LOOP; ++it)
      MimConvolve(MilSubImage00, MilDstImage, MilLinearFilterIIRContext);
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);
   MosPrintf(MIL_TEXT("Deriche filter: %f ms\n"), Time*1000/NB_LOOP);
   MimFree(MilLinearFilterIIRContext);

   
   /* Perform filtering using a bilateral filter. */
   MimFilterAdaptive(M_BILATERAL, MilSubImage00, MilSubImage10, 20, 0.1, 5, M_DEFAULT);


   MimFilterAdaptive(M_BILATERAL, MilSubImage00, MilDstImage, 20, 0.1, 5, M_DEFAULT);
   MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);
   for (MIL_INT it = 0; it < NB_LOOP; ++it)
      MimFilterAdaptive(M_BILATERAL, MilSubImage00, MilDstImage, 20, 0.1, 5, M_DEFAULT);      
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);
   MosPrintf(MIL_TEXT("Bilateral filter: %f ms\n"), Time*1000/NB_LOOP);

   /* Perform filtering using a noise peak removal filter. */
   MimFilterAdaptive(M_NOISE_PEAK_REMOVAL, MilSubImage00, MilSubImage11, 5, 5, 15, M_DEFAULT);

   MimFilterAdaptive(M_NOISE_PEAK_REMOVAL, MilSubImage00, MilDstImage, 5, 5, 15, M_DEFAULT);
   MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);
   for (MIL_INT it = 0; it < NB_LOOP; ++it)
      MimFilterAdaptive(M_NOISE_PEAK_REMOVAL, MilSubImage00, MilDstImage, 5, 5, 15, M_DEFAULT);
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);
   MosPrintf(MIL_TEXT("Noise peak removal filter: %f ms\n\n"), Time*1000/NB_LOOP);

   /* Display the image buffer and prepare overlay annotations. */
   MdispSelect(MilDisplay, MilImageId);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayId);

   MgraText(M_DEFAULT, MilOverlayId,     0,     0, MIL_TEXT("Source image"));
   MgraText(M_DEFAULT, MilOverlayId, SizeX,     0, MIL_TEXT("Deriche filter"));
   MgraText(M_DEFAULT, MilOverlayId,     0, SizeY, MIL_TEXT("Bilateral filter"));
   MgraText(M_DEFAULT, MilOverlayId, SizeX, SizeY, MIL_TEXT("Noise peak removal filter"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("The image has been filtered using various techniques. \n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MbufFree(MilDstImage);
   MbufFree(MilSubImage11);
   MbufFree(MilSubImage10);
   MbufFree(MilSubImage01);
   MbufFree(MilSubImage00);
   MbufFree(MilImageId);
   }


/*****************************************************************************/
/* Salt and pepper filtering.                                                */
/*****************************************************************************/

#define  SIZE_X   271
#define  SIZE_Y   256
#define  BUFFER_TYPE M_UNSIGNED+8
/* Source image file. */
MIL_CONST_TEXT_PTR ORIGINAL_IMAGE = M_IMAGE_PATH MIL_TEXT("CircuitBoardPart0.mim");
 
#define SEED_VALUE 42

void AddSaltAndPepperNoise(MIL_ID MilSystem,
                           MIL_ID MilImageId)
   {
   MIL_ID   AugmentContext;

   /* Allocate a DataAugment Context */
   MimAlloc(MilSystem, M_AUGMENTATION_CONTEXT, M_DEFAULT, &AugmentContext);

   /* Generate multiple results. To make them repeatable, the randomness can be controlled with the seed. */
   MimControl(AugmentContext, M_AUG_SEED_MODE, M_RNG_INIT_VALUE);
   MimControl(AugmentContext, M_AUG_RNG_INIT_VALUE, SEED_VALUE);

   /* Noise: Salt And Pepper */
   MimControl(AugmentContext, M_AUG_NOISE_SALT_PEPPER_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_NOISE_SALT_PEPPER_OP_DENSITY, 0.035);

   MimAugment(AugmentContext, MilImageId, MilImageId, M_DEFAULT, M_DEFAULT);

   MimFree(AugmentContext);
   }

void SaltAndPepperFilter (MIL_ID MilSystem,
                          MIL_ID MilDisplay)
   {
   MIL_ID MilImageId,                     /* Image buffer identifier.        */
          MilSubImage00,                  /* Child buffer identifier.        */
          MilSubImage01,                  /* Child buffer identifier.        */ 
          MilSubImage10,                  /* Child buffer identifier.        */
          MilSubImage11,                  /* Child buffer identifier.        */
          MilDstImage,                    /* Image buffer identifier.        */
          MilOverlayId;                   /* Overlay image buffer identifier.*/
          
   MIL_DOUBLE Time;

   /* Allocate image buffers. */
   MbufAlloc2d(MilSystem, 2 * SIZE_X, 2 * SIZE_Y, BUFFER_TYPE, M_IMAGE + M_PROC + M_DISP, &MilImageId);
   MbufChild2d(MilImageId,      0,      0, SIZE_X, SIZE_Y, &MilSubImage00);
   MbufChild2d(MilImageId, SIZE_X,      0, SIZE_X, SIZE_Y, &MilSubImage01);
   MbufChild2d(MilImageId,      0, SIZE_Y, SIZE_X, SIZE_Y, &MilSubImage10);
   MbufChild2d(MilImageId, SIZE_X, SIZE_Y, SIZE_X, SIZE_Y, &MilSubImage11);
   MbufAlloc2d(MilSystem, SIZE_X, SIZE_Y, BUFFER_TYPE, M_IMAGE + M_PROC + M_DISP, &MilDstImage);

   MbufClear(MilImageId, 0);

   /* Load an image. */
   MbufLoad(ORIGINAL_IMAGE, MilSubImage00);

   /* Add noise. */
   MbufCopy(MilSubImage00, MilSubImage01);
   AddSaltAndPepperNoise(MilSystem, MilSubImage01);

   MosPrintf(MIL_TEXT("*************************\n"));
   MosPrintf(MIL_TEXT("Salt and pepper filtering\n"));
   MosPrintf(MIL_TEXT("*************************\n\n"));

   /* We will time each filtering method:
      A destination image that is not displayed is used to have the real 
      operation time. Also the function must be called once before the 
      timing loop for more accurate time (dll load, ...).
   */

   /* Perform filtering using a median filter. */
   MimRank(MilSubImage01, MilSubImage10, M_3X3_RECT, M_MEDIAN, M_GRAYSCALE);

   MimRank(MilSubImage01, MilSubImage10, M_3X3_RECT, M_MEDIAN, M_GRAYSCALE);
   MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);
   for (MIL_INT it = 0; it < NB_LOOP; ++it)
      MimRank(MilSubImage01, MilSubImage10, M_3X3_RECT, M_MEDIAN, M_GRAYSCALE);
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);
   MosPrintf(MIL_TEXT("Median rank: %f ms\n"), Time*1000/NB_LOOP);

   /* Perform filtering using a noise peak removal filter. */
   MIL_DOUBLE  NbIter         = 3;
   MIL_DOUBLE  Gap            = 0;
   /* Use MinVariation to avoid blurring the text included in the image. */
   MIL_DOUBLE  MinVariation   = 30;
   MimFilterAdaptive(M_NOISE_PEAK_REMOVAL, MilSubImage01, MilSubImage11, NbIter, Gap, MinVariation, M_DEFAULT);

   MimFilterAdaptive(M_NOISE_PEAK_REMOVAL, MilSubImage01, MilDstImage, NbIter, Gap, MinVariation, M_DEFAULT);
   MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);
   for (MIL_INT it = 0; it < NB_LOOP; ++it)
      MimFilterAdaptive(M_NOISE_PEAK_REMOVAL, MilSubImage01, MilDstImage, NbIter, Gap, MinVariation, M_DEFAULT);

   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);
   MosPrintf(MIL_TEXT("Noise peak removal filter: %f ms\n\n"), Time*1000/NB_LOOP);

   /* Display the image buffer and prepare overlay annotations. */
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   MdispSelect(MilDisplay, MilImageId);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayId);

   MgraText(M_DEFAULT, MilOverlayId,      0,      0, MIL_TEXT("Original image"));
   MgraText(M_DEFAULT, MilOverlayId, SIZE_X,      0, MIL_TEXT("Noisy image"));
   MgraText(M_DEFAULT, MilOverlayId,      0, SIZE_Y, MIL_TEXT("Median rank filter"));
   MgraText(M_DEFAULT, MilOverlayId, SIZE_X, SIZE_Y, MIL_TEXT("Noise peak removal filter"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("The noise peak removal technique better preserves the edges.\n"));
   MosPrintf(MIL_TEXT("Also, it only modifies pixels that fit the parameters; the median\nrank modifies all pixels.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   MbufFree(MilDstImage);
   MbufFree(MilSubImage11);
   MbufFree(MilSubImage10);
   MbufFree(MilSubImage01);
   MbufFree(MilSubImage00);
   MbufFree(MilImageId);
   }
