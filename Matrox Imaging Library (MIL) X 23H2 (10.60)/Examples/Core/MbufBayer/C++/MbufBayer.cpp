﻿/*****************************************************************************/
/*
 * File name: MbufBayer.cpp
 *
 * Synopsis:  This program converts a single-band, Bayer color-encoded
 *            image into a 3-band image.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
 
#include <mil.h>

/* Target MIL image. */
#define IMAGE_FILE                     M_IMAGE_PATH MIL_TEXT("ColorChartBayerRaw.mim")

/* Position of the white patch ROI in the image. */
#define WHITE_POSITION_X              195
#define WHITE_POSITION_Y              280
#define WHITE_SIZE_X                   45
#define WHITE_SIZE_Y                   45

/* Display information. */
#define DISPLAY_SPACING_X              10  /* Spacing in x between display windows. */
#define DISPLAY_SPACING_Y              30  /* Spacing in y between display windows. */
#define DISPLAY_CHILD_OFFSET_X        0    /* X offset of the displayed area.       */
#define DISPLAY_CHILD_OFFSET_Y        145  /* Y offset of the displayed area.       */
#define DISPLAY_ZOOM_FACTOR           1.0  /* Display zoom factor.                  */

/* Display title names. */
#define DISPLAY_TITLE_0                MIL_TEXT("Original Bayer raw")
#define DISPLAY_TITLE_1                MIL_TEXT("Average 2x2 demosaicing")
#define DISPLAY_TITLE_2                MIL_TEXT("Bilinear demosaicing")
#define DISPLAY_TITLE_3                MIL_TEXT("Adaptive fast demosaicing")
#define DISPLAY_TITLE_4                MIL_TEXT("Adaptive demosaicing")

/* Utility function declaration. */
void DisplayBayerToRGB(MIL_ID       SourceId, 
                       MIL_ID       ChildId, 
                       MIL_INT      ChildPosX,
                       MIL_INT      ChildPosY,
                       MIL_INT      ChildSizeX,
                       MIL_INT      ChildSizeY,
                       MIL_ID       DisplayId, 
                       MIL_CONST_TEXT_PTR DisplayTitle, 
                       MIL_INT      DisplayPosX, 
                       MIL_INT      DisplayPosY);

