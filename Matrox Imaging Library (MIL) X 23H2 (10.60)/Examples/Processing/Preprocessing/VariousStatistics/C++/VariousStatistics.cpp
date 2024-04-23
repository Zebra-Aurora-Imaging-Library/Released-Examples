﻿/************************************************************************************/
/*
* File name: VariousStatistics.cpp
*
* Synopsis:  This example demonstrates how to calculate various statistics using MIL.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

/* Image names and paths. */
#define SIMPLESTAT_IMAGE_FILE               M_IMAGE_PATH       MIL_TEXT("Rotwafer.mim")
#define CUMULATIVESTAT_IMAGE_FILE           M_IMAGE_PATH       MIL_TEXT("LargeWafer.mim")
#define WINDOWSTAT_IMAGE_FILE               M_IMAGE_PATH       MIL_TEXT("Preprocessing/DefectiveFabric.tif")

/* Util constant */
#define MIN_STANDARD_DEVIATION 10

/* Examples. */
void BasicStats(MIL_ID MilSystem, MIL_ID MillDisplay);
void CumulativeStats(MIL_ID MilSystem, MIL_ID MillDisplay);
void MovingWindowStats(MIL_ID MilSystem, MIL_ID MillDisplay);

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("VariousStatistics\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to calculate various statistics using MIL.\n\n")
             
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, buffer, display, graphics,\n")
             MIL_TEXT("image processing, pattern matching, system.\n\n"));
   }

int MosMain()
   {
   MIL_ID   MilApplication,         /* Application identifier.             */
            MilSystem,              /* System identifier.                  */
            MilDisplay,             /* Display identifier.                 */
            MilGraphicList;         /* MIL graphicList buffer identifier.  */

   PrintHeader();
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Allocate a graphic list to hold the sub pixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);
   
   MosPrintf(MIL_TEXT("Example 1 - Basic statistics with conditions.\n"));
   MosPrintf(MIL_TEXT("---------------------------------------------\n\n"));
   BasicStats(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("Example 2 - Using cumulative statistics to define a better model.\n"));
   MosPrintf(MIL_TEXT("-----------------------------------------------------------------\n\n"));
   CumulativeStats(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("Example 3 - Texture statistics applied to defect detection.\n"));
   MosPrintf(MIL_TEXT("-----------------------------------------------------------\n\n"));
   MovingWindowStats(MilSystem, MilDisplay);

   /* Free objects. */
   MgraFree(MilGraphicList);
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
   return 0;
   }

void BasicStats(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   /* Load the source image to display. */
   MIL_ID MilImage = MbufRestore(SIMPLESTAT_IMAGE_FILE, MilSystem, M_NULL);
   MdispSelect(MilDisplay, MilImage);
      
   /* Allocate the statistic context and result buffer. */
   MIL_ID MilStatContext = MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_NULL);
   MIL_ID MilStatResult = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);

   /* Enable the statistics for extremes and calculate. */
   MimControl(MilStatContext, M_STAT_MIN, M_ENABLE);
   MimControl(MilStatContext, M_STAT_MAX, M_ENABLE);
   MimStatCalculate(MilStatContext, MilImage, MilStatResult, M_DEFAULT);

   /* Get extremes. */
   MIL_DOUBLE StatMinVal, StatMaxVal, StatMeanVal;
   MimGetResult(MilStatResult, M_STAT_MIN, &StatMinVal);
   MimGetResult(MilStatResult, M_STAT_MAX, &StatMaxVal);
   
   /* Enable the statistics for the mean value*/
   MimControl(MilStatContext, M_STAT_MIN, M_DISABLE);
   MimControl(MilStatContext, M_STAT_MAX, M_DISABLE);
   MimControl(MilStatContext, M_STAT_MEAN, M_ENABLE);

   /* Set the condition to exclude the extremes. */
   MimControl(MilStatContext, M_CONDITION, M_IN_RANGE);
   MimControl(MilStatContext, M_COND_LOW, StatMinVal +1);
   MimControl(MilStatContext, M_COND_HIGH, StatMaxVal -1);

   /* Calculate, then get the mean value. */
   MimStatCalculate(MilStatContext, MilImage, MilStatResult, M_DEFAULT);
   MimGetResult(MilStatResult, M_STAT_MEAN, &StatMeanVal);

   /* Print out the statistic results. */
   MosPrintf(MIL_TEXT("Global image statistics:\n")
             MIL_TEXT("------------------------\n"));
   MosPrintf(MIL_TEXT("The minimum pixel value is %.2f.\n"), StatMinVal);
   MosPrintf(MIL_TEXT("The maximum pixel value is %.2f.\n"), StatMaxVal);
   MosPrintf(MIL_TEXT("Excluding the extreme values, the image's average pixel value is %.2f.\n"), StatMeanVal);

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MimFree(MilStatResult);
   MimFree(MilStatContext);
   MbufFree(MilImage);
   }

