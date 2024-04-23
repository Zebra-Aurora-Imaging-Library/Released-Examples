//***************************************************************************************
//
// File name: MechanicalPartScan.h
//
// Synopsis:  Header file for MechanicalPartScan example
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef MechanicalPartScan_H
#define MechanicalPartScan_H

#include <mil.h>
#include <math.h>
#include "BaseCommon.h"

// Macro defining the example's filepath.
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("MechanicalPartScan/") MIL_TEXT(x))

// Global constant.
static const MIL_INT NUM_CAMERAS = 1;

///*****************************************************
// Function to encapsulate the example manager creation.
CExampleManagerFor3D* MakeExampleManager()
   {
   ///***************************
   // Constants 
   //****************************

   const MIL_DOUBLE DISPLAY_ZOOM_FACTOR_X = 0.4;
   const MIL_DOUBLE DISPLAY_ZOOM_FACTOR_Y = 0.4;

   const SDisplayInfo DISPLAY_INFO[NUM_CAMERAS] =
      { { { EX_PATH("grid_1.mim"), 0, 0 }, // DigInfo
           DISPLAY_ZOOM_FACTOR_X,
           DISPLAY_ZOOM_FACTOR_Y
         }
      };

   // System specifications.
   SIllustrations STEP_ILLUSTRATION_FILES[eNum3dExampleSteps] =
      {  { 1, { EX_PATH("MetalPart3dScanningCamCalibration.tif") } },
         { 1, { EX_PATH("MetalPart3dScanningLaserCalibration.tif") } },
         { 2, { EX_PATH("MetalPart3dScanningSetup.tif"), 
             EX_PATH("MetalPartObject.tif") } },
         { 0, { M_NULL } }
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
// Class to analyze the mechanical part.
class CAnalyzeMechanicalPart : public IAnalyzeDepthMap
   {
   public:
   CAnalyzeMechanicalPart() :
      m_MilMechPartModel(M_NULL),
      m_MilMechPartResult(M_NULL),
      m_MilMechPartFixtureOffset(M_NULL),
      m_MilPlaneGeometry(M_NULL),
      m_MilMethodDisplay(M_NULL),
      m_MilDispMethodImage(M_NULL),
      m_MilFullMethodImage(M_NULL),
      m_MethodImageSizeX(-1),
      m_MethodImageSizeY(-1)
      {}

   virtual void AllocProcessingObjects(MIL_ID MilSystem);
   virtual void FreeProcessingObjects();

   virtual void Analyze(SCommonAnalysisObjects& CommonAnalysisObjects);

   virtual const SMapGeneration* GetMapGenInfo() const { return NULL; };

   protected:

   bool FixturePart(MIL_ID MilDepthMap,
                     MIL_ID MilSearchImage,
                     MIL_ID MilFixtureDestination,
                     MIL_ID MilGraList);

   static void CalculateAndDisplayRelativeHeights(MIL_INT MilDepthMap,
                                                  MIL_ID MilGraphicList,
                                                  MIL_INT ReferencePointIndex);

   static void CalculateWorldZ(MIL_ID MilDepthMap,
                               MIL_INT NbPoints,
                               const MIL_DOUBLE* pInWorldPointX,
                               const MIL_DOUBLE* pInWorldPointY,
                               MIL_DOUBLE* pOutWorldPointX,
                               MIL_DOUBLE* pOutWorldPointY,
                               MIL_DOUBLE* pWorldPointZ);

   static void DisplayHeights(MIL_INT ReferenceHeightIndex,
                              MIL_ID MilGraphicList,
                              const MIL_DOUBLE* pWorldPointX,
                              const MIL_DOUBLE* pWorldPointY,
                              const MIL_DOUBLE* pWorldPointZ);

   void SetCurrentMethodImage(MIL_INT MethodIndex);

   private:

   MIL_ID m_MilMechPartModel;
   MIL_ID m_MilMechPartResult;
   MIL_ID m_MilMechPartFixtureOffset;
   MIL_ID m_MilPlaneGeometry;

   MIL_ID m_MilMethodDisplay;
   MIL_ID m_MilDispMethodImage;
   MIL_ID m_MilFullMethodImage;
   MIL_INT m_MethodImageSizeX;
   MIL_INT m_MethodImageSizeY;
   };

#endif