int MosMain(void)
   {
   MIL_ID    MilApplication,           /* Application identifier.             */
             MilSystem,                /* System identifier.                  */
             MilDisplayRaw,            /* Display RAW bayer image.            */
             MilOverlay,               /* Display overlay identifier.         */
             MilDisplay1,              /* Display identifier.                 */
             MilDisplay2,              /* Display identifier.                 */
             MilDisplay3,              /* Display identifier.                 */
             MilDisplay4,              /* Display identifier.                 */
             MilDisplayBuffer1,        /* Buffer identifier.                  */
             MilDisplayBuffer2,        /* Buffer identifier.                  */
             MilDisplayBuffer3,        /* Buffer identifier.                  */
             MilDisplayBuffer4,        /* Buffer identifier.                  */
             MilCoefWB,                /* White balancing correction coef.    */
             MilBayerRaw,              /* Bayer raw source image.             */
             MilChildBayerRaw,         /* White patch child buffer.           */
             MilDestination;           /* Bayer to RGB destination buffer.    */
             

   MIL_INT   RawSizeX,
             RawSizeY,
             ChildSizeX,
             ChildSizeY,
             DisplaySizeY;

   MosPrintf(MIL_TEXT("\nBAYER TO RGB CONVERSION:\n"));
   MosPrintf(MIL_TEXT("------------------------\n\n"));
   MosPrintf(MIL_TEXT("This program converts a single-band Bayer color-encoded image into ")
             MIL_TEXT("a 3-band\nimage using four different demosaicing algorithms.\n\n"));

   MosPrintf(MIL_TEXT("The white region, which is displayed in red, is used\n")
             MIL_TEXT("to perform the white balancing.\n\n"));

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, M_NULL, M_NULL);

   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplayRaw);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay1);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay2);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay3);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay4);

   /* Restore the source image (Bayer). */
   MbufRestore(IMAGE_FILE, MilSystem, &MilBayerRaw);

   /* Allocate the white patch child buffer. */
   MbufChild2d(MilBayerRaw, WHITE_POSITION_X, WHITE_POSITION_Y, WHITE_SIZE_X, WHITE_SIZE_Y, &MilChildBayerRaw);

   /* Allocate the white balance coefficients buffer. */
   MbufAlloc1d(MilSystem, 3, 32 + M_FLOAT, M_ARRAY, &MilCoefWB);

   /* Initialize the white balance coefficients. */
   MbufBayer(MilChildBayerRaw, M_NULL, MilCoefWB, M_BAYER_GB + M_WHITE_BALANCE_CALCULATE);

   /* Destination buffer allocation. */
   MbufInquire(MilBayerRaw, M_SIZE_X, &RawSizeX);
   MbufInquire(MilBayerRaw, M_SIZE_Y, &RawSizeY);
   MbufAllocColor(MilSystem, 3, RawSizeX, RawSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDestination);

   /* Calculate the RGB display buffers size Y. */
   DisplaySizeY = (RawSizeY - (3 * DISPLAY_SPACING_Y)) / 4;

   /* Display buffer allocations. */
   ChildSizeX = (MIL_INT) (RawSizeX / DISPLAY_ZOOM_FACTOR);
   ChildSizeY = (MIL_INT) (DisplaySizeY / DISPLAY_ZOOM_FACTOR);
   MbufAllocColor(MilSystem, 3, ChildSizeX, ChildSizeY,
      8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDisplayBuffer1);

   MbufAllocColor(MilSystem, 3, ChildSizeX, ChildSizeY,
      8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDisplayBuffer2);

   MbufAllocColor(MilSystem, 3, ChildSizeX, ChildSizeY,
      8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDisplayBuffer3);

   MbufAllocColor(MilSystem, 3, ChildSizeX, ChildSizeY,
      8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDisplayBuffer4);

   /* Display the source image (Bayer). */   
   MdispControl(MilDisplayRaw, M_OVERLAY, M_ENABLE);
   MdispControl(MilDisplayRaw, M_TITLE, DISPLAY_TITLE_0);
   MdispSelect(MilDisplayRaw, MilBayerRaw);
   MdispInquire(MilDisplayRaw, M_OVERLAY_ID, &MilOverlay);

   /* Draw the white patch ROI in red. */
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MgraRect(M_DEFAULT, MilOverlay, WHITE_POSITION_X, WHITE_POSITION_Y, WHITE_POSITION_X+WHITE_SIZE_X, WHITE_POSITION_Y+WHITE_SIZE_Y);

   /* Draw the displayed result ROI in green.*/
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraRect(M_DEFAULT, MilOverlay, DISPLAY_CHILD_OFFSET_X, DISPLAY_CHILD_OFFSET_Y, DISPLAY_CHILD_OFFSET_X + ChildSizeX, DISPLAY_CHILD_OFFSET_Y + ChildSizeY);

   /* Calculate then display the Bayer to RGB conversion using average 2x2 method. */
   MbufBayer(MilBayerRaw, MilDestination, MilCoefWB, M_BAYER_GB + M_AVERAGE_2X2);

   /* Display the result. */
   DisplayBayerToRGB(MilDestination, MilDisplayBuffer1, DISPLAY_CHILD_OFFSET_X, DISPLAY_CHILD_OFFSET_Y, 
      ChildSizeX, ChildSizeY, MilDisplay1, DISPLAY_TITLE_1, RawSizeX + DISPLAY_SPACING_X, 0);

   /* Calculate then display the Bayer to RGB conversion using bilinear method. */
   MbufBayer(MilBayerRaw, MilDestination, MilCoefWB, M_BAYER_GB);
   
   /* Display the result. */
   DisplayBayerToRGB(MilDestination, MilDisplayBuffer2, DISPLAY_CHILD_OFFSET_X, DISPLAY_CHILD_OFFSET_Y, 
      ChildSizeX, ChildSizeY, MilDisplay2, DISPLAY_TITLE_2, RawSizeX + DISPLAY_SPACING_X, DisplaySizeY + DISPLAY_SPACING_Y);

   /* Calculate then display the Bayer to RGB conversion using adaptive method. */
   MbufBayer(MilBayerRaw, MilDestination, MilCoefWB, M_BAYER_GB + M_ADAPTIVE_FAST);

   /* Display the result. */
   DisplayBayerToRGB(MilDestination, MilDisplayBuffer3, DISPLAY_CHILD_OFFSET_X, DISPLAY_CHILD_OFFSET_Y, 
      ChildSizeX, ChildSizeY, MilDisplay3, DISPLAY_TITLE_3, RawSizeX + DISPLAY_SPACING_X, 2 * (DisplaySizeY + DISPLAY_SPACING_Y));

   /* Calculate then display the Bayer to RGB conversion using adaptive method. */
   MbufBayer(MilBayerRaw, MilDestination, MilCoefWB, M_BAYER_GB + M_ADAPTIVE + M_COLOR_CORRECTION);

   /* Display the result. */
   DisplayBayerToRGB(MilDestination, MilDisplayBuffer4, DISPLAY_CHILD_OFFSET_X, DISPLAY_CHILD_OFFSET_Y, 
      ChildSizeX, ChildSizeY, MilDisplay4, DISPLAY_TITLE_4, RawSizeX + DISPLAY_SPACING_X, 3 * (DisplaySizeY + DISPLAY_SPACING_Y));

   /* Print a message. */
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   /* Free all allocations. */
   MbufFree(MilDisplayBuffer1);
   MbufFree(MilDisplayBuffer2);
   MbufFree(MilDisplayBuffer3);
   MbufFree(MilDisplayBuffer4);
   MbufFree(MilCoefWB);
   MbufFree(MilChildBayerRaw);
   MbufFree(MilBayerRaw);
   MbufFree(MilDestination);
   MdispFree(MilDisplayRaw);
   MdispFree(MilDisplay1);
   MdispFree(MilDisplay2);
   MdispFree(MilDisplay3);
   MdispFree(MilDisplay4);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

/* Utility function. */
void DisplayBayerToRGB(MIL_ID       SourceId,
                       MIL_ID       ChildId,
                       MIL_INT      ChildPosX,
                       MIL_INT      ChildPosY,
                       MIL_INT      ChildSizeX,
                       MIL_INT      ChildSizeY,
                       MIL_ID       DisplayId,
                       MIL_CONST_TEXT_PTR DisplayTitle,
                       MIL_INT      DisplayPosX,
                       MIL_INT      DisplayPosY)
   {
   MbufCopyColor2d(SourceId, ChildId, M_ALL_BANDS, ChildPosX, ChildPosY, M_ALL_BANDS, 0, 0, ChildSizeX, ChildSizeY);
   MdispControl(DisplayId, M_TITLE, DisplayTitle);
   MdispControl(DisplayId, M_WINDOW_INITIAL_POSITION_X, DisplayPosX);
   MdispControl(DisplayId, M_WINDOW_INITIAL_POSITION_Y, DisplayPosY);
   MdispZoom(DisplayId, DISPLAY_ZOOM_FACTOR, DISPLAY_ZOOM_FACTOR);
   MdispSelect(DisplayId, ChildId);
   }
