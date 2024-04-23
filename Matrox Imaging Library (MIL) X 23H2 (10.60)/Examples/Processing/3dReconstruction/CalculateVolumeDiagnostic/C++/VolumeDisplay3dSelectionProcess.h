//***************************************************************************************/
//
// File name: VolumeDisplay3dSelectionProcess.h
//
// Synopsis:  Declaration of CVolume3dDisplaySelectionProcessing class that
//            updates the volume diagnosyic 3D display according to the selected pixel
//            of the zoom display.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#ifndef VOLUME_DISPLAY_3D_SELECTION_PROCESS_H
#define VOLUME_DISPLAY_3D_SELECTION_PROCESS_H

static const MIL_INT UNSELECTED_SURFACE_OPACITY = 100;
static const MIL_INT SELECTED_SURFACE_OPACITY = 20;

class CVolume3dDisplaySelectionProcessing: public ISelectionProcessing
   {
   public:
      CVolume3dDisplaySelectionProcessing(MIL_ID MilSystem,
                                          MIL_ID MilVolumeResult,
                                          MIL_ID Mil3dDisplay);
      virtual ~CVolume3dDisplaySelectionProcessing() {};

      void InitSelection(MIL_ID MilIndexImage, MIL_INT SurfaceLabel);
      virtual void ProcessSelection(MIL_INT SelectedValue, MIL_INT SelectedPosX, MIL_INT SelectedPosY);

   private:

      MIL_UNIQUE_3DMET_ID m_Mil3dMetSingleDrawContext;
            
      MIL_ID m_MilVolumeResult;
      MIL_ID m_Mil3dDisplay;
      MIL_ID m_Mil3dGraList;

      MIL_ID m_MilIndexImage;
      MIL_INT m_ZoomLabel = 0;
      MIL_INT m_SurfaceLabel = 0;
   };

#endif // VOLUME_DISPLAY_3D_SELECTION_PROCESS_H
