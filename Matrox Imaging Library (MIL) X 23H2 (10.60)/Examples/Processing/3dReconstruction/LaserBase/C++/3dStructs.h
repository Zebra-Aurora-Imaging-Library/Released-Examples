//*******************************************************************************
//
// File name: 3dStructs.h
//
// Synopsis:  Includes structures used for 3d examples.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef STRUCT3D_H
#define STRUCT3D_H

static const bool RELOCATE         = true;
static const bool NO_RELOCATE      = false;

static const bool SHOW_COLOR       = true;
static const bool SHOW_NO_COLOR    = false;

enum LineROIExtctEnum
   {
   eLineNoROI      = 0,
   eLineChildROI   = 1,
   eLineOffsetOnly = 2
   };

// Structure for digitizer information
struct SDigInfo
   {
   void UpdateInfoFromDisk()
      {
      SizeX    = MbufDiskInquire(DigFormat, M_SIZE_X, M_NULL);
      SizeY    = MbufDiskInquire(DigFormat, M_SIZE_Y, M_NULL);
      SizeBand = MbufDiskInquire(DigFormat, M_SIZE_BAND, M_NULL);
      Type     = MbufDiskInquire(DigFormat, M_TYPE, M_NULL);
      NbFrames = MbufDiskInquire(DigFormat, M_NUMBER_OF_IMAGES, M_NULL);
      }

   MIL_CONST_TEXT_PTR DigFormat;

   MIL_INT SizeX;
   MIL_INT SizeY;
   MIL_INT SizeBand;
   MIL_INT Type;
   MIL_INT NbFrames;
   };

// Structure for display
struct SDisplayInfo
   {
   SDigInfo DigitizerInfo;
   MIL_DOUBLE ZoomFactorX;
   MIL_DOUBLE ZoomFactorY;
   };

// Structure for illustrations
struct SIllustrations
   {   
   MIL_INT NumIllustrations;
   MIL_TEXT_CHAR IllustrationFiles[MAX_NB_ILLUSTRATIONS_PER_STEP]
                                  [MAX_FILENAME_LEN];

   inline const SIllustrations& operator= (const SIllustrations& o);
   };

//Structure for Camera calibration information
struct SCameraCalibrationInfo
   {
   MIL_DOUBLE           CornerHintX;
   MIL_DOUBLE           CornerHintY;
   MIL_DOUBLE           OffsetZ;
   MIL_INT              NbRows;
   MIL_INT              NbCols;
   MIL_DOUBLE           RowSpacing;
   MIL_DOUBLE           ColSpacing;
   MIL_INT64            CalibrationType;
   MIL_CONST_TEXT_PTR   GridImageFilename;

   bool                 Relocate;
   MIL_DOUBLE           RelocatedCornerHintX;
   MIL_DOUBLE           RelocatedCornerHintY;
   MIL_DOUBLE           RelocatedOffsetZ;
   MIL_CONST_TEXT_PTR   RelocatedGridImageFilename;   
   };

// Structure for child extraction information
struct SLineExtractionInROI
   {
   MIL_INT OffsetX;
   MIL_INT OffsetY;
   MIL_INT SizeX;
   MIL_INT SizeY;
   };

// Structure for calibration laser planes information
struct SRefPlaneInfo
   {
   MIL_TEXT_CHAR RefImageName[MAX_FILENAME_LEN];
   MIL_DOUBLE Z;
   };

// Structure for laser calibration information
struct SCameraLaserInfo
   {
   MIL_INT     NumLasersPerImage;
   MIL_INT     NumRefPlanes;
   MIL_DOUBLE  CalMinContrast;
   MIL_INT     CalNbRefPlanes;
   MIL_INT     CalScanOrientation;
   MIL_INT     CalPeakWidthNominal;
   MIL_INT     CalPeakWidthDelta;

   SRefPlaneInfo LaserCalibrationPlanes[MAX_NB_REF_PLANES];

   MIL_INT     LaserLabel;
   MIL_INT     CameraLabel;

   LineROIExtctEnum     LineExtractionInROI;
   SLineExtractionInROI LineExtractionInROIInfo;
   };

// Map generation parameters
struct SMapGeneration
   {
   MIL_DOUBLE BoxCornerX;
   MIL_DOUBLE BoxCornerY;
   MIL_DOUBLE BoxCornerZ;
   MIL_DOUBLE BoxSizeX;
   MIL_DOUBLE BoxSizeY;
   MIL_DOUBLE BoxSizeZ;
   MIL_INT    MapSizeX;
   MIL_INT    MapSizeY;
   MIL_DOUBLE PixelSizeX;
   MIL_DOUBLE PixelSizeY;
   MIL_DOUBLE GrayScaleZ;
   MIL_INT    IntensityMapType;

   bool SetExtractOverlap;
   MIL_INT ExtractOverlap;

   MIL_DOUBLE FillXThreshold;
   MIL_DOUBLE FillYThreshold;
   };

// Structure for scan and analyze information
struct SD3DSysInfo
   {
   MIL_DOUBLE D3DDisplayRefreshPerSec;
   bool ShowColor;
   MIL_DOUBLE InitLookAtX;
   MIL_DOUBLE InitLookAtY;
   MIL_DOUBLE InitLookAtZ;
   MIL_DOUBLE InitEyeDist;
   MIL_DOUBLE InitEyeTheta;
   MIL_DOUBLE InitEyePhi;
   };

struct SPointCloudAcquisitionInfo
   {
   SD3DSysInfo D3DSysInfo;

   MIL_INT    CameraMapMinContrast   [MAX_NB_LASERS];
   MIL_INT    CameraMapPeakWidth     [MAX_NB_LASERS];
   MIL_INT    CameraMapPeakWidthDelta[MAX_NB_LASERS];
   MIL_DOUBLE CameraMapScanSpeed     [MAX_NB_LASERS];
   MIL_DOUBLE CameraMaxFrames;
   MIL_DOUBLE CameraDisplacementMode;

   LineROIExtctEnum LineExtractionInROI;
   SLineExtractionInROI ChildExtractionInfo[MAX_NB_LASERS];
   SMapGeneration MapVisualizationData;

   SDigInfo DigInfo[MAX_NB_CAMERAS];
   MIL_TEXT_CHAR ScanDisplayText[MAX_STRING_LEN];
   };

// Scan/analyze structures and functions used for grab hook function
struct SCommonAnalysisObjects
   {
   MIL_ID MilSystem;

   MIL_ID MilGraphics;
   MIL_ID MilGraphicList;

   MIL_ID MilPtCldCtnr;

   MIL_ID MilDepthMap;

   MIL_INT NumLaserScanObjects;

   CMILDisplayManager* MilDisplays;
   CMILDisplayManager* MilResultsDisplay;

   const SMapGeneration* GenerationInfo;
   };

inline const SIllustrations& SIllustrations::operator= (const SIllustrations& o)
   {
   NumIllustrations = o.NumIllustrations;
   for (MIL_INT i = 0; i < o.NumIllustrations; i++)
      {
      MosStrcpy(IllustrationFiles[i], MAX_FILENAME_LEN, o.IllustrationFiles[i]);
      }

   return (*this);
   }

#endif
