﻿//*******************************************************************************
// 
// File name: ExampleManagerFor3D.cpp  
//
// Synopsis:  Class that manages the 3D processing example.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "BaseCommon.h"

//*******************************************************************************
// Allocates and initializes the object
//*******************************************************************************
CExampleManagerFor3D::CExampleManagerFor3D(MIL_INT NumCameras,
                                           const SDisplayInfo* DisplayInfo, 
                                           const SIllustrations* IllustrationInfo)
   {
   m_NumCameras = NumCameras;
   m_MilDisplays = NULL;

   m_DepthmapContinuous = M_NULL;

   m_NumCameraLaserContexts = -1;
   m_NumLasersPerImage = -1;
   for (MIL_INT c = 0; c < MAX_NB_CAMERAS; c++)
      {
      m_MilDisplayImages[c] = M_NULL;
      m_MilGraphics     [c] = M_NULL;
      m_MilGraphicList  [c] = M_NULL;
      }

   // Allocate a host system.
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &m_MilSystem);

   // Register the display information.
   for (MIL_INT i = 0; i < m_NumCameras; i++)
      {
      m_DisplayInfo[i] = DisplayInfo[i];
      m_DisplayInfo[i].DigitizerInfo.UpdateInfoFromDisk();
      }

   if (IllustrationInfo)
      {
      for (MIL_INT i = 0; i < eNum3dExampleSteps; i++)
         {
         m_IllustrationInfo[i] = IllustrationInfo[i];
         }
      }

   SetupMILDisplay();
   SetupGraphics();
   }

//*******************************************************************************
// Frees the object.
//*******************************************************************************
CExampleManagerFor3D::~CExampleManagerFor3D()
   {
   if(m_DepthmapContinuous != M_NULL)
      { MbufFree(m_DepthmapContinuous); m_DepthmapContinuous = M_NULL; };

   FreeMILDisplay();
   for (MIL_INT i = 0; i < m_NumCameras; i++)
      {
      MgraFree(m_MilGraphics[i]);
      MgraFree(m_MilGraphicList[i]);
      }

   MsysFree(m_MilSystem);
   m_MilSystem = M_NULL;
   }

//*******************************************************************************
// Sets up the MIL display.
//*******************************************************************************
void CExampleManagerFor3D::SetupMILDisplay()
   {
   m_MilDisplays = new CMILDisplayManager[m_NumCameras];

   AllocateMILDisplayObjects();

   // Setup the display.
   MIL_DOUBLE PreviousDisplayEndPos = 0;
   for (MIL_INT i = 0; i < m_NumCameras; i++)
      {
      m_MilDisplays[i].Control(M_WINDOW_INITIAL_POSITION_X, PreviousDisplayEndPos);
      m_MilDisplays[i].Control(M_UPDATE_SYNCHRONIZATION, M_SYNCHRONOUS);

      m_MilDisplays[i].Zoom(m_DisplayInfo[i].ZoomFactorX, m_DisplayInfo[i].ZoomFactorY);
      PreviousDisplayEndPos =  (MIL_DOUBLE)(m_DisplayInfo[i].DigitizerInfo.SizeX * m_DisplayInfo[i].ZoomFactorX);
      }

   m_MilResultsDisplay.Zoom(m_DisplayInfo[0].ZoomFactorX, m_DisplayInfo[0].ZoomFactorY);
   }

//*******************************************************************************
// Frees the MIL display.
//*******************************************************************************
void CExampleManagerFor3D::FreeMILDisplay()
   {
   if (m_MilDisplays)
      {
      for (MIL_INT i = 0; i < m_NumCameras; i++)
         {
         MbufFree(m_MilDisplayImages[i]);
         m_MilDisplayImages[i] = M_NULL;

         m_MilDisplays[i].Free();
         }

      delete[] m_MilDisplays;
      m_MilDisplays = NULL;
      m_MilResultsDisplay.Free();

      if (m_IllustrationInfo[0].NumIllustrations > 0)
         {
         for (MIL_INT i = 0; i < MAX_NB_ILLUSTRATIONS_PER_STEP; i++)
            {
            m_IllustrationsDisplay[i].Free();
            MbufFree(m_MilIllustrationsImage[i]);
            m_MilIllustrationsImage[i] = M_NULL;
            }
         }

      delete [] m_MilDisplays;
      m_MilDisplays = M_NULL;
      }
   }

//*******************************************************************************
// Sets up the MIL graphics objects
//*******************************************************************************
void CExampleManagerFor3D::SetupGraphics()
   {
   for (MIL_INT i = 0; i < m_NumCameras; i++)
      {
      MgraAlloc(m_MilSystem, &m_MilGraphics[i]);
      MgraControl(m_MilGraphics[i], M_BACKGROUND_MODE, M_TRANSPARENT);
      MgraControl(m_MilGraphics[i], M_FONT_SIZE, TEXT_FONT_SIZE_MEDIUM);

      MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);  
      MgraFont(m_MilGraphics[i], TEXT_FONT_NAME);
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);   

      // Associate the graphic list to the display for annotations.
      MgraAllocList(m_MilSystem, M_DEFAULT, &m_MilGraphicList[i]);
      m_MilDisplays[i].Control(M_ASSOCIATED_GRAPHIC_LIST_ID, (MIL_DOUBLE)m_MilGraphicList[i]);
      }
   }

