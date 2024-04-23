/*******************************************************************************/
/*
* File name: MgraVectors.cpp
*
* Synopsis:  This program illustrates the use of graphical vector
*            annotations by displaying a gradient vector field.
*
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

#define EXAMPLE_IMAGE_PATH    M_IMAGE_PATH   MIL_TEXT("Seals.mim")

#define MIN_BLOB_RADIUS       1L
#define LOWPARAM_THRESHOLD    80
#define VECTOR_FIELD_STRIDE   2
#define VECTOR_FIELD_SCALE    0.03
#define DISP_ZOOM_FACTOR      4
#define DISP_PAN_OFFSET_X     310
#define DISP_PAN_OFFSET_Y     80

int MosMain(void)
   {

   MIL_ID MilApplication,      /* Application identifier.   */
          MilSystem,           /* System Identifier.        */
          MilDisplay,          /* Display identifier.       */
          MilImage,            /* Image buffer identifier.  */
          DerivativeX,         /* Image buffer identifier.  */
          DerivativeY,         /* Image buffer identifier.  */
          MilMagnitude,        /* Image buffer identifier.  */
          MilMask;             /* Image buffer identifier.  */

   /* Print header. */
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("MgraVectors\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("The image gradient is calculated and the gradient \n")
             MIL_TEXT("vector field along the objects' contours is displayed.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Buffer, Display, Graphics, Image Processing\n\n")
             );

   /* Allocate defaults. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Restore the source image. */
   MbufRestore(EXAMPLE_IMAGE_PATH, MilSystem, &MilImage);

   /* Allocate buffers. */
   MIL_INT SizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   MbufAlloc2d(MilSystem, SizeX, SizeY, 32 + M_FLOAT, M_IMAGE + M_PROC, &DerivativeX);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 32 + M_FLOAT, M_IMAGE + M_PROC, &DerivativeY);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 32 + M_FLOAT, M_IMAGE + M_PROC, &MilMagnitude);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1L, M_IMAGE + M_PROC, &MilMask);

   /* Compute first derivatives. */
   MimConvolve(MilImage, MilImage, M_SMOOTH);
   MimConvolve(MilImage, DerivativeX, M_SOBEL_X);
   MimConvolve(MilImage, DerivativeY, M_SOBEL_Y);

   /* Compute the gradient magnitude. */
   MimTransform(DerivativeX, DerivativeY, MilMagnitude, M_NULL, M_POLAR, M_DEFAULT);

   /* Create a mask image along the strongest gradients. */
   MimBinarize(MilMagnitude, MilMask, M_FIXED + M_GREATER, LOWPARAM_THRESHOLD, M_NULL);

   /* Remove small particles and small holes. */
   MimOpen(MilMask, MilMask, MIN_BLOB_RADIUS, M_BINARY);
   MimClose(MilMask, MilMask, MIN_BLOB_RADIUS, M_BINARY);

   /* Apply mask image */
   MbufClearCond(DerivativeX, 0, M_NULL, M_NULL, MilMask, M_EQUAL, 0);
   MbufClearCond(DerivativeY, 0, M_NULL, M_NULL, MilMask, M_EQUAL, 0);

   /* Allocate a graphic list to hold annotations to draw. */
   MIL_ID MilGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   /* Allocate a graphics context for the draw operations. */
   MIL_ID GraContextId = MgraAlloc(MilSystem, M_NULL);

   /* Set the drawing color to red. */
   MgraColor(GraContextId, M_COLOR_RED);

   /* Associate the list to the display. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   /* Draw a few arbitrary vectors in the list. */
   MgraVectorsGrid(GraContextId,
                   MilGraphicList,
                   DerivativeX,
                   DerivativeY,
                   VECTOR_FIELD_STRIDE,
                   M_ABSOLUTE,
                   VECTOR_FIELD_SCALE,
                   M_SKIP_NULL_VECTORS |
                   M_FIXED_LENGTH_ARROWHEADS);

   /* Display the image buffer. */
   MdispControl(MilDisplay, M_VIEW_MODE, M_AUTO_SCALE);
   MdispSelect(MilDisplay, MilImage);
   MdispZoom(MilDisplay, DISP_ZOOM_FACTOR, DISP_ZOOM_FACTOR);
   MdispPan(MilDisplay, DISP_PAN_OFFSET_X, DISP_PAN_OFFSET_Y);

   /* Pause to show the result. */
   MosPrintf(MIL_TEXT("Press <ENTER> to end"));
   MosGetchar();

   /* Remove the association between the list and the display. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL);

   /* Free buffers. */
   MbufFree(MilImage);
   MbufFree(DerivativeX);
   MbufFree(DerivativeY);
   MbufFree(MilMagnitude);
   MbufFree(MilMask);

   /* Free graphic list. */
   MgraFree(MilGraphicList);

   /* Free graphic context. */
   MgraFree(GraContextId);

   /* Release defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }
