//*******************************************************************************
// 
// File name: MILDisplayManager.cpp
//
// Synopsis:  Class in charge of managing the MIL 2D displays for 3D analysis 
//            examples.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "BaseCommon.h"

//*******************************************************************************
// CMILDisplayManager.
//*******************************************************************************
CMILDisplayManager::CMILDisplayManager():
   m_MilDisplay(0), m_MilDisplayBuffer(0)
   {
   }

//*******************************************************************************
// Allocates the display.
//*******************************************************************************
void CMILDisplayManager::Alloc(MIL_ID MilSystemId, MIL_INT DispNum, 
                               MIL_CONST_TEXT_PTR DispFormat, MIL_INT64 InitFlag)
   {
   MdispAlloc(MilSystemId, DispNum, DispFormat, InitFlag, &m_MilDisplay);
   }

//*******************************************************************************
// Frees the display.
//*******************************************************************************
void CMILDisplayManager::Free()
   {
   if (m_MilDisplay)
      {
      MdispFree(m_MilDisplay);
      m_MilDisplay = M_NULL;
      }
   }

//*******************************************************************************
// Shows the provided image.
//*******************************************************************************
void CMILDisplayManager::Show(MIL_ID MilDisplayBuffer)
   {
   SetDisplayBufferID(MilDisplayBuffer);
   Show();
   }

//*******************************************************************************
// Shows the display.
//*******************************************************************************
void CMILDisplayManager::Show()
   {
   MdispSelect(m_MilDisplay, m_MilDisplayBuffer);
   m_Showing = true;
   }

//*******************************************************************************
// Hides the display.
//*******************************************************************************
void CMILDisplayManager::Hide()
   {
   if (m_Showing)
      {
      MdispSelect(m_MilDisplay, M_NULL);
      m_Showing = false;
      }
   }

//*******************************************************************************
// Zooms the display.
//*******************************************************************************
void CMILDisplayManager::Zoom(MIL_DOUBLE x, MIL_DOUBLE y)
   {
   MdispZoom(m_MilDisplay, x, y);
   }
