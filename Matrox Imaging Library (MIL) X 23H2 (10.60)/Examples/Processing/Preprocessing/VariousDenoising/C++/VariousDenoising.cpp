﻿/************************************************************************************/
/*
 * File name: VariousDenoising.cpp 
 *
 * Synopsis:  This program demonstrates various methods to denoise an image.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include "utilGenNoise.h"

/* Example functions prototypes. */
void VariousDenoising (MIL_ID             MilSystem                          , 
                       MIL_ID             MilDisplay                         , 
                       MIL_CONST_TEXT_PTR SourceForGaussianNoiseFilename     , 
                       MIL_CONST_TEXT_PTR SourceForPoissonNoiseFilename      , 
                       MIL_CONST_TEXT_PTR SourceForSaltAndPepperNoiseFilename);

void Denoise          (MIL_ID             MilSystem        , 
                       MIL_ID             MilDisplay       , 
                       MIL_ID             MilOriginalImage , 
                       MIL_ID             MilNoisyImage    , 
                       MIL_CONST_TEXT_PTR NoiseDescription ,
                       MIL_ID             MilWaveletContext, 
                       MIL_INT            SizeX            , 
                       MIL_INT            SizeY            , 
                       MIL_INT            SizeBand         ,
                       MIL_INT            Type             );

/* Source images filename. */
MIL_CONST_TEXT_PTR IMAGE_FOR_GAUSSIAN_NOISE        = M_IMAGE_PATH MIL_TEXT("Bird.mim");
MIL_CONST_TEXT_PTR IMAGE_FOR_POISSON_NOISE         = M_IMAGE_PATH MIL_TEXT("CircuitsBoard.mim");
MIL_CONST_TEXT_PTR IMAGE_FOR_SALT_AND_PAPPER_NOISE = M_IMAGE_PATH MIL_TEXT("LicPlate.mim");

/* Local variables definitions. */
const MIL_DOUBLE GAUSSIAN_NOISE_VAR            = 600;
const MIL_DOUBLE SALT_AND_PEPPER_NOISE_DENSITY = 0.1;
const MIL_INT    NB_DECOMPOSITION_LEVEL        = 4;
const MIL_INT    WAVELET_TYPE                  = M_DAUBECHIES_8;
const MIL_INT    DECOMPOSITION_MODE            = M_UNDECIMATED + M_CENTER;

