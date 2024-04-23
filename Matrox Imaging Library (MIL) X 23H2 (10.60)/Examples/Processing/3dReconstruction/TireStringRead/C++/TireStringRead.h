//*******************************************************************************
//
// File name: TireStringRead.h
//
// Synopsis:  Header file for TireStringRead example
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved


#ifndef TireStringRead_H
#define TireStringRead_H

#include <mil.h>
#include <math.h>
#include "BaseCommon.h"

// Macro defining the example's filepath.
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("TireStringRead/") MIL_TEXT(x))
static const MIL_STRING FILENAME = EX_PATH("Tire.ply");
// Global constant.
static const MIL_INT NUM_CAMERAS = 2;

///*****************************************************
// Class to read the strings on the tire.
class CTireStringRead 
   {
   public:
      CTireStringRead() :
         m_MilAdaptiveEqualizeContext(M_NULL),
         m_MilCircleMarker(M_NULL),
         m_MilModel(M_NULL),
         m_MilModelResult(M_NULL),
         m_MilFirstStringReader(M_NULL),
         m_MilFirstStringReaderResult(M_NULL),
         m_MilSecondStringReader(M_NULL),
         m_MilSecondStringReaderResult(M_NULL)
         { }

      virtual void AllocProcessingObjects(MIL_ID MilSystem);
      virtual void FreeProcessingObjects();

      virtual void Analyze(MIL_ID Mil3dDisplay, MIL_ID MilDepthMap);

   private:

      MIL_ID m_MilSystem;
      MIL_ID m_MilAdaptiveEqualizeContext;
      MIL_ID m_MilCircleMarker;
      MIL_ID m_MilModel;
      MIL_ID m_MilModelResult;
      MIL_ID m_MilFirstStringReader;
      MIL_ID m_MilFirstStringReaderResult;
      MIL_ID m_MilSecondStringReader;
      MIL_ID m_MilSecondStringReaderResult;
   };

#endif