#define CUMULATIVESTAT_MODEL_OFF_X  36
#define CUMULATIVESTAT_MODEL_OFF_Y  66
#define CUMULATIVESTAT_MODEL_SIZE_X 350
#define CUMULATIVESTAT_MODEL_SIZE_Y 350
#define MAX_OCCURRENCE_NUM          10

void CumulativeStats(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID   MilGraphicList,            /* Graphics list for annotations.   */
            MilImage,                  /* Image buffer identifier.         */
            MilChildImage,             /* Moving child buffer identifier.  */
            MilStatImage,              /* Statistic result image buffer.   */
            MilStatCumulativeContext,  /* MIL identifier of the cumulative statistic context buffer.  */
            MilStatCumulativeResult;   /* MIL identifier of the statistic result buffer.              */

   MIL_INT  NumResults, i;             /* Number of results found.         */

   MIL_DOUBLE  AnnotationColor = M_COLOR_GREEN,  
               MaskColor       = M_COLOR_RED,
               ScoreArray[MAX_OCCURRENCE_NUM]={0}, 
               PosXArray[MAX_OCCURRENCE_NUM] ={0}, 
               PosYArray[MAX_OCCURRENCE_NUM] ={0};

   /* Load the source image to display. */
   MbufRestore(CUMULATIVESTAT_IMAGE_FILE, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);
   MdispInquire(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, &MilGraphicList);

   /* Allocate a normalized pattern matching context. */
   MIL_ID MilPatContxt = MpatAlloc(MilSystem, M_NORMALIZED, M_DEFAULT, M_NULL);

   MosPrintf(MIL_TEXT("Define the pattern matching model using the top-left occurrence.\n\n"));

   /* Define a regular model using the top-left occurrence. */
   MpatDefine(MilPatContxt, M_REGULAR_MODEL, MilImage, CUMULATIVESTAT_MODEL_OFF_X, CUMULATIVESTAT_MODEL_OFF_Y, CUMULATIVESTAT_MODEL_SIZE_X, CUMULATIVESTAT_MODEL_SIZE_Y, M_DEFAULT);

   /* Move the reference to (0,0) in the model, and search all occurrences. */
   MpatControl(MilPatContxt, M_DEFAULT, M_NUMBER, M_ALL);
   MpatControl(MilPatContxt, M_DEFAULT, M_REFERENCE_X, 0);
   MpatControl(MilPatContxt, M_DEFAULT, M_REFERENCE_Y, 0);

   /* Preprocess the model. */
   MpatPreprocess(MilPatContxt, M_DEFAULT, MilImage);

   /* Draw a box around the model in the model image. */
   MgraColor(M_DEFAULT, AnnotationColor);
   MpatDraw(M_DEFAULT, MilPatContxt, MilGraphicList, M_DRAW_BOX, M_DEFAULT, M_ORIGINAL);
   
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Find other model occurrences.\n"));

   /* Allocate pattern matching result and search. */
   MIL_ID MilPatResult = MpatAllocResult(MilSystem, M_DEFAULT, M_NULL);
   MpatFind(MilPatContxt, MilImage, MilPatResult);

   /* Retrieve the number of occurrences, position and scores. */
   MpatGetResult(MilPatResult, M_GENERAL, M_NUMBER+M_TYPE_MIL_INT, &NumResults);
   /* Retrieve results and draw result annotations. */
   MpatGetResult(MilPatResult, M_DEFAULT, M_POSITION_X, PosXArray);
   MpatGetResult(MilPatResult, M_DEFAULT, M_POSITION_Y, PosYArray);
   MpatGetResult(MilPatResult, M_DEFAULT, M_SCORE, ScoreArray);

   MgraColor(M_DEFAULT, AnnotationColor);
   MpatDraw(M_DEFAULT, MilPatResult, MilGraphicList, M_DRAW_BOX,M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("- %d occurrences found\n\n"), NumResults);
   
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Calculate the average and the deviation statistic images using the\n%d occurrences.\n"), NumResults);
      
   /* Allocate a result image buffer. */
   MbufAlloc2d(MilSystem, CUMULATIVESTAT_MODEL_SIZE_X, CUMULATIVESTAT_MODEL_SIZE_Y, 8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, &MilStatImage);

   /* Allocate cumulative statistic buffers. */
   MilStatCumulativeContext = MimAlloc(MilSystem, M_STATISTICS_CUMULATIVE_CONTEXT, M_DEFAULT, M_NULL);
   MilStatCumulativeResult  = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);

   /* Enable statistics and define the size of the images. */
   MimControl(MilStatCumulativeContext, M_STAT_MEAN, M_ENABLE);
   MimControl(MilStatCumulativeContext, M_STAT_STANDARD_DEVIATION, M_ENABLE);
   MimControl(MilStatCumulativeContext, M_SOURCE_SIZE_X, CUMULATIVESTAT_MODEL_SIZE_X);
   MimControl(MilStatCumulativeContext, M_SOURCE_SIZE_Y, CUMULATIVESTAT_MODEL_SIZE_Y);

   /* Preprocess the context. */
   MimStatCalculate(MilStatCumulativeContext, M_NULL, MilStatCumulativeResult, M_PREPROCESS);

   /* Move the child buffer onto each occurrence to accumulate statistics. */
   MbufChild2d(MilImage, 0, 0, CUMULATIVESTAT_MODEL_SIZE_X, CUMULATIVESTAT_MODEL_SIZE_Y, &MilChildImage);
   for(i =0; i<NumResults; i++)
      {
      MbufChildMove(MilChildImage, (MIL_INT)PosXArray[i], (MIL_INT)PosYArray[i], CUMULATIVESTAT_MODEL_SIZE_X, CUMULATIVESTAT_MODEL_SIZE_Y, M_DEFAULT);
      MimStatCalculate(MilStatCumulativeContext, MilChildImage, MilStatCumulativeResult, M_DEFAULT);
      }

   MosPrintf(MIL_TEXT("A new model is defined using the average image.\n")
             MIL_TEXT("Unstable pixels are identified using a threshold operation on the standard\n")
             MIL_TEXT(" deviation image. The resulting image is used to mask out model pixels.\n\n"));

   /* Retrieve the average image statistic.*/
   MimDraw(M_DEFAULT, MilStatCumulativeResult, M_NULL, MilStatImage, M_DRAW_STAT_RESULT, M_STAT_MEAN, M_NULL, M_DEFAULT);

   /* Re-define the model using the average image statistic. */
   MpatDefine(MilPatContxt, M_DELETE, M_NULL, M_ALL, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MpatDefine(MilPatContxt, M_REGULAR_MODEL, MilStatImage, 0, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MpatControl(MilPatContxt, M_DEFAULT, M_NUMBER, M_ALL);
   MpatControl(MilPatContxt, M_DEFAULT, M_REFERENCE_X, 0);
   MpatControl(MilPatContxt, M_DEFAULT, M_REFERENCE_Y, 0);

   /* Retrieve the deviation image statistic.*/
   MimDraw(M_DEFAULT, MilStatCumulativeResult, M_NULL, MilStatImage, M_DRAW_STAT_RESULT, M_STAT_STANDARD_DEVIATION, M_NULL, M_DEFAULT);
   
   /* Threshold the standard deviation image to keep the higher values. */
   MimBinarize(MilStatImage, MilStatImage, M_FIXED + M_GREATER, MIN_STANDARD_DEVIATION, M_NULL);

   /* Use the binarized deviation as a model don't care mask. */
   MpatMask(MilPatContxt, M_DEFAULT, MilStatImage, M_DONT_CARE, M_DEFAULT);

   /* Preprocess the context. */
   MpatPreprocess(MilPatContxt, M_DEFAULT, MilImage);

   /* Display the new model. */
   MIL_ID MilDisplayStat = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MbufClear(MilStatImage, 0);
   MdispSelect(MilDisplayStat, MilStatImage);
   MIL_ID MilOverlay = MdispInquire(MilDisplayStat, M_OVERLAY_ID, M_NULL);
   MdispControl(MilDisplayStat, M_WINDOW_INITIAL_POSITION_X, MbufInquire(MilImage, M_SIZE_X, M_NULL) + 15);
   MgraColor(M_DEFAULT, MaskColor);

   MpatDraw(M_DEFAULT, MilPatContxt, MilStatImage, M_DRAW_IMAGE, M_DEFAULT, M_DEFAULT);
   MpatDraw(M_DEFAULT, MilPatContxt, MilOverlay, M_DRAW_DONT_CARE, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("\nThe new model and its mask (in red) are displayed.\n\n"));
   
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MdispSelect(MilDisplayStat, M_NULL);

   MosPrintf(MIL_TEXT("Find occurrences using the improved model:\n\n"));
   
   /* Use the context to find the occurrences. */
   MpatFind(MilPatContxt, MilImage, MilPatResult);

   /* Retrieve the results and display annotations. */
   MpatGetResult(MilPatResult, M_DEFAULT, M_POSITION_X, PosXArray);
   MpatGetResult(MilPatResult, M_DEFAULT, M_POSITION_Y, PosYArray);
   MpatGetResult(MilPatResult, M_DEFAULT, M_SCORE,      ScoreArray);

   MgraClear(M_DEFAULT, MilGraphicList);
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MpatDraw(M_DEFAULT, MilPatResult, MilGraphicList, M_DRAW_BOX,M_DEFAULT, M_DEFAULT);

   for(i =0; i<NumResults; i++)
      MosPrintf(MIL_TEXT("%.2f at (%.2f, %.2f)\n"), ScoreArray[i], PosXArray[i], PosYArray[i]);
 
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Free objects. */
   MgraClear(M_DEFAULT, MilGraphicList);
   MdispFree(MilDisplayStat);
   MbufFree(MilStatImage);
   MbufFree(MilChildImage);
   MbufFree(MilImage);

   MimFree(MilStatCumulativeResult);
   MimFree(MilStatCumulativeContext);

   MpatFree(MilPatResult);
   MpatFree(MilPatContxt);
   }


#define WINDOW_SIZE_X	12
#define WINDOW_SIZE_Y   12
#define WINDOW_STEP_X	12
#define WINDOW_STEP_Y	12
#define MAX_NB_EVENTS   20

void MovingWindowStats(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID  MilGraphicList;
   MdispInquire(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, &MilGraphicList);

   /* Load the image to display. */
   MIL_ID MilImage       = MbufRestore(WINDOWSTAT_IMAGE_FILE, MilSystem, M_NULL);
   MdispSelect(MilDisplay, MilImage);

   /* Inquire the source image size. */
   MIL_INT SizeX    = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT SizeY    = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   /* Allocate the second display to show the statistic results. */
   MIL_ID MilDisplayStat = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MdispControl(MilDisplayStat, M_WINDOW_INITIAL_POSITION_X, SizeX+15);   

   /* Allocate the statistic context and result buffer. */
   MIL_ID MilStatContext = MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_NULL);
   MIL_ID MilStatResult = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);

   /* Set the moving window size, step size and offset. */
   MimControl(MilStatContext, M_TILE_SIZE_X, WINDOW_SIZE_X);
   MimControl(MilStatContext, M_TILE_SIZE_Y, WINDOW_SIZE_Y);
   MimControl(MilStatContext, M_GLCM_PAIR_OFFSET_X, 2);
   MimControl(MilStatContext, M_STEP_SIZE_X, WINDOW_STEP_X);
   MimControl(MilStatContext, M_STEP_SIZE_Y, WINDOW_STEP_Y);

   /* Enable the GLCM statistics to be computed for the texture, then calculate the homogeneity statistics. */
   MimControl(MilStatContext, M_STAT_GLCM_HOMOGENEITY, M_ENABLE);
   MimStatCalculate(MilStatContext, MilImage, MilStatResult, M_DEFAULT);

   /* Draw the homogeneity result image. */
   MIL_ID MilFloatImage = MbufAlloc2d(MilSystem, SizeX/WINDOW_STEP_X, SizeY/WINDOW_STEP_Y, 32+M_FLOAT, M_IMAGE+M_PROC+M_DISP, M_NULL);
   MimDraw(M_DEFAULT, MilStatResult, M_NULL, MilFloatImage, M_DRAW_STAT_RESULT, M_STAT_GLCM_HOMOGENEITY, M_NULL, M_DEFAULT);
   
   /* Display the result image in auto scale. */
   MdispZoom(MilDisplayStat, WINDOW_STEP_X, WINDOW_STEP_Y);
   MdispControl(MilDisplayStat, M_VIEW_MODE, M_AUTO_SCALE);
   MdispControl(MilDisplayStat, M_SCALE_DISPLAY, M_ENABLE);
   MdispSelect(MilDisplayStat, MilFloatImage);
  
   MosPrintf(MIL_TEXT("A source image of a textured object is restored and displayed.\n"));
   MosPrintf(MIL_TEXT("The homogeneity windowed statistic is calculated and displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MimControl(MilStatContext, M_STAT_GLCM_HOMOGENEITY, M_DISABLE);
   MimControl(MilStatContext, M_TILE_SIZE_X, M_DEFAULT);
   MimControl(MilStatContext, M_TILE_SIZE_Y, M_DEFAULT);
   
   MimControl(MilStatContext, M_STEP_SIZE_X, M_DEFAULT);
   MimControl(MilStatContext, M_STEP_SIZE_Y, M_DEFAULT);

   /* Calculate the mean and the standard deviation of the homogeneity result image. */
   MimControl(MilStatContext, M_STAT_MEAN, M_ENABLE);
   MimControl(MilStatContext, M_STAT_STANDARD_DEVIATION, M_ENABLE);
   MimStatCalculate(MilStatContext, MilFloatImage, MilStatResult, M_DEFAULT);
   
   MosPrintf(MIL_TEXT("A threshold is applied to the homogeneity statistic based on its deviation\nto the mean."));

   MIL_DOUBLE Mean, Sigma, Threshold;
   MimGetResult(MilStatResult, M_STAT_MEAN, &Mean);
   MimGetResult(MilStatResult, M_STAT_STANDARD_DEVIATION, &Sigma);

   Threshold = Mean + 3*Sigma;

   MosPrintf(MIL_TEXT(" The result is then analyzed using MimLocateEvent() function to\n")
      MIL_TEXT("determine the presence of defects.\n\n"));

   /* Allocate the event result buffer. */ 
   MIL_INT MilEventResult = MimAllocResult(MilSystem, MAX_NB_EVENTS, M_EVENT_LIST, M_NULL),
      NbEvents;

   /* Locate the coordinates of pixels above the threshold. */
   MimLocateEvent(MilFloatImage, MilEventResult, M_GREATER_OR_EQUAL, Threshold, M_NULL);
   MimGetResult(MilEventResult, M_NB_EVENT, &NbEvents);

   MIL_INT* EventXPtr = new MIL_INT[MAX_NB_EVENTS];
   MIL_INT* EventYPtr = new MIL_INT[MAX_NB_EVENTS];

   MimGetResult(MilEventResult, M_POSITION_X + M_TYPE_MIL_INT, EventXPtr);
   MimGetResult(MilEventResult, M_POSITION_Y + M_TYPE_MIL_INT, EventYPtr);

   /* Draw rectangles around defects. */
   MgraColor(M_DEFAULT, M_COLOR_RED);
   for(MIL_INT i=0; i<NbEvents; i++)
      MgraRect(M_DEFAULT, MilGraphicList, EventXPtr[i]*WINDOW_SIZE_X, EventYPtr[i]*WINDOW_SIZE_Y, EventXPtr[i]*WINDOW_SIZE_X+WINDOW_SIZE_X, EventYPtr[i]*WINDOW_SIZE_Y+WINDOW_SIZE_Y);

   MosPrintf(MIL_TEXT("The resulting defects, if any, are displayed.\n\n"));

   delete [] EventYPtr;
   delete [] EventXPtr;

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Free objects. */
   MimFree(MilEventResult);
   MdispFree(MilDisplayStat);
   MbufFree(MilFloatImage);
   MimFree(MilStatResult);
   MimFree(MilStatContext);
   MbufFree(MilImage);
   }
