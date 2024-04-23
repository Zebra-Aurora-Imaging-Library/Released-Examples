//*******************************************************************************
// 
// File name: labelingtoolview.cpp
//
// Synopsis: This file implements the View class of the Model-View-Controller
//           pattern.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#include "labelingtoolview.h"
#include "labelingtool.h"
#include "labelingtoolcontroller.h"

static const MIL_INT ALPHA_VALUE = 50;
static const MIL_INT SELECTION_RADIUS = 10;
static const MIL_DOUBLE MAX_IMAGE_DISPLAY_SIZE = 800;

//==============================================================================
//! 
MIL_INT MFTYPE OnGraListClick(MIL_INT /*HookType*/, MIL_ID EventId, void* UserData)
   {
   CLabelingToolController& Controller = *(CLabelingToolController*)UserData;
   CLabelingToolView& View = Controller.GetLabelingToolView();

   // Get the label of the selected graphic.
   MIL_INT GraphicSelected;
   MIL_INT GraphicList;
   MgraGetHookInfo(EventId, M_GRAPHIC_LABEL_VALUE, &GraphicSelected);
   MgraGetHookInfo(EventId, M_GRAPHIC_LIST_ID, &GraphicList);

   auto& NavigationBar     = View.GetNavigationBar();
   auto& LabelingBar       = View.GetLabelingBar();
   auto& ModelSelectionBar = View.GetModelSelectionBar();
   if(GraphicSelected == NavigationBar.m_NextImageButton.GetRectLabel())
      {
      Controller.NextImage();
      NavigationBar.m_NextImageButton.Unselect();
      }
   else if(GraphicSelected == NavigationBar.m_PreviousImageButton.GetRectLabel())
      {
      Controller.PreviousImage();
      NavigationBar.m_PreviousImageButton.Unselect();
      }
   else if(GraphicSelected == NavigationBar.m_LastImageButton.GetRectLabel())
      {
      Controller.LastImage();
      NavigationBar.m_LastImageButton.Unselect();
      }
   else if(GraphicSelected == NavigationBar.m_FirstImageButton.GetRectLabel())
      {
      Controller.FirstImage();
      NavigationBar.m_FirstImageButton.Unselect();
      }
   else if(GraphicSelected == ModelSelectionBar.m_ValidateButton.GetRectLabel())
      {
      Controller.SelectModel();
      }
   else if(GraphicSelected == LabelingBar.m_AddPositiveButton.GetRectLabel())
      {
      Controller.AddPositiveLabel();
      LabelingBar.m_AddPositiveButton.Unselect();
      }
   else if(GraphicSelected == LabelingBar.m_AddNegativeButton.GetRectLabel())
      {
      Controller.AddNegativeLabel();
      LabelingBar.m_AddNegativeButton.Unselect();
      }
   else if(GraphicSelected == LabelingBar.m_ValidateButton.GetRectLabel())
      {
      Controller.Validate();
      LabelingBar.m_ValidateButton.Unselect();
      }
   else if(GraphicSelected == LabelingBar.m_DeleteButton.GetRectLabel())
      {
      Controller.Delete();
      LabelingBar.m_DeleteButton.Unselect();
      }
   else if(GraphicSelected == LabelingBar.m_SaveButton.GetRectLabel())
      {
      Controller.Save();
      LabelingBar.m_SaveButton.Unselect();
      }
   else if(View.IsExistingLabel(GraphicSelected)) // GraphicSelected is an existing movable bounding box.
      {
      Controller.SelectExistingLabel();
      }
   else if(GraphicSelected == M_NO_LABEL)// Keep the current selected graphic active.
      {
      MIL_INT GraphicUnselected;
      MgraGetHookInfo(EventId, M_GRAPHIC_LABEL_VALUE_DESELECTED, &GraphicUnselected);
      View.SelectBox(GraphicUnselected);
      }

   return 0;
   }

