﻿/*************************************************************************************/
/*
 * File name: DataLatch.cpp
 *
 * Synopsis:  This program uses the data latch API to latch information
 *            (timestamp and auxiliary I/Os values) on each grabbed frame.
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
   MosPrintf(MIL_TEXT("DataLatch\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program uses the data latch API to deliver additional information with\n")\
             MIL_TEXT("each grabbed frame. \n\n")\
             MIL_TEXT("The additional information (timestamp, aux I/O values, rotary encoder count)\n")\
             MIL_TEXT("can be latched by:\n")\
             MIL_TEXT(" 1: start of grab;\n")\
             MIL_TEXT(" 2: end of grab;\n")\
             MIL_TEXT(" 3: each grabbed line;\n")\
             MIL_TEXT(" 4: rotary encoder trigger;\n")\
             MIL_TEXT(" 5: an external signal.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, digitizer.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\r"));
   MosGetch();
   }


/* Specifies the number of images in the buffering grab queue.
   Generally, increasing this number improves the real-time grab results.
   */
#define BUFFERING_SIZE_MAX 20

/* User-defined processing function Data Latch structure. */
typedef struct
   {
   MIL_ID  MilDigitizer;
   MIL_ID  MilImageDisp;
   MIL_INT ProcessedImageCount;
   } HookDataStruct;

/* Verifies if this example can run on the selected system. */
bool SystemSupportsDataLatch(MIL_ID MilSystem);

