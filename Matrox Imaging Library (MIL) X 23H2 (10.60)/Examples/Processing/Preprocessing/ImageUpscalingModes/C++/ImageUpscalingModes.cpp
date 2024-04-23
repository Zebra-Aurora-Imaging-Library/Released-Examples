﻿/************************************************************************************/
/*
* File name: UpscalingInterpolation.cpp
*
* Synopsis:  This program demonstrates the resize operation to increase the size of 
*            an image using various interpolation modes.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/


#include <mil.h>


/* Example functions prototypes. */
void Upsample(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR ImageFile);

/* Source images file name. */
#define MIL_IMAGE_TEXT                     M_IMAGE_PATH  MIL_TEXT("Preprocessing/PrintedText.mim")


/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("Image Upscaling Modes\n\n")
      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program demonstrates how the resize operation increases\n") 
      MIL_TEXT("the size of an image using various interpolation modes.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n") 
      MIL_TEXT("              graphics, image processing.\n\n"));

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
   Upsample(MilSystem, MilDisplay, MIL_IMAGE_TEXT);
   
   /* Free objects. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

/*****************************************************************************/
/*       Subsamples an image using different interpolation algorithms.       */
void Upsample(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR ImageFile)
   {
   MIL_ID MilImage,         /* Image buffer identifier.         */
      MilOriginalImage,     /* Child buffer identifier.         */
      MilZoneOfInterest,    /* Child buffer identifier.         */
      MilSubImage0,         /* Child buffer identifier.         */
      MilSubImage1,         /* Child buffer identifier.         */
      MilSubImage2;         /* Child buffer identifier.         */
   
   MIL_INT Type, SizeX, SizeY;

   /* Print a message. */
   MosPrintf(MIL_TEXT("An image is loaded and resized using various modes.\n"));

   /* Inquire the image Size and Type. */
   MbufDiskInquire(ImageFile, M_SIZE_X, &SizeX);
   MbufDiskInquire(ImageFile, M_SIZE_Y, &SizeY);
   MbufDiskInquire(ImageFile, M_TYPE, &Type);

   /* Set size and offset of the zone of interest. */
   MIL_INT ZOI_SizeX = SizeX / 16;
   MIL_INT ZOI_SizeY = SizeY / 16;
   MIL_INT ZOI_OffsetX = 185;
   MIL_INT ZOI_OffsetY = 90;

   /* Deduce buffers size. */
   MIL_DOUBLE DisplayScaleFactor = 1 / 3.0;
   MIL_INT ReducedSizeX = (MIL_INT) ((MIL_DOUBLE)SizeX * DisplayScaleFactor);
   MIL_INT ReducedSizeY = (MIL_INT) ((MIL_DOUBLE)SizeY * DisplayScaleFactor);

   MIL_INT CanvasSizeY = (SizeY >= 3 * ReducedSizeY) ? SizeY : 3 * ReducedSizeY;
   MIL_INT CanvasSizeX = SizeX + ReducedSizeX;

   /* Allocate image buffer. */
   MbufAlloc2d(MilSystem, CanvasSizeX, CanvasSizeY, Type, M_IMAGE | M_DISP | M_PROC, &MilImage);

   /* Allocate child buffers. */
   MbufChild2d(MilImage, 0, 0, SizeX, SizeY, &MilOriginalImage);
   MbufChild2d(MilOriginalImage, ZOI_OffsetX, ZOI_OffsetY, ZOI_SizeX, ZOI_SizeY, &MilZoneOfInterest);
   MbufChild2d(MilImage, SizeX, 0, ReducedSizeX, ReducedSizeY, &MilSubImage0);
   MbufChild2d(MilImage, SizeX, ReducedSizeY, ReducedSizeX, ReducedSizeY, &MilSubImage1);
   MbufChild2d(MilImage, SizeX, 2 * ReducedSizeY, ReducedSizeX, SizeY - (2 * ReducedSizeY), &MilSubImage2);

   /* Load an image. */
   MbufLoad(ImageFile, MilOriginalImage);

   /* Perform upsampling. */
   MimResize(MilZoneOfInterest, MilSubImage0, M_FILL_DESTINATION, M_FILL_DESTINATION, M_NEAREST_NEIGHBOR);
   MimResize(MilZoneOfInterest, MilSubImage1, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BILINEAR);
   MimResize(MilZoneOfInterest, MilSubImage2, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BICUBIC);

   /* Draw the annotation indicating the zone of interest. */
   MgraColor(M_DEFAULT, 255);

   MgraRect(M_DEFAULT,
      MilOriginalImage,
      ZOI_OffsetX,
      ZOI_OffsetY,
      ZOI_OffsetX + ZOI_SizeX,
      ZOI_OffsetY + ZOI_SizeY);

   /* Display the image buffer. */
   MdispSelect(MilDisplay, MilImage);

   /* Annotate the images. */
   MgraText(M_DEFAULT, MilOriginalImage, 0, 0, MIL_TEXT("Source image"));
   MgraText(M_DEFAULT, MilSubImage0,     0, 0, MIL_TEXT("Nearest Neighbor"));
   MgraText(M_DEFAULT, MilSubImage1,     0, 0, MIL_TEXT("Bilinear"));
   MgraText(M_DEFAULT, MilSubImage2,     0, 0, MIL_TEXT("Bicubic"));
   
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free the buffers. */
   MbufFree(MilZoneOfInterest);
   MbufFree(MilSubImage2);
   MbufFree(MilSubImage1);
   MbufFree(MilSubImage0);
   MbufFree(MilOriginalImage);
   MbufFree(MilImage);

   }
