//*****************************************************************************/
// 
// File name: LaserSystemConfiguration.h
// 
// Synopsis:  This file contains the configuration of a complete system
//            for calibration. Duplicate and edit this file to test a different 
//            configuration.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef LASER_SYSTEM_CONFIGURATION_H
#define LASER_SYSTEM_CONFIGURATION_H

#include "LaserSystemDefinition.h"

#define ARRAY_COUNT(x) (sizeof((x))/sizeof((x)[0])) // for array on the stack only.

//*****************************************************************************
//*****************************************************************************
// MODIFY THE NB OF CAMERAS AND NB OF LASERS FOR THE COMPLETE CONFIGURATION.
//*****************************************************************************
//*****************************************************************************
static const MIL_INT NB_CAMERAS = 2;
static const MIL_INT NB_LASERS = 2;

//*****************************************************************************
//*****************************************************************************
// MODIFY THE NB OF SYSTEMS, I.E. THE NB OF CAMERA-LASER PAIRS.
//*****************************************************************************
//*****************************************************************************
static const MIL_INT NB_SYSTEMS = 4;

// Tells the example if the illustration of the setup needs to be shown.
static const bool    IS_DEFAULT_SCANNING_SYSTEM = true;

//*****************************************************************************
// Declaration of the laser configuration class.
//*****************************************************************************
class CLaserSysConfig
   {
   public:
      CLaserSysConfig();
      virtual ~CLaserSysConfig(){};
      const SSingleSystemCal& System(MIL_INT Index) {return m_SingleSys[Index];}
      const SCameraCal& CamCal(MIL_INT Index)       {return m_CamCal[Index];}
   private:
      SCameraCal        m_CamCal[NB_CAMERAS];
      SLaserCal         m_LaserCal[NB_LASERS];
      SSingleSystemCal  m_SingleSys[NB_SYSTEMS];
   };


//*****************************************************************************
// Declaration of the general elements of the laser configuration.
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
// MODIFY THE PARAMETERS OF THE GRID FOR THE CAMERA CALIBRATIONS. CHOOSE BETWEEN
// THE 3 POSSIBLE GRID CALIBRATIONS :
//
//          1. Complete Grid Calibration      (SGridCal class)
//          2. Partial Grid Calibration       (SPartialGrid class)
//          3. Grid with fiducial Calibration (SFiducialGrid class)
//
// YOU CAN ADD AS MANY GRID CALIBRATION CLASSES AS THERE ARE DIFFERENT GRIDS
// TO CALIBRATE THE CAMERAS OF THE CONFIGURATION.
//
// MULTIPLE SYSTEMS CAN SHARE THE SAME GRID CALIBRATION PARAMETERS.
//*****************************************************************************
//*****************************************************************************

// static const SGridCal GridCal
//    (
//    M_CHESSBOARD_GRID,  // Type;
//    M_NONE,             // HintX;
//    M_NONE,             // HintY;
//    M_NONE,             // HintAngleX;
//    22,                 // RowNb;
//    18,                 // ColNb;
//    10,                 // RowSpacing;
//    10,                 // ColSpacing;
//    0.0,                // OffsetX;
//    0.0,                // OffsetY;
//    0.0,                // OffsetZ;
//    M_Y_AXIS_CLOCKWISE  // YaxisDirection;
//    );

// static const SPartialGridCal PartialGridCal
//    (
//    M_NONE,             // HintX;
//    M_NONE,             // HintY;
//    M_NONE,             // HintAngleX;
//    10,                 // RowSpacing;
//    10,                 // ColSpacing;
//    0.0,                // OffsetX;
//    0.0,                // OffsetY;
//    0.0,                // OffsetZ;
//    M_Y_AXIS_CLOCKWISE  // YaxisDirection;
//    );

// The same grid with fiducial is visible to all cameras.
static const SFiducialGridCal FiducialGridCal
   (
   0.0,                   // OffsetX;
   0.0,                   // OffsetY;
   0.0,                    // OffsetZ;
   M_Y_AXIS_CLOCKWISE     // YaxisDirection
   );

