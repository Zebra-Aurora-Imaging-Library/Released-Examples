//***************************************************************************************
// 
// File name: LidColorsVerification.cpp
//
// Synopsis: This file contains the declaration of the CLidColorsVerification class
//           which is the inspection task used to verify the colors of the lid.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved


#include <mil.h>
#include "LidIdentification.h"

//*****************************************************************************
// Constants.
//*****************************************************************************

//*****************************************************************************
// Constructor.
//*****************************************************************************
CLidIdentification::CLidIdentification(MIL_CONST_TEXT_PTR PatContextPath, 
                                       MIL_INT ColorConversion /* = M_NONE */, 
                                       CInspectionTask* FixtureProvider /* = M_NULL */, 
                                       CInspectionTask* ImageProvider /* = M_NULL */, 
                                       CInspectionTask* RegionProvider /* = M_NULL */)
 : CPatMatchingTask(PatContextPath, ColorConversion, FixtureProvider, ImageProvider)
   {
   }

//*****************************************************************************
// Destructor.
//*****************************************************************************
CLidIdentification::~CLidIdentification()
   {
   }

//*****************************************************************************
// Draw graphical result.
//*****************************************************************************
void CLidIdentification::DrawGraphicalResult(MIL_ID MilGraContext, MIL_ID MilDest)
   {
   if(IsResultValid())
      {
      CPatMatchingTask::DrawGraphicalResult(MilGraContext, MilDest);
      MgraColor(MilGraContext, M_COLOR_GREEN);
      MpatDraw(MilGraContext, MilResult(), MilDest, M_DRAW_BOX, M_DEFAULT, M_DEFAULT);
      }
   }

//*****************************************************************************
// Draw text result.
//*****************************************************************************
void CLidIdentification::DrawTextResult(MIL_ID MilGraContext, MIL_ID MilDest)
   {
   if(GetResultStatus() == eUnknown)
      {
      MgraColor(MilGraContext, M_COLOR_YELLOW);
      MgraText(MilGraContext, MilDest, 0, 0, MIL_TEXT("Lid pattern identification: UNKNOWN"));
      }
   else
      {
      if(IsResultValid())
         {
         MgraColor(MilGraContext, M_COLOR_GREEN);
         MgraText(MilGraContext, MilDest, 0, 0, MIL_TEXT("Lid pattern identification: PASS"));
         }
      else
         {
         MgraColor(MilGraContext, M_COLOR_RED);
         MgraText(MilGraContext, MilDest, 0, 0, MIL_TEXT("Lid pattern identification: FAIL"));
         }
      }
   
   MoveGraphicContextYOffset(MilGraContext, 1);
   }
