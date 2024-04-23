/*****************************************************************************
/*
* File name: WaveletTransformation.cpp
*
* Synopsis:  This program performs a wavelet transformation and
*            then displays the resulting wavelet transforms.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

/*****************************************************************************/
/* Example description.                                                      */
/*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("WaveletTransformation\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program performs a wavelet transformation and \n")
      MIL_TEXT("then displays the resulting wavelet transforms.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n")
      MIL_TEXT("image processing.\n\n"));
   }

/* Source images filename. */
MIL_CONST_TEXT_PTR SOURCE_IMAGE = M_IMAGE_PATH MIL_TEXT("lead.mim");

/* Wavelet transformation settings. */
#define WAVELET_TYPE          M_HAAR         // M_HAAR, M_SYMLET_4, M_DAUBECHIES_3_COMPLEX,...
#define TRANSFORMATION_MODE   M_DYADIC       // M_DYADIC, M_UNDECIMATED, or M_UNDECIMATED + M_CENTER
#define TRANSFORMATION_LEVEL  3              // Value >= 1
#define DRAW_OVERSCAN_COEF    M_FALSE        // M_FALSE or M_TRUE

/* Utility function. */
void SourceCompensation(MIL_ID&   MilSourceImage,
                        MIL_INT   TransformationMode,
                        MIL_INT   TransformationLevel);