//*******************************************************************************
// Calibrate the camera-laser pair contexts.
//*******************************************************************************
bool CExampleManagerFor3D::CalibrateSheetOfLight(const SCameraLaserInfo* pCameraLaserInfo,
                                                 MIL_ID* pCameraCalibrations,
                                                 MIL_ID* pOutCameraLaserCtxs)
   {
   bool LaserCalibrationSuccessful = false;

   // Annotations settings.
   for (MIL_INT i = 0; i < m_NumCameras; i++)
      {
      MgraClear(M_DEFAULT, m_MilGraphicList[i]);
      MgraControl(m_MilGraphics[i], M_FONT_SIZE, TEXT_FONT_SIZE_MEDIUM);
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
      MgraFont(m_MilGraphics[i], TEXT_FONT_NAME);
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
      MgraColor(m_MilGraphics[i], REF_PLANE_COLOR);
      }

   ShowStepIllustrations(eLaserCalibration,
                         m_MilDisplays[0].GetDisplaySizeX(), 
                         m_MilDisplays[0].GetDisplaySizeY());

   m_NumLasersPerImage      = pCameraLaserInfo[0].NumLasersPerImage;
   m_NumCameraLaserContexts = m_NumLasersPerImage * m_NumCameras;

   // Array to hold the camera calibration id for each camera/laser context.
   MIL_ID* pCameraCalibrationIds = new MIL_ID[m_NumCameraLaserContexts];

   // Initialize the contexts.
   MIL_INT CameraIdx = -1;
   MIL_INT LastCameraLbl = -1;
   for (MIL_INT i = 0; i < m_NumCameraLaserContexts; i++)
      {
      M3dmapAlloc(m_MilSystem,
                  M_LASER,
                  M_CALIBRATED_CAMERA_LINEAR_MOTION +
                  M_CAMERA_LABEL(pCameraLaserInfo[i].CameraLabel) +
                  M_LASER_LABEL(pCameraLaserInfo[i].LaserLabel),
                  &pOutCameraLaserCtxs[i]);

      // Laser line extraction settings.
      MIL_ID LocatePeakCtx = M_NULL;
      M3dmapInquire(pOutCameraLaserCtxs[i], M_DEFAULT, M_LOCATE_PEAK_1D_CONTEXT_ID + M_TYPE_MIL_ID, &LocatePeakCtx);
      MimControl(LocatePeakCtx, M_SCAN_LANE_DIRECTION, pCameraLaserInfo[i].CalScanOrientation);
      MimControl(LocatePeakCtx, M_PEAK_WIDTH_NOMINAL , pCameraLaserInfo[i].CalPeakWidthNominal);
      MimControl(LocatePeakCtx, M_PEAK_WIDTH_DELTA   , pCameraLaserInfo[i].CalPeakWidthDelta);
      MimControl(LocatePeakCtx, M_MINIMUM_CONTRAST   , pCameraLaserInfo[i].CalMinContrast);

      if (pCameraLaserInfo[i].LineExtractionInROI != eLineNoROI)
         {
         const SLineExtractionInROI& LineInfo = pCameraLaserInfo[i].LineExtractionInROIInfo;

         M3dmapControl(pOutCameraLaserCtxs[i], M_DEFAULT, M_EXTRACTION_CHILD_OFFSET_X, (MIL_DOUBLE) LineInfo.OffsetX);
         M3dmapControl(pOutCameraLaserCtxs[i], M_DEFAULT, M_EXTRACTION_CHILD_OFFSET_Y, (MIL_DOUBLE) LineInfo.OffsetY);
         }

      // Fill the array of camera calibration MIL_IDs to be used in M3dmapCalibrateMulti.
      // We may have many times the same camera calibration MIL_ID.   
      if (LastCameraLbl != pCameraLaserInfo[i].CameraLabel)
         {
         LastCameraLbl = pCameraLaserInfo[i].CameraLabel;
         ++CameraIdx;
         }
      pCameraCalibrationIds[i] = pCameraCalibrations[CameraIdx];
      }

   //...............................
   // Extract the calibration lines.

   MosPrintf(MIL_TEXT("Adding scans for the laser-profiling system ")
             MIL_TEXT("calibration...\n\n"));

   for (MIL_INT i = 0; i < m_NumCameras; i++)
      { MbufClear(m_MilDisplayImages[i], 0); }

   // Array of result objects to hold the laser line on different reference plane.
   MIL_ID* pLaserLineRes = new MIL_ID[m_NumCameraLaserContexts];
   for (MIL_INT i = 0; i < m_NumCameraLaserContexts; i++)  
      {
      // Allocate the object to hold the laser lines at different heights.
      pLaserLineRes[i] = M3dmapAllocResult(m_MilSystem, M_LASER_CALIBRATION_DATA, M_DEFAULT, M_NULL);
      }

   // Add laser line images at different heights for laser-plane calibration.
   CameraIdx = 0;
   LastCameraLbl = -1;
   MIL_CONST_TEXT_PTR pCalLaserPlaneStr = MIL_TEXT("Calibrating laser plane");
   for (MIL_INT i = 0; i < m_NumCameras; i++)
      {
      MIL_INT NumRefPlanes = pCameraLaserInfo[i].CalNbRefPlanes;
      MIL_INT LaserIdx = i * m_NumLasersPerImage;
      for (MIL_INT j = 0; j < NumRefPlanes; j++)
         {
         MIL_CONST_TEXT_PTR pLineImgFilename = pCameraLaserInfo[i].LaserCalibrationPlanes[j].RefImageName;
         MbufImport(pLineImgFilename, M_DEFAULT, M_LOAD, M_NULL, &m_MilDisplayImages[CameraIdx]);

         for (MIL_INT k = 0; k < m_NumLasersPerImage; k++)
            {
            MdispControl(m_MilDisplays[CameraIdx].GetDisplayID(), M_UPDATE_GRAPHIC_LIST, M_DISABLE);
            MgraClear(M_DEFAULT, m_MilGraphicList[CameraIdx]);

            MIL_ID LineImageToUse = m_MilDisplayImages[CameraIdx];
            if(pCameraLaserInfo[LaserIdx].LineExtractionInROI == eLineChildROI)
               {
               const SLineExtractionInROI& LineInfo = pCameraLaserInfo[LaserIdx + k].LineExtractionInROIInfo;
               LineImageToUse = MbufChild2d(m_MilDisplayImages[CameraIdx],
                                            LineInfo.OffsetX,
                                            LineInfo.OffsetY,
                                            LineInfo.SizeX,
                                            LineInfo.SizeY,
                                            M_NULL);

               pCalLaserPlaneStr = MIL_TEXT("Calibrating laser plane in ROI");
               MgraColor(m_MilGraphics[CameraIdx], REF_PLANE_ROI_COLOR);
               MgraRectAngle(m_MilGraphics[CameraIdx], m_MilGraphicList[i], LineInfo.OffsetX, LineInfo.OffsetY, LineInfo.SizeX, LineInfo.SizeY, 0.0, M_CORNER_AND_DIMENSION);
               MgraColor(m_MilGraphics[CameraIdx], REF_PLANE_COLOR);
               }

            const SRefPlaneInfo& RefPlaneInfo = pCameraLaserInfo[LaserIdx + k].LaserCalibrationPlanes[j];
            MgraText(m_MilGraphics[CameraIdx], m_MilGraphicList[i], TEXT_OFFSET_X, TEXT_OFFSET_Y, pCalLaserPlaneStr);

            MIL_TEXT_CHAR ZString[MAX_STRING_LEN];
            MosSprintf(ZString, MAX_STRING_LEN, MIL_TEXT("Z = %.1f"), RefPlaneInfo.Z);
            MgraText(m_MilGraphics[i], m_MilGraphicList[i], TEXT_OFFSET_X + (k * 800), TEXT_OFFSET_Y + 50, ZString);

            // Add laser line image to result.
            M3dmapControl(pOutCameraLaserCtxs[i], M_DEFAULT, M_CORRECTED_DEPTH, RefPlaneInfo.Z);
            M3dmapAddScan(pOutCameraLaserCtxs[i], pLaserLineRes[LaserIdx + k],
                          LineImageToUse, M_NULL, M_NULL, M_DEFAULT, M_DEFAULT);

            // Free child buffer if necessary.
            if(LineImageToUse != m_MilDisplayImages[CameraIdx])
               { MbufFree(LineImageToUse); LineImageToUse = M_NULL; }

            // Delay execution for display.
            MdispControl(m_MilDisplays[CameraIdx].GetDisplayID(), M_UPDATE_GRAPHIC_LIST, M_ENABLE);
            MosSleep(100);
            }
         }

      if (LastCameraLbl != pCameraLaserInfo[i].CameraLabel)
         {
         LastCameraLbl = pCameraLaserInfo[i].CameraLabel;
         ++CameraIdx;
         }
      }

   // Calibrate the sheet-of-light profiling system for all camera/laser pairs (contexts).
   M3dmapCalibrateMultiple(pOutCameraLaserCtxs, pLaserLineRes, pCameraCalibrationIds, m_NumCameraLaserContexts, M_DEFAULT);

   // Check if the calibration was successful.
   LaserCalibrationSuccessful = true;
   for(MIL_INT j = 0; j < m_NumCameraLaserContexts && LaserCalibrationSuccessful; j++)
      {
      MIL_INT CalibrationStatus = M3dmapInquire(pOutCameraLaserCtxs[j], M_DEFAULT, M_CALIBRATION_STATUS, M_NULL);
      LaserCalibrationSuccessful = (CalibrationStatus == M_CALIBRATED && LaserCalibrationSuccessful);
      }

   if (LaserCalibrationSuccessful)
      {      
      for (MIL_INT i = 0; i < m_NumCameras; i++)
         {
         MgraClear(M_DEFAULT, m_MilGraphicList[i]);
         MgraText(m_MilGraphics[i],
                  m_MilGraphicList[i],
                  TEXT_OFFSET_X, 
                  TEXT_OFFSET_Y,
                  MIL_TEXT("3D Calibration successful"));
         }

      MosPrintf(MIL_TEXT("The sheet-of-light profiling system has been calibrated ")
                MIL_TEXT("using\nthe laser line images.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue...\n\n"));
      MosGetch();
      }
   else
      {
      MosPrintf(MIL_TEXT("The sheet-of-light calibration could not be properly completed.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   // Free laser line results.
   for (MIL_INT i = 0; i < m_NumCameraLaserContexts; i++)
      {
      M3dmapFree(pLaserLineRes[i]);
      pLaserLineRes[i] = M_NULL;
      }

   // Free arrays.
   delete [] pLaserLineRes;
   pLaserLineRes = NULL;

   delete [] pCameraCalibrationIds;
   pCameraCalibrationIds = NULL;

   return LaserCalibrationSuccessful;
   }

struct SCamGrab
   {
   SDigInfo DigInfo;
   MIL_ID   LaserLineImage;
   };

struct SGrabThr
   {
   MIL_INT                       NbCameras;
   MIL_INT                       NbLaserPerImage;
   MIL_ID                        CameraLaserCtx          [MAX_NB_CAMERAS * MAX_NB_LASERS];
   MIL_INT                       PointCloudLabel         [MAX_NB_CAMERAS * MAX_NB_LASERS];
   SCamGrab                      Camera                  [MAX_NB_CAMERAS];
   LineROIExtctEnum              LineExtractionInROI;
   SLineExtractionInROI          LineROIExtractionInfo   [MAX_NB_LASERS];
   MIL_ID                        UsedLaserLineImage      [MAX_NB_CAMERAS * MAX_NB_LASERS];
   C3DDisplayManager*            p3dDisplay;
   MIL_ID                        PtCldCtnr;

   IContinuousAnalyzer*          pContinuousAnalyzer;
   IAnalyzeDepthMap*             pAnalysisObj;
   MIL_INT                       NbFramesPerAnalysis;
   };

//*******************************************************************************
// Grab (simulated from reading a sequence file (.avi).
//*******************************************************************************
MIL_UINT32 MFTYPE GrabLaserLineSequences(void* pUserDataPtr)
   {
   SGrabThr& ThrData = *(static_cast<SGrabThr*>(pUserDataPtr));

   MIL_ID DepthMapForContinuousAnalysis = M_NULL;
   MIL_CONST_TEXT_PTR SeqFilenameArray[MAX_NB_CAMERAS];
   MIL_INT MaxNbFrames = -1;

   for(MIL_INT c = 0; c < ThrData.NbCameras; c++)
      {
      SeqFilenameArray[c] = ThrData.Camera[c].DigInfo.DigFormat;
      MbufImportSequence(SeqFilenameArray[c], M_DEFAULT, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL, M_OPEN);

      if(ThrData.Camera[c].DigInfo.NbFrames > MaxNbFrames)
         { MaxNbFrames = ThrData.Camera[c].DigInfo.NbFrames; }
      
      for(MIL_INT k = 0; k < ThrData.NbLaserPerImage; k++)
         {
         MIL_INT PtCldIdx = (c * ThrData.NbLaserPerImage) + k;
         const SLineExtractionInROI& LineInfo = ThrData.LineROIExtractionInfo[PtCldIdx];

         ThrData.UsedLaserLineImage[PtCldIdx] = ThrData.Camera[c].LaserLineImage;
         if(ThrData.LineExtractionInROI == eLineChildROI)
            {
            ThrData.UsedLaserLineImage[PtCldIdx] = 
            MbufChild2d(ThrData.Camera[c].LaserLineImage,
                        LineInfo.OffsetX,
                        LineInfo.OffsetY,
                        LineInfo.SizeX,
                        LineInfo.SizeY, 
                        M_NULL);
            }
         }
      }

   bool ContinuousLoop = (ThrData.pContinuousAnalyzer != NULL);
   MIL_INT ContinuousFrame = 0;

   // Acquires each image frame.
   MIL_INT f = 0;
   bool ContinueGrab = true;
   while((f < MaxNbFrames) && ContinueGrab)
      {
      if(ThrData.p3dDisplay != NULL)
         { ThrData.p3dDisplay->Lock(); }

      // For each camera.
      for(MIL_INT c = 0; c < ThrData.NbCameras; c++)
         {
         if(f < ThrData.Camera[c].DigInfo.NbFrames)
            {
            MbufImportSequence(SeqFilenameArray[c], M_DEFAULT, M_LOAD, M_NULL, &ThrData.Camera[c].LaserLineImage, f, 1, M_READ);

            MIL_INT NbLasersPerImage = ThrData.NbLaserPerImage;
            for(MIL_INT k = 0; k < NbLasersPerImage; k++)
               {
               MIL_INT PtCldIdx = (c * NbLasersPerImage) + k;

               M3dmapAddScan(ThrData.CameraLaserCtx[PtCldIdx],
                             ThrData.PtCldCtnr,
                             ThrData.UsedLaserLineImage[PtCldIdx],
                             M_NULL,
                             M_NULL,
                             M_POINT_CLOUD_LABEL(PtCldIdx+1),
                             M_DEFAULT);
               }
            }
         }

      if(ThrData.pContinuousAnalyzer)
         {
         if(0 == ContinuousFrame)
            { ThrData.pContinuousAnalyzer->AnalyzeDepthMapContinuous(ThrData.PtCldCtnr, ThrData.pAnalysisObj); }
         ContinuousFrame = ((ContinuousFrame + 1) % ThrData.NbFramesPerAnalysis);
         }         

      if(ThrData.p3dDisplay != NULL)
         { ThrData.p3dDisplay->Unlock(); }
            
      // Yield some time to other threads (for CPUs with limited number of cores).
      MosSleep(1);
     
      ++f;
      if(ContinuousLoop)
         {
         // Make the acquisition infinite without user intervention.
         f = (f % MaxNbFrames);

         // Pressing ENTER will end the continuous acquisition.
         if(UserPressedEnter())
            {
            ContinueGrab = false;
            MosPrintf(MIL_TEXT(" Acquisition stopped.\n"));
            }
         }
      else         
         {
         // Pressing ENTER will end the continuous acquisition. 
         if(UserPressedEnter())
            {
            ContinueGrab = false;
            MosPrintf(MIL_TEXT(" Acquisition stopped.\n"));
            }
         else
            PrintGrabProgress(f, MaxNbFrames);
         }

      }

   // Close all opened sequence files.
   for(MIL_INT c = 0; c < ThrData.NbCameras; c++)
      { MbufImportSequence(SeqFilenameArray[c], M_DEFAULT, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL, M_CLOSE); }

   return 0;
   }
//*******************************************************************************
// Perform the point cloud acquisition while displaying the scanning process.
// Some parameters are used only if AcquireMode is eScanWithContinuousAnalysis.
//*******************************************************************************
bool CExampleManagerFor3D::AcquirePointCloud(PointCloudAcquisitionModeEnum AcquireMode,
                                             const SPointCloudAcquisitionInfo* pScanInfo,
                                             MIL_ID* pCameraLaserCtxs,
                                             MIL_ID* pOutPointCloudContainer,                                             
                                             IAnalyzeDepthMap* pContinuousAnalysisObj /*= NULL*/,
                                             MIL_INT NbFramePerContinuousAnalysis /*= 100*/)
   {
   // Allocate the point cloud container.
   M3dmapAllocResult(m_MilSystem, M_POINT_CLOUD_RESULT, M_DEFAULT, pOutPointCloudContainer);
   MIL_ID PtCldCtnr = *pOutPointCloudContainer;

   M3dmapControl(PtCldCtnr, M_GENERAL, M_MAX_FRAMES               , pScanInfo->CameraMaxFrames   );
   M3dmapControl(PtCldCtnr, M_GENERAL, M_RESULTS_DISPLACEMENT_MODE, pScanInfo->CameraDisplacementMode);

   // Set parameters to the camera laser contexts for laser extraction.
   for (MIL_INT i = 0;  i < m_NumCameraLaserContexts; i++)
      {
      MIL_ID LocatePeakCtx = M_NULL;
      M3dmapInquire(pCameraLaserCtxs[i], M_DEFAULT, M_LOCATE_PEAK_1D_CONTEXT_ID + M_TYPE_MIL_ID, &LocatePeakCtx);
      MimControl(LocatePeakCtx, M_PEAK_WIDTH_NOMINAL, (MIL_DOUBLE) pScanInfo->CameraMapPeakWidth[i]);
      MimControl(LocatePeakCtx, M_PEAK_WIDTH_DELTA  , (MIL_DOUBLE) pScanInfo->CameraMapPeakWidthDelta[i]);
      MimControl(LocatePeakCtx, M_MINIMUM_CONTRAST  , (MIL_DOUBLE) pScanInfo->CameraMapMinContrast[i]);

      M3dmapControl(pCameraLaserCtxs[i], M_DEFAULT, M_SCAN_SPEED, (MIL_DOUBLE)pScanInfo->CameraMapScanSpeed[i]);
      if (pScanInfo->LineExtractionInROI != eLineNoROI)
         {
         M3dmapControl(pCameraLaserCtxs[i], M_DEFAULT, M_EXTRACTION_CHILD_OFFSET_X, pScanInfo->ChildExtractionInfo[i].OffsetX);
         M3dmapControl(pCameraLaserCtxs[i], M_DEFAULT, M_EXTRACTION_CHILD_OFFSET_Y, pScanInfo->ChildExtractionInfo[i].OffsetY);
         }
      }

   // Clear the displays.
   for (MIL_INT i = 0; i < m_NumCameras; i++)
      {
      MbufClear(m_MilDisplayImages[i], 0);

      m_MilDisplays[i].Hide();
      m_MilDisplays[i].Control(M_TITLE, MIL_TEXT(" "));

      MgraClear(M_DEFAULT, m_MilGraphicList[i]);
      }

   if (AcquireMode == eScanWithContinuousAnalysis)
      {
      pContinuousAnalysisObj->AllocProcessingObjects(m_MilSystem);
      m_MilResultsDisplay.Control(M_TITLE, MIL_TEXT(" "));

      // Associate the graphics list to the results display in continuous mode.
      for (MIL_INT i = 0; i < m_NumCameras; i++)
         { m_MilDisplays[i].Control(M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL); }

      m_MilResultsDisplay.Control(M_ASSOCIATED_GRAPHIC_LIST_ID, (MIL_DOUBLE)m_MilGraphicList[0]);
      m_MilResultsDisplay.Control(M_WINDOW_INITIAL_POSITION_X, (MIL_DOUBLE)(M3D_DISPLAY_SIZE_X+30));
      }

   C3DDisplayManager DispScan3d;
   bool DisplayIn3d = DispScan3d.Alloc(m_MilSystem, pCameraLaserCtxs, m_NumCameraLaserContexts, &pScanInfo->MapVisualizationData);

   if (AcquireMode == eScanWithContinuousAnalysis)
      {
      m_MilResultsDisplay.Control(M_WINDOW_INITIAL_POSITION_Y, 
                                  (MIL_DOUBLE)(MbufInquire(m_MilDisplayImages[0], M_SIZE_Y, M_NULL)));
      }

   // Build the acquisition thread data.
   MIL_ID GrabThr = M_NULL;
   SGrabThr GrabThrData;
   GrabThrData.p3dDisplay          = DisplayIn3d ? &DispScan3d : NULL;
   GrabThrData.PtCldCtnr           = PtCldCtnr;
   GrabThrData.NbCameras           = m_NumCameras;
   GrabThrData.NbLaserPerImage     = m_NumLasersPerImage;
   GrabThrData.pAnalysisObj        = (AcquireMode == eScanWithContinuousAnalysis) ? pContinuousAnalysisObj : NULL;
   GrabThrData.pContinuousAnalyzer = (AcquireMode == eScanWithContinuousAnalysis) ? this : NULL;
   GrabThrData.NbFramesPerAnalysis = NbFramePerContinuousAnalysis;

   for(MIL_INT c = 0; c < m_NumCameras; c++)
      {
      GrabThrData.Camera[c].DigInfo = pScanInfo->DigInfo[c];

      // Allocate buffers.
      GrabThrData.Camera[c].LaserLineImage = MbufAllocColor(m_MilSystem,
                                                            pScanInfo->DigInfo[c].SizeBand,
                                                            pScanInfo->DigInfo[c].SizeX,
                                                            pScanInfo->DigInfo[c].SizeY,
                                                            pScanInfo->DigInfo[c].Type,
                                                            M_IMAGE + M_PROC + M_DISP,
                                                            M_NULL);

      for(MIL_INT k = 0; k < m_NumLasersPerImage; k++)
         {
         MIL_INT CamLaserPairIdx = (m_NumLasersPerImage * c + k);

         GrabThrData.PointCloudLabel[CamLaserPairIdx]    = CamLaserPairIdx + 1;
         GrabThrData.CameraLaserCtx[CamLaserPairIdx]     = pCameraLaserCtxs[CamLaserPairIdx];
         GrabThrData.UsedLaserLineImage[CamLaserPairIdx] = M_NULL;
         GrabThrData.LineExtractionInROI                 = pScanInfo->LineExtractionInROI;
         GrabThrData.LineROIExtractionInfo[k]            = pScanInfo->ChildExtractionInfo[k];
         
         }
      }

   // Build and prepare the needed data to refresh the 3d display.
   MIL_ID*  pLastGrabImages   = new MIL_ID [m_NumCameraLaserContexts];
   MIL_ID*  pPointCloudCntrs  = DisplayIn3d ? (new MIL_ID [m_NumCameraLaserContexts]) : NULL;
   MIL_INT* pPointCloudLabels = DisplayIn3d ? (new MIL_INT[m_NumCameraLaserContexts]) : NULL;

   MIL_INT CamIdx = 0;
   for(MIL_INT s = 0; s < m_NumCameraLaserContexts; s++)
      {
      if(DisplayIn3d)
         {
         if(0 == s)
            {
            pPointCloudCntrs[0] = *pOutPointCloudContainer;
            pPointCloudLabels[0] = (m_NumCameras == 1) ? (s+1) : M_ALL;
            }
         else
            {
            pPointCloudCntrs[s]  = *pOutPointCloudContainer;
            pPointCloudLabels[s] = s+1;
            }
         }
      }

   // Starting the acquisition thread.
   MosPrintf(pScanInfo->ScanDisplayText);
   MosPrintf(MIL_TEXT("\nSimulating 3D point cloud acquisition...\n"));
   MosPrintf(MIL_TEXT("   * Note that the scan speed is slower than a typical camera-laser setup,\n"));
   MosPrintf(MIL_TEXT("     due to live 3D display, AVI sequence decompression and disk access.\n"));

   if(AcquireMode == eScanWithContinuousAnalysis)
      { MosPrintf(MIL_TEXT("Press ENTER to end continuous acquisition.")); }
   else if(DisplayIn3d)
      { MosPrintf(MIL_TEXT("Press ENTER to cancel live 3D display.\n")); }

   // Starts the thread.
   MthrAlloc(m_MilSystem, M_THREAD, M_DEFAULT, GrabLaserLineSequences, &GrabThrData, &GrabThr);

   // Wait for the threads to have created all the point clouds.
   MIL_INT ExpectedNbPtCld = m_NumCameraLaserContexts;
   MIL_INT NbCreatedPtCld = 0;
   while(NbCreatedPtCld < ExpectedNbPtCld)
      {
      M3dmapInquire(*pOutPointCloudContainer, M_GENERAL, M_NUMBER_OF_POINT_CLOUDS + M_TYPE_MIL_INT, &NbCreatedPtCld);
      // Give some time for the grab thread to start.
      MosSleep(30);
      }

   // Get the MIL_IDs of the buffers used in the grab thread.
   for(MIL_INT s = 0; s < m_NumCameraLaserContexts; s++)
      { pLastGrabImages[s] = GrabThrData.UsedLaserLineImage[s]; }

   bool FinalDispUpdate = false;
   bool Display3dCanceled = false;
   if(DisplayIn3d)
      {
      ShowStepIllustrations(eObjectScan,
                            DispScan3d.GetDisplaySizeX(), 
                            DispScan3d.GetDisplaySizeY() / ((AcquireMode == eScanWithContinuousAnalysis) ? 1 : 2));

      // Now update the 3d display while the point cloud acquisition is done in parallel.
      MIL_DOUBLE Delay3dUpdateSec = (1.0 / MIL_DOUBLE(pScanInfo->D3DSysInfo.D3DDisplayRefreshPerSec));
      MIL_DOUBLE LastUpdateSec = 0.0;

      bool AcquisitionDone = false;


      MIL_UNIQUE_3DGEO_ID MilBox = M3dgeoAlloc(m_MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      M3dgeoBox(MilBox, M_CORNER_AND_DIMENSION,
                pScanInfo->MapVisualizationData.BoxCornerX,
                pScanInfo->MapVisualizationData.BoxCornerY,
                pScanInfo->MapVisualizationData.BoxCornerZ,
                pScanInfo->MapVisualizationData.BoxSizeX,
                pScanInfo->MapVisualizationData.BoxSizeY,
                pScanInfo->MapVisualizationData.BoxSizeZ, M_DEFAULT);

      MIL_UNIQUE_BUF_ID MilContainer1 = MbufAllocContainer(m_MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
      MIL_UNIQUE_BUF_ID MilContainer2 = MbufAllocContainer(m_MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
      std::vector<MIL_ID> MilContainers(NbCreatedPtCld);
      for(MIL_INT i = 0; i < NbCreatedPtCld; ++i)
         {
         MilContainers[i] = MbufAllocContainer(M_DEFAULT_HOST, M_PROC, M_DEFAULT, M_NULL);
         }

      std::vector<MIL_ID> MilMatrix(NbCreatedPtCld);
      std::vector<MIL_DOUBLE> values;
      MIL_DOUBLE MinZ = pScanInfo->MapVisualizationData.BoxCornerZ;
      MIL_DOUBLE MaxZ = pScanInfo->MapVisualizationData.BoxCornerZ + pScanInfo->MapVisualizationData.BoxSizeZ;
      if(MaxZ < MinZ)
         { std::swap(MinZ, MaxZ); }

      for(MIL_INT i = 0; i < NbCreatedPtCld; ++i)
         {
         MilMatrix [i]= M3dgeoAlloc(m_MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_NULL);
         McalGetCoordinateSystem(pPointCloudCntrs[i], M_RELATIVE_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM, M_HOMOGENEOUS_MATRIX,
                                 MilMatrix[i],
                                 M_NULL, M_NULL, M_NULL, M_NULL);
         }
      while(!AcquisitionDone)
         {
         if(AcquireMode != eScanWithContinuousAnalysis)
            {
            if(UserPressedEnter()&& !Display3dCanceled)
               {
               Display3dCanceled = true;
               MosPrintf(MIL_TEXT(" Live 3d display canceled by user. Please wait...\n"));
               }
            }

         // Check to update the 3d display.
         MIL_DOUBLE NowSec   = MappTimer(M_TIMER_READ, M_NULL);
         MIL_DOUBLE DelaySec = (NowSec - LastUpdateSec);

         bool RefreshDelayExpired = ((DelaySec >= Delay3dUpdateSec) && !Display3dCanceled);
        
         if(RefreshDelayExpired || FinalDispUpdate)
            {
            DispScan3d.Lock();
            for(MIL_INT i = 0; i < NbCreatedPtCld; ++i)
               {
               M3dmapCopyResult(pPointCloudCntrs[i], M_POINT_CLOUD_INDEX(i), MilContainers[i], M_POINT_CLOUD + M_ABSOLUTE_COORDINATE_SYSTEM, M_NO_REFLECTANCE);
               }
            DispScan3d.Unlock();
               
            M3dimMerge(MilContainers, MilContainer1, (MIL_INT)NbCreatedPtCld, M_NULL, M_DEFAULT);

            DispScan3d.Disable();
            M3dimCrop(MilContainer1, MilContainer2, MilBox, M_NULL, M_SAME, M_DEFAULT);
            DispScan3d.Enable();
          
            // Update 3D display.
            LastUpdateSec = NowSec;
            DispScan3d.Show(MilContainer2, MinZ, MaxZ);

            if(FinalDispUpdate)
               AcquisitionDone = true;
            }

         MIL_INT ThrState = MthrWait(GrabThr, M_THREAD_END_WAIT + M_THREAD_TIMEOUT(1), M_NULL);
         FinalDispUpdate = (ThrState == M_SIGNALED);
         }

      for(MIL_INT i = 0; i < NbCreatedPtCld; ++i)
         {
         MbufFree(MilContainers[i]);
         M3dgeoFree(MilMatrix[i]);
         }
      }
   else
      {
      // If for any reason the display in 3d is not available, we display the laser scan lines directly.
      for(MIL_INT c = 0; c < m_NumCameras; c++)
         {
         m_MilDisplays[c].SetDisplayBufferID(pLastGrabImages[c]);
         m_MilDisplays[c].Show();
         }

      ShowStepIllustrations(eObjectScan,
                            m_MilDisplays[0].GetDisplaySizeX(), 
                            m_MilDisplays[0].GetDisplaySizeY());

      // Wait for the acquisition thread.
      MthrWait(GrabThr, M_THREAD_END_WAIT + M_THREAD_TIMEOUT(M_INFINITE), M_NULL);

      for(MIL_INT c = 0; c < m_NumCameras; c++)
         {
         m_MilDisplays[c].Hide();
         }
      }

   if(AcquireMode != eScanWithContinuousAnalysis && !Display3dCanceled)
      {
      MosPrintf(MIL_TEXT("Acquisition done. Press ENTER to continue.\n\n"));
      MosGetch();
      }

   if(DisplayIn3d)
      {
      DispScan3d.Hide();
      DispScan3d.Free();
      }

   // Free all images used in grab threads.
   for(MIL_INT c = 0; c < m_NumCameras; c++)
      {
      for(MIL_INT k = 0; k < m_NumLasersPerImage; k++)
         {
         MIL_INT CamLaserPairIdx = (m_NumLasersPerImage * c + k);
         if(GrabThrData.UsedLaserLineImage[CamLaserPairIdx] != GrabThrData.Camera[c].LaserLineImage)
            {
            MbufFree(GrabThrData.UsedLaserLineImage[CamLaserPairIdx]);
            GrabThrData.UsedLaserLineImage[CamLaserPairIdx] = M_NULL;
            }
         }

      MbufFree(GrabThrData.Camera[c].LaserLineImage);
      GrabThrData.Camera[c].LaserLineImage = M_NULL;
      }

   MthrFree(GrabThr);
   GrabThr = M_NULL;

   if (AcquireMode == eScanWithContinuousAnalysis)
      {
      pContinuousAnalysisObj->FreeProcessingObjects();
      }

   delete [] pPointCloudCntrs;
   pPointCloudCntrs = NULL;

   delete [] pPointCloudLabels;
   pPointCloudLabels = NULL;

   delete [] pLastGrabImages;
   pLastGrabImages = NULL;
   return true;
   }

//*******************************************************************************
// Generates the depth map from a point cloud container.
//*******************************************************************************
bool CExampleManagerFor3D::GenerateDepthMap(MIL_ID PointCloudContainer, const SMapGeneration& GenerationInfo, MIL_ID* pOutDepthmap) const
   {
   MIL_UNIQUE_BUF_ID ContainerId = MbufAllocContainer(m_MilSystem, M_PROC, M_DEFAULT, M_UNIQUE_ID);
   M3dmapCopyResult(PointCloudContainer, M_ALL, ContainerId, M_POINT_CLOUD_UNORGANIZED, M_NO_REFLECTANCE);
   ProjectDepthMap(m_MilSystem, ContainerId, GenerationInfo, pOutDepthmap);

   return true;
   }
//*******************************************************************************
// Generates the depth map from an M_CONTAINER.
//*******************************************************************************
bool ProjectDepthMap(MIL_ID MilSystem, MIL_ID MilContainer, const SMapGeneration& GenerationInfo, MIL_ID* pOutDepthmap)
   {

   if(M_NULL == *pOutDepthmap)
      {
      MbufAlloc2d(MilSystem,
                  GenerationInfo.MapSizeX,
                  GenerationInfo.MapSizeY,
                  16 + M_UNSIGNED,
                  M_IMAGE + M_PROC + M_DISP,
                  pOutDepthmap);
      }
   MIL_UNIQUE_3DGEO_ID MilBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoBox(MilBox, M_CORNER_AND_DIMENSION,
             GenerationInfo.BoxCornerX,
             GenerationInfo.BoxCornerY,
             GenerationInfo.BoxCornerZ,
             GenerationInfo.BoxSizeX,
             GenerationInfo.BoxSizeY,
             GenerationInfo.BoxSizeZ, M_DEFAULT);
   M3dimCrop(MilContainer, MilContainer, MilBox, M_NULL, M_UNORGANIZED, M_DEFAULT);

   M3dimCalibrateDepthMap(MilBox, *pOutDepthmap, M_NULL, M_NULL, M_DEFAULT, M_NEGATIVE, M_DEFAULT);

   M3dimProject(MilContainer, *pOutDepthmap, M_NULL, M_DEFAULT, GenerationInfo.ExtractOverlap, M_DEFAULT, M_DEFAULT);
   MIL_UNIQUE_3DIM_ID FillGapsContext = M3dimAlloc(MilSystem, M_FILL_GAPS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(FillGapsContext, M_FILL_MODE, M_X_THEN_Y);
   M3dimControl(FillGapsContext, M_FILL_SHARP_ELEVATION, M_DISABLE);
   M3dimControl(FillGapsContext, M_FILL_THRESHOLD_X, GenerationInfo.FillXThreshold);
   M3dimControl(FillGapsContext, M_FILL_THRESHOLD_Y, GenerationInfo.FillYThreshold);

   M3dimFillGaps(FillGapsContext, *pOutDepthmap, M_NULL, M_DEFAULT);
   return true;
   }

//*******************************************************************************
// Function to analyze the extracted depth map.
//*******************************************************************************
bool CExampleManagerFor3D::AnalyzeDepthMap(IAnalyzeDepthMap* pProcObj, MIL_ID Depthmap, MIL_ID PtCldCtnr , const SMapGeneration& GenerationInfo)
   {
   pProcObj->AllocProcessingObjects(m_MilSystem);

   SCommonAnalysisObjects CommonObjects;
   CommonObjects.MilSystem = m_MilSystem;   

   CommonObjects.MilGraphics = m_MilGraphics[0];
   CommonObjects.MilGraphicList = m_MilGraphicList[0];

   CommonObjects.MilPtCldCtnr = PtCldCtnr;
   CommonObjects.MilDepthMap = Depthmap;

   CommonObjects.NumLaserScanObjects = m_NumCameraLaserContexts;

   CommonObjects.MilDisplays = m_MilDisplays;
   CommonObjects.MilResultsDisplay = &m_MilResultsDisplay;

   CommonObjects.GenerationInfo = &GenerationInfo;

   ShowStepIllustrations(eObjectAnalysis,
                         (MIL_INT) (MbufInquire(Depthmap, M_SIZE_X, M_NULL) * m_DisplayInfo[0].ZoomFactorX),
                         MbufInquire(Depthmap, M_SIZE_Y, M_NULL) / 2);

   pProcObj->Analyze(CommonObjects);
   pProcObj->FreeProcessingObjects();

   return true;
   }

//*******************************************************************************
// Function to analyze the extracted depth map in 'continuous analysis' mode.
//*******************************************************************************
bool CExampleManagerFor3D::AnalyzeDepthMapContinuous(MIL_ID PtCldCntr, IAnalyzeDepthMap* pProcObj)
   {
   GenerateDepthMap(PtCldCntr, *pProcObj->GetMapGenInfo(), &m_DepthmapContinuous);

   SCommonAnalysisObjects CommonObjects;
   CommonObjects.MilSystem = m_MilSystem;   

   CommonObjects.MilGraphics = m_MilGraphics[0];
   CommonObjects.MilGraphicList = m_MilGraphicList[0];

   CommonObjects.MilPtCldCtnr = PtCldCntr;
   CommonObjects.MilDepthMap = m_DepthmapContinuous;

   CommonObjects.NumLaserScanObjects = m_NumCameraLaserContexts;

   CommonObjects.MilDisplays = m_MilDisplays;
   CommonObjects.MilResultsDisplay = &m_MilResultsDisplay;

   pProcObj->Analyze(CommonObjects);

   return true;
   }

//*******************************************************************************
// Allocate all required buffers for the display.
//*******************************************************************************
void CExampleManagerFor3D::AllocateMILDisplayObjects()
   {
   MIL_TEXT_CHAR DisplayTitle[MAX_STRING_LEN];

   for (MIL_INT i = 0; i < m_NumCameras; i++)
      {         
      m_MilDisplays[i].Alloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED);

      MosSprintf(DisplayTitle, MAX_STRING_LEN, MIL_TEXT("Camera %d"), i+1);
      m_MilDisplays[i].Control(M_TITLE, DisplayTitle);

      MbufAllocColor(m_MilSystem,
                     m_DisplayInfo[i].DigitizerInfo.SizeBand,
                     m_DisplayInfo[i].DigitizerInfo.SizeX,
                     m_DisplayInfo[i].DigitizerInfo.SizeY,
                     m_DisplayInfo[i].DigitizerInfo.Type,
                     M_IMAGE + M_DISP + M_GRAB + M_PROC, &m_MilDisplayImages[i]);
      MbufClear(m_MilDisplayImages[i], 0);
      }

   // Allocate the results display objects.
   m_MilResultsDisplay.Alloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED);

   // Allocate illustrations display objects.
   if (m_IllustrationInfo[0].NumIllustrations > 0)
      {
      for (MIL_INT i = 0; i < MAX_NB_ILLUSTRATIONS_PER_STEP; i++)
         {
         m_IllustrationsDisplay[i].Alloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED);

         MIL_CONST_TEXT_PTR pIllustFilename = m_IllustrationInfo[0].IllustrationFiles[0];
         MbufAllocColor(m_MilSystem, 3,
                        MbufDiskInquire(pIllustFilename, M_SIZE_X, M_NULL),
                        MbufDiskInquire(pIllustFilename, M_SIZE_Y, M_NULL),
                        MbufDiskInquire(pIllustFilename, M_TYPE, M_NULL),
                        M_IMAGE + M_DISP + M_PROC,
                        &(m_MilIllustrationsImage[i]));
         }
      }
   }

//*******************************************************************************
// Calibrate all the cameras.
//*******************************************************************************
bool CExampleManagerFor3D::CalibrateCameras(const SCameraCalibrationInfo* pCalibrationInfo, 
                                            MIL_INT NumCameras,
                                            MIL_ID* OutCamCalibrations)
   {
   bool AllCalibrationSuccessful = true;
   MosPrintf(MIL_TEXT("Calibrating the camera%s...\n\n"), ((NumCameras > 1) ? MIL_TEXT("s") : MIL_TEXT("")));

   // Load calibration grid image for each cameras.
   for (MIL_INT i = 0; i < NumCameras; i++)
      {
      // Get calibration grid image.
      MbufLoad(pCalibrationInfo[i].GridImageFilename, m_MilDisplayImages[i]);

      // Draw annotations.
      MgraColor(m_MilGraphics[i], CAMERA_CAL_COLOR);
      MgraText(m_MilGraphics[i], m_MilGraphicList[i], TEXT_OFFSET_X, TEXT_OFFSET_X,
               MIL_TEXT("Calibrating camera..."));

      // Show the calibration grids.
      m_MilDisplays[i].Show(m_MilDisplayImages[i]);
      }

   ShowStepIllustrations(eCameraCalibration,
                         m_MilDisplays[0].GetDisplaySizeX(),
                         m_MilDisplays[0].GetDisplaySizeY());

   // Camera calibration.
   for (MIL_INT i = 0; i < NumCameras && AllCalibrationSuccessful; i++)
      {
      const SCameraCalibrationInfo& CalInfo = pCalibrationInfo[i];

      // Allocate the calibration context.
      McalAlloc(m_MilSystem, M_TSAI_BASED, M_DEFAULT, &(OutCamCalibrations[i]));

      MIL_ID OutCal = OutCamCalibrations[i];

      // Set the corner hints.
      McalControl(OutCal, M_GRID_HINT_PIXEL_X, CalInfo.CornerHintX);
      McalControl(OutCal, M_GRID_HINT_PIXEL_Y, CalInfo.CornerHintY);

      // Perform the camera calibration.
      McalGrid(OutCal, m_MilDisplayImages[i],
               0.0, 0.0, CalInfo.OffsetZ,
               CalInfo.NbRows, CalInfo.NbCols,
               CalInfo.RowSpacing, CalInfo.ColSpacing,
               M_FULL_CALIBRATION,
               CalInfo.CalibrationType);

      MIL_INT CalibrationStatus = McalInquire(OutCal, M_CALIBRATION_STATUS, M_NULL);
      bool IsCalibrated = (CalibrationStatus == M_CALIBRATED);

      // Draw calibration annotations.
      if (IsCalibrated)
         {
         MgraClear(M_DEFAULT, m_MilGraphicList[i]);
         MgraText(m_MilGraphics[i], m_MilGraphicList[i], TEXT_OFFSET_X, TEXT_OFFSET_Y,
            MIL_TEXT("Calibration successful"));

         McalDraw(m_MilGraphics[i], OutCal, m_MilGraphicList[i],
                  M_DRAW_IMAGE_POINTS, M_DEFAULT, M_DEFAULT);
         }
      else
         {
         MgraClear(M_DEFAULT, m_MilGraphicList[i]);
         MgraText(m_MilGraphics[i], m_MilGraphicList[i], TEXT_OFFSET_X, TEXT_OFFSET_Y,
                  MIL_TEXT("Calibration failed"));
         }

      AllCalibrationSuccessful = AllCalibrationSuccessful && IsCalibrated;
      }

   // Code to perform a 'relocate camera' calibration procedure.
   // It happens when the calibration grid is needed to be tilted relatively to the camera
   // in the first calibration step.
   if(AllCalibrationSuccessful)
      {
      for (MIL_INT i = 0; i < NumCameras; i++)
         {
         const SCameraCalibrationInfo& CalInfo = pCalibrationInfo[i];

         if (CalInfo.Relocate)
            {
            // delay in order to see the previous calibration results.
            MosSleep(500);

            // Relocation is needed when the camera is parallel to the conveyor.
            // A grid with 3D information (with perspective) must be provided to perform
            // a proper 3d camera calibration (so the tilted grid for the first calibration).
            // Then, in a second step, the grid is put back flat on the conveyor and the 
            // new grid position (or camera position relatively to the grid) is found.

            MIL_ID OutCal = OutCamCalibrations[i];

            MgraClear(m_MilGraphics[i], m_MilGraphicList[i]);
            if(0 == i)
               { MosPrintf(MIL_TEXT("Relocating camera%s from a grid pose flat to the conveyor...\n\n"), ((NumCameras > 1) ? MIL_TEXT("s") : MIL_TEXT(""))); }

            // Draw annotations.
            MgraColor(m_MilGraphics[i], CAMERA_CAL_COLOR);
            MgraText(m_MilGraphics[i], m_MilGraphicList[i], TEXT_OFFSET_X, TEXT_OFFSET_X, MIL_TEXT("Relocating camera pose..."));

            // Set the corner hints.
            McalControl(OutCal, M_GRID_HINT_PIXEL_X, CalInfo.RelocatedCornerHintX);
            McalControl(OutCal, M_GRID_HINT_PIXEL_Y, CalInfo.RelocatedCornerHintY);

            // Relocate calibration.
            MbufLoad(CalInfo.RelocatedGridImageFilename, m_MilDisplayImages[i]);

            // Show the calibration grid at a final different orientation.
            m_MilDisplays[i].Show(m_MilDisplayImages[i]);

            // Calibrate.
            McalGrid(OutCal, m_MilDisplayImages[i],
                     0.0, 0.0, CalInfo.OffsetZ,
                     CalInfo.NbRows, CalInfo.NbCols,
                     CalInfo.RowSpacing, CalInfo.ColSpacing,
                     M_DISPLACE_CAMERA_COORD,
                     CalInfo.CalibrationType);

            MIL_INT CalibrationStatus = McalInquire(OutCal, M_CALIBRATION_STATUS, M_NULL);
            bool IsCalibrated = (CalibrationStatus == M_CALIBRATED);

            if (IsCalibrated)
               {
               MgraClear(M_DEFAULT, m_MilGraphicList[i]);
               MgraText(m_MilGraphics[i], m_MilGraphicList[i], TEXT_OFFSET_X, TEXT_OFFSET_Y, MIL_TEXT("Relocation successful"));
               McalDraw(m_MilGraphics[i], OutCal, m_MilGraphicList[i], M_DRAW_IMAGE_POINTS, M_DEFAULT, M_DEFAULT);
               }
            else
               {
               MgraClear(M_DEFAULT, m_MilGraphicList[i]);
               MgraText(m_MilGraphics[i], m_MilGraphicList[i], TEXT_OFFSET_X, TEXT_OFFSET_Y, MIL_TEXT("Calibration failed"));
               }

            AllCalibrationSuccessful = AllCalibrationSuccessful && IsCalibrated;
            }         
         }
      }

   if (AllCalibrationSuccessful)
      {
      MosPrintf(MIL_TEXT("The camera%s calibrated using a chessboard grid.\n\n"), (m_NumCameras == 1) ? MIL_TEXT(" was") : MIL_TEXT("s were"));
      }
   else
      {
      MosPrintf(MIL_TEXT("Camera calibration could not be properly completed.\n"));
      }

   return AllCalibrationSuccessful;
   }

//*******************************************************************************
// Show the illustrations according to the example step.
//*******************************************************************************
void CExampleManagerFor3D::ShowStepIllustrations(ExampleSteps Step, 
                                                 MIL_INT DisplaySizeX,
                                                 MIL_INT DisplaySizeY)
   {
   // Hide the previous illustrations.
   if (Step != eCameraCalibration || 
       (m_IllustrationInfo[Step].NumIllustrations<=0 && Step != eCameraCalibration))
      {
      for (MIL_INT i = 0; i < m_IllustrationInfo[Step-1].NumIllustrations; i++)
         {
         m_IllustrationsDisplay[i].Hide();
         }
      }

   // Setup the illustration below.
   if (m_IllustrationInfo[Step].NumIllustrations > 0)
      {
      // Show the new illustrations.
      MIL_DOUBLE IllustrationPosX, IllustrationPosY;

      if (DisplaySizeX <= 0)
         {
         DisplaySizeX = (MIL_INT)(m_DisplayInfo[0].DigitizerInfo.SizeX*m_DisplayInfo[0].ZoomFactorX);
         IllustrationPosX = (MIL_DOUBLE)(DisplaySizeX - (MbufInquire(m_MilIllustrationsImage[0], M_SIZE_X, M_NULL) / 2));
         }
      else
         {
         IllustrationPosX = (MIL_DOUBLE)(DisplaySizeX + 20);
         }

      if (DisplaySizeY <= 0)
         {
         DisplaySizeY = (MIL_INT)(m_DisplayInfo[0].DigitizerInfo.SizeY *
                                  m_DisplayInfo[0].ZoomFactorY);
         }

      MIL_INT IllustrationSizeY = MbufInquire(m_MilIllustrationsImage[0], 
                                              M_SIZE_Y, M_NULL);

      IllustrationPosY = (MIL_DOUBLE)(DisplaySizeY - (IllustrationSizeY*m_IllustrationInfo[Step].NumIllustrations));


      for (MIL_INT i = 0; i < m_IllustrationInfo[Step].NumIllustrations; i++)
         { MbufLoad(m_IllustrationInfo[Step].IllustrationFiles[i], m_MilIllustrationsImage[i]); }

      MIL_TEXT_CHAR DisplayTitle[MAX_STRING_LEN] = MIL_TEXT("");
      switch (Step)
         {
         case eCameraCalibration:
            MosSprintf(DisplayTitle, MAX_STRING_LEN, MIL_TEXT("Camera Calibration"));
            break;
         case eLaserCalibration:
            MosSprintf(DisplayTitle, MAX_STRING_LEN, MIL_TEXT("Laser Calibration"));
            break;
         case eObjectScan:
            MosSprintf(DisplayTitle, MAX_STRING_LEN, MIL_TEXT("Object Scan"));
            break;
         case eObjectAnalysis:
            MosSprintf(DisplayTitle, MAX_STRING_LEN, MIL_TEXT("Object Analysis"));
            break;
         default:
            break;
         }

      MIL_INT NextYPos = Max((MIL_INT)IllustrationPosY, (MIL_INT)0) + 40;
      MIL_TEXT_CHAR FormattedDisplayTitle[MAX_STRING_LEN] = MIL_TEXT("");
      MIL_TEXT_CHAR DisplayTitleFormat[MAX_STRING_LEN] = MIL_TEXT("%s %d");

      for (MIL_INT i = m_IllustrationInfo[Step].NumIllustrations-1; i >= 0; i--)
         {
         m_IllustrationsDisplay[i].Control(M_WINDOW_INITIAL_POSITION_X, IllustrationPosX);
         m_IllustrationsDisplay[i].Control(M_WINDOW_INITIAL_POSITION_Y, (MIL_DOUBLE)NextYPos);

         NextYPos += (IllustrationSizeY + 40);

         MosSprintf(FormattedDisplayTitle, MAX_STRING_LEN, DisplayTitle);

         if (m_IllustrationInfo[Step].NumIllustrations > 1)
            {
            MosSprintf(FormattedDisplayTitle, MAX_STRING_LEN, DisplayTitleFormat, DisplayTitle,
                       (m_IllustrationInfo[Step].NumIllustrations-1)-i);
            }

         m_IllustrationsDisplay[i].Control(M_TITLE, FormattedDisplayTitle);
         m_IllustrationsDisplay[i].Control(M_UPDATE_SYNCHRONIZATION, M_SYNCHRONOUS);
         m_IllustrationsDisplay[i].Show(m_MilIllustrationsImage[i]);
         }
      }
   }
