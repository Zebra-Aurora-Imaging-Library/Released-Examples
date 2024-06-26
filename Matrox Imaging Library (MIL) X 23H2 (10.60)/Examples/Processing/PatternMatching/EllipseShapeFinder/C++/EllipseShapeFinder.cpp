﻿/*************************************************************************************/
/*
* File name: EllipseShapeFinder.cpp
*
* Synopsis:  This example uses model finder to define ellipse models and search for ellipses
*            in target images. Different cases are presented, such as searching for multiple
*            occurrences in a defined aspect ratio range, and using a large scale range to
*            search in a complex scene with challenging search conditions).
*            
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

//***************************************************************************
// Example description.
//***************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("EllipseShapeFinder\n\n"));
   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example uses model finder to define ellipse models and search for\n"));
   MosPrintf(MIL_TEXT("ellipses in target images. Different cases are presented, such as searching\n"));
   MosPrintf(MIL_TEXT("for multiple occurrences in a defined aspect ratio range, and using a large\n"));
   MosPrintf(MIL_TEXT("range to search in a complex scene with challenging search conditions).\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display,\n geometric model finder.\n\n"));

   // Wait for a key to be pressed.
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }


/* Example functions declarations. */
void EllipseSearchExample1(MIL_ID MilSystem, MIL_ID MilDisplay);
void EllipseAspectRatioRangeSearchExample1(MIL_ID MilSystem, MIL_ID MilDisplay);
void EllipseAspectRatioRangeSearchExample2(MIL_ID MilSystem, MIL_ID MilDisplay);
void EllipseSearchExample2(MIL_ID MilSystem, MIL_ID MilDisplay);
void EllipseSearchExample3(MIL_ID MilSystem, MIL_ID MilDisplay);

