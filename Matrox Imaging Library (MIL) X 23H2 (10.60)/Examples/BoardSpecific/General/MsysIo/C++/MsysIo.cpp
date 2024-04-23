﻿/******************************************************************************/
/*
 * File name: MsysIo.cpp 
 *
 * Synopsis:  This example shows how to use the hardware command list and
 *            the timers to achieve real-time and determinist control of
 *            the auxiliary I/Os.
 *
 *            The example can be configured to do a rotary encoder based
 *            distance delay or a time based delay on an auxiliary
 *            output.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

#define PULSE_WIDTH 1000000000  /* 1 second pulse width. */

/* Auxiliary I/O change callback structure. */
typedef struct _HOOK_PARAM 
   {
   MIL_ID MilSystem;
   MIL_ID CmdListId;
   MIL_INT64 Operation;
   MIL_DOUBLE Delay;
   } HOOK_PARAM, *PHOOK_PARAM;

/* Example functions declarations. */
void RotaryEncoderDelayExample(MIL_ID MilSystem);
void TimeDelayExample(MIL_ID MilSystem);
MIL_INT MFTYPE IoHookFunction(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr);

/*****************************************************************************
Main.
*****************************************************************************/
int MosMain(void)
   {
   MIL_ID MilApplication,  /* Application identifier.  */
          MilSystem;       /* System identifier.       */
   MIL_INT Selection=1;

   /* Allocate a MIL application. */
   MappAlloc(M_DEFAULT, &MilApplication);

   MosPrintf(MIL_TEXT("MsysIo example on 4Sight GPm or Indio:\n"));
   MosPrintf(MIL_TEXT("----------------------------------\n\n"));
   MosPrintf(MIL_TEXT("This example shows how to register a timed pulse after a specifed\n"));
   MosPrintf(MIL_TEXT("number of rotary encoder ticks or after a specified time when\n"));
   MosPrintf(MIL_TEXT("a trigger is received.\n"));
   MosPrintf(MIL_TEXT("Choose the system to use:\n"));
   MosPrintf(MIL_TEXT("1) 4Sight GPm.\n"));
   MosPrintf(MIL_TEXT("2) Indio.\n"));
   Selection = MosGetch();

   /* Allocate a system, assume first system of specified type (M_DEV0). */
   switch (Selection)
   {
   case '1':
   case '\r':
      MsysAlloc(M_SYSTEM_HOST, M_DEV0, M_DEFAULT, &MilSystem);
      break;

   case '2':
      MsysAlloc(M_SYSTEM_INDIO, M_DEV0, M_DEFAULT, &MilSystem);
      break;

   default:
      MsysAlloc(M_SYSTEM_HOST, M_DEV0, M_DEFAULT, &MilSystem);
      break;
   }

   MosPrintf(MIL_TEXT("Choose the type of delay:\n"));
   MosPrintf(MIL_TEXT("1) The delay is based on rotary encoder ticks.\n") );
   MosPrintf(MIL_TEXT("2) The delay is based on time.\n"));
   Selection = MosGetch();

   switch(Selection)
   {
   case '1':
   case '\r':
       RotaryEncoderDelayExample(MilSystem);
       break;

   case '2':
       TimeDelayExample(MilSystem);
       break;

   default:
       MosPrintf(MIL_TEXT("\nInvalid selection !.\n\nUsing delay based on time.\n\n"));
       TimeDelayExample(MilSystem);
       break;
   }

   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
}

/*****************************************************************************
 Delay in number of rotary encoder ticks example. 
 *****************************************************************************/
