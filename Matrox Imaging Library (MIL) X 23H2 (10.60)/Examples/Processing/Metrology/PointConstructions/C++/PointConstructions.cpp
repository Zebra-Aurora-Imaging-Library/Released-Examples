//***************************************************************************************/
//
// File name: PointConstructions.cpp
//
// Synopsis:  This program illustrates various Metrology point constructions.
//            See the PrintHeader() function below for detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("PointConstructions\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example illustrates various construction methods\n"));
   MosPrintf(MIL_TEXT("for adding point features to a metrology context.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, system, display, buffer, graphic and metrology.\n\n"));
   }

//****************************************************************************
void WaitForKey()
   {
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//****************************************************************************
void CalculateAndDraw(MIL_ID  MilMetrolContext,
                      MIL_ID  MilMetrolResult,
                      MIL_ID  MilImage, 
                      MIL_ID  MilGraList,
                      MIL_INT* pRefLabel,
                      MIL_INT RefColor,
                      MIL_INT NbRef,
                      MIL_INT Label,
                      MIL_INT Color)
   {
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   MgraColor(M_DEFAULT, M_COLOR_GRAY);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraList, M_DRAW_FEATURE, M_GLOBAL_FRAME, M_DEFAULT);

   if(pRefLabel!=M_NULL)
      {
      MgraColor(M_DEFAULT, (MIL_DOUBLE)RefColor);
      for (MIL_INT ii = 0; ii < NbRef; ii++)
         MmetDraw(M_DEFAULT, MilMetrolResult, MilGraList, M_DRAW_FEATURE, pRefLabel[ii], M_DEFAULT);
      }
   MgraColor(M_DEFAULT, (MIL_DOUBLE)Color);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(Label), M_DEFAULT);
   WaitForKey();
   }


