/************************************************************************************/
/*
 * File name: Projection.cpp 
 *
 * Synopsis:  This program demonstrates how to use the projection 
 *            primitive with various operators.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/* Example functions prototypes. */
void GraphProjection      (MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename);

/* Source images file name. */
#define IMAGE_WAFER           M_IMAGE_PATH  MIL_TEXT("Wafer.mim")

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("Projection\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program demonstrates how to use the projection\n")
      MIL_TEXT("primitive with various operators.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n")
      MIL_TEXT("graphic, image processing.\n\n"));
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

   /* Run the projection visualisation example. */
   GraphProjection(MilSystem,MilDisplay,IMAGE_WAFER);
         
   /* Free objects. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
}


/*********************************************************************************************************/
/*  Show different projection results on a single graph. */

void GraphProjection(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_CONST_TEXT_PTR Filename)
   {

   MIL_ID MilImage,           /* Image buffer identifier.                 */
          MilOverlayImage,    /* Overlay image buffer identifier.         */
          MilGraphicList,     /* Graphic list identifier.                 */
          MilSubImage00,      /* Child buffer identifier.                 */
          MilSubImage01,      /* Child buffer identifier.                 */
          MilOverlaySubImage, /* Child buffer identifier.                 */
          MilResultId;        /* Result buffer identifier.                */

   MIL_INT  Type,SizeX, SizeY, OverlayClearColor;

   /* Inquire the image Size and Type. */
   MbufDiskInquire(Filename,M_SIZE_X,&SizeX);
   MbufDiskInquire(Filename,M_SIZE_Y,&SizeY);
   MbufDiskInquire(Filename,M_TYPE,  &Type);

   const MIL_INT GraphSizeY = 256 + 50;

   /* Allocate a display buffer and clear it. */
   MbufAlloc2d(MilSystem, SizeX, SizeY + GraphSizeY,
      Type, M_IMAGE+M_PROC+M_DISP, &MilImage);
   MbufClear(MilImage, M_COLOR_BLACK);

   /* Display the image buffer and prepare for overlay annotations. */
   MdispSelect (MilDisplay,   MilImage);
   MdispInquire(MilDisplay,   M_TRANSPARENT_COLOR, &OverlayClearColor);
   MdispInquire(MilDisplay,   M_OVERLAY_ID,        &MilOverlayImage);
   MbufClear(MilOverlayImage, (MIL_DOUBLE)OverlayClearColor);

   /* Allocate graphic list.*/
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);

   /* Allocate child buffers in the 4 quadrants of the display image. */
   MbufChild2d(MilImage,        0L,    0L, SizeX, SizeY,      &MilSubImage00);
   MbufChild2d(MilImage,        0L, SizeY, SizeX, GraphSizeY, &MilSubImage01);
   MbufChild2d(MilOverlayImage, 0L, SizeY, SizeX, GraphSizeY, &MilOverlaySubImage);

   /*Allocate the Result buffer. */
   MimAllocResult(MilSystem, SizeX, M_PROJ_LIST, &MilResultId);

   /* Allocate graph data. */
   MIL_DOUBLE* ColumnIndexes   = new MIL_DOUBLE[SizeX];
   MIL_DOUBLE* ProjectedValues = new MIL_DOUBLE[SizeX];
   
   /* Initialize column indexes. */
   for (MIL_INT ii = 0; ii < SizeX; ++ii)
      ColumnIndexes[ii] = (MIL_DOUBLE)ii;

   /* Load a noisy image. */
   MbufLoad(Filename,     MilSubImage00);
   MbufClear(MilSubImage01, 0);


   /* Prepare projection operations. */
   const MIL_INT   NumberOfOperations    = 6;
   const MIL_INT64 Operations[6]         = {M_SUM, M_MAX, M_MIN, M_MEDIAN, M_RANK, M_RANK_PERCENTILE };
   const MIL_INT64 OperationColors[6]    = {M_COLOR_DARK_CYAN, M_COLOR_GREEN, M_COLOR_RED, M_COLOR_YELLOW, M_COLOR_LIGHT_GRAY, M_COLOR_MAGENTA};
   const MIL_DOUBLE OperationValues[6] =   {M_NULL, M_NULL, M_NULL, M_NULL, 75, 90};
   MIL_CONST_TEXT_PTR OperationString[6] = {MIL_TEXT("Average"), 
                                            MIL_TEXT("Maximum"), 
                                            MIL_TEXT("Minimum"), 
                                            MIL_TEXT("Median"),
                                            MIL_TEXT("Rank"),
                                            MIL_TEXT("Rank percentile")};
   
   /* Execute projection operations. */
   for (MIL_INT Op = 0; Op < NumberOfOperations; ++Op)
      {
      /* Project image according to operation. */
      MimProjection(MilSubImage00, MilResultId, M_0_DEGREE, Operations[Op], OperationValues[Op]);

      /* Get projected result. */
      MimGetResult(MilResultId, M_VALUE + M_TYPE_DOUBLE, ProjectedValues);

      /* Adjust projected values for visualisation. */
      if (Operations[Op] == M_SUM)
         {
         for (MIL_INT ii = 0; ii < SizeX; ++ii)
            ProjectedValues[ii] = GraphSizeY - (ProjectedValues[ii] / (MIL_DOUBLE)SizeY);
         }
      else
         {
         for (MIL_INT ii = 0; ii < SizeX; ++ii)
            ProjectedValues[ii] = GraphSizeY - ProjectedValues[ii];
         }
      
      /* Select the color to use when drawing the projection. */
      MgraColor(M_DEFAULT, (MIL_DOUBLE)OperationColors[Op]);

      /* Draw the projection in the graphic list. */
      MgraLines(M_DEFAULT, MilGraphicList, SizeX, ColumnIndexes, ProjectedValues, M_NULL, M_NULL, M_POLYLINE);

      /* Identify projection. */
      MgraText(M_DEFAULT,MilOverlaySubImage, 10, 14 * (1 + Op), OperationString[Op]);
      }

   /* Draw the content of the graphic list in the image. */
   MgraDraw(MilGraphicList, MilOverlaySubImage, M_DEFAULT);
   
   /* Identify images. */
   MgraColor(M_DEFAULT, M_COLOR_WHITE);
   MgraText(M_DEFAULT, MilSubImage00, SizeX - 100, 0, MIL_TEXT("Source image"));
   MgraText(M_DEFAULT, MilSubImage01, SizeX - 100, 0, MIL_TEXT("Projections"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("Several projections of an image have been performed.\n")
             MIL_TEXT("The projection results are drawn as follows:\n")
             MIL_TEXT("\tThe average         of each column has been drawn in dark cyan.\n")
             MIL_TEXT("\tThe maximum         of each column has been drawn in green.\n")
             MIL_TEXT("\tThe minimum         of each column has been drawn in red.\n")
             MIL_TEXT("\tThe median          of each column has been drawn in yellow.\n")
             MIL_TEXT("\tThe rank            of each column has been drawn in light gray.\n")
             MIL_TEXT("\tThe rank percentile of each column has been drawn in magenta.\n")
   );
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free buffers. */
   MbufFree(MilOverlaySubImage);
   MbufFree(MilSubImage00);
   MbufFree(MilSubImage01);
   MbufFree(MilImage);
   MimFree(MilResultId);
   MgraFree(MilGraphicList);

   delete[] ProjectedValues;
   delete[] ColumnIndexes;   
   }
