//******************************************************************************
// 
// File name: ProfileInspector.h
//
// Synopsis:  This file holds the profile inspector class.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//******************************************************************************

#ifndef __PROFILE_INSPECTOR_H
#define __PROFILE_INSPECTOR_H
#include <mil.h>
#include "Utilities.h"

//*******************************************************************************
// Class that performs the profile inspection of a scanned object.
//*******************************************************************************
class ProfileInspector
   {
   public:
      ProfileInspector(MIL_ID MilSystem,
                       const ProfileXY<MIL_DOUBLE>& RefProfilePoints,
                       const ProfileObject ScannedObject,
                       const MIL_DOUBLE MaxAreaTolerance,
                       MIL_ID MilDispScanned,
                       MIL_ID MilDispProfile);
      ~ProfileInspector() {};

      void IsVerbose(bool IsVerbose) { m_IsVerbose = IsVerbose; }
      bool IsVerbose() const { return m_IsVerbose; }
      void InspectProfiles();

   private:
      MIL_ID m_MilSystem;                               // MIL system ID.
      const ProfileXY<MIL_DOUBLE> m_RefProfilePoints;   // Reference object's profile points.
      ProfileXY<MIL_DOUBLE> m_ScannedProfilePoints;     // Scanned object's profile points.
      ProfileObject m_ScannedObject;                    // Scanned object's information.
      MIL_DOUBLE m_MaxAreaTolerance;                    // Maximum area inspection tolerance.
      MIL_UNIQUE_3DDISP_ID m_MilDispScanned;            // MIL display ID of the scanned object.
      MIL_UNIQUE_DISP_ID m_MilDispProfile;              // MIL display ID of the profiles.
      bool m_IsVerbose;                                 // Verbosity flag.
      std::vector<FailedResult> m_FailedResults;        // Vector holding failed inspection results.
      MIL_UNIQUE_3DIM_ID m_MilScannedProfileResult;     // Result of the scanned object's profile.
      MIL_UNIQUE_MET_ID m_MilMetContext;                // Context for M3dmetCalculate, which does the profile inspection.
      MIL_UNIQUE_MET_ID m_MilMetResult;                 // Result for M3dmetCalculate, which does the profile inspection.
      MIL_UNIQUE_DISP_ID m_MilFailedDisplay;            // MIL display ID of a failed profile inspection.

      InspectionResult SliceAndInspectProfile(MIL_DOUBLE Position, MIL_DOUBLE PlaneSize);
      InspectionResult InspectProfile();
      void PrintFailedResults() const;
      void InteractivelyDisplayFailures();
   };

#endif
