/*******************************************************************************/
/* 
 * File name: SimpleDilateErode.cpp
 *
 * Synopsis:  This program improves the quality of the segmented image
 *            using morphological erosion and dilation operations.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/* Source MIL image file specifications. */
#define IMAGE_FILE             M_IMAGE_PATH MIL_TEXT("Preprocessing/DotMatrixSerial.mim")

/* Small salt and pepper noise radius (in pixels). */
#define SMALL_NOISE_RADIUS    1L

/* Max distance between the dots of the characters (in pixels). */
#define CHARACTER_MAX_DOT_SPACING 6L

int MosMain(void)
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\nSimpleDilateErode\n\n"));

   MIL_ID MilApplication,  /* Application identifier.  */
          MilSystem,       /* System identifier.       */
          MilDisplay,      /* Display identifier.      */
          MilImage,        /* Image buffer identifier. */
          BinImage;        /* Binary image buffer identifier. */

   MIL_INT SizeX, SizeY;

   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Restore source image in an image buffer and display it. */
   MbufRestore(IMAGE_FILE, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Inquire the image dimensions. */
   MbufInquire(MilImage, M_SIZE_X, &SizeX);
   MbufInquire(MilImage, M_SIZE_Y, &SizeY);

   /* Allocate a binary image buffer for fast processing. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &BinImage);

   /* Pause to show the original image. */
   MosPrintf(MIL_TEXT("\nThis program segments the dot matrix\n"));
   MosPrintf(MIL_TEXT("characters in the displayed image.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Binarize the image. */
   MimBinarize(MilImage, BinImage, M_BIMODAL+M_LESS, M_NULL, M_NULL);

   /* Erode the image to remove small salt and pepper noise. */
   MimErode(BinImage, BinImage, SMALL_NOISE_RADIUS, M_BINARY);

   /* Dilate the image to merge the character dots. */
   MimDilate(BinImage, BinImage, CHARACTER_MAX_DOT_SPACING/2 + SMALL_NOISE_RADIUS, M_BINARY);

   /* Display the resulting image. */
   MbufClear(MilImage, 0);
   MbufClearCond(MilImage, 255, 0, 255, BinImage, M_EQUAL, 1);

   /* Pause to show the resulting image. */
   MosPrintf(MIL_TEXT("The dot matrix characters have been segmented using\n"));
   MosPrintf(MIL_TEXT("morphological erosion and dilation operations.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   /* Free all allocations. */
   MbufFree(BinImage);
   MbufFree(MilImage);
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
   return 0;
   }
