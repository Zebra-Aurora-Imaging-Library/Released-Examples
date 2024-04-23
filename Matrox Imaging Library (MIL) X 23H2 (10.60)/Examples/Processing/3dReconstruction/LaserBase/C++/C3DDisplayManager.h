//*******************************************************************************
//
// File name: CD3DDisplayManager.h
//
// Synopsis:  Interface class for managing the 3D displays for 3d examples
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef C3DDISPLAYMANAGER_H
#define C3DDISPLAYMANAGER_H

#include <vector>

//*****************************************************************************
// Interface class for managing the 3D displays for 3d examples
//*****************************************************************************
class C3DDisplayManager : public CDisplayManager 
   {
   public:
      C3DDisplayManager();
      virtual ~C3DDisplayManager() { Free(); }

      bool Alloc(MIL_ID MilSystem,
                 MIL_ID* pCameraLaserCtxs,
                 MIL_INT NumCamLaserPairs,
                 const SMapGeneration* MapVisualizationData);
      void Free();

      virtual void  Show(MIL_ID ContainerId, MIL_DOUBLE MinZ, MIL_DOUBLE MaxZ);
      virtual void  Hide();

      void LockCameraData(MIL_INT CamIdx) {};
      void UnlockCameraData(MIL_INT CamIdx) {};

      void LockAll() {}
      void UnlockAll() {};
      
      void Lock();
      void Unlock();
      void Disable();
      void Enable();
      void Show();

      void Control(MIL_INT ControlType, MIL_DOUBLE ControlValue)
         {
         M3ddispControl(m_DispHandle, ControlType, ControlValue);
         }
      virtual MIL_INT GetDisplaySizeX() { return (MIL_INT)(M3ddispInquire(m_DispHandle, M_SIZE_X, M_NULL)); }
      virtual MIL_INT GetDisplaySizeY() { return -1; }
   protected:

      MIL_ID               m_DispHandle;
      MIL_INT              m_NumCamLaserPairs;
      MIL_ID               m_MilSystem;
      MIL_ID               m_Mutex;
      MIL_ID*              m_pCameraLaserCtxs;

   private:
      // Disallow copy
      C3DDisplayManager(const C3DDisplayManager&);
      C3DDisplayManager& operator=(const C3DDisplayManager&);

      volatile bool m_LimitationWarningVerified;

   };

#endif