//==============================================================================
//!
MIL_INT MFTYPE OverlayOnBoxMove(MIL_INT /*HookType*/, MIL_ID EventId, void* UserData)
   {
   CLabelingToolController& Controller = *(CLabelingToolController*)UserData;
   CLabelingToolView& View = Controller.GetLabelingToolView();

   MIL_INT ControlType;
   MgraGetHookInfo(EventId, M_GRAPHIC_CONTROL_TYPE, &ControlType);

   if(ControlType == M_GRAPHIC_INTERACTIVE)
      {
      View.OverlayModel();
      }

   return 0;
   }

//==============================================================================
//!
MIL_INT MFTYPE DisplaySizeOnBoxMove(MIL_INT /*HookType*/, MIL_ID EventId, void* UserData)
   {
   CLabelingToolController& Controller = *(CLabelingToolController*)UserData;
   CLabelingToolView& View = Controller.GetLabelingToolView();
   MIL_INT ControlType;
   MgraGetHookInfo(EventId, M_GRAPHIC_CONTROL_TYPE, &ControlType);
   if(ControlType == M_GRAPHIC_INTERACTIVE)
      {
      MthrWait(M_DEFAULT, M_THREAD_WAIT, M_NULL); // Wait for the bouding box to be modified before displaying its size.
      View.DisplayFloatingBoxSize();
      if(View.IsFloatingBoxInsideImage())
         {
         View.GetModelSelectionBar().m_ValidateButton.SetSelectableState(CButton::ESelectableState::eEnabled);
         }
      else
         {
         View.GetModelSelectionBar().m_ValidateButton.SetSelectableState(CButton::ESelectableState::eDisabled);
         }
      }
   return 0;
   }

//==============================================================================
//!
static SdSize2D CmptMaxImageSize(const std::vector<SLabeledImage>& Images)
   {
   SdSize2D MaxImageSize {0.0, 0.0};
   for(const auto& Image : Images)
      {
      MaxImageSize.x = std::max(MaxImageSize.x, (MIL_DOUBLE)MbufInquire(Image.m_Id, M_SIZE_X, M_NULL));
      MaxImageSize.y = std::max(MaxImageSize.y, (MIL_DOUBLE)MbufInquire(Image.m_Id, M_SIZE_Y, M_NULL));
      }
   return MaxImageSize;
   }

