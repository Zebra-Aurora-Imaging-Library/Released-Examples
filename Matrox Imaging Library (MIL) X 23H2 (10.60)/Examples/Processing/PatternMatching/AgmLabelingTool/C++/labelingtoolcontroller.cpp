//*******************************************************************************
// 
// File name: labelingtoolcontroller.cpp
//
// Synopsis: This file implements the Controller class of the Model-View-Controller
//           pattern.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#include "labelingtoolcontroller.h"
#include "labelingtool.h"

//==============================================================================
//!
CLabelingToolController::CLabelingToolController(
   CLabelingTool& LabelingTool,
   CLabelingToolView& View)
   : m_rLabelingTool(LabelingTool)
   , m_rLabelingToolView(View)
   {
   // Ensure there are images in the labeling tool.
   if(m_rLabelingTool.GetNbImage() == 0)
      {
      return;
      }

   m_rLabelingToolView.RegisterOnGraListEvent(this);
   if(m_rLabelingTool.GetModelImage() == M_NULL)
      {
      m_rLabelingToolView.ShowModelSelectionView();
      m_rLabelingToolView.RegisterDisplaySizeOnBoxMovetEvent(this);
      }
   else
      {
      m_rLabelingToolView.ShowImageLabelingView();
      m_rLabelingToolView.RegisterOverlayOnBoxMoveEvent(this);
      }
   }

//==============================================================================
//!
void CLabelingToolController::NextImage() const
   {
   MIL_INT ImageIdx = m_rLabelingTool.GetCurImageIndex();
   m_rLabelingTool.SetCurImageIndex(ImageIdx + 1);
   m_rLabelingToolView.UpdateAndRedrawSelectedBox();
   }

//==============================================================================
//!
void CLabelingToolController::PreviousImage() const
   {
   MIL_INT ImageIdx = m_rLabelingTool.GetCurImageIndex();
   m_rLabelingTool.SetCurImageIndex(ImageIdx - 1);
   m_rLabelingToolView.UpdateAndRedrawSelectedBox();
   }

//==============================================================================
//!
void CLabelingToolController::LastImage() const
   {
   MIL_INT LastImageIdx = m_rLabelingTool.GetNbImage() - 1;
   m_rLabelingTool.SetCurImageIndex(LastImageIdx);
   m_rLabelingToolView.UpdateAndRedrawSelectedBox();
   }

//==============================================================================
//!
void CLabelingToolController::FirstImage() const
   {
   m_rLabelingTool.SetCurImageIndex(0);
   m_rLabelingToolView.UpdateAndRedrawSelectedBox();
   }

//==============================================================================
//!
void CLabelingToolController::AddPositiveLabel() const
   {
   m_rLabelingToolView.AddFloatingBox(M_COLOR_BLUE);
   m_rLabelingToolView.OverlayModel();
   m_rLabelingToolView.DisableAllBoxExceptCurrent();
   auto& ButtonBar = m_rLabelingToolView.GetLabelingBar();
   ButtonBar.m_AddNegativeButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_AddPositiveButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_SaveButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_ValidateButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   ButtonBar.m_DeleteButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   }

//==============================================================================
//!
void CLabelingToolController::AddNegativeLabel() const
   {
   m_rLabelingToolView.AddFloatingBox(M_COLOR_RED);
   m_rLabelingToolView.OverlayModel();
   m_rLabelingToolView.DisableAllBoxExceptCurrent();
   auto& ButtonBar = m_rLabelingToolView.GetLabelingBar();
   ButtonBar.m_AddNegativeButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_AddPositiveButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_SaveButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_ValidateButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   ButtonBar.m_DeleteButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   }

//==============================================================================
//!
void CLabelingToolController::Validate() const
   {
   SRectangle CurSelectedFloatingBox = m_rLabelingToolView.GetFloatingBox();
   m_rLabelingTool.AddLabel(CurSelectedFloatingBox);
   m_rLabelingToolView.DeleteFloatingBox();
   auto& ButtonBar = m_rLabelingToolView.GetLabelingBar();
   ButtonBar.m_AddNegativeButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   ButtonBar.m_AddPositiveButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   ButtonBar.m_SaveButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   ButtonBar.m_ValidateButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_DeleteButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   m_rLabelingToolView.Update();
   }

//==============================================================================
//!
void CLabelingToolController::Delete() const
   {
   m_rLabelingToolView.DeleteFloatingBox();
   auto& ButtonBar = m_rLabelingToolView.GetLabelingBar();
   ButtonBar.m_AddNegativeButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   ButtonBar.m_AddPositiveButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   ButtonBar.m_SaveButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   ButtonBar.m_ValidateButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_DeleteButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   m_rLabelingToolView.EnableAllBox();
   }

//==============================================================================
//!
void CLabelingToolController::Save() const
   {
   m_rLabelingTool.SaveLabeledImages();
   m_rLabelingToolView.GetLabelingBar().m_SaveButton.ChangeColor(M_COLOR_GRAY);
   MosSleep(100);
   m_rLabelingToolView.GetLabelingBar().m_SaveButton.ChangeColor(M_COLOR_BRIGHT_GRAY);
   }

//==============================================================================
//!
void CLabelingToolController::SelectExistingLabel() const
   {
   m_rLabelingToolView.UpdateCurSelectedFloatingBoxLabel();
   MIL_INT CurDisplayedBoxLabel = m_rLabelingToolView.GetCurSelectedFloatingBoxLabel();
   MIL_INT IndexOfLabel = m_rLabelingToolView.GetIndexOf(CurDisplayedBoxLabel);
   
   m_rLabelingTool.DeleteLabel(IndexOfLabel);
   m_rLabelingToolView.UpdateAndRedrawSelectedBox();
   m_rLabelingToolView.DisableAllBoxExceptCurrent();
   m_rLabelingToolView.OverlayModel();
   auto& ButtonBar = m_rLabelingToolView.GetLabelingBar();
   ButtonBar.m_AddNegativeButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_AddPositiveButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_SaveButton.SetSelectableState(CButton::ESelectableState::eDisabled);
   ButtonBar.m_ValidateButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   ButtonBar.m_DeleteButton.SetSelectableState(CButton::ESelectableState::eEnabled);
   }

//==============================================================================
//!
void CLabelingToolController::SelectModel()
   {
   SRectangle CurSelectedFloatingBox = m_rLabelingToolView.GetFloatingBox();
   m_rLabelingTool.SetModelImageAt(CurSelectedFloatingBox);
   m_rLabelingTool.SaveModelImage();
   SiSize2D ModelSize = m_rLabelingTool.GetModelSize();
   SRectangle ModelBox(CurSelectedFloatingBox.GetCenter(), ModelSize);
   m_rLabelingTool.AddLabel(ModelBox);
   m_rLabelingToolView.DeleteFloatingBox();
   m_rLabelingToolView.ShowImageLabelingView();
   m_rLabelingToolView.UnRegisterDisplaySizeOnBoxMovetEvent(this);
   m_rLabelingToolView.RegisterOverlayOnBoxMoveEvent(this);
   }
