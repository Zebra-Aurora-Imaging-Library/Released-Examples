﻿/*************************************************************************************/
/*
* File name: RectangleShapeFinder.cpp
*
* Synopsis:  This example uses model finder to define rectangle models and search
*            for rectangles in target images. Different cases are presented, such
*            as searching all occurrences of a specific rectangle size and searching
*            for multiple occurrences in complex images.
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
   MosPrintf(MIL_TEXT("RectangleShapeFinder\n\n"));
   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example uses model finder to define rectangle models and search\n"));
   MosPrintf(MIL_TEXT("for rectangles in target images. Different cases are presented, such\n"));
   MosPrintf(MIL_TEXT("as searching for all occurrences of a specific rectangle size and searching\n"));
   MosPrintf(MIL_TEXT("for multiple occurrences in complex images.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, display, model finder, system.\n\n"));

   // Wait for a key to be pressed.
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   }

/* Example functions declarations. */
void RectangleSearchExample1(MIL_ID MilSystem, MIL_ID MilDisplay);
void Example1FindInWholeImage  (MIL_ID MilSearchContext, MIL_ID MilImage, MIL_ID MilResult, MIL_ID GraphicList);
void Example1FindInSearchRegion(MIL_ID MilSearchContext, MIL_ID MilImage, MIL_ID MilResult, MIL_ID GraphicList);
void RectangleSearchExample2(MIL_ID MilSystem, MIL_ID MilDisplay);
void RectangleSearchExample3(MIL_ID MilSystem, MIL_ID MilDisplay);
void RectangleSearchExample4(MIL_ID MilSystem, MIL_ID MilDisplay);

