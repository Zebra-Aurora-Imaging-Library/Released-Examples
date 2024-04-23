//*******************************************************************************
//
// File name: BaseCommon.h
//
// Synopsis:  Includes all common headers and defines some common constants.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef BASECOMMON_H
#define BASECOMMON_H

#include <mil.h>
#include <math.h>
#include <algorithm>

using namespace std;

// Common constants
// Maximal sizes of arrays.
static const MIL_INT MAX_FILENAME_LEN              = 256;
static const MIL_INT MAX_STRING_LEN                = 512;
static const MIL_INT MAX_NB_CAMERAS                = 8;
static const MIL_INT MAX_NB_LASERS                 = 8;
static const MIL_INT MAX_NB_REF_PLANES             = 16;
static const MIL_INT MAX_NB_ILLUSTRATIONS_PER_STEP = 2;

// Common graphics constants.
static const MIL_INT    TEXT_OFFSET_X           = 20;
static const MIL_INT    TEXT_OFFSET_Y           = 20;
static const MIL_DOUBLE TEXT_FONT_SIZE_SMALL    = 12;
static const MIL_DOUBLE TEXT_FONT_SIZE_MEDIUM   = 14;
static const MIL_DOUBLE TEXT_FONT_SIZE_LARGE    = 16;
#define TEXT_FONT_NAME MIL_FONT_NAME(M_FONT_DEFAULT_TTF MIL_TEXT(":Bold"))

// 3D Display constants.
static const MIL_INT M3D_DISPLAY_SIZE_X         = 700;
static const MIL_INT M3D_DISPLAY_SIZE_Y         = 700;
static const MIL_DOUBLE MAX_DISTANCE_Z          = 2.0;

// Forward declarations.
class CMILDisplayManager;

// Enum for analysis wait events.
enum AnalysisWaitType
   {
   eKillThread,
   eStartAnalysis,
   eLastAnalysisWaitType
   };

// Enumeration for example steps.
enum ExampleSteps
   {
   eCameraCalibration,
   eLaserCalibration,
   eObjectScan,
   eObjectAnalysis,
   eNum3dExampleSteps
   };

// Common headers to include.
#include "CommonUtils.h"
#include "3dStructs.h"
inline bool GenerateDepthMap(MIL_ID MilContainer, MIL_ID MilSystem, const SMapGeneration& GenerationInfo, MIL_ID* pOutDepthmap)
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
//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.  
//*****************************************************************************
inline MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n\n"));
      }
   return MilDisplay3D;
   }

#include "DisplayManager.h"
#include "MilDisplayManager.h"
#include "C3DDisplayManager.h"
#include "ExampleManagerFor3D.h"

#endif