void RotaryEncoderDelayExample(MIL_ID MilSystem)
   {
   MIL_ID MilCmdList; /* I/O Command List identifier. */
   HOOK_PARAM HookParam;
   MIL_STRING SysName;

   if (MsysInquire(MilSystem, M_SYSTEM_TYPE, M_NULL) == M_SYSTEM_HOST_TYPE)
      SysName = MIL_TEXT("4Sight GPm");
   else
      SysName = MIL_TEXT("Indio");


   /* First input corresponds to M_AUX_IO8 in MIL. */
   /* Second input corresponds to M_AUX_IO9 in MIL. */
   /* Third input corresponds to M_AUX_IO10 in MIL. */
   /* Last output corresponds to M_AUX_IO7 in MIL. */
   MosPrintf(MIL_TEXT("The delay will be based on rotary encoder ticks.\n\n"));
   MosPrintf(MIL_TEXT("Do the following connection:\n"));
   MosPrintf(MIL_TEXT("1- Connect a trigger signal or switch on M_AUX_IO8 of %s.\n"), SysName.c_str());
   MosPrintf(MIL_TEXT("2- Connect Line A of rotary encoder to M_AUX_IO9 of %s.\n"), SysName.c_str());
   MosPrintf(MIL_TEXT("3- Connect Line B of rotary encoder to M_AUX_IO10 of %s.\n"), SysName.c_str());
   MosPrintf(MIL_TEXT("4- Verify or probe Output 8 of %s.\n"), SysName.c_str());
   MosPrintf(MIL_TEXT("Press <Enter> when ready.\n\n"));
   MosGetch();

   /* Setup Rotary encoder input (ex: positional information). */
   MsysControl(MilSystem, M_ROTARY_ENCODER_BIT0_SOURCE + M_ROTARY_ENCODER1, 
      M_AUX_IO9);  /* Connector's Input 2. */
   MsysControl(MilSystem, M_ROTARY_ENCODER_BIT1_SOURCE + M_ROTARY_ENCODER1, 
      M_AUX_IO10); /* Connector's Input 3. */
   MsysControl(MilSystem, M_ROTARY_ENCODER_OUTPUT_MODE + M_ROTARY_ENCODER1, 
      M_STEP_FORWARD);
   MsysControl(MilSystem, M_ROTARY_ENCODER_STATE + M_ROTARY_ENCODER1, M_ENABLE);

   /* Setup I/O Output (Ex: ejector pulse). */
   MsysControl(MilSystem, M_IO_SOURCE + M_AUX_IO7, M_TIMER1); /* Connector's Output 8. */
   MsysControl(MilSystem, M_TIMER_TRIGGER_ACTIVATION+M_TIMER1, M_DEFAULT); /* Edge rising. */
   MsysControl(MilSystem, M_TIMER_DELAY+M_TIMER1, 0);
   MsysControl(MilSystem, M_TIMER_DURATION+M_TIMER1, PULSE_WIDTH);
   MsysControl(MilSystem, M_TIMER_TRIGGER_SOURCE + M_TIMER1, 
      M_IO_COMMAND_LIST1 + M_IO_COMMAND_BIT0);
   MsysControl(MilSystem, M_TIMER_STATE + M_TIMER1, M_ENABLE);

   /* Allocate a command list based on Rotary Encoder 1. */
   MilCmdList = MsysIoAlloc(MilSystem, M_IO_COMMAND_LIST1, M_IO_COMMAND_LIST, 
      M_ROTARY_ENCODER1, M_NULL);

   if (MilCmdList!= M_NULL)
      {
      /* We will latch counter upon reception of a trigger on connector's first input */
      /* (M_AUX_IO8). */
      MsysIoControl(MilCmdList, M_REFERENCE_LATCH_TRIGGER_SOURCE+M_LATCH1, M_AUX_IO8);
      MsysIoControl(MilCmdList, M_REFERENCE_LATCH_ACTIVATION+M_LATCH1, M_EDGE_RISING);
      MsysIoControl(MilCmdList, M_REFERENCE_LATCH_STATE+M_LATCH1, M_ENABLE);

      /* Debounce Input 1 during 25 ms. */
      MsysControl(MilSystem, M_IO_DEBOUNCE_TIME+M_AUX_IO8, 25000000);

      /* Enable interrupt generation on reception of a trigger on M_AUX_IO8 */
      /* (ex: object detection) and hook a callback function to it. */
      MsysControl(MilSystem, M_IO_INTERRUPT_ACTIVATION+M_AUX_IO8, M_EDGE_RISING);
      HookParam.MilSystem = MilSystem;
      HookParam.CmdListId = MilCmdList;
      HookParam.Operation = M_IMPULSE;
      HookParam.Delay = 100; /* 100 rotary encoder ticks after trigger */
      MsysHookFunction(MilSystem, M_IO_CHANGE, IoHookFunction, &HookParam);
      MsysControl(MilSystem, M_IO_INTERRUPT_STATE+M_AUX_IO8, M_ENABLE);

      MosPrintf(MIL_TEXT("Activate your rotary encoder.\n"));
      MosPrintf(MIL_TEXT("Send a rising edge trigger to M_AUX_IO8.\n"));
      MosPrintf(MIL_TEXT("Verify 1 second pulse (100 ticks after trigger) on M_AUX_IO7.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> when you are ready to end.\n\n"));
      MosGetch();

      /* Put back important controls to default value. */
      MsysControl(MilSystem, M_TIMER_STATE + M_TIMER1, M_DEFAULT);
      MsysControl(MilSystem, M_IO_SOURCE+M_AUX_IO0, M_USER_BIT0);
      MsysIoControl(MilCmdList, M_REFERENCE_LATCH_STATE+M_LATCH1, M_DEFAULT);
      MsysControl(MilSystem, M_IO_INTERRUPT_STATE+M_AUX_IO8, M_DEFAULT);
      MsysControl(MilSystem, M_IO_DEBOUNCE_TIME+M_AUX_IO8, 0);

      /* Unhook the callback function. */
      MsysHookFunction(MilSystem, M_IO_CHANGE+M_UNHOOK, IoHookFunction, &HookParam);

      /* Free the I/O command list. */
      MsysIoFree(MilCmdList);
      }
   else
      {
      MosPrintf(MIL_TEXT("MIL was unable to allocate an I/O command list.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      }
}

/*****************************************************************************
 Delay in time example. 
 *****************************************************************************/
void TimeDelayExample(MIL_ID MilSystem)
   {
   MIL_ID MilCmdList; /* I/O Command List identifier. */
   HOOK_PARAM HookParam;
   MIL_STRING SysName;

   if (MsysInquire(MilSystem, M_SYSTEM_TYPE, M_NULL) == M_SYSTEM_HOST_TYPE)
      SysName = MIL_TEXT("4Sight GPm");
   else
      SysName = MIL_TEXT("Indio");

   /* First input corresponds to M_AUX_IO8 in MIL. */
   /* Last output corresponds to M_AUX_IO7 in MIL. */
   MosPrintf(MIL_TEXT("The delay will be based on time.\n\n"));
   MosPrintf(MIL_TEXT("Do the following connection:\n"));
   MosPrintf(MIL_TEXT("1- Connect a trigger signal or switch on M_AUX_IO8 of %s.\n"), SysName.c_str());
   MosPrintf(MIL_TEXT("2- Verify or probe M_AUX_IO7 of %s.\n"), SysName.c_str());
   MosPrintf(MIL_TEXT("Press <Enter> when ready.\n\n"));
   MosGetch();
   
   MsysControl(MilSystem, M_IO_SOURCE + M_AUX_IO7, 
      M_IO_COMMAND_LIST1 + M_IO_COMMAND_BIT0); /* Connector's last output. */

   /* Allocate a command list based on time. */
   MilCmdList = MsysIoAlloc(MilSystem, M_IO_COMMAND_LIST1, M_IO_COMMAND_LIST, 
      M_CLOCK, M_NULL);

   if (MilCmdList!= M_NULL)
      {
      /* We will latch counter upon reception of a trigger on connector's first input */ 
      /* (M_AUX_IO8). */
      MsysIoControl(MilCmdList, M_REFERENCE_LATCH_TRIGGER_SOURCE+M_LATCH1, M_AUX_IO8);
      MsysIoControl(MilCmdList, M_REFERENCE_LATCH_ACTIVATION+M_LATCH1, M_EDGE_RISING);
      MsysIoControl(MilCmdList, M_REFERENCE_LATCH_STATE+M_LATCH1, M_ENABLE);

      /* Debounce INPUT 1 during 25 ms. */
      MsysControl(MilSystem, M_IO_DEBOUNCE_TIME+M_AUX_IO8, 25000000);

      /* Enable interrupt generation on reception of a trigger on M_AUX_IO8 */
      /* (ex: object detection) and hook a callback function to it. */
      MsysControl(MilSystem, M_IO_INTERRUPT_ACTIVATION+M_AUX_IO8, M_EDGE_RISING);
      HookParam.MilSystem = MilSystem;
      HookParam.CmdListId = MilCmdList;
      HookParam.Operation = M_EDGE_RISING;
      HookParam.Delay = 2; /* 2 seconds after trigger. */
      MsysHookFunction(MilSystem, M_IO_CHANGE, IoHookFunction, &HookParam);
      MsysControl(MilSystem, M_IO_INTERRUPT_STATE+M_AUX_IO8, M_ENABLE);

      MosPrintf(MIL_TEXT("Send a rising edge trigger to Input 1.\n"));
      MosPrintf(MIL_TEXT("Verify toggle of M_AUX_IO7, 2 seconds after trigger.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> when you are ready to end.\n\n"));
      MosGetch();

      /* Put back important controls to default value. */
      MsysControl(MilSystem, M_IO_SOURCE+M_AUX_IO0, M_USER_BIT0);
      MsysIoControl(MilCmdList, M_REFERENCE_LATCH_STATE+M_LATCH1, M_DEFAULT);
      MsysControl(MilSystem, M_IO_INTERRUPT_STATE+M_AUX_IO8, M_DEFAULT);
      MsysControl(MilSystem, M_IO_DEBOUNCE_TIME+M_AUX_IO8, 0);

      /* Unhook the callback function. */
      MsysHookFunction(MilSystem, M_IO_CHANGE+M_UNHOOK, IoHookFunction, &HookParam);

      /* Free the I/O command list. */
      MsysIoFree(MilCmdList);
      }
   else
      {
      MosPrintf(MIL_TEXT("MIL was unable to allocate an I/O command list.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      }
}

/* Interrupt hook function. */
MIL_INT MFTYPE IoHookFunction(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr)
   {
   PHOOK_PARAM pHookParam = (PHOOK_PARAM)UserDataPtr;
   MIL_INT PinNb = 0;
   MIL_INT Status;
   MIL_INT64 RefStamp;

   /* If the callback is for M_AUX_IO8. */
   MsysGetHookInfo(pHookParam->MilSystem, EventId, M_IO_INTERRUPT_SOURCE, &PinNb);
   if (PinNb == M_AUX_IO8)
      {
      MsysGetHookInfo(pHookParam->MilSystem, EventId, 
         M_REFERENCE_LATCH_VALUE+M_IO_COMMAND_LIST1+M_LATCH1, &RefStamp);
      MsysIoCommandRegister(pHookParam->CmdListId, pHookParam->Operation, 
         RefStamp, pHookParam->Delay, M_DEFAULT, M_IO_COMMAND_BIT0, &Status);

      if (pHookParam->Operation == M_EDGE_RISING)
         pHookParam->Operation = M_EDGE_FALLING;
      else if (pHookParam->Operation == M_EDGE_FALLING)
         pHookParam->Operation = M_EDGE_RISING;

      if (Status == M_NULL)
         MosPrintf(MIL_TEXT("MIL successfully registered an I/O event.\n"));
      else if (Status == M_INVALID)
         MosPrintf(MIL_TEXT(
            "MIL determined that the position where to register an I/O is passed.\n"));
      }

   return M_NULL;
}