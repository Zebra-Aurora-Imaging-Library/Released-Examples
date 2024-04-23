//***************************************************************************************
// 
// File name: MechanicalPartScan.cpp  
//
// Synopsis: Demonstrates the scan and 3D reconstruction of a mechanical part.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "MechanicalPartScan.h"

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("MechanicalPartScan\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the 3D reconstruction of a mechanical\n")
             MIL_TEXT("part using sheet of light profiling. The system consists of one\n")
             MIL_TEXT("camera and two lasers. \n"));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Display, Buffer, Graphics,\n"));
   MosPrintf(MIL_TEXT("Image Processing, Calibration, 3D Reconstruction, Model Finder,\n"));
   MosPrintf(MIL_TEXT("3D Image Processing, 3D Metrology, 3D Display and 3D Graphics.\n\n"));
   }

static const MIL_INT MAP_SIZE_X = 487;
static const MIL_INT MAP_SIZE_Y = 1319;

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
   // Initialize data.
   SCameraCalibrationInfo CAMERA_CALIBRATION_INFO;
   CAMERA_CALIBRATION_INFO.CornerHintX          = 1400;
   CAMERA_CALIBRATION_INFO.CornerHintY          = 100;
   CAMERA_CALIBRATION_INFO.OffsetZ              = 0;
   CAMERA_CALIBRATION_INFO.NbRows               = 22;
   CAMERA_CALIBRATION_INFO.NbCols               = 18;
   CAMERA_CALIBRATION_INFO.RowSpacing           = 8.83;
   CAMERA_CALIBRATION_INFO.ColSpacing           = 8.83;
   CAMERA_CALIBRATION_INFO.CalibrationType      = M_CHESSBOARD_GRID;
   CAMERA_CALIBRATION_INFO.GridImageFilename    = EX_PATH("grid_1.mim");
   CAMERA_CALIBRATION_INFO.Relocate             = RELOCATE;
   CAMERA_CALIBRATION_INFO.RelocatedCornerHintX = 1340;
   CAMERA_CALIBRATION_INFO.RelocatedCornerHintY = 140;
   CAMERA_CALIBRATION_INFO.RelocatedOffsetZ     = 0;
   CAMERA_CALIBRATION_INFO.RelocatedGridImageFilename = EX_PATH("grid_2.mim");

   //.................................
   // 1.1 Execute cameras calibration.
   MIL_ID CameraCalibration;
   bool CameraCalibrationOk = pExampleMngrFor3D->CalibrateCameras(&CAMERA_CALIBRATION_INFO, NUM_CAMERAS, &CameraCalibration);
   //..................................................................
   // 2. Then continue to calibrate the laser planes (sheets-of-light).
   if(CameraCalibrationOk)
      {
      MosPrintf(MIL_TEXT("Press <Enter> to calibrate laser planes.\n\n"));
      MosGetch();

      // Sheet-of-Light (laser plane) calibration.
      const MIL_INT    NUM_LASERS_PER_IMAGE                       = 2;
      const MIL_INT    NUM_REF_PLANES                             = 7;
      const MIL_DOUBLE CAL_MIN_CONTRAST    [NUM_LASERS_PER_IMAGE] = { 100.0, 100.0 };
      const MIL_INT    CAL_NB_REF_PLANES   [NUM_LASERS_PER_IMAGE] = { NUM_REF_PLANES, NUM_REF_PLANES };
      const MIL_INT    CAL_SCAN_ORIENTATION[NUM_LASERS_PER_IMAGE] = { M_HORIZONTAL, M_HORIZONTAL };
      const MIL_INT    CAL_PEAK_WIDTH      [NUM_LASERS_PER_IMAGE] = { 12, 12 };
      const MIL_INT    CAL_PEAK_WIDTH_DELTA[NUM_LASERS_PER_IMAGE] = { 12, 12 };
      const MIL_INT    LASER_LABELS        [NUM_LASERS_PER_IMAGE] = { 1 , 2  };
      const MIL_INT    CAMERA_LABELS       [NUM_LASERS_PER_IMAGE] = { 1 , 1  }; // One camera, two lasers

      const SLineExtractionInROI CHILD_EXTRACTION_INFO [NUM_CAMERAS * NUM_LASERS_PER_IMAGE] =
         {  {  330,  130,  170,  910 },
            { 1090,    0,  185, 1200 } };
      
      const MIL_DOUBLE PLANE_Z[NUM_CAMERAS][MAX_NB_REF_PLANES] =
         {  { 0.0, -5.86, -11.72, -17.58, -23.44, -29.30, -35.15 } };

      const SRefPlaneInfo LASER_CALIBRATION_PLANES[NUM_CAMERAS * NUM_LASERS_PER_IMAGE][NUM_REF_PLANES] = 
         { { // first laser line
               // RefImageName                                Zs
               { EX_PATH("RefPlanes/laser_0.mim"), PLANE_Z[0][0] },
               { EX_PATH("RefPlanes/laser_1.mim"), PLANE_Z[0][1] },
               { EX_PATH("RefPlanes/laser_2.mim"), PLANE_Z[0][2] },
               { EX_PATH("RefPlanes/laser_3.mim"), PLANE_Z[0][3] },
               { EX_PATH("RefPlanes/laser_4.mim"), PLANE_Z[0][4] },
               { EX_PATH("RefPlanes/laser_5.mim"), PLANE_Z[0][5] },
               { EX_PATH("RefPlanes/laser_6.mim"), PLANE_Z[0][6] }
            },
            { // second laser line
               // RefImageName                                Zs
               { EX_PATH("RefPlanes/laser_0.mim"), PLANE_Z[0][0] },
               { EX_PATH("RefPlanes/laser_1.mim"), PLANE_Z[0][1] },
               { EX_PATH("RefPlanes/laser_2.mim"), PLANE_Z[0][2] },
               { EX_PATH("RefPlanes/laser_3.mim"), PLANE_Z[0][3] },
               { EX_PATH("RefPlanes/laser_4.mim"), PLANE_Z[0][4] },
               { EX_PATH("RefPlanes/laser_5.mim"), PLANE_Z[0][5] },
               { EX_PATH("RefPlanes/laser_6.mim"), PLANE_Z[0][6] }
            } };

      SCameraLaserInfo LASER_CALIBRATION_INFO[NUM_CAMERAS * NUM_LASERS_PER_IMAGE];
      for(MIL_INT c = 0; c < (NUM_CAMERAS * NUM_LASERS_PER_IMAGE); c++)
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
         LCI.LineExtractionInROI = eLineChildROI;
         LCI.LineExtractionInROIInfo = CHILD_EXTRACTION_INFO[c];
         }

      //............................................................
      // 2.1 Execute the calibration of the laser planes.
      // Generates the needed calibrated camera-laser pair contexts.
      MIL_ID CameraLaserCtxts[NUM_CAMERAS * NUM_LASERS_PER_IMAGE];
      bool SheetOfLightOk = pExampleMngrFor3D->CalibrateSheetOfLight(&LASER_CALIBRATION_INFO[0],
                                                                     &CameraCalibration,
                                                                     &CameraLaserCtxts[0]);
      if (SheetOfLightOk)
         {
         // Map generation specifications.
         const MIL_DOUBLE M3D_DISPLAY_REFRESH_PER_SEC = 1.0; // 3D Display FPS
         const MIL_DOUBLE M3D_DISPLAY_LOOK_AT_X       = 0.0;
         const MIL_DOUBLE M3D_DISPLAY_LOOK_AT_Y       = 120.98;
         const MIL_DOUBLE M3D_DISPLAY_LOOK_AT_Z       = 96.85;
         const MIL_DOUBLE M3D_DISPLAY_EYE_DIST        = 676.62;
         const MIL_DOUBLE M3D_DISPLAY_EYE_THETA       = 37.81;
         const MIL_DOUBLE M3D_DISPLAY_EYE_PHI         = 64.17; 
         const MIL_INT    CAMERA_MAP_MIN_CONTRAST[]   = {  20,  20 };
         const MIL_INT    CAMERA_MAP_PEAK_WIDTH[]     = {  12,  12 };
         const MIL_INT    CAMERA_MAP_PEAK_DELTA[]     = {  20,  20 };
         const MIL_DOUBLE CAMERA_MAP_SCAN_SPEED[]     = { 0.3125, 0.3125 };
         const MIL_DOUBLE CAMERA_MAX_FRAMES           = 1318;
         const MIL_DOUBLE CAMERA_DISPLACEMENT_MODE    = M_CURRENT;

         // Visualization volume information.
         SMapGeneration MapData;
         MapData.BoxCornerX       =    5.00;
         MapData.BoxCornerY       = -260.00;
         MapData.BoxCornerZ       =   -4.00;
         MapData.BoxSizeX         =  120.00;
         MapData.BoxSizeY         =  650.00;
         MapData.BoxSizeZ         = - 30.00;
         MapData.MapSizeX         = MAP_SIZE_X;
         MapData.MapSizeY         = MAP_SIZE_Y;
         MapData.PixelSizeX       = 0.22;
         MapData.PixelSizeY       = 0.22;
         MapData.GrayScaleZ       = (MapData.BoxSizeZ / 65534.0);
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
            eLineChildROI,
            { CHILD_EXTRACTION_INFO[0], CHILD_EXTRACTION_INFO[1] },
            MapData,
            {  // DigInfo
               // DigFormat                       SX  SY  SB Type NbFrames
               { EX_PATH("mechanical_part.avi"),   0,  0,  0,  0,  0 }
            },
            MIL_TEXT("") // ScanDisplayText
            };

         // Update some information from the sequences on disk.
         for(MIL_INT d = 0; d < NUM_CAMERAS; d++)
            { SCAN_INFO.DigInfo[d].UpdateInfoFromDisk(); }

         //....................................................
         // 3. Acquire a 3D point cloud by scanning the object.
         //    The point cloud container will hold one point cloud per camera-laser pair.
         MIL_ID PointCloudContainer = M_NULL;
         bool PointCloudOk = pExampleMngrFor3D->AcquirePointCloud(eScan, &SCAN_INFO, CameraLaserCtxts, &PointCloudContainer);

         //....................................................
         // 4. Copy all 3D point clouds to M_CONTAINER.
         MIL_UNIQUE_BUF_ID MilContainerId = MbufAllocContainer(pExampleMngrFor3D->GetSystem(), M_PROC, M_DEFAULT, M_UNIQUE_ID);
         M3dmapCopyResult(PointCloudContainer, M_ALL, MilContainerId, M_POINT_CLOUD_UNORGANIZED, M_DEFAULT);

         //.....................................................................................
         // 5. Generate the depth map (orthogonal 2D-projection) of the acquired 3D point cloud.
         MIL_ID MechanicalPartDepthmap = M_NULL;
         ProjectDepthMap(pExampleMngrFor3D->GetSystem(), MilContainerId, SCAN_INFO.MapVisualizationData, &MechanicalPartDepthmap);

         //....................................
         // 6. Analyze the generated depth map.
         CAnalyzeMechanicalPart ProbObj;
         pExampleMngrFor3D->AnalyzeDepthMap(&ProbObj, MechanicalPartDepthmap, MilContainerId, SCAN_INFO.MapVisualizationData);

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
         if(MechanicalPartDepthmap != M_NULL)
            { MbufFree(MechanicalPartDepthmap); }
         }
      }
   else
      {
      // A problem occurred calibrating the camera.
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   // Free camera calibrations.
   if(CameraCalibration != M_NULL)
      {
      McalFree(CameraCalibration);
      CameraCalibration = M_NULL;
      }

   delete pExampleMngrFor3D;
   pExampleMngrFor3D = NULL;

   // Free the MIL application.
   MappFree(MilApplication);

   return 0;
   }

