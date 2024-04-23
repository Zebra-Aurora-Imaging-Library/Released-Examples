//***************************************************************************************/
//
// File name: TolerancesOverview.cpp
//
// Synopsis:  This program contains examples of various MIL Metrology tolerances.
//            See the PrintHeader() function below for detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#if M_MIL_USE_LINUX
#include <stdlib.h>
#include <math.h>
#endif
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Tolerances\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example illustrates various MIL metrology tolerances."));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, system, display, buffer, graphic and metrology.\n\n"));
   }

//****************************************************************************
void WaitForKey()
   {
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();
   }

//****************************************************************************
// Returns pseudo-random value between 0 and 1.
MIL_DOUBLE RandomValue()
   {
   int RandomInteger = rand();
   MIL_DOUBLE RandomDouble = ((MIL_DOUBLE)RandomInteger) / RAND_MAX;
   return RandomDouble;
   }

//****************************************************************************
void CalculateAndDisplay(MIL_ID   MilMetrolContext,
                         MIL_ID   MilMetrolResult,
                         MIL_ID   MilGraphicList,
                         MIL_INT* pBaseFeatures,
                         MIL_INT  NbBaseFeatures,
                         MIL_INT  ToleranceLabel,
                         MIL_CONST_TEXT_PTR pToleranceName,
                         MIL_CONST_TEXT_PTR pToleranceUnits)
   {
   MmetCalculate(MilMetrolContext, M_NULL, MilMetrolResult, M_DEFAULT);

   MgraClear(M_DEFAULT, MilGraphicList); 
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   for (MIL_INT ii = 0; ii < NbBaseFeatures; ii++)
      MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(pBaseFeatures[ii]), M_DEFAULT);
   
   MIL_DOUBLE ResultValue;
   MgraColor(M_DEFAULT, M_COLOR_LIGHT_GRAY);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(ToleranceLabel), M_DEFAULT);
   MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(ToleranceLabel), M_TOLERANCE_VALUE, &ResultValue);
   MosPrintf(MIL_TEXT("%s: %-3.2f %s."), pToleranceName, ResultValue, pToleranceUnits);
   WaitForKey();
   }
void CalculateAndDisplayAreaBetweenCurves(MIL_ID   MilMetrolContext,
                                          MIL_ID   MilMetrolResult,
                                          MIL_ID   MilGraphicList,
                                          MIL_INT* pBaseFeatures,
                                          MIL_INT  NbBaseFeatures,
                                          MIL_INT  ToleranceLabel,
                                          MIL_CONST_TEXT_PTR pToleranceName,
                                          MIL_CONST_TEXT_PTR pToleranceUnits)
  {
  MmetCalculate(MilMetrolContext, M_NULL, MilMetrolResult, M_DEFAULT);

  MgraClear(M_DEFAULT, MilGraphicList);

  MIL_INT OppositesSubstract;
  MmetInquire(MilMetrolContext, M_TOLERANCE_LABEL(120), M_AREA_BETWEEN_CURVES_OPPOSITES_SUBTRACT+M_TYPE_MIL_INT, &OppositesSubstract);

  if (OppositesSubstract == M_ENABLE)
     {
     MgraColor(M_DEFAULT, M_COLOR_DARK_BLUE);
     MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE_AREA_POSITIVE, M_TOLERANCE_LABEL(ToleranceLabel), M_DEFAULT);

     MgraColor(M_DEFAULT, M_COLOR_DARK_MAGENTA);
     MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE_AREA_NEGATIVE, M_TOLERANCE_LABEL(ToleranceLabel), M_DEFAULT);
     }
  else
     {
     MgraColor(M_DEFAULT, M_COLOR_DARK_BLUE);
     MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE_AREA, M_TOLERANCE_LABEL(ToleranceLabel), M_DEFAULT);
     }

  MIL_DOUBLE ResultValue;
  MmetGetResult(MilMetrolResult, M_TOLERANCE_LABEL(ToleranceLabel), M_TOLERANCE_VALUE, &ResultValue);

  MgraColor(M_DEFAULT, M_COLOR_YELLOW);
  for (MIL_INT ii = 0; ii < NbBaseFeatures; ii++)
     MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_FEATURE + M_DRAW_LABEL, M_FEATURE_LABEL(pBaseFeatures[ii]), M_DEFAULT);

  MgraColor(M_DEFAULT, M_COLOR_LIGHT_GRAY);
  MmetDraw(M_DEFAULT, MilMetrolResult, MilGraphicList, M_DRAW_TOLERANCE, M_TOLERANCE_LABEL(ToleranceLabel), M_DEFAULT);

  MosPrintf(MIL_TEXT("%s: %-3.2f %s."), pToleranceName, ResultValue, pToleranceUnits);
  if (OppositesSubstract == M_ENABLE)
     {
     MosPrintf(MIL_TEXT("\nPositive area between curves shown with blue.\n"));
     MosPrintf(MIL_TEXT("Negative area between curves shown with magenta."));
     }
  else
     {
     MosPrintf(MIL_TEXT("\nThe area between curves shown with blue."));
     }

  WaitForKey();
}

