//***************************************************************************************
//
// File name: interactiveexample.h
//
// Synopsis:  Subclass implementing CExampleInterface that uses live grabbing and the
//            console to prompt the user.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef INTERACTIVE_EXAMPLE_H
#define INTERACTIVE_EXAMPLE_H

#include "exampleinterface.h"

//*****************************************************************************
// Subclass implementing CExampleInterface for the interactive case.
// Every grab function will actually grab an image using a MIL digitizer.
// Every question will be asked to the user with MosGetch().
//*****************************************************************************
class CInteractiveExample : public CExampleInterface
   {
   public:
      CInteractiveExample();
      virtual ~CInteractiveExample();

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
      static bool AskYesNo(MIL_CONST_TEXT_PTR QuestionString);

      MIL_ID m_MilDigitizer;  // ID obtained with MdigAlloc().
   };

#endif
