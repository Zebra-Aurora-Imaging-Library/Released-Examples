//*******************************************************************************
//
// File name: CookieDetection.h
//
// Synopsis:  Header file for CookieDetection example
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef CookieDetection_H
#define CookieDetection_H

#include <mil.h>
#include <math.h>
#include "BaseCommon.h"

// Macro defining the example's filepath
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("CookieDetection/") MIL_TEXT(x))

// Global constant
static const MIL_INT NUM_CAMERAS = 2;

MIL_CONST_TEXT_PTR COOKIE_BOX_MODEL = M_IMAGE_PATH MIL_TEXT("CookieDetection/BoxModel.mmf");
MIL_CONST_TEXT_PTR DEPTH_MAP_CALIBRATION = M_IMAGE_PATH MIL_TEXT("CookieDetection/DepthMapCalibration.mca");


///*****************************************************
// Class to analyze the cookie box for presence/absence
class CCookieCounting 
   {
   public:
      CCookieCounting() :
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
