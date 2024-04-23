//*******************************************************************************
// 
// File name: buttons.h
//
// Synopsis: Class representing a button that can be clicked in a Mil Display.
//            
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#pragma once

// ===========================================================================
const MIL_DOUBLE H_SIZE        = 40;
const MIL_DOUBLE W_SIZE        = 150;
const MIL_DOUBLE MARGIN_H_SIZE = 5;
const MIL_DOUBLE MARGIN_W_SIZE = 10;
const MIL_DOUBLE LINES         = 4;

//==============================================================================
//!
class CButton
   {
   static const MIL_INT32 SELECTABLE_ENABLED_COLOR  = M_COLOR_BRIGHT_GRAY;
   static const MIL_INT32 SELECTABLE_DISABLED_COLOR = M_COLOR_GRAY;

   public:
      enum class ESelectableState
         {
         eEnabled,
         eDisabled
         };

      CButton(
         MIL_ID GraphicList,
         MIL_ID GraContext,
         MIL_STRING Text,
         ESelectableState SelectableState,
         MIL_DOUBLE PosX,
         MIL_DOUBLE PosY,
         MIL_DOUBLE ZoomFactor):
         m_GraphicList(GraphicList),
         m_GraContext(GraContext),
         m_Text(Text),
         m_PosX(PosX),
         m_PosY(PosY),
         m_ZoomFactor(ZoomFactor)
         {
         MgraRectAngle(m_GraContext, m_GraphicList,
                       PosX * (W_SIZE + MARGIN_W_SIZE) / m_ZoomFactor,
                       -(LINES - PosY) * (H_SIZE + MARGIN_H_SIZE) / m_ZoomFactor,
                       W_SIZE / m_ZoomFactor,
                       H_SIZE / m_ZoomFactor,
                       0, M_FILLED);

         MgraInquireList(m_GraphicList, M_LIST, M_DEFAULT, M_LAST_LABEL, &m_RectLabel);
         MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_RectLabel), M_DEFAULT, M_TRANSLATABLE, M_DISABLE);
         SetSelectableState(SelectableState);
         SetText(Text);
         }

      void SetSelectableState(ESelectableState State)
         {
         switch(State)
            {
            case ESelectableState::eEnabled:
               MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_RectLabel), M_DEFAULT, M_SELECTABLE, M_ENABLE);
               MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_RectLabel), M_DEFAULT, M_COLOR, M_COLOR_BRIGHT_GRAY);
               break;
            case ESelectableState::eDisabled:
               MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_RectLabel), M_DEFAULT, M_SELECTABLE, M_DISABLE);
               MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_RectLabel), M_DEFAULT, M_COLOR, M_COLOR_GRAY);
               break;
            default:
               break;
            }
         }

      void Unselect()
         {
         MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_RectLabel), M_DEFAULT, M_GRAPHIC_SELECTED, M_FALSE);
         }

      void SetText(MIL_STRING Text)
         {
         if(m_TextLabel != M_INVALID)
            {
            MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_TextLabel), M_DEFAULT, M_DELETE, M_DEFAULT);
            }
         MgraColor(m_GraContext, M_COLOR_BLACK);
         MgraText(m_GraContext, m_GraphicList,
                  (m_PosX * (W_SIZE + MARGIN_W_SIZE) + W_SIZE / 2) / m_ZoomFactor,
                  (-(LINES - m_PosY) * (H_SIZE + MARGIN_H_SIZE) + (H_SIZE / 2)) / m_ZoomFactor,
                  Text);
         MgraInquireList(m_GraphicList, M_LIST, M_DEFAULT, M_LAST_LABEL, &m_TextLabel);
         MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_TextLabel), M_DEFAULT, M_SELECTABLE, M_DISABLE);
         }

      MIL_INT GetRectLabel() const
         {
         return m_RectLabel;
         }

      void ChangeColor(MIL_DOUBLE Color)
         {
         MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_RectLabel), M_DEFAULT, M_COLOR, Color);
         }

      void Appear()
         {
         MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_RectLabel), M_DEFAULT, M_VISIBLE, M_TRUE);
         MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_TextLabel), M_DEFAULT, M_VISIBLE, M_TRUE);
         }

      void Disappear()
         {
         MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_RectLabel), M_DEFAULT, M_VISIBLE, M_FALSE);
         MgraControlList(m_GraphicList, M_GRAPHIC_LABEL(m_TextLabel), M_DEFAULT, M_VISIBLE, M_FALSE);
         }

   private:
      MIL_ID m_GraphicList {M_NULL};
      MIL_ID m_GraContext {M_NULL};;
      MIL_DOUBLE m_Color {0.0};
      MIL_STRING m_Text;
      MIL_INT m_RectLabel {M_INVALID};
      MIL_INT m_TextLabel {M_INVALID};
      MIL_DOUBLE m_PosX {0.0};
      MIL_DOUBLE m_PosY {0.0};
      MIL_DOUBLE m_ZoomFactor {1.0};
   };

