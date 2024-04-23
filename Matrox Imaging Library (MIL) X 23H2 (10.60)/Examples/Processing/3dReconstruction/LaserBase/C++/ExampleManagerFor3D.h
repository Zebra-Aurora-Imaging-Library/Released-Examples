//*******************************************************************************
//
// File name: ExampleManagerFor3D.h
//
// Synopsis:  Class that manages the processing steps in 3D examples.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef EXAMPLEMANAGERFOR3D_H
#define EXAMPLEMANAGERFOR3D_H

//Grab related defines
static const MIL_DOUBLE CAMERA_CAL_COLOR    = M_COLOR_GREEN;
static const MIL_DOUBLE REF_PLANE_COLOR     = M_COLOR_GREEN;
static const MIL_DOUBLE REF_PLANE_ROI_COLOR = M_COLOR_DARK_GREEN;

static const MIL_INT MAX_LASER_SCANS = 2;

enum PointCloudAcquisitionModeEnum
   {
   eScan,
   eScanWithContinuousAnalysis
   };

class IAnalyzeDepthMap;
class IContinuousAnalyzer
   {
   public:
      virtual
      bool AnalyzeDepthMapContinuous(MIL_ID PtCldCntr, IAnalyzeDepthMap* pProcObj) = 0;
   };

bool ProjectDepthMap(MIL_ID MilSystem, MIL_ID ContainerId, const SMapGeneration& GenerationInfo, MIL_ID* pOutDepthmap);

//*****************************************************************************
// Class that manages the processing steps for 3D examples.
//*****************************************************************************
class CExampleManagerFor3D : public IContinuousAnalyzer
   {
   public:
      CExampleManagerFor3D(MIL_INT NumCameras, 
                           const SDisplayInfo* DisplayInfo, 
                           const SIllustrations* IllustrationInfo);
      virtual ~CExampleManagerFor3D();

      bool CalibrateCameras(const SCameraCalibrationInfo* pCalibrationInfo,
                            MIL_INT NumCameras,
                            MIL_ID* pOutCamCalibrations);

      bool CalibrateSheetOfLight(const SCameraLaserInfo* pCameraLaserInfo,
                                 MIL_ID* pCameraCalibrations,
                                 MIL_ID* pOutCameraLaserCtxs);

      bool AcquirePointCloud(PointCloudAcquisitionModeEnum AcquireMode,
                             const SPointCloudAcquisitionInfo* pScanInfo,
                             MIL_ID* pCameraLaserCtxs,
                             MIL_ID* pOutPointCloudContainer,
                             IAnalyzeDepthMap* pContinuousAnalysisObj = NULL,
                             MIL_INT NbFramePerContinuousAnalysis = 100);

      bool GenerateDepthMap(MIL_ID PointCloudContainer,
                            const SMapGeneration& MapGenInfo,
                            MIL_ID* pOutDepthmap) const;
      
      bool AnalyzeDepthMap(IAnalyzeDepthMap* pProcObj, MIL_ID Depthmap, MIL_ID PtCldCtnr, const SMapGeneration& GenerationInfo);
      virtual bool AnalyzeDepthMapContinuous(MIL_ID PtCldCntr, IAnalyzeDepthMap* pProcObj);

      MIL_ID GetSystem() const { return m_MilSystem; }

   protected:

      void SetupMILDisplay();
      void FreeMILDisplay();

      void SetupGraphics();

      void AllocateMILDisplayObjects();

      void ShowStepIllustrations(ExampleSteps Step,
                                 MIL_INT DisplaySizeX, 
                                 MIL_INT DisplaySizeY);
   private:

      // Disallow copy.
      CExampleManagerFor3D(const CExampleManagerFor3D&);
      CExampleManagerFor3D& operator=(const CExampleManagerFor3D&);

      MIL_INT              m_NumCameras;

      // Used host system.
      MIL_ID               m_MilSystem;

      // Display objects.
      SDisplayInfo         m_DisplayInfo[MAX_NB_CAMERAS];
      CMILDisplayManager*  m_MilDisplays; 
      // Display image buffer identifiers.
      MIL_ID               m_MilDisplayImages[MAX_NB_CAMERAS];

      // Results display objects.
      CMILDisplayManager  m_MilResultsDisplay;       
      MIL_ID              m_MilResultsDisplayImage;  

      // Illustrations objects.
      SIllustrations      m_IllustrationInfo[eNum3dExampleSteps];
      CMILDisplayManager  m_IllustrationsDisplay[MAX_NB_ILLUSTRATIONS_PER_STEP];
      MIL_ID              m_MilIllustrationsImage[MAX_NB_ILLUSTRATIONS_PER_STEP];

      // Graphics objects.
      MIL_ID              m_MilGraphics   [MAX_NB_CAMERAS];
      MIL_ID              m_MilGraphicList[MAX_NB_CAMERAS];

      // Laser calibration objects.
      MIL_INT             m_NumLasersPerImage;
      MIL_INT             m_NumCameraLaserContexts;

      // For continuous depth map analysis.
      MIL_ID              m_DepthmapContinuous;
   };

//*****************************************************************************
// Interface class to be implemented by the specific analysis object.
//*****************************************************************************
class IAnalyzeDepthMap
   {
   public:
      virtual void AllocProcessingObjects(MIL_ID MilSystem) = 0;
      virtual void FreeProcessingObjects() = 0;

      virtual void Analyze(SCommonAnalysisObjects& CommonAnalysisObjects) = 0;

      virtual const SMapGeneration* GetMapGenInfo() const { return NULL; };
   };


#endif
