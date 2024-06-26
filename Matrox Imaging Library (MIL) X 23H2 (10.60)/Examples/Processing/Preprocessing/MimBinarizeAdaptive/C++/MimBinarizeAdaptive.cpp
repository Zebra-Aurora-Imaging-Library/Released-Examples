﻿/************************************************************************************/
/*
* File name: MimBinarizeAdaptive.cpp
*
* Synopsis:  This program demonstrates how to perform adaptive binarization.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

/* Example functions prototypes. */
void  TextBinarization(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename);
void  PlasticCupBinarization(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename);
void  HardDiskBinarization(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename);

/* Source images file name. */

#define IMAGE_TEXT     M_IMAGE_PATH  MIL_TEXT("Preprocessing/PrintedText.mim")
#define IMAGE_CUP      M_IMAGE_PATH  MIL_TEXT("PlasticCup.mim")
#define IMAGE_DISK     M_IMAGE_PATH  MIL_TEXT("Preprocessing/HardDisk.mim")

/* Coordinates of the zone of interest of the plastic cup. */
#define CUP_INTERIOR_X_BEGIN   145
#define CUP_INTERIOR_Y_BEGIN   165
#define CUP_INTERIOR_X_LENGTH  300
#define CUP_INTERIOR_Y_LENGTH  190

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("BinarizeAdaptive\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program demonstrates how to perform\n")
             MIL_TEXT("adaptive binarization.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

int MosMain()
   {

   PrintHeader();

   MIL_ID MilApplication,    /* Application identifier.   */
      MilSystem,             /* System identifier.        */
      MilDisplay;            /* Display identifier.       */

   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   TextBinarization(MilSystem, MilDisplay, IMAGE_TEXT);
   PlasticCupBinarization(MilSystem, MilDisplay, IMAGE_CUP);
   HardDiskBinarization(MilSystem, MilDisplay, IMAGE_DISK);

   /* Free objects. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }


/*****************************************************************************/
/*            Performs text binarization using Niblack's algorithm.               */
void  TextBinarization(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename)
   {
   MIL_ID   MilImage,          /* Image buffer identifier.         */
            MilOverlayImage,   /* Overlay image buffer identifier. */
            MilSubImage00,     /* Child buffer identifier.         */
            MilSubImage01,     /* Child buffer identifier.         */
            MilContext;        /* Processing context identifier.   */

   MIL_INT   Type, SizeX, SizeY, OverlayClearColor;

   /* Inquire the image Size and Type. */
   MbufDiskInquire(Filename, M_SIZE_X, &SizeX);
   MbufDiskInquire(Filename, M_SIZE_Y, &SizeY);
   MbufDiskInquire(Filename, M_TYPE, &Type);

   /* Allocate image buffers. */
   MbufAlloc2d(MilSystem, SizeX * 2, SizeY,
      Type, M_IMAGE + M_PROC + M_DISP, &MilImage);

   MbufClear(MilImage, 0);

   /* Allocate child buffers in the 2 halves of the display image. */
   MbufChild2d(MilImage, 0L, 0L, SizeX, SizeY, &MilSubImage00);
   MbufChild2d(MilImage, SizeX, 0L, SizeX, SizeY, &MilSubImage01);
   /* Load an image. */
   MbufLoad(Filename, MilSubImage00);

   /* Perform global binarization. */
   MimBinarize(MilSubImage00, MilSubImage01, M_BIMODAL + M_GREATER, M_NULL, M_NULL);
   
   /* Display the image buffer and prepare overlay annotations. */
   MdispSelect (MilDisplay, MilImage);
   MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, &OverlayClearColor);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MbufClear   (MilOverlayImage, (MIL_DOUBLE) OverlayClearColor);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraText(M_DEFAULT, MilOverlayImage, 0, 0, MIL_TEXT(" Source image"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX, 0, MIL_TEXT(" Global binarization"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("The image has been segmented using a global bimodal binarization. \n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Allocate the context. */
   MimAlloc(MilSystem, M_BINARIZE_ADAPTIVE_CONTEXT,
      M_DEFAULT, &MilContext);

   /* Set binarization controls. */
   MimControl(MilContext, M_THRESHOLD_MODE, M_NIBLACK);
   MimControl(MilContext, M_FOREGROUND_VALUE, M_FOREGROUND_BLACK);
   MimControl(MilContext, M_MINIMUM_CONTRAST, 6);
   MimControl(MilContext, M_GLOBAL_MIN, 65);

   /* Perform binarization. */
   MimBinarizeAdaptive(MilContext, MilSubImage00,
      M_NULL, M_NULL, MilSubImage01, M_NULL, M_DEFAULT);

   /* Update annotations. */
   MbufClear(MilOverlayImage, (MIL_DOUBLE) OverlayClearColor);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraText(M_DEFAULT, MilOverlayImage, 0, 0, MIL_TEXT(" Source image"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX, 0, MIL_TEXT(" Local adaptive binarization"));
   /* Print a message. */
   MosPrintf(MIL_TEXT("The image has been segmented using Niblack\'s local adaptive binarization. \n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Free allocated identifiers. */
   MimFree(MilContext);
   MbufFree(MilSubImage00);
   MbufFree(MilSubImage01);
   MbufFree(MilImage);

   }


/*****************************************************************************/
/*            Performs plastic cup binarization using Niblack's algorithm.               */
void  PlasticCupBinarization(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename)
   {
   MIL_ID   MilImage,      /* Image buffer identifier.         */
      MilOverlayImage,     /* Overlay image buffer identifier. */
      MilZoneOfInterest00, /* Child buffer identifier.         */
      MilZoneOfInterest01, /* Child buffer identifier.         */
      MilSubImage00,       /* Child buffer identifier.         */
      MilSubImage01,       /* Child buffer identifier.         */
      MilContext;          /* Processing context identifier.   */

   MIL_INT   Type, SizeX, SizeY, OverlayClearColor;

   /* Inquire the image Size and Type. */
   MbufDiskInquire(Filename, M_SIZE_X, &SizeX);
   MbufDiskInquire(Filename, M_SIZE_Y, &SizeY);
   MbufDiskInquire(Filename, M_TYPE, &Type);

   /* Allocate image buffers. */
   MbufAlloc2d(MilSystem, SizeX * 2, SizeY,
      Type, M_IMAGE + M_PROC + M_DISP, &MilImage);

   MbufClear(MilImage, 0);

   /* Display the image buffer and prepare for overlay annotations. */

   /* Allocate child buffers in the 2 halves of the display image. */
   MbufChild2d(MilImage, 0L, 0L, SizeX, SizeY, &MilSubImage00);
   MbufChild2d(MilImage, SizeX, 0L, SizeX, SizeY, &MilSubImage01);

   /* Allocate child buffers for the zones of interest. */
   MbufChild2d(MilSubImage00, CUP_INTERIOR_X_BEGIN, CUP_INTERIOR_Y_BEGIN,
      CUP_INTERIOR_X_LENGTH, CUP_INTERIOR_Y_LENGTH, &MilZoneOfInterest00);
   MbufChild2d(MilSubImage01, CUP_INTERIOR_X_BEGIN, CUP_INTERIOR_Y_BEGIN,
      CUP_INTERIOR_X_LENGTH, CUP_INTERIOR_Y_LENGTH, &MilZoneOfInterest01);


   /* Display the image buffer and prepare overlay annotations. */
   MdispSelect(MilDisplay, MilImage);
   MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, &OverlayClearColor);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MbufClear(MilOverlayImage, (MIL_DOUBLE) OverlayClearColor);

   /* Draw the zone of interest location. */
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraRect(M_DEFAULT,
            MilOverlayImage,
            CUP_INTERIOR_X_BEGIN,
            CUP_INTERIOR_Y_BEGIN,
            CUP_INTERIOR_X_BEGIN + CUP_INTERIOR_X_LENGTH,
            CUP_INTERIOR_Y_BEGIN + CUP_INTERIOR_Y_LENGTH);

   /* Load an image. */
   MbufLoad(Filename, MilSubImage00);

   /* Perform global binarization. */
   MimBinarize(MilZoneOfInterest00, MilZoneOfInterest01, M_BIMODAL + M_GREATER, M_NULL, M_NULL);

   /* Post-process the resulting image. */
   MimClose(MilZoneOfInterest01, MilZoneOfInterest01, 1, M_BINARY);
   MimRank(MilZoneOfInterest01, MilZoneOfInterest01,
      M_3X3_RECT, M_MEDIAN, M_BINARY);

   /* Resize the zone of interest to the size of display. */
   MimResize(MilZoneOfInterest01, MilSubImage01,
      M_FILL_DESTINATION, M_FILL_DESTINATION, M_BILINEAR);

   MgraText(M_DEFAULT, MilOverlayImage, 0, 0, MIL_TEXT(" Source image"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX, 0, MIL_TEXT(" Global binarization"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("The image has been segmented using a global bimodal binarization. \n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Allocate the context. */
   MimAlloc(MilSystem, M_BINARIZE_ADAPTIVE_CONTEXT,
      M_DEFAULT, &MilContext);

   /* Set binarization controls. */
   MimControl(MilContext, M_THRESHOLD_MODE, M_PSEUDOMEDIAN);
   MimControl(MilContext, M_FOREGROUND_VALUE, M_FOREGROUND_WHITE);
   MimControl(MilContext, M_GLOBAL_OFFSET, 10);

   /* Perform binarization. */
   MimBinarizeAdaptive(MilContext, MilZoneOfInterest00,
      M_NULL, M_NULL, MilZoneOfInterest01, M_NULL, M_DEFAULT);

   /* Post-process the resulting image. */
   MimClose(MilZoneOfInterest01, MilZoneOfInterest01, 1, M_BINARY);
   MimRank(MilZoneOfInterest01, MilZoneOfInterest01,
      M_3X3_RECT, M_MEDIAN, M_BINARY);

   /* Resize the zone of interest to the size of display. */
   MimResize(MilZoneOfInterest01, MilSubImage01,
      M_FILL_DESTINATION, M_FILL_DESTINATION, M_BILINEAR);

   /* Update annotations. */
   MbufClear(MilOverlayImage, (MIL_DOUBLE) OverlayClearColor);
   MgraRect(M_DEFAULT,
      MilOverlayImage,
      CUP_INTERIOR_X_BEGIN,
      CUP_INTERIOR_Y_BEGIN,
      CUP_INTERIOR_X_BEGIN + CUP_INTERIOR_X_LENGTH,
      CUP_INTERIOR_Y_BEGIN + CUP_INTERIOR_Y_LENGTH);

   MgraText(M_DEFAULT, MilOverlayImage, 0, 0, MIL_TEXT(" Source image"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX, 0, MIL_TEXT(" Local adaptive binarization"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("The image has been segmented using the pseudomedian local adaptive\nbinarization.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Clear the overlay buffer. */
   MbufClear(MilOverlayImage, (MIL_DOUBLE) OverlayClearColor);

   /* Free allocated identifiers. */
   MimFree(MilContext);
   MbufFree(MilZoneOfInterest00);
   MbufFree(MilZoneOfInterest01);
   MbufFree(MilSubImage00);
   MbufFree(MilSubImage01);
   MbufFree(MilImage);

   }

/*****************************************************************************/
/*       Performs hard drive binarization using Niblack's algorithm.         */
void  HardDiskBinarization(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename)
   {
   MIL_ID   MilImage,          /* Image buffer identifier.         */
            MilOverlayImage,   /* Overlay image buffer identifier. */
            MilSubImage00,     /* Child buffer identifier.         */
            MilSubImage01,     /* Child buffer identifier.         */
            MilContext;        /* Processing context identifier.   */

   MIL_INT   Type, SizeX, SizeY, OverlayClearColor;

   /* Inquire the image Size and Type. */
   MbufDiskInquire(Filename, M_SIZE_X, &SizeX);
   MbufDiskInquire(Filename, M_SIZE_Y, &SizeY);
   MbufDiskInquire(Filename, M_TYPE, &Type);

   /* Allocate image buffers. */
   MbufAlloc2d(MilSystem, SizeX * 2, SizeY,
      Type, M_IMAGE + M_PROC + M_DISP, &MilImage);

   MbufClear(MilImage, 0);

   /* Allocate child buffers in the 2 halves of the display image. */
   MbufChild2d(MilImage, 0L, 0L, SizeX, SizeY, &MilSubImage00);
   MbufChild2d(MilImage, SizeX, 0L, SizeX, SizeY, &MilSubImage01);

   /* Load an image. */
   MbufLoad(Filename, MilSubImage00);

   MimBinarize(MilSubImage00, MilSubImage01, M_BIMODAL + M_GREATER, M_NULL, M_NULL);

   /* Post-process the resulting image. */
   MimRank(MilSubImage01, MilSubImage01,
      M_3X3_RECT, M_MEDIAN, M_BINARY);

   /* Display the image buffer and prepare overlay annotations. */
   MdispSelect(MilDisplay, MilImage);
   MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, &OverlayClearColor);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MbufClear(MilOverlayImage, (MIL_DOUBLE) OverlayClearColor);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraText(M_DEFAULT, MilOverlayImage, 0, 0, MIL_TEXT(" Source image"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX, 0, MIL_TEXT(" Global binarization"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("The image has been segmented using a global bimodal binarization. \n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Allocate the context. */
   MimAlloc(MilSystem, M_BINARIZE_ADAPTIVE_CONTEXT, M_DEFAULT, &MilContext);

   /* Set binarization controls. */
   MimControl(MilContext, M_THRESHOLD_MODE, M_NIBLACK);
   MimControl(MilContext, M_FOREGROUND_VALUE, M_FOREGROUND_BLACK);
   MimControl(MilContext, M_MINIMUM_CONTRAST, 1.65);
   MimControl(MilContext, M_NIBLACK_BIAS, 0.3);
   MimControl(MilContext, M_AVERAGE_MODE, M_GAUSSIAN);


   /* Perform binarization. */
   MimBinarizeAdaptive(MilContext, MilSubImage00,
      M_NULL, M_NULL, MilSubImage01, M_NULL, M_DEFAULT);

   /* Post-process the resulting image. */
   MimRank(MilSubImage01, MilSubImage01,
      M_3X3_RECT, M_MEDIAN, M_BINARY);

   /* Display resulting image. */
   MdispSelect(MilDisplay, MilImage);

   /* Update annotations. */
   MbufClear(MilOverlayImage, (MIL_DOUBLE) OverlayClearColor);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraText(M_DEFAULT, MilOverlayImage, 0, 0, MIL_TEXT(" Source image"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX, 0, MIL_TEXT(" Local adaptive binarization"));
   
   /* Print a message. */
   MosPrintf(MIL_TEXT("The image has been binarized using Niblack\'s local adaptive binarization with\n"));
   MosPrintf(MIL_TEXT("a Gaussian average mode. \n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free allocated identifiers. */
   MimFree(MilContext);
   MbufFree(MilSubImage00);
   MbufFree(MilSubImage01);
   MbufFree(MilImage);

   }
