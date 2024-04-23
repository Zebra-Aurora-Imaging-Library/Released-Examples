﻿/*******************************************************************************/
/*
/* File name: UnwarpMultiviewCylinder.cpp
/*
/* Synopsis:  This program shows how to unwarp multiple views to straighten a fixed
/*            cylinder into a single flattened image using the calibration module.
/*            The straightened views are optionally registered and smoothly rendered
/*            using the registration tool.
/*
/* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
/* All Rights Reserved
*/

#include <mil.h>

/* Example functions declaration. */
void UnwarpImage(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilSourceImage, MIL_ID MilDestinationImage, 
                     MIL_DOUBLE CalibrationWorldOffsetX, MIL_ID MilUnwarpedImage, MIL_ID MilCalibration);

/* The number of cameras(views). */
#define CAMERA_NUMBER                     2

/* Source image files specification. */
#define EXAMPLE_IMAGE_PATH                M_IMAGE_PATH       MIL_TEXT("UnwarpMultiviewCylinder/")
#define RIGHT_GRID_IMAGE_FILE             EXAMPLE_IMAGE_PATH MIL_TEXT("RightGrid.mim")
#define LEFT_GRID_IMAGE_FILE              EXAMPLE_IMAGE_PATH MIL_TEXT("LeftGrid.mim")
#define Mask_IMAGE_FILE                   EXAMPLE_IMAGE_PATH MIL_TEXT("GridMask.mim")

#define RIGHT_IMAGE_FILE                  EXAMPLE_IMAGE_PATH MIL_TEXT("RightImage.mim")
#define LEFT_IMAGE_FILE                   EXAMPLE_IMAGE_PATH MIL_TEXT("LeftImage.mim")

/* World description of the calibration grid. */
#define GRID_OFFSET_X                     0
#define GRID_OFFSET_Y                     0
#define GRID_OFFSET_Z                     0
#define GRID_ROW_SPACING                  2.5 //unit: mm
#define GRID_COLUMN_SPACING               2.5 //unit: mm
#define GRID_ROW_NUMBER                   30
#define GRID_COLUMN_NUMBER                24

/* Specifies the scale between the world and pixel units */ 
/* in both X and Y direction in the destination buffer */
#define PIXEL_SIZE                        0.1

/* The pixel offset of the child grids and the child images. */
#define CHILD_OFFSET_X                    320
#define CHILD_OFFSET_Y                    55

/* The size of the destination result image in world units */
#define DESTINATION_GRID_NUM_X            40
#define DESTINATION_GRID_NUM_Y            29
#define DESTINATION_CHILD_GRID_NUM_X      23
#define OVERLAP_GRID_NUM_X                6
const MIL_DOUBLE StitchLineOffsetWorldX      =(DESTINATION_CHILD_GRID_NUM_X-OVERLAP_GRID_NUM_X)*GRID_COLUMN_SPACING;

/* The size of the destination result image in pixel units */
const MIL_INT DestnationImageSizeX     =(MIL_INT)((DESTINATION_GRID_NUM_X*GRID_ROW_SPACING)/PIXEL_SIZE);
const MIL_INT DestnationImageSizeY     =(MIL_INT)((DESTINATION_GRID_NUM_Y*GRID_COLUMN_SPACING)/PIXEL_SIZE);
const MIL_INT DestinationChildSizeX    =(MIL_INT)((DESTINATION_CHILD_GRID_NUM_X*GRID_COLUMN_SPACING)/PIXEL_SIZE);

/* Camera name array */
MIL_CONST_TEXT_PTR CAMERA[2] = {MIL_TEXT("right"), MIL_TEXT("left")};

