//*******************************************************************************
//
// File name: BlisterPackInspection.h
//
// Synopsis:  Header file for BlisterPackInspection example
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef BLISTERPACKINSPECTION_H
#define BLISTERPACKINSPECTION_H

#include <mil.h>
#include <math.h>
#include "BaseCommon.h"

// Macro defining the example's filepath.
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("BlisterPackInspection/") MIL_TEXT(x))

// Global constant.
static const MIL_INT NUM_CAMERAS = 2;

///*****************************************************
// Class to analyze the blister pack depth map.
class CAnalyzeBlisterPack 
   {
   public:
      CAnalyzeBlisterPack() :
         m_MilModel(M_NULL),
         m_MilModelResult(M_NULL),
         m_MilDepthMapCalibration(M_NULL)
         { }

      virtual void AllocProcessingObjects(MIL_ID MilSystem);
      virtual void FreeProcessingObjects();

      virtual void Analyze(MIL_ID MilDepthMap);

   private:

      MIL_ID m_MilSystem;
      MIL_ID m_MilModel;
      MIL_ID m_MilModelResult;
      MIL_ID m_MilDepthMapCalibration;
   };

#endif
