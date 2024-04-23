//*******************************************************************************
//
// File name: CD3DDisplayManager.h
//
// Synopsis:  Class that manages the MIL displays for 3d examples
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef MILDISPLAYMANAGER_H
#define MILDISPLAYMANAGER_H

//*****************************************************************************
// Class that manages the MIL display functionalities of 3d examples
//*****************************************************************************
class CMILDisplayManager : public CDisplayManager
   {
   public:
      CMILDisplayManager();
      virtual ~CMILDisplayManager() {Free();}

      void Alloc(MIL_ID MilSystemId, MIL_INT DispNum,
                 MIL_CONST_TEXT_PTR DispFormat, MIL_INT64 InitFlag);
      void Free();
      
      template<class T>
      void Control(MIL_INT64 ControlType, T ControlValue);


      MIL_ID GetDisplayID()
         { return m_MilDisplay; }

      void SetDisplayBufferID(MIL_ID MIlDisplayBuffer)
         { m_MilDisplayBuffer = MIlDisplayBuffer;}

      void Show(MIL_ID MilDisplayBuffer);

      virtual void  Show();
      virtual void  Hide();

      void          Zoom(MIL_DOUBLE x, MIL_DOUBLE y);

   protected:
      MIL_ID   m_MilDisplay;
      MIL_ID   m_MilDisplayBuffer;

   private:
      // Disallow copy
      CMILDisplayManager(const CMILDisplayManager&);
      CMILDisplayManager& operator=(const CMILDisplayManager&);
   };
template<class T>
void CMILDisplayManager::Control(MIL_INT64 ControlType, T ControlValue)
   {
   MdispControl(m_MilDisplay, ControlType, ControlValue);
   }
#endif
