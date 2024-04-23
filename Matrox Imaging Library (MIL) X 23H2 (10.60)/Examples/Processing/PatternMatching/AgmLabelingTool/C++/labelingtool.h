//*******************************************************************************
// 
// File name: labelingtool.h
//
// Synopsis: This file declares the Model class of the Model-View-Controller
//           pattern.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#pragma once
#include "util.h"

//==============================================================================
//!
class CLabelingTool
   {
   public:
      CLabelingTool(MIL_ID MilSystem, const std::vector<SImage>& Images, MIL_ID ModelImage);

      MIL_ID GetSystemId() const;
      MIL_INT GetNbImage() const;
      MIL_ID GetCurImageId() const;
      const std::vector<SLabeledImage>& GetLabeledImages() const;
      const MIL_INT GetCurImageIndex() const;
      bool IsValidImageIndex(MIL_INT Index) const;
      void SetCurImageIndex(MIL_INT Index);
      MIL_ID GetModelImage() const;
      SiSize2D GetModelSize() const;
      void SetModelImageAt(const SRectangle& Box);
      void AddLabel(const SRectangle& LabeledBox);
      void SaveLabeledImages() const;
      void SaveModelImage() const;
      void DeleteLabel(MIL_INT LabelIndex);
      bool GetFirstLabeledBox(SRectangle* pBox) const;
      void SetSavedModelImagePath(const MIL_STRING& FilePath) { m_ModelImagePath = FilePath; };
      void SetSavedLabeledImagesPath(const MIL_STRING& FilePath) { m_SavedContainerPath = FilePath; };

   private:
      MIL_ID m_System {M_DEFAULT_HOST};
      MIL_UNIQUE_BUF_ID m_ModelImage;
      std::vector<SLabeledImage> m_LabeledImages;
      MIL_INT m_CurImageIndex = M_INVALID;
      MIL_STRING m_ModelImagePath {MIL_TEXT("ModelImage.mim")};
      MIL_STRING m_SavedContainerPath {MIL_TEXT("LabeledImagesContainer.mbfuc")};

   };
