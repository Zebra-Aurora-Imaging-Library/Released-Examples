//***************************************************************************************/
//
// File name: DisplayLinker.h
//
// Synopsis:  This file contains a class that links 3D displays together so that 
//            moving the view in one display also moves the view in the other displays.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#pragma once

#include <mil.h>
#include <atomic>

//-----------------------------------------------------------------------------
// Class that links 3D displays together and unlinks them on destruction.
//-----------------------------------------------------------------------------
class CDisplayLinker
   {
   public:
      CDisplayLinker(std::vector<MIL_ID> Displays);

      void StopLink();

      ~CDisplayLinker() { StopLink(); }

   private:
      static const MIL_INT POLLING_DELAY = 50;  // The polling delay, in msec.

      bool                 m_Exit = false;      // Tells the polling thread when to exit.
      std::vector<MIL_ID>  m_Displays;          // List of linked displays.
      MIL_UNIQUE_THR_ID    m_PollingThread;     // Thread that continuously polls and updates the displays.

      void DoPolling() const;
      
   };

//-----------------------------------------------------------------------------
// Links the provided displays together.
// If several displays are updated at once, the one that comes first in the vector has priority.
//-----------------------------------------------------------------------------
CDisplayLinker::CDisplayLinker(std::vector<MIL_ID> Displays):
   m_Displays(std::move(Displays))
   {
   if(m_Displays.size() > 1)
      {
      m_Exit = false;
      MIL_ID System = MobjInquire(m_Displays[0], M_OWNER_SYSTEM, M_NULL);
      m_PollingThread = MthrAlloc(System, M_THREAD, M_DEFAULT, [](void* UserDataPtr) -> MIL_UINT32
         {
         static_cast<CDisplayLinker*>(UserDataPtr)->DoPolling();
         return 0;
         }, this, M_UNIQUE_ID);
      }
   }

//-----------------------------------------------------------------------------
// Unlinks the displays.
//-----------------------------------------------------------------------------
void CDisplayLinker::StopLink()
   {
   if(m_PollingThread)
      {
      m_Exit = true;
      MthrWait(m_PollingThread, M_THREAD_END_WAIT, M_NULL);
      m_PollingThread = M_NULL;
      }
   }

//-----------------------------------------------------------------------------
// Continuously polls and updates the displays.
//-----------------------------------------------------------------------------
void CDisplayLinker::DoPolling() const
   {
   // Allocate matrices representing the displays' views.
   MIL_ID System = MobjInquire(m_Displays[0], M_OWNER_SYSTEM, M_NULL);
   auto InvPrevMat = M3dgeoAlloc(System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);   // The inverse of the previous view.
   auto CurrentMat = M3dgeoAlloc(System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);   // The current view.
   auto TempMat = M3dgeoAlloc(System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);      // Temporary matrix used in calculations.

   // Polling loop.
   while(!m_Exit)
      {
      // Check if any display view has changed.
      MIL_ID ChangedDisplay = M_NULL;
      for(const auto& Display : m_Displays)
         {
         // Get the current view.
         M3ddispCopy(Display, CurrentMat, M_VIEW_MATRIX, M_DEFAULT);

         // Multiply with the inverse of the previous view. If this does not give the identity matrix, the view has changed.
         M3dgeoMatrixSetTransform(TempMat, M_COMPOSE_TWO_MATRICES, InvPrevMat, CurrentMat, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         if(!M3dgeoInquire(TempMat, M_IDENTITY, M_NULL))
            { // The view has changed. Save the new inverse view.
            ChangedDisplay = Display;
            M3dgeoMatrixSetTransform(InvPrevMat, M_INVERSE, CurrentMat, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
            break;
            }
         }

      // If one of the displays has changed, copy the new view to all other displays.
      if(ChangedDisplay != M_NULL)
         {
         for(const auto& Display : m_Displays)
            {
            if(Display != ChangedDisplay)
               M3ddispCopy(CurrentMat, Display, M_VIEW_MATRIX, M_DEFAULT);
            }
         }

      MosSleep(POLLING_DELAY);
      }
   }
