﻿//*******************************************************************************
// 
// File name: CanInspection.cpp  
//
// Synopsis: Demonstrates continuous inspection of cans using 3D data.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "CanInspection.h"

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("CanInspection\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the continuous inspection of cans using")
             MIL_TEXT("\n3d sheet-of-light profiling. The system consists of two cameras")
             MIL_TEXT("\nand one laser. Note that during the setup of the grab, the ")
             MIL_TEXT("cameras\nwere synchronized so the same laser scan was provided to ")
             MIL_TEXT("all\ncameras at the same time.\n"));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Display, Buffer, Graphics, \n")
             MIL_TEXT("Image Processing, Calibration, 3D Map, Model Finder, 3D Metrology,\n"));
   MosPrintf(MIL_TEXT("3D Image Processing, 3D Display, and 3D Graphics. \n"));
   }

// Constants.
static const MIL_INT MAP_SIZE_X =  400;
static const MIL_INT MAP_SIZE_Y = 1020;

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   // Allocate the MIL application.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);

   // Initialization.
   CExampleManagerFor3D* pExampleMngrFor3D = MakeExampleManager();
   if (!pExampleMngrFor3D)
      {
      MappFree(MilApplication);
      return -1;
      }

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   //.......................................................................
   // 1. To calibrate the setup, the first step is to calibrate the cameras.
   // Camera calibration specifications.
   const MIL_DOUBLE COL_SPACING                [NUM_CAMERAS] = { 5.0, 5.0 };
   const MIL_DOUBLE ROW_SPACING                [NUM_CAMERAS] = { 5.0, 5.0 };
   const MIL_INT    NB_ROWS                    [NUM_CAMERAS] = { 16, 16 };
   const MIL_INT    NB_COLS                    [NUM_CAMERAS] = { 15, 15 };
   const MIL_DOUBLE CORNER_HINT_X              [NUM_CAMERAS] = { 1200, 750 };
   const MIL_DOUBLE CORNER_HINT_Y              [NUM_CAMERAS] = { 300, 200 };
   const MIL_DOUBLE OFFSET_Z                   [NUM_CAMERAS] = { 0.0, 0.0 };
   const MIL_INT64  CALIBRATION_TYPE           [NUM_CAMERAS] = { M_CHESSBOARD_GRID, M_CHESSBOARD_GRID};
   const MIL_CONST_TEXT_PTR GRID_IMG_FILENAME  [NUM_CAMERAS] = { EX_PATH("Cam1_grid.mim"), EX_PATH("Cam2_grid.mim") };

   // Initialize data.
   SCameraCalibrationInfo CAMERA_CALIBRATION_INFO[NUM_CAMERAS];
   for (MIL_INT c = 0; c < NUM_CAMERAS; c++)
      {
      SCameraCalibrationInfo& CCI = CAMERA_CALIBRATION_INFO[c];
      CCI.CornerHintX          = CORNER_HINT_X[c];
      CCI.CornerHintY          = CORNER_HINT_Y[c];
      CCI.OffsetZ              = OFFSET_Z[c];
      CCI.NbRows               = NB_ROWS[c];
      CCI.NbCols               = NB_COLS[c];
      CCI.RowSpacing           = ROW_SPACING[c];
      CCI.ColSpacing           = COL_SPACING[c];
      CCI.CalibrationType      = CALIBRATION_TYPE[c];
      CCI.GridImageFilename    = GRID_IMG_FILENAME[c];
      CCI.Relocate             = NO_RELOCATE;
      CCI.RelocatedGridImageFilename = NULL;
      }

   //.................................
   // 1.1 Execute cameras calibration.
   MIL_ID CameraCalibrations[NUM_CAMERAS];
   bool CameraCalibrationOk = pExampleMngrFor3D->CalibrateCameras(CAMERA_CALIBRATION_INFO, NUM_CAMERAS, &CameraCalibrations[0]);

   // 2. Then continue to calibrate the laser planes (sheets-of-light).
   if(CameraCalibrationOk)
      {
      MosPrintf(MIL_TEXT("Press <Enter> to calibrate laser planes.\n\n"));
      MosGetch();

      // Sheet-of-Light (laser plane) calibration      
      const MIL_INT    NUM_REF_PLANES                    = 5;
      const MIL_DOUBLE CAL_MIN_CONTRAST    [NUM_CAMERAS] = { 100, 100 };
      const MIL_INT    CAL_NB_REF_PLANES   [NUM_CAMERAS] = { NUM_REF_PLANES, NUM_REF_PLANES };
      const MIL_INT    CAL_SCAN_ORIENTATION[NUM_CAMERAS] = { M_HORIZONTAL, M_HORIZONTAL };
      const MIL_INT    CAL_PEAK_WIDTH      [NUM_CAMERAS] = { 5, 5 };
      const MIL_INT    CAL_PEAK_WIDTH_DELTA[NUM_CAMERAS] = { 3, 3 };
      const MIL_INT    LASER_LABELS        [NUM_CAMERAS] = { 1, 1 };
      const MIL_INT    CAMERA_LABELS       [NUM_CAMERAS] = { 1, 2 };

      const MIL_DOUBLE PLANE_Z[NUM_CAMERAS][MAX_NB_REF_PLANES] =
         {  { 11.72, 5.86, 0.00, -5.86, -11.72 },
            { 11.72, 5.86, 0.00, -5.86, -11.72 } };

      const SRefPlaneInfo LASER_CALIBRATION_PLANES[NUM_CAMERAS][MAX_NB_REF_PLANES] = 
         { { // first camera
               // RefImageName                                 Zs
               { EX_PATH("Cam1RefPlanes/Cam1_laser_h-2.mim"), PLANE_Z[0][0] },
               { EX_PATH("Cam1RefPlanes/Cam1_laser_h-1.mim"), PLANE_Z[0][1] },
               { EX_PATH("Cam1RefPlanes/Cam1_laser_h0.mim") , PLANE_Z[0][2] },
               { EX_PATH("Cam1RefPlanes/Cam1_laser_h1.mim") , PLANE_Z[0][3] },
               { EX_PATH("Cam1RefPlanes/Cam1_laser_h2.mim") , PLANE_Z[0][4] }
            },
            { // second camera
               // RefImageName                                 Zs
               { EX_PATH("Cam2RefPlanes/Cam2_laser_h-2.mim"), PLANE_Z[0][0] },
               { EX_PATH("Cam2RefPlanes/Cam2_laser_h-1.mim"), PLANE_Z[0][1] },
               { EX_PATH("Cam2RefPlanes/Cam2_laser_h0.mim") , PLANE_Z[0][2] },
               { EX_PATH("Cam2RefPlanes/Cam2_laser_h1.mim") , PLANE_Z[0][3] },
               { EX_PATH("Cam2RefPlanes/Cam2_laser_h2.mim") , PLANE_Z[0][4] }
            } };

      const MIL_INT NUM_LASERS_PER_IMAGE = 1;

      SCameraLaserInfo LASER_CALIBRATION_INFO[NUM_CAMERAS * NUM_LASERS_PER_IMAGE];
      for(MIL_INT c = 0; c < NUM_CAMERAS; c++)
         {
         SCameraLaserInfo& LCI = LASER_CALIBRATION_INFO[c];

         LCI.NumLasersPerImage  = NUM_LASERS_PER_IMAGE;
         LCI.NumRefPlanes       = NUM_REF_PLANES;
         LCI.CalMinContrast     = CAL_MIN_CONTRAST[c];
         LCI.CalNbRefPlanes     = CAL_NB_REF_PLANES[c];
         LCI.CalScanOrientation = CAL_SCAN_ORIENTATION[c];
         LCI.CalPeakWidthNominal= CAL_PEAK_WIDTH[c];
         LCI.CalPeakWidthDelta  = CAL_PEAK_WIDTH_DELTA[c];
         for(MIL_INT l = 0; l < LCI.CalNbRefPlanes; l++)
            {
            LCI.LaserCalibrationPlanes[l] = LASER_CALIBRATION_PLANES[c][l];
            }
         LCI.LaserLabel  = LASER_LABELS[c];
         LCI.CameraLabel = CAMERA_LABELS[c];
         LCI.LineExtractionInROI = eLineNoROI;
         }

      //............................................................
      // 2.1 Execute the calibration of the laser planes.
      // Generates the needed calibrated camera-laser pair contexts.
      MIL_ID CameraLaserCtxts[NUM_CAMERAS * NUM_LASERS_PER_IMAGE];
      bool SheetOfLightOk = pExampleMngrFor3D->CalibrateSheetOfLight(&LASER_CALIBRATION_INFO[0],
                                                                     &CameraCalibrations[0],
                                                                     &CameraLaserCtxts[0]);
      if (SheetOfLightOk)
         {
         // Map generation specifications.
         const MIL_DOUBLE  M3D_DISPLAY_REFRESH_PER_SEC = 0.9; // 3D Display FPS
         const MIL_DOUBLE  M3D_DISPLAY_LOOK_AT_X = 0.0;//21.26; //Orbit parameters of Move view Point
         const MIL_DOUBLE  M3D_DISPLAY_LOOK_AT_Y = 123.16;
         const MIL_DOUBLE  M3D_DISPLAY_LOOK_AT_Z = 28.0; //28.41;
         const MIL_DOUBLE  M3D_DISPLAY_EYE_DIST       = 517.23;
         const MIL_DOUBLE  M3D_DISPLAY_EYE_THETA      = 43.55;
         const MIL_DOUBLE  M3D_DISPLAY_EYE_PHI        = 56.72;
         const MIL_INT    CAMERA_MAP_MIN_CONTRAST[]   = { 100, 100};
         const MIL_INT    CAMERA_MAP_PEAK_WIDTH[]     = {  4 ,  4 };
         const MIL_INT    CAMERA_MAP_PEAK_DELTA[]     = { 16 , 16 };
         const MIL_DOUBLE CAMERA_MAP_SCAN_SPEED[]     = { 0.2927, 0.2927 };
         const MIL_DOUBLE CAMERA_MAX_FRAMES           = 670;
         const MIL_DOUBLE CAMERA_DISPLACEMENT_MODE    = M_CURRENT;
        

         // Visualization volume information.
         SMapGeneration MapData;
         MapData.BoxCornerX       = - 26.90;
         MapData.BoxCornerY       =    5.47;
         MapData.BoxCornerZ       =    1.00;
         MapData.BoxSizeX         =  118.00;
         MapData.BoxSizeY         =  300.00;
         MapData.BoxSizeZ         = - 13.00;
         MapData.MapSizeX         = MAP_SIZE_X;
         MapData.MapSizeY         = MAP_SIZE_Y;
         MapData.PixelSizeX       = MapData.BoxSizeX / (MAP_SIZE_X - 1.0);
         MapData.PixelSizeY       = MapData.BoxSizeY / (MAP_SIZE_Y - 1.0);
         MapData.GrayScaleZ       = MapData.BoxSizeZ / 65534.0;
         MapData.IntensityMapType = 8 + M_UNSIGNED;
         MapData.SetExtractOverlap= true;
         MapData.ExtractOverlap   = M_MAX_Z;
         MapData.FillXThreshold   = 1.0;
         MapData.FillYThreshold   = 1.0;

         // Scan and analyze information.
         SPointCloudAcquisitionInfo SCAN_INFO =
            {
            // SD3DSysInfo
            { M3D_DISPLAY_REFRESH_PER_SEC, SHOW_COLOR, M3D_DISPLAY_LOOK_AT_X, M3D_DISPLAY_LOOK_AT_Y, M3D_DISPLAY_LOOK_AT_Z, M3D_DISPLAY_EYE_DIST, M3D_DISPLAY_EYE_THETA, M3D_DISPLAY_EYE_PHI },
            { CAMERA_MAP_MIN_CONTRAST[0] , CAMERA_MAP_MIN_CONTRAST[1] },
            { CAMERA_MAP_PEAK_WIDTH[0]   , CAMERA_MAP_PEAK_WIDTH[1]   },
            { CAMERA_MAP_PEAK_DELTA[0]   , CAMERA_MAP_PEAK_DELTA[1]   },
            { CAMERA_MAP_SCAN_SPEED[0]   , CAMERA_MAP_SCAN_SPEED[1]   },
            CAMERA_MAX_FRAMES,
            CAMERA_DISPLACEMENT_MODE,            
            eLineNoROI,
            // SLineExtractionInROI
            { 0, 0, 0, 0 },
            MapData,
            {  // DigInfo
               // DigFormat               SX  SY  SB Type Nb Frames
               { EX_PATH("Cam1_cans.avi"), 0,  0,  0,  0,  0 },
               { EX_PATH("Cam2_cans.avi"), 0,  0,  0,  0,  0 }
            },
            MIL_TEXT("All scan results are continuously accumulated into a single result\n")
            MIL_TEXT("object. Each can is inspected when most of it has been scanned.\n")
            MIL_TEXT("Color legend:\n")
            MIL_TEXT("   Dark blue     = minimum height\n")
            MIL_TEXT("   Green, Yellow = middle height\n")
            MIL_TEXT("   Dark red      = maximum height\n\n")
            };

         // Update some information from the sequences on disk.
         for(MIL_INT d = 0; d < NUM_CAMERAS; d++)
            { SCAN_INFO.DigInfo[d].UpdateInfoFromDisk(); }

         //....................................................
         // 3. Acquire a 3D point cloud by scanning the object.
         //    The point cloud container will hold one point cloud per camera-laser pair.
         //    Perform the analysis during the acquisition continuously.     
         CContinuousCanInspection ProcObj(MapData);

         const MIL_INT NB_FRAME_FOR_ANALYSIS = 20;
         MIL_ID PointCloudContainer = M_NULL;
         pExampleMngrFor3D->AcquirePointCloud(eScanWithContinuousAnalysis,
                                              &SCAN_INFO,
                                              CameraLaserCtxts,
                                              &PointCloudContainer,
                                              &ProcObj,
                                              NB_FRAME_FOR_ANALYSIS);

         // Free camera-laser contexts.
         for (MIL_INT c = 0; c < NUM_CAMERAS; c++)
            {
            for (MIL_INT l = 0; l < NUM_LASERS_PER_IMAGE; l++)
               {
               MIL_ID& CameraLaserCtx = CameraLaserCtxts[(c*NUM_LASERS_PER_IMAGE) + l];
               if (CameraLaserCtx != M_NULL)
                  {
                  M3dmapFree(CameraLaserCtx);
                  CameraLaserCtx = M_NULL;
                  }
               }
            }

         M3dmapFree(PointCloudContainer);
         }
      }
   else
      {
      // A problem occurred calibrating the cameras.
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   // Free camera calibrations.
   for (MIL_INT c = 0; c < NUM_CAMERAS; c++)
      {
      if(CameraCalibrations[c] != M_NULL)
         {
         McalFree(CameraCalibrations[c]);
         CameraCalibrations[c] = M_NULL;
         }
      }
   delete pExampleMngrFor3D;
   pExampleMngrFor3D = NULL;

   // Free the MIL application.
   MappFree(MilApplication);

   return 0;
   }