/*****************************************************************************/
/* Example description.                                                      */
/*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("VariousDenoising\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program demonstrates how to denoise an image\n")
             MIL_TEXT("using different shrinkage methods for various types\n")
             MIL_TEXT("of noise.\n")
             MIL_TEXT("To evaluate the performance, the Mean Square Error\n")
             MIL_TEXT("(MSE) quality metric is computed.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("image processing.\n\n"));

   }

/****************************************************************************/
int MosMain(void)
   {
   srand(42);

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

   /* Run the wavelet denoising example. */
   VariousDenoising(MilSystem, MilDisplay, IMAGE_FOR_GAUSSIAN_NOISE, IMAGE_FOR_POISSON_NOISE, IMAGE_FOR_SALT_AND_PAPPER_NOISE);

   /* Free defaults. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

/*****************************************************************************/
/*  Compute the sizes needed according to the wavelet decomposition mode.    */
/*****************************************************************************/
void SizeModificationAccordingToWaveletMode(MIL_INT * SizeX               ,
                                            MIL_INT * SizeY               ,
                                            MIL_INT   DecompositionMode   ,
                                            MIL_INT   NbLevelDecomposition)
   {
   if(DecompositionMode == M_UNDECIMATED          ||
      DecompositionMode == M_UNDECIMATED + M_CENTER)
      {
      MIL_INT MaxLevelFactor  = MIL_INT(pow(2.0,int(NbLevelDecomposition))); 
      if((*SizeX%MaxLevelFactor != 0) || (*SizeY%MaxLevelFactor != 0))
         {
         *SizeX = *SizeX + MaxLevelFactor - *SizeX%MaxLevelFactor;
         *SizeY = *SizeY + MaxLevelFactor - *SizeY%MaxLevelFactor;
         }
      }
   }

/*****************************************************************************/
/*  Denoise various type of noisy image using different shrinkage methods.   */
/*****************************************************************************/
void VariousDenoising (MIL_ID             MilSystem                          , 
                       MIL_ID             MilDisplay                         , 
                       MIL_CONST_TEXT_PTR SourceForGaussianNoiseFilename     , 
                       MIL_CONST_TEXT_PTR SourceForPoissonNoiseFilename      , 
                       MIL_CONST_TEXT_PTR SourceForSaltAndPepperNoiseFilename)
   {
   MIL_ID MilSourceForGaussianNoise,      /* Image source for Gaussian noise buffer identifier.        */
          MilSourceForPoissonNoise,       /* Image source for Poisson noise buffer identifier.         */
          MilSourceForSaltAndPepper,      /* Image source for Salt and Pepper noise buffer identifier. */
          MilGaussianNoiseImage,          /* Gaussian noise image buffer identifier.                   */
          MilPoissonNoiseImage,           /* Poisson noise image buffer identifier.                    */
          MilSaltAndPepperImage,          /* Salt and Pepper noise image buffer identifier.            */
          MilWaveletContext;              /* Context defining the wavelet characteristics.             */

   MIL_INT Type = (8 + M_UNSIGNED);

   MIL_INT SizeXGaussian     , SizeYGaussian     , SizeBandGaussian     , TypeGaussian     ;
   MIL_INT SizeXPoisson      , SizeYPoisson      , SizeBandPoisson      , TypePoisson      ;
   MIL_INT SizeXSaltAndPepper, SizeYSaltAndPepper, SizeBandSaltAndPepper, TypeSaltAndPepper;

   /* Inquire the images size and type. */
   MbufDiskInquire(SourceForGaussianNoiseFilename, M_SIZE_X   , &SizeXGaussian   );
   MbufDiskInquire(SourceForGaussianNoiseFilename, M_SIZE_Y   , &SizeYGaussian   );
   MbufDiskInquire(SourceForGaussianNoiseFilename, M_SIZE_BAND, &SizeBandGaussian);
   MbufDiskInquire(SourceForGaussianNoiseFilename, M_TYPE     , &TypeGaussian    );

   MbufDiskInquire(SourceForPoissonNoiseFilename, M_SIZE_X   , &SizeXPoisson   );
   MbufDiskInquire(SourceForPoissonNoiseFilename, M_SIZE_Y   , &SizeYPoisson   );
   MbufDiskInquire(SourceForPoissonNoiseFilename, M_SIZE_BAND, &SizeBandPoisson);
   MbufDiskInquire(SourceForPoissonNoiseFilename, M_TYPE     , &TypePoisson    );

   MbufDiskInquire(SourceForSaltAndPepperNoiseFilename, M_SIZE_X   , &SizeXSaltAndPepper   );
   MbufDiskInquire(SourceForSaltAndPepperNoiseFilename, M_SIZE_Y   , &SizeYSaltAndPepper   );
   MbufDiskInquire(SourceForSaltAndPepperNoiseFilename, M_SIZE_BAND, &SizeBandSaltAndPepper);
   MbufDiskInquire(SourceForSaltAndPepperNoiseFilename, M_TYPE     , &TypeSaltAndPepper    );

   /*If needed, resize the image to match the wavelet transform computation. */
   SizeModificationAccordingToWaveletMode(&SizeXGaussian     , &SizeYGaussian     , DECOMPOSITION_MODE, NB_DECOMPOSITION_LEVEL);
   SizeModificationAccordingToWaveletMode(&SizeXPoisson      , &SizeYPoisson      , DECOMPOSITION_MODE, NB_DECOMPOSITION_LEVEL);
   SizeModificationAccordingToWaveletMode(&SizeXSaltAndPepper, &SizeYSaltAndPepper, DECOMPOSITION_MODE, NB_DECOMPOSITION_LEVEL);

   /* Allocate all the display buffers and clear them. */
   MbufAllocColor(MilSystem, SizeBandGaussian     , SizeXGaussian     , SizeYGaussian     , Type, M_IMAGE+M_PROC+M_DISP, &MilSourceForGaussianNoise);
   MbufAllocColor(MilSystem, SizeBandPoisson      , SizeXPoisson      , SizeYPoisson      , Type, M_IMAGE+M_PROC+M_DISP, &MilSourceForPoissonNoise);
   MbufAllocColor(MilSystem, SizeBandSaltAndPepper, SizeXSaltAndPepper, SizeYSaltAndPepper, Type, M_IMAGE+M_PROC+M_DISP, &MilSourceForSaltAndPepper);
   MbufClone(MilSourceForGaussianNoise, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, &MilGaussianNoiseImage);
   MbufClone(MilSourceForPoissonNoise , M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, &MilPoissonNoiseImage );
   MbufClone(MilSourceForSaltAndPepper, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, &MilSaltAndPepperImage);

   /* Load the source images. */
   MbufClear(MilSourceForGaussianNoise, 0);
   MbufClear(MilSourceForPoissonNoise , 0);
   MbufClear(MilSourceForSaltAndPepper, 0);
   MbufLoad(SourceForGaussianNoiseFilename     , MilSourceForGaussianNoise);
   MbufLoad(SourceForPoissonNoiseFilename      , MilSourceForPoissonNoise );
   MbufLoad(SourceForSaltAndPepperNoiseFilename, MilSourceForSaltAndPepper);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nGenerating images..."));

   /* Generate the noisy images. */
   AddGaussianNoise     (MilSourceForGaussianNoise, MilGaussianNoiseImage, SizeXGaussian*SizeYGaussian*SizeBandGaussian, Type, GAUSSIAN_NOISE_VAR);
   AddPoissonNoise      (MilSourceForPoissonNoise , MilPoissonNoiseImage , SizeXPoisson*SizeYPoisson*SizeBandPoisson, Type);
   AddSaltAndPepperNoise(MilSourceForSaltAndPepper, MilSaltAndPepperImage, SizeXSaltAndPepper*SizeYSaltAndPepper, SizeBandSaltAndPepper, Type, SALT_AND_PEPPER_NOISE_DENSITY);
   MosPrintf(MIL_TEXT(" Done.\n"));

   /* Allocate the wavelet context and specify some characteristics */
   MimAlloc(MilSystem, M_WAVELET_TRANSFORM_CONTEXT, M_DEFAULT, &MilWaveletContext);
   MimControl(MilWaveletContext, M_WAVELET_TYPE       , WAVELET_TYPE      );
   MimControl(MilWaveletContext, M_TRANSFORMATION_MODE, DECOMPOSITION_MODE);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nA White Gaussian noise is applied to the image.")
             MIL_TEXT("\nThe results of the denoising operation and the")
             MIL_TEXT("\nquality metrics are displayed for the different")
             MIL_TEXT("\nshrinkage methods.\n"));

   /* Add / Remove Gaussian Noise to the original image. */
   Denoise(MilSystem                         , 
           MilDisplay                        , 
           MilSourceForGaussianNoise         , 
           MilGaussianNoiseImage             ,
           MIL_TEXT(" White Gaussian noise "),
           MilWaveletContext                 , 
           SizeXGaussian                     , 
           SizeYGaussian                     ,
           SizeBandGaussian                  ,
           Type                             );

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nA Poisson noise is applied to the image.")
             MIL_TEXT("\nThe results of the denoising operation and the")
             MIL_TEXT("\nquality metrics are displayed for the different")
             MIL_TEXT("\nshrinkage methods.\n"));

  /* Add/Remove Poisson Noise to the original image. */
   Denoise(MilSystem                  , 
           MilDisplay                 , 
           MilSourceForPoissonNoise   , 
           MilPoissonNoiseImage       ,
           MIL_TEXT(" Poisson noise "),
           MilWaveletContext          , 
           SizeXPoisson               , 
           SizeYPoisson               , 
           SizeBandPoisson            ,
           Type                      );

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nA Salt and Pepper noise is applied to the image.")
             MIL_TEXT("\nThe results of the denoising operation and the")
             MIL_TEXT("\nquality metrics are displayed for the different")
             MIL_TEXT("\nshrinkage methods.\n"));

   /* Add/Remove Salt and Pepper Noise to the original image. */
   Denoise(MilSystem                          , 
           MilDisplay                         , 
           MilSourceForSaltAndPepper          , 
           MilSaltAndPepperImage              ,
           MIL_TEXT(" Salt And Pepper noise "),
           MilWaveletContext                  , 
           SizeXSaltAndPepper                 , 
           SizeYSaltAndPepper                 , 
           SizeBandSaltAndPepper              ,
           Type                              );

   /* Free buffers. */
   MbufFree(MilSourceForGaussianNoise);
   MbufFree(MilSourceForPoissonNoise );
   MbufFree(MilSourceForSaltAndPepper);
   MbufFree(MilGaussianNoiseImage    );
   MbufFree(MilPoissonNoiseImage     );
   MbufFree(MilSaltAndPepperImage    );
   MimFree (MilWaveletContext        );
   }