void AddCurvedShape(MIL_ID MilMetrolContext,
                    MIL_DOUBLE OffsetX,
                    MIL_DOUBLE OffsetY,
                    MIL_INT Label)
   {
   const MIL_DOUBLE PI = 3.14159265358979;
   
   const MIL_INT NumberPointEdgel = 200;
   MIL_DOUBLE EdgelPositionX[NumberPointEdgel];
   MIL_DOUBLE EdgelPositionY[NumberPointEdgel];
   for (MIL_INT i = 0; i < NumberPointEdgel; i++)
      {
      EdgelPositionX[i] = OffsetX + 100.0 * cos((MIL_DOUBLE(i) / NumberPointEdgel) * 2 * PI);
      EdgelPositionY[i] = OffsetY + 50.0 * sin((MIL_DOUBLE(i) / NumberPointEdgel) * 2 * PI) + 30.0 * cos((MIL_DOUBLE(i) / NumberPointEdgel) * 2 * PI * 2);
      }

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(Label), M_EXTERNAL_FEATURE, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetPut(MilMetrolContext, M_FEATURE_LABEL(Label), NumberPointEdgel, M_NULL, EdgelPositionX, EdgelPositionY, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(Label), M_EDGEL_PROVIDED_ORDER, M_SEQUENTIAL);
   }

void AddCurvedProfile(MIL_ID MilMetrolContext,
                      MIL_DOUBLE OffsetX,
                      MIL_DOUBLE OffsetY,
                      MIL_INT Label)
   {
   const MIL_DOUBLE PI = 3.14159265358979;

   const MIL_INT NumberPointEdgel = 100;
   MIL_DOUBLE EdgelPositionX[NumberPointEdgel];
   MIL_DOUBLE EdgelPositionY[NumberPointEdgel];
   for (MIL_INT i = 0; i < NumberPointEdgel; i++)
      {
      MIL_DOUBLE EdgelNoise = 5.0 + RandomValue() * 40.0;
      MIL_DOUBLE t = (MIL_DOUBLE(i) / NumberPointEdgel);
      EdgelPositionX[i] = OffsetX + 2.0 * i + 1.0 * EdgelNoise;
      EdgelPositionY[i] = OffsetY + 1.0 * i - 30.0 * sin(2 * PI * t) - 2.0 * EdgelNoise;
      }
  
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(Label), M_EXTERNAL_FEATURE, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetPut(MilMetrolContext, M_FEATURE_LABEL(Label), NumberPointEdgel, M_NULL, EdgelPositionX, EdgelPositionY, M_NULL, M_NULL, M_DEFAULT);
   }

void AddNoisySegment(MIL_ID MilMetrolContext, 
                     MIL_DOUBLE StartX,
                     MIL_DOUBLE StartY,
                     MIL_DOUBLE EndX,
                     MIL_DOUBLE EndY,
                     MIL_INT Label)
   {
   const MIL_INT NumberPointEdgel = 100;
   MIL_DOUBLE EdgelPositionX[NumberPointEdgel];
   MIL_DOUBLE EdgelPositionY[NumberPointEdgel];

   MIL_DOUBLE Vx = EndX - StartX;
   MIL_DOUBLE Vy = EndY - StartY;

   for (MIL_INT i = 0; i < NumberPointEdgel; i++)
      {
      MIL_DOUBLE VStep = RandomValue();
      EdgelPositionX[i] = StartX + VStep*Vx + 4.0*(RandomValue() - 0.5);
      EdgelPositionY[i] = StartY + VStep*Vy + 4.0*(RandomValue() - 0.5);
      }

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(Label), M_EXTERNAL_FEATURE, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetPut(MilMetrolContext, M_FEATURE_LABEL(Label), NumberPointEdgel, M_NULL, EdgelPositionX, EdgelPositionY, M_NULL, M_NULL, M_DEFAULT);
   }

