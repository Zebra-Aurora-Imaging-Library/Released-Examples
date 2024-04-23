﻿/********************************************************************/
/* 
 * File name: DirectionalLightingRemoval.cpp
 *
 * Synopsis:  This program demonstrates how to use the projection 
 *            primitive to remove a lighting ramp of an image.
 *            The method works well when the lighting ramp is axis
 *            aligned. This situation happens in applications using 
 *            line scan cameras.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/* Source image files. */
#define IMAGE_FILE_DIRECTIONALLIGHTING    M_IMAGE_PATH MIL_TEXT("Preprocessing/LightingRampWithNoise.mim")

//**********************************
// Header function 
//**********************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("DirectionalLightingRemoval\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program demonstrates how to use the projection \n")
             MIL_TEXT("primitives to remove a lighting ramp of an image.\n")
             MIL_TEXT("The method works well when the lighting ramp is axis aligned.\n")
             MIL_TEXT("This situation happens in applications using line scan cameras.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("image processing.\n\n"));
   }

int MosMain(void)
   {

   MIL_UNIQUE_APP_ID               MilApplication;                      /* Application identifier.                  */
   MIL_UNIQUE_SYS_ID               MilSystem;                           /* System identifier.                       */
   MIL_UNIQUE_DISP_ID              MilDisplay;                          /* Display identifier.                      */
   MIL_ID                          MilOverlayImage;                     /* Overlay image buffer identifier.         */
   MIL_INT                         SizeX, SizeY, OverlayClearColor;     /* Dimensions and Type of the source image. */

   /* Print Header. */
   PrintHeader();

   /* Allocate defaults. */
   MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);

   /* Restore source image in an image buffer. */
   auto MilImage = MbufRestore(IMAGE_FILE_DIRECTIONALLIGHTING, MilSystem, M_UNIQUE_ID);

   /* Set display properties. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);

   /* Pause to display the image buffer and prepare for overlay annotations. */
   MdispSelect(MilDisplay, MilImage);
   MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, &OverlayClearColor);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MgraText(M_DEFAULT, MilOverlayImage, 10, 10, MIL_TEXT("Source image"));
   MosPrintf(MIL_TEXT("The original image contains a lighting ramp.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Inquire the image dimensions. */
   MbufInquire(MilImage, M_SIZE_X, &SizeX);
   MbufInquire(MilImage, M_SIZE_Y, &SizeY);

   MbufClear(MilOverlayImage, (MIL_DOUBLE)OverlayClearColor);

   /************************************************ Remove a lighting ramp using MimProjection ************************************************/
   MosPrintf(MIL_TEXT("************************* Remove a lighting ramp using MimProjection ************************\n"));

   /* Clone an original image width to the 1D buffer. */
   auto ProjImage = MbufClone(MilImage, MilSystem, M_DEFAULT, 1, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   
   /* Project the source image to a 1D buffer (M_0_DEGREE) to obtain the estimated background. Projection is done using M_RANK_PERCENTILE = 10
      because of the presence of the big square in the center of the source image.*/
   MimProjection(MilImage, ProjImage, M_0_DEGREE, M_RANK_PERCENTILE, 10.0);
   
   /* Clone an original image. */
   auto ResizedBackgroundImage = MbufClone(MilImage, MilSystem, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   
   /* Resize the projected buffer by the Y-scale factor to have the same dimension as the source image. */
   MimResize(ProjImage, ResizedBackgroundImage, 1.0, (MIL_DOUBLE)SizeY, M_NEAREST_NEIGHBOR);
   
   /* Pause to display the source image and its background estimation. */
   MdispSelect(MilDisplay, ResizedBackgroundImage);
   MgraText(M_DEFAULT, MilOverlayImage, 10, 10, MIL_TEXT("Estimated background"));
   MosPrintf(MIL_TEXT("\nThis image shows the background estimation of the original image.\n")
             MIL_TEXT("This estimated background is obtained using the 2 following steps:\n")
             MIL_TEXT("\n\tSTEP 1: MimProjection is used to project the source image onto the axis at 0 degrees.\n")
             MIL_TEXT("\t\tIn this method, the projection axis angle is set to M_0_DEGREE, the operation\n")
             MIL_TEXT("\t\tis set to M_RANK_PERCENTILE, and the value of M_RANK_PERCENTILE is set to 10.\n")
             MIL_TEXT("\t\tM_RANK_PERCENTILE = 10 is selected due to the presence of the big square in the\n")
             MIL_TEXT("\t\tmiddle of the source image.\n")
             MIL_TEXT("\n\tSTEP 2: Since the original image is projected in M_0_DEGREE, MimResize is used\n")
             MIL_TEXT("\t\tto scale back the projected buffer to the original image size, and obtain the\n")
             MIL_TEXT("\t\testimated background.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Clear the overlay buffer for future use. */
   MbufClear(MilOverlayImage, (MIL_DOUBLE)OverlayClearColor);

   /* Subtract the the estimated background with a lighting ramp from the source image and save the result in a overlay image buffer.*/
   MimArith(MilImage, ResizedBackgroundImage, MilOverlayImage, M_SUB + M_SATURATION);
   
   /* Combine source image and result image. */
   auto CombinedImageProjection = MbufClone(MilImage, MilSystem, SizeX * 2, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MbufCopy(MilImage, CombinedImageProjection);
   MbufCopyColor2d(MilOverlayImage, CombinedImageProjection, M_ALL_BANDS, 0, 0, M_ALL_BANDS, SizeX, 0, SizeX, SizeY);

   /* Clear the overlay buffer for future use. */
   MbufClear(MilOverlayImage, (MIL_DOUBLE)OverlayClearColor);
   
   /* Pause to display the source image, the image with the removed lighting ramp, and prepare for overlay annotations. */
   MdispSelect(MilDisplay, CombinedImageProjection);
   MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, &OverlayClearColor);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MgraColor(M_DEFAULT, M_COLOR_WHITE);
   MgraText(M_DEFAULT, MilOverlayImage, 10, 10, MIL_TEXT("1. Lighting ramp included"));
   MgraText(M_DEFAULT, MilOverlayImage, 10, 30, MIL_TEXT("(Source image)"));
   MgraText(M_DEFAULT, MilOverlayImage, 10 + SizeX, 10, MIL_TEXT("2. Lighting ramp removed"));
   MgraText(M_DEFAULT, MilOverlayImage, 10 + SizeX, 30, MIL_TEXT("(using MimProjection)"));
   MosPrintf(MIL_TEXT("Image 1 shows the original image with a lighting ramp.\n"));
   MosPrintf(MIL_TEXT("Image 2 shows the result image obtained by removing the lighting ramp. This image is\n")
             MIL_TEXT("\t obtained by subtracting the estimated background from the source image. \n"));

   /* Wait for a key press. */
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n"));
   MosGetch();

   /* No free needed. */

   return 0;
   }




