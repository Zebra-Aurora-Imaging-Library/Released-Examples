//***************************************************************************************
//
// File name: standaloneexample.h
//
// Synopsis:  Subclass implementing CExampleInterface that reloads images on disk and
//            hardcodes the answers to all user interaction.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef STANDALONE_EXAMPLE_H
#define STANDALONE_EXAMPLE_H

#include "exampleinterface.h"

//*****************************************************************************
// Subclass implementing CExampleInterface for the stand-alone case.
// Every grab function will reload an image from disk.
// Every "question" asked to the user will be automatically answered according
// to iteration counters to show different calibration situations.
//*****************************************************************************
class CStandAloneExample : public CExampleInterface
   {
   public:
      CStandAloneExample();
      virtual ~CStandAloneExample();

      virtual bool    IsValid() const;

      virtual void    PauseInStandAloneMode() const;
      virtual void    PrintExplanationForMinContrast() const;

      virtual bool    AskMinContrastAdjust(MIL_INT* pMinContrast);
      virtual bool    AskIfFeatureExtractionAccurate();
      virtual bool    AskIfCameraCalibrationAccurate();
      virtual bool    AskIfLineExtractionAccurate();
      virtual bool    AskIfLaserCalibrationAccurate();

      virtual MIL_ID  TryToReloadCameraCalibration(MIL_CONST_TEXT_PTR CalibrationFileName) const;

      virtual void    GrabCalibrationGrid();
      virtual void    GrabLaserLineToAdjustContrast();
      virtual bool    GrabCalibrationLaserLine(MIL_INT ReferencePlaneIndex, MIL_DOUBLE CalibrationDepth, bool ShouldAskIfFinished);

   private:
      MIL_INT m_CalibrationGridCounter;            // Iteration counter used in GrabCalibrationGrid().
      MIL_INT m_LaserLineToAdjustContrastCounter;  // Iteration counter used in AskMinContrastAdjust().
      MIL_INT m_CalibrationLaserLineCounter;       // Iteration counter used in GrabCalibrationLaserLine().
      MIL_INT m_CameraCalibrationCounter;          // Iteration counter used in AskIfCameraCalibrationAccurate().
      MIL_INT m_LineExtractionCounter;             // Iteration counter used in AskIfLineExtractionAccurate().
      MIL_INT m_LaserCalibrationCounter;           // Iteration counter used in AskIfLaserCalibrationAccurate().
   };

#endif
