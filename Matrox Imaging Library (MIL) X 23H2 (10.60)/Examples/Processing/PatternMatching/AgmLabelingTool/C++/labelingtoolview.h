//*******************************************************************************
// 
// File name: labelingtoolview.h
//
// Synopsis: This file declares the View class of the Model-View-Controller
//           pattern.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#pragma once
#include <mil.h>
#include "buttons.h"
#include "util.h"
#include <unordered_map>

class CLabelingTool;
class CLabelingToolController;

//==============================================================================
//!
class CLabelingToolView
   {
   public:
      CLabelingToolView(const CLabelingTool& LabelingTool);

      void Update();
      void UpdateAndRedrawSelectedBox();
      void RegisterOverlayOnBoxMoveEvent(CLabelingToolController* pController);
      void RegisterOnGraListEvent(CLabelingToolController* pController);
      void RegisterDisplaySizeOnBoxMovetEvent(CLabelingToolController* pController);
      void UnRegisterDisplaySizeOnBoxMovetEvent(CLabelingToolController* pController);
      SImageNavigationBar& GetNavigationBar() const { return *m_pNavigationBar; }
      SButtonsLabelTargetImages& GetLabelingBar() const { return *m_pLabelingBar; }
      SButtonsModelSelection& GetModelSelectionBar() const { return *m_pModelSelectionBar; }
      void AddFloatingBox(MIL_INT Color);
      void AddFloatingBox(const SRectangle& Box);
      void DeleteFloatingBox();
      MIL_INT GetCurSelectedFloatingBoxLabel() const;
      void UpdateCurSelectedFloatingBoxLabel();
      SRectangle GetFloatingBox() const;
      void DisableAllBoxExceptCurrent();
      void EnableAllBox();
      void SelectBox(MIL_INT GraLabel);
      MIL_INT GetIndexOf(MIL_INT DisplayedBoxLabel) const;
      void OverlayModel();
      void ShowModelSelectionView();
      void ShowImageLabelingView();
      bool IsExistingLabel(MIL_INT GraLabel) const;
      void DisplayFloatingBoxSize();
      bool IsFloatingBoxInsideImage() const;

   private:
      bool IsValidGraLabel(MIL_INT GraLabel) const;
      void ClearLabels();
      void DrawLabels();
      void ResetOverlay();

   private:
      const CLabelingTool& m_rLabelingTool;

      MIL_ID m_System = M_DEFAULT_HOST;
      MIL_UNIQUE_GRA_ID m_GraCtx;
      MIL_UNIQUE_GRA_ID m_GraList;
      MIL_UNIQUE_DISP_ID m_Display;
      MIL_UNIQUE_BUF_ID m_ModelOverlay;
      std::unique_ptr<SImageNavigationBar> m_pNavigationBar;
      std::unique_ptr<SButtonsLabelTargetImages> m_pLabelingBar;
      std::unique_ptr<SButtonsModelSelection> m_pModelSelectionBar;

      MIL_DOUBLE m_ZoomFactor {0.0};
      std::unordered_map<MIL_INT, MIL_INT> m_LabelToIndexMap;
      MIL_INT m_CurSelectedFloatingBoxLabel = M_INVALID;
      bool m_MustShowLabels = true;
   };
