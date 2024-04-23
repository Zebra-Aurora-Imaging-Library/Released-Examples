//*******************************************************************************
// 
// File name: C3DDisplayManager.cpp
//
// Synopsis:  Class in charge of managing the D3D Sys displays for 3D analysis 
//            examples.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#include "BaseCommon.h"

//*******************************************************************************
// Initializes the object.
//*******************************************************************************
C3DDisplayManager::C3DDisplayManager() :
   m_NumCamLaserPairs(0),
   m_MilSystem(0),
   m_Mutex(M_NULL),
   m_pCameraLaserCtxs(NULL),
   m_DispHandle(M_NULL),
   m_LimitationWarningVerified(false)
   {
   }


//*******************************************************************************
// Allocates the 3D display objects and starts the update thread.
//*******************************************************************************
bool C3DDisplayManager::Alloc(MIL_ID MilSystem,
                              MIL_ID* pCameraLaserCtxs,
                              MIL_INT NumCamLaserPairs,
                              const SMapGeneration* MapVisualizationData)
   {
   bool Success = false;

   Free();

   m_NumCamLaserPairs = NumCamLaserPairs;
   m_MilSystem = MilSystem;

   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   m_DispHandle = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   Success = (m_DispHandle != M_NULL);

   if(Success)
      {
      M3ddispControl(m_DispHandle, M_SIZE_X, M3D_DISPLAY_SIZE_X);
      M3ddispControl(m_DispHandle, M_SIZE_Y, M3D_DISPLAY_SIZE_Y);
      M3ddispSetView(m_DispHandle, M_AZIM_ELEV_ROLL, 120.0, 210.0, 0.0, M_DEFAULT);

      MIL_ID MilGraphicList = M_NULL;
      M3ddispInquire(m_DispHandle, M_3D_GRAPHIC_LIST_ID, &MilGraphicList);
      M3dgraControl(MilGraphicList, M_DEFAULT_SETTINGS, M_FONT_SIZE, 12);

      for(MIL_INT i = 0; i < NumCamLaserPairs; ++i)
         {
         M3dmapDraw3d(M_DEFAULT, pCameraLaserCtxs[i], M_DEFAULT, MilGraphicList, M_DEFAULT, M_NULL, M_DEFAULT);
         }

      MIL_INT64 MilGrid = M3dgraGrid(MilGraphicList, M_ROOT_NODE, M_SIZE_AND_SPACING, M_DEFAULT, 750.0, 750.0, 25, 25, M_DEFAULT);
      M3dgraControl(MilGraphicList, MilGrid, M_OPACITY, 10);

      m_LimitationWarningVerified = false;

      MthrAlloc(MilSystem, M_MUTEX, M_DEFAULT, M_NULL, M_NULL, &m_Mutex);
      }
   
   return Success;
   }

//*******************************************************************************
// Frees the display.
//*******************************************************************************
void C3DDisplayManager::Free()
   {
   Hide();
   if (m_DispHandle)
      {
      // End the update thread
      MthrFree(m_Mutex);

      m_pCameraLaserCtxs = NULL;

      M3ddispFree(m_DispHandle);
      m_DispHandle = M_NULL;

      m_LimitationWarningVerified = false;
      }

   }

//*******************************************************************************
// Shows the display.
//*******************************************************************************
void C3DDisplayManager::Show(MIL_ID ContainerId, MIL_DOUBLE MinZ, MIL_DOUBLE MaxZ)
   {
   if(m_DispHandle != M_NULL && !m_Showing)
      {
      MIL_ID GraphicList = M_NULL;
      M3ddispInquire(m_DispHandle, M_3D_GRAPHIC_LIST_ID, &GraphicList);
      M3ddispControl(m_DispHandle, M_UPDATE, M_DISABLE);
      MIL_INT64 ContainerGraphics = M3ddispSelect(m_DispHandle, ContainerId, M_SELECT, M_DEFAULT);
      M3ddispSetView(m_DispHandle, M_ZOOM, 2, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      M3dgraCopy(M_COLORMAP_TURBO + M_FLIP, M_DEFAULT, GraphicList, ContainerGraphics, M_COLOR_LUT, M_DEFAULT);

      M3dgraControl(GraphicList, ContainerGraphics, M_COLOR_USE_LUT, M_TRUE);
      M3dgraControl(GraphicList, ContainerGraphics, M_COLOR_LIMITS, M_USER_DEFINED);
      M3dgraControl(GraphicList, ContainerGraphics, M_COLOR_LIMITS_MIN, MinZ);
      M3dgraControl(GraphicList, ContainerGraphics, M_COLOR_LIMITS_MAX, MaxZ);
      M3dgraControl(GraphicList, ContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
      M3dgraControl(GraphicList, ContainerGraphics, M_COLOR_COMPONENT_BAND, 2);
      M3ddispControl(m_DispHandle, M_UPDATE, M_ENABLE);

      m_Showing = true;
      }
   }

void C3DDisplayManager::Show()
   {
   if(m_DispHandle != M_NULL && !m_Showing)
      {
      M3ddispSelect(m_DispHandle, M_NULL, M_OPEN, M_DEFAULT);
      m_Showing = true;
      }
   }
//*******************************************************************************
// Hides the display.
//*******************************************************************************
void C3DDisplayManager::Hide()
   {
   if (m_Showing)
      {
      M3ddispSelect(m_DispHandle, M_NULL, M_CLOSE, M_DEFAULT);
      m_Showing = false;
      }
   }

//*******************************************************************************
// Lock the mutex.
//*******************************************************************************
void C3DDisplayManager::Lock()
   {
   MthrControl(m_Mutex, M_LOCK, M_DEFAULT);
   }

//*******************************************************************************
// Unlock the mutex.
//*******************************************************************************
void C3DDisplayManager::Unlock()
   {   
   MthrControl(m_Mutex, M_UNLOCK, M_DEFAULT);
    }
void C3DDisplayManager::Disable()
   {
   M3ddispControl(m_DispHandle, M_UPDATE, M_DISABLE);
   }
void C3DDisplayManager::Enable()
   {
   M3ddispControl(m_DispHandle, M_UPDATE, M_ENABLE);
   }




