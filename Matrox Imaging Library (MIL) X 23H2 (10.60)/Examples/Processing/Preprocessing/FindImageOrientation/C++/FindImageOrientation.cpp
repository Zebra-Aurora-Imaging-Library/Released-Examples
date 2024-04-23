﻿/************************************************************************************/
/*
 * File name: FindImageOrientation.cpp 
 *
 * Synopsis:  This program demonstrates how to find and correct the orientation of the image for various user cases.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#include <math.h>

/* Example functions prototypes. */
void FindMultipleOrientations(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename);
void AlignImage(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename);

/* Source images file name. */
#define IMAGE_SHOE_SOLE            M_IMAGE_PATH  MIL_TEXT("Preprocessing/ShoeSole.mim")
#define IMAGE_TEXT                 M_IMAGE_PATH  MIL_TEXT("Preprocessing/PrintedText.mim")
#define IMAGE_CELL                 M_IMAGE_PATH  MIL_TEXT("Cell.mbufi")
#define IMAGE_PCB                  M_IMAGE_PATH  MIL_TEXT("PCBModelMatching/PCBrotTarget.mim")

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("FindImageOrientation\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program demonstrates how to find and correct\n")
      MIL_TEXT("the orientation of the image for various user cases.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n")
      MIL_TEXT("graphic, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

int MosMain(void)
   {
   PrintHeader();

   MIL_ID MilApplication,    /* Application identifier.                  */
          MilSystem,         /* System identifier.                       */
          MilDisplay;        /* Display identifier.                      */
          
   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Set display properties. */
   MdispControl(MilDisplay, M_OVERLAY,       M_ENABLE);
   MdispControl(MilDisplay, M_SCALE_DISPLAY, M_ENABLE);

   /* Run the search of multiple orientations example. */
   FindMultipleOrientations(MilSystem, MilDisplay, IMAGE_SHOE_SOLE);

   /* Run the align text example. */
   AlignImage(MilSystem, MilDisplay, IMAGE_TEXT);

   /* Run the align irregular object example. */
   AlignImage(MilSystem, MilDisplay, IMAGE_CELL);

   /* Run the align PCB example. */
   AlignImage(MilSystem, MilDisplay, IMAGE_PCB);

   /* Free defaults. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
}


/*****************************************************************************/
/*            Find three principal orientations of an image.               */

#define NB_ORIENTATIONS 3

void FindMultipleOrientations(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename)
   {
   MIL_ID MilImage,          /* Image buffer identifier.                 */
          MilGraphicList,    /* Graphic list identifier.                 */
          MilOverlayImage,   /* Overlay image buffer identifier.         */
          MilResultId;       /* Result buffer identifier.                */

   MIL_INT   Type,SizeX, SizeY,OverlayClearColor;
   MIL_FLOAT Orientations[NB_ORIENTATIONS];  /*  Orientations  */
   MIL_FLOAT Score[NB_ORIENTATIONS];         /*  Scores        */

   /* Inquire the image Size and Type. */
   MbufDiskInquire(Filename, M_SIZE_X, &SizeX);
   MbufDiskInquire(Filename, M_SIZE_Y, &SizeY);
   MbufDiskInquire(Filename, M_TYPE,   &Type);

   /* Allocate a display buffer and clear it. */
   MbufAlloc2d(MilSystem, SizeX, SizeY,
      Type, M_IMAGE+M_PROC+M_DISP, &MilImage);
   MbufClear(MilImage, 0L);

   /* Allocate the graphic list and associate it with the display. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   /* Display the image buffer and prepare for overlay annotations. */
   MdispSelect (MilDisplay, MilImage);
   MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, &OverlayClearColor);
   MdispInquire(MilDisplay, M_OVERLAY_ID,        &MilOverlayImage);
   MbufClear(MilOverlayImage, (MIL_DOUBLE)OverlayClearColor);

   /*Allocate the Result buffer */
   MimAllocResult(MilSystem, NB_ORIENTATIONS, M_FIND_ORIENTATION_LIST + M_FLOAT, &MilResultId);

   /* Load a noisy image. */
   MbufLoad(Filename, MilImage);

   /* Find the main orientation of the image. */
   MimFindOrientation(M_DEFAULT, MilImage, MilResultId, M_DEFAULT);

   MimGetResult1d(MilResultId, 0, NB_ORIENTATIONS, M_ANGLE, Orientations);
   MimGetResult1d(MilResultId, 0, NB_ORIENTATIONS, M_SCORE, Score);

   /* Draw the orientations on the overlay. */
   MgraControl(M_DEFAULT, M_LINE_THICKNESS, 3);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MimDraw(M_DEFAULT, MilResultId, M_NULL, MilGraphicList, M_DRAW_IMAGE_ORIENTATION, 0, 1, M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MimDraw(M_DEFAULT, MilResultId, M_NULL, MilGraphicList, M_DRAW_IMAGE_ORIENTATION, 1, 1, M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   MimDraw(M_DEFAULT, MilResultId, M_NULL, MilGraphicList, M_DRAW_IMAGE_ORIENTATION, 2, 1, M_DEFAULT);

   /* Print a message. */
   MosPrintf(MIL_TEXT("3 principal orientations of the image are calculated and displayed\n\n")
      MIL_TEXT(" \tOrientation\tScore\n\n")
      MIL_TEXT("Green: \t %5.1f degrees\t%5.1f%%\nYellow:  %5.1f degrees\t%5.1f%% \nRed:  \t %5.1f degrees\t%5.1f%%\n"),
      Orientations[0L], Score[0L], Orientations[1L], Score[1L], Orientations[2L], Score[2L]);
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free buffers. */
   MbufFree(MilImage);
   MgraFree(MilGraphicList);
   MimFree(MilResultId);

   }

/*********************************************************************************************************/
/*  Find the main orientation of the image and rotate the image appropriately to correct its alignment. */

void AlignImage(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename)
   {
   MIL_ID MilImage,           /* Image buffer identifier.                 */
          MilOverlayImage,    /* Overlay image buffer identifier.         */
          MilSubImage00,      /* Child buffer identifier.                 */
          MilSubImage01,      /* Child buffer identifier.                 */
          MilOverlaySubImage, /* Child buffer identifier.                 */
          MilWarpMatrix,      /* Warp matrix identifier.                  */
          MilResultId;        /* Result buffer identifier.                */

   MIL_INT Type, SizeX, SizeY, OverlayClearColor;

   MIL_FLOAT Orientation = 0.0, CorrectedOrientation = 0.0; /* Orientation of the image */

   /* Inquire the image Size and Type. */
   MbufDiskInquire(Filename, M_SIZE_X, &SizeX);
   MbufDiskInquire(Filename, M_SIZE_Y, &SizeY);
   MbufDiskInquire(Filename, M_TYPE,   &Type);

   /* Allocate a display buffer and clear it. */
   MbufAlloc2d(MilSystem, 2*SizeX, SizeY, Type, M_IMAGE+M_PROC+M_DISP, &MilImage);
   MbufClear(MilImage, 0L);

   /* Display the image buffer and prepare for overlay annotations. */
   MdispSelect (MilDisplay,   MilImage);
   MdispInquire(MilDisplay,   M_TRANSPARENT_COLOR, &OverlayClearColor);
   MdispInquire(MilDisplay,   M_OVERLAY_ID,        &MilOverlayImage);
   MbufClear(MilOverlayImage, (MIL_DOUBLE)OverlayClearColor);

   /* Allocate child buffers in the 4 quadrants of the display image. */
   MbufChild2d(MilImage,        0L,    0L, SizeX, SizeY, &MilSubImage00);
   MbufChild2d(MilImage,        SizeX, 0L, SizeX, SizeY, &MilSubImage01);
   MbufChild2d(MilOverlayImage, 0L,    0L, SizeX, SizeY, &MilOverlaySubImage);

   /*Allocate the Result buffer. */
   MimAllocResult(MilSystem, 1L, M_FIND_ORIENTATION_LIST + M_FLOAT, &MilResultId);

   /* Allocate the warp matrix. */
   MbufAlloc2d(MilSystem, 3L, 3L, M_FLOAT+32, M_ARRAY, &MilWarpMatrix);

   /* Load a noisy image. */
   MbufLoad(Filename,     MilSubImage00);
   MbufClear(MilSubImage01, M_COLOR_WHITE);

   /* Find the main orientation of the image. */
   MimFindOrientation(M_DEFAULT, MilSubImage00, MilResultId, M_DEFAULT);

   /* Get the result from the result buffer. */
   MimGetResult1d(MilResultId, 0L, 1L, M_ANGLE, &Orientation);

   /* Evaluate shortest rotation to horizontal alignment. */
   CorrectedOrientation = (Orientation < 180 - Orientation)?-Orientation:180-Orientation;

   /* Generate the warp matrix */
   MgenWarpParameter(M_NULL,        MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_TRANSLATE, (MIL_DOUBLE)(-SizeX / 2), (MIL_DOUBLE)(-SizeX / 2));
   MgenWarpParameter(MilWarpMatrix, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_ROTATE,    CorrectedOrientation, M_NULL);
   MgenWarpParameter(MilWarpMatrix, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_TRANSLATE, (MIL_DOUBLE)(SizeX / 2), (MIL_DOUBLE)(SizeX / 2));
   
   /* Warp the image to correct the orientation. */
   MimWarp(MilSubImage00, MilSubImage01, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_BICUBIC + M_OVERSCAN_DISABLE);

   /* Draw the orientations on the overlay. */
   MgraControl(M_DEFAULT, M_LINE_THICKNESS, 3);
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MimDraw(M_DEFAULT, MilResultId, M_NULL, MilOverlaySubImage, M_DRAW_IMAGE_ORIENTATION, 0, 1, M_DEFAULT);

   /* Identify images. */
   MgraText(M_DEFAULT, MilSubImage00, 0, 0, MIL_TEXT("Source image"));
   MgraText(M_DEFAULT, MilSubImage01, 0, 0, MIL_TEXT("Aligned image"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("The image is aligned according to its main orientation (%.1f degrees)\n"), Orientation);
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free buffers. */
   MbufFree(MilOverlaySubImage);
   MbufFree(MilSubImage00);
   MbufFree(MilSubImage01);
   MbufFree(MilImage);
   MbufFree(MilWarpMatrix);
   MimFree(MilResultId);
   }