//*****************************************************************************
//*****************************************************************************
// MODIFY THE PARAMETERS OF THE EXTRACTION CHILD OF EACH SYSTEM. 
// YOU CAN ADD AS MANY AS THERE ARE DIFFERENT EXTRACTION CHILD BUFFERS
// IN THE CONFIGURATION.
//
// SET THE ChildType TO MATCH YOUR SETUP:
//
//          1.eChild       : The extraction is done in a child of a larger image.
//          2.ePartialScan : The extraction is done in a partial scan of the 
//                           camera. The offsetX and offsetY must match the
//                           offsets of the camera.
//
// MULTIPLE SYSTEMS CAN SHARE THE SAME EXTRACTION CHILD PARAMETERS.
//*****************************************************************************
//*****************************************************************************

// The cameras and lasers are set up so the extraction child buffers of both
// lasers are the same.
static const SExtractionChild ExtractionChilds[NB_LASERS] =
   {
      {
      eChild,             // ChildType;
      0,                  // OffsetX;
      0,                  // OffsetY;
      1920,               // SizeX;
      396                 // SizeY;
      },                  
                          
      {                   
      eChild,             // ChildType;
      0,                  // OffsetX;
      396,                // OffsetY;
      1920,               // SizeX;
      489                 // SizeY;
      },
   };

//*****************************************************************************
//*****************************************************************************
// MODIFY THE PARAMETERS OF THE LASER LINE EXTRACTION.
// YOU CAN ADD AS MANY AS THE NUMBER OF SYSTEMS.
//
//          See MIL HELP for more information on the peak extraction 
//          parameters.
//
// MULTIPLE SYSTEMS CAN SHARE THE SAME LASER LINE EXTRACTION PARAMETERS.
//*****************************************************************************
//*****************************************************************************

// All systems will share the same laser line extraction parameters so only one is
// declared.
static const SLaserLineExtraction LaserLineExtraction =
   {
   15,                    // PeakWidthNominal;
   15,                    // PeakWidthDelta;
   80,                    // MinimumContrast;
   M_VERTICAL             // ScanLaneDirection
   };

//*****************************************************************************
//*****************************************************************************
// MODIFY THE CORRECTED DEPTH VIEWED BY EACH SYSTEM TO CALIBRATE ITS LASER LINE.
// YOU CAN ADD AS MANY ARRAYS AS THE NUMBER OF LASERS.
//
// MULTIPLE LASERS CAN SHARE THE SAME CORRECTED DEPTHS.
//*****************************************************************************
//*****************************************************************************

// All systems will share the same list of corrected depth for their calibration.
static const MIL_DOUBLE CorrectedDepths[] =
   {
   -6.5,
   -22,
   -33.5,
   };

