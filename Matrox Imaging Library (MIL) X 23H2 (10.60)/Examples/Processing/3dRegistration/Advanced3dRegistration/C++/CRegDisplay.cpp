//*******************************************************************************
// 
// File name: CRegDisplay.cpp
//
// Synopsis:  Class in charge of managing the 2D/3D displays for 3D  
//            examples.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#include "CRegDisplay.h"

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_RIGHT 77
#define KEY_LEFT 75
#define KEY_SKIP 224


//*****************************************************************************
// Constructor.  
//*****************************************************************************
CRegDisplay::CRegDisplay(MIL_ID MilRefContainer, MIL_ID MilTargetContainer, MIL_ID MilContext,
                         const CWindowParameters& WindowParams,
                         const CCameraParameters& CameraParameters)
   {
   /* Allocate context, result, and display objects */
   m_MilDrawContext = M3dregAlloc(M_DEFAULT_HOST, M_DRAW_3D_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   m_MilRegResult = M3dregAllocResult(M_DEFAULT_HOST, M_PAIRWISE_REGISTRATION_RESULT, M_DEFAULT, M_UNIQUE_ID);
   m_MilDisplay = M3ddispAlloc(M_DEFAULT_HOST, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   /* Set up display. */
   WindowParams.ApplyToDisplay(m_MilDisplay);

   /* Configure draw settings. */
   M3dregControlDraw(m_MilDrawContext, M_DRAW_OVERLAP_POINTS, M_ACTIVE, M_ENABLE);
   M3dregControlDraw(m_MilDrawContext, M_DRAW_OVERLAP_POINTS, M_THICKNESS, 5);
   M3dregControlDraw(m_MilDrawContext, M_DRAW_PAIRS, M_ACTIVE, M_ENABLE);

   /* Execute registration while saving information about point pairs. */
   std::vector<MIL_ID> Containers = {MilRefContainer, MilTargetContainer};
   M3dregControl(MilContext, M_CONTEXT, M_SAVE_PAIRS_INFO, M_TRUE);
   M3dregCalculate(MilContext, Containers, M_DEFAULT, m_MilRegResult, M_DEFAULT);
   M3dregControl(MilContext, M_CONTEXT, M_SAVE_PAIRS_INFO, M_FALSE);

   m_NumIterations = (MIL_INT)M3dregGetResult(m_MilRegResult, 1, M_NB_ITERATIONS, M_NULL);

   /* Open window and start display thread. */
   m_Running.store( true, std::memory_order_relaxed);
   M3ddispSelect(m_MilDisplay, M_NULL, M_OPEN, M_DEFAULT);

   /* Adjust the viewpoint of the 3D display. */
   CameraParameters.ApplyToDisplay(m_MilDisplay);

   m_MilDisplayThread = MthrAlloc(M_DEFAULT_HOST, M_THREAD, M_DEFAULT,
                                  CRegDisplay::ProcessDisplayThread, this, M_UNIQUE_ID);
   }

//*****************************************************************************
// Destructor.   
//*****************************************************************************
CRegDisplay::~CRegDisplay()
   {
   End();
   }

//*****************************************************************************
// Close the display. 
//*****************************************************************************
void CRegDisplay::End()
   {   
   if(m_Running.load(std::memory_order_relaxed))
      {
      /* Signal threads to end and wait. */
      m_Running.store(false, std::memory_order_relaxed);
      if(m_MilDisplayThread != M_NULL)
         {
         MthrWait(m_MilDisplayThread, M_THREAD_END_WAIT, M_NULL);
         }

      /* Close display. */
      M3ddispSelect(m_MilDisplay, M_NULL, M_CLOSE, M_DEFAULT);
      }
   }

//*****************************************************************************
// Update display.      
//*****************************************************************************
MIL_UINT32 MFTYPE CRegDisplay::ProcessDisplayThread(void* pUserData)
   {
   /* Retrieve data. */
   CRegDisplay& Data = *(CRegDisplay*)pUserData;
   auto GraList = (MIL_ID)M3ddispInquire(Data.m_MilDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   MIL_INT NbIteration = (MIL_INT)M3dregGetResult(Data.m_MilRegResult, 1, M_NB_ITERATIONS, M_NULL);

   if(Data.m_LoopIteration >= NbIteration)
      {
      Data.m_LoopIteration = 0;
      }

   MIL_INT LastDrawnIteration = -1;

   MIL_INT64 DrawNode = M_NULL;
   while(true)
      {
      MappTimer(M_TIMER_RESET, M_NULL);

      MIL_INT TargetIteration = (Data.m_Mode == VisualizationMode::RUN) ? Data.m_LoopIteration : Data.m_TargetIteration;

      if(TargetIteration != LastDrawnIteration)
         {
         M3ddispControl(Data.m_MilDisplay, M_UPDATE, M_DISABLE);

         /* Clear display. */
         if(DrawNode)
            M3dgraRemove(GraList, M_ALL, M_DEFAULT);

         /* Draw registration result for a given iteration. */
         DrawNode = M3dregDraw3d(Data.m_MilDrawContext, Data.m_MilRegResult, 1, TargetIteration,
                                 0, GraList, M_DEFAULT, M_DEFAULT);

         M3ddispControl(Data.m_MilDisplay, M_UPDATE, M_ENABLE);

         LastDrawnIteration = TargetIteration;
         }

      MIL_DOUBLE DrawTime = MappTimer(M_TIMER_READ, M_NULL) * 1000;

      if(!Data.m_Running.load(std::memory_order_relaxed))
         return 0;

      /* Delay time between 2 drawings. */
      static const MIL_INT ITERATION_SHOW_TIME = 100; // in msec
      static const MIL_INT LAST_ITERATION_SHOW_TIME = 2000; // in msec
      MIL_INT ShowTime = 0;
      if(Data.m_Mode == VisualizationMode::RUN)
         {
         if(Data.m_LoopIteration == NbIteration - 1)
            {
            ShowTime = LAST_ITERATION_SHOW_TIME;
            }
         else
            {
            ShowTime = ITERATION_SHOW_TIME;
            }
         }

      if(DrawTime < ShowTime)
         {
         MosSleep(ShowTime - (MIL_INT)DrawTime);
         }
      if(Data.m_Mode == VisualizationMode::RUN)
         {
         Data.m_LoopIteration++;
         }
      else
         {
         /* Even when not in run mode, we need to sync m_LoopIteration. */
         Data.m_LoopIteration = Data.m_TargetIteration;
         }

      if(Data.m_LoopIteration >= NbIteration)
         {
         Data.m_LoopIteration = NbIteration-1;
         }

      if(!Data.m_Running.load(std::memory_order_relaxed))
         return 0;
      }
   }

//*****************************************************************************
// Show next step.
//*****************************************************************************
void CRegDisplay::ShowNextStep()
   {
   if(m_Mode == VisualizationMode::RUN)
      {
      m_TargetIteration = m_LoopIteration;
      }
   m_TargetIteration++;
   if(m_TargetIteration > m_NumIterations - 1)
      {
      m_TargetIteration = m_NumIterations - 1;
      }
   m_Mode = VisualizationMode::SINGLE;
   }

//*****************************************************************************
// Show previous step.
//*****************************************************************************
void CRegDisplay::ShowPreviousStep()
   {
   if(m_Mode == VisualizationMode::RUN)
      {
      m_TargetIteration = m_LoopIteration;
      }
   m_TargetIteration--;
   if(m_TargetIteration < 0)
      {
      m_TargetIteration = 0;
      }
   m_Mode = VisualizationMode::SINGLE;
   }

//*****************************************************************************
// Run.
//*****************************************************************************
void CRegDisplay::Run()
   {
   m_Mode = VisualizationMode::RUN;
   }

MIL_ID CRegDisplay::GetMilDisplayID()
   {
   return m_MilDisplay.get();
   }

//*****************************************************************************
// Constructor.
//*****************************************************************************
CDisplayController::CDisplayController():
   m_Running({false})
   {
   }

//*****************************************************************************
// Destructor.
//*****************************************************************************
CDisplayController::~CDisplayController()
   {
   End();
   }

//*****************************************************************************
// Register display.
//*****************************************************************************
void CDisplayController::RegisterDisplay(CRegDisplay * Display)
   {
   m_RegisteredDisplay.push_back(Display);
   }

//*****************************************************************************
// Start.
//*****************************************************************************
void CDisplayController::Start(bool IsFinalDisplay)
   {
   End();

   m_Running.store(true, std::memory_order_relaxed);

   /* Print instructions. */
   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("Use Up Key to see the next iteration.\n"));
   MosPrintf(MIL_TEXT("Use Down Key or Left Key to see the previous iteration.\n"));
   MosPrintf(MIL_TEXT("Use Right Key to loop.\n"));
   MosPrintf(MIL_TEXT("\n"));
   if(IsFinalDisplay)
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   else
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosPrintf(MIL_TEXT("\n"));

   m_MilControlThread = MthrAlloc(M_DEFAULT_HOST, M_THREAD, M_DEFAULT,
                                  CDisplayController::ProcessControlThread, this, M_UNIQUE_ID);
   MthrWait(m_MilControlThread, M_THREAD_END_WAIT, M_NULL);
   }

//*****************************************************************************
// End.
//*****************************************************************************
void CDisplayController::End()
   {
   if(m_Running.load(std::memory_order_relaxed))
      {
      /* Signal threads to end and wait. */
      m_Running.store(false, std::memory_order_relaxed);

      if(m_MilControlThread != M_NULL)
         {
         MthrWait(m_MilControlThread, M_THREAD_END_WAIT, M_NULL);
         }
      }
   }

//*****************************************************************************
// Control thread.
//*****************************************************************************
MIL_UINT32 MFTYPE CDisplayController::ProcessControlThread(void* pUserData)
   {
   CDisplayController& Data = *(CDisplayController*)pUserData;

   /* Read pressed key and control visualization mode. */
   while(Data.m_Running.load(std::memory_order_relaxed))
      {
      MIL_INT Input = MosGetch();
      switch(Input)
         {
         case KEY_UP:
         {
         for(MIL_UINT Index = 0; Index < Data.m_RegisteredDisplay.size(); Index++)
            {
            CRegDisplay* Display = Data.m_RegisteredDisplay[Index];
            if(Display != M_NULL)
               {
               Display->ShowNextStep();
               }
            }

         break;
         }
         case KEY_LEFT:
         case KEY_DOWN:
         {
         for(MIL_UINT Index = 0; Index < Data.m_RegisteredDisplay.size(); Index++)
            {
            CRegDisplay* Display = Data.m_RegisteredDisplay[Index];
            if(Display != M_NULL)
               {
               Display->ShowPreviousStep();
               }
            }

         break;
         }
         case KEY_RIGHT:
         {
         for(MIL_UINT Index = 0; Index < Data.m_RegisteredDisplay.size(); Index++)
            {
            CRegDisplay* Display = Data.m_RegisteredDisplay[Index];
            if(Display != M_NULL)
               {
               Display->Run();
               }
            }

         break;
         }
         case KEY_SKIP:
         {
         break;
         }
         default:
            for(MIL_UINT Index = 0; Index < Data.m_RegisteredDisplay.size(); Index++)
               {
               CRegDisplay* Display = Data.m_RegisteredDisplay[Index];
               if(Display != M_NULL)
                  {
                  Display->End();
                  }
               }
            return 0;
         }
      }

   return 0;
   }