/*****************************************************************************/
/* Main.
******************************************************************************/
int MosMain(void)
   {
   MIL_ID MilApplication,  /* Application identifier. */
          MilSystem,       /* System Identifier.      */
          MilDisplay;      /* Display identifier.     */

   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Print example description. */
   PrintHeader();

   RectangleSearchExample1(MilSystem, MilDisplay);
   RectangleSearchExample2(MilSystem, MilDisplay);
   RectangleSearchExample3(MilSystem, MilDisplay);
   RectangleSearchExample4(MilSystem, MilDisplay);

   /* Free objects. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }


/*****************************************************************************/
#define RECTANGLE_SEARCH_TARGET_IMAGE_1   M_IMAGE_PATH MIL_TEXT("/RectangleShapeFinder/SingleServeFood.mim")

#define MODEL_WIDTH_1                     100
#define MODEL_HEIGHT_1                    60
#define MODEL_MAX_OCCURRENCES_1           20L
#define SCALE_MIN_FACTOR_VALUE_1          0.95
#define SCALE_MAX_FACTOR_VALUE_1          1.1

void RectangleSearchExample1(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID MilImage,         /* Image buffer identifier. */
          GraphicList,      /* Graphic list identifier. */
          MilSearchContext, /* Search context           */
          MilResult;        /* Result identifier.       */

   /* Restore the target image and display it. */
   MbufRestore(RECTANGLE_SEARCH_TARGET_IMAGE_1, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a rectangle finder context. */
   MmodAlloc(MilSystem, M_SHAPE_RECTANGLE, M_DEFAULT, &MilSearchContext);

   /* Allocate a rectangle finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_RECTANGLE, &MilResult);

   /* Define the model . */
   MmodDefine(MilSearchContext, M_RECTANGLE, M_DEFAULT, MODEL_WIDTH_1,
              MODEL_HEIGHT_1, M_DEFAULT, M_DEFAULT);

   /* Specify to find all occurrences, not only the highest score */
   MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, M_ALL);

   /* Set small factor scale range. */
   MmodControl(MilSearchContext, M_DEFAULT, M_SCALE_MIN_FACTOR, SCALE_MIN_FACTOR_VALUE_1);
   MmodControl(MilSearchContext, M_DEFAULT, M_SCALE_MAX_FACTOR, SCALE_MAX_FACTOR_VALUE_1);

   MosPrintf(MIL_TEXT("\n\nA rectangle model was defined with "));
   MosPrintf(MIL_TEXT("a nominal width of %d pixels "), MODEL_WIDTH_1);
   MosPrintf(MIL_TEXT("and a nominal \nheight of %d pixels.\n"), MODEL_HEIGHT_1);

   /* Preprocess the search context. */
   MmodPreprocess(MilSearchContext, M_DEFAULT);

   /* Find the model in the whole image. */
   Example1FindInWholeImage(MilSearchContext, MilImage, MilResult, GraphicList);

   /* Clear GraphicList before the second find. */
   MgraClear(M_DEFAULT, GraphicList);

   /* Find the model in a rectangle search region at angle. */
   Example1FindInSearchRegion(MilSearchContext, MilImage, MilResult, GraphicList);

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }

void Example1FindInWholeImage(MIL_ID MilSearchContext, MIL_ID MilImage, MIL_ID MilResult, MIL_ID GraphicList)
   {
   MIL_DOUBLE  PositionDrawColor = M_COLOR_RED;    /* Position symbol draw color. */
   MIL_DOUBLE  ModelDrawColor    = M_COLOR_GREEN;  /* Model draw color.           */
   MIL_INT     NumResults        = 0L;             /* Number of results found.    */
   MIL_DOUBLE  Time              = 0.0;            /* Bench variable.             */
   std::vector<MIL_DOUBLE>  Score,                 /* Model score.                */
                            XPosition,             /* Model X position.           */
                            YPosition,             /* Model Y position.           */
                            Width,                 /* Model occurrence width.     */
                            Height;                /* Model occurrence height.    */

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the model. */
   MmodFind(MilSearchContext, MilImage, MilResult);

   /* Read the find time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

   /* Get the number of models found. */
   MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

   /* If a model was found above the acceptance threshold. */
   if((NumResults >= 1))
      {
      /* Get the results of the rectangle search. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_WIDTH     , Width    );
      MmodGetResult(MilResult, M_DEFAULT, M_HEIGHT    , Height   );
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE     , Score    );

      /* Print number of found occurrences. */
      MosPrintf(MIL_TEXT("\n%d rectangles were found in the whole image :\n\n"), NumResults);

      MosPrintf(MIL_TEXT("Result   X-Position Y-Position    Width   Height     Score\n\n"));
      for(int i = 0; i < NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%6d%13.2f%11.2f%9.2f%9.2f%9.2f%%\n"), i,
                   XPosition[i], YPosition[i], 
                   Width[i], Height[i],
                   Score[i]);
         }

      /* Print the search time. */
      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      /* Draw edges, position, over the occurrences that were found. */
      MgraColor(M_DEFAULT, PositionDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, ModelDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      MosPrintf(MIL_TEXT("The model was not found.\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   }

void Example1FindInSearchRegion(MIL_ID MilSearchContext, MIL_ID MilImage, MIL_ID MilResult, MIL_ID GraphicList)
   {
   MIL_DOUBLE  PositionDrawColor = M_COLOR_RED;      /* Position symbol draw color. */
   MIL_DOUBLE  ModelDrawColor    = M_COLOR_GREEN;    /* Model draw color.           */
   MIL_DOUBLE  SearchRegionColor = M_COLOR_DARK_RED; /* Model draw color.           */
   MIL_INT     NumResults = 0L;                      /* Number of results found.    */
   std::vector<MIL_DOUBLE>  Score,                   /* Model score.                */
                            XPosition,               /* Model X position.           */
                            YPosition,               /* Model Y position.           */
                            TopRightX,               /* Model X coordinate for top    right corner*/
                            TopRightY,               /* Model Y coordinate for top    right corner*/
                            TopLeftX,                /* Model X coordinate for top    left  corner*/
                            TopLeftY,                /* Model Y coordinate for top    left  corner*/
                            BottomRightX,            /* Model X coordinate for bottom right corner*/
                            BottomRightY,            /* Model Y coordinate for bottom right corner*/
                            BottomLeftX,             /* Model X coordinate for bottom left  corner*/
                            BottomLeftY;             /* Model Y coordinate for bottom left  corner*/

   MgraColor(M_DEFAULT, SearchRegionColor);

   static const MIL_DOUBLE  SEARCH_REGION_CENTER_X = 250.0;
   static const MIL_DOUBLE  SEARCH_REGION_CENTER_Y = 250.0;
   static const MIL_DOUBLE  SEARCH_REGION_WIDTH    = 400.0;
   static const MIL_DOUBLE  SEARCH_REGION_HEIGHT   =  50.0;
   static const MIL_DOUBLE  SEARCH_REGION_ANGLE    = 110.0;

   /* Define a rectangle search region at angle. */
   MgraRectAngle(M_DEFAULT, GraphicList, SEARCH_REGION_CENTER_X, SEARCH_REGION_CENTER_Y,
                                         SEARCH_REGION_WIDTH   , SEARCH_REGION_HEIGHT  ,
                                         SEARCH_REGION_ANGLE   , M_CENTER_AND_DIMENSION);

   MmodControl(MilSearchContext, M_DEFAULT, M_SEARCH_POSITION_FROM_GRAPHIC_LIST, GraphicList);

   /* Preprocess the search context. */
   MmodPreprocess(MilSearchContext, M_DEFAULT);

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the model. */
   MmodFind(MilSearchContext, MilImage, MilResult);

   /* Get the number of models found. */
   MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

   /* If a model was found above the acceptance threshold. */
   if((NumResults >= 1))
      {
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X    , XPosition   );
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y    , YPosition   );
      MmodGetResult(MilResult, M_DEFAULT, M_TOP_RIGHT_X   , TopRightX   );
      MmodGetResult(MilResult, M_DEFAULT, M_TOP_RIGHT_Y   , TopRightY   );
      MmodGetResult(MilResult, M_DEFAULT, M_TOP_LEFT_X    , TopLeftX    );
      MmodGetResult(MilResult, M_DEFAULT, M_TOP_LEFT_Y    , TopLeftY    );
      MmodGetResult(MilResult, M_DEFAULT, M_BOTTOM_RIGHT_X, BottomRightX);
      MmodGetResult(MilResult, M_DEFAULT, M_BOTTOM_RIGHT_Y, BottomRightY);
      MmodGetResult(MilResult, M_DEFAULT, M_BOTTOM_LEFT_X , BottomLeftX );
      MmodGetResult(MilResult, M_DEFAULT, M_BOTTOM_LEFT_Y , BottomLeftY );
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE         , Score       );

      /* Print number of found occurrences. */
      MosPrintf(MIL_TEXT("\n%d rectangles were found in the search region :\n"), NumResults);

      for(int i = 0; i < NumResults; i++)
         {
         MosPrintf(MIL_TEXT("\nResult %d with a score of %.2f%%. Its center and corners coordinates are :\n"), i,  Score[i]);

         MosPrintf(MIL_TEXT("\t                       X      \tY  \n"));
         MosPrintf(MIL_TEXT("\tCenter               : %3.2f\t%3.2f\n"), XPosition[i]   , YPosition[i]   );
         MosPrintf(MIL_TEXT("\tTop right corner     : %3.2f\t%3.2f\n"), TopRightX[i]   , TopRightY[i]   );
         MosPrintf(MIL_TEXT("\tBottom rightX corner : %3.2f\t%3.2f\n"), BottomRightX[i], BottomRightY[i]);
         MosPrintf(MIL_TEXT("\tTop left corner      : %3.2f\t%3.2f\n"), TopLeftX[i]    , TopLeftY[i]    );
         MosPrintf(MIL_TEXT("\tBottom left corner   : %3.2f\t%3.2f\n"), BottomLeftX [i], BottomLeftY [i]);
         }

      /* Draw edges, position, over the occurrences that were found. */
      MgraColor(M_DEFAULT, PositionDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
      MgraColor(M_DEFAULT, ModelDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      MosPrintf(MIL_TEXT("The model was not found.\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   }

/*****************************************************************************/
#define RECTANGLE_SEARCH_TARGET_IMAGE_2   M_IMAGE_PATH MIL_TEXT("MultipleDatamatrixCodeRead/MultipleDatamatrix.mim")

#define MODEL_WIDTH_2                        75
#define MODEL_HEIGHT_2                       75
#define MODEL_MAX_OCCURRENCES_2            100L
#define SMOOTHNESS_VALUE_2                   90

void RectangleSearchExample2(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilImage,                           /* Image buffer identifier.    */
               GraphicList;                        /* Graphic list identifier.    */
   MIL_ID      MilSearchContext,                   /* Search context              */
               MilResult;                          /* Result identifier.          */
   MIL_DOUBLE  PositionDrawColor = M_COLOR_RED;    /* Position symbol draw color. */
   MIL_DOUBLE  ModelDrawColor = M_COLOR_GREEN;     /* Model draw color.           */
   MIL_DOUBLE  BoxDrawColor = M_COLOR_BLUE;        /* Model box draw color.       */
   MIL_INT     NumResults = 0L;                    /* Number of results found.    */
   MIL_DOUBLE  Score    [MODEL_MAX_OCCURRENCES_2], /* Model score.                */
               XPosition[MODEL_MAX_OCCURRENCES_2], /* Model X position.           */
               YPosition[MODEL_MAX_OCCURRENCES_2], /* Model Y position.           */
               Width    [MODEL_MAX_OCCURRENCES_2], /* Model occurrence width.     */
               Height   [MODEL_MAX_OCCURRENCES_2], /* Model occurrence height.    */
      Time = 0.0;                                  /* Bench variable.             */

   /* Restore the target image and display it. */
   MbufRestore(RECTANGLE_SEARCH_TARGET_IMAGE_2, MilSystem, &MilImage);
   MdispZoom(MilDisplay, 0.5, 0.5);
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a rectangle finder context. */
   MmodAlloc(MilSystem, M_SHAPE_RECTANGLE, M_DEFAULT, &MilSearchContext);

   /* Allocate a rectangle finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_RECTANGLE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_RECTANGLE, M_DEFAULT, MODEL_WIDTH_2,
              MODEL_HEIGHT_2, M_DEFAULT, M_DEFAULT);

   /* Set the smoothness for the edge extraction in the search context. */
   MmodControl(MilSearchContext, M_CONTEXT, M_SMOOTHNESS, SMOOTHNESS_VALUE_2);

   MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, M_ALL);

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

   MosPrintf(MIL_TEXT("\n\nA rectangle model was defined with "));
   MosPrintf(MIL_TEXT("a nominal width of %d pixels "), MODEL_WIDTH_2);
   MosPrintf(MIL_TEXT("and a nominal \nheight of %d pixels.\n"), MODEL_HEIGHT_2);

   MosPrintf(MIL_TEXT("In this example, the smoothness is increased to fuse high-frequency patterns.\n\n"));

   /* If a model was found above the acceptance threshold. */
   if((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES_2))
      {
      /* Get the results of the rectangle search. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_WIDTH, Width);
      MmodGetResult(MilResult, M_DEFAULT, M_HEIGHT, Height);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print number of found occurrences. */
      MosPrintf(MIL_TEXT("\n%d rectangles were found : \n\n"), NumResults);

      MosPrintf(MIL_TEXT("Result   X-Position Y-Position    Width   Height   Aspect-Ratio    Score\n\n"));
      for(int i = 0; i < NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%6d%13.2f%11.2f%9.2f%9.2f%11.2f%12.2f%%\n"), i,
                   XPosition[i], YPosition[i],
                   Width[i], Height[i],
                   Width[i] / Height[i],
                   Score[i]);
         }

      /* Print the search time. */
      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      /* Draw edges, position, and box over the occurrences that were found. */
      MgraColor(M_DEFAULT, PositionDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);

      MgraColor(M_DEFAULT, ModelDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      MosPrintf(MIL_TEXT("The model was not found.\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }

/*****************************************************************************/
#define RECTANGLE_SEARCH_TARGET_IMAGE_3   M_IMAGE_PATH MIL_TEXT("/RectangleShapeFinder/Diamond.mim")

#define MODEL_WIDTH_3                        300
#define MODEL_HEIGHT_3                       300
#define ACCEPTANCE_VALUE_3                  40.0
#define MIN_SIDE_COVERAGE_VALUE_3           30.0

#define NUMBER_SEARCHED_OCCURRENCES_3         2L

#define SCALE_MIN_FACTOR_VALUE_3             0.9
#define SCALE_MAX_FACTOR_VALUE_3             1.1

void RectangleSearchExample3(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilImage,                                 /* Image buffer identifier.    */
               GraphicList;                              /* Graphic list identifier.    */
   MIL_ID      MilSearchContext,                         /* Search context              */
               MilResult;                                /* Result identifier.          */
   MIL_DOUBLE  PositionDrawColor = M_COLOR_RED;          /* Position symbol draw color. */
   MIL_DOUBLE  ModelDrawColor = M_COLOR_GREEN;           /* Model draw color.           */
   MIL_DOUBLE  BoxDrawColor = M_COLOR_BLUE;              /* Model box draw color.       */
   MIL_INT     NumResults = 0L;                          /* Number of results found.    */
   MIL_DOUBLE  Score    [NUMBER_SEARCHED_OCCURRENCES_3], /* Model score.                */
               XPosition[NUMBER_SEARCHED_OCCURRENCES_3], /* Model X position.           */
               YPosition[NUMBER_SEARCHED_OCCURRENCES_3], /* Model Y position.           */
               Width    [NUMBER_SEARCHED_OCCURRENCES_3], /* Model occurrence width.     */
               Height   [NUMBER_SEARCHED_OCCURRENCES_3], /* Model occurrence height.    */
               Time = 0.0;                               /* Bench variable.             */

   /* Restore the target image and display it. */
   MbufRestore(RECTANGLE_SEARCH_TARGET_IMAGE_3, MilSystem, &MilImage);
   MdispZoom(MilDisplay, 1, 1);
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a rectangle finder context. */
   MmodAlloc(MilSystem, M_SHAPE_RECTANGLE, M_DEFAULT, &MilSearchContext);

   /* Allocate a rectangle finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_RECTANGLE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_RECTANGLE, M_DEFAULT, MODEL_WIDTH_3,
              MODEL_HEIGHT_3, M_DEFAULT, M_DEFAULT);

   /* Set the smoothness for the edge extraction in the search context. */
   MmodControl(MilSearchContext, M_CONTEXT, M_DETAIL_LEVEL, M_HIGH);

   /* Set the polarity to any. */
   MmodControl(MilSearchContext, M_DEFAULT, M_POLARITY, M_ANY);

   /* Set the number of occurrences to 2. */
   MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, NUMBER_SEARCHED_OCCURRENCES_3);

   /* Modify the acceptance for all the model that was defined. */
   MmodControl(MilSearchContext, M_DEFAULT, M_ACCEPTANCE, ACCEPTANCE_VALUE_3);

   /* Set small factor scale range. */
   MmodControl(MilSearchContext, M_DEFAULT, M_SCALE_MIN_FACTOR, SCALE_MIN_FACTOR_VALUE_3);

   /* Set small factor scale range. */
   MmodControl(MilSearchContext, M_DEFAULT, M_SCALE_MAX_FACTOR, SCALE_MAX_FACTOR_VALUE_3);

   /* Set the min side coverage. */
   MmodControl(MilSearchContext, M_DEFAULT, M_MIN_SIDE_COVERAGE, MIN_SIDE_COVERAGE_VALUE_3);

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

   MosPrintf(MIL_TEXT("\n\nA rectangle model was defined with "));
   MosPrintf(MIL_TEXT("a nominal width of %d pixels "), MODEL_WIDTH_3);
   MosPrintf(MIL_TEXT("and a nominal \nheight of %d pixels\n\n"), MODEL_HEIGHT_3);

   /* If a model was found above the acceptance threshold. */
   if((NumResults >= 1) && (NumResults <= NUMBER_SEARCHED_OCCURRENCES_3))
      {
      /* Get the results of the rectangle search. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_WIDTH, Width);
      MmodGetResult(MilResult, M_DEFAULT, M_HEIGHT, Height);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print the results for each rectangle found. */
      MosPrintf(MIL_TEXT("The occurrences were found in the target image by reducing the \n"));
      MosPrintf(MIL_TEXT("minimum coverage value and by restraining the scale factor range.\n\n"));
      
      
                          
      MosPrintf(MIL_TEXT("Result   X-Position Y-Position    Width   Height   Aspect-Ratio    Score\n\n"));
      for(int i = 0; i < NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%6d%13.2f%11.2f%9.2f%9.2f%11.2f%12.2f%%\n"), i,
                   XPosition[i], YPosition[i],
                   Width[i], Height[i],
                   Width[i] / Height[i],
                   Score[i]);
         }

      /* Print the search time. */
      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      /* Draw edges, position, and box over the occurrences that were found. */

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
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }

/*****************************************************************************/
#define RECTANGLE_SEARCH_TARGET_IMAGE_4   M_IMAGE_PATH MIL_TEXT("/RectangleShapeFinder/Circuit.mim")

#define MODEL_WIDTH_4                        40
#define MODEL_HEIGHT_4                       40
#define SMOOTHNESS_VALUE_4                 50.0
#define ACCEPTANCE_VALUE_4                 40.0
#define MODEL_MAX_OCCURRENCES_4            100L

#define MIN_SIDE_COVERAGE_VALUE_4        33.0

#define SCALE_MIN_FACTOR_VALUE_4          0.9
#define SCALE_MAX_FACTOR_VALUE_4          1.2

void RectangleSearchExample4(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilImage,                           /* Image buffer identifier.    */
               GraphicList;                        /* Graphic list identifier.    */
   MIL_ID      MilSearchContext,                   /* Search context              */
               MilResult;                          /* Result identifier.          */
   MIL_DOUBLE  PositionDrawColor = M_COLOR_RED;    /* Position symbol draw color. */
   MIL_DOUBLE  ModelDrawColor = M_COLOR_GREEN;     /* Model draw color.           */
   MIL_INT     NumResults = 0L;                    /* Number of results found.    */
   MIL_DOUBLE  Score    [MODEL_MAX_OCCURRENCES_4], /* Model score.                */
               XPosition[MODEL_MAX_OCCURRENCES_4], /* Model X position.           */
               YPosition[MODEL_MAX_OCCURRENCES_4], /* Model Y position.           */
               Width    [MODEL_MAX_OCCURRENCES_4], /* Model occurrence width.     */
               Height   [MODEL_MAX_OCCURRENCES_4], /* Model occurrence height.    */
               Time = 0.0;                         /* Bench variable.             */

   /* Restore the target image and display it. */
   MbufRestore(RECTANGLE_SEARCH_TARGET_IMAGE_4, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a rectangle finder context. */
   MmodAlloc(MilSystem, M_SHAPE_RECTANGLE, M_DEFAULT, &MilSearchContext);

   /* Allocate a rectangle finder result buffer. */
   MmodAllocResult(MilSystem, M_SHAPE_RECTANGLE, &MilResult);

   /* Define the model. */
   MmodDefine(MilSearchContext, M_RECTANGLE, M_DEFAULT, MODEL_WIDTH_4,
              MODEL_HEIGHT_4, M_DEFAULT, M_DEFAULT);

   /* Specify to find all occurrences, not only the highest score */
   MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, M_ALL);

   /* Set the detail level and smoothness for the edge extraction in the search context. */
   MmodControl(MilSearchContext, M_CONTEXT, M_DETAIL_LEVEL, M_HIGH);

   /* Set small factor scale range. */
   MmodControl(MilSearchContext, M_DEFAULT, M_SCALE_MIN_FACTOR, SCALE_MIN_FACTOR_VALUE_4);
   MmodControl(MilSearchContext, M_DEFAULT, M_SCALE_MAX_FACTOR, SCALE_MAX_FACTOR_VALUE_4);

   /* Set The Polarity Constraints. */
   MmodControl(MilSearchContext, M_DEFAULT, M_POLARITY, M_REVERSE);

   /* Modify acceptance and coverage values. */
   MmodControl(MilSearchContext, M_DEFAULT, M_ACCEPTANCE, ACCEPTANCE_VALUE_4);
   MmodControl(MilSearchContext, M_DEFAULT, M_MIN_SIDE_COVERAGE, MIN_SIDE_COVERAGE_VALUE_4);

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

   MosPrintf(MIL_TEXT("\n\nA rectangle model was defined with "));
   MosPrintf(MIL_TEXT("a nominal width of %d pixels "), MODEL_WIDTH_4);
   MosPrintf(MIL_TEXT("and a nominal \nheight of %d pixels.\n"), MODEL_HEIGHT_4);

   /* If a model was found above the acceptance threshold. */
   if((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES_4))
      {
      /* Get the results of the rectangle search. */
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
      MmodGetResult(MilResult, M_DEFAULT, M_WIDTH, Width);
      MmodGetResult(MilResult, M_DEFAULT, M_HEIGHT, Height);
      MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

      /* Print number of found occurrences. */
      MosPrintf(MIL_TEXT("\n%d rectangles were found, using a low side coverage value,\n"),
                NumResults);
      MosPrintf(MIL_TEXT("and a reduced scale factor range.\n\n"));

      MosPrintf(MIL_TEXT("Result   X-Position Y-Position    Width   Height   Aspect-Ratio    Score\n\n"));
      for(int i = 0; i < NumResults; i++)
         {
         MosPrintf(MIL_TEXT("%6d%13.2f%11.2f%9.2f%9.2f%11.2f%12.2f%%\n"), i,
                   XPosition[i], YPosition[i],
                   Width[i], Height[i],
                   Width[i] / Height[i],
                   Score[i]);
         }

      /* Print the search time. */
      MosPrintf(MIL_TEXT("\nThe search time was %.1f ms.\n\n"), Time*1000.0);

      MgraColor(M_DEFAULT, ModelDrawColor);
      MmodDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      MosPrintf(MIL_TEXT("The model was not found.\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   /* Free MIL objects. */
   MgraFree(GraphicList);
   MbufFree(MilImage);
   MmodFree(MilSearchContext);
   MmodFree(MilResult);
   }