/****************************************************************************/
int MosMain(void)
   {
   PrintHeader();

   MIL_ID MilApplication,      /* Application identifier.                     */
          MilSystem,           /* System identifier.                          */
          MilDisplayRe,        /* Display identifier.                         */
          MilDisplayIm;        /* Display identifier.                         */

   MIL_ID MilSourceImage,      /* Image source buffer.                        */
          MilDestinationRe,    /* Destination buffer for real coef.           */
          MilDestinationIm,    /* Destination buffer for imaginary coef.      */
          MilWaveletContext,   /* Wavelet context identifier.                 */
          MilWaveletResult;    /* Wavelet result identifier.                  */

   MIL_INT SizeX,              /* Source image size X.                        */
           SizeY,              /* Source image size Y.                        */
           SizeBand;           /* Source image number of bands.               */

   MIL_INT DestSizeX,          /* Destination size X.                         */
           DestSizeY;          /* Destination size Y.                         */

   MIL_INT TransformationType; /* Type of the transformation, real or complex.*/

   /* Allocate defaults. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);

   /* Allocate displays. */
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplayRe);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplayIm);

   /* Restore then display the source image. */
   MbufRestore(SOURCE_IMAGE, MilSystem, &MilSourceImage);

   MdispSelect(MilDisplayRe, MilSourceImage);
   MosPrintf(MIL_TEXT("\nThe source image is displayed.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Compensation of the source sizes, if needed. */
   SourceCompensation(MilSourceImage, TRANSFORMATION_MODE, TRANSFORMATION_LEVEL);
   
   MbufInquire(MilSourceImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSourceImage, M_SIZE_Y, &SizeY);
   MbufInquire(MilSourceImage, M_SIZE_BAND, &SizeBand);

   /* Allocate the wavelet context. */
   MimAlloc(MilSystem, M_WAVELET_TRANSFORM_CONTEXT, M_DEFAULT, &MilWaveletContext);

   /* Set wavelet context parameters. */
   MimControl(MilWaveletContext, M_WAVELET_TYPE, WAVELET_TYPE);
   MimControl(MilWaveletContext, M_TRANSFORMATION_MODE, TRANSFORMATION_MODE);

   /* Allocate a wavelet result. */
   MimAllocResult(MilSystem, M_DEFAULT, M_WAVELET_TRANSFORM_RESULT, &MilWaveletResult);
   
   /* Compute the wavelet transformation. */
   MimWaveletTransform(MilWaveletContext, MilSourceImage, MilWaveletResult, M_FORWARD, TRANSFORMATION_LEVEL, M_DEFAULT);

   /* Retrieve the sizes to draw the transformation. */
   if (DRAW_OVERSCAN_COEF==M_FALSE)
      {
      MimGetResult(MilWaveletResult, M_WAVELET_DRAW_SIZE_X, &DestSizeX);
      MimGetResult(MilWaveletResult, M_WAVELET_DRAW_SIZE_Y, &DestSizeY);
      }
   else
      {
      MimGetResult(MilWaveletResult, M_WAVELET_DRAW_SIZE_X_WITH_PADDING, &DestSizeX);
      MimGetResult(MilWaveletResult, M_WAVELET_DRAW_SIZE_Y_WITH_PADDING, &DestSizeY);
      }

   /* Retrieve whether the transformation has an imaginary part. */
   MimGetResult(MilWaveletResult, M_TRANSFORMATION_DOMAIN, &TransformationType);

   if (TransformationType == M_REAL)
      {
      /* Allocate the destination buffer. */
      MbufAllocColor(MilSystem, SizeBand, DestSizeX, DestSizeY, 32 + M_FLOAT, M_IMAGE + M_PROC + M_DISP, &MilDestinationRe);

      /* Draw the transformation coefficients in the destination buffer. */
      if (DRAW_OVERSCAN_COEF == M_FALSE)
         MimDraw(M_DEFAULT, MilWaveletResult, M_NULL, MilDestinationRe, M_DRAW_WAVELET, 0, 0, M_AUTO_SCALE);
      else
         MimDraw(M_DEFAULT, MilWaveletResult, M_NULL, MilDestinationRe, M_DRAW_WAVELET_WITH_PADDING, 0, 0, M_AUTO_SCALE);

      /* Display the transformation coefficients. */
      MosPrintf(MIL_TEXT("\nThe wavelet transformation coefficients are displayed.\n\n"));
      MdispControl(MilDisplayRe, M_VIEW_MODE, M_AUTO_SCALE);
      MdispSelect(MilDisplayRe, MilDestinationRe);
      MdispControl(MilDisplayRe, M_TITLE, MIL_TEXT("Wavelet coefficients."));
      }
   else
      {
      /* Allocate the destination buffers. */
      MbufAllocColor(MilSystem, SizeBand, DestSizeX, DestSizeY, 32 + M_FLOAT, M_IMAGE + M_PROC + M_DISP, &MilDestinationRe);
      MbufAllocColor(MilSystem, SizeBand, DestSizeX, DestSizeY, 32 + M_FLOAT, M_IMAGE + M_PROC + M_DISP, &MilDestinationIm);

      /* Draw the transformation coefficients in the destination buffer. */
      if (DRAW_OVERSCAN_COEF == M_FALSE)
         {
         MimDraw(M_DEFAULT, MilWaveletResult, M_NULL, MilDestinationRe, M_DRAW_WAVELET + M_REAL_PART, 0, 0, M_AUTO_SCALE);
         MimDraw(M_DEFAULT, MilWaveletResult, M_NULL, MilDestinationIm, M_DRAW_WAVELET + M_IMAGINARY_PART, 0, 0, M_AUTO_SCALE);
         }
      else
         {
         MimDraw(M_DEFAULT, MilWaveletResult, M_NULL, MilDestinationRe, M_DRAW_WAVELET_WITH_PADDING + M_REAL_PART, 0, 0, M_AUTO_SCALE);
         MimDraw(M_DEFAULT, MilWaveletResult, M_NULL, MilDestinationIm, M_DRAW_WAVELET_WITH_PADDING + M_IMAGINARY_PART, 0, 0, M_AUTO_SCALE);
         }

      /* Display the transformation coefficients. */
      MosPrintf(MIL_TEXT("\nThe wavelet transformation complex coefficients are displayed.\n\n"));

      MdispControl(MilDisplayRe, M_VIEW_MODE, M_AUTO_SCALE);
      MdispSelect(MilDisplayRe, MilDestinationRe);
      MdispControl(MilDisplayRe, M_TITLE, MIL_TEXT("Complex wavelet coefficients (real part)."));

      MdispControl(MilDisplayIm, M_VIEW_MODE, M_AUTO_SCALE);
      MdispSelect(MilDisplayIm, MilDestinationIm);
      MdispControl(MilDisplayIm, M_TITLE, MIL_TEXT("Complex wavelet coefficients (imaginary part)."));
      }

   MosPrintf(MIL_TEXT("Press <Enter> to terminate.\n\n"));
   MosGetch();

   /* Free resources. */
   MbufFree(MilSourceImage);
   MbufFree(MilDestinationRe);

   if(TransformationType == M_COMPLEX)
      MbufFree(MilDestinationIm);

   MimFree(MilWaveletContext);
   MimFree(MilWaveletResult);

   MdispFree(MilDisplayRe);
   MdispFree(MilDisplayIm);
   
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }


/******************************************************************************
/* If needed, this utility function compensates for the sizes of the source buffer.
/*
/* To perform an undecimated decomposition with N levels, the source image  
/* size X and size Y must be multiples of 2^N.
/* If this is not the case, a larger image padded with zeros is allocated and 
/* used to hold the source image.
*/
void SourceCompensation(MIL_ID&   MilSourceImage,
                        MIL_INT   TransformationMode,
                        MIL_INT   TransformationLevel)
   {   
   if(TransformationMode == M_UNDECIMATED          || 
      TransformationMode == M_UNDECIMATED + M_CENTER)
      {      
      MIL_INT SrcSizeX = MbufInquire(MilSourceImage, M_SIZE_X, M_NULL);
      MIL_INT SrcSizeY = MbufInquire(MilSourceImage, M_SIZE_Y, M_NULL);

      MIL_INT NewSrcSizeX = SrcSizeX;
      MIL_INT NewSrcSizeY = SrcSizeY;

      MIL_INT MaxLevelFactor  = (MIL_INT)1 << TransformationLevel;

      if(SrcSizeX%MaxLevelFactor != 0)
         NewSrcSizeX = SrcSizeX + MaxLevelFactor - SrcSizeX%MaxLevelFactor;
      
      if (SrcSizeY%MaxLevelFactor != 0)
         NewSrcSizeY = SrcSizeY + MaxLevelFactor - SrcSizeY%MaxLevelFactor;
      
      if ((SrcSizeX != NewSrcSizeX) || (SrcSizeY != NewSrcSizeY))
         {
         MIL_ID NewMilSourceImage = MbufClone(MilSourceImage, M_DEFAULT, NewSrcSizeX, NewSrcSizeY, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_NULL);
         MbufClear(NewMilSourceImage, 0);
         MbufCopy(MilSourceImage, NewMilSourceImage);
         MbufFree(MilSourceImage);
         MilSourceImage = NewMilSourceImage;
         }
      }
   }