void AddNoisyCircle(MIL_ID MilMetrolContext,
                    MIL_DOUBLE CX,
                    MIL_DOUBLE CY,
                    MIL_DOUBLE R,
                    MIL_INT Label)
   {
   const MIL_INT NumberPointEdgel = 100;
   MIL_DOUBLE EdgelPositionX[NumberPointEdgel];
   MIL_DOUBLE EdgelPositionY[NumberPointEdgel];

   for (MIL_INT i = 0; i < NumberPointEdgel; i++)
      {
      MIL_DOUBLE A = 2.0*(RandomValue()-0.5) + i * 6.28 / (MIL_DOUBLE)NumberPointEdgel;
      EdgelPositionX[i] = CX + R*sin(A) + 4.0*(RandomValue() - 0.5);
      EdgelPositionY[i] = CY + R*cos(A) + 4.0*(RandomValue() - 0.5);
      }

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(Label), M_EXTERNAL_FEATURE, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetPut(MilMetrolContext, M_FEATURE_LABEL(Label), NumberPointEdgel, M_NULL, EdgelPositionX, EdgelPositionY, M_NULL, M_NULL, M_DEFAULT);
   }

void AddWaveCurve(MIL_ID     MilMetrolContext,
                  MIL_DOUBLE ParamAmp,
                  MIL_INT    Label)
   {
   // Creation of edgels.
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_EDGEL, M_FEATURE_LABEL(Label), M_EXTERNAL_FEATURE, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   
   MIL_INT NumberPointEdgel = 100;
   MIL_DOUBLE* EdgelPositionX = new MIL_DOUBLE[NumberPointEdgel];
   MIL_DOUBLE* EdgelPositionY = new MIL_DOUBLE[NumberPointEdgel];
   for (MIL_INT i = 0; i < NumberPointEdgel; i++)
      {
      EdgelPositionX[i] = 270.0 + 1.5 * i + ParamAmp * sin((MIL_DOUBLE(i) / (NumberPointEdgel - 1)) * 4.0 * 3.14159);
      EdgelPositionY[i] = 350.0 - 1.5 * i + ParamAmp * sin((MIL_DOUBLE(i) / (NumberPointEdgel - 1)) * 4.0 * 3.14159);
      }
   MmetPut(MilMetrolContext, M_FEATURE_LABEL(Label), NumberPointEdgel, M_NULL, EdgelPositionX, EdgelPositionY, M_NULL, M_NULL, M_DEFAULT);
   delete[] EdgelPositionX;
   delete[] EdgelPositionY;

   // Edgels are already ordered.
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(Label), M_EDGEL_PROVIDED_ORDER, M_SEQUENTIAL);
   }