//===========================================================================
//!
struct SImageNavigationBar
   {
   SImageNavigationBar(
      MIL_ID GraphicList,
      MIL_ID GraContext,
      MIL_DOUBLE ZoomFactor):
      m_StepTitle(GraphicList          , GraContext, MIL_TEXT("")   , CButton::ESelectableState::eDisabled, 2, 0, ZoomFactor),
      m_FirstImageButton(GraphicList   , GraContext, MIL_TEXT("<< "), CButton::ESelectableState::eEnabled , 0, 1, ZoomFactor),
      m_PreviousImageButton(GraphicList, GraContext, MIL_TEXT("<")  , CButton::ESelectableState::eEnabled , 1, 1, ZoomFactor),
      m_CurrentImageButton(GraphicList , GraContext, MIL_TEXT("")   , CButton::ESelectableState::eDisabled, 2, 1, ZoomFactor),
      m_NextImageButton(GraphicList    , GraContext, MIL_TEXT(">")  , CButton::ESelectableState::eEnabled , 3, 1, ZoomFactor),
      m_LastImageButton(GraphicList    , GraContext, MIL_TEXT(">>") , CButton::ESelectableState::eEnabled , 4, 1, ZoomFactor),
      m_ImageNameButton(GraphicList    , GraContext, MIL_TEXT("")   , CButton::ESelectableState::eDisabled, 2, 3, ZoomFactor)
      {
      m_StepTitle.ChangeColor(M_COLOR_WHITE);
      m_ImageNameButton.ChangeColor(M_COLOR_WHITE);
      }

   void DispCurrentImageIndex(MIL_INT CurrentIndex, MIL_INT NumImages)
      {
      MIL_STRING ImageNumber = M_TO_STRING(CurrentIndex + 1) + MIL_TEXT(" / ") + M_TO_STRING((MIL_INT)NumImages);
      m_CurrentImageButton.SetText(ImageNumber);
      }

   CButton m_StepTitle;
   CButton m_FirstImageButton;
   CButton m_PreviousImageButton;
   CButton m_CurrentImageButton;
   CButton m_NextImageButton;
   CButton m_LastImageButton;
   CButton m_ImageNameButton;
   };

//==============================================================================
//!
struct SButtonsModelSelection
   {
   SButtonsModelSelection(
      MIL_ID GraphicList,
      MIL_ID GraContext,
      MIL_DOUBLE ZoomFactor):
      m_ValidateButton(GraphicList, GraContext, MIL_TEXT("Validate"), CButton::ESelectableState::eEnabled , 2.0, 2.0, ZoomFactor),
      m_DispSizeButton(GraphicList, GraContext, MIL_TEXT("")        , CButton::ESelectableState::eDisabled, 0.5, 2.0, ZoomFactor),
      m_AllButtons {&m_ValidateButton, &m_DispSizeButton}
      {
      m_DispSizeButton.ChangeColor(M_COLOR_WHITE);
      }

   void Appear()
      {
      for(auto& Button : m_AllButtons)
         {
         Button->Appear();
         }
      }

   void Disappear()
      {
      for(auto& Button : m_AllButtons)
         {
         Button->Disappear();
         }
      }

   CButton m_ValidateButton;
   CButton m_DispSizeButton;
   std::vector<CButton*> m_AllButtons;
   };

//==============================================================================
//!
struct SButtonsLabelTargetImages
   {
   SButtonsLabelTargetImages(
      MIL_ID GraphicList,
      MIL_ID GraContext,
      MIL_DOUBLE ZoomFactor):
      m_AddPositiveButton(GraphicList, GraContext, MIL_TEXT("Add positive label"), CButton::ESelectableState::eEnabled , 0, 2, ZoomFactor),
      m_AddNegativeButton(GraphicList, GraContext, MIL_TEXT("Add negative label"), CButton::ESelectableState::eEnabled , 1, 2, ZoomFactor),
      m_ValidateButton(GraphicList   , GraContext, MIL_TEXT("Validate")          , CButton::ESelectableState::eDisabled, 2, 2, ZoomFactor),
      m_DeleteButton(GraphicList     , GraContext, MIL_TEXT("Delete")            , CButton::ESelectableState::eDisabled, 3, 2, ZoomFactor),
      m_SaveButton(GraphicList       , GraContext, MIL_TEXT("Save")              , CButton::ESelectableState::eEnabled , 4, 2, ZoomFactor),
      m_AllButtons {&m_AddPositiveButton, &m_AddNegativeButton, &m_ValidateButton, &m_DeleteButton, &m_SaveButton}
      {
      }

   void Appear()
      {
      for(auto& Button : m_AllButtons)
         {
         Button->Appear();
         }
      }

   void Disappear()
      {
      for(auto& Button : m_AllButtons)
         {
         Button->Disappear();
         }
      }

   CButton m_AddPositiveButton;
   CButton m_AddNegativeButton;
   CButton m_ValidateButton;
   CButton m_DeleteButton;
   CButton m_SaveButton;
   std::vector<CButton*> m_AllButtons;
   };