//****************************************************************************
int MosMain()
   {
   PrintHeader();

   // Allocate general MIL objects
   MIL_ID MilApplication   = MappAlloc(M_DEFAULT, M_NULL);
   MIL_ID MilSystem        = M_DEFAULT_HOST;
   MIL_ID MilDisplay       = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MIL_ID MilGraphicList   = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MIL_ID MilImage         = MbufAlloc2d(MilSystem, 512, 512, 8+M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &MilImage);
   MIL_ID MilMetrolContext = MmetAlloc(MilSystem, M_CONTEXT, M_NULL);
   MIL_ID MilMetrolResult  = MmetAllocResult(MilSystem, M_DEFAULT, M_NULL);

   MIL_INT BaseFeatures[10];

   MbufClear(MilImage, M_COLOR_BLACK);
   MdispSelect(MilDisplay, MilImage);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Util features
   MmetControl(MilMetrolContext, M_GLOBAL_FRAME, M_POSITION_X, 20.0);
   MmetControl(MilMetrolContext, M_GLOBAL_FRAME, M_POSITION_Y, 20.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_ARC, M_FEATURE_LABEL(100), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(100), M_POSITION_X,  300.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(100), M_POSITION_Y,  350.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(100), M_RADIUS,      100.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(100), M_ANGLE_START,  15.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(100), M_ANGLE_END,   220.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(101), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(101), M_POSITION_START_X, 100.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(101), M_POSITION_START_Y, 100.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(101), M_POSITION_END_X,   350.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(101), M_POSITION_END_Y,   400.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(102), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(102), M_POSITION_START_X,  50.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(102), M_POSITION_START_Y, 300.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(102), M_POSITION_END_X,   150.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(102), M_POSITION_END_Y,   350.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_CIRCLE, M_FEATURE_LABEL(103), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(103), M_POSITION_X, 120.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(103), M_POSITION_Y, 200.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(103), M_RADIUS,      80.0);

   // Constructing various points

   MosPrintf(MIL_TEXT("A point defined by its position\n")
             MIL_TEXT("===============================\n\n"));

   MgraClear(M_DEFAULT, MilGraphicList);

   MosPrintf(MIL_TEXT("1- A point defined by its position in the frame.\n"));
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(1), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(1), M_POSITION_X, 150);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(1), M_POSITION_Y, 150);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, M_NULL, M_NULL, 0, 1, M_COLOR_GREEN);

   MosPrintf(MIL_TEXT("2- A point at the start position of an oriented feature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(2), M_POSITION_START, BaseFeatures, M_NULL, 1, M_DEFAULT);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 1, 2, M_COLOR_YELLOW);

   MosPrintf(MIL_TEXT("3- A point at the end position of an oriented feature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(3), M_POSITION_END, BaseFeatures, M_NULL, 1, M_DEFAULT);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 1, 3, M_COLOR_RED);

   MosPrintf(MIL_TEXT("4- A point at an absolute distance (e.g. 40 pixels) along an oriented feature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(4), M_POSITION_ABSOLUTE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(4), M_POSITION, 40.0);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 1, 4, M_COLOR_MAGENTA);
   
   MosPrintf(MIL_TEXT("5- A point at a relative distance (e.g. 75 %%) along an oriented feature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(5), M_POSITION_RELATIVE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(5), M_POSITION, 75.0);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 1, 5, M_COLOR_CYAN);
   
   MosPrintf(MIL_TEXT("6- A point at the middle position of a feature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(6), M_MIDDLE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 1, 6, M_COLOR_LIGHT_GREEN);

   MosPrintf(MIL_TEXT("A point defined by its angle along a circular feature\n")
             MIL_TEXT("=====================================================\n\n"));
   
   MgraClear(M_DEFAULT, MilGraphicList);

   MosPrintf(MIL_TEXT("7- A point at an absolute angle (e.g. 30 degrees) along an oriented circular\nfeature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(7), M_ANGLE_ABSOLUTE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(7), M_ANGLE, 30.0);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 1, 7, M_COLOR_GREEN);

   MosPrintf(MIL_TEXT("8- A point at a relative angle (e.g. 75 %%) along an oriented circular feature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(8), M_ANGLE_RELATIVE, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(8), M_ANGLE, 75.0);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 1, 8, M_COLOR_CYAN);

   MosPrintf(MIL_TEXT("A point defined by an intersection between features\n")
             MIL_TEXT("===================================================\n\n"));

   MgraClear(M_DEFAULT, MilGraphicList);

   MosPrintf(MIL_TEXT("9- A point at the intersection between two features.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   BaseFeatures[1] = M_FEATURE_LABEL(101);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(9), M_INTERSECTION, BaseFeatures, M_NULL, 2, M_DEFAULT);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 2, 9, M_COLOR_GREEN);

   MosPrintf(MIL_TEXT("10- A point at the extended intersection between two features.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   BaseFeatures[1] = M_FEATURE_LABEL(102);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(10), M_EXTENDED_INTERSECTION, BaseFeatures, M_NULL, 2, M_DEFAULT);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 2, 10, M_COLOR_CYAN);


   MosPrintf(MIL_TEXT("A point defined by its distance to a feature\n")
             MIL_TEXT("=============================================\n\n"));

   MgraClear(M_DEFAULT, MilGraphicList);

   MosPrintf(MIL_TEXT("11- A point of a feature at the closest distance to a location along another\nfeature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(103);
   BaseFeatures[1] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(11), M_CLOSEST, BaseFeatures, M_NULL, 2, M_DEFAULT);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 2, 11, M_COLOR_GREEN);

   MosPrintf(MIL_TEXT("12- A point of a feature at the farthest distance to a location along another\nfeature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(103);
   BaseFeatures[1] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(12), M_MAX_DISTANCE_POINT, BaseFeatures, M_NULL, 2, M_DEFAULT);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 2, 12, M_COLOR_CYAN);
   
   MosPrintf(MIL_TEXT("13- A point of a feature at the largest minimum distance to a location along\nanother feature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(103);
   BaseFeatures[1] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(13), M_MAX_OF_MIN_DISTANCE_POINT, BaseFeatures, M_NULL, 2, M_DEFAULT);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 2, 13, M_COLOR_YELLOW);
   
   MosPrintf(MIL_TEXT("14- A point of a feature at the closest directional (e.g. 0 degrees) distance\nto a location along another feature.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(103);
   BaseFeatures[1] = M_FEATURE_LABEL(100);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(14), M_CLOSEST, BaseFeatures, M_NULL, 2, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(14), M_DISTANCE_MODE, M_REFERENCE_ANGLE);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(14), M_ANGLE, 0.0);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 2, 14, M_COLOR_MAGENTA);

   MosPrintf(MIL_TEXT("15- A point of a feature at the closest directional (e.g. 45 degrees) distance\nto a point at infinity.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(103);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(15), M_CLOSEST_TO_INFINITE_POINT, BaseFeatures, M_NULL, 1, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(15), M_ANGLE, 45.0);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 1, 15, M_COLOR_RED);

   MosPrintf(MIL_TEXT("A point defined at the center of multiple features\n")
             MIL_TEXT("==================================================\n\n"));

   MgraClear(M_DEFAULT, MilGraphicList);

   MosPrintf(MIL_TEXT("16- A point at the center of gravity of several features.\n"));
   BaseFeatures[0] = M_FEATURE_LABEL(100);
   BaseFeatures[1] = M_FEATURE_LABEL(101);
   BaseFeatures[2] = M_FEATURE_LABEL(102);
   BaseFeatures[3] = M_FEATURE_LABEL(103);
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(16), M_CENTER, BaseFeatures, M_NULL, 4, M_DEFAULT);
   CalculateAndDraw(MilMetrolContext, MilMetrolResult, MilImage, MilGraphicList, BaseFeatures, M_COLOR_BLUE, 4, 16, M_COLOR_RED);

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n"));
   MosGetch();

   // Free allocated objects.
   MmetFree(MilMetrolContext);
   MmetFree(MilMetrolResult);
   MgraFree(MilGraphicList);
   MbufFree(MilImage);
   MdispFree(MilDisplay);

   if (MilSystem != M_DEFAULT_HOST)
      { MsysFree(MilSystem); }
   MappFree(MilApplication);

   return 0;
   }
