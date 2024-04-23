//***************************************************************************************
//
// File name: ProfileDisplay.h 
//
// Synopsis:  This file contains the declaration of the CProfileDisplay class 
//            which displays the profile associated to a measurement marker.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef PROFILEDISPLAY_H
#define PROFILEDISPLAY_H

//*****************************************************************************
// Profile display constants.
//*****************************************************************************
static const MIL_INT PROFILE_SIZE_X    = 750;
static const MIL_INT PROFILE_SIZE_Y    = 401;
static const MIL_DOUBLE PROFILE_BLUE   = M_RGB888(51,153,255);

class CProfileDisplay
   {
   public:
      // Constructor.
      CProfileDisplay(MIL_ID MilSystem);

      // Destructor.
      virtual ~CProfileDisplay();

      // Function to create the profile.
      void CreateProfile(MIL_ID MilImage, MIL_ID MilMeasMarker);
      
      // Update function.
      void SetUpdate(MIL_INT Update);

      // Clear functions.
      void ClearAnnotations();
      void Clear();
      
      // Accessor.
      MIL_INT ProfileImageSizeY() const;

   private:

      // Function that computes the intensity profile of the created profile.
      void GetIntensityProfile();

      MIL_ID m_MilSystem;
      MIL_ID m_MilProfileDisplay;
      MIL_ID m_MilProfileImage;
      MIL_ID m_MilProfileGraList;

      MIL_ID m_MilProjResult;
      MIL_DOUBLE m_ProfileValues[PROFILE_SIZE_X];
      MIL_DOUBLE m_ProfileValuesPos[PROFILE_SIZE_X];
   };

#endif // PROFILEDISPLAY_H
