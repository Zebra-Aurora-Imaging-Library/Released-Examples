/************************************************************************************/
/*
 * File name: HistogramEqualizeAdaptive.cpp 
 *
 * Synopsis:  This program demonstrates how to use MimHistogramEqualizeAdaptive to
 * enhance a source image using a contrast limited adaptive histogram equalization
 * operation.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

//*****************************************************************************
// Example description.
//*****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("HistogramEqualizeAdaptive\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program demonstrates how to use MimHistogramEqualizeAdaptive\n")
             MIL_TEXT(" to enhance a source image using a contrast limited adaptive\n")
             MIL_TEXT(" histogram equalization operation.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

/* Subfunction header */
void Annotate(MIL_ID MilGraContext, MIL_ID MilOverlayImage);

/* Source images file name. */
#define EXAMPLE_IMAGE_PATH M_IMAGE_PATH MIL_TEXT("HistogramEqualizeAdaptive/")
static MIL_CONST_TEXT_PTR IMAGE_FILE   = EXAMPLE_IMAGE_PATH MIL_TEXT("ArmsMono8bit.mim");

int MosMain(void)
{
   /* Print Header. */
   PrintHeader();

   /* Allocate the MIL objects. */
   MIL_ID   MilApplication,               /* Application identifier.         */
            MilSystem,                    /* System identifier.              */
            MilDisplay,                   /* Display identifier.             */
            MilOverlayImage,              /* Display overlay identifier      */
            MilSourceImage,               /* Image buffer identifier.        */
            MilDisplayImage,              /* Image buffer identifier.        */
            MilDispChildImage0,           /* Image buffer identifier.        */
            MilDispChildImage1,           /* Image buffer identifier.        */
            MilHistogramEqualizeAdaptiveContext,	/* HistogramEqualizeAdaptive context identifier. */
            MilGraContext;                /* Graphic context identifier.     */

   MIL_INT  ImageSizeBand,
            ImageWidth,
            ImageHeight, 
            ImageType;

   /* Allocate defaults. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Allocate graphic context */
   MgraAlloc(MilSystem, &MilGraContext);

   /* Restore the source image. */
   MbufRestore(IMAGE_FILE, MilSystem, &MilSourceImage);   

   /* Allocate display buffer. */
   MbufInquire(MilSourceImage, M_SIZE_BAND, &ImageSizeBand);
   MbufInquire(MilSourceImage, M_SIZE_X   , &ImageWidth);
   MbufInquire(MilSourceImage, M_SIZE_Y   , &ImageHeight);
   MbufInquire(MilSourceImage, M_TYPE     , &ImageType);
   MbufAllocColor(MilSystem, 
                  ImageSizeBand,
                  ImageWidth * 2,
                  ImageHeight,
                  ImageType,
                  M_IMAGE+M_PROC+M_DISP,
                  &MilDisplayImage);
   MbufClear(MilDisplayImage, 0);

   /* Allocate 2 child buffers to display the source and MimHistogramEqualizeAdaptive result. */
   MbufChild2d(MilDisplayImage, 0, 0, ImageWidth, ImageHeight, &MilDispChildImage0);
   MbufChild2d(MilDisplayImage, ImageWidth, 0, ImageWidth, ImageHeight, &MilDispChildImage1);

   /* Display source image. */
   MbufCopy(MilSourceImage, MilDispChildImage0);
   MdispSelect(MilDisplay, MilDisplayImage);

   /* Prepare display for overlay annotations. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);

   /* Allocate a histogram equalize adaptive context. */
   MimAlloc(MilSystem, M_HISTOGRAM_EQUALIZE_ADAPTIVE_CONTEXT, M_DEFAULT, &MilHistogramEqualizeAdaptiveContext);

   /* Adaptive equalize with default values, M_UNIFORM, clip 1%, 8x8 tiles */
   MimHistogramEqualizeAdaptive(MilHistogramEqualizeAdaptiveContext, MilSourceImage, MilDispChildImage1, M_DEFAULT);

   /* Annotation */
   Annotate(MilGraContext, MilOverlayImage);

   /* Display message. */
   MosPrintf(MIL_TEXT("Press <Enter> to terminate.\n\n"));
   MosGetch();

   /* Free objects. */
   MbufFree(MilSourceImage);
   MbufFree(MilDispChildImage0);
   MbufFree(MilDispChildImage1);
   MbufFree(MilDisplayImage);
   MgraFree(MilGraContext);
   MdispFree(MilDisplay);
   MimFree(MilHistogramEqualizeAdaptiveContext);

   /* Free objects. */
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
}

void Annotate(MIL_ID MilGraContext, MIL_ID MilOverlayImage)
   {
   MIL_INT  OverlaySizeX;
   MIL_INT  OverlaySizeY;

   /* Extract overlay buffer information */
   MbufInquire(MilOverlayImage, M_SIZE_X, &OverlaySizeX);
   MbufInquire(MilOverlayImage, M_SIZE_Y, &OverlaySizeY);

   /* Set overlay color */
   MgraColor(MilGraContext, M_COLOR_GREEN);

   /* Annotation */
   MgraText(MilGraContext, MilOverlayImage, OverlaySizeX / 4 - 48, OverlaySizeY - 24, MIL_TEXT("Source image"));
   MgraText(MilGraContext, MilOverlayImage, OverlaySizeX * 3 / 4 - 64, OverlaySizeY - 24, MIL_TEXT("Destination image"));
   }
