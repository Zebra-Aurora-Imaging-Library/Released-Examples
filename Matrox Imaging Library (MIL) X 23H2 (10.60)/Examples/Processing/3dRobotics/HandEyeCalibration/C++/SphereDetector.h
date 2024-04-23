//***************************************************************************************/
// 
// File name: SphereDetector.h  
//
// Synopsis:  This class detects and localize spheres in a calibration model.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//**************************************************************************************/

#pragma once
#include <mil.h>
#include <algorithm>
#include <vector>
#include"HandEyeUtils.h"

//*****************************************************************************
// Struct that holds information about detected spheres
//*****************************************************************************
struct SphereDetectorResult
   {
   MIL_UNIQUE_3DMOD_ID MilModResult;
   std::vector<SphereStats> DetectedSpheres;
   };

//*****************************************************************************
// Class that looks for spheres in a model
//*****************************************************************************
class SphereDetector
   {
   public:
      SphereDetector(MIL_INT aNumSpheres, const MIL_DOUBLE* aRadiusClasses, MIL_DOUBLE aRadiusTolerance);
      std::vector<SphereStats> RetrieveModelSpheres(const MIL_ID &MilSystem, const MIL_ID MilCloud, MIL_ID MilSpheresDisplay, MIL_ID MilPoseDisplay);

   private:    
      SphereDetectorResult DetectSpheresAllRadius(MIL_ID MilSystem, MIL_ID MilCloud);
      std::vector<SphereStats> DetectSpheres(MIL_ID MilCloud, const MIL_DOUBLE MinRadius, const MIL_DOUBLE MaxRadius, MIL_ID Context, MIL_ID Result, MIL_INT NumberOfSpheres);

   private:
      std::vector<MIL_DOUBLE> mSphereRadii;
      MIL_DOUBLE mRadiusTolerance;
   };
