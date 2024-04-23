﻿//***************************************************************************************
// 
// File name: MetrologyOnBlobs.cpp  
//
// Synopsis: This example demonstrates metrology operations along blob contours.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("MetrologyOnBlobs\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates metrology operations along blob contours.\n"));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, system, display, buffer, graphic,\n")
             MIL_TEXT("image processing, blob, and metrology.\n"));
   }

// Macro defining the example's file path.
#define IMAGE_FILENAME "OcrImage.mim"
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT(x))

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   MIL_ID MilApplication,
          MilSystem,
          MilImage,
          MilDisplayImage,
          GraphicListImage;

   MIL_ID MilBlobContext,
          MilBlobResult;

   MIL_ID MetContext,
          MetResult;

   MIL_INT ii,
           SizeX,
           SizeY,
           NumberOfBlobs,
           NumberOfChainedPixels;

   MIL_INT* LabelArray;

   MIL_DOUBLE *ChainXArray,
              *ChainYArray,
              *CogXArray,
              *CogYArray;

   /* Allocate the MIL application. */
   MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);

   /* Allocate the MIL system. */
   MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   
   /* Load the source image of multiple profiles obtained using, 
      for example, Coherent StingRay structured light lasers. */
   MilImage = MbufRestore(EX_PATH(IMAGE_FILENAME), MilSystem, M_NULL);

   /* Retrieving the source image sizes. */
   SizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   SizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   /* Display the source image. */
   MilDisplayImage = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   
   GraphicListImage = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   MdispControl(MilDisplayImage, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicListImage);
   
   MdispSelect(MilDisplayImage, MilImage);

   MosPrintf(MIL_TEXT("\n\nA source image has been loaded and is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Binarize the source image. */
   MimBinarize(MilImage, MilImage, M_BIMODAL + M_GREATER, M_NULL, M_NULL);

   /* Allocate the blob context and result. */
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobResult);

   /* Set the blob features to calculate. */
   MblobControl(MilBlobContext, M_RECTANGULARITY, M_ENABLE);
   MblobControl(MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);
   MblobControl(MilBlobContext, M_CHAINS, M_ENABLE);
   
   /* Calculate the blobs. */
   MblobCalculate(MilBlobContext, MilImage, M_NULL, MilBlobResult);

   /* Select large rectangular blobs only. */
   MblobSelect(MilBlobResult, M_EXCLUDE, M_AREA, M_LESS, 500, M_NULL);
   MblobSelect(MilBlobResult, M_EXCLUDE, M_RECTANGULARITY, M_LESS, 0.9, M_NULL);

   /* Draw the selected blobs. */
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MblobDraw(M_DEFAULT, MilBlobResult, GraphicListImage, M_DRAW_BLOBS, M_INCLUDED_BLOBS, M_DEFAULT);

   MosPrintf(MIL_TEXT("The source image has been segmented.\n")
             MIL_TEXT("The resulting blobs have been calculated and\n")
             MIL_TEXT("the rectangular ones have been selected.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Retrieve the number of selected blobs. */
   MblobGetResult(MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumberOfBlobs);

   /* Retrieve the blob results. */
   LabelArray = new MIL_INT[NumberOfBlobs];
   MblobGetResult(MilBlobResult, M_DEFAULT, M_LABEL_VALUE + M_TYPE_MIL_INT, LabelArray);

   CogXArray = new MIL_DOUBLE[NumberOfBlobs];
   CogYArray = new MIL_DOUBLE[NumberOfBlobs];
   MblobGetResult(MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_X + M_BINARY + M_TYPE_MIL_DOUBLE, CogXArray);
   MblobGetResult(MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_Y + M_BINARY + M_TYPE_MIL_DOUBLE, CogYArray);

   /* Allocate the Metrology context and result. */
   MetContext = MmetAlloc(MilSystem, M_DEFAULT, M_NULL);
   MetResult = MmetAllocResult(MilSystem, M_DEFAULT, M_NULL);

   /* Add an external feature. */
   const MIL_INT CurrentProfileLbl = 100;
   MmetAddFeature(MetContext, M_CONSTRUCTED, M_EDGEL, CurrentProfileLbl, M_EXTERNAL_FEATURE, M_NULL, M_NULL, 0, M_DEFAULT);

   /* Retrieve the blob contours and add it to the Metrology external feature. */
   for (ii = 0; ii < NumberOfBlobs; ii++)
      {
      /* Get the number of chained pixels. */
      MblobGetResult(MilBlobResult, LabelArray[ii], M_NUMBER_OF_CHAINED_PIXELS + M_TYPE_MIL_INT, &NumberOfChainedPixels);

      /* Retrieve the contour pixel coordinates. */
      ChainXArray = new MIL_DOUBLE[NumberOfChainedPixels];
      ChainYArray = new MIL_DOUBLE[NumberOfChainedPixels];

      MblobGetResult(MilBlobResult, LabelArray[ii], M_CHAIN_X + M_TYPE_DOUBLE, ChainXArray);
      MblobGetResult(MilBlobResult, LabelArray[ii], M_CHAIN_Y + M_TYPE_DOUBLE, ChainYArray);

      /* Add the blob contour to the Metrology external feature. */
      MmetPut(MetContext, M_FEATURE_LABEL(CurrentProfileLbl), NumberOfChainedPixels, M_NULL, ChainXArray, ChainYArray, M_NULL, M_NULL, M_INTERPOLATE_ANGLE);

      delete[] ChainXArray;
      delete[] ChainYArray;

      /* Set Metrology measures relative to the new accumulated blob chain. */
      MIL_INT NewLabelOdd = CurrentProfileLbl + 2*ii + 1;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_SEGMENT, NewLabelOdd, M_FIT, &CurrentProfileLbl, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_LABEL(NewLabelOdd), M_DEFAULT, M_RECTANGLE, CogXArray[ii] - 35, CogYArray[ii] - 30, 70, 20, 0, M_NULL);
      MmetControl(MetContext, M_FEATURE_LABEL(NewLabelOdd), M_EDGEL_RELATIVE_ANGLE, M_SAME_OR_REVERSE);
      MmetControl(MetContext, M_FEATURE_LABEL(NewLabelOdd), M_EDGEL_ANGLE_RANGE, 90);

      MIL_INT NewLabelEven = CurrentProfileLbl + 2*ii + 2;
      MmetAddFeature(MetContext, M_CONSTRUCTED, M_SEGMENT, NewLabelEven, M_FIT, &CurrentProfileLbl, M_NULL, 1, M_DEFAULT);
      MmetSetRegion(MetContext, M_FEATURE_LABEL(NewLabelEven), M_DEFAULT, M_RECTANGLE, CogXArray[ii] + 35, CogYArray[ii] + 30, 70, 20, 180, M_NULL);
      MmetControl(MetContext, M_FEATURE_LABEL(NewLabelEven), M_EDGEL_RELATIVE_ANGLE, M_SAME_OR_REVERSE);
      MmetControl(MetContext, M_FEATURE_LABEL(NewLabelEven), M_EDGEL_ANGLE_RANGE, 90);

      MIL_INT ParaLabels[2] = { NewLabelOdd, NewLabelEven };
      MmetAddTolerance(MetContext, M_PARALLELISM, M_DEFAULT, 0.0, 2.0, ParaLabels, M_NULL, 2, M_DEFAULT);
      }
   MosPrintf(MIL_TEXT("The blob contours have been retrieved and added\n")
             MIL_TEXT("to a Metrology context as an external feature.\n"));

   /* Perform the Metrology calculation. */
   MmetCalculate(MetContext, M_NULL, MetResult, M_DEFAULT);

   /* Display the Metrology regions and features. */
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MmetDraw(M_DEFAULT, MetResult, GraphicListImage, M_DRAW_REGION, M_DEFAULT, M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_RED);
   for (ii = 0; ii < 2*NumberOfBlobs; ii++)
      MmetDraw(M_DEFAULT, MetResult, GraphicListImage, M_DRAW_FEATURE, M_FEATURE_INDEX(2+ii), M_DEFAULT);

   /* Display the Metrology tolerances. */
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MmetDraw(M_DEFAULT, MetResult, GraphicListImage, M_DRAW_TOLERANCE, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("\n\nThe Metrology context has been calculated to determine the\n")
             MIL_TEXT("parallelism of the top and bottom edges of the rectangular blobs.\n")
             MIL_TEXT("The parallelism measures are retrieved and displayed:\n\n"));

   MIL_DOUBLE ParallelismValue = 0.0;
   for (ii = 0; ii < NumberOfBlobs; ii++)
      {
      MmetGetResult(MetResult, M_TOLERANCE_INDEX(ii), M_TOLERANCE_VALUE + M_TYPE_DOUBLE, &ParallelismValue);
      MosPrintf(MIL_TEXT("\t- blob %d: %.2f degrees.\n"), ii, ParallelismValue);
      }

   MosPrintf(MIL_TEXT("\nPress <Enter> to terminate.\n\n"));
   MosGetch();

   // Release allocated resources.
   MbufFree(MilImage);
   MdispFree(MilDisplayImage);
   MgraFree(GraphicListImage);

   MblobFree(MilBlobContext);
   MblobFree(MilBlobResult);

   MmetFree(MetContext);
   MmetFree(MetResult);

   // Free the MIL system and application.
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