void AddUtilFeatures(MIL_ID MilMetrolContext)
   {
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(1), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(1), M_POSITION_START_X, 325.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(1), M_POSITION_START_Y, 180.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(1), M_POSITION_END_X,   240.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(1), M_POSITION_END_Y,   360.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(2), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(2), M_POSITION_START_X, 460.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(2), M_POSITION_START_Y, 100.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(2), M_POSITION_END_X,   550.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(2), M_POSITION_END_Y,   350.0);
   
   AddNoisySegment(MilMetrolContext, 360.0, 100.0, 450.0, 350.0, 7);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(3), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(3), M_POSITION_X, 115.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(3), M_POSITION_Y, 255.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(4), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(4), M_POSITION_X, 400.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(4), M_POSITION_Y, 350.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_POINT, M_FEATURE_LABEL(5), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(5), M_POSITION_X, 420.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(5), M_POSITION_Y, 150.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(50), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(50), M_POSITION_START_X, 325.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(50), M_POSITION_START_Y, 210.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(50), M_POSITION_END_X,   200.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(50), M_POSITION_END_Y,   252.0);

   AddNoisySegment(MilMetrolContext, 325, 210, 200, 252, 77);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_SEGMENT, M_FEATURE_LABEL(51), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(51), M_POSITION_START_X, 325.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(51), M_POSITION_START_Y, 180.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(51), M_POSITION_END_X,   400.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(51), M_POSITION_END_Y,   400.0);
   
   AddNoisySegment(MilMetrolContext, 325, 180, 400, 400, 78);
   
   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_CIRCLE, M_FEATURE_LABEL(10), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_POSITION_X, 320.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_POSITION_Y, 270.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(10), M_RADIUS,     75.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_CIRCLE, M_FEATURE_LABEL(11), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(11), M_POSITION_X, 318.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(11), M_POSITION_Y, 272.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(11), M_RADIUS,     125.0);

   AddNoisyCircle(MilMetrolContext, 320, 270, 75, 30);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LOCAL_FRAME, M_FEATURE_LABEL(70), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(70), M_POSITION_X, 100);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(70), M_POSITION_Y, 100);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(70), M_ANGLE, 25);
   
   AddCurvedShape(MilMetrolContext, 400, 300, 100);

   AddCurvedProfile(MilMetrolContext, 400, 200, 112);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LINE, M_FEATURE_LABEL(111), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(111), M_LINE_A,   1.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(111), M_LINE_B,  -2.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(111), M_LINE_C, 500.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LINE, M_FEATURE_LABEL(113), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(113), M_LINE_A,   1.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(113), M_LINE_B,  -2.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(113), M_LINE_C, -100.0);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_ARC, M_FEATURE_LABEL(99), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(99), M_POSITION_X,  325.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(99), M_POSITION_Y,  250.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(99), M_ANGLE_START,  25.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(99), M_ANGLE_END,   280.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(99), M_RADIUS,      125.0);
   
   AddWaveCurve(MilMetrolContext, 40.0, 120);
   AddWaveCurve(MilMetrolContext, 20.0, 121);

   MmetAddFeature(MilMetrolContext, M_CONSTRUCTED, M_LOCAL_FRAME, M_FEATURE_LABEL(122), M_PARAMETRIC, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(122), M_POSITION_X, 100.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(122), M_POSITION_Y, 100.0);
   MmetControl(MilMetrolContext, M_FEATURE_LABEL(122), M_ANGLE, 45.0);
   }