//==============================================================================
//!
CLabelingToolView::CLabelingToolView(const CLabelingTool& LabelingTool)
   : m_rLabelingTool(LabelingTool)
   {
   // Ensure there are images in the labeling tool.
   if(m_rLabelingTool.GetNbImage() == 0)
      {
      return;
      }

   m_System = LabelingTool.GetSystemId();
   m_Display = MdispAlloc(m_System, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   m_GraCtx = MgraAlloc(m_System, M_UNIQUE_ID);
   m_GraList = MgraAllocList(m_System, M_DEFAULT, M_UNIQUE_ID);

   // Set up graphic list.
   MgraControlList(m_GraList, M_LIST, M_DEFAULT, M_SELECTION_RADIUS, SELECTION_RADIUS);
   MgraControlList(m_GraList, M_LIST, M_DEFAULT, M_ACTION_KEYS, M_ENABLE);
   MgraControlList(m_GraList, M_LIST, M_DEFAULT, M_MULTIPLE_SELECTION, M_DISABLE);

   // Set up graphic context.
   MgraControl(m_GraCtx, M_TEXT_ALIGN_VERTICAL, M_CENTER);
   MgraControl(m_GraCtx, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
   MgraControl(m_GraCtx, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraControl(m_GraCtx, M_ROTATABLE, M_DISABLE);
   MgraControl(m_GraCtx, M_RESIZABLE, M_DISABLE);

   // Set up MIL display.
   SdSize2D MaxImageSize = CmptMaxImageSize(m_rLabelingTool.GetLabeledImages());
   MIL_DOUBLE MaxDim = std::max(MaxImageSize.x, MaxImageSize.y);
   m_ZoomFactor = (MaxDim > MAX_IMAGE_DISPLAY_SIZE) ? (MAX_IMAGE_DISPLAY_SIZE / MaxDim) : 1.0;
   MdispControl(m_Display, M_GRAPHIC_LIST_INTERACTIVE, M_ENABLE);
   MdispControl(m_Display, M_ASSOCIATED_GRAPHIC_LIST_ID, m_GraList);
   MdispControl(m_Display, M_KEYBOARD_USE, M_DISABLE);
   MdispControl(m_Display, M_MOUSE_USE, M_DISABLE);
   MdispControl(m_Display, M_OVERLAY_OPACITY, ALPHA_VALUE);
   MIL_INT WindowX = (MIL_INT)std::max(MaxImageSize.x * m_ZoomFactor + 2 * MARGIN_W_SIZE, (W_SIZE + MARGIN_W_SIZE) * 5 + MARGIN_W_SIZE);
   MIL_INT WindowY = (MIL_INT)(MaxImageSize.y * m_ZoomFactor + LINES * (H_SIZE + MARGIN_H_SIZE) + MARGIN_H_SIZE);
   MdispControl(m_Display, M_WINDOW_INITIAL_SIZE_X, WindowX);
   MdispControl(m_Display, M_WINDOW_INITIAL_SIZE_Y, WindowY);
   MdispControl(m_Display, M_WINDOW_SIZE_AUTO_RESET, M_DISABLE);
   MdispZoom(m_Display, m_ZoomFactor, m_ZoomFactor);

   // Create buttons.
   m_pNavigationBar = std::make_unique<SImageNavigationBar>(m_GraList, m_GraCtx, m_ZoomFactor);
   m_pLabelingBar = std::make_unique<SButtonsLabelTargetImages>(m_GraList, m_GraCtx, m_ZoomFactor);
   m_pModelSelectionBar = std::make_unique<SButtonsModelSelection>(m_GraList, m_GraCtx, m_ZoomFactor);
   
   // Initialize overlay image.
   m_ModelOverlay.reset(MbufAlloc2d(m_System, (MIL_INT)MaxImageSize.x, (MIL_INT)MaxImageSize.y, 8+M_UNSIGNED, M_IMAGE+M_PROC, M_UNIQUE_ID).release());

   // Display first image.
   MdispSelect(m_Display, m_rLabelingTool.GetLabeledImages()[0].m_Id);
   MdispPan(m_Display, -MARGIN_W_SIZE / m_ZoomFactor, -LINES * (H_SIZE + MARGIN_H_SIZE) / m_ZoomFactor);
   }

//==============================================================================
//!
bool CLabelingToolView::IsValidGraLabel(MIL_INT GraLabel) const
   {
   return MgraInquireList(m_GraList, M_GRAPHIC_LABEL(GraLabel), M_DEFAULT, M_INDEX_VALUE, M_NULL) != M_INVALID;
   }

//==============================================================================
void CLabelingToolView::ClearLabels()
   {
   // A Label is a non-filled rectangle (as opposed to a button, which is a filled rectangle).
   MIL_INT NbGraphic = MgraInquireList(m_GraList, M_LIST, M_DEFAULT, M_NUMBER_OF_GRAPHICS + M_TYPE_MIL_INT, M_NULL);
   std::vector<MIL_INT> LabelToDeletes;
   for(MIL_INT i = 0; i < NbGraphic; ++i)
      {
      MIL_INT GraphicType = MgraInquireList(m_GraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_GRAPHIC_TYPE, M_NULL);
      if(GraphicType == M_GRAPHIC_TYPE_RECT)
         {
         MIL_INT IsFilled = MgraInquireList(m_GraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_FILLED, M_NULL);
         if(IsFilled == M_FALSE)
            {
            MIL_INT CurLabel = MgraInquireList(m_GraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_LABEL_VALUE, M_NULL);
            LabelToDeletes.push_back(CurLabel);
            }
         }
      }
   for(MIL_INT Label : LabelToDeletes)
      {
      MgraControlList(m_GraList, M_GRAPHIC_LABEL(Label), M_DEFAULT, M_DELETE, M_DEFAULT);
      }
   m_LabelToIndexMap.clear();
   }

//==============================================================================
//!
void CLabelingToolView::DrawLabels()
   {
   MdispControl(m_Display, M_UPDATE_GRAPHIC_LIST, M_DISABLE);
   MIL_INT CurImageIndex = m_rLabelingTool.GetCurImageIndex();
   const auto& CurLabeledImage = m_rLabelingTool.GetLabeledImages()[CurImageIndex];
   m_LabelToIndexMap.clear();
   for(MIL_INT Index = 0; Index < (MIL_INT)CurLabeledImage.m_Labels.size(); ++Index)
      {
      const auto& Label = CurLabeledImage.m_Labels[Index];
      MgraRect(m_GraCtx, m_GraList, Label.TopLeft.x, Label.TopLeft.y, Label.BottomRight.x, Label.BottomRight.y);
      MIL_INT LastBoxLabel {M_INVALID};
      MgraInquireList(m_GraList, M_LIST, M_DEFAULT, M_LAST_LABEL, &LastBoxLabel);
      MgraControlList(m_GraList, M_GRAPHIC_LABEL(LastBoxLabel), M_DEFAULT, M_COLOR, Label.Color);
      MgraControlList(m_GraList, M_GRAPHIC_LABEL(LastBoxLabel), M_DEFAULT, M_TRANSLATABLE, M_DISABLE);
      m_LabelToIndexMap.insert({LastBoxLabel, Index});
      }
   MdispControl(m_Display, M_UPDATE_GRAPHIC_LIST, M_ENABLE);
   }

//==============================================================================
//!
void CLabelingToolView::DisableAllBoxExceptCurrent()
   {
   MdispControl(m_Display, M_UPDATE_GRAPHIC_LIST, M_DISABLE);
   MIL_INT NbGraphic = MgraInquireList(m_GraList, M_LIST, M_DEFAULT, M_NUMBER_OF_GRAPHICS + M_TYPE_MIL_INT, M_NULL);
   for(MIL_INT i = 0; i < NbGraphic; ++i)
      {
      MIL_INT CurLabel = MgraInquireList(m_GraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_LABEL_VALUE, M_NULL);
      if(IsExistingLabel(CurLabel) && CurLabel != m_CurSelectedFloatingBoxLabel)
         {
         MgraControlList(m_GraList, M_GRAPHIC_LABEL(CurLabel), M_DEFAULT, M_SELECTABLE, M_DISABLE);
         }
      }
   MdispControl(m_Display, M_UPDATE_GRAPHIC_LIST, M_ENABLE);
   }

//==============================================================================
//!
void CLabelingToolView::EnableAllBox()
   {
   MIL_INT NbGraphic = MgraInquireList(m_GraList, M_LIST, M_DEFAULT, M_NUMBER_OF_GRAPHICS + M_TYPE_MIL_INT, M_NULL);
   for(MIL_INT i = 0; i < NbGraphic; ++i)
      {
      MIL_INT CurLabel = MgraInquireList(m_GraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_LABEL_VALUE, M_NULL);
      if(IsExistingLabel(CurLabel))
         {
         MgraControlList(m_GraList, M_GRAPHIC_LABEL(CurLabel), M_DEFAULT, M_SELECTABLE, M_ENABLE);
         }
      }
   }

//==============================================================================
//!
void CLabelingToolView::SelectBox(MIL_INT GraLabel)
   {
   if(IsValidGraLabel(GraLabel))
      {
      MgraControlList(m_GraList, M_GRAPHIC_LABEL(GraLabel), M_DEFAULT, M_GRAPHIC_SELECTED, M_TRUE);
      }
   }

//==============================================================================
//!
MIL_INT CLabelingToolView::GetIndexOf(MIL_INT DisplayedBoxLabel) const
   {
   if(m_LabelToIndexMap.find(DisplayedBoxLabel) != m_LabelToIndexMap.end())
      {
      return m_LabelToIndexMap.at(DisplayedBoxLabel);
      }
   else
      {
      return M_INVALID;
      }
   }

//==============================================================================
//!
void CLabelingToolView::OverlayModel()
   {
   if(m_rLabelingTool.GetModelImage() == M_NULL)
      {
      return;
      }
   SdPoint2D TopLeft;
   MgraInquireList(m_GraList, M_GRAPHIC_LABEL(m_CurSelectedFloatingBoxLabel), M_DEFAULT, M_CORNER_TOP_LEFT_X, &TopLeft.x);
   MgraInquireList(m_GraList, M_GRAPHIC_LABEL(m_CurSelectedFloatingBoxLabel), M_DEFAULT, M_CORNER_TOP_LEFT_Y, &TopLeft.y);

   MIL_ID CurImage = m_rLabelingTool.GetCurImageId();
   MIL_ID MilOverlay = MdispInquire(m_Display, M_OVERLAY_ID, M_NULL);
   MbufCopy(CurImage, m_ModelOverlay);

   MimTranslate(m_rLabelingTool.GetModelImage(),
                m_ModelOverlay,
                TopLeft.x,
                TopLeft.y,
                M_BILINEAR + M_OVERSCAN_ENABLE);

   MbufCopy(m_ModelOverlay, MilOverlay);
   }

//==============================================================================
//!
void CLabelingToolView::ShowModelSelectionView()
   {
   m_pLabelingBar->Disappear();
   m_pModelSelectionBar->Appear();
   m_pNavigationBar->m_StepTitle.SetText(MIL_TEXT("1) Select reference model"));
   m_MustShowLabels = false;
   Update();

   MIL_DOUBLE TargetCenterX = (MIL_DOUBLE)MbufInquire(m_rLabelingTool.GetCurImageId(), M_SIZE_X, M_NULL) / 2.0;
   MIL_DOUBLE TargetCenterY = (MIL_DOUBLE)MbufInquire(m_rLabelingTool.GetCurImageId(), M_SIZE_Y, M_NULL) / 2.0;
   SRectangle LabeledBox;
   if(m_rLabelingTool.GetFirstLabeledBox(&LabeledBox))
      {
      SdSize2D BoxSize = LabeledBox.GetSize();
      LabeledBox = SRectangle({TargetCenterX, TargetCenterY}, BoxSize);
      AddFloatingBox(LabeledBox);
      }
   else
      {
      SdSize2D DefaultBoxSize {100.0, 100.0};
      SRectangle DefaultBox = SRectangle({TargetCenterX, TargetCenterY}, DefaultBoxSize);
      DefaultBox.Resizable = M_ENABLE;
      AddFloatingBox(DefaultBox);
      DisplayFloatingBoxSize();
      }
   }

//==============================================================================
//!
void CLabelingToolView::ShowImageLabelingView()
   {
   m_pModelSelectionBar->Disappear();
   m_pLabelingBar->Appear();
   m_MustShowLabels = true;
   m_pNavigationBar->m_StepTitle.SetText(MIL_TEXT("2) Label images"));
   Update();
   }

//==============================================================================
//!
bool CLabelingToolView::IsExistingLabel(MIL_INT Label) const
   {
   return (GetIndexOf(Label) == M_INVALID) ? false : true;
   }

//==============================================================================
//!
void CLabelingToolView::DisplayFloatingBoxSize()
   {
   auto CurBox = GetFloatingBox();
   SiSize2D ModelSizeInt = CurBox.GetSize();
   MIL_STRING SizeModel = MIL_TEXT("Size : (") + M_TO_STRING(ModelSizeInt.x) + MIL_TEXT(",") + M_TO_STRING(ModelSizeInt.y) + MIL_TEXT(")");
   m_pModelSelectionBar->m_DispSizeButton.SetText(SizeModel);
   }

//==============================================================================
//!
bool CLabelingToolView::IsFloatingBoxInsideImage() const
   {
   auto CurBox = GetFloatingBox();
   SiPoint2D SrcTopLeft = CvtToIntBoxCorner(CurBox.TopLeft);
   SiPoint2D SrcBottomRight = CvtToIntBoxCorner(CurBox.BottomRight);

   MIL_INT ImageSizeX = MbufInquire(m_rLabelingTool.GetCurImageId(), M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(m_rLabelingTool.GetCurImageId(), M_SIZE_Y, M_NULL);

   if(SrcTopLeft.x >= 0 &&
      SrcTopLeft.y >= 0 &&
      SrcBottomRight.x < ImageSizeX &&
      SrcBottomRight.y < ImageSizeY)
      {
      return true;
      }
   else
      {
      return false;
      }
   }

//==============================================================================
//!
void CLabelingToolView::ResetOverlay()
   {
   MIL_ID MilOverlay = MdispInquire(m_Display, M_OVERLAY_ID, M_NULL);
   MbufCopy(m_rLabelingTool.GetCurImageId(), MilOverlay);
   }

//==============================================================================
//!
void CLabelingToolView::UpdateAndRedrawSelectedBox()
   {
   SRectangle PreviousSelectedBox;
   if(m_CurSelectedFloatingBoxLabel != M_INVALID)
      {
      PreviousSelectedBox = GetFloatingBox();
      }
   Update();
   if(m_CurSelectedFloatingBoxLabel != M_INVALID)
      {
      AddFloatingBox(PreviousSelectedBox);
      OverlayModel();
      DisableAllBoxExceptCurrent();
      }
   }

//==============================================================================
//!
void CLabelingToolView::Update()
   {
   MIL_INT CurImageIndex = m_rLabelingTool.GetCurImageIndex();
   if(m_rLabelingTool.IsValidImageIndex(CurImageIndex))
      {
      m_pNavigationBar->m_ImageNameButton.SetText(m_rLabelingTool.GetLabeledImages()[CurImageIndex].m_FileName);
      ResetOverlay();
      m_pNavigationBar->DispCurrentImageIndex(CurImageIndex, m_rLabelingTool.GetNbImage());
      
      ClearLabels();
      if(m_MustShowLabels)
         {
         DrawLabels();
         }

      MdispSelect(m_Display, m_rLabelingTool.GetCurImageId());
      }
   }

//==============================================================================
//!
void CLabelingToolView::RegisterOverlayOnBoxMoveEvent(CLabelingToolController* pController)
   {
   MgraHookFunction(m_GraList, M_GRAPHIC_MODIFIED, OverlayOnBoxMove, pController);
   }

//==============================================================================
//!
void CLabelingToolView::RegisterOnGraListEvent(CLabelingToolController* pController)
   {
   MgraHookFunction(m_GraList, M_GRAPHIC_SELECTION_MODIFIED, OnGraListClick, pController);
   }

//==============================================================================
//!
void CLabelingToolView::RegisterDisplaySizeOnBoxMovetEvent(CLabelingToolController* pController)
   {
   MgraHookFunction(m_GraList, M_GRAPHIC_MODIFIED, DisplaySizeOnBoxMove, pController);
   }

//==============================================================================
//!
void CLabelingToolView::UnRegisterDisplaySizeOnBoxMovetEvent(CLabelingToolController* pController)
   {
   MgraHookFunction(m_GraList, M_GRAPHIC_MODIFIED + M_UNHOOK, DisplaySizeOnBoxMove, pController);
   }

//==============================================================================
//! Add floating box in the center of the current image.
void CLabelingToolView::AddFloatingBox(MIL_INT Color)
   {
   MIL_INT CurImageIdx = m_rLabelingTool.GetCurImageIndex();
   if(!m_rLabelingTool.IsValidImageIndex(CurImageIdx))
      {
      return;
      }
   const auto& CurImage = m_rLabelingTool.GetLabeledImages()[CurImageIdx];
   MIL_DOUBLE TargetCenterX = (MIL_DOUBLE)MbufInquire(CurImage.m_Id, M_SIZE_X, M_NULL) / 2.0;
   MIL_DOUBLE TargetCenterY = (MIL_DOUBLE)MbufInquire(CurImage.m_Id, M_SIZE_Y, M_NULL) / 2.0;
   TargetCenterX += (MIL_DOUBLE)CurImage.m_Labels.size(); // to shift a bit for better visualization
   SiSize2D ModelSize = m_rLabelingTool.GetModelSize();
   SRectangle Box(SdPoint2D{TargetCenterX, TargetCenterY}, ModelSize);
   Box.Color = Color;
   AddFloatingBox(Box);
   }

//==============================================================================
//!
void CLabelingToolView::AddFloatingBox(const SRectangle& Box)
   {
   MdispControl(m_Display, M_UPDATE_GRAPHIC_LIST, M_DISABLE);
   MgraRect(m_GraCtx, m_GraList, Box.TopLeft.x, Box.TopLeft.y, Box.BottomRight.x, Box.BottomRight.y);
   MIL_INT LastLabel {M_INVALID};
   MgraInquireList(m_GraList, M_LIST, M_DEFAULT, M_LAST_LABEL, &LastLabel);
   m_CurSelectedFloatingBoxLabel = LastLabel;
   MgraControlList(m_GraList, M_GRAPHIC_LABEL(LastLabel), M_DEFAULT, M_COLOR, Box.Color);
   MgraControlList(m_GraList, M_GRAPHIC_LABEL(LastLabel), M_DEFAULT, M_RESIZABLE, Box.Resizable);
   MgraControlList(m_GraList, M_GRAPHIC_LABEL(LastLabel), M_DEFAULT, M_TRANSLATABLE, M_ENABLE);
   MgraControlList(m_GraList, M_LIST, M_DEFAULT, M_SELECTED_COLOR, (Box.Color == M_COLOR_BLUE) ? M_COLOR_CYAN : M_COLOR_MAGENTA);
   MgraControlList(m_GraList, M_GRAPHIC_LABEL(LastLabel), M_DEFAULT, M_GRAPHIC_SELECTED, M_TRUE);
   MdispControl(m_Display, M_UPDATE_GRAPHIC_LIST, M_ENABLE);
   }

//==============================================================================
//!
void CLabelingToolView::DeleteFloatingBox()
   {
   if(m_CurSelectedFloatingBoxLabel != M_INVALID)
      {
      MgraControlList(m_GraList, M_GRAPHIC_LABEL(m_CurSelectedFloatingBoxLabel), M_DEFAULT, M_DELETE, M_DEFAULT);
      ResetOverlay();
      }
   m_CurSelectedFloatingBoxLabel = M_INVALID;
   }

//==============================================================================
//!
MIL_INT CLabelingToolView::GetCurSelectedFloatingBoxLabel() const
   {
   return m_CurSelectedFloatingBoxLabel;
   }

//==============================================================================
//!
void CLabelingToolView::UpdateCurSelectedFloatingBoxLabel()
   {
   m_CurSelectedFloatingBoxLabel = M_INVALID;
   MIL_INT NbGraphic = MgraInquireList(m_GraList, M_LIST, M_DEFAULT, M_NUMBER_OF_GRAPHICS, M_NULL);
   for(MIL_INT i = 0; i < NbGraphic; ++i)
      {
      MIL_INT IsSelected = MgraInquireList(m_GraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_GRAPHIC_SELECTED, M_NULL);
      if(IsSelected == M_TRUE)
         {
         m_CurSelectedFloatingBoxLabel = MgraInquireList(m_GraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_LABEL_VALUE, M_NULL);
         }
      }
   }

//==============================================================================
//!
SRectangle CLabelingToolView::GetFloatingBox() const
   {
   if(m_CurSelectedFloatingBoxLabel != M_INVALID)
      {
      return CvtGraRectangle(m_GraList, m_CurSelectedFloatingBoxLabel);
      }
   else
      {
      return SRectangle();
      }
   }

