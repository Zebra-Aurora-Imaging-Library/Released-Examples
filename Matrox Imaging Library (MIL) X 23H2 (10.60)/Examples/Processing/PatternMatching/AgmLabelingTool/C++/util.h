//***************************************************************************************
//
// File name: util.h
//
// Synopsis: Defines utility stuctures and functions for the labeling tool.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************

#pragma once

#include <mil.h>

template<typename T>
struct SPoint2D
   {
   T x {0};
   T y {0};

   operator SPoint2D<MIL_DOUBLE>() const
      {
      return {static_cast<MIL_DOUBLE>(x), static_cast<MIL_DOUBLE>(y)};
      }
   operator SPoint2D<MIL_INT>() const
      {
      return {static_cast<MIL_INT>(x), static_cast<MIL_INT>(y)};
      }
   };

using SdPoint2D = SPoint2D<MIL_DOUBLE>;
using SdSize2D  = SdPoint2D;
using SiPoint2D = SPoint2D<MIL_INT>;
using SiSize2D  = SiPoint2D;

inline MIL_INT HalfRoundUp(MIL_DOUBLE Value)
   {
   return static_cast<MIL_INT>(std::trunc(std::floor(Value + 0.5)));
   }

inline SiPoint2D CvtToIntBoxCorner(const SdPoint2D& FloatCorner)
   {
   return {HalfRoundUp(FloatCorner.x), HalfRoundUp(FloatCorner.y)};
   }

struct SRectangle
   {
   SRectangle() = default;
   SRectangle(const SdPoint2D& Center, const SdSize2D& Size)
      {
      TopLeft.x = Center.x - (Size.x / 2.0);
      TopLeft.y = Center.y - (Size.y / 2.0);
      BottomRight.x = Center.x + (Size.x / 2.0);
      BottomRight.y = Center.y + (Size.y / 2.0);
      }

   SdPoint2D TopLeft {0.0, 0.0};
   SdPoint2D BottomRight {0.0, 0.0};
   MIL_INT Color {M_COLOR_BLUE};
   MIL_INT Resizable = M_DISABLE;

   SdPoint2D GetCenter() const { return SdPoint2D {TopLeft.x + (BottomRight.x - TopLeft.x) / 2.0, TopLeft.y + (BottomRight.y - TopLeft.y) / 2.0}; }
   SdSize2D GetSize() const { return SdSize2D {BottomRight.x - TopLeft.x, BottomRight.y - TopLeft.y}; }
   };

struct SImage
   {
   SImage(MIL_ID MilSystem, const MIL_STRING& ImageFolder, const MIL_STRING& FileName)
      :m_FileName(FileName)
      {
      m_Id = MbufRestore(ImageFolder + FileName, MilSystem, M_UNIQUE_ID);
      // Ensure there is no region of interest in the restored image.
      MbufSetRegion(m_Id, M_NULL, M_DEFAULT, M_DELETE, M_DEFAULT);
      }

   SImage(MIL_ID MilSystem, MIL_ID ImageToClone, const MIL_STRING& ImageName)
      :m_FileName(ImageName)
      {
      m_Id = MbufClone(ImageToClone, MilSystem, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_UNIQUE_ID);
      MbufSetRegion(ImageToClone, m_Id, M_DEFAULT, M_COPY, M_DEFAULT);
      }

   MIL_STRING m_FileName;
   MIL_UNIQUE_BUF_ID m_Id;
   };

struct SLabeledImage
   {
   MIL_UNIQUE_BUF_ID m_Id;
   std::vector<SRectangle> m_Labels;
   MIL_STRING m_FileName;
   };

std::vector<MIL_STRING> ListImagesInFolder(const MIL_STRING& FileToSearch);
SRectangle CvtGraRectangle(MIL_ID GraList, MIL_INT RectGraLabel);
