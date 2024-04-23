//***************************************************************************************/
//
// File name: VolumeDisplay3dSelectionProcess.cpp
//
// Synopsis:  Implementation of CVolume3dDisplaySelectionProcessing class that
//            updates the volume diagnostic 3D display according to the selected pixel
//            of the zoom display.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include "ZoomDisplay.h"
#include "VolumeDisplay3dSelectionProcess.h"

//****************************************************************************
// Constructor.
//****************************************************************************
CVolume3dDisplaySelectionProcessing::CVolume3dDisplaySelectionProcessing(MIL_ID MilSystem,
                                                                         MIL_ID MilVolumeResult,
                                                                         MIL_ID Mil3dDisplay)
   : m_Mil3dDisplay(Mil3dDisplay),
   m_MilVolumeResult(MilVolumeResult)
   {
   M3ddispInquire(Mil3dDisplay, M_3D_GRAPHIC_LIST_ID, &m_Mil3dGraList);
   m_Mil3dMetSingleDrawContext = M3dmetAlloc(MilSystem, M_DRAW_3D_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   }

//****************************************************************************
// Initialize the selection processing by selecting the index image and the label
// of the volume surface annotation.
//****************************************************************************
void CVolume3dDisplaySelectionProcessing::InitSelection(MIL_ID MilIndexImage,
                                                        MIL_INT SurfaceLabel)
   {
   m_ZoomLabel = 0;
   m_SurfaceLabel = SurfaceLabel;
   m_MilIndexImage = MilIndexImage;
   }

//****************************************************************************
// Processes the selection by drawing the zoomed volume element in the
// 3D display. The volume surface annotation opacity is also reduced for better
// visibility of the selected element.
//****************************************************************************
void CVolume3dDisplaySelectionProcessing::ProcessSelection(MIL_INT SelectedValue,
                                                           MIL_INT SelectedPosX,
                                                           MIL_INT SelectedPosY)
   {
   M3ddispControl(m_Mil3dDisplay, M_UPDATE, M_DISABLE);

   if(m_ZoomLabel)
      {
      M3dgraRemove(m_Mil3dGraList, m_ZoomLabel, M_DEFAULT);
      m_ZoomLabel = 0;
      }

   bool HasChangedSurfaceOpacity = false;
   if(SelectedValue != 255)
      {
      MIL_UINT32 ElementIndex;
      MIL_INT ZoomWindowCenterX = SelectedPosX;
      MIL_INT ZoomWindowCenterY = SelectedPosY;
      MbufGet2d(m_MilIndexImage, ZoomWindowCenterX, ZoomWindowCenterY, 1, 1, &ElementIndex);
      if(ElementIndex != MIL_UINT32_MAX)
         {
         M3dmetControlDraw(m_Mil3dMetSingleDrawContext, M_CONTEXT, M_VOLUME_ELEMENT_INDEX, ElementIndex);
         M3dmetControlDraw(m_Mil3dMetSingleDrawContext, M_DRAW_VOLUME_ELEMENTS, M_COLOR, M_COLOR_WHITE);
         m_ZoomLabel = M3dmetDraw3d(m_Mil3dMetSingleDrawContext, m_MilVolumeResult,
                                    m_Mil3dGraList, M_DEFAULT, M_DEFAULT);
         if(m_SurfaceLabel)
            {
            M3dgraControl(m_Mil3dGraList, m_SurfaceLabel, M_OPACITY + M_RECURSIVE, SELECTED_SURFACE_OPACITY);
            HasChangedSurfaceOpacity = true;
            }
         }
      }

   if(!HasChangedSurfaceOpacity && m_SurfaceLabel)
      M3dgraControl(m_Mil3dGraList, m_SurfaceLabel, M_OPACITY + M_RECURSIVE, UNSELECTED_SURFACE_OPACITY);

   M3ddispControl(m_Mil3dDisplay, M_UPDATE, M_ENABLE);
   }

