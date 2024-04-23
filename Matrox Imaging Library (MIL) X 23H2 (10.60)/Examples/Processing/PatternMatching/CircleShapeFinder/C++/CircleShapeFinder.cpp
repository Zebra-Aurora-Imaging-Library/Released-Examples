﻿/*************************************************************************************/
/*
* File name: CircleShapeFinder.cpp
*
* Synopsis:  This example uses model finder to define circle models and search for circles
*            in target images. A simple circle finder example is presented first (multiple 
*            occurrences and a small radius range with good search conditions), followed by
*            more complex examples (multiple occurrences and a large radius range in a
*            complex scene with bad search conditions).
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
   MosPrintf(MIL_TEXT("CircleShapeFinder\n\n"));
   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example uses model finder to define circle models and search for circles\n"));
   MosPrintf(MIL_TEXT("in target images. A simple circle finder example is presented first (multiple\n"));
   MosPrintf(MIL_TEXT("occurrences and a small radius range with good search conditions), followed by\n"));
   MosPrintf(MIL_TEXT("more complex examples (multiple occurrences and a large radius range in a\n"));
   MosPrintf(MIL_TEXT("complex scene with bad search conditions) and an example of how to use\n"));
   MosPrintf(MIL_TEXT("M_RESOLUTION_COARSENESS_LEVEL to find very small circles.\n\n"));


   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display,\n")); 
   MosPrintf(MIL_TEXT("calibration, geometric model finder.\n\n"));

   // Wait for a key to be pressed.
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }


/* Example functions declarations. */
void SimpleCircleSearchExample(MIL_ID MilSystem, MIL_ID MilDisplay);
void ComplexCircleSearchExample1(MIL_ID MilSystem, MIL_ID MilDisplay);
void ComplexCircleSearchExample2(MIL_ID MilSystem, MIL_ID MilDisplay);
void SmallCircleSearchExample(MIL_ID MilSystem, MIL_ID MilDisplay);

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
   
   /* Run simple circle search example. */
   SimpleCircleSearchExample(MilSystem, MilDisplay);

   /* Run first complex circle search example. */
   ComplexCircleSearchExample1(MilSystem, MilDisplay);

   /* Run second complex circle search example. */
   ComplexCircleSearchExample2(MilSystem, MilDisplay);

   /* Run small circle search example. */
   SmallCircleSearchExample(MilSystem, MilDisplay);

   /* Free objects. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }


/*****************************************************************************/
/* Simple circle search example. */

/* Target MIL image file specifications. */
#define SIMPLE_CIRCLE_SEARCH_TARGET_IMAGE   M_IMAGE_PATH MIL_TEXT("/CircleShapeFinder/SimpleCircleSearchTarget.mim")

/* Number of models. */
#define NUMBER_OF_MODELS            18L

/* Model radius. */
#define MODEL_RADIUS                30.0

/* Model max number of occurrences. */
#define MODEL_MAX_OCCURRENCES       30L