/* User-defined processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId,
                                  void* HookDataPtr);

#define INDEX_FOR_FRAME_START M_LATCH0
#define INDEX_FOR_FRAME_END   M_LATCH1
#define INDEX_FOR_AUXIO       M_LATCH2

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
   MIL_DOUBLE ProcessFrameRate = 0;
   HookDataStruct UserHookData;

   /* Allocate a default application. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay,
                    &MilDigitizer, &MilImageDisp);

   if(!SystemSupportsDataLatch(MilSystem))
      {
      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
      return 1;
      }

   PrintHeader();

   /* Allocate the grab buffers and clear them. */
   MappControl(M_ERROR, M_PRINT_DISABLE);
   MIL_INT NonPagedMemoryTotalSize = 0;
   MIL_INT NonPagedMemoryUsed = 0;
   MappInquire(M_NON_PAGED_MEMORY_SIZE, &NonPagedMemoryTotalSize);

   for(MilGrabBufferListSize = 0;
      MilGrabBufferListSize < BUFFERING_SIZE_MAX; MilGrabBufferListSize++)
      {
      MbufAlloc2d(MilSystem,
         MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
         MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
         8 + M_UNSIGNED,
         M_IMAGE + M_GRAB,
         &MilGrabBufferList[MilGrabBufferListSize]);

      if(MilGrabBufferList[MilGrabBufferListSize])
         MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);

      // Leave about 80% of free non paged memory for temporary buffer allocations.
      MappInquire(M_NON_PAGED_MEMORY_USED, &NonPagedMemoryUsed);
      if(((double)NonPagedMemoryUsed / (double)NonPagedMemoryTotalSize) > 0.8)
         {
         MilGrabBufferListSize++;
         break;
         }
      }
   MappControl(M_ERROR, M_PRINT_ENABLE);

   /* Initialize the user-defined processing function data structure. */
   UserHookData.MilDigitizer = MilDigitizer;
   UserHookData.MilImageDisp = MilImageDisp;
   UserHookData.ProcessedImageCount = 0;

   /* Enable latching the timestamp upon the start of frame. */
   MdigControl(MilDigitizer, M_DATA_LATCH_TRIGGER_SOURCE + INDEX_FOR_FRAME_START, M_GRAB_FRAME_START);
   MdigControl(MilDigitizer, M_DATA_LATCH_TYPE  + INDEX_FOR_FRAME_START, M_TIME_STAMP);
   MdigControl(MilDigitizer, M_DATA_LATCH_STATE + INDEX_FOR_FRAME_START, M_ENABLE);

   /* Enable latching the timestamp upon the end of frame. */
   MdigControl(MilDigitizer, M_DATA_LATCH_TRIGGER_SOURCE + INDEX_FOR_FRAME_END, M_GRAB_FRAME_END);
   MdigControl(MilDigitizer, M_DATA_LATCH_TYPE  + INDEX_FOR_FRAME_END, M_TIME_STAMP);
   MdigControl(MilDigitizer, M_DATA_LATCH_STATE + INDEX_FOR_FRAME_END, M_ENABLE);

   /* Enable latching the aux I/O values upon the each grabbed line. */
   MdigControl(MilDigitizer, M_DATA_LATCH_TRIGGER_SOURCE + INDEX_FOR_AUXIO, M_GRAB_LINE);
   MdigControl(MilDigitizer, M_DATA_LATCH_TYPE  + INDEX_FOR_AUXIO, M_IO_STATUS_ALL);
   MdigControl(MilDigitizer, M_DATA_LATCH_STATE + INDEX_FOR_AUXIO, M_ENABLE);

   MosPrintf(MIL_TEXT("Grab in progress. Press <Enter> to stop.\n\n"));

   MosPrintf(MIL_TEXT(" Timestamp on   | Timestamp on  | Frame time | AuxIO status   | AuxIO status   \n"));
   MosPrintf(MIL_TEXT(" start of frame | end of frame  | end - start| bits latched on| bits latched on\n"));
   MosPrintf(MIL_TEXT(" (clock ticks)  | (clock ticks) |    (ms)    | the first line | the last line  \n"));
   MosPrintf(MIL_TEXT("================|===============|============|================|================\n"));

   /* Start processing. The processing function is called once for every frame grabbed. */
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

   /* Print resulting statistics. */
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT, &ProcessFrameCount);
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &ProcessFrameRate);
   MosPrintf(MIL_TEXT("\n\n%ld frames grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
             ProcessFrameCount,
             ProcessFrameRate,
             1000.0 / ProcessFrameRate);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Disable the DataLatchs. */
   MdigControl(MilDigitizer, M_DATA_LATCH_STATE + INDEX_FOR_FRAME_START, M_DISABLE);
   MdigControl(MilDigitizer, M_DATA_LATCH_STATE + INDEX_FOR_FRAME_END, M_DISABLE);
   MdigControl(MilDigitizer, M_DATA_LATCH_STATE + INDEX_FOR_AUXIO, M_DISABLE);

   /* Free the grab buffers. */
   while(MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);

   /* Release the defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);

   return 0;
   }

/* Verify whether this example can run on the selected system. */
bool SystemSupportsDataLatch(MIL_ID MilSystem)
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

   MosPrintf(MIL_TEXT("This example program can only be used with the Matrox Driver for ")
               MIL_TEXT("Radient eV-CXP, Rapixo CXP, RadientPro, Radient eV-CL or Rapixo Pro CL.\n"));
   MosPrintf(MIL_TEXT("Please ensure that the default system type is set accordingly in ")
               MIL_TEXT("MIL Config.\n"));
   MosPrintf(MIL_TEXT("-------------------------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
   MosGetch();
   return false;
   }

/* This user-defined processing function is called each time a grab buffer is modified. */
/* -------------------------------------------------------------------------------------*/


MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId,
   void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_INT NumberOfFrameStarts = 0; 
   MIL_INT NumberOfFrameEnds = 0; 
   MIL_INT64 TimeStampOnFrameStartInTicks = 0;
   MIL_INT64 TimeStampOnFrameEndInTicks = 0;
   MIL_DOUBLE TimeStampDeltaInMSec = 0;

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);

   /* Get the number of StartOfGrab and EndOfGrab data latched. */
   MdigGetHookInfo(HookId, M_DATA_LATCH_VALUE_COUNT + INDEX_FOR_FRAME_START, &NumberOfFrameStarts);
   MdigGetHookInfo(HookId, M_DATA_LATCH_VALUE_COUNT + INDEX_FOR_FRAME_END, &NumberOfFrameEnds);

   /* Get the timestamp latched on the frame start and frame end. */
   if((NumberOfFrameStarts > 0) && (NumberOfFrameEnds > 0))
      {
      MdigGetHookInfo(HookId, M_DATA_LATCH_VALUE + INDEX_FOR_FRAME_START, &TimeStampOnFrameStartInTicks);
      MdigGetHookInfo(HookId, M_DATA_LATCH_VALUE + INDEX_FOR_FRAME_END, &TimeStampOnFrameEndInTicks);

      /* Calculate the frame time (end - start) in msecs. */
      MIL_INT TimeStampClockFreqInHz = 0;
      MdigInquire(UserHookDataPtr->MilDigitizer, M_DATA_LATCH_CLOCK_FREQUENCY, &TimeStampClockFreqInHz);
      TimeStampDeltaInMSec = (TimeStampOnFrameEndInTicks - TimeStampOnFrameStartInTicks) * (1000.0 / TimeStampClockFreqInHz);
      }

   /* Print the time stamps to the console. */
   MosPrintf(MIL_TEXT(" %013llX  |"), (long long)TimeStampOnFrameStartInTicks);
   MosPrintf(MIL_TEXT(" %013llX |"), (long long)TimeStampOnFrameEndInTicks);
   MosPrintf(MIL_TEXT(" %9.5f  |"), TimeStampDeltaInMSec);

   /* Get the AuxIO status sampled at each grabbed line. */
   MIL_INT NumberOfLines = 0;
   MdigGetHookInfo(HookId, M_DATA_LATCH_VALUE_COUNT + INDEX_FOR_AUXIO, &NumberOfLines);
   if(NumberOfLines > 0)
      {
      MIL_INT64 *pData = new MIL_INT64[NumberOfLines];

      /* Get all the Data Latch with one function call. */
      MdigGetHookInfo(HookId, M_DATA_LATCH_VALUE_ALL + INDEX_FOR_AUXIO, pData);

      /* Print the resulting values to the console. */
      MosPrintf(MIL_TEXT("   %08llX     |"), (long long)pData[0]);
      MosPrintf(MIL_TEXT("   %08llX"), (long long)pData[NumberOfLines - 1]);

      delete[]pData;
      }

   MosPrintf(MIL_TEXT("\r"));

   /* Update the display. */
   MbufCopy(ModifiedBufferId, UserHookDataPtr->MilImageDisp);

   return 0;
   }