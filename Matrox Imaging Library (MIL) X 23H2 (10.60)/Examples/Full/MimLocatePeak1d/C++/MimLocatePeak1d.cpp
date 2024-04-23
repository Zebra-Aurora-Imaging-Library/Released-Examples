/*****************************************************************************/
/*
 * File name: MImLocatePeak1d.cpp
 *
 * Synopsis:  This program finds the peak in each column of an input sequence
 *            and reconstruct the height of a 3D object using it.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include <mil.h>

/* Input sequence specifications. */
#define SEQUENCE_FILE   M_IMAGE_PATH MIL_TEXT("HandWithLaser.avi")

//     ^            +
//     |        +       +
//     |      + <-Width-> + <------------
//     |     +             +             | Min contrast
//     | ++++               ++++++++ <---
//     |
//     |
//     ------------------------------>
//        Peak intensity profile

/* Peak detection parameters. */
#define LINE_WIDTH_AVERAGE      20
#define LINE_WIDTH_DELTA        20
#define MIN_CONTRAST           100.0
#define NB_FIXED_POINT           4

/* 3D display parameters. */
#define M3D_MESH_SCALING_X     1.0
#define M3D_MESH_SCALING_Y     4.0
#define M3D_MESH_SCALING_Z    -0.13

/* Utility functions. */
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);
void AutoRotate3dDisplay(MIL_ID MilDisplay);

