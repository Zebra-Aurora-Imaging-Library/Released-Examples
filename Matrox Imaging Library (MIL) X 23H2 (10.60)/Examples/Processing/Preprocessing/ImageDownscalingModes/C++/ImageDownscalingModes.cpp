/************************************************************************************/
/*
* File name: ImageDownscalingModes.cpp
*
* Synopsis:  This program demonstrates the resize operation to reduce
*            the size of an image using various interpolation modes.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/


#include <mil.h>


/* Example functions prototypes. */
void Downscale(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR ImageFile);

/* Source images file name. */
#define MIL_IMAGE_WAFER                    M_IMAGE_PATH MIL_TEXT("Wafer.mim")
#define MIL_IMAGE_OCR                      M_IMAGE_PATH MIL_TEXT("OcrSemi1292.mim")


/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("Image Downscaling Modes\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program demonstrates how the resize operation reduces\n")
      MIL_TEXT("the size of an image using various interpolation modes.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n")
      MIL_TEXT("              graphics, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }


int MosMain()
   {
   PrintHeader();

   MIL_ID MilApplication,    /* Application identifier.    */
          MilSystem,         /* System identifier.         */
          MilDisplay;        /* Display identifier.        */

   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Laod, resize, and display images. */
   Downscale(MilSystem, MilDisplay, MIL_IMAGE_WAFER);

   Downscale(MilSystem, MilDisplay, MIL_IMAGE_OCR);
   
   /* Free objects. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

/*****************************************************************************/
/*       Downscales an image using different interpolation algorithms.       */
void Downscale(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR ImageFile)
   {
   MIL_ID MilImage,           /* Image buffer identifier.         */
          MilOriginalImage,   /* Child buffer identifier.         */
          MilSubImage00,      /* Child buffer identifier.         */
          MilSubImage01,      /* Child buffer identifier.         */
          MilSubImage10,      /* Child buffer identifier.         */
          MilSubImage11,      /* Child buffer identifier.         */
          MilSubImage02,      /* Child buffer identifier.         */
          MilSubImage12;      /* Child buffer identifier.         */

   MIL_INT Type, SizeX, SizeY;

   MosPrintf(MIL_TEXT("A new image is loaded and resized using various modes.\n"));

   /* Inquire the image Size and Type. */
   MbufDiskInquire(ImageFile, M_SIZE_X, &SizeX);
   MbufDiskInquire(ImageFile, M_SIZE_Y, &SizeY);
   MbufDiskInquire(ImageFile, M_TYPE,   &Type);
   
   /* Deduce buffers size. */
   MIL_DOUBLE DisplayScaleFactor = 1 / 3.0;
   MIL_INT ReducedSizeX = (MIL_INT) ((MIL_DOUBLE) SizeX * DisplayScaleFactor);
   MIL_INT ReducedSizeY = (MIL_INT) ((MIL_DOUBLE) SizeY * DisplayScaleFactor);

   MIL_INT CanvasSizeY = (SizeY >=  3 * ReducedSizeY) ? SizeY : 3 * ReducedSizeY;
   MIL_INT CanvasSizeX = SizeX + 2 * ReducedSizeX;

   /* Allocate image buffer. */
   MbufAlloc2d(MilSystem, CanvasSizeX, CanvasSizeY, Type, M_IMAGE | M_DISP | M_PROC, &MilImage);

   /* Allocate child buffers. */
   MbufChild2d(MilImage, 0, 0, SizeX, SizeY, &MilOriginalImage);
   MbufChild2d(MilImage, SizeX, 0, ReducedSizeX, ReducedSizeY, &MilSubImage00);
   MbufChild2d(MilImage, SizeX + ReducedSizeX, 0, ReducedSizeX, ReducedSizeY, &MilSubImage01);
   MbufChild2d(MilImage, SizeX, ReducedSizeY, ReducedSizeX, ReducedSizeY, &MilSubImage10);
   MbufChild2d(MilImage, SizeX + ReducedSizeX, ReducedSizeY, ReducedSizeX, ReducedSizeY, &MilSubImage11);
   MbufChild2d(MilImage, SizeX, 2 * ReducedSizeY, ReducedSizeX, ReducedSizeY, &MilSubImage02);
   MbufChild2d(MilImage, SizeX + ReducedSizeX, 2 * ReducedSizeY, ReducedSizeX, ReducedSizeY, &MilSubImage12);

   /* Load an image. */
   MbufLoad(ImageFile, MilOriginalImage);

   /* Perform subsampling. */
   MimResize(MilOriginalImage, MilSubImage00, M_FILL_DESTINATION, M_FILL_DESTINATION, M_NEAREST_NEIGHBOR);
   MimResize(MilOriginalImage, MilSubImage01, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BILINEAR);
   MimResize(MilOriginalImage, MilSubImage10, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BICUBIC);
   MimResize(MilOriginalImage, MilSubImage11, M_FILL_DESTINATION, M_FILL_DESTINATION, M_AVERAGE);
   MimResize(MilOriginalImage, MilSubImage02, M_FILL_DESTINATION, M_FILL_DESTINATION, M_MIN);
   MimResize(MilOriginalImage, MilSubImage12, M_FILL_DESTINATION, M_FILL_DESTINATION, M_MAX);

   /* Display the image buffer. */
   MdispSelect(MilDisplay, MilImage);

   /* Annotate the images. */
   MgraColor(M_DEFAULT, 255);

   MgraText(M_DEFAULT, MilOriginalImage, 0, 0, MIL_TEXT("Source image"));
   MgraText(M_DEFAULT, MilSubImage00,    0, 0, MIL_TEXT("Nearest Neighbor"));
   MgraText(M_DEFAULT, MilSubImage01,    0, 0, MIL_TEXT("Bilinear"));
   MgraText(M_DEFAULT, MilSubImage10,    0, 0, MIL_TEXT("Bicubic"));
   MgraText(M_DEFAULT, MilSubImage11,    0, 0, MIL_TEXT("Average"));
   MgraText(M_DEFAULT, MilSubImage02,    0, 0, MIL_TEXT("Min"));
   MgraText(M_DEFAULT, MilSubImage12,    0, 0, MIL_TEXT("Max"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free the buffers. */
   MbufFree(MilSubImage12);
   MbufFree(MilSubImage02);
   MbufFree(MilSubImage11);
   MbufFree(MilSubImage10);
   MbufFree(MilSubImage01);
   MbufFree(MilSubImage00);
   MbufFree(MilOriginalImage);
   MbufFree(MilImage);
   }
