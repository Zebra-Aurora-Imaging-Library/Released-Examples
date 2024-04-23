//***************************************************************************************
//
// File name: Dispatcher.h
//
// Synopsis:  Class that defines the dispatcher of the processing for the MP example
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef DISPATCHER_H
#define DISPATCHER_H

typedef long (*PROC_FUNCTION_PTR)(void* UserData);

//*****************************************************************************
// Class used to define the dispatcher of the MP processing example.
// It manages the processing thread and its associated events.
//*****************************************************************************
class CDispatcher
   {
   public:
      CDispatcher(MIL_ID MilSystem, PROC_FUNCTION_PTR ProcessingFunctionPtr, void* DataPtr);
      ~CDispatcher();

      //Dispatcher thread control operations
      void StartThread();
      void StopThread();
      void Run();
      void Pause();

      MIL_ID GetThreadId() { return m_MilDispatchThread;}

      //Dispatcher inquire information functions
      inline bool ThreadStarted() const;
      inline bool IsRunning() const;

      inline MIL_DOUBLE GetFrameRate() const;

   private:
      //Disallow copy
      CDispatcher(const CDispatcher&);
      CDispatcher& operator=(const CDispatcher&);

      MIL_ID              m_MilSystem;
      MIL_ID              m_MilEvents[NUM_EVENTS];
      MIL_ID              m_MilDispatchThread;
      volatile MIL_DOUBLE m_FrameRate;
      bool                m_ThreadStarted;
      volatile bool       m_DispatchRunning;
      MIL_ID              m_MilDispatchStoppedEvent;

      //Variable that holds a pointer to the processing function to call in the dispatcher thread
      PROC_FUNCTION_PTR m_ProcessingFunctionPtr;
      void*             m_DataPtr;

      //Functions called from the dispatcher thread
      static MIL_UINT32 MFTYPE DispatchFunction(void *UserDataPtr);
      void RunDispatcher();
   };

//*******************************************************************************
// ThreadStarted.  Returns whether the dispatch thread has been started.
//*******************************************************************************
inline bool CDispatcher::ThreadStarted() const 
   { 
   return m_ThreadStarted; 
   }

//*******************************************************************************
// IsRunning.  Returns whether the dispatching is running.
//*******************************************************************************
inline bool CDispatcher::IsRunning() const 
   { 
   return (m_ThreadStarted&&m_DispatchRunning); 
   }

//*******************************************************************************
// GetFrameRate.  Returns the frame rate of the dispatcher
//*******************************************************************************
inline MIL_DOUBLE CDispatcher::GetFrameRate() const
   {
   return m_FrameRate;
   }

#endif
