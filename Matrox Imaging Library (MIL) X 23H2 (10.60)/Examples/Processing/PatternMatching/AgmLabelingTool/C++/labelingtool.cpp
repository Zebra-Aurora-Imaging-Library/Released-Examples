//*******************************************************************************
// 
// File name: labelingtool.cpp
//
// Synopsis: This file implements the Model class of the Model-View-Controller
//           pattern.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#include "labelingtool.h"

//==================================================================================================
//!
static bool AreAllRegionsSameSizeAsModel(const std::vector<SImage>& Images, MIL_ID ModelImage)
   {
   MIL_INT ModelSizeX = MbufInquire(ModelImage, M_SIZE_X, M_NULL);
   MIL_INT ModelSizeY = MbufInquire(ModelImage, M_SIZE_Y, M_NULL);
   for(const auto& Image : Images)
      {
      auto ExtractedGraList = MgraAllocList(M_DEFAULT_HOST, M_DEFAULT, M_UNIQUE_ID);
      MbufSetRegion(Image.m_Id, ExtractedGraList, M_DEFAULT, M_EXTRACT, M_DEFAULT);
      MIL_INT NbGraphic = MgraInquireList(ExtractedGraList, M_LIST, M_DEFAULT, M_NUMBER_OF_GRAPHICS, M_NULL);
      for(MIL_INT i = 0; i < NbGraphic; ++i)
         {
         MIL_INT GraType = MgraInquireList(ExtractedGraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_GRAPHIC_TYPE, M_NULL);
         if(GraType == M_GRAPHIC_TYPE_RECT)
            {
            MIL_INT GraLabel = MgraInquireList(ExtractedGraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_LABEL_VALUE, M_NULL);
            SdSize2D BoxSize;
            MgraInquireList(ExtractedGraList, M_GRAPHIC_LABEL(GraLabel), M_DEFAULT, M_RECTANGLE_WIDTH, &BoxSize.x);
            MgraInquireList(ExtractedGraList, M_GRAPHIC_LABEL(GraLabel), M_DEFAULT, M_RECTANGLE_HEIGHT, &BoxSize.y);

            if((BoxSize.x != ModelSizeX) || (BoxSize.y != ModelSizeY))
               {
               MosPrintf(MIL_TEXT("Existing labeled occurrences must be the same size as the model image.\n"));
               return false;
               }
            }
         }
      }
   return true;
   }

//==================================================================================================
//!
CLabelingTool::CLabelingTool(MIL_ID MilSystem, const std::vector<SImage>& Images, MIL_ID ModelImage)
   : m_System(MilSystem)
   {
   if(ModelImage != M_NULL && !AreAllRegionsSameSizeAsModel(Images, ModelImage))
      {
      return;
      }

   for(const auto& Image : Images)
      {
      SLabeledImage LabeledImage;
      LabeledImage.m_Id = MbufClone(Image.m_Id, m_System, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_UNIQUE_ID);
      LabeledImage.m_FileName = Image.m_FileName;
      auto ExtractedGraList = MgraAllocList(m_System, M_DEFAULT, M_UNIQUE_ID);
      MbufSetRegion(Image.m_Id, ExtractedGraList, M_DEFAULT, M_EXTRACT, M_DEFAULT);
      MIL_INT NbGraphic = MgraInquireList(ExtractedGraList, M_LIST, M_DEFAULT, M_NUMBER_OF_GRAPHICS, M_NULL);
      for(MIL_INT i = 0; i < NbGraphic; ++i)
         {
         MIL_INT GraType = MgraInquireList(ExtractedGraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_GRAPHIC_TYPE, M_NULL);
         if(GraType == M_GRAPHIC_TYPE_RECT)
            {
            MIL_INT GraLabel = MgraInquireList(ExtractedGraList, M_GRAPHIC_INDEX(i), M_DEFAULT, M_LABEL_VALUE, M_NULL);
            SRectangle Box = CvtGraRectangle(ExtractedGraList, GraLabel);
            LabeledImage.m_Labels.push_back(Box);
            }
         }
      m_LabeledImages.push_back(std::move(LabeledImage));
      }
   if(ModelImage != M_NULL)
      {
      m_ModelImage = MbufClone(ModelImage, m_System, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_UNIQUE_ID);
      }
   
   // Start with the first image.
   m_CurImageIndex = Images.empty() ? M_INVALID : 0;
   }

//==================================================================================================
//!
MIL_ID CLabelingTool::GetSystemId() const
   {
   return m_System;
   }

//==================================================================================================
//!
MIL_INT CLabelingTool::GetNbImage() const
   {
   return static_cast<MIL_INT>(m_LabeledImages.size());
   }

//==================================================================================================
//!
MIL_ID CLabelingTool::GetCurImageId() const
   {
   return (m_LabeledImages.empty() || !IsValidImageIndex(m_CurImageIndex)) ? M_NULL : m_LabeledImages[m_CurImageIndex].m_Id;
   }

//==================================================================================================
//!
const std::vector<SLabeledImage>& CLabelingTool::GetLabeledImages() const
   {
   return m_LabeledImages;
   }

//==================================================================================================
//!
const MIL_INT CLabelingTool::GetCurImageIndex() const
   {
   return m_CurImageIndex;
   }

//==================================================================================================
//!
bool CLabelingTool::IsValidImageIndex(MIL_INT Index) const
   {
   return (Index >= 0) && (Index < GetNbImage());
   }

