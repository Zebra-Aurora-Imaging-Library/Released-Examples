//*******************************************************************************
//
// File name: CanInspection.h
//
// Synopsis:  Header file for CanInspection example
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef CanInspection_H
#define CanInspection_H

#include <mil.h>
#include <math.h>
#include "BaseCommon.h"

// Macro defining the example's filepath.
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("CanInspection/") MIL_TEXT(x))

// Global constants.
static const MIL_INT NUM_CAMERAS = 2;
static const MIL_INT NUM_SMOOTH_BUFFERS = 2;

///*****************************************************
// Function to encapsulate the example manager creation.
CExampleManagerFor3D* MakeExampleManager()
   {
   ///***************************
   // Constants.
   //****************************

   // Display related defines.
   const MIL_DOUBLE DISPLAY_ZOOM_FACTOR_X[NUM_CAMERAS]    = { 0.8, 0.8 };
   const MIL_DOUBLE DISPLAY_ZOOM_FACTOR_Y[NUM_CAMERAS]    = { 0.8, 0.8 };
   const SDisplayInfo DISPLAY_INFO[NUM_CAMERAS] =
      {
         { { EX_PATH("Cam1_grid.mim"), 0, 0 }, // DigInfo
           DISPLAY_ZOOM_FACTOR_X[0],
           DISPLAY_ZOOM_FACTOR_Y[0]
         },
         { { EX_PATH("Cam2_grid.mim"), 0, 0 }, // DigInfo
           DISPLAY_ZOOM_FACTOR_X[1],
           DISPLAY_ZOOM_FACTOR_Y[1]
         },
      };

   // System specifications.
   SIllustrations STEP_ILLUSTRATION_FILES[eNum3dExampleSteps] =
      {
            { 1, { EX_PATH("Cans3dScanningCamCalibration.tif")  } },
            { 1, { EX_PATH("Cans3dScanningLaserCalibration.tif")  } },
            { 1, { EX_PATH("Cans3dScanningSetup.tif")             } },
            { 0, { M_NULL } },
      };

   // Verify if the needed files are present.
   if (!CheckForRequiredMILFile(STEP_ILLUSTRATION_FILES[0].IllustrationFiles[0]))
      {
      return NULL;
      }

   CExampleManagerFor3D* pExampleManager =
      new CExampleManagerFor3D(NUM_CAMERAS,
                               DISPLAY_INFO,
                               STEP_ILLUSTRATION_FILES);
   return pExampleManager;
   }

///*****************************************************
// Class to analyze the blister pack depth map.
class CContinuousCanInspection : public IAnalyzeDepthMap
   {
   public:
      CContinuousCanInspection(const SMapGeneration& MapGenInfo) :
      m_MapGenInfo(MapGenInfo),      
      m_Remapped8BitImage(M_NULL),
      m_CanModel(M_NULL),
      m_CanModelResult(M_NULL),
      m_TabModel(M_NULL),
      m_TabModelResult(M_NULL),
      m_CanFoundSoFar(false)
         {
         }

      virtual void AllocProcessingObjects(MIL_ID MilSystem);
      virtual void FreeProcessingObjects();

      virtual void Analyze(SCommonAnalysisObjects& CommonAnalysisObjects);

      virtual const SMapGeneration* GetMapGenInfo() const { return &m_MapGenInfo; }

   private:

      SMapGeneration m_MapGenInfo;      
      MIL_ID         m_Remapped8BitImage;

      MIL_ID         m_CanModel;
      MIL_ID         m_CanModelResult;
      MIL_ID         m_TabModel;
      MIL_ID         m_TabModelResult;

      bool           m_CanFoundSoFar;
   };
#endif