//****************************************************************************
void AngularityTolerance(MIL_ID MilMetrolContext, 
                         MIL_ID MilMetrolResult, 
                         MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabels[2];

   BaseFeatureLabels[0] = M_FEATURE_LABEL(1);
   BaseFeatureLabels[1] = M_FEATURE_LABEL(2);
   MmetAddTolerance(MilMetrolContext, M_ANGULARITY,  M_TOLERANCE_LABEL(1), 180, 200, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 1, MIL_TEXT("Angularity between 2 segments"), MIL_TEXT("degrees"));


   BaseFeatureLabels[0] = M_FEATURE_LABEL(1);
   BaseFeatureLabels[1] = M_FEATURE_LABEL(7);
   MmetAddTolerance(MilMetrolContext, M_ANGULARITY, M_TOLERANCE_LABEL(3), 2, 8, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(3), M_ANGLE, 45.0);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 3, MIL_TEXT("Angularity between a segment and egdels"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void PerpendicularityTolerance(MIL_ID MilMetrolContext, 
                               MIL_ID MilMetrolResult, 
                               MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabels[2];
   
   BaseFeatureLabels[0] = M_FEATURE_LABEL(2);
   BaseFeatureLabels[1] = M_FEATURE_LABEL(50);
   MmetAddTolerance(MilMetrolContext, M_PERPENDICULARITY, M_TOLERANCE_LABEL(50), 0, 1.0, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 50, MIL_TEXT("Perpendicularity between 2 segments"), MIL_TEXT("degrees"));

   BaseFeatureLabels[0] = M_FEATURE_LABEL(2);
   BaseFeatureLabels[1] = M_FEATURE_LABEL(77);
   MmetAddTolerance(MilMetrolContext, M_PERPENDICULARITY, M_TOLERANCE_LABEL(52), 3, 8, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 52, MIL_TEXT("Perpendicularity between a segment and edgels"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void ParallelismTolerance(MIL_ID MilMetrolContext,
                          MIL_ID MilMetrolResult,
                          MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabels[2];

   BaseFeatureLabels[0] = M_FEATURE_LABEL(2);
   BaseFeatureLabels[1] = M_FEATURE_LABEL(51);
   MmetAddTolerance(MilMetrolContext, M_PARALLELISM, M_TOLERANCE_LABEL(51), 0, 1.0, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 51, MIL_TEXT("Parallelism between 2 segments"), MIL_TEXT("degrees"));

   BaseFeatureLabels[0] = M_FEATURE_LABEL(2);
   BaseFeatureLabels[1] = M_FEATURE_LABEL(78);
   MmetAddTolerance(MilMetrolContext, M_PARALLELISM, M_TOLERANCE_LABEL(53), 3, 8, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 53, MIL_TEXT("Parallelism between a segment and edgels"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void ConcentricityTolerance(MIL_ID MilMetrolContext, 
                            MIL_ID MilMetrolResult, 
                            MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabels[2];
   
   BaseFeatureLabels[0] = M_FEATURE_LABEL(10);
   BaseFeatureLabels[1] = M_FEATURE_LABEL(11);
   MmetAddTolerance(MilMetrolContext, M_CONCENTRICITY,  M_TOLERANCE_LABEL(10), 30, 35, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 10, MIL_TEXT("Concentricity between 2 circles"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void RadiusTolerance(MIL_ID MilMetrolContext, 
                     MIL_ID MilMetrolResult, 
                     MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabel = M_FEATURE_LABEL(10);
   MmetAddTolerance(MilMetrolContext, M_RADIUS, M_TOLERANCE_LABEL(20), 75.0, 82.0, &BaseFeatureLabel, M_NULL, 1, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, &BaseFeatureLabel, 1, 20, MIL_TEXT("Radius"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void AreaTolerance(MIL_ID MilMetrolContext,
                   MIL_ID MilMetrolResult,
                   MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabel = M_FEATURE_LABEL(100);

   MmetAddTolerance(MilMetrolContext, M_AREA_SIMPLE, M_TOLERANCE_LABEL(95), 500.0, 600.0, &BaseFeatureLabel, M_NULL, 1, M_DEFAULT);
   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, &BaseFeatureLabel, 1, 95, MIL_TEXT("Surface area"), MIL_TEXT("squared pixels"));


   MmetAddTolerance(MilMetrolContext, M_AREA_CONVEX_HULL, M_TOLERANCE_LABEL(96), 500.0, 600.0, &BaseFeatureLabel, M_NULL, 1, M_DEFAULT);
   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, &BaseFeatureLabel, 1, 96, MIL_TEXT("Surface area using convex hull"), MIL_TEXT("squared pixels"));
   }

//****************************************************************************
void AreaUnderTheCurve(MIL_ID MilMetrolContext,
                       MIL_ID MilMetrolResult,
                       MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabel[2];
   BaseFeatureLabel[0] = M_FEATURE_LABEL(112);
   BaseFeatureLabel[1] = M_FEATURE_LABEL(111);

   MmetAddTolerance(MilMetrolContext, M_AREA_UNDER_CURVE_MAX, M_TOLERANCE_LABEL(110), 39000.0, 40000.0, BaseFeatureLabel, M_NULL, 2, M_DEFAULT);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(110), M_CURVE_EDGEL_GAP_SIZE, 10.0);
   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabel, 2, 110, MIL_TEXT("Max area under curve"), MIL_TEXT("squared pixels"));

   MmetAddTolerance(MilMetrolContext, M_AREA_UNDER_CURVE_MIN, M_TOLERANCE_LABEL(111), 39000.0, 40000.0, BaseFeatureLabel, M_NULL, 2, M_DEFAULT);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(111), M_CURVE_EDGEL_GAP_SIZE, 10.0);
   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabel, 2, 111, MIL_TEXT("Min area under curve"), MIL_TEXT("squared pixels"));
   }

//****************************************************************************
void AreaBetweenEdgels(MIL_ID MilMetrolContext,
                       MIL_ID MilMetrolResult,
                       MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabel[2];
   BaseFeatureLabel[0] = M_FEATURE_LABEL(120);
   BaseFeatureLabel[1] = M_FEATURE_LABEL(121);

   MmetAddTolerance(MilMetrolContext, M_AREA_BETWEEN_CURVES, M_TOLERANCE_LABEL(120), 3700.0, 3900.0, BaseFeatureLabel, M_NULL, 2, M_DEFAULT);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(120), M_CURVE_INFO, M_FEATURE_LABEL(122));
   CalculateAndDisplayAreaBetweenCurves(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabel, 2, 120, MIL_TEXT("Area between curves"), MIL_TEXT("squared pixels"));

   
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(120), M_AREA_BETWEEN_CURVES_OPPOSITES_SUBTRACT, M_ENABLE);
   CalculateAndDisplayAreaBetweenCurves(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabel, 2, 120, MIL_TEXT("Area between curves measured using opposites subtract"), MIL_TEXT("squared pixels"));
   }

//****************************************************************************
void RoundnessTolerance(MIL_ID MilMetrolContext, 
                        MIL_ID MilMetrolResult, 
                        MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabel = M_FEATURE_LABEL(30);
   MmetAddTolerance(MilMetrolContext, M_ROUNDNESS, M_TOLERANCE_LABEL(30), 5.0, 10.0, &BaseFeatureLabel, M_NULL, 1, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, &BaseFeatureLabel, 1, 30, MIL_TEXT("Roundness"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void StraightnessTolerance(MIL_ID MilMetrolContext, 
                           MIL_ID MilMetrolResult, 
                           MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabel = M_FEATURE_LABEL(7);
   MmetAddTolerance(MilMetrolContext, M_STRAIGHTNESS, M_TOLERANCE_LABEL(40), 5.0, 10.0, &BaseFeatureLabel, M_NULL, 1, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, &BaseFeatureLabel, 1, 40, MIL_TEXT("Straightness"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void LengthTolerance(MIL_ID MilMetrolContext, 
                     MIL_ID MilMetrolResult, 
                     MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabel = M_FEATURE_LABEL(2);
   MmetAddTolerance(MilMetrolContext, M_LENGTH, M_TOLERANCE_LABEL(60), 510.0, 515.0, &BaseFeatureLabel, M_NULL, 1, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, &BaseFeatureLabel, 1, 60, MIL_TEXT("Length"), MIL_TEXT("pixels"));

   BaseFeatureLabel = M_FEATURE_LABEL(99);
   MmetAddTolerance(MilMetrolContext, M_LENGTH, M_TOLERANCE_LABEL(61), 500.0, 505.0, &BaseFeatureLabel, M_NULL, 1, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, &BaseFeatureLabel, 1, 61, MIL_TEXT("Length"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void PositionTolerance(MIL_ID MilMetrolContext, 
                       MIL_ID MilMetrolResult, 
                       MIL_ID MilGraphicList)
   {   
   MIL_ID BaseFeatureLabels[2];

   BaseFeatureLabels[0] = M_FEATURE_LABEL(70);
   BaseFeatureLabels[1] = M_FEATURE_LABEL(10);
   MmetAddTolerance(MilMetrolContext, M_POSITION_X, M_TOLERANCE_LABEL(70), 120.0, 125.0, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);
   MmetAddTolerance(MilMetrolContext, M_POSITION_Y, M_TOLERANCE_LABEL(71), 65.0, 75.0, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 70, MIL_TEXT("Position X"), MIL_TEXT("pixels"));
   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 71, MIL_TEXT("Position Y"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void DistanceTolerance(MIL_ID MilMetrolContext, 
                       MIL_ID MilMetrolResult, 
                       MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabels[2];

   BaseFeatureLabels[0] = M_FEATURE_LABEL(2);
   BaseFeatureLabels[1] = M_FEATURE_LABEL(10);
   MmetAddTolerance(MilMetrolContext, M_DISTANCE_MIN, M_TOLERANCE_LABEL(80), 180.0, 195.0, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);
   MmetAddTolerance(MilMetrolContext, M_DISTANCE_MAX, M_TOLERANCE_LABEL(81), 360.0, 385.0, BaseFeatureLabels, M_NULL, 2, M_DEFAULT);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 80, MIL_TEXT("Distance min"), MIL_TEXT("pixels"));
   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 81, MIL_TEXT("Distance max"), MIL_TEXT("pixels"));

   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(80), M_DISTANCE_MODE, M_GAP_AT_ANGLE);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(80), M_ANGLE, 10.0);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(81), M_DISTANCE_MODE, M_FERET_AT_ANGLE);
   MmetControl(MilMetrolContext, M_TOLERANCE_LABEL(81), M_ANGLE, 10.0);

   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 80, MIL_TEXT("Gap distance at 10 degrees"), MIL_TEXT("pixels"));
   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, BaseFeatureLabels, 2, 81, MIL_TEXT("Feret distance at 10 degrees"), MIL_TEXT("pixels"));
   }

//****************************************************************************
void SurfacePerimeterTolerance(MIL_ID MilMetrolContext, 
                               MIL_ID MilMetrolResult, 
                               MIL_ID MilGraphicList)
   {
   MIL_ID BaseFeatureLabel = M_FEATURE_LABEL(100);
   
   MmetAddTolerance(MilMetrolContext, M_PERIMETER_SIMPLE, M_TOLERANCE_LABEL(90), 500.0, 600.0, &BaseFeatureLabel, M_NULL, 1, M_DEFAULT);
   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, &BaseFeatureLabel, 1, 90, MIL_TEXT("Surface perimeter"), MIL_TEXT("pixels"));


   MmetAddTolerance(MilMetrolContext, M_PERIMETER_CONVEX_HULL, M_TOLERANCE_LABEL(91), 500.0, 600.0, &BaseFeatureLabel, M_NULL, 1, M_DEFAULT);
   CalculateAndDisplay(MilMetrolContext, MilMetrolResult, MilGraphicList, &BaseFeatureLabel, 1, 91, MIL_TEXT("Surface perimeter using convex hull"), MIL_TEXT("pixels"));
   }

//****************************************************************************
int MosMain()
   {
   PrintHeader();

   // Allocate main MIL objects
   MIL_ID MilApplication      = MappAlloc(M_DEFAULT, M_NULL);
   MIL_ID MilSystem           = M_DEFAULT_HOST;
   MIL_ID MilDisplay          = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MIL_ID MilGraphicList      = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MIL_ID MilMetrolContext    = MmetAlloc(MilSystem, M_CONTEXT, M_NULL);
   MIL_ID MilMetrolResult     = MmetAllocResult(MilSystem, M_DEFAULT, M_NULL);
   MIL_ID MilImage            = MbufAllocColor(MilSystem, 3, 800, 600, 32, M_IMAGE + M_DISP + M_PROC, &MilImage);
   MIL_ID MilSingleBandImage  = MbufChildColor(MilImage, M_GREEN, M_NULL);
   
   // Select image to display.
   MbufClear(MilImage, M_COLOR_BLACK);
   MdispSelect(MilDisplay, MilImage);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);
   
   // Seed random functions.
   srand(42);

   // Add util features to the context.
   AddUtilFeatures(MilMetrolContext);

   MosPrintf(MIL_TEXT("Orientation tolerances:\n")
             MIL_TEXT("=======================\n\n"));
   
   AngularityTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);
   
   PerpendicularityTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);

   ParallelismTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);

   MosPrintf(MIL_TEXT("Dimension tolerances:\n")
             MIL_TEXT("=====================\n\n"));
   
   RadiusTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);

   LengthTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);

   SurfacePerimeterTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);

   MosPrintf(MIL_TEXT("Area tolerances:\n")
             MIL_TEXT("====================\n\n"));

   AreaTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);

   AreaUnderTheCurve(MilMetrolContext, MilMetrolResult, MilGraphicList);

   AreaBetweenEdgels(MilMetrolContext, MilMetrolResult, MilGraphicList);

   MosPrintf(MIL_TEXT("Form tolerances:\n")
             MIL_TEXT("================\n\n"));

   RoundnessTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);
   
   StraightnessTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);

   MosPrintf(MIL_TEXT("Location tolerances:\n")
             MIL_TEXT("====================\n\n"));
   
   PositionTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);
   
   DistanceTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);

   ConcentricityTolerance(MilMetrolContext, MilMetrolResult, MilGraphicList);

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n"));
   MosGetch();

   MmetFree(MilMetrolContext);
   MmetFree(MilMetrolResult);
   MgraFree(MilGraphicList);
   MbufFree(MilSingleBandImage);
   MbufFree(MilImage);
   MdispFree(MilDisplay);
   if (MilSystem != M_DEFAULT_HOST)
      { MsysFree(MilSystem); }
   MappFree(MilApplication);

   return 0;
   }
