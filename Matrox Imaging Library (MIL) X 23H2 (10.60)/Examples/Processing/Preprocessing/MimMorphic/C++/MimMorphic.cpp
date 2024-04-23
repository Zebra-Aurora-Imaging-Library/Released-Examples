﻿/*******************************************************************************/
/* 
 * File name: MimMorphic.cpp
 *
 * Synopsis:  This program shows the use of multiple custom structuring
 *            elements to perform a morphological operation.
 *            This example also demonstrates how to use MIL_UNIQUE_ID.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#include <math.h>

/* Source image files. */
#define IMAGE_FILE_CONNECTOR    M_IMAGE_PATH MIL_TEXT("Preprocessing/Connector.tif")
#define IMAGE_FILE_DEPTHMAP     M_IMAGE_PATH MIL_TEXT("Preprocessing/DepthMap.mim")

/* Structuring elements information. */
const MIL_INT STRUCT_ELEM_WIDTH  = 5,   /* Width of the rectangular structuring element.*/
              STRUCT_ELEM_HEIGHT = 1,   /* Height of the rectangular structuring element.*/
              STRUCT_ELEM_SIZE   = 5,   /* Size of the custom structuring element.*/
              STRUCT_ELEM_RADIUS = 5,   /* Radius of the circular and spheric structuring element.*/
              STRUCT_ELEM_DEPTH  = 32;  /* Depth of the structuring elements.*/
              
/* Custom structuring element data. */
const MIL_INT32  WEIGHTED_STRUCT_ELEM_ARR[STRUCT_ELEM_SIZE][STRUCT_ELEM_SIZE] =
   {{ M_DONT_CARE, 0, 0, 0, M_DONT_CARE },
     {           0, 2, 1, 2, 0 },
     {           0, 1, 3, 1, 0 },
     {           0, 2, 1, 2, 0 },
     { M_DONT_CARE, 0, 0, 0, M_DONT_CARE }
   };                                  

/* Number of morphological iterations. */
const MIL_INT ITERATIONS = 3;

//**********************************
// Header function 
//**********************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("MimMorphic\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program shows the use of multiple custom structuring\n")
             MIL_TEXT("elements to perform morphological operations.\n")
             MIL_TEXT("This example also demonstrates how to use MIL_UNIQUE_ID.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("image processing.\n\n"));
   }

/*********************************************/
/* Structuring elements allocation functions */ 
/*********************************************/
MIL_UNIQUE_BUF_ID AllocateCustomStructElem(MIL_ID MilSystem);
MIL_UNIQUE_BUF_ID AllocateRectangularStructElem(MIL_ID MilSystem);
MIL_UNIQUE_BUF_ID AllocateCircularStructElem(MIL_ID MilSystem);
MIL_UNIQUE_BUF_ID AllocateSphericStructElem(MIL_ID MilSystem);

