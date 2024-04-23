/*************************************************************************************/
/*
 * File name: FrameBurst.cpp
 *
 * Synopsis:  Demonstrates the grab frame burst API.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("FrameBurst\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program uses the frame burst API to aggregate multiple frames into each\n")\
             MIL_TEXT("grab command. The total number of acquired frames per grab command issued is\n")\
             MIL_TEXT("tabulated and the results are shown on screen.\n\n")\
             MIL_TEXT("This API is useful when acquiring from high frame rate cameras that might\n")\
             MIL_TEXT("cause frames to be lost if they are grabbed only one frame at a time.\n\n")\
             MIL_TEXT("The end of the grab can be signaled by up to 3 events:\n")\
             MIL_TEXT(" 1: an external signal has triggered the end of frame aggregation;\n")\
             MIL_TEXT(" 2: the frame aggregation count has been reached;\n")\
             MIL_TEXT(" 3: the maximum aggregation time has elapsed.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, digitizer.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\r"));
   MosGetch();
   }

/* Specifies the number of grabs in the buffered grab queue. */
#define BUFFERING_SIZE_MAX 10

/* User-defined data latch structure. */
typedef struct
   {
   MIL_ID  MilDigitizer;
   MIL_INT SizeX;
   MIL_INT SizeY;
   MIL_ID  MilImageDisp;
   MIL_INT TotalGrabCount;
   MIL_INT TotalFrameCount;
   MIL_INT FrameBurstEndCount;
   MIL_INT FrameBurstEndTrig;
   MIL_INT FrameBurstEndTime;
   } HookDataStruct;

/* Verifies whether this example can run on the selected system. */
bool SystemSupportsFrameBurst(MIL_ID MilSystem);

/* User-defined processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId,
                                  void* HookDataPtr);

/* Main function. */
/* ---------------*/

