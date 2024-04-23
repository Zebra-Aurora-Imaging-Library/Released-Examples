﻿/************************************************************************************/
/*
* File name: TextureStatistics.cpp
*
* Synopsis:  This example demonstrates how to calculate texture statistics.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

/* Example function prototype. */
void CalculateTextureStatistics(MIL_ID MilSystem, MIL_ID MilDisplayImage, MIL_ID MilOverlay, MIL_CONST_TEXT_PTR ImageFilename);

/* Source images file name. */
#define IMAGE_FABRIC                       M_IMAGE_PATH MIL_TEXT("Preprocessing/Fabric.mim")
#define IMAGE_ROUGH_FABRIC                 M_IMAGE_PATH MIL_TEXT("Preprocessing/RoughFabric.mim")
#define IMAGE_TOWEL                        M_IMAGE_PATH MIL_TEXT("Preprocessing/Towel.mim")
#define IMAGE_DOT                          M_IMAGE_PATH MIL_TEXT("Preprocessing/DotMatrixSerial.mim")
#define IMAGE_STRUCTURED_NOISE             M_IMAGE_PATH MIL_TEXT("noise.mim")
#define IMAGE_LARGE_WAFER                  M_IMAGE_PATH MIL_TEXT("LargeWafer.mim")

/* Parametric values definitions. */
#define STRING_LENGTH_MAX      128
#define TEXTURE_SIZE           256
#define X_DISPLACEMENT_OFFSET  2
#define Y_DISPLACEMENT_OFFSET  0

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("TextureStatistics\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to calculate texture statistics.\n\n")
             
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, buffer, display, \n")
             MIL_TEXT("graphic, image processing, system.\n\n"));

   }