//*****************************************************************************
//*****************************************************************************
// MODIFY THE CONSTRUCTOR OF THE LASER SYSTEM CONFIGURATION TO BUILD ALL THE 
// SYSTEMS (CAMERA-LASER PAIR).
//
// FIRST, BUILD THE PARAMETERS OF EACH CAMERA CALIBRATION. YOU HAVE TO SET:
//
//             1. The label of the camera (each camera must have a unique label).
//             2. Assign the camera a pointer to the parameters of the grid calibrations defined above for:
//
//                   -pIntrinsicCal: The initial calibration where the 3d camera model is learned.
//                   -pExtrinsicCal (optional): The second calibration to learn the position
//                                              of the camera with regards to the conveyor.
//
// SECOND, BUILD THE PARAMETERS OF EACH LASER CALIBRATION. YOU HAVE TO SET:
//
//             1. The label of the laser (each laser must have a unique label).
//             2. A pointer to the array of corrected depth. Use the ARRAY_COUNT macro
//                to set the NbPlanes.
//
// THIRD, BUILD EACH SINGLE SYSTEM (CAMERA-LASER PAIR). YOU HAVE TO SET:
//
//             1. THE CAMERA CALIBRATION: A copy of one SCameraCal structure defined previously
//                                        in the constructor.
//             2. THE EXTRACTION CHILD: A copy of one SExtractionChild structure defined
//                                      previously in the file.
//
//             3. THE LASER CALIBRATION: A copy of one SLaserCal structure defined previously
//                                       in the constructor.
//
//             4. THE LASER LINE EXTRACTION PARAMETERS : A copy of one SLaserLineExtraction
//                                                       structure defined previously in the file.
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
// Constructor. Creates the configuration of the laser system.
//*****************************************************************************
CLaserSysConfig::CLaserSysConfig()
   {
   // Define the camera calibration parameters.
   m_CamCal[0].CamLabel         = 1;
   m_CamCal[0].pIntrinsicCal    = &FiducialGridCal;
   m_CamCal[0].pExtrinsicCal    = NULL;
   
   m_CamCal[1].CamLabel         = 2;
   m_CamCal[1].pIntrinsicCal    = &FiducialGridCal;
   m_CamCal[1].pExtrinsicCal    = NULL;

   // Define the laser calibration parameters.
   m_LaserCal[0].LaserLabel       = 1;
   m_LaserCal[0].NbPlanes         = ARRAY_COUNT(CorrectedDepths);
   m_LaserCal[0].pCorrectedDepths = CorrectedDepths;

   m_LaserCal[1].LaserLabel       = 2;
   m_LaserCal[1].NbPlanes         = ARRAY_COUNT(CorrectedDepths);
   m_LaserCal[1].pCorrectedDepths = CorrectedDepths;

   // Define the systems.
   m_SingleSys[0].CamCal               = m_CamCal[0];
   m_SingleSys[0].ExtractionChild      = ExtractionChilds[0];
   m_SingleSys[0].LaserCal             = m_LaserCal[0];
   m_SingleSys[0].LaserLineExtraction  = LaserLineExtraction;

   m_SingleSys[1].CamCal               = m_CamCal[1];
   m_SingleSys[1].ExtractionChild      = ExtractionChilds[1];
   m_SingleSys[1].LaserCal             = m_LaserCal[0];
   m_SingleSys[1].LaserLineExtraction  = LaserLineExtraction;

   m_SingleSys[2].CamCal               = m_CamCal[0];
   m_SingleSys[2].ExtractionChild      = ExtractionChilds[1];
   m_SingleSys[2].LaserCal             = m_LaserCal[1];
   m_SingleSys[2].LaserLineExtraction  = LaserLineExtraction;

   m_SingleSys[3].CamCal               = m_CamCal[1];
   m_SingleSys[3].ExtractionChild      = ExtractionChilds[0];
   m_SingleSys[3].LaserCal             = m_LaserCal[1];
   m_SingleSys[3].LaserLineExtraction  = LaserLineExtraction;
   }

//*****************************************************************************
// Setup illustration image.
//*****************************************************************************
#define EX_PATH(x) (M_IMAGE_PATH MIL_TEXT("MultiCameraLaserCalibration/") MIL_TEXT(x))
static MIL_CONST_TEXT_PTR SETUP_IMAGE = EX_PATH("Default3dScanningSetup.tif");

//*****************************************************************************
//*****************************************************************************
// MODIFY THE PATH OF THE IMAGES OF THE CALIBRATION GRIDS TO CALIBRATE THE 
// CAMERA.
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
// Camera calibration images.
//*****************************************************************************
static MIL_CONST_TEXT_PTR CAMERA_INT_PARAMS_IMAGE[NB_CAMERAS] = 
   {
   EX_PATH("GridImage0.mim"),
   EX_PATH("GridImage1.mim")
   };

static MIL_CONST_TEXT_PTR CAMERA_EXT_PARAMS_IMAGE[NB_CAMERAS] = 
   {
   NULL,
   NULL
   };

//*****************************************************************************
//*****************************************************************************
// MODIFY THE PATH OF THE IMAGES OF THE LASER LINE TO CALIBRATE EACH SINGLE SYSTEM.
//
// THE IMAGES OF THE LASER LINES MUST ALL BE IN THE SAME FOLDER
// AND THEIR NAMES MUST FOLLOW A NUMBERING CONVENTION. THEIR NAME WILL BE BUILT DYNAMICALLY
// BASED ON THE "SYSTEM_INDEX" AND THE "PLANE_INDEX":
//
//       SomeText(SYSTEM_INDEX)SomeOtherText(PLANE_INDEX)
//
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
// Laser calibration images.
//*****************************************************************************
static MIL_CONST_TEXT_PTR SYS_LASER_CAL_IMAGES = EX_PATH("Sys%i_H%i.mim");

#endif