int MosMain(void)
   {

   MIL_UNIQUE_APP_ID    MilApplication;          /* Application identifier.  */
   MIL_UNIQUE_SYS_ID    MilSystem;               /* System identifier.       */
   MIL_UNIQUE_DISP_ID   MilDisplay;              /* Display identifier.      */
   MIL_INT              SizeX, SizeY, Type;      /* Dimensions and Type of the source image.*/

   /* Allocate defaults. */
   MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);

   /* Print Header. */
   PrintHeader();

   /* Restore source image in an image buffer and display it. */
   auto MilSrcImgConnector = MbufRestore(IMAGE_FILE_CONNECTOR, MilSystem, M_UNIQUE_ID);
   MdispSelect(MilDisplay, MilSrcImgConnector);

   /* Inquire the image dimensions. */
   MbufInquire(MilSrcImgConnector, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImgConnector, M_SIZE_Y, &SizeY);
   MbufInquire(MilSrcImgConnector, M_TYPE, &Type);

   /* Allocate destination image buffer to store results. */
   auto MilDstImgConnector = MbufAlloc2d(MilSystem, SizeX, SizeY, Type,
                                         M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);

   MosPrintf(MIL_TEXT("Erosion operation using custom structuring elements:\n"));
   MosPrintf(MIL_TEXT("---------------------------------------------------\n"));

   /* Pause to show the first image. */
   MosPrintf(MIL_TEXT("Image of an object has been restored.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /*----------------Grayscale Erosion Using Custom Structuring Element------------------*/
   /* Allocate a custom structuring element. */
   auto MilCustomStructElem = AllocateCustomStructElem(MilSystem);

   /* Perform erosion. */ 
   MimMorphic(MilSrcImgConnector, MilDstImgConnector, MilCustomStructElem,
              M_ERODE, ITERATIONS, M_GRAYSCALE);

   /* Display the resulting image. */
   MdispSelect(MilDisplay, MilDstImgConnector);

   MosPrintf(MIL_TEXT("An erosion operation has been applied to the source\n"));
   MosPrintf(MIL_TEXT("image using a 5x5 weighted structuring element.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /*-----------------Grayscale Erosion Using Horizontal Structuring Element--------------*/

   /* Allocate a horizontal structuring element. */
   auto MilHorizontalStructElem = AllocateRectangularStructElem(MilSystem);

   /* Perform erosion. */
   MimMorphic(MilSrcImgConnector, MilDstImgConnector, MilHorizontalStructElem,
              M_ERODE, ITERATIONS, M_GRAYSCALE);

   /* Display the resulting image. */
   MdispSelect(MilDisplay, MilDstImgConnector);

   MosPrintf(MIL_TEXT("An erosion operation has been applied to the source\n"));
   MosPrintf(MIL_TEXT("image using a horizontal structuring element.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Closing operation using custom structuring elements:\n"));
   MosPrintf(MIL_TEXT("---------------------------------------------------\n"));

   /* Restore source image in an image buffer and display it. */
   auto MilSrcImgDepthMap = MbufRestore(IMAGE_FILE_DEPTHMAP, MilSystem, M_UNIQUE_ID);

   /* Display new image. */
   MdispControl(MilDisplay, M_VIEW_MODE, M_AUTO_SCALE);
   MdispSelect(MilDisplay, MilSrcImgDepthMap);
   MosPrintf(MIL_TEXT("Depth map of a surface has been restored.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Inquire the image dimensions. */
   MbufInquire(MilSrcImgDepthMap, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImgDepthMap, M_SIZE_Y, &SizeY);
   MbufInquire(MilSrcImgDepthMap, M_TYPE, &Type);

   /* Allocate a 16-bit desination to prevent saturation. */
   auto MilDstImgDepthMap = MbufAlloc2d(MilSystem, SizeX, SizeY, 16 + M_UNSIGNED,
                                         M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);

   /*-------------------Grayscale Closing Using Circular Structuring Element--------------*/
   /* Allocate a circular structuring element. */
   auto MilCircularStructElem = AllocateCircularStructElem(MilSystem);

   /* Perform closing. */
   MbufClear(MilDstImgDepthMap, 0);
   MimMorphic(MilSrcImgDepthMap, MilDstImgDepthMap, MilCircularStructElem, M_CLOSE, ITERATIONS,
              M_GRAYSCALE);

   /* Display the resulting image. */
   MdispSelect(MilDisplay, MilDstImgDepthMap);

   MosPrintf(MIL_TEXT("A closing operation has been applied to the source\n"));
   MosPrintf(MIL_TEXT("image using a circular structuring element.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /*-----------------------Grayscale Closing Using Spheric Structuring Element-----------*/

   /* Allocate a spheric structuring element. */
   auto MilSphericStructElem = AllocateSphericStructElem(MilSystem);

   /* Perform closing. */
   MbufClear(MilDstImgDepthMap, 0);
   MimMorphic(MilSrcImgDepthMap, MilDstImgDepthMap, MilSphericStructElem, M_CLOSE, ITERATIONS,
              M_GRAYSCALE);

   /* Display the resulting image. */
   MdispSelect(MilDisplay, MilDstImgDepthMap);

   MosPrintf(MIL_TEXT("A closing operation has been applied to the source\n"));
   MosPrintf(MIL_TEXT("image using a spheric structuring element (Rolling Ball).\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to finish.\n\n"));
   MosGetch();

   /* No free needed. */
   return 0;
   }

   /*****************************************************************************************
   *
   * Name              : AllocateCustomStructElem
   *
   * Synopsis          : Creates a custom weighted structuring element from an array
   *
   * Parameters        :  - MilSystem    System on which to allocate the structuring element
   *
   * Return            :  Weighted structurig element (Type: MIL_UNIQUE_BUF_ID)
   ******************************************************************************************/
   MIL_UNIQUE_BUF_ID AllocateCustomStructElem(MIL_ID MilSystem)
      {
      /* Allocate a structuring element. */
      auto MilCustomStructElem = MbufAlloc2d(MilSystem, STRUCT_ELEM_SIZE, STRUCT_ELEM_SIZE,
                                             STRUCT_ELEM_DEPTH + M_SIGNED, M_STRUCT_ELEMENT, M_UNIQUE_ID);

      /* Load buffer with data. */
      MbufPut2d(MilCustomStructElem, 0L, 0L, STRUCT_ELEM_SIZE, STRUCT_ELEM_SIZE,
                WEIGHTED_STRUCT_ELEM_ARR);

      return MilCustomStructElem;
      }

   /*****************************************************************************************
   *
   * Name              : AllocateRectangularStructElem
   *
   * Synopsis          : Creates a rectangular structuring element
   *
   * Parameters        :  - MilSystem    System on which to allocate the structuring element 
   *
   * Return            :  Rectangular structurig element (Type: MIL_UNIQUE_BUF_ID)
   ******************************************************************************************/
   MIL_UNIQUE_BUF_ID AllocateRectangularStructElem(MIL_ID MilSystem)
      {
      /* Allocate a structuring element. */
      auto MilRectangularStructElem = MbufAlloc2d(MilSystem, STRUCT_ELEM_WIDTH, STRUCT_ELEM_HEIGHT,
                                                  STRUCT_ELEM_DEPTH + M_UNSIGNED, M_STRUCT_ELEMENT,
                                                  M_UNIQUE_ID);

      /* Clear all the elements to 0. */
      MbufClear(MilRectangularStructElem, 0);

      return MilRectangularStructElem;
      }

   /*****************************************************************************************
   *
   * Name            : AllocateCircularStructElem
   *
   * Synopsis        : Creates a circular structuring element
   *
   * Parameters      : - MilSystem    System on which to allocate the structuring element
   *
   * Return          : Circular structurig element (Type: MIL_UNIQUE_BUF_ID)
   *
   * Comments        : Equation of a circle centered at 0: x^2 + y^2 = r^2
   *                   hence: if x^2 + y^2 =< CircleRadius * CircleRadius z = 0 else 
   *                   z = M_DONT_CARE
   *
   ******************************************************************************************/
   MIL_UNIQUE_BUF_ID AllocateCircularStructElem(MIL_ID MilSystem)
      {
      /* Size of the structuring element. */
      MIL_INT  BufSize = (2 * STRUCT_ELEM_RADIUS) + 1;
      /* Memory array to hold the data for the the structuring element. */
      MIL_UINT32  *CircleData = new MIL_UINT32[BufSize*BufSize];
      /* Index used to fill the memory array. */
      MIL_INT  DataIndex = 0;
      /* Structuring element identifier. */
      MIL_ID   MilCircularStruct = M_NULL;
      /* Memory array to hold squared radius value */
      MIL_INT  SquaredCircleRadius = STRUCT_ELEM_RADIUS * STRUCT_ELEM_RADIUS;
      /* Memory array to hold squared y value */
      MIL_INT  SquaredY = 0;

      /* Fill the circular structuring element. */
      for(MIL_INT y = -STRUCT_ELEM_RADIUS; y <= STRUCT_ELEM_RADIUS; y++)
         {
         SquaredY = y * y;
         for(MIL_INT x = -STRUCT_ELEM_RADIUS; x <= STRUCT_ELEM_RADIUS; x++)
            {
            /* If x^2 + y^2 is bigger than the squared radius, the coordinates fall outside 
             the circle. */
            if(x * x + SquaredY <= SquaredCircleRadius)
               {
               CircleData[DataIndex] = 0;
               }
            else
               CircleData[DataIndex] = M_DONT_CARE;

            DataIndex++;
            }
         }

      /* Allocate the structuring element. */
      auto MilCircularStructElem = MbufAlloc2d(MilSystem, BufSize, BufSize,
                                               STRUCT_ELEM_DEPTH + M_UNSIGNED,
                                               M_STRUCT_ELEMENT, M_UNIQUE_ID);

      /* Put the data in the structuring element buffer. */
      MbufPut2d(MilCircularStructElem, 0, 0, BufSize, BufSize, CircleData);

      /* Free the memory */
      delete[] CircleData;

      return MilCircularStructElem;
      }

   /*****************************************************************************************
   *
   * Name              : AllocateSphericStructElem
   *
   * Synopsis          : Creates a spheric structuring element
   *
   * Parameters        :  - MilSystem    System on which to allocate the structuring element  
   *
   * Return            :  Spheric structurig element (Type: MIL_UNIQUE_BUF_ID)
   *
   * Comments          : Equation of a sphere centered at 0: x^2 + y^2 + z^2 = r^2
   *                                                  hence: z = sqrt(r^2 - x^2 - y^2)
   *
   ******************************************************************************************/
   MIL_UNIQUE_BUF_ID AllocateSphericStructElem(MIL_ID MilSystem)
      {
      /* Size of the structuring element */
      MIL_INT     BufSize = (2 * STRUCT_ELEM_RADIUS) + 1;
      /* Memory array to hold the data for the the structuring element. */
      MIL_UINT32  *SphereData = new MIL_UINT32[BufSize * BufSize];
      /* Index used to fill the memory array */
      MIL_INT     DataIndex = 0;
      /* Holds the value r^2 */
      MIL_INT     SquaredRadius = STRUCT_ELEM_RADIUS * STRUCT_ELEM_RADIUS;
      /* Holds the value r^2 - x^2 - y^2 */
      MIL_INT     SquaredZ = 0;
      /* Memory array to hold squared y value */
      MIL_INT     SquaredY = 0;

      /* Compute the z values of the sphere. */
      for(MIL_INT y = -STRUCT_ELEM_RADIUS; y <= STRUCT_ELEM_RADIUS; y++)
         {
         SquaredY = y * y;
         for(MIL_INT x = -STRUCT_ELEM_RADIUS; x <= STRUCT_ELEM_RADIUS; x++)
            {
            SquaredZ = SquaredRadius - SquaredY - x * x;

            /* If the difference is negative, the coordinates fall outside the sphere. */
            if(SquaredZ >= 0)
               {
               /* Round the value to the closest integer value. */
               SphereData[DataIndex] = (MIL_UINT32)(sqrt(SquaredZ) + 0.5);
               }
            else
               SphereData[DataIndex] = M_DONT_CARE;

            DataIndex++;
            }
         }

      /* Allocate the structuring element. */
      auto MilSphericStructElem = MbufAlloc2d(MilSystem, BufSize, BufSize,
                                              STRUCT_ELEM_DEPTH + M_UNSIGNED,
                                              M_STRUCT_ELEMENT, M_UNIQUE_ID);

      /* Put the data in the structuring element buffer. */
      MbufPut2d(MilSphericStructElem, 0, 0, BufSize, BufSize, SphereData);

      /* Free the memory. */
      delete[] SphereData;

      return MilSphericStructElem;
      }