//*****************************************************************************
int MosMain(void)
   {
   MIL_ID MilApplication,          /* Application identifier.        */
          MilSystem,               /* System identifier.             */
          MilDisplay,              /* Display identifier.            */
          MilDisplayImage,         /* Image buffer identifier.       */
          MilGraList,              /* Graphic list identifier.       */
          MilImage,                /* Image buffer identifier.       */
          MilPosYImage,            /* Image buffer identifier.       */
          MilValImage,             /* Image buffer identifier.       */
          MilContext,              /* Processing context identifier. */
          MilLocatePeak,           /* Processing result identifier.  */
          MilStatContext = M_NULL, /* Statistics context identifier. */
          MilExtreme = M_NULL;     /* Result buffer identifier.      */

   MIL_INT SizeX,
           SizeY,
           NumberOfImages,  
           n;
   MIL_INT ExtremePosY[2] = { 0, 0 };
   MIL_DOUBLE FrameRate,
              PreviousTime,
              StartTime,
              EndTime,
              TotalProcessTime,
              WaitTime;

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem,
      &MilDisplay, M_NULL, M_NULL);

   /* Inquire characteristics of the input sequence. */
   MbufDiskInquire(SEQUENCE_FILE, M_SIZE_X, &SizeX);
   MbufDiskInquire(SEQUENCE_FILE, M_SIZE_Y, &SizeY);
   MbufDiskInquire(SEQUENCE_FILE, M_NUMBER_OF_IMAGES, &NumberOfImages);
   MbufDiskInquire(SEQUENCE_FILE, M_FRAME_RATE, &FrameRate);

   /* Allocate buffers to hold images. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &MilImage);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_DISP, &MilDisplayImage);
   MbufAlloc2d(MilSystem, SizeX, NumberOfImages, 16 + M_UNSIGNED, M_IMAGE + M_PROC, &MilPosYImage);
   MbufAlloc2d(MilSystem, SizeX, NumberOfImages, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &MilValImage);

   /* Allocate context for MimLocatePeak1D. */
   MimAlloc(MilSystem, M_LOCATE_PEAK_1D_CONTEXT, M_DEFAULT, &MilContext);

   /* Allocate result for MimLocatePeak1D. */
   MimAllocResult(MilSystem, M_DEFAULT, M_LOCATE_PEAK_1D_RESULT, &MilLocatePeak);

   /* Allocate graphic list. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraList);

   /* Select display. */
   MdispSelect(MilDisplay, MilDisplayImage);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nEXTRACTING 3D IMAGE FROM A LASER LINE (SHEET-OF-LIGHT):\n"));
   MosPrintf(MIL_TEXT("--------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("The position of a laser line is being extracted from ")
      MIL_TEXT("an image\n"));
   MosPrintf(MIL_TEXT("to generate a depth image.\n\n"));

   /* Open the sequence file for reading. */
   MbufImportSequence(SEQUENCE_FILE, M_DEFAULT, M_NULL, M_NULL, NULL, M_NULL, M_NULL, M_OPEN);

   /* Preprocess the context. */
   MimLocatePeak1d(MilContext,
      MilImage,
      MilLocatePeak,
      M_NULL,
      M_NULL,
      M_NULL,
      M_PREPROCESS,
      M_DEFAULT);

   /* Read and process all images in the input sequence. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &PreviousTime);
   TotalProcessTime = 0.0;

   for (n = 0; n < NumberOfImages; n++)
      {
      /* Read image from sequence. */
      MbufImportSequence(SEQUENCE_FILE, M_DEFAULT, M_LOAD, M_NULL,
         &MilImage, M_DEFAULT, 1, M_READ);

      /* Display the image. */
      MbufCopy(MilImage, MilDisplayImage);

      /* Locate the peak in each column of the image. */
      MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &StartTime);

      MimLocatePeak1d(MilContext,
         MilImage,
         MilLocatePeak,
         LINE_WIDTH_AVERAGE,
         LINE_WIDTH_DELTA,
         MIN_CONTRAST,
         M_DEFAULT,
         M_DEFAULT);

      /* Draw extracted peaks. */
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MgraClear(M_DEFAULT, MilGraList);
      MimDraw(M_DEFAULT, MilLocatePeak, M_NULL, MilGraList, M_DRAW_PEAKS + M_CROSS, M_ALL, M_DEFAULT, M_DEFAULT);

      /* Draw peak's data to depth map. */
      MimDraw(M_DEFAULT, MilLocatePeak, M_NULL, MilPosYImage, M_DRAW_DEPTH_MAP_ROW, MIL_DOUBLE(n), M_NULL, M_FIXED_POINT + NB_FIXED_POINT);
      MimDraw(M_DEFAULT, MilLocatePeak, M_NULL, MilValImage, M_DRAW_INTENSITY_MAP_ROW, MIL_DOUBLE(n), M_NULL, M_DEFAULT);

      MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &EndTime);
      TotalProcessTime += EndTime - StartTime;

      /* Wait to have a proper frame rate. */
      WaitTime = (1.0 / FrameRate) - (EndTime - PreviousTime);
      if (WaitTime > 0)
         MappTimer(M_DEFAULT, M_TIMER_WAIT, &WaitTime);
      MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &PreviousTime);
      }

   MgraClear(M_DEFAULT, MilGraList);

   /* Close the sequence file. */
   MbufImportSequence(SEQUENCE_FILE, M_DEFAULT, M_NULL, M_NULL, NULL, M_NULL, M_NULL, M_CLOSE);

   MosPrintf(MIL_TEXT("%d images processed in %7.2lf s (%7.2lf ms/image).\n"),
      (int)NumberOfImages, TotalProcessTime,
      TotalProcessTime / (MIL_DOUBLE)NumberOfImages * 1000.0);

   /* Pause to show the result. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("The reconstructed images are being displayed.\n"));

   /* Draw extracted peak position in each column of each image. */
   const MIL_INT VisualizationDelayMsec = 10;
   for (n = 0; n < NumberOfImages; n++)
      {
      MbufClear(MilImage, 0);
      MimDraw(M_DEFAULT, MilPosYImage, MilValImage, MilImage,
         M_DRAW_PEAKS + M_VERTICAL + M_LINES, (MIL_DOUBLE)n,
         1, M_FIXED_POINT + NB_FIXED_POINT);

      /* Display the result image. */
      MbufCopy(MilImage, MilDisplayImage);

      MosSleep(VisualizationDelayMsec);
      }

   /* Pause to show the result. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Try to allocate 3D display. */
   MIL_ID MilDisplay3D = Alloc3dDisplayId(MilSystem);
   if(MilDisplay3D)
      {
      McalUniform(MilPosYImage, 0.0, 0.0, M3D_MESH_SCALING_X, M3D_MESH_SCALING_Y, 0.0, M_DEFAULT);
      McalControl(MilPosYImage, M_GRAY_LEVEL_SIZE_Z, M3D_MESH_SCALING_Z);

      MIL_ID ContainerId = MbufAllocContainer(MilSystem, M_PROC | M_DISP, M_DEFAULT, M_NULL);
      MbufConvert3d(MilPosYImage, ContainerId, M_NULL, M_DEFAULT, M_DEFAULT);
      MIL_ID Reflectance = MbufAllocComponent(ContainerId, 1, SizeX, NumberOfImages, 8 + M_UNSIGNED, M_IMAGE, M_COMPONENT_REFLECTANCE, M_NULL);
      MbufCopy(MilValImage, Reflectance);
      MosPrintf(MIL_TEXT("The depth buffer is displayed using 3D MIL.\n"));
      MosPrintf(MIL_TEXT("Press <R> on the display window to stop/start the rotation.\n\n"));

      /* Hide Mil Display. */
      MdispControl(MilDisplay, M_WINDOW_SHOW, M_DISABLE);

      M3ddispSelect(MilDisplay3D, ContainerId, M_SELECT, M_DEFAULT);
      AutoRotate3dDisplay(MilDisplay3D);

      /* Pause to show the result. */
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      M3ddispFree(MilDisplay3D);
      MbufFree(ContainerId);      
      }
   else
      {
      MosPrintf(MIL_TEXT("The depth buffer is displayed using 2D MIL.\n"));

      /* Find the remapping for result buffers. */
      MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, &MilStatContext);
      MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, &MilExtreme);

      MimControl(MilStatContext, M_STAT_MIN, M_ENABLE);
      MimControl(MilStatContext, M_STAT_MAX, M_ENABLE);
      MimControl(MilStatContext, M_CONDITION, M_NOT_EQUAL);
      MimControl(MilStatContext, M_COND_LOW, 0xFFFF);

      MimStatCalculate(MilStatContext, MilPosYImage, MilExtreme, M_DEFAULT);
      MimGetResult(MilExtreme, M_STAT_MIN + M_TYPE_MIL_INT, &ExtremePosY[0]);
      MimGetResult(MilExtreme, M_STAT_MAX + M_TYPE_MIL_INT, &ExtremePosY[1]);

      MimFree(MilExtreme);
      MimFree(MilStatContext);

      /* Free the display and reallocate a new one of the proper dimension for results. */
      MbufFree(MilDisplayImage);
      MbufAlloc2d(MilSystem,
         (MIL_INT) ( (MIL_DOUBLE) SizeX * (M3D_MESH_SCALING_X > 0 ? M3D_MESH_SCALING_X : -M3D_MESH_SCALING_X) ),
         (MIL_INT) ( (MIL_DOUBLE) NumberOfImages * M3D_MESH_SCALING_Y),
         8 + M_UNSIGNED,
         M_IMAGE + M_PROC + M_DISP,
         &MilDisplayImage);

      MdispSelect(MilDisplay, MilDisplayImage);

      /* Display the height buffer. */
      MimClip(MilPosYImage, MilPosYImage, M_GREATER,
         (MIL_DOUBLE) ExtremePosY[1], M_NULL,
         (MIL_DOUBLE) ExtremePosY[1], M_NULL);
      MimArith(MilPosYImage, (MIL_DOUBLE) ExtremePosY[0],
         MilPosYImage, M_SUB_CONST);
      MimArith(MilPosYImage, ((ExtremePosY[1] - ExtremePosY[0]) / 255.0) + 1,
         MilPosYImage, M_DIV_CONST);
      MimResize(MilPosYImage, MilDisplayImage,
         M_FILL_DESTINATION,
         M_FILL_DESTINATION,
         M_BILINEAR);

      /* Pause to show the result. */
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      }

   /* Free all allocations. */
   MimFree(MilLocatePeak);
   MimFree(MilContext);
   MbufFree(MilImage);
   MgraFree(MilGraList);
   MbufFree(MilDisplayImage);
   MbufFree(MilPosYImage);
   MbufFree(MilValImage);
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier. 
//*****************************************************************************
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n\n"));
      }
   return MilDisplay3D;
   }

//****************************************************************************
// Auto rotate the 3D object.
//****************************************************************************
void AutoRotate3dDisplay(MIL_ID MilDisplay)
   {
   M3ddispControl(MilDisplay, M_AUTO_ROTATE, M_ENABLE);
   }