int MosMain()
   {

   PrintHeader();

   MIL_ID MilApplication,    /* Application identifier.          */
      MilSystem,             /* System identifier.               */
      MilDisplay,            /* Display identifier.              */
      MilOverlay,            /* Overlay image buffer identifier. */
      MilOverlayChild,       /* Child image identifier           */
      MilImage,              /* Image buffer identifier.         */
      MilSampleImage;        /* Child image identifier.          */

   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);
      
   /* Allocate the image to display. */
   MbufAlloc2d(MilSystem, 
               4 * TEXTURE_SIZE, 
               3 * TEXTURE_SIZE, 
               32 + M_UNSIGNED, 
               M_PROC + M_DISP + M_IMAGE,
               &MilImage);
   
   /* Select the image for display. */
   MdispSelect(MilDisplay, MilImage);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlay);

   /* Configure the graphic context. */
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_RIGHT );
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraControl(M_DEFAULT, M_COLOR, M_COLOR_LIGHT_BLUE );

   /* Allocate child image buffers. */
   MbufChild2d(MilImage, 
               0, 
               0, 
               2 * TEXTURE_SIZE, 
               TEXTURE_SIZE, 
               &MilSampleImage);

   MbufChildColor2d(MilOverlay, 
                    M_ALL_BANDS, 
                    0, 
                    0, 
                    2 * TEXTURE_SIZE, 
                    TEXTURE_SIZE, 
                    &MilOverlayChild);

   /* Calculate texture statistics on first image. */
   CalculateTextureStatistics(MilSystem, MilSampleImage, MilOverlayChild, IMAGE_FABRIC);

   /* Move child image buffers. */
   MbufChildMove(MilSampleImage,  2 * TEXTURE_SIZE, 0, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);
   MbufChildMove(MilOverlayChild, 2 * TEXTURE_SIZE, 0, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);
   
   /* Calculate texture statistics on second image. */
   CalculateTextureStatistics(MilSystem, MilSampleImage, MilOverlayChild, IMAGE_DOT);

   /* Move child image buffers. */
   MbufChildMove(MilSampleImage,  0, TEXTURE_SIZE, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);
   MbufChildMove(MilOverlayChild, 0, TEXTURE_SIZE, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);

   /* Calculate texture statistics on third image. */
   CalculateTextureStatistics(MilSystem, MilSampleImage, MilOverlayChild, IMAGE_TOWEL);

   /* Move child image buffers. */
   MbufChildMove(MilSampleImage,  2 * TEXTURE_SIZE, TEXTURE_SIZE, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);
   MbufChildMove(MilOverlayChild, 2 * TEXTURE_SIZE, TEXTURE_SIZE, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);

   /* Calculate texture statistics on fourth image. */
   CalculateTextureStatistics(MilSystem, MilSampleImage, MilOverlayChild, IMAGE_ROUGH_FABRIC);

   /* Move child image buffers. */
   MbufChildMove(MilSampleImage,  0, 2 * TEXTURE_SIZE, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);
   MbufChildMove(MilOverlayChild, 0, 2 * TEXTURE_SIZE, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);
   
   /* Calculate texture statistics on fifth image. */
   CalculateTextureStatistics(MilSystem, MilSampleImage, MilOverlayChild, IMAGE_LARGE_WAFER);

   /* Move child image buffers. */
   MbufChildMove(MilSampleImage,  2 * TEXTURE_SIZE, 2 * TEXTURE_SIZE, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);
   MbufChildMove(MilOverlayChild, 2 * TEXTURE_SIZE, 2 * TEXTURE_SIZE, 2 * TEXTURE_SIZE, TEXTURE_SIZE, M_DEFAULT);

   /* Calculate texture statistics on sixth image. */
   CalculateTextureStatistics(MilSystem, MilSampleImage, MilOverlayChild, IMAGE_STRUCTURED_NOISE);

   MosPrintf(MIL_TEXT("The co-occurrence matrices (GLCM) have been calculated for\n")
             MIL_TEXT("each texture sample and are displayed.\n\n")
             MIL_TEXT("Texture metrics (Haralick statistics) derived from the GLCM\n")
             MIL_TEXT("are then calculated and printed for each sample.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Reset the default graphic context. */
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_DEFAULT );
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_DEFAULT);
   MgraControl(M_DEFAULT, M_COLOR, M_DEFAULT );

   /* Free the identifiers. */
   MbufFree(MilOverlayChild);
   MbufFree(MilSampleImage);
   MbufFree(MilImage);

   /* Free objects. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }


/******************************************************************************/
/* Calculates texture statistics based on the gray-level co-occurrence matrix */ 
/* of an image.                                                               */
   void  CalculateTextureStatistics(MIL_ID MilSystem, 
                                    MIL_ID MilImage, 
                                    MIL_ID MilOverlay, 
                                    MIL_CONST_TEXT_PTR ImageFilename)
   {
   MIL_ID   MilTextureImage,        /* Image buffer identifier.         */
            TextureDisplaySample,   /* Child image buffer identifier.   */
            CooccurrenceMatrixId,   /* Child image buffer identifier.   */
            MilContext,             /* Processing context identifier.   */
            MilResult;              /* Processing result identifier.    */
   
   /* Allocate a statistical context. */
   MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, &MilContext);

   /* Allocate a statistical result. */
   MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, &MilResult);

   MIL_INT   Type, SizeX, SizeY;

   /* Inquire the image Size and Type. */
   MbufDiskInquire(ImageFilename, M_SIZE_X, &SizeX);
   MbufDiskInquire(ImageFilename, M_SIZE_Y, &SizeY);
   MbufDiskInquire(ImageFilename, M_TYPE,   &Type);

   /* Allocate image buffers. */
   MbufAlloc2d(MilSystem, SizeX, SizeY,
               Type, M_IMAGE + M_PROC, &MilTextureImage);

   /* Allocate child images. */
   MbufChild2d(MilImage,            0, 0, TEXTURE_SIZE, TEXTURE_SIZE, &TextureDisplaySample);
   MbufChild2d(MilImage, TEXTURE_SIZE, 0, TEXTURE_SIZE, TEXTURE_SIZE, &CooccurrenceMatrixId);

   /* Load texture image. */
   MbufLoad(ImageFilename, MilTextureImage);

   /* Calculate the sample coordinates. */
   const MIL_INT SizeDifferenceX = (SizeX - TEXTURE_SIZE) / 2;
   const MIL_INT SizeDifferenceY = (SizeY - TEXTURE_SIZE) / 2;
   const MIL_INT SrcOffsetX = SizeDifferenceX > 0 ? SizeDifferenceX : 0;
   const MIL_INT SrcOffsetY = SizeDifferenceY > 0 ? SizeDifferenceY : 0;
   const MIL_INT DstOffsetX = SizeDifferenceX > 0 ? 0 : -SizeDifferenceX;
   const MIL_INT DstOffsetY = SizeDifferenceY > 0 ? 0 : -SizeDifferenceY;
   const MIL_INT SizeXToCopy = SizeX > TEXTURE_SIZE ? TEXTURE_SIZE : SizeX;
   const MIL_INT SizeYToCopy = SizeY > TEXTURE_SIZE ? TEXTURE_SIZE : SizeY;

   /* Copy a sample of the image to the display buffer. */
   MbufCopyColor2d(MilTextureImage, 
                   TextureDisplaySample, 
                   M_ALL_BANDS, 
                   SrcOffsetX, 
                   SrcOffsetY, 
                   M_ALL_BANDS, 
                   DstOffsetX, 
                   DstOffsetY, 
                   SizeXToCopy, 
                   SizeYToCopy);

   /* Set the distance between paired pixels. */
   MimControl(MilContext, M_GLCM_PAIR_OFFSET_X, X_DISPLACEMENT_OFFSET);
   MimControl(MilContext, M_GLCM_PAIR_OFFSET_Y, Y_DISPLACEMENT_OFFSET);

   /* Enable the GLCM statistics to be computed for the texture. */
   MimControl(MilContext, M_STAT_GLCM_ENERGY,         M_ENABLE);
   MimControl(MilContext, M_STAT_GLCM_CONTRAST,       M_ENABLE);
   MimControl(MilContext, M_STAT_GLCM_CORRELATION,    M_ENABLE);
   MimControl(MilContext, M_STAT_GLCM_ENTROPY,        M_ENABLE);
   MimControl(MilContext, M_STAT_GLCM_DISSIMILARITY,  M_ENABLE);
   MimControl(MilContext, M_STAT_GLCM_HOMOGENEITY,    M_ENABLE);

   /* Calculate the texture's statistics. */
   MimStatCalculate(MilContext, MilTextureImage, MilResult, M_DEFAULT);

   MIL_DOUBLE Energy  = 0, 
              Entropy = 0, 
              Contrast = 0, 
              Dissimilarity = 0, 
              Homogeneity = 0, 
              Correlation = 0,
              Max = 0;

   MimGetResult(MilResult, M_STAT_GLCM_ENERGY,          &Energy);
   MimGetResult(MilResult, M_STAT_GLCM_CONTRAST,        &Contrast);
   MimGetResult(MilResult, M_STAT_GLCM_ENTROPY,         &Entropy);
   MimGetResult(MilResult, M_STAT_GLCM_DISSIMILARITY,   &Dissimilarity);
   MimGetResult(MilResult, M_STAT_GLCM_HOMOGENEITY,     &Homogeneity);
   MimGetResult(MilResult, M_STAT_GLCM_CORRELATION,     &Correlation);
   
   
   /* Display the texture's statistics. */
   MIL_TEXT_CHAR TextToDisplay[STRING_LENGTH_MAX];
   MosSprintf(TextToDisplay, STRING_LENGTH_MAX, MIL_TEXT("Dissimilarity: %4.2f"), Dissimilarity);
   MgraText(M_DEFAULT, MilOverlay, 510, 0, TextToDisplay);
   MosSprintf(TextToDisplay, STRING_LENGTH_MAX, MIL_TEXT("Homogeneity: %4.2f"),   Homogeneity);
   MgraText(M_DEFAULT, MilOverlay, 510, 15, TextToDisplay);
   MosSprintf(TextToDisplay, STRING_LENGTH_MAX, MIL_TEXT("Correlation: %4.2f"),   Correlation);
   MgraText(M_DEFAULT, MilOverlay, 510, 30, TextToDisplay);
   MosSprintf(TextToDisplay, STRING_LENGTH_MAX, MIL_TEXT("Contrast: %4.2f"),      Contrast);
   MgraText(M_DEFAULT, MilOverlay, 510, 45, TextToDisplay);
   MosSprintf(TextToDisplay, STRING_LENGTH_MAX, MIL_TEXT("Entropy: %4.2f"),       Entropy);   
   MgraText(M_DEFAULT, MilOverlay, 510, 60, TextToDisplay);
   MosSprintf(TextToDisplay, STRING_LENGTH_MAX, MIL_TEXT("Energy: %4.2f"),        Energy);
   MgraText(M_DEFAULT, MilOverlay, 510, 75, TextToDisplay);
   
   /* Draw the gray-level co-occurrence matrix (GLCM) to the display buffer. */
   MimDraw(M_DEFAULT, 
           MilResult, 
           M_NULL, 
           CooccurrenceMatrixId, 
           M_DRAW_GLCM_MATRIX, 
           M_NULL, 
           M_NULL, 
           M_DEFAULT);
   
   /* Adjust the range of the GLCM for the display. */
   MimStatCalculate(M_STAT_CONTEXT_MAX, CooccurrenceMatrixId, MilResult, M_DEFAULT);
   MimGetResult(MilResult, M_STAT_MAX,                  &Max);

   MimArith(CooccurrenceMatrixId, 255.0 / Max, CooccurrenceMatrixId, M_MULT_CONST + M_FLOAT_PROC);

   /* Free identifiers. */
   MimFree(MilResult);
   MimFree(MilContext);
   MbufFree(TextureDisplaySample);
   MbufFree(CooccurrenceMatrixId);
   MbufFree(MilTextureImage);

   }