//==================================================================================================
//!
void CLabelingTool::SetCurImageIndex(MIL_INT Index)
   {
   if(IsValidImageIndex(Index))
      {
      m_CurImageIndex = Index;
      }
   }

//==================================================================================================
//!
MIL_ID CLabelingTool::GetModelImage() const
   {
   return m_ModelImage.get();
   }

//==================================================================================================
//!
SiSize2D CLabelingTool::GetModelSize() const
   {
   SiSize2D ModelSize {0, 0};
   if(m_ModelImage != M_NULL)
      {
      ModelSize.x = MbufInquire(m_ModelImage, M_SIZE_X, M_NULL);
      ModelSize.y = MbufInquire(m_ModelImage, M_SIZE_Y, M_NULL);
      }
   return ModelSize;
   }

//==================================================================================================
//!
void CLabelingTool::SetModelImageAt(const SRectangle& Box)
   {
   MIL_ID CurImage = GetCurImageId();
   if(CurImage != M_NULL)
      {
      SiPoint2D SrcTopLeft = CvtToIntBoxCorner(Box.TopLeft);
      SiSize2D ModelSize = Box.GetSize();
      m_ModelImage = MbufAlloc2d(m_System, ModelSize.x, ModelSize.y, 8 + M_UNSIGNED, M_IMAGE + M_PROC, M_UNIQUE_ID);
      MbufCopyColor2d(CurImage, m_ModelImage, M_ALL_BANDS, SrcTopLeft.x, SrcTopLeft.y, M_ALL_BANDS, 0, 0, ModelSize.x, ModelSize.y);
      }
   }

//==============================================================================
//!
static MIL_DOUBLE MoveToClosest05(MIL_DOUBLE Value)
   {
   return std::ceil(Value - 1.0) + 0.5;
   }

//==============================================================================
//!
static SRectangle AlignWithPixelBorder(const SRectangle& Box)
   {
   SRectangle AlignedBox = Box;
   AlignedBox.TopLeft.x = MoveToClosest05(Box.TopLeft.x);
   AlignedBox.TopLeft.y = MoveToClosest05(Box.TopLeft.y);
   AlignedBox.BottomRight.x = AlignedBox.TopLeft.x + Box.GetSize().x;
   AlignedBox.BottomRight.y = AlignedBox.TopLeft.y + Box.GetSize().y;
   return AlignedBox;
   }

//==================================================================================================
//!
void CLabelingTool::AddLabel(const SRectangle& LabeledBox)
   {
   if(IsValidImageIndex(m_CurImageIndex))
      {
      auto AlignedBox = AlignWithPixelBorder(LabeledBox);
      m_LabeledImages[m_CurImageIndex].m_Labels.push_back(AlignedBox);
      }
   }

//==================================================================================================
//!
void CLabelingTool::SaveLabeledImages() const
   {
   auto ContainerId = MbufAllocContainer(m_System, M_PROC | M_DISP, M_DEFAULT, M_UNIQUE_ID);
   for(MIL_INT i = 0; i < GetNbImage(); ++i)
      {
      const auto& LabeledImage = m_LabeledImages[i];
      MbufCopyComponent(LabeledImage.m_Id, ContainerId, M_DEFAULT, M_APPEND, M_DEFAULT);
      MIL_ID CurComponentId;
      MbufInquireContainer(ContainerId, M_COMPONENT_BY_INDEX(i), M_COMPONENT_ID, &CurComponentId);
      auto AllRegions = MgraAllocList(m_System, M_DEFAULT, M_UNIQUE_ID);
      for(MIL_INT BoxIdx = 0; BoxIdx < (MIL_INT)LabeledImage.m_Labels.size(); ++BoxIdx)
         {
         const auto& LabeledBox = LabeledImage.m_Labels[BoxIdx];
         MgraRect(M_DEFAULT, AllRegions, LabeledBox.TopLeft.x, LabeledBox.TopLeft.y, LabeledBox.BottomRight.x, LabeledBox.BottomRight.y);
         MgraControlList(AllRegions, M_GRAPHIC_INDEX(BoxIdx), M_DEFAULT, M_COLOR, LabeledBox.Color);
         }
      MbufSetRegion(CurComponentId, AllRegions, M_DEFAULT, M_NO_RASTERIZE, M_DEFAULT);
      MbufControl(CurComponentId, M_REGION_USE, M_USE);
      }
   MbufSave(m_SavedContainerPath, ContainerId);
   }

//==================================================================================================
//!
void CLabelingTool::SaveModelImage() const
   {
   if(m_ModelImage != M_NULL)
      {
      MbufSave(m_ModelImagePath, m_ModelImage);
      }
   }

//==================================================================================================
//!
void CLabelingTool::DeleteLabel(MIL_INT LabelIndex)
   {
   if(IsValidImageIndex(m_CurImageIndex))
      {
      auto& CurLabels = m_LabeledImages[m_CurImageIndex].m_Labels;
      CurLabels.erase(CurLabels.begin() + LabelIndex);
      }
   }

//==================================================================================================
//!
bool CLabelingTool::GetFirstLabeledBox(SRectangle* pBox) const
   {
   for(const auto& Image : m_LabeledImages)
      {
      if(!Image.m_Labels.empty())
         {
         *pBox = Image.m_Labels.front();
         return true;
         }
      }
   return false;
   }
