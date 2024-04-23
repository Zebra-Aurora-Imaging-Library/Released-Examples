/////////////////////////////////////////////////////////////////////////////////////////
// 
// File name   :  MimIIRFilter.cpp
// 
// Synopsis    :  This example shows how to use an IIR filter context with 
//                MimConvolve and MimDifferential.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
/////////////////////////////////////////////////////////////////////////////////////////
#include <mil.h>

#define IMAGE_FILE       M_IMAGE_PATH MIL_TEXT("Wafer.mim")

// Example description.                                                     
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("MimIIRFilter\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows how to use an IIR filter context\n")
             MIL_TEXT("with MimConvolve and MimDifferential.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, buffer, display,       \n")
             MIL_TEXT("image processing, system.                      \n\n"));

   // Wait for a key press.
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

// Main.
int MosMain(void)
{
   MIL_ID MilApplication,              /* Application identifier.       */
          MilSystem,                   /* System identifier.            */
          MilSrcImage,                 /* Image identifier.             */
          MilConvolveDispImage,        /* Display image buffer.         */
          MilTmpDstSmoothImage,        /* Image identifier.             */
          MilDstSmoothImage,           /* Image identifier.             */
          MilDstFDerXImage,            /* Image identifier.             */
          MilDstFDerYImage,            /* Image identifier.             */
          MilDstSDerXImage,            /* Image identifier.             */
          MilDstSDerYImage,            /* Image identifier.             */
          MilDstSDerXYImage,           /* Image identifier.             */
          MilDifferentialDispImage,    /* Display image buffer.         */
          MilDstGradientIntImage,      /* Image identifier.             */
          MilDstGradientAngleImage,    /* Image identifier.             */
          MilTmpDstLaplacianImage,     /* Image identifier.             */
          MilDstLaplacianImage,        /* Image identifier.             */
          MilDstSharpenImage,          /* Image identifier.             */
          MilLinearFilterIIRContext,   /* Context identifier.           */
          MilSrcDisplay,               /* Display identifier.           */
          MilDstDisplay,               /* Display identifier.           */
          MilDstOverlay;               /* Display overlay identifier.   */

   PrintHeader();

   // Allocate application and system.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);

   // Allocate displays.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilSrcDisplay);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDstDisplay);
   MdispControl(MilDstDisplay, M_VIEW_MODE, M_AUTO_SCALE);

   // Restore source image.
   MbufRestore(IMAGE_FILE, MilSystem, &MilSrcImage);

   // Display source image.
   MdispSelect(MilSrcDisplay, MilSrcImage);
   MosPrintf(MIL_TEXT("The filter will be applied to the displayed source image.\n\n"));


   // Get the size of the images;
   MIL_INT ImageSizeX = MbufInquire(MilSrcImage, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(MilSrcImage, M_SIZE_Y, M_NULL);

   MosPrintf(MIL_TEXT("Choose a filter type:\n")
             MIL_TEXT("   1. M_DERICHE (default)\n")
             MIL_TEXT("   2. M_SHEN\n")
             MIL_TEXT("   3. M_VLIET\n\n")
             MIL_TEXT("Your choice : "));

   MIL_INT FilterType   = M_NULL;
   while(FilterType == M_NULL)
      {
      switch (MosGetch())
         {
         case MIL_TEXT('1'):
            FilterType  = M_DERICHE;
            MosPrintf(MIL_TEXT("1. M_DERICHE\n"));
            break;

         case MIL_TEXT('2'):
            FilterType  = M_SHEN;
            MosPrintf(MIL_TEXT("2. M_SHEN\n"));
            break;

         case MIL_TEXT('3'):
            FilterType  = M_VLIET;
            MosPrintf(MIL_TEXT("3. M_VLIET\n"));
            break;
         }
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate context.
   MimAlloc(MilSystem, M_LINEAR_FILTER_IIR_CONTEXT, M_DEFAULT, &MilLinearFilterIIRContext);
   MimControl(MilLinearFilterIIRContext, M_FILTER_TYPE, FilterType);
   MimControl(MilLinearFilterIIRContext, M_FILTER_SMOOTHNESS_TYPE, M_SIZE);
   MimControl(MilLinearFilterIIRContext, M_FILTER_SMOOTHNESS, 15);
   MimControl(MilLinearFilterIIRContext, M_FILTER_RESPONSE_TYPE, M_STEP);

   // Allocate convolution destination display image.
   MbufAlloc2d(MilSystem, ImageSizeX*3, ImageSizeY*2, 8 + M_SIGNED, M_IMAGE + M_PROC + M_DISP, &MilConvolveDispImage);

   // Allocate MimConvolve operation destinations.
   MbufAlloc2d(MilSystem, ImageSizeX, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE+M_PROC, &MilDstSmoothImage);
   MbufChild2d(MilConvolveDispImage, 0           , 0         , ImageSizeX, ImageSizeY, &MilTmpDstSmoothImage);
   MbufChild2d(MilConvolveDispImage, ImageSizeX  , 0         , ImageSizeX, ImageSizeY, &MilDstFDerXImage);
   MbufChild2d(MilConvolveDispImage, ImageSizeX*2, 0         , ImageSizeX, ImageSizeY, &MilDstFDerYImage);
   MbufChild2d(MilConvolveDispImage, 0           , ImageSizeY, ImageSizeX, ImageSizeY, &MilDstSDerXImage);
   MbufChild2d(MilConvolveDispImage, ImageSizeX  , ImageSizeY, ImageSizeX, ImageSizeY, &MilDstSDerYImage);
   MbufChild2d(MilConvolveDispImage, ImageSizeX*2, ImageSizeY, ImageSizeX, ImageSizeY, &MilDstSDerXYImage);

   // Execute MimConvolve for each operation.
   MimControl(MilLinearFilterIIRContext, M_FILTER_OPERATION, M_SMOOTH);
   MimConvolve(MilSrcImage, MilDstSmoothImage, MilLinearFilterIIRContext);
   // Adjustment for signed display buffer.
   MimArith(MilDstSmoothImage, 128, MilTmpDstSmoothImage, M_SUB_CONST);

   MimControl(MilLinearFilterIIRContext, M_FILTER_OPERATION, M_FIRST_DERIVATIVE_X);
   MimConvolve(MilSrcImage, MilDstFDerXImage, MilLinearFilterIIRContext);

   MimControl(MilLinearFilterIIRContext, M_FILTER_OPERATION, M_FIRST_DERIVATIVE_Y);
   MimConvolve(MilSrcImage, MilDstFDerYImage, MilLinearFilterIIRContext);

   MimControl(MilLinearFilterIIRContext, M_FILTER_OPERATION, M_SECOND_DERIVATIVE_X);
   MimConvolve(MilSrcImage, MilDstSDerXImage, MilLinearFilterIIRContext);

   MimControl(MilLinearFilterIIRContext, M_FILTER_OPERATION, M_SECOND_DERIVATIVE_Y);
   MimConvolve(MilSrcImage, MilDstSDerYImage, MilLinearFilterIIRContext);

   MimControl(MilLinearFilterIIRContext, M_FILTER_OPERATION, M_SECOND_DERIVATIVE_XY);
   MimConvolve(MilSrcImage, MilDstSDerXYImage, MilLinearFilterIIRContext);

   // Display MimConvolve destination image.
   MdispSelect(MilDstDisplay, MilConvolveDispImage);
   MdispInquire(MilDstDisplay, M_OVERLAY_ID, &MilDstOverlay);
   MgraText(M_DEFAULT, MilDstOverlay, 5             , 5           , MIL_TEXT("Smooth"));
   MgraText(M_DEFAULT, MilDstOverlay, ImageSizeX+5  , 5           , MIL_TEXT("First Derivative X"));
   MgraText(M_DEFAULT, MilDstOverlay, ImageSizeX*2+5, 5           , MIL_TEXT("First Derivative Y"));
   MgraText(M_DEFAULT, MilDstOverlay, 5             , ImageSizeY+5, MIL_TEXT("Second Derivative X"));
   MgraText(M_DEFAULT, MilDstOverlay, ImageSizeX+5  , ImageSizeY+5, MIL_TEXT("Second Derivative Y"));
   MgraText(M_DEFAULT, MilDstOverlay, ImageSizeX*2+5, ImageSizeY+5, MIL_TEXT("Second Derivative XY"));
   
   MosPrintf(MIL_TEXT("Display result of MimConvolve operations.\n"));
  
   // Wait for a key press.
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MdispSelect(MilDstDisplay, M_NULL);

   // Allocate MimDifferential destination display image.
   MbufAlloc2d(MilSystem, ImageSizeX*2, ImageSizeY*2, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDifferentialDispImage);

   // Allocate MimDifferential operation destinations.
   MbufChild2d(MilDifferentialDispImage, 0           , 0         , ImageSizeX, ImageSizeY, &MilDstGradientIntImage);
   MbufChild2d(MilDifferentialDispImage, ImageSizeX  , 0         , ImageSizeX, ImageSizeY, &MilDstGradientAngleImage);
   MbufChild2d(MilDifferentialDispImage, 0           , ImageSizeY, ImageSizeX, ImageSizeY, &MilTmpDstLaplacianImage);
   MbufCreate2d(MilSystem, M_DEFAULT, M_DEFAULT, 8 + M_SIGNED, M_IMAGE+M_PROC, M_MIL_ID, M_DEFAULT, MilTmpDstLaplacianImage, &MilDstLaplacianImage);
   MbufChild2d(MilDifferentialDispImage, ImageSizeX  , ImageSizeY, ImageSizeX, ImageSizeY, &MilDstSharpenImage);

   // Execute MimDifferential operations.
   MimDifferential(MilDstFDerXImage, MilDstFDerYImage, M_NULL, M_NULL, M_NULL, MilDstGradientIntImage, MilDstGradientAngleImage, M_DEFAULT, M_GRADIENT, M_DEFAULT);
   MimDifferential(MilDstSDerXImage, MilDstSDerYImage, M_NULL, M_NULL, M_NULL, MilDstLaplacianImage, M_NULL, M_DEFAULT, M_LAPLACIAN, M_DEFAULT);
   // Adjustement for unsigned display buffer.
   MimArith(MilDstLaplacianImage, 128, MilDstLaplacianImage, M_ADD_CONST);
   MIL_DOUBLE  DefaultSharpenParam;
   MimInquire(MilLinearFilterIIRContext, M_FILTER_DEFAULT_SHARPEN_PARAM, &DefaultSharpenParam);
   MimDifferential(MilDstSDerXImage, MilDstSDerYImage, MilDstSmoothImage, M_NULL, M_NULL, MilDstSharpenImage, M_NULL, DefaultSharpenParam, M_SHARPEN, M_DEFAULT);

   // Display destination image.
   MdispControl(MilDstDisplay, M_VIEW_MODE, M_DEFAULT);
   MdispControl(MilDstDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   MdispSelect(MilDstDisplay, MilDifferentialDispImage);
   MdispInquire(MilDstDisplay, M_OVERLAY_ID, &MilDstOverlay);
   MgraText(M_DEFAULT, MilDstOverlay, 5             , 5           , MIL_TEXT("Gradient intensity"));
   MgraText(M_DEFAULT, MilDstOverlay, ImageSizeX+5  , 5           , MIL_TEXT("Gradient angle"));
   MgraText(M_DEFAULT, MilDstOverlay, 5             , ImageSizeY+5, MIL_TEXT("Laplacian"));
   MgraText(M_DEFAULT, MilDstOverlay, ImageSizeX+5  , ImageSizeY+5, MIL_TEXT("Sharpen"));
   
   MosPrintf(MIL_TEXT("Display result of MimDifferential operations.\n\n"));
  
   // Wait for a key press.
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   MbufFree(MilDstSharpenImage);
   MbufFree(MilDstLaplacianImage);
   MbufFree(MilTmpDstLaplacianImage);
   MbufFree(MilDstGradientAngleImage);
   MbufFree(MilDstGradientIntImage);
   MbufFree(MilDifferentialDispImage);

   MbufFree(MilDstSDerXYImage);
   MbufFree(MilDstSDerYImage);
   MbufFree(MilDstSDerXImage);
   MbufFree(MilDstFDerYImage);
   MbufFree(MilDstFDerXImage);
   MbufFree(MilTmpDstSmoothImage);
   MbufFree(MilDstSmoothImage);
   MbufFree(MilConvolveDispImage);

   MdispFree(MilDstDisplay);
   MdispFree(MilSrcDisplay);
   MimFree(MilLinearFilterIIRContext);
   MbufFree(MilSrcImage);

   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
}