//*******************************************************************************
// Function that analyzes the scanned object.
//*******************************************************************************
void CContinuousCanInspection::Analyze(SCommonAnalysisObjects& CommonAnalysisObjects)
   {
   // Processing constants.
   const MIL_INT    NUM_SMOOTH           = 99;
   const MIL_DOUBLE BINARIZE_THRESHOLD   = 62;
   const MIL_INT    CAN_DELTA_X          = 60;
   const MIL_INT    CAN_DELTA_Y          = 60;
   const MIL_INT    MAX_CAN_MISSING_DATA = 1000;
   const MIL_DOUBLE MIN_TAB_HEIGHT       = 2.5;
   const MIL_DOUBLE MAX_TAB_HEIGHT       = 4.85;

   const MIL_INT YOffset = 30;

   // Color specifications.
   const MIL_DOUBLE PROC_TEXT_COLOR  = M_COLOR_GREEN;
   const MIL_DOUBLE PROC_PASS_COLOR  = M_COLOR_GREEN;
   const MIL_DOUBLE PROC_FAIL_COLOR  = M_COLOR_RED;

   const MIL_INT PROC_TEXT_OFFSET_X = 320;

   MIL_ID MilSystem        = CommonAnalysisObjects.MilSystem;
   MIL_ID MilGraphics      = CommonAnalysisObjects.MilGraphics;
   MIL_ID MilGraphicList   = CommonAnalysisObjects.MilGraphicList;
   MIL_ID MilDepthMap      = CommonAnalysisObjects.MilDepthMap;
   CMILDisplayManager* MilResultsDisplay =  CommonAnalysisObjects.MilResultsDisplay;

   // Disable update to display.
   MilResultsDisplay->Control(M_UPDATE, M_DISABLE);

   // Disable graphics list update.
   MdispControl(MilResultsDisplay->GetDisplayID(), M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Setup the display.
   MgraClear(M_DEFAULT, MilGraphicList);   
   if(!m_CanFoundSoFar)
      { MgraText(MilGraphics, MilGraphicList, TEXT_OFFSET_X, TEXT_OFFSET_Y,  MIL_TEXT("Scanning...")); }
   MbufClear(m_Remapped8BitImage, 0);

   // Remap 16-bit depth map to 8 bit.   
   MimShift(MilDepthMap, m_Remapped8BitImage, -8);
   // Set the invalid data to 0.
   MbufClearCond(m_Remapped8BitImage, 0, 0, 0, m_Remapped8BitImage, M_EQUAL, 255.0);

   // Disassociate the calibration from the binarized image because we will not use it.
   McalAssociate(M_NULL, m_Remapped8BitImage, M_DEFAULT);

   // Find the cans.
   MmodFind(m_CanModel, m_Remapped8BitImage, m_CanModelResult);

   // Get information on the find.
   MmodControl(m_CanModelResult, M_DEFAULT, M_RESULT_OUTPUT_UNITS, M_PIXEL);

   std::vector<MIL_INT> PositionX;
   std::vector<MIL_INT> PositionY;
   MmodGetResult(m_CanModelResult, M_DEFAULT, M_POSITION_X, PositionX);
   MmodGetResult(m_CanModelResult, M_DEFAULT, M_POSITION_Y, PositionY);

   MIL_INT NumOfOccurences = static_cast<MIL_INT>(PositionX.size());

   MIL_TEXT_CHAR CanString[MAX_STRING_LEN] = MIL_TEXT("");

   // Find the tab for each can.
   for (MIL_INT i = 0; i < NumOfOccurences; i++)
      {
      m_CanFoundSoFar = true;
      bool CanOpen = false;
      MgraColor(MilGraphics, PROC_PASS_COLOR);

      MIL_INT PosX, PosY;
      PosX = Max(PositionX[i] - CAN_DELTA_X, MIL_INT(0));
      PosY = Max(PositionY[i] - CAN_DELTA_Y, MIL_INT(0));

      MIL_INT DeltaX = PositionX[i] - PosX;
      MIL_INT DeltaY = PositionY[i] - PosY;

      MIL_ID CanChild;
      MIL_INT MissingData = 0;
      MbufChild2d(MilDepthMap, 
                  PosX, 
                  PosY,
                  Min(DeltaX * 2, MAP_SIZE_X),
                  Min(DeltaY * 2, MAP_SIZE_Y), 
                  &CanChild);

      // Check if the can is open by looking for missing data.
      MIL_UNIQUE_3DIM_ID ResultId = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
      M3dimStat(M_STAT_CONTEXT_NUMBER_OF_POINTS, CanChild, ResultId, M_DEFAULT);
      M3dimGetResult(ResultId,M_NUMBER_OF_POINTS_MISSING_DATA, &MissingData);

      if (MissingData > MAX_CAN_MISSING_DATA)
         { CanOpen = true; }

      MbufFree(CanChild);
      MmodDraw(MilGraphics, m_CanModelResult, MilGraphicList, M_DRAW_POSITION + M_DRAW_EDGES, i, M_DEFAULT);

      // Set the search area of the tab centered in the found can.
      MmodControl(m_TabModel, M_DEFAULT, M_POSITION_X, PositionX[i]);
      MmodControl(m_TabModel, M_DEFAULT, M_POSITION_Y, PositionY[i]);

      MmodFind(m_TabModel, m_Remapped8BitImage, m_TabModelResult);

      MIL_INT TabOccurences = 0;
      MmodGetResult(m_TabModelResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &TabOccurences);

      bool TabElevated = false;
      bool TabBroken   = false;
      bool TabFound    = (TabOccurences == 1);
      if(TabFound)
         {
         MIL_INT TabPositionX, TabPositionY;

         // Check if the tab is elevated.
         MmodGetResult(m_TabModelResult, M_DEFAULT, M_POSITION_X + M_TYPE_MIL_INT, &TabPositionX);
         MmodGetResult(m_TabModelResult, M_DEFAULT, M_POSITION_Y + M_TYPE_MIL_INT, &TabPositionY);

         MIL_ID TabChild;
         MIL_DOUBLE DeviationMean = -DBL_MAX;
         const MIL_INT STAT_SIZE = 4;
         const MIL_DOUBLE DEVIATION_OUTLIER_DISTANCE = 12.0;

         PosX = Max(TabPositionX - STAT_SIZE, MIL_INT(0));
         PosY = Max(TabPositionY - STAT_SIZE, MIL_INT(0));

         MbufChild2d(MilDepthMap, PosX, PosY, STAT_SIZE, STAT_SIZE, &TabChild);

         // Compute deviation from the plane Z = 0, in the negative direction.
         MIL_UNIQUE_3DMET_ID ResultId = M3dmetAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
         M3dmetStat(M_STAT_CONTEXT_MEAN, TabChild, M_XY_PLANE, ResultId, M_SIGNED_DISTANCE_Z_TO_SURFACE, M_LESS, 0.0, M_NULL, M_DEFAULT); 
         M3dmetGetResult(ResultId, M_STAT_MEAN, &DeviationMean);
         DeviationMean = std::fabs(DeviationMean);
         MbufFree(TabChild);

          // Set the tab status thresholding the computed deviation mean.
         TabBroken   = (DeviationMean < MIN_TAB_HEIGHT);
         TabElevated = (DeviationMean > MAX_TAB_HEIGHT);
         }

      // Draw surface status.
      MgraColor(MilGraphics, CanOpen ? PROC_FAIL_COLOR : PROC_PASS_COLOR);
      MosSprintf(CanString, MAX_STRING_LEN, MIL_TEXT("Can surface: %s"), CanOpen ? MIL_TEXT("open") : MIL_TEXT("pass"));
      MgraText(MilGraphics, MilGraphicList, PROC_TEXT_OFFSET_X, PositionY[i]-CAN_DELTA_Y, CanString);

      // Draw tab status.
      MIL_CONST_TEXT_PTR pTabStr = M_NULL; 
      if(TabFound)
         {
         if(TabElevated)
            {
            MgraColor(MilGraphics, PROC_FAIL_COLOR);
            pTabStr = MIL_TEXT("elevated");
            }
         else if(TabBroken)
            {
            MgraColor(MilGraphics, PROC_FAIL_COLOR);
            pTabStr = MIL_TEXT("broken");
            }
         else
            {
            MgraColor(MilGraphics, PROC_PASS_COLOR);
            pTabStr = MIL_TEXT("pass");
            }
         }
      else
         {
         MgraColor(MilGraphics, PROC_FAIL_COLOR);
         pTabStr = MIL_TEXT("missing");
         }

      MosSprintf(CanString, MAX_STRING_LEN, MIL_TEXT("Tab : %s"), pTabStr);
      MgraText(MilGraphics, MilGraphicList, PROC_TEXT_OFFSET_X, PositionY[i]-CAN_DELTA_Y+YOffset, CanString);

      // Draw edges with the same tab status color.
      MmodDraw(MilGraphics, m_TabModelResult, MilGraphicList, M_DRAW_EDGES, M_ALL, M_DEFAULT);
      }

   // Enable graphics list update.
   MdispControl(MilResultsDisplay->GetDisplayID(), M_UPDATE_GRAPHIC_LIST, M_ENABLE);

   // Update the display.
   MilResultsDisplay->Control(M_TITLE,  MIL_TEXT("Inspection results"));
   MilResultsDisplay->Show(m_Remapped8BitImage);
   MilResultsDisplay->Control(M_UPDATE, M_ENABLE);
   }

//*******************************************************************************
// Function that allocates processing objects.
//*******************************************************************************
void CContinuousCanInspection::AllocProcessingObjects(MIL_ID MilSystem)
   {
   MIL_CONST_TEXT_PTR CAN_MODEL = EX_PATH("CanModel.mmf");
   MIL_CONST_TEXT_PTR TAB_MODEL = EX_PATH("TabModel.mmf");
   
   // Scans per can.
   const MIL_INT CAN_SIZE_Y = 240;
   
   const MIL_INT RESULTS_SIZE_X = 630;
   const MIL_INT SizeY = CAN_SIZE_Y * 2;

   // Allocate the necessary buffers for processing.
   MbufAlloc2d(MilSystem, RESULTS_SIZE_X, SizeY, 8, M_IMAGE + M_PROC + M_DISP, &m_Remapped8BitImage);

   // Restore and setup the models.
   // Can model.
   MmodAllocResult(MilSystem, M_DEFAULT, &m_CanModelResult);
   MmodRestore(CAN_MODEL, MilSystem, M_DEFAULT, &m_CanModel);

   // Preprocess the model.
   MmodPreprocess(m_CanModel, M_DEFAULT);

   // Tab model.
   MmodAllocResult(MilSystem, M_DEFAULT, &m_TabModelResult);
   MmodRestore(TAB_MODEL, MilSystem, M_DEFAULT, &m_TabModel);

   // Preprocess the model.
   MmodPreprocess(m_TabModel, M_DEFAULT);
   }


//*******************************************************************************
// Function that frees processing objects.
//*******************************************************************************
void CContinuousCanInspection::FreeProcessingObjects()
   {
   MbufFree(m_Remapped8BitImage);

   MmodFree(m_CanModel);
   MmodFree(m_CanModelResult);

   MmodFree(m_TabModel);
   MmodFree(m_TabModelResult);

   m_CanFoundSoFar = false;
   }