/*****************************************************************************/
/* Main.
******************************************************************************/
int MosMain(void)
   {
   MIL_ID MilApplication,     /* Application identifier. */
          MilSystem,          /* System Identifier.      */
          MilDisplay;         /* Display identifier.     */


   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Print example description. */
   PrintHeader();
     
   /* Run first ellipse search example. */
   EllipseSearchExample1(MilSystem, MilDisplay);

   /* Run first aspect ratio range search example. */
   EllipseAspectRatioRangeSearchExample1(MilSystem, MilDisplay);

   /* Run second aspect ratio range search example. */
   EllipseAspectRatioRangeSearchExample2(MilSystem, MilDisplay);

   /* Run third ellipse search example. */
   EllipseSearchExample2(MilSystem, MilDisplay);

   /* Run third ellipse search example. */
   EllipseSearchExample3(MilSystem, MilDisplay);

   /* Free objects. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }


/*****************************************************************************/
/* First ellipse search example. */

/* Target MIL image file specifications. */
#define ELLIPSE_SEARCH_TARGET_IMAGE_1   M_IMAGE_PATH MIL_TEXT("/EllipseShapeFinder/EllipseSearchTarget1.mim")


/* Model width. */
#define MODEL_WIDTH_1                       160.0

/* Model height. */
#define MODEL_HEIGHT_1                      120.0

/* Model max number of occurrences. */
#define MODEL_MAX_OCCURRENCES               10L


void EllipseSearchExample1(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilImage,                         /* Image buffer identifier.    */
               GraphicList;                      /* Graphic list identifier.    */
   MIL_ID      MilSearchContext,                 /* Search context              */
               MilResult;                        /* Result identifier.          */
   MIL_DOUBLE  PositionDrawColor = M_COLOR_RED;  /* Position symbol draw color. */
   MIL_DOUBLE  ModelDrawColor = M_COLOR_GREEN;   /* Model draw color.           */
   MIL_DOUBLE  BoxDrawColor = M_COLOR_BLUE;      /* Model box draw color.       */
   MIL_INT     NumResults = 0L;                  /* Number of results found.    */        
   MIL_DOUBLE  Score[MODEL_MAX_OCCURRENCES],     /* Model correlation score.    */
               XPosition[MODEL_MAX_OCCURRENCES], /* Model X position.           */
               YPosition[MODEL_MAX_OCCURRENCES], /* Model Y position.           */
               Width[MODEL_MAX_OCCURRENCES],     /* Model occurrence width.     */
               Height[MODEL_MAX_OCCURRENCES],    /* Model occurrence height.    */
               Time = 0.0;                       /* Bench variable.             */
   int         i;                                /* Loop variable.              */


   /* Restore the target image and display it. */
   MbufRestore(ELLIPSE_SEARCH_TARGET_IMAGE_1, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate an ellipse finder context. */
   MmodAlloc(MilSystem, M_SHAPE_ELLIPSE, M_DEFAULT, &MilSearchContext);

   /* Allocate an ellipse finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_ELLIPSE, &MilResult);
   
   /* Define the model. */
   MmodDefine(MilSearchContext, M_ELLIPSE, M_DEFAULT,  MODEL_WIDTH_1,
              MODEL_HEIGHT_1, M_DEFAULT, M_DEFAULT);

   /* Preprocess the search context. */
   MmodPreprocess(MilSearchContext, M_DEFAULT);

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the model. */
   MmodFind(MilSearchContext, MilImage, MilResult);

   /* Read the find time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

   /* Get the number of models found. */
   MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

   MosPrintf(MIL_TEXT("\n\nAn ellipse model was defined with "));
   MosPrintf(MIL_TEXT("a nominal width of %-3.1f%. "), MODEL_WIDTH_1);
   MosPrintf(MIL_TEXT("and a nominal \nheight of %-3.1f%..\n\n"), MODEL_HEIGHT_1);


   /* If a model was found above the acceptance threshold. */
   if ((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
      {
      /* Get the results of the ellipse search. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_WIDTH,  Width);
      MmodGetResult(MilResult, M_DEFAULT, M_HEIGHT, Height);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print the results for each ellipse found. */
      MosPrintf(MIL_TEXT("The ellipse was found in the target image:\n\n"));

      MosPrintf(MIL_TEXT("Result   X-Position   Y-Position   Width    Height   Aspect-Ratio   Score\n\n"));
      for(i = 0; i < NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%-9d%-13.2f%-13.2f%-9.2f%-11.2f%-13.2f%-5.2f%%\n"), i, XPosition[i],
                   YPosition[i], Width[i], Height[i], Width[i] / Height[i], Score[i]);
         }

      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      /* Draw edges, position, and box over the occurrences that were found. */

      MgraColor(M_DEFAULT, PositionDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, BoxDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_BOX, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, ModelDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      MosPrintf(MIL_TEXT("The model was not found or the number of models ")
                MIL_TEXT("found is greater than\n"));
      MosPrintf(MIL_TEXT("the specified maximum number of occurrence !\n\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }


   /* Sagitta tolerance. */
#define SAGITTA_TOLERANCE_1                     40

   /* Number of models. */
#define NUMBER_OF_MODEL_1                       7


void EllipseAspectRatioRangeSearchExample1(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilImage,                         /* Image buffer identifier.    */
               GraphicList;                      /* Graphic list identifier.    */
   MIL_ID      MilSearchContext,                 /* Search context              */
               MilResult;                        /* Result identifier.          */
   MIL_DOUBLE  PositionDrawColor = M_COLOR_RED;  /* Position symbol draw color. */
   MIL_DOUBLE  ModelDrawColor = M_COLOR_GREEN;   /* Model draw color.           */
   MIL_INT     NumResults = 0L;                  /* Number of results found.    */
   MIL_DOUBLE  Score[MODEL_MAX_OCCURRENCES],     /* Model correlation score.    */
               XPosition[MODEL_MAX_OCCURRENCES], /* Model X position.           */
               YPosition[MODEL_MAX_OCCURRENCES], /* Model Y position.           */
               Width[MODEL_MAX_OCCURRENCES],     /* Model occurrence width.     */
               Height[MODEL_MAX_OCCURRENCES],    /* Model occurrence height.    */
               Time = 0.0;                       /* Bench variable.             */
   int         i;                                /* Loop variable.              */


   /* Restore the target image and display it. */
   MbufRestore(ELLIPSE_SEARCH_TARGET_IMAGE_1, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate an ellipse finder context. */
   MmodAlloc(MilSystem, M_SHAPE_ELLIPSE, M_DEFAULT, &MilSearchContext);

   /* Allocate an ellipse finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_ELLIPSE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_ELLIPSE, M_DEFAULT, MODEL_WIDTH_1,
               MODEL_HEIGHT_1, M_DEFAULT, M_DEFAULT);

   /* Set the detail level and smoothness for the edge extraction in the search context. */
   MmodControl(MilSearchContext, M_CONTEXT, M_DETAIL_LEVEL, M_HIGH);

   /* Enable large search aspect ratio range. */
   MmodControl(MilSearchContext, 0, M_MODEL_ASPECT_RATIO_MAX_FACTOR, M_INFINITE);

   MmodControl(MilSearchContext, 0, M_MODEL_ASPECT_RATIO_MIN_FACTOR, M_CIRCLE_ASPECT_RATIO);

   /* Enable large search scale ratio range. */
   MmodControl(MilSearchContext, 0, M_SCALE_MAX_FACTOR, M_INFINITE);

   MmodControl(MilSearchContext, 0, M_SCALE_MIN_FACTOR, 0);

   /* Increase sagitta tolerance. */
   MmodControl(MilSearchContext, M_DEFAULT, M_SAGITTA_TOLERANCE, SAGITTA_TOLERANCE_1);
   
   /* Set the number of occurrences. */
   MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, NUMBER_OF_MODEL_1);

   /*Disable the minimum separation angle verification. */
   MmodControl(MilSearchContext, M_DEFAULT, M_MIN_SEPARATION_ANGLE, M_DISABLE);

   /* Preprocess the search context. */
   MmodPreprocess(MilSearchContext, M_DEFAULT);

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the model. */
   MmodFind(MilSearchContext, MilImage, MilResult);

   /* Read the find time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

   /* Get the number of models found. */
   MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

   MosPrintf(MIL_TEXT("\n\nAn ellipse model was defined with "));
   MosPrintf(MIL_TEXT("a nominal width of %-3.1f%, "), MODEL_WIDTH_1);
   MosPrintf(MIL_TEXT("a nominal \nheight of %-3.1f%, "), MODEL_HEIGHT_1);
   MosPrintf(MIL_TEXT("an infinite scale tolerance, "));
   MosPrintf(MIL_TEXT("and an infinite aspect \nration tolerance.\n\n"));

   /* If a model was found above the acceptance threshold. */
   if((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
      {
      /* Get the results of the ellipse search. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_WIDTH, Width);
      MmodGetResult(MilResult, M_DEFAULT, M_HEIGHT, Height);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print information about the target image. */
      MosPrintf(MIL_TEXT("Multiple ellipses, as indicated below, were found "));
      MosPrintf(MIL_TEXT("in the target image, \ndespite the following complexities:\n"));
      MosPrintf(MIL_TEXT("\t. Large aspect ratio range.\n"));
      MosPrintf(MIL_TEXT("\t. Large scale range.\n"));
      MosPrintf(MIL_TEXT("\t. Low contrast.\n\n"));

      MosPrintf(MIL_TEXT("Result   X-Position   Y-Position   Width    Height   Aspect-Ratio   Score\n\n"));
      for(i = 0; i < NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%-9d%-13.2f%-13.2f%-9.2f%-11.2f%-13.2f%-5.2f%%\n"), i, XPosition[i],
                   YPosition[i], Width[i], Height[i], Width[i] / Height[i], Score[i]);
         }

      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      /* Draw edges and position over the occurrences that were found. */

      MgraColor(M_DEFAULT, PositionDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
   
      MgraColor(M_DEFAULT, ModelDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      MosPrintf(MIL_TEXT("The model was not found or the number of models ")
                  MIL_TEXT("found is greater than\n"));
      MosPrintf(MIL_TEXT("the specified maximum number of occurrence !\n\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }

   
/* Scale reference. */
#define SCALE_REFERENCE_1                       0.5

/* Minimum aspect ratio factor value. */
#define MIN_ASPECT_RATIO_FACTOR_VALUE_1         0.9

/* Minimum aspect ratio factor value. */
#define MAX_ASPECT_RATIO_FACTOR_VALUE_1         1.1


void EllipseAspectRatioRangeSearchExample2(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilImage,                         /* Image buffer identifier.    */
               GraphicList;                      /* Graphic list identifier.    */
   MIL_ID      MilSearchContext,                 /* Search context              */
               MilResult;                        /* Result identifier.          */
   MIL_DOUBLE  PositionDrawColor = M_COLOR_RED;  /* Position symbol draw color. */
   MIL_DOUBLE  ModelDrawColor = M_COLOR_GREEN;   /* Model draw color.           */
   MIL_INT     NumResults = 0L;                  /* Number of results found.    */
   MIL_DOUBLE  Score[MODEL_MAX_OCCURRENCES],     /* Model correlation score.    */
               XPosition[MODEL_MAX_OCCURRENCES], /* Model X position.           */
               YPosition[MODEL_MAX_OCCURRENCES], /* Model Y position.           */
               Width[MODEL_MAX_OCCURRENCES],     /* Model occurrence width.     */
               Height[MODEL_MAX_OCCURRENCES],    /* Model occurrence height.    */
               Time = 0.0;                       /* Bench variable.             */
   int         i;                                /* Loop variable.              */


   /* Restore the target image and display it. */
   MbufRestore(ELLIPSE_SEARCH_TARGET_IMAGE_1, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate an ellipse finder context. */
   MmodAlloc(MilSystem, M_SHAPE_ELLIPSE, M_DEFAULT, &MilSearchContext);

   /* Allocate an ellipse finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_ELLIPSE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_ELLIPSE, M_DEFAULT, MODEL_WIDTH_1,
               MODEL_HEIGHT_1, M_DEFAULT, M_DEFAULT);

   /* Set the detail level and smoothness for the edge extraction in the search context. */
   MmodControl(MilSearchContext, M_CONTEXT, M_DETAIL_LEVEL, M_HIGH);

   /* Set scale reference. */
   MmodControl(MilSearchContext, M_DEFAULT, M_SCALE, SCALE_REFERENCE_1);
   
   /* Set small search aspect ratio range. */
   MmodControl(MilSearchContext, M_DEFAULT, M_MODEL_ASPECT_RATIO_MIN_FACTOR, MIN_ASPECT_RATIO_FACTOR_VALUE_1);

   MmodControl(MilSearchContext, M_DEFAULT, M_MODEL_ASPECT_RATIO_MAX_FACTOR, MAX_ASPECT_RATIO_FACTOR_VALUE_1);

   /* Do not force occurrence to be inside the defined aspect ratio range. */
   MmodControl(MilSearchContext, M_DEFAULT, M_SEARCH_ASPECT_RATIO_CONSTRAINT, M_DISABLE);

   /* Increase sagitta tolerance. */
   MmodControl(MilSearchContext, M_DEFAULT, M_SAGITTA_TOLERANCE, SAGITTA_TOLERANCE_1);

   /* Preprocess the search context. */
   MmodPreprocess(MilSearchContext, M_DEFAULT);

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the model. */
   MmodFind(MilSearchContext, MilImage, MilResult);

   /* Read the find time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

   /* Get the number of models found. */
   MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

   MosPrintf(MIL_TEXT("\n\nAn ellipse model was defined with "));
   MosPrintf(MIL_TEXT("a nominal width of %-3.1f%, "), SCALE_REFERENCE_1 * MODEL_WIDTH_1);
   MosPrintf(MIL_TEXT("a nominal \nheight of %-3.1f%, "), SCALE_REFERENCE_1 * MODEL_HEIGHT_1);
   MosPrintf(MIL_TEXT("and an aspect ratio tolerance of 10 %%.\n\n"));


   /* If a model was found above the acceptance threshold. */
   if((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
      {
      /* Get the results of the ellipse search. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_WIDTH, Width);
      MmodGetResult(MilResult, M_DEFAULT, M_HEIGHT, Height);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print information about the target image. */
      MosPrintf(MIL_TEXT("The ellipse was found in the target image, "));
      MosPrintf(MIL_TEXT("while respecting the aspect \nratio range.\n\n"));

      MosPrintf(MIL_TEXT("Result   X-Position   Y-Position   Width    Height   Aspect-Ratio   Score\n\n"));
      for(i = 0; i < NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%-9d%-13.2f%-13.2f%-9.2f%-11.2f%-13.2f%-5.2f%%\n"), i, XPosition[i],
                   YPosition[i], Width[i], Height[i], Width[i] / Height[i], Score[i]);
         }

      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      /* Draw edges and position over the occurrences that were found. */

      MgraColor(M_DEFAULT, PositionDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
    
      MgraColor(M_DEFAULT, ModelDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      MosPrintf(MIL_TEXT("The model was not found or the number of models ")
                  MIL_TEXT("found is greater than\n"));
      MosPrintf(MIL_TEXT("the specified maximum number of occurrence !\n\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }

/*****************************************************************************
/* Second ellipse search example. */

/* Target MIL image file specifications. */
#define ELLIPSE_SEARCH_TARGET_IMAGE_2   M_IMAGE_PATH MIL_TEXT("/EllipseShapeFinder/EllipseSearchTarget2.mim")

/* Number of models. */
#define NUMBER_OF_MODELS_2        7L

/* Model width. */
#define MODEL_WIDTH_2             320.0

 /* Model height. */
#define MODEL_HEIGHT_2            180.0

/* Smoothness value. */
#define SMOOTHNESS_VALUE_2        100.0

/* Minimum scale factor value. */
#define MIN_SCALE_FACTOR_VALUE_2  0.1



void EllipseSearchExample2(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID     MilImage,                                      /* Image buffer identifier.    */
              GraphicList;                                   /* Graphic list identifier.    */
   MIL_ID     MilSearchContext,                              /* Search context              */
              MilResult;                                     /* Result identifier.          */
    
   MIL_DOUBLE PositionDrawColor = M_COLOR_RED;               /* Position symbol draw color. */
   MIL_DOUBLE ModelDrawColor = M_COLOR_GREEN;                /* Model draw color.           */
   MIL_INT    NumResults = 0L;                               /* Number of results found.    */
   MIL_DOUBLE Score[MODEL_MAX_OCCURRENCES],                  /* Model correlation scores.   */
              XPosition[MODEL_MAX_OCCURRENCES],              /* Model X positions.          */
              YPosition[MODEL_MAX_OCCURRENCES],              /* Model Y positions.          */
              Width[MODEL_MAX_OCCURRENCES],                  /* Model occurrence width.     */
              Height[MODEL_MAX_OCCURRENCES],                 /* Model occurrence height.    */
              Time = 0.0;                                    /* Time variable.              */
   int        i;                                             /* Loop variable               */


   /* Restore the target image and display it. */
   MbufRestore(ELLIPSE_SEARCH_TARGET_IMAGE_2, MilSystem, &MilImage);

   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate an ellipse finder context. */
   MmodAlloc(MilSystem, M_SHAPE_ELLIPSE, M_DEFAULT, &MilSearchContext);

   /* Allocate an ellipse finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_ELLIPSE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_ELLIPSE, M_DEFAULT, MODEL_WIDTH_2,
              MODEL_HEIGHT_2, M_DEFAULT, M_DEFAULT);
    
   /* Increase the detail level and smoothness for the edge extraction in the search context. */
   MmodControl(MilSearchContext, M_CONTEXT, M_SMOOTHNESS, SMOOTHNESS_VALUE_2);

   /* Enable large search scale range*/
   MmodControl(MilSearchContext, 0, M_SCALE_MIN_FACTOR, MIN_SCALE_FACTOR_VALUE_2);

   /* Set the number of occurrences to 7. */
   MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, NUMBER_OF_MODELS_2);

   /* Preprocess the search context. */
   MmodPreprocess(MilSearchContext, M_DEFAULT);

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the models. */
   MmodFind(MilSearchContext, MilImage, MilResult);

   /* Read the find time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

   /* Get the number of models found. */
   MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);
   
   MosPrintf(MIL_TEXT("\n\nAn ellipse model was defined with "));
   MosPrintf(MIL_TEXT("a nominal width of %-3.1f%. "), MODEL_WIDTH_2);
   MosPrintf(MIL_TEXT("and a nominal \nheight of %-3.1f%..\n\n"), MODEL_HEIGHT_2);

   /* If the models were found above the acceptance threshold. */
   if ((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
      {
      /* Get the results for each ellipse found. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_WIDTH, Width);
      MmodGetResult(MilResult, M_DEFAULT, M_HEIGHT, Height);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print information about the target image. */
      MosPrintf(MIL_TEXT("Multiple ellipses, as indicated below, were found in the "));
      MosPrintf(MIL_TEXT("target image, \ndespite the following complexitie:\n    "));
      MosPrintf(MIL_TEXT("\t. High scale range.\n\n"));

      /* Print the results for the found ellipses. */
      MosPrintf(MIL_TEXT("Result   X-Position   Y-Position   Width    Height   Aspect-Ratio   Score\n\n"));
      for (i = 0; i<NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%-9d%-13.2f%-13.2f%-9.2f%-11.2f%-13.2f%-5.2f%%\n"), i, XPosition[i],
                   YPosition[i], Width[i], Height[i], Width[i] / Height[i], Score[i]);
         }
      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      /* Draw edges and positions over the occurrences that were found. */
      MgraColor(M_DEFAULT, PositionDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
    
      MgraColor(M_DEFAULT, ModelDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
     
      }
   else
      {
      MosPrintf(MIL_TEXT("The ellipses were not found or the number of ")
                MIL_TEXT("ellipses found is greater than\n"));
      MosPrintf(MIL_TEXT("the defined value of maximum occurrences !\n\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }


/*****************************************************************************
/* Third ellipse search example. */

/* Target MIL image file specifications. */
#define ELLIPSE_SEARCH_TARGET_IMAGE_3   M_IMAGE_PATH MIL_TEXT("/EllipseShapeFinder/EllipseSearchTarget3.mim")

/* Number of models. */
#define NUMBER_OF_MODELS_3            2L

/* Model width. */
#define MODEL_WIDTH_3                 480.0

/* Model height. */
#define MODEL_HEIGHT_3                360.0

/* Smoothness value. */
#define SMOOTHNESS_VALUE_3            60.0

/* Sagitta tolerance value. */
#define SAGITTA_TOLERANCE_3           10


void EllipseSearchExample3(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID     MilImage,                                     /* Image buffer identifier.       */
              GraphicList;                                  /* Graphic list identifier.       */
   MIL_ID     MilSearchContext,                             /* Search context                 */
              MilResult;                                    /* Result identifier.             */

   MIL_DOUBLE PositionDrawColor = M_COLOR_RED;              /* Position symbol draw color.    */
   MIL_DOUBLE ModelDrawColor = M_COLOR_GREEN;               /* Model draw color.              */
   MIL_INT    NumResults = 0L;                              /* Number of results found.       */
   MIL_DOUBLE Score[MODEL_MAX_OCCURRENCES],                 /* Model correlation scores.      */
              XPosition[MODEL_MAX_OCCURRENCES],             /* Model X positions.             */
              YPosition[MODEL_MAX_OCCURRENCES],             /* Model Y positions.             */
              Width[MODEL_MAX_OCCURRENCES],                 /* Model occurrence width.        */
              Height[MODEL_MAX_OCCURRENCES],                /* Model occurrence height.       */
              Time = 0.0;                                   /* Time variable.                 */
   int        i;                                            /* Loop variable                  */

   /* Restore the target image and its calibration and display it. */
   MbufRestore(ELLIPSE_SEARCH_TARGET_IMAGE_3, MilSystem, &MilImage);

   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate an ellipse finder context. */
   MmodAlloc(MilSystem, M_SHAPE_ELLIPSE, M_DEFAULT, &MilSearchContext);

   /* Allocate an ellipse finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_ELLIPSE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_ELLIPSE, M_DEFAULT, MODEL_WIDTH_3,
              MODEL_HEIGHT_3,  M_DEFAULT, M_DEFAULT);
      
   /* Set the detail level and smoothness for the edge extraction in the search context. */
   MmodControl(MilSearchContext, M_CONTEXT, M_DETAIL_LEVEL, M_VERY_HIGH);
   MmodControl(MilSearchContext, M_CONTEXT, M_SMOOTHNESS, SMOOTHNESS_VALUE_3);

   /* Set the number of occurrences to 2. */
   MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, NUMBER_OF_MODELS_3);

   /* Set a small sagitta tolerance. */
   MmodControl(MilSearchContext, M_DEFAULT, M_SAGITTA_TOLERANCE, SAGITTA_TOLERANCE_3);

   /* Preprocess the search context. */
   MmodPreprocess(MilSearchContext, M_DEFAULT);

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the models. */
   MmodFind(MilSearchContext, MilImage, MilResult);

   /* Read the find time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

   /* Get the number of models found. */
   MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

   MosPrintf(MIL_TEXT("\n\nAn ellipse model was defined with "));
   MosPrintf(MIL_TEXT("a nominal width of %-3.1f%. "), MODEL_WIDTH_3);
   MosPrintf(MIL_TEXT("and a nominal \nwidth of %-3.1f%..\n\n"), MODEL_HEIGHT_3);

   /* If the models were found above the acceptance threshold. */
   if ((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
      {
      /* Get the results for each ellipse found. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_WIDTH, Width);
      MmodGetResult(MilResult, M_DEFAULT, M_HEIGHT, Height);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print information about the target image. */
      MosPrintf(MIL_TEXT("Multiple ellipses, as indicated below, were found in the target image, "));
      MosPrintf(MIL_TEXT(" \ndespite the following complexities:\n"));
      MosPrintf(MIL_TEXT("\t. Occlusion.\n"));
      MosPrintf(MIL_TEXT("\t. Low contrast.\n"));
      MosPrintf(MIL_TEXT("\t. Noisy edges.\n\n"));

      /* Print the results for the found ellipses. */
      MosPrintf(MIL_TEXT("Result   X-Position   Y-Position   Width    Height   Aspect-Ratio   Score\n\n"));
      for(i = 0; i < NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%-9d%-13.2f%-13.2f%-9.2f%-11.2f%-13.2f%-5.2f%%\n"), i, XPosition[i],
                   YPosition[i], Width[i], Height[i], Width[i] / Height[i], Score[i]);
         }

      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      /* Draw edges and positions over the occurrences that were found. */

      MgraColor(M_DEFAULT, PositionDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, ModelDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      MosPrintf(MIL_TEXT("The ellipses were not found or the number of ")
                MIL_TEXT("ellipses found is greater than\n"));
      MosPrintf(MIL_TEXT("the defined value of maximum occurrences !\n\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }


  