int MosMain(void)
   {
   MIL_ID MilApplication;
   MIL_ID MilSystem;
   MIL_ID MilDigitizer;
   MIL_ID MilDisplay;
   MIL_ID MilImageDisp;
   MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX] = { 0 };
   MIL_INT MilGrabBufferListSize;
   MIL_INT ProcessFrameCount = 0;
   MIL_INT NbFrames = 0, n = 0;
   MIL_DOUBLE ProcessGrabRate = 0;
   MIL_INT FrameBurstSize = 10;
   MIL_DOUBLE FrameBurstMaxTime = 0.0;
   HookDataStruct UserHookData;

   /* Allocates a default application. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay,
                    &MilDigitizer, &MilImageDisp);

   if(!SystemSupportsFrameBurst(MilSystem))
      {
      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
      return 1;
      }

   PrintHeader();

   /* Sets the maximum number of frames in each grab buffer. */
   MdigControl(MilDigitizer, M_GRAB_FRAME_BURST_SIZE, FrameBurstSize);

   /* Sets the maximum amount of time to wait for all the frames to be grabbed. */
   /* The value, in seconds, is set to 100ms. */
   FrameBurstMaxTime = 0.100;
   MdigControl(MilDigitizer, M_GRAB_FRAME_BURST_MAX_TIME, FrameBurstMaxTime);

   /* Specifies that an external AUX IO signal can trigger the end of a grab. */
   MdigControl(MilDigitizer, M_GRAB_FRAME_BURST_END_TRIGGER_STATE, M_ENABLE);
   MdigControl(MilDigitizer, M_GRAB_FRAME_BURST_END_TRIGGER_SOURCE, M_AUX_IO0);

   /* Inquires the size of the grab image. Used for buffer allocations. */
   MdigInquire(MilDigitizer, M_SIZE_X, &UserHookData.SizeX);
   MdigInquire(MilDigitizer, M_SIZE_Y, &UserHookData.SizeY);

   /* Allocates the grab buffers and clear them. */
   MappControl(M_ERROR, M_PRINT_DISABLE);
   for (MilGrabBufferListSize = 0;
        MilGrabBufferListSize < BUFFERING_SIZE_MAX; MilGrabBufferListSize++)
      {
      /* Frame burst buffers must be allocated on-board with a SizeY multiplied  */
      /* by the FrameBurstSize (resulting in the sum of SizeY for every frame in the frame burst). */
      MbufAlloc2d(MilSystem,
                  UserHookData.SizeX,
                  UserHookData.SizeY * FrameBurstSize,
                  8 + M_UNSIGNED,
                  M_IMAGE + M_GRAB,
                  &MilGrabBufferList[MilGrabBufferListSize]);
      if(MilGrabBufferList[MilGrabBufferListSize] == M_NULL)
         break;
      }
   MappControl(M_ERROR, M_PRINT_ENABLE);

   if (MilGrabBufferListSize < 2)
      {
      MosPrintf(MIL_TEXT("\nError. Not enough memory to allocate grab buffer.\n"));
      MosPrintf(MIL_TEXT("This example is used to aggregate multiple frames from the camera into one buffer.\n"));
      MosPrintf(MIL_TEXT("This example is not intended to be used with large area scan cameras.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to exit.\n"));
      MosGetch();
      while (MilGrabBufferListSize > 0)
         MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);
      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
      return 1;
      }

   /* Initializes the user-defined processing function's data structure. */
   UserHookData.MilDigitizer = MilDigitizer;
   UserHookData.MilImageDisp = MilImageDisp;
   UserHookData.TotalFrameCount = 0;
   UserHookData.TotalGrabCount = 0;
   UserHookData.FrameBurstEndCount = 0;
   UserHookData.FrameBurstEndTime = 0;
   UserHookData.FrameBurstEndTrig = 0;

   MosPrintf(MIL_TEXT("Grab in progress. Press <Enter> to stop.\n\n\n"));

   MosPrintf(MIL_TEXT(" Total grab | Total frames | Frames   | End of frame aggregation   \n"));
   MosPrintf(MIL_TEXT(" commands   | acquired     | per grab | event:                     \n"));
   MosPrintf(MIL_TEXT(" issued     |              | command  | Trig  | Count  | Max time  \n"));
   MosPrintf(MIL_TEXT("============|==============|==========|=======|========|===========\n"));

   /* Start processing. The processing function is called once for each frame grabbed. */
   MdigProcess(MilDigitizer,
               MilGrabBufferList,
               MilGrabBufferListSize,
               M_START,
               M_DEFAULT,
               ProcessingFunction,
               &UserHookData);

   MosGetch();

   /* Stop processing. */
   MdigProcess(MilDigitizer,
               MilGrabBufferList,
               MilGrabBufferListSize,
               M_STOP,
               M_DEFAULT,
               ProcessingFunction,
               &UserHookData);

   MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &ProcessGrabRate);

   MosPrintf(MIL_TEXT("\n\n%ld frames grabbed at %.1f frames/sec or %.1f grabs/sec.\n"),
             UserHookData.TotalFrameCount,
             ProcessGrabRate * UserHookData.TotalFrameCount / UserHookData.TotalGrabCount,
             ProcessGrabRate);

   MosPrintf(MIL_TEXT("\n\n%ld bursts ended when the frame aggregation count has been reached.\n"), UserHookData.FrameBurstEndCount);
   MosPrintf(MIL_TEXT("%ld bursts ended when the maximum aggregation time has elapsed.\n"), UserHookData.FrameBurstEndTime);
   MosPrintf(MIL_TEXT("%ld bursts ended when an external signal has triggered the end of frame\naggregation. \n"), UserHookData.FrameBurstEndTrig);

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Frees the grab buffers. */
   while (MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);

   /* Releases the defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);

   return 0;
   }

/* Verifies whether this example can run on the selected system. */
bool SystemSupportsFrameBurst(MIL_ID MilSystem)
   {
   MIL_INT SystemType = 0;

   MsysInquire(MilSystem, M_SYSTEM_TYPE, &SystemType);
   if(SystemType == M_SYSTEM_RADIENTCXP_TYPE ||
      SystemType == M_SYSTEM_RADIENTPRO_TYPE ||
      SystemType == M_SYSTEM_RADIENTEVCL_TYPE ||
      SystemType == M_SYSTEM_RAPIXOCL_TYPE ||
      SystemType == M_SYSTEM_RAPIXOCXP_TYPE)
      {
      return true;
      }

   MosPrintf(MIL_TEXT("This example program can only be used with the Matrox driver for ")
             MIL_TEXT("the Matrox Radient family and the Matrox Rapixo.\n"));
   MosPrintf(MIL_TEXT("Ensure that the default system type is set accordingly in ")
              MIL_TEXT("MIL Config.\n"));
   MosPrintf(MIL_TEXT("-------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
   MosGetch();
   return false;
   }

/* The user-defined processing function is called each time a grab buffer is modified. */
/* -------------------------------------------------------------------------------------*/
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId,
   void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_ID BufChild;
   MIL_INT FrameCount = 0;
   MIL_INT FrameBurstEndSource = 0;

   /* Retrieves information of the grab buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);
   MdigGetHookInfo(HookId, M_GRAB_FRAME_BURST_COUNT, &FrameCount);
   MdigGetHookInfo(HookId, M_GRAB_FRAME_BURST_END_SOURCE, &FrameBurstEndSource);

   /* Copies each frame in the modified grab buffer to a display buffer. */
   /* The destination (display) buffer must be allocated in M_NON_PAGED memory. */
   MbufChild2d(ModifiedBufferId, 0, 0, UserHookDataPtr->SizeX, UserHookDataPtr->SizeY, &BufChild);
   for(MIL_INT i = 0; i < FrameCount; i++)
      {
      MbufChildMove(BufChild, 0, i * UserHookDataPtr->SizeY, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MbufCopy(BufChild, UserHookDataPtr->MilImageDisp);
      }
   MbufFree(BufChild);

   UserHookDataPtr->TotalGrabCount++;
   UserHookDataPtr->TotalFrameCount += FrameCount;
   if(FrameBurstEndSource & M_BURST_TRIGGER)
      UserHookDataPtr->FrameBurstEndTrig++;
   if(FrameBurstEndSource & M_BURST_COUNT)
      UserHookDataPtr->FrameBurstEndCount++;
   if(FrameBurstEndSource & M_BURST_MAX_TIME)
      UserHookDataPtr->FrameBurstEndTime++;

   MosPrintf(MIL_TEXT(" %10lld | %12lld | %8lld | %5s | %5s  | %8s  \r"),
             (long long)UserHookDataPtr->TotalGrabCount,
             (long long)UserHookDataPtr->TotalFrameCount,
             (long long)FrameCount,
             FrameBurstEndSource & M_BURST_TRIGGER  ? MIL_TEXT("Trig") : MIL_TEXT("----"),
             FrameBurstEndSource & M_BURST_COUNT    ? MIL_TEXT("Count") : MIL_TEXT("-----"),
             FrameBurstEndSource & M_BURST_MAX_TIME ? MIL_TEXT("Max time") : MIL_TEXT("-------"));

   return 0;
   }