void SimpleCircleSearchExample(MIL_ID MilSystem, MIL_ID MilDisplay)
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
               Radius[MODEL_MAX_OCCURRENCES],    /* Model occurrence radii.     */
               Time = 0.0;                       /* Bench variable.             */
   int         i;                                /* Loop variable.              */


   /* Restore the target image and display it. */
   MbufRestore(SIMPLE_CIRCLE_SEARCH_TARGET_IMAGE, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a circle finder context. */
   MmodAlloc(MilSystem, M_SHAPE_CIRCLE, M_DEFAULT, &MilSearchContext);

   /* Allocate a circle finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_CIRCLE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_CIRCLE, M_DEFAULT, MODEL_RADIUS,
              M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MmodControl(MilSearchContext, 0, M_NUMBER, NUMBER_OF_MODELS);
   
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

   MosPrintf(MIL_TEXT("\nUsing model finder M_SHAPE_CIRCLE in a simple situation:\n"));
   MosPrintf(MIL_TEXT("--------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("A circle model was defined with "));
   MosPrintf(MIL_TEXT("a nominal radius of %-3.1f%.\n\n"), MODEL_RADIUS);
  
   /* If a model was found above the acceptance threshold. */
   if ((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
      {
      /* Get the results of the circle search. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_RADIUS, Radius);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print the results for each circle found. */
      MosPrintf(MIL_TEXT("The circles were found in the target image:\n\n"));
      MosPrintf(MIL_TEXT("Result   X-Position   Y-Position   Radius   Score\n\n"));
     
      for (i = 0; i<NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%-9d%-13.2f%-13.2f%-8.2f%-5.2f%%\n"), i, XPosition[i],
                   YPosition[i], Radius[i], Score[i]);
         }
      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      /* Draw edges, position and box over the occurrences that were found. */

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
      MosPrintf(MIL_TEXT("the specified maximum number of occurrences!\n\n"));
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
/* Complex circle search example. */

/* Target MIL image file specifications. */
#define COMPLEX_CIRCLE_SEARCH_TARGET_IMAGE_1   M_IMAGE_PATH MIL_TEXT("/CircleShapeFinder/ComplexCircleSearchTarget1.mim")

/* Number of models. */
#define NUMBER_OF_MODELS_1        4L

/* Model radius. */
#define MODEL_RADIUS_1            100.0

/* Smoothness value. */
#define SMOOTHNESS_VALUE_1        75.0

/* Minimum scale factor value. */
#define MIN_SCALE_FACTOR_VALUE_1  0.1



void ComplexCircleSearchExample1(MIL_ID MilSystem, MIL_ID MilDisplay)
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
              Radius[MODEL_MAX_OCCURRENCES],                 /* Model occurrence radii.     */
              Time = 0.0;                                    /* Time variable.              */
   int        i;                                             /* Loop variable               */


   /* Restore the target image and display it. */
   MbufRestore(COMPLEX_CIRCLE_SEARCH_TARGET_IMAGE_1, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a model finder M_SHAPE_CIRCLE context. */
   MmodAlloc(MilSystem, M_SHAPE_CIRCLE, M_DEFAULT, &MilSearchContext);

   /* Allocate a model finder M_SHAPE_CIRCLE result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_CIRCLE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_CIRCLE, M_DEFAULT, MODEL_RADIUS_1,
      M_DEFAULT, M_DEFAULT, M_DEFAULT);
    
   /* Increase the detail level and smoothness for the edge extraction in the search context. */
   MmodControl(MilSearchContext, M_CONTEXT, M_DETAIL_LEVEL, M_VERY_HIGH);
   MmodControl(MilSearchContext, M_CONTEXT, M_SMOOTHNESS, SMOOTHNESS_VALUE_1);

   /* Enable large search scale range*/
   MmodControl(MilSearchContext, 0, M_SCALE_MIN_FACTOR, MIN_SCALE_FACTOR_VALUE_1);

   /* Set the number of occurrences to 4. */
   MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, NUMBER_OF_MODELS_1);

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

   MosPrintf(MIL_TEXT("\nUsing model finder M_SHAPE_CIRCLE in a complex situation:\n"));
   MosPrintf(MIL_TEXT("---------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("A circle model was defined with "));
   MosPrintf(MIL_TEXT("a nominal radius of %-3.1f%.\n\n"), MODEL_RADIUS_1);

   /* If the models were found above the acceptance threshold. */
   if ((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
      {
      /* Get the results for each circle found. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_RADIUS, Radius);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print information about the target image. */
      MosPrintf(MIL_TEXT("The circles were found in the target "));
      MosPrintf(MIL_TEXT("image, despite the following complexities:\n    "));
      MosPrintf(MIL_TEXT("\t. High scale range\n"));
      MosPrintf(MIL_TEXT("\t. Low contrast\n"));
      MosPrintf(MIL_TEXT("\t. Noisy edges\n\n"));

      /* Print the results for the found circles. */
      MosPrintf(MIL_TEXT("Result   X-Position   Y-Position   Radius   Score\n\n"));
      for (i = 0; i<NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%-9d%-13.2f%-13.2f%-8.2f%-5.2f%%\n"),
                  i, XPosition[i], YPosition[i], Radius[i], Score[i]);
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
      MosPrintf(MIL_TEXT("The circles were not found or the number of ")
                MIL_TEXT("circles found is greater than\n"));
      MosPrintf(MIL_TEXT("the defined value of maximum occurrences!\n\n"));
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
/* Complex circle search example. */

/* Target MIL image file specifications. */
#define COMPLEX_CIRCLE_SEARCH_TARGET_IMAGE_2   M_IMAGE_PATH MIL_TEXT("/CircleShapeFinder/ComplexCircleSearchTarget2.mim")

/* MIL calibration file specifications. */
#define COMPLEX_CIRCLE_SEARCH_CALIBRATION_2   M_IMAGE_PATH MIL_TEXT("/CircleShapeFinder/ComplexCircleSearchCalibration2.mca")

/* Number of models. */
#define NUMBER_OF_MODELS_2            23L

/* Model radius. */
#define MODEL_RADIUS_2                1.0

/* Smoothness value. */
#define SMOOTHNESS_VALUE_2            65.0

/* Acceptance value. */
#define ACCEPTANCE_VALUE_2            50.0

/* Minimum separation scale value. */
#define MIN_SEPARATION_SCALE_VALUE_2  1.5

/* Minimum separation position value. */
#define MIN_SEPARATION_XY_VALUE_2     30.0


void ComplexCircleSearchExample2(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID     MilImage,                                     /* Image buffer identifier.       */
              MilCalibration,                               /* Calibration object identifier. */
              GraphicList;                                  /* Graphic list identifier.       */
   MIL_ID     MilSearchContext,                             /* Search context                 */
              MilResult;                                    /* Result identifier.             */

   MIL_DOUBLE PositionDrawColor = M_COLOR_RED;              /* Position symbol draw color.    */
   MIL_DOUBLE ModelDrawColor = M_COLOR_GREEN;               /* Model draw color.              */
   MIL_INT    NumResults = 0L;                              /* Number of results found.       */
   MIL_DOUBLE Score[MODEL_MAX_OCCURRENCES],                 /* Model correlation scores.      */
              XPosition[MODEL_MAX_OCCURRENCES],             /* Model X positions.             */
              YPosition[MODEL_MAX_OCCURRENCES],             /* Model Y positions.             */
              Radius[MODEL_MAX_OCCURRENCES],                /* Model occurrence radii.        */
              Time = 0.0;                                   /* Time variable.                 */
   int        i;                                            /* Loop variable                  */

   /* Restore the target image and its calibration and display it. */
   MbufRestore(COMPLEX_CIRCLE_SEARCH_TARGET_IMAGE_2, MilSystem, &MilImage);

   McalRestore(COMPLEX_CIRCLE_SEARCH_CALIBRATION_2, MilSystem, M_DEFAULT, &MilCalibration);
   McalAssociate(MilCalibration, MilImage, M_DEFAULT);

   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a model finder M_SHAPE_CIRCLE context. */
   MmodAlloc(MilSystem, M_SHAPE_CIRCLE, M_DEFAULT, &MilSearchContext);

   /* Allocate a model finder M_SHAPE_CIRCLE result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_CIRCLE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_CIRCLE, M_DEFAULT, MODEL_RADIUS_2,
      M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Set the detail level and smoothness for the edge extraction in the search context. */
   MmodControl(MilSearchContext, M_CONTEXT, M_DETAIL_LEVEL, M_VERY_HIGH);
   MmodControl(MilSearchContext, M_CONTEXT, M_SMOOTHNESS, SMOOTHNESS_VALUE_2);

   /* Modify the acceptance for all the models that were defined. */
   MmodControl(MilSearchContext, M_DEFAULT, M_ACCEPTANCE, ACCEPTANCE_VALUE_2);
    
   /* Set minimum separation between occurrences constraints. */
   MmodControl(MilSearchContext, 0, M_MIN_SEPARATION_SCALE, MIN_SEPARATION_SCALE_VALUE_2);
   MmodControl(MilSearchContext, 0, M_MIN_SEPARATION_X, MIN_SEPARATION_XY_VALUE_2);
   MmodControl(MilSearchContext, 0, M_MIN_SEPARATION_Y, MIN_SEPARATION_XY_VALUE_2);

   /* Set the polarity constraints. */
   MmodControl(MilSearchContext, 0, M_POLARITY, M_REVERSE);

   /* Set the number of occurrences to 23. */
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

   MosPrintf(MIL_TEXT("\nUsing model finder M_SHAPE_CIRCLE with a calibrated target:\n"));
   MosPrintf(MIL_TEXT("-----------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("A circle model was defined with "));
   MosPrintf(MIL_TEXT("a nominal radius of %-3.1f%.\n\n"), MODEL_RADIUS_2);

   /* If the models were found above the acceptance threshold. */
   if ((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
      {
      /* Get the results for each circle found. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_RADIUS, Radius);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print information about the target image. */
      MosPrintf(MIL_TEXT("The circles were found in the calibrated target "));
      MosPrintf(MIL_TEXT("image, despite the following\ncomplexities:\n"));
      MosPrintf(MIL_TEXT("\t. Occlusion.\n"));
      MosPrintf(MIL_TEXT("\t. Low contrast.\n"));
      MosPrintf(MIL_TEXT("\t. Noisy edges.\n\n"));

      /* Print the results for the found circles. */
      MosPrintf(MIL_TEXT("Result   X-Position   Y-Position   Radius   Score\n\n"));
      for (i = 0; i<NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%-9d%-13.2f%-13.2f%-8.2f%-5.2f%%\n"),
                   i, XPosition[i], YPosition[i], Radius[i], Score[i]);
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
      MosPrintf(MIL_TEXT("The circles were not found or the number of ")
                MIL_TEXT("circles found is greater than\n"));
      MosPrintf(MIL_TEXT("the defined value of maximum occurrences!\n\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free MIL objects. */
   McalFree(MilCalibration);
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }

/* Target image. */
#define SMALL_CIRCLE_IMAGE   M_IMAGE_PATH MIL_TEXT("/CircleShapeFinder/ManySmallCircles.mim")

/* Model radius. */
#define MODEL_RADIUS_3     5.0

void SmallCircleSearchExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID   MilImage,          /* Image buffer identifier.  */
            GraphicList,       /* Graphic list identifier.  */
            MilSearchContext,  /* Search context            */
            MilResult;         /* Result identifier.        */

   MIL_INT  NumResults = 0L;   /* Number of results found.  */

   MIL_DOUBLE  Score[MODEL_MAX_OCCURRENCES],       /* Model correlation scores.      */
               XPosition[MODEL_MAX_OCCURRENCES],   /* Model X positions.             */
               YPosition[MODEL_MAX_OCCURRENCES],   /* Model Y positions.             */
               Radius[MODEL_MAX_OCCURRENCES],      /* Model occurrence radii.        */
               Time = 0.0;                         /* Time variable.                 */

   MosPrintf(MIL_TEXT("\nUsing model finder M_SHAPE_CIRCLE with M_RESOLUTION_COARSENESS_LEVEL control\n"));
   MosPrintf(MIL_TEXT("----------------------------------------------------------------------------\n\n"));

   /* Restore the target image and display it. */
   MbufRestore(SMALL_CIRCLE_IMAGE, MilSystem, &MilImage);
   MdispControl(MilDisplay, M_TITLE, MIL_TEXT("Target image"));
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a M_SHAPE_CIRCLE Model Finder context. */
   MmodAlloc(MilSystem, M_SHAPE_CIRCLE, M_DEFAULT, &MilSearchContext);

   /* Allocate a result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_CIRCLE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_DEFAULT, M_DEFAULT, MODEL_RADIUS_3, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Control the context. */
   MmodControl(MilSearchContext, 0, M_NUMBER, M_ALL);

   /* Preprocess the search context. */
   MmodPreprocess(MilSearchContext, M_DEFAULT);

   /* Pause to display information. */
   MosPrintf(MIL_TEXT("A circle model was defined with "));
   MosPrintf(MIL_TEXT("a nominal radius of %-3.1f%.\n\n"), MODEL_RADIUS_3);
   MosPrintf(MIL_TEXT("Circles will be searched with the model finder M_SHAPE_CIRCLE context.\n\n"));

   MosPrintf(MIL_TEXT("a) M_RESOLUTION_COARSENESS_LEVEL = 50 (default value)\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   auto FindAndDisplayResults = [&]()
      {
      /* Reset the timer. */
      MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

      /* Find the model. */
      MmodFind(MilSearchContext, MilImage, MilResult);

      /* Read the find time. */
      MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

      /* Get the number of models found. */
      MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

      /* If the models were found above the acceptance threshold. */
      if((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
         {
         /* Get the results for each circle found. */
         MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
         MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
         MmodGetResult(MilResult, M_DEFAULT, M_RADIUS, Radius);
         MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

         /* Print the results for the found circles. */
         MosPrintf(MIL_TEXT("Result   X-Position   Y-Position   Radius   Score\n\n"));
         for(int i = 0; i < NumResults; i++)
            {
            MosPrintf(MIL_TEXT("%-9d%-13.2f%-13.2f%-8.2f%-5.2f%%\n"),
                      i, XPosition[i], YPosition[i], Radius[i], Score[i]);
            }
         MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

         /* Draw edges, position and box over the occurrences that were found. */
         for(int i = 0; i < NumResults; i++)
            {
            MgraColor(M_DEFAULT, M_COLOR_RED);
            MmodDraw(M_DEFAULT, MilResult, GraphicList,
                     M_DRAW_EDGES + M_DRAW_BOX + M_DRAW_POSITION, i, M_DEFAULT);
            }
         }
      else
         {
         MosPrintf(MIL_TEXT("The circles were not found or the number of ")
                   MIL_TEXT("circles found is greater than\n"));
         MosPrintf(MIL_TEXT("the defined value of maximum occurrences!\n\n"));
         }
      };

   FindAndDisplayResults();

   /* Print information about the target image. */
   MosPrintf(MIL_TEXT("Here, there are 3 occurrences that are not found. However, they can be found\n"));
   MosPrintf(MIL_TEXT("by decreasing the M_RESOLUTION_COARSENESS_LEVEL.\n\n"));

   MosPrintf(MIL_TEXT("b) M_RESOLUTION_COARSENESS_LEVEL = 40\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   /* Clear annotations. */
   MgraClear(M_DEFAULT, GraphicList);

   /* Control the M_RESOLUTION_COARSENESS_LEVEL to improve the find */
   MmodControl(MilSearchContext, M_CONTEXT, M_RESOLUTION_COARSENESS_LEVEL, 40);

   /* Do the find in the target image. */
   FindAndDisplayResults();

   MosPrintf(MIL_TEXT("Now, all occurrences are found and the scores are higher.\n\n"));

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }
