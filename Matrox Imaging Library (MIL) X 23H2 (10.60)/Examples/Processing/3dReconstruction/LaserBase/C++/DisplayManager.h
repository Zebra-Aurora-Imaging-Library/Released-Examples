//***************************************************************************
//
// File name: CDisplayManager.h
//
// Synopsis:  Class that manages the display for examples.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

//*****************************************************************************
// Class that manages the display functionalities of 3d examples
//*****************************************************************************
class CDisplayManager
   {
   public:
      CDisplayManager()                
         { 
         m_DisplayUpdateEnabled = true;
         m_RefreshPeriod = 1; //Refresh at every frame by default
         m_SynchronizeDisplays = false;
         m_Showing = false;
         }
      virtual ~CDisplayManager(){}

      virtual void  Show() = 0;
      virtual void  Hide() = 0;
      virtual MIL_INT GetDisplaySizeX() { return -1; }
      virtual MIL_INT GetDisplaySizeY() { return -1; }

      bool UpdateEnabled()                            
         { return m_DisplayUpdateEnabled; }

      void UpdateEnabled(bool Val)
         { m_DisplayUpdateEnabled = Val; }

   protected:

      bool     m_DisplayUpdateEnabled;
      MIL_INT  m_RefreshPeriod;  
      bool     m_SynchronizeDisplays;
      bool     m_Showing;

   private:

      // Disallow copy
      CDisplayManager(const CDisplayManager&);
      CDisplayManager& operator=(const CDisplayManager&);
   };

#endif
