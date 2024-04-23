/*****************************************************************************************/
/*
 * File name: DMILAddConstantSlave.cpp
 *
 * Synopsis:  This example shows how to use the MIL Function Development module to
 *            create a custom MIL function that adds a constant to an image using
 *            custom C code that access the buffer's data pointer directly.
 *
 *            The example creates a Slave function which retrieves all the parameters
 *            and calls a Target processing function that adds a constant to the source
 *            buffer and writes the result to the target image.
 *
 *            Note: For simplicity, the images are assumed to be 8-bit unsigned and of
 *            the same dimensions.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

/* Standard MIL header. */
#include <mil.h>



/* Slave and Target MIL functions declarations */
#ifdef __cplusplus
extern "C" {
#endif

   void MFTYPE SlaveAddConstantC(MIL_ID Func);
   void MFTYPE TargetAddConstantC(MIL_ID SrcImage, MIL_ID DstImage, MIL_UINT Constant);
#ifdef __cplusplus
   }
#endif

/* Slave MIL Function definition. */
/* ------------------------------ */

void MFTYPE SlaveAddConstantC(MIL_ID Func)
   {
   MIL_ID SrcImage, DstImage;
   MIL_UINT Constant;

   /* Now we are on the Target system and we will call the Target processing function. */

   /* Retrieve the function parameters. */
   MfuncParamValue(Func, 1, &SrcImage);
   MfuncParamValue(Func, 2, &DstImage);
   MfuncParamValue(Func, 3, &Constant);

   /* Call the target slave function. */
   TargetAddConstantC(SrcImage, DstImage, Constant);
   }


/* Target MIL processing function definition using C code. */
/* ------------------------------------------------------- */
/*
 * This function adds a constant to a MIL buffer.
 *
 * For simplicity, it assumes that the buffers are 8-bit unsigned
 * buffers of the same dimensions.
 */

/* Target function restrictions and error code. */
#define FUNCTION_SUPPORTED_IMAGE_DEPTH    (8+M_UNSIGNED)
#define FUNCTION_PARAMETER_ERROR_CODE     1

void MFTYPE TargetAddConstantC(MIL_ID Src, MIL_ID Dst, MIL_UINT Constant)
   {
   unsigned char* pSrc, *pDst;
   MIL_UINT       PixelValue;
   MIL_INT        SizeX, SizeY, SrcPitch, DstPitch, x, y;

   /* Read the MIL buffers informations assuming same buffer dimensions. */
   MbufInquire(Src, M_HOST_ADDRESS, &pSrc);
   MbufInquire(Src, M_PITCH, &SrcPitch);
   MbufInquire(Dst, M_HOST_ADDRESS, &pDst);
   MbufInquire(Dst, M_SIZE_X, &SizeX);
   MbufInquire(Dst, M_SIZE_Y, &SizeY);
   MbufInquire(Dst, M_PITCH, &DstPitch);

    /* Lock the source and destination for direct access. */
   MbufControl(Src, M_LOCK, M_DEFAULT);
   MbufControl(Dst, M_LOCK, M_DEFAULT);

   if((pSrc != M_NULL) && (pDst != M_NULL))
      {
      /* If the images have the proper type and dimensions, process them.*/
      if ((MbufInquire(Src, M_TYPE, M_NULL) == FUNCTION_SUPPORTED_IMAGE_DEPTH) &&
          (MbufInquire(Dst, M_TYPE, M_NULL) == FUNCTION_SUPPORTED_IMAGE_DEPTH) &&
          (MbufInquire(Src, M_SIZE_X, M_NULL) == SizeX) &&
          (MbufInquire(Src, M_SIZE_Y, M_NULL) == SizeY)
         )
         {
         /* Add the constant using C code (taking saturation into account). */
         for(y = 0; y < SizeY; y++)
            {
            for(x = 0; x < SizeX; x++)
               {
               PixelValue = pSrc[x] + (unsigned char)Constant;
               pDst[x] = (PixelValue < 255) ? (unsigned char)PixelValue : 255;
               }
            
            /* Go to next line. */
            pSrc += SrcPitch;
            pDst += DstPitch;
            }

         /* Signal MIL that the destination buffer has been modified. */
         MbufControl(Dst, M_MODIFIED, M_DEFAULT);
         }
      else
         {
         /* Report a MIL error. */
         MfuncErrorReport(M_DEFAULT, M_FUNC_ERROR + FUNCTION_PARAMETER_ERROR_CODE,
                MIL_TEXT("Invalid parameter."),
                MIL_TEXT("Images must have the same dimensions and must be 8-bit unsigned."),
                M_NULL,
                M_NULL
                );
         }
      }
   else
      {
      /* Report a MIL error. */
      MfuncErrorReport(M_DEFAULT, M_FUNC_ERROR + FUNCTION_PARAMETER_ERROR_CODE,
            MIL_TEXT("Invalid parameter."),
            MIL_TEXT("One of the buffer has a NULL host address."),
            M_NULL,
            M_NULL
            );
      }
   
   /* Unlock the source and destination. */
   MbufControl(Src, M_UNLOCK, M_DEFAULT);
   MbufControl(Dst, M_UNLOCK, M_DEFAULT);
   }