//*******************************************************************************
// Analysis implementation of the scanned object.
//*******************************************************************************
static const MIL_INT NB_HEIGHT_MEASURES = 10;

void CAnalyzeMechanicalPart::Analyze(SCommonAnalysisObjects& CommonAnalysisObjects)
   {
   const MIL_INT REFERENCE_POINT_INDEX = 3;

   const MIL_DOUBLE DIV_180_PI = 57.295779513082320866997945294156;

   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_X = 1.0;
   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_Y = 1.0;

   const MIL_INT DepthMapChildOffsetX = 0;
   const MIL_INT DepthMapChildOffsetY = 400;
   const MIL_INT DepthMapChildSizeX = MAP_SIZE_X;
   const MIL_INT DepthMapChildSizeY = MAP_SIZE_Y - DepthMapChildOffsetY;
   
   // Plane fit circle parameter.
   const MIL_DOUBLE PLANE_FIT_CENTER_X = 47.88;
   const MIL_DOUBLE PLANE_FIT_CENTER_Y = 39.29;
   const MIL_DOUBLE PLANE_FIT_RADIUS = 24;

   MIL_ID MilSystem                   = CommonAnalysisObjects.MilSystem;
   MIL_ID MilPtCldCntr                = CommonAnalysisObjects.MilPtCldCtnr;
   MIL_ID MilDepthMap                 = CommonAnalysisObjects.MilDepthMap;
   MIL_ID MilGraphicList              = CommonAnalysisObjects.MilGraphicList;
   CMILDisplayManager* MilDisplayMngr = CommonAnalysisObjects.MilDisplays;
   MIL_INT NumLaserScans              = CommonAnalysisObjects.NumLaserScanObjects;
   const SMapGeneration* GenerationInfo = CommonAnalysisObjects.GenerationInfo;
   
   // Allocate the necessary buffers for processing.
   MIL_ID MilDepthMapChild = MbufChild2d(MilDepthMap, DepthMapChildOffsetX, DepthMapChildOffsetY,
                                         DepthMapChildSizeX, DepthMapChildSizeY, M_NULL);

   MIL_ID MilDiffMap = MbufAlloc2d(MilSystem,
                                   DepthMapChildSizeX,
                                   DepthMapChildSizeY,
                                   16, M_IMAGE + M_PROC, M_NULL);

   MIL_ID MilRemapped8BitImage = MbufAlloc2d(MilSystem,
                                             DepthMapChildSizeX,
                                             DepthMapChildSizeY,
                                             8, M_IMAGE + M_PROC + M_DISP,
                                             M_NULL);

   // Clear the graphics list.
   MgraClear(M_DEFAULT, MilGraphicList);

   // Setup the display.
   MilDisplayMngr->Zoom(PROC_DISPLAY_ZOOM_FACTOR_X, PROC_DISPLAY_ZOOM_FACTOR_Y);
   MilDisplayMngr->Control(M_VIEW_MODE, M_AUTO_SCALE);
   MilDisplayMngr->Show(MilDepthMapChild);
   MilDisplayMngr->UpdateEnabled(false);

   // Get the world position of the depth map middle point of the visualization volume.
   MIL_DOUBLE VisualizationCenterX = (DepthMapChildSizeX - 1) / 2;
   MIL_DOUBLE VisualizationCenterY = (DepthMapChildSizeY - 1) / 2;
   MIL_DOUBLE VisualizationCenterZ = (MIL_DOUBLE)(MIL_UINT16_MAX-1)/2;
   McalTransformCoordinate3dList(MilDepthMapChild,
                                 M_PIXEL_COORDINATE_SYSTEM,
                                 M_RELATIVE_COORDINATE_SYSTEM,
                                 1,
                                 &VisualizationCenterX,
                                 &VisualizationCenterY,
                                 &VisualizationCenterZ,
                                 &VisualizationCenterX,
                                 &VisualizationCenterY,
                                 &VisualizationCenterZ,
                                 M_DEPTH_MAP);

   // Fixture the part.
   if (FixturePart(MilDepthMapChild, MilRemapped8BitImage, MilDepthMapChild, MilGraphicList))
      {
      // Print fixturing success message.
      MilDisplayMngr->UpdateEnabled(true);
      MosPrintf(MIL_TEXT("The mechanical part was fixtured using Model Finder in the depth map.\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      MilDisplayMngr->UpdateEnabled(false);

      // Calculate the heights relative to a given point and display the results.
      CalculateAndDisplayRelativeHeights(MilDepthMapChild, MilGraphicList, REFERENCE_POINT_INDEX);
      
      // Show the current measuring method.
      SetCurrentMethodImage(0);
      MilDisplayMngr->UpdateEnabled(true);
      MosPrintf(MIL_TEXT("METHOD 1:\n")
                MIL_TEXT("The heights, relative to the point in Magenta (index #3) and\n")
                MIL_TEXT("measured along the Z-axis, are shown.\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      MilDisplayMngr->UpdateEnabled(false);

      // Clear the graphic list.
      MgraClear(M_DEFAULT, MilGraphicList);

      // Redraw the found occurrence.
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MmodDraw(M_DEFAULT, m_MilMechPartResult, MilGraphicList, M_DRAW_EDGES + M_MODEL, M_DEFAULT, M_DEFAULT);

      // Associate an ROI where to fit the plane.
      MIL_UNIQUE_GRA_ID ROI = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);
      MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
      MgraArcFill(M_DEFAULT, ROI,
                  PLANE_FIT_CENTER_X, PLANE_FIT_CENTER_Y,
                  PLANE_FIT_RADIUS, PLANE_FIT_RADIUS, 0, 360);
      MbufSetRegion(MilDepthMapChild, ROI, M_DEFAULT, M_RASTERIZE_AND_DISCARD_LIST, M_DEFAULT);

      // Draw ROI annotation.
      MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
      MgraArcFill(M_DEFAULT, MilGraphicList,
                  PLANE_FIT_CENTER_X, PLANE_FIT_CENTER_Y,
                  PLANE_FIT_RADIUS, PLANE_FIT_RADIUS, 0, 360);
      MgraControl(M_DEFAULT, M_INPUT_UNITS, M_PIXEL);
      
      // Use the depth map with ROI for plane fitting, then remove ROI.
      MIL_UNIQUE_3DMET_ID FitResultId = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_UNIQUE_ID);
      M3dmetFit(M_DEFAULT, MilDepthMapChild, M_PLANE, FitResultId, M_DEFAULT, M_DEFAULT);
      M3dmetCopyResult(FitResultId, m_MilPlaneGeometry, M_FITTED_GEOMETRY, M_DEFAULT);
      MbufSetRegion(MilDepthMapChild, M_NULL, M_DEFAULT, M_DELETE, M_DEFAULT);

      // Get the difference between the depth map and the plane.
      M3dimArith(MilDepthMapChild, m_MilPlaneGeometry, MilDiffMap,M_NULL, M_SUB, M_MIN_Z, M_FIT_SCALES);

      // Display the heights relative to the fitted plane.
      CalculateAndDisplayRelativeHeights(MilDiffMap, MilGraphicList, -1);

      // Show the current measuring method.
      SetCurrentMethodImage(1);

      // Print message.
      MilDisplayMngr->UpdateEnabled(true);
      MosPrintf(MIL_TEXT("METHOD 2:\n")
                MIL_TEXT("The heights, relative to the plane fitted from the data in\n")
                MIL_TEXT("magenta and measured along the Z-axis, are shown.\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      MilDisplayMngr->UpdateEnabled(false);

      MbufFree(MilDepthMapChild); MilDepthMapChild = M_NULL;

      // Clear the graphic list.
      MgraClear(M_DEFAULT, MilGraphicList);

      // Get the parameters of the plane.
      MIL_DOUBLE Ax;
      MIL_DOUBLE Ay;
      MIL_DOUBLE Az;
      M3dgeoInquire(m_MilPlaneGeometry, M_COEFFICIENT_A, &Ax);
      M3dgeoInquire(m_MilPlaneGeometry, M_COEFFICIENT_B, &Ay);
      M3dgeoInquire(m_MilPlaneGeometry, M_COEFFICIENT_C, &Az);

      // Use the plane parameters to move the container before generating the depth map.
      MIL_UNIQUE_3DGEO_ID MilMatrixId = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
      M3dgeoMatrixSetTransform(MilMatrixId, M_ROTATION_AXIS_Z, Ax, Ay, Az, M_DEFAULT, M_DEFAULT);
    
      MIL_UNIQUE_BUF_ID  MilContainerId = MbufAllocContainer(MilSystem, M_PROC, M_DEFAULT, M_UNIQUE_ID);
      M3dimMatrixTransform(MilPtCldCntr, MilContainerId, MilMatrixId, M_DEFAULT);

      // Regenerate the depth map.
       ProjectDepthMap(MilSystem, MilContainerId, *GenerationInfo, &MilDepthMap);

      // Reset the child.     
      MbufChild2d(MilDepthMap, DepthMapChildOffsetX, DepthMapChildOffsetY,
                  DepthMapChildSizeX, DepthMapChildSizeY, &MilDepthMapChild);
      MilDisplayMngr->Show(MilDepthMapChild);

      // Fixture the part.
      if (FixturePart(MilDepthMapChild, MilRemapped8BitImage, MilDepthMapChild, MilGraphicList))
         {         
         // Calculate the heights relative to a given point and display the results.
         CalculateAndDisplayRelativeHeights(MilDepthMapChild, MilGraphicList, REFERENCE_POINT_INDEX);

         // Show the current method.
         SetCurrentMethodImage(2);
         MilDisplayMngr->UpdateEnabled(true);
         MosPrintf(MIL_TEXT("METHOD 3:\n")
                   MIL_TEXT("The depth map was regenerated with the Z axis perpendicular to the\n")
                   MIL_TEXT("the fitted plane in order to measure perpendicularly to the plane.\n")
                   MIL_TEXT("The heights, relative to the point in Magenta (index #3) and\n")
                   MIL_TEXT("measured along the new Z-axis perpendicular to the fitted plane,\n")
                   MIL_TEXT("are shown.\n"));
         }
      else
         MosPrintf(MIL_TEXT("Unable to find the part in the corrected depth map.\n"));
      }  
   else
      { MosPrintf(MIL_TEXT("Unable to find the part in the depth map.\n")); }

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   // Free the necessary buffers for processing.
   MbufFree(MilDiffMap);
   MbufFree(MilDepthMapChild);
   MbufFree(MilRemapped8BitImage);
   }

//*******************************************************************************
// Finds the model, fixture a destination and draw the occurrence in
// the graphic list.
//*******************************************************************************
bool CAnalyzeMechanicalPart::FixturePart(MIL_ID MilDepthMap, MIL_ID MilSearchImage, MIL_ID MilFixtureDestination, MIL_ID MilGraList)
   {
   // Remap to 8 bit.
   M3dimRemapDepthMap(M_REMAP_CONTEXT_BUFFER_LIMITS, MilDepthMap, MilSearchImage, M_DEFAULT);

   // Find the model.
   MmodFind(m_MilMechPartModel, MilSearchImage, m_MilMechPartResult);

   // Get information on the find.
   MIL_INT NumOfOccurences = 0;
   MmodGetResult(m_MilMechPartResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumOfOccurences);

   if (NumOfOccurences)
      {
      // Fixture the depth map.
      McalFixture(MilFixtureDestination, m_MilMechPartFixtureOffset, M_MOVE_RELATIVE, M_RESULT_MOD, m_MilMechPartResult, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      // Draw the found occurrence.
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MmodDraw(M_DEFAULT, m_MilMechPartResult, MilGraList, M_DRAW_EDGES + M_MODEL, M_DEFAULT, M_DEFAULT);
      }

   return (NumOfOccurences > 0);
   }

//*******************************************************************************
// Calculates the heights relative to a point and displays the results.
// If the index is set to -1 the absolute heights are displayed.
//*******************************************************************************
void CAnalyzeMechanicalPart::CalculateAndDisplayRelativeHeights(MIL_INT MilDepthMap, MIL_ID MilGraphicList, MIL_INT ReferencePointIndex)
   {   
   const MIL_DOUBLE WORLD_HEIGHT_MEASURES_X[NB_HEIGHT_MEASURES] = {  44,  26.5,    41,  48.5,    30,    56,    25,  55.5,    20,  63.5 };
   const MIL_DOUBLE WORLD_HEIGHT_MEASURES_Y[NB_HEIGHT_MEASURES] = { -10, -20.5,    12,  38.5,    90,  93.5,   113,   141,   160, 107.5 };

   MIL_DOUBLE MeasuredWorldPointX[NB_HEIGHT_MEASURES];
   MIL_DOUBLE MeasuredWorldPointY[NB_HEIGHT_MEASURES];
   MIL_DOUBLE MeasuredWorldPointZ[NB_HEIGHT_MEASURES];

   // Calculate the Z position of the points.
   CalculateWorldZ(MilDepthMap, NB_HEIGHT_MEASURES, WORLD_HEIGHT_MEASURES_X, WORLD_HEIGHT_MEASURES_Y, MeasuredWorldPointX, MeasuredWorldPointY, MeasuredWorldPointZ);

   if (ReferencePointIndex > 0)
      {
      // Get the height of the points relative to a given point.
      for (MIL_INT PointIdx = 0; PointIdx < NB_HEIGHT_MEASURES; PointIdx++)
         {
         if (PointIdx != ReferencePointIndex)
            { MeasuredWorldPointZ[PointIdx] -= MeasuredWorldPointZ[ReferencePointIndex]; }
         }
      MeasuredWorldPointZ[ReferencePointIndex] = 0;
      }

   // Display the height values.
   DisplayHeights(ReferencePointIndex, MilGraphicList, MeasuredWorldPointX, MeasuredWorldPointY, MeasuredWorldPointZ);
   }

//*******************************************************************************
// Calculates the Z value of the input world coordinates.
//*******************************************************************************
void CAnalyzeMechanicalPart::CalculateWorldZ(MIL_ID MilDepthMap,
                                             MIL_INT NbPoints,
                                             const MIL_DOUBLE* pInWorldPointX,
                                             const MIL_DOUBLE* pInWorldPointY,
                                             MIL_DOUBLE* pOutWorldPointX,
                                             MIL_DOUBLE* pOutWorldPointY,
                                             MIL_DOUBLE* pWorldPointZ)
   {
   McalTransformCoordinateList(MilDepthMap, M_WORLD_TO_PIXEL, NbPoints, pInWorldPointX, pInWorldPointY, pOutWorldPointX, pOutWorldPointY);

   // Get the height of the pixel coordinate.
   McalTransformCoordinate3dList(MilDepthMap,
                                 M_PIXEL_COORDINATE_SYSTEM,
                                 M_RELATIVE_COORDINATE_SYSTEM,
                                 NB_HEIGHT_MEASURES,
                                 pOutWorldPointX,
                                 pOutWorldPointY,
                                 M_NULL,
                                 pOutWorldPointX,
                                 pOutWorldPointY,
                                 pWorldPointZ,
                                 M_DEPTH_MAP);
   }

//*******************************************************************************
// Displays the heights in the graphic list and in the console.
//*******************************************************************************
void CAnalyzeMechanicalPart::DisplayHeights(MIL_INT ReferenceHeightIndex, 
                                            MIL_ID MilGraphicList,
                                            const MIL_DOUBLE* pWorldPointX,
                                            const MIL_DOUBLE* pWorldPointY,
                                            const MIL_DOUBLE* pWorldPointZ)
   {
   static MIL_DOUBLE MEASURE_POINT_ARC_RADIUS = 1.0;

   // Print the table header.
   MosPrintf(MIL_TEXT("|-------|-----------------|\n")
             MIL_TEXT("| Index | Measured height |\n")
             MIL_TEXT("|-------|-----------------|\n"));

   // Set the drawing to be in world units.
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);

   for (MIL_INT HeightIdx = 0; HeightIdx < NB_HEIGHT_MEASURES; HeightIdx++)
      {
      MgraColor(M_DEFAULT, pWorldPointZ[HeightIdx] == M_INVALID_POINT ? M_COLOR_RED : (HeightIdx == ReferenceHeightIndex ? M_COLOR_MAGENTA : M_COLOR_GREEN));
      MIL_TEXT_CHAR HeightIndexString[4];
      MosSprintf(HeightIndexString, 4, MIL_TEXT("%i"), (int)HeightIdx);
      MgraText(M_DEFAULT, MilGraphicList, pWorldPointX[HeightIdx] + MEASURE_POINT_ARC_RADIUS, pWorldPointY[HeightIdx], HeightIndexString);
      MgraArcFill(M_DEFAULT, MilGraphicList, pWorldPointX[HeightIdx], pWorldPointY[HeightIdx], MEASURE_POINT_ARC_RADIUS, MEASURE_POINT_ARC_RADIUS, 0, 360);
      MosPrintf(MIL_TEXT("| %5i | %15.2f |\n"), (int)HeightIdx, pWorldPointZ[HeightIdx]);
      }
   MosPrintf(MIL_TEXT("|-------|-----------------|\n\n"));

   // Set the drawing to be in world units.
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_PIXEL);
   }

//*******************************************************************************
// Function that sets the focus on a given method image.
//*******************************************************************************
void CAnalyzeMechanicalPart::SetCurrentMethodImage(MIL_INT MethodIndex)
   {
   // Disable the display updates.
   MdispControl(m_MilMethodDisplay, M_UPDATE, M_DISABLE);

   // Gray out the entire display method image.
   MimArithMultiple(m_MilFullMethodImage, 128, 127, 256, M_NULL, m_MilDispMethodImage, M_MULTIPLY_ACCUMULATE_1, M_DEFAULT);

   // Copy the chosen method.
   MbufCopyColor2d(m_MilFullMethodImage, m_MilDispMethodImage,
                   M_ALL_BANDS, 0, MethodIndex*m_MethodImageSizeY,
                   M_ALL_BANDS, 0, MethodIndex*m_MethodImageSizeY,
                   m_MethodImageSizeX, m_MethodImageSizeY);

   // Select the image on the display.
   MdispSelect(m_MilMethodDisplay, m_MilDispMethodImage);

   // Enable the display updates.
   MdispControl(m_MilMethodDisplay, M_UPDATE, M_ENABLE);
   }


//*******************************************************************************
// Function that allocates processing objects.
//*******************************************************************************
void CAnalyzeMechanicalPart::AllocProcessingObjects(MIL_ID MilSystem)
   {
   const MIL_INT WINDOWS_OFFSET_X = 15;
   const MIL_INT WINDOWS_OFFSET_Y = 38;

   // Method images path.
   const MIL_INT NB_MEASURING_METHODS = 3;
   MIL_CONST_TEXT_PTR MEASURING_METHOD_ILLUSTATIONS_FILES[NB_MEASURING_METHODS] = 
      {
      EX_PATH("MetalPart3dMeasuringMethod1.tif"),
      EX_PATH("MetalPart3dMeasuringMethod2.tif"),
      EX_PATH("MetalPart3dMeasuringMethod3.tif")
      };

   MIL_CONST_TEXT_PTR MECHANICAL_PART_MODEL = EX_PATH("ModelFinderContext.mmf");

   // Get the size of the a single method image.
   MbufDiskInquire(MEASURING_METHOD_ILLUSTATIONS_FILES[0], M_SIZE_X, &m_MethodImageSizeX);
   MbufDiskInquire(MEASURING_METHOD_ILLUSTATIONS_FILES[0], M_SIZE_Y, &m_MethodImageSizeY);
   
   // Allocate the full method image.
   MbufAllocColor(MilSystem, 3, m_MethodImageSizeX, m_MethodImageSizeY * NB_MEASURING_METHODS, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &m_MilFullMethodImage);
   MbufAllocColor(MilSystem, 3, m_MethodImageSizeX, m_MethodImageSizeY * NB_MEASURING_METHODS, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &m_MilDispMethodImage);
   
   // Restore the method images.
   for (MIL_INT MethodIdx = 0; MethodIdx < NB_MEASURING_METHODS; MethodIdx++)
      {
      MIL_ID MilMethodChild = MbufChild2d(m_MilFullMethodImage, 0, m_MethodImageSizeY * MethodIdx, m_MethodImageSizeX, m_MethodImageSizeY, M_NULL);
      MbufLoad(MEASURING_METHOD_ILLUSTATIONS_FILES[MethodIdx], MilMethodChild);
      MbufFree(MilMethodChild);
      }

   // Allocate the displays for the methods.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &m_MilMethodDisplay);
   MdispControl(m_MilMethodDisplay, M_WINDOW_INITIAL_POSITION_X, MAP_SIZE_X + WINDOWS_OFFSET_X);

   // Restore and setup the model.
   MmodAllocResult(MilSystem, M_DEFAULT, &m_MilMechPartResult);
   MmodRestore(MECHANICAL_PART_MODEL, MilSystem, M_WITH_CALIBRATION, &m_MilMechPartModel);
   MmodPreprocess(m_MilMechPartModel, M_DEFAULT);

   // Create the fixturing offset.
   McalAlloc(MilSystem, M_FIXTURING_OFFSET, M_DEFAULT, &m_MilMechPartFixtureOffset);
   McalFixture(M_NULL, m_MilMechPartFixtureOffset, M_LEARN_OFFSET, M_MODEL_MOD, m_MilMechPartModel, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Allocate the plane geometry.
   M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, &m_MilPlaneGeometry);
   }

//*******************************************************************************
// Function that frees processing objects.
//*******************************************************************************
void CAnalyzeMechanicalPart::FreeProcessingObjects()
   {
   MmodFree(m_MilMechPartModel);          m_MilMechPartModel         = M_NULL;
   MmodFree(m_MilMechPartResult);         m_MilMechPartResult        = M_NULL;
   McalFree(m_MilMechPartFixtureOffset);  m_MilMechPartFixtureOffset = M_NULL;
   M3dgeoFree(m_MilPlaneGeometry);        m_MilPlaneGeometry         = M_NULL;

   MdispFree(m_MilMethodDisplay);         m_MilMethodDisplay         = M_NULL;
   MbufFree(m_MilDispMethodImage);        m_MilDispMethodImage       = M_NULL;
   MbufFree(m_MilFullMethodImage);        m_MilFullMethodImage       = M_NULL;
   }
