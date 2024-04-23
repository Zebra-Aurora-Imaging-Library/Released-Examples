//*******************************************************************************
//
// File name: BottleCapInspection.h
//
// Synopsis:  Header file for BottleCapInspection example
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef BottleCapInspection_H
#define BottleCapInspection_H

#include <mil.h>
#include <math.h>
#include "BaseCommon.h"

// Macro defining the example's filepath
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("BottleCapInspection/") MIL_TEXT(x))

// Global constant
static const MIL_INT NUM_CAMERAS = 2;

///*****************************************************
// Class to analyze the bottle caps depth map
class CAnalyzeBottleCap
   {
   public:
      CAnalyzeBottleCap() :
         m_CapModel(M_NULL),
         m_CapModelResult(M_NULL),
         m_Geometry(M_NULL),
         m_ReferenceGeometry(M_NULL)
         { }

      virtual void AllocProcessingObjects(MIL_ID MilSystem);
      virtual void FreeProcessingObjects();

      virtual void Analyze(MIL_ID MilDepthMap);

      // Structure required to store results
      struct SResults
         {
         MIL_TEXT_CHAR MissingData[MAX_STRING_LEN];
         MIL_TEXT_CHAR Angle[MAX_STRING_LEN];
         MIL_TEXT_CHAR MeanDeviation[MAX_STRING_LEN];
         MIL_TEXT_CHAR Status[MAX_STRING_LEN];
         MIL_DOUBLE    PosX;
         MIL_DOUBLE    PosY;
         };

   protected:
      static void SortCapPositions(MIL_INT* pX, MIL_INT* pY, MIL_INT Nb);

   private:
      MIL_ID m_MilSystem;
      MIL_ID m_CapModel;
      MIL_ID m_CapModelResult;

      MIL_ID m_Geometry;
      MIL_ID m_ReferenceGeometry;
   };

#endif
