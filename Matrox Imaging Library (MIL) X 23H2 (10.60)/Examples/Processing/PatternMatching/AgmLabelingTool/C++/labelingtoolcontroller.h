//*******************************************************************************
// 
// File name: labelingtoolcontroller.h
//
// Synopsis: This file declares the Controller class of the Model-View-Controller
//           pattern.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#pragma once
#include "labelingtoolview.h"

class CLabelingTool;
class CLabelingToolView;

//==============================================================================
//!
class CLabelingToolController
   {
   public:
      CLabelingToolController(CLabelingTool& LabelingTool, CLabelingToolView& View);

      CLabelingToolView& GetLabelingToolView() const { return m_rLabelingToolView; }
      void NextImage() const;
      void PreviousImage() const;
      void LastImage() const;
      void FirstImage() const;
      void AddPositiveLabel() const;
      void AddNegativeLabel() const;
      void Validate() const;
      void Delete() const;
      void Save() const;
      void SelectExistingLabel() const;
      void SelectModel();

   private:
      CLabelingTool& m_rLabelingTool;
      CLabelingToolView& m_rLabelingToolView;

   };