/*************************************************************************/
/*  Denoise one type of noisy image using different shrinkage methods.   */
/*************************************************************************/
void Denoise(MIL_ID             MilSystem        , 
             MIL_ID             MilDisplay       , 
             MIL_ID             MilOriginalImage , 
             MIL_ID             MilNoisyImage    , 
             MIL_CONST_TEXT_PTR NoiseDescription ,
             MIL_ID             MilWaveletContext, 
             MIL_INT            SizeX            , 
             MIL_INT            SizeY            , 
             MIL_INT            SizeBand         ,
             MIL_INT            Type             )
   {
   MIL_INT            OverlayClearColor,        /* Overlay clear color                                     */
                      DisplaySizeX,             /* Display size X                                          */
                      DisplaySizeY,             /* Display size Y                                          */
                      OffsetSize;               /* Display size between the original and the noisy images. */
   MIL_DOUBLE         MSE;                      /* Mean Square Error.                                      */
   MIL_ID             MilImageDisplay,          /* Images of the de-noising process.                       */
                      MilImageResult,           /* Images of the de-noising process.                       */
                      MilChildImageDisplay,     /* De-noising result using different shrink method.        */
                      MilChildColorImageResult, /* One band of the result image.                           */
                      MilChildColorNoisyImage,  /* One band of the noisy image.                            */
                      MilOverlayImage;          /* Overlay image buffer identifier.                        */
   
   /* Define the display buffers size and the offset size. */
   DisplaySizeX = (256 > SizeX) ? SizeX : 256;
   DisplaySizeY = (256 > SizeY) ? SizeY : 256;
   OffsetSize   = DisplaySizeX/3;

   /* Allocate the display buffers and clear it. */
   MbufAllocColor  (MilSystem, SizeBand, SizeX, SizeY, Type, M_IMAGE+M_PROC+M_DISP, &MilImageResult);
   MbufAllocColor  (MilSystem, SizeBand, 3*DisplaySizeX, 3*DisplaySizeY, Type, M_IMAGE+M_PROC+M_DISP, &MilImageDisplay);
   MbufChildColor2d(MilImageDisplay, M_ALL_BANDS, OffsetSize, 0, DisplaySizeX, DisplaySizeY, &MilChildImageDisplay);
   MbufClear(MilImageDisplay, 0);

   /* Display the result buffer and prepare for overlay annotations. */
   MdispSelect (MilDisplay, MilImageDisplay);
   MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, &OverlayClearColor);
   MdispInquire(MilDisplay, M_OVERLAY_ID       , &MilOverlayImage  );
   MbufClear(MilOverlayImage, (MIL_DOUBLE)OverlayClearColor);
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

   /* Display the original image */
   MbufCopy(MilOriginalImage, MilChildImageDisplay);
   MgraText(M_DEFAULT, MilOverlayImage, OffsetSize, 0, MIL_TEXT(" Original image ")); 
   
   /* Display the noisy image and evaluate MSE. */
   MbufChildMove(MilChildImageDisplay, DisplaySizeX + 2*OffsetSize, 0, DisplaySizeX, DisplaySizeY, M_DEFAULT);
   MbufCopy(MilNoisyImage, MilChildImageDisplay);
   ComputeMSE(MilOriginalImage, MilNoisyImage, SizeX*SizeY*SizeBand, Type, &MSE);
   MgraText(M_DEFAULT, MilOverlayImage, DisplaySizeX+ 2*OffsetSize, 0, NoiseDescription); 
   MosPrintf(MIL_TEXT("\nNoisy Image\t\tMSE : %.2f"), MSE);

   /* Denoise the noisy image by median ranking and evaluate MSE. */
   MbufChildMove(MilChildImageDisplay, 0, DisplaySizeY, DisplaySizeX, DisplaySizeY, M_DEFAULT);
   for(MIL_INT i = 0; i < SizeBand; i++)
      {
      MbufChildColor(MilImageResult, i, &MilChildColorImageResult);
      MbufChildColor(MilNoisyImage , i, &MilChildColorNoisyImage );
      MimRank(MilChildColorNoisyImage, MilChildColorImageResult, M_3X3_RECT, M_MEDIAN, M_GRAYSCALE);
      MbufFree(MilChildColorNoisyImage );
      MbufFree(MilChildColorImageResult);
      }
   MbufCopy(MilImageResult, MilChildImageDisplay);
   ComputeMSE(MilOriginalImage, MilImageResult, SizeX*SizeY*SizeBand, Type, &MSE);
   MgraText(M_DEFAULT, MilOverlayImage, 0, DisplaySizeY, MIL_TEXT(" Median Ranking destination ")); 
   MosPrintf(MIL_TEXT("\nMedian Ranking\t\tMSE : %.2f"), MSE);

   /* Denoise the noisy image by smoothing (one iteration) and evaluate MSE */
   MbufChildMove(MilChildImageDisplay, DisplaySizeX, DisplaySizeY, DisplaySizeX, DisplaySizeY, M_DEFAULT);
   MimConvolve(MilNoisyImage, MilImageResult, M_SMOOTH);
   MbufCopy(MilImageResult, MilChildImageDisplay);
   ComputeMSE(MilOriginalImage, MilImageResult, SizeX*SizeY*SizeBand, Type, &MSE);
   MgraText(M_DEFAULT, MilOverlayImage, DisplaySizeX, DisplaySizeY, MIL_TEXT(" Smoothing destination (1 iter) ")); 
   MosPrintf(MIL_TEXT("\nSmoothing (1 iter)\tMSE : %.2f"), MSE);

   /* Denoise the noisy image by smoothing (ten iterations) and evaluate MSE */
   MbufChildMove(MilChildImageDisplay, 2*DisplaySizeX, DisplaySizeY, DisplaySizeX, DisplaySizeY, M_DEFAULT);
   MbufCopy(MilNoisyImage, MilImageResult);
   for(MIL_INT iter = 0; iter < 10; iter++)
      MimConvolve(MilImageResult, MilImageResult, M_SMOOTH);
   MbufCopy(MilImageResult, MilChildImageDisplay);
   ComputeMSE(MilOriginalImage, MilImageResult, SizeX*SizeY*SizeBand, Type, &MSE);
   MgraText(M_DEFAULT, MilOverlayImage, 2*DisplaySizeX, DisplaySizeY, MIL_TEXT(" Smoothing destination (10 iter) ")); 
   MosPrintf(MIL_TEXT("\nSmoothing (10 iter)\tMSE : %.2f"), MSE);

   /* Denoise the noisy image with the Bayes shrink method and evaluate MSE */
   MbufChildMove(MilChildImageDisplay, 0, 2*DisplaySizeY, DisplaySizeX, DisplaySizeY, M_DEFAULT);
   MimWaveletDenoise(MilWaveletContext, MilNoisyImage, MilImageResult, NB_DECOMPOSITION_LEVEL, M_BAYES_SHRINK, M_DEFAULT);
   MbufCopy(MilImageResult, MilChildImageDisplay);
   ComputeMSE(MilOriginalImage, MilImageResult, SizeX*SizeY*SizeBand, Type, &MSE);
   MgraText(M_DEFAULT, MilOverlayImage, 0, 2*DisplaySizeY, MIL_TEXT(" Bayes Shrink destination ")); 
   MosPrintf(MIL_TEXT("\nBayes Shrink\t\tMSE : %.2f"), MSE);

   /* Denoise the noisy image with the Sure shrink method and evaluate MSE. */
   MbufChildMove(MilChildImageDisplay, DisplaySizeX, 2*DisplaySizeY, DisplaySizeX, DisplaySizeY, M_DEFAULT);
   MimWaveletDenoise(MilWaveletContext, MilNoisyImage, MilImageResult , NB_DECOMPOSITION_LEVEL, M_SURE_SHRINK , M_DEFAULT);
   MbufCopy(MilImageResult, MilChildImageDisplay);
   ComputeMSE(MilOriginalImage, MilImageResult, SizeX*SizeY*SizeBand, Type, &MSE);
   MgraText(M_DEFAULT, MilOverlayImage, DisplaySizeX, 2*DisplaySizeY, MIL_TEXT(" Sure Shrink destination ")); 
   MosPrintf(MIL_TEXT("\nSure Shrink\t\tMSE : %.2f"), MSE);

   /* Denoise the noisy image with the Neigh shrink method and evaluate MSE. */
   MbufChildMove(MilChildImageDisplay, 2*DisplaySizeX, 2*DisplaySizeY, DisplaySizeX, DisplaySizeY, M_DEFAULT);
   MimWaveletDenoise(MilWaveletContext, MilNoisyImage, MilImageResult , NB_DECOMPOSITION_LEVEL, M_NEIGH_SHRINK, M_DEFAULT);
   MbufCopy(MilImageResult, MilChildImageDisplay);
   ComputeMSE(MilOriginalImage, MilImageResult, SizeX*SizeY*SizeBand, Type, &MSE);
   MgraText(M_DEFAULT, MilOverlayImage, 2*DisplaySizeX, 2*DisplaySizeY, MIL_TEXT(" Neigh Shrink destination ")); 
   MosPrintf(MIL_TEXT("\nNeigh Shrink\t\tMSE : %.2f\n"), MSE);
   
   /* Display the noisy buffer and the results of de-noising using different shrinking method. */
   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);   

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Free buffers. */
   MbufFree(MilChildImageDisplay);
   MbufFree(MilImageDisplay     );
   MbufFree(MilImageResult      );
   }