/*****************************************************************************
 Example description.
*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("UnwarpMultiviewCylinder\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program shows how to unwarp multiple views to straighten a fixed\n")
             MIL_TEXT("cylinder into a single flattened image using the calibration module.\n")
             MIL_TEXT("The straightened views are optionally registered and smoothly rendered\n")
             MIL_TEXT("using the registration tool.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, graphics, buffer,\n")
             MIL_TEXT("image processing, calibration, registration.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

/* Main function. */
int MosMain(void)
   {
   MIL_ID   MilApplication,                  /* Application identifier.             */
            MilSystem,                       /* System Identifier.                  */
            MilDisplay,                      /* Display identifier.                 */
            MilGraphicList,                  /* Graphic list identifier.            */
            MilGridImage[CAMERA_NUMBER],     /* Grid image identifier arra.y        */
            MilCalibration[CAMERA_NUMBER],   /* Calibration identifier array.       */
            MilSourceImage[CAMERA_NUMBER],   /* Original image identifier array.    */
            MilGridMaskImage,                /* Grid mask image identifier.         */
            MilUnwarpedImage[CAMERA_NUMBER], /* The unwarped image identifier array.*/
            MilDestinationImage;             /* Destination result image identifier */

   MIL_INT  Result,                          /* Registration result variable.       */
            i;

   /* Allocations. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   PrintHeader();

   /* Allocate a graphic list to draw annotations. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);

   /* Restore the grid image grabbed by the right camera. */
   MbufRestore(RIGHT_GRID_IMAGE_FILE, MilSystem, MilGridImage);
   /* Restore the grid image grabbed by the left camera. */
   MbufRestore(LEFT_GRID_IMAGE_FILE, MilSystem, MilGridImage+1);

   /* Restore the mask image for the grid image. */
   MbufRestore(Mask_IMAGE_FILE, MilSystem, &MilGridMaskImage);

   /*****************************************************************************
    Calibrate each camera
   ******************************************************************************/
   for(i =0; i<CAMERA_NUMBER; i++)
      {
      /* Display the grid image */
      MdispSelect(MilDisplay, MilGridImage[i]);
      MosPrintf(MIL_TEXT("The cylindrical grid image grabbed by the %s camera is displayed.\n\n"), CAMERA[i]);
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      /* Mask the irrelevant areas in the grid image. */
      MimArith(MilGridImage[i], MilGridMaskImage, MilGridImage[i], M_AND);
      MosPrintf(MIL_TEXT("Irrelevant areas are masked out.\n"));

      /* Allocate a camera calibration context for each camera. */
      McalAlloc(MilSystem, M_DEFAULT, M_DEFAULT, MilCalibration+i);

      /* Calibrate the camera. */
      McalGrid(MilCalibration[i], MilGridImage[i], GRID_OFFSET_X, GRID_OFFSET_Y, GRID_OFFSET_Z,
                  GRID_ROW_NUMBER, GRID_COLUMN_NUMBER, GRID_ROW_SPACING, GRID_COLUMN_SPACING, M_DEFAULT, M_DEFAULT);

      if(McalInquire(MilCalibration[i], M_CALIBRATION_STATUS + M_TYPE_MIL_INT, M_NULL)==M_CALIBRATED)
         {
         /* Draw the world points on the grid. */
         McalDraw(M_DEFAULT, MilCalibration[i], MilGraphicList, M_DRAW_WORLD_POINTS, M_DEFAULT, M_DEFAULT);
         MosPrintf(MIL_TEXT("The %s camera has been calibrated.\n\n"), CAMERA[i]);
         MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
         MosGetch();
         /* Clear the overlay annotation. */
         MgraClear(M_DEFAULT, MilGraphicList);
         }
      }

   /* Restore the target image grabbed by the right camera. */
   MbufRestore(RIGHT_IMAGE_FILE, MilSystem, MilSourceImage);
   MbufRestore(LEFT_IMAGE_FILE, MilSystem, MilSourceImage+1);

   /* Allocate a destination image to store the final stitched images. */
   MbufAlloc2d(MilSystem,  DestnationImageSizeX,  DestnationImageSizeY, 8+M_UNSIGNED, 
                  M_IMAGE+M_PROC+M_DISP, &MilDestinationImage);
   MbufClear(MilDestinationImage, 0x0);

   /* Calibrate the destination image. */
   McalUniform(MilDestinationImage, 0, 0, PIXEL_SIZE, PIXEL_SIZE, 0, M_DEFAULT);

   /*****************************************************************************
    Unwarp each view into the flattened destination image.
   ******************************************************************************/
   for(i =0; i<CAMERA_NUMBER; i++)
      {
      /* Display the image of the product. */
      MdispSelect(MilDisplay, MilSourceImage[i]);
      MosPrintf(MIL_TEXT("An image of the cylindrical product is grabbed by\n")
                MIL_TEXT("the %s camera and displayed.\n\n"), CAMERA[i]);
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      /* Allocate images to store the unwarped images later for registration. */
      MbufAlloc2d(MilSystem, DestinationChildSizeX, DestnationImageSizeY, 8+M_UNSIGNED,
         M_IMAGE+M_PROC+M_DISP, MilUnwarpedImage+i);

      /* Unwarp the image into the flatted destination buffer. */
      UnwarpImage(MilSystem, MilDisplay, MilSourceImage[i], MilDestinationImage, 
                     StitchLineOffsetWorldX*i, MilUnwarpedImage[i], MilCalibration[i]);

      if(i>0)
         {
            MosPrintf(MIL_TEXT("The %s image is straightened and placed in the destination buffer\n"), CAMERA[i]);
            MosPrintf(MIL_TEXT("where it overlaps the straightened %s image to form\n"), CAMERA[i-1]);
            MosPrintf(MIL_TEXT("a single flattened image.\n\n"));
         }
      else
         {
         MosPrintf(MIL_TEXT("The %s image is straightened and placed in a destination buffer.\n\n"), CAMERA[0]);
         }
         
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }

   /* Allocate a registration context. */
   MIL_ID MilRegContext = MregAlloc(MilSystem, M_STITCHING, M_DEFAULT, M_NULL);

   /* Allocate a new empty registration result buffer. */
   MIL_ID MilRegResult = MregAllocResult(MilSystem, M_DEFAULT, M_NULL);

   /* Set the X offset of the images(except the first one) in the destination buffer. */
   MregControl(MilRegContext, M_ALL, M_OPTIMIZE_LOCATION, M_DISABLE);
   for(i =1; i<CAMERA_NUMBER; i++)
      MregSetLocation(MilRegContext, i, M_DEFAULT,M_POSITION_XY, (StitchLineOffsetWorldX/PIXEL_SIZE)*i, 
                        0, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Set the mosaic composing mode - fuse the images by progressively blending overlapping pixels. */
   MregControl(MilRegResult, M_GENERAL, M_MOSAIC_COMPOSITION, M_FUSION_IMAGE);

   /*  Register all the unwarped images. */
   MregCalculate(MilRegContext, MilUnwarpedImage, MilRegResult, CAMERA_NUMBER, M_DEFAULT);

   /* Verify if registration is successful. */
   MregGetResult(MilRegResult, M_GENERAL, M_RESULT + M_TYPE_MIL_INT, &Result);
   if(Result == M_SUCCESS)
      /* Compose the mosaic from the source images into a single flatter image. */
      MregTransformImage(MilRegResult, MilUnwarpedImage, MilDestinationImage, 
                           CAMERA_NUMBER, M_BILINEAR+M_OVERSCAN_CLEAR, M_DEFAULT);

   MosPrintf(MIL_TEXT("The result can be improved by fusing the two images in the destination buffer\n")
             MIL_TEXT("using the registration module.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to finish.\n\n"));
   MosGetch();

   /* Free allocations */
   MregFree(MilRegResult);
   MregFree(MilRegContext);

   /* Free allocations */
   for(i =0; i<CAMERA_NUMBER; i++)
      {
      MbufFree(MilUnwarpedImage[i]);
      MbufFree(MilSourceImage[i]);
      McalFree(MilCalibration[i]);
      MbufFree(MilGridImage[i]);
      }

   /* Free objects. */    
   MbufFree(MilDestinationImage);
   MbufFree(MilGridMaskImage);
   MgraFree(MilGraphicList);
   MdispFree(MilDisplay);

   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
   return 0;
   }

/*****************************************************************************
 Unwarp a view of a fixed cylinder into a single flattened image.
*****************************************************************************/
void UnwarpImage(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilSourceImage, MIL_ID MilDestinationImage, 
                     MIL_DOUBLE CalibrationWorldOffsetX, MIL_ID MilUnwarpedImage, MIL_ID MilCalibration)
{
   MIL_ID     MilDestChildImage;     /* The child in the destination result image. */

   /* Associate the calibration to the image of the product. */
   McalAssociate(MilCalibration, MilSourceImage, M_DEFAULT);

   /* Set the child buffer (in the destination buffer) where to unwarp the image into. */
   MbufChild2d(MilDestinationImage, (MIL_INT)(CalibrationWorldOffsetX/PIXEL_SIZE), 0, 
                  DestinationChildSizeX, DestnationImageSizeY, &MilDestChildImage);

   /*Sets the X-offset of a child buffer, relative to the calibrated destination buffer. */
   McalControl(MilDestinationImage, M_CALIBRATION_CHILD_OFFSET_X, -CalibrationWorldOffsetX/PIXEL_SIZE);

   /* Unwarp each image into a big flatter destination buffer, overlapping the previous one. */
   McalTransformImage(MilSourceImage, MilDestChildImage, MilCalibration, M_BILINEAR + M_OVERSCAN_DISABLE, 
                        M_DEFAULT, M_WARP_IMAGE+M_USE_DESTINATION_CALIBRATION);
   MdispSelect(MilDisplay, MilDestinationImage);

   /* Store the unwarped images for registration. */
   MbufCopy(MilDestChildImage, MilUnwarpedImage);

   /* Free allocations */
   MbufFree(MilDestChildImage);
}

