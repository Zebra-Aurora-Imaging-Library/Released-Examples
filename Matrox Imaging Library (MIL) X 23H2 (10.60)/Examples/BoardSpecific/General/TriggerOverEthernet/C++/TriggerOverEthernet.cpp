/*************************************************************************************/
/*
 * File name: TriggerOverEthernet.cpp
 *
 * Synopsis:  This program shows the use of the Matrox ConcordPoE's trigger over Ethernet
 *            offload feature to trigger GigE Vision devices using the Action command.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include "TriggerOverEthernet.h"

/******************************************************************************/
/* Example description.                                                       */
/******************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n\n"));
   MosPrintf(MIL_TEXT("TriggerOverEthernet\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n\n"));
   MosPrintf(
      MIL_TEXT("This program enumerates GigE Vision devices that support trigger\n")\
      MIL_TEXT("over Ethernet and that are physically connected to a Matrox\n")\
      MIL_TEXT("ConcordPoE board that supports TOE functionality. The GigE\n")\
      MIL_TEXT("Vision devices will be set-up in triggered mode using the\n")\
      MIL_TEXT("action command packet or the software trigger packet as trigger\n")\
      MIL_TEXT("source. The Matrox ConcordPoE will be set-up in such a way as to\n")\
      MIL_TEXT("send periodic trigger over Ethernet packets to the GigE Vision\n")\
      MIL_TEXT("devices in order to trigger frame capture.\n")\
      MIL_TEXT("\nPress <Enter> to start.\n\n")\
   );
   }


/* Frequency at which frames will be triggered.                                                         */
/* Make sure to keep the frequency less or equal to the maximum frame rate of your GigE Vision devices. */
MIL_DOUBLE TriggerFrequency = 10.0;

/* Set to true to force usage of software trigger ToE packets. */
bool ForceSoftwareTriggerPackets = false;

/* Utility macro to convert a frequency to a period in nanoseconds */
#define FREQ_TO_PERIOD_IN_NS(N)   ((1.0 / N) * 1E9)

  /* User's processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

void SetupToEUsingActions(MIL_ID MilConcordPoESystem, const std::vector<ToEDevice>& Devices);
void SetupToEUsingSoftware(MIL_ID MilConcordPoESystem, const std::vector<ToEDevice>& Devices);

int MosMain(void)
   {
   MIL_ID MilApplication = M_NULL,
          MilConcordPoESystem = M_NULL,
          MilGigESystem = M_NULL;

   PrintHeader();
   MosGetch();

   /* Allocate a GigE Vision system */
   MappAlloc(M_DEFAULT, &MilApplication);
   MsysAlloc(M_SYSTEM_GIGE_VISION, M_DEV0, M_DEFAULT, &MilGigESystem);

   /* Allocate a ConcordPoE system and validate it has trigger over Ethernet (TOE) support. */
   MsysAlloc(M_SYSTEM_CONCORD_POE, M_DEV0, M_DEFAULT, &MilConcordPoESystem);
   if (!(MsysInquire(MilConcordPoESystem, M_BOARD_TYPE, M_NULL) & M_TOE))
      {
      MsysFree(MilConcordPoESystem);
      MilConcordPoESystem = M_NULL;
      }

   /* Find GigE Vision devices that supports the action command and are physically */
   /* connected to the ConcordPoE system.                                          */
   bool UseActionCapableDevices = !ForceSoftwareTriggerPackets;
   std::vector<ToEDevice> Devices = FindToEDevices(MilConcordPoESystem, MilGigESystem, UseActionCapableDevices);

   /* Check if compatible devices have been found. */
   if (Devices.empty() && !ForceSoftwareTriggerPackets)
      {
      /* No devices supporting actions have been found.                                   */
      /* Find GigE Vision devices that supports the software trigger and are physically   */
      /* connected to the ConcordPoE system.                                              */
      UseActionCapableDevices = false;
      Devices = FindToEDevices(MilConcordPoESystem, MilGigESystem, UseActionCapableDevices);
      }

   if (Devices.empty())
      {
      MosPrintf(MIL_TEXT("No compatible ToE %s have been found.\n"),
         (MilConcordPoESystem == M_NULL ? MIL_TEXT("systems") : MIL_TEXT("devices")));
      MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
      MosGetch();
      if (MilConcordPoESystem)
         {
         MsysFree(MilConcordPoESystem);
         }
      MappFreeDefault(MilApplication, MilGigESystem, M_NULL, M_NULL, M_NULL);
      return 0;
      }

   MosPrintf(MIL_TEXT("Found %d compatible devices using %s:\n"), (int)Devices.size(),
      UseActionCapableDevices ? MIL_TEXT("ACTION COMMAND PACKETS") : MIL_TEXT("SOFTWARE TRIGGER PACKETS"));

   /* Allocate MIL resources (display, grab buffers, etc.) for each device found. */
   for (size_t i = 0; i < Devices.size(); i++)
      {
      MosPrintf(MIL_TEXT("%.2d\t%s %s\n"), (int)i, Devices[i].Vendor.c_str(), Devices[i].Model.c_str());
      Devices[i].Allocate();
      if (UseActionCapableDevices)
         {
         Devices[i].GetActionSelectorInfo();
         }
      }

   MosPrintf(MIL_TEXT("\nPress <enter> to continue.\n"));
   MosGetch();

   /* We will be using M_TIMER1 as a trigger source for ToE packets. We do this for convenience */
   /* reasons in this demo. The user is free to select a different trigger source such as an    */
   /* auxiliary input signal or a rotary decoder for example.                                   */

   /* Set-up M_TIMER1 to run periodically at the specified frequency in Hz. */
   MsysControl(MilConcordPoESystem, M_TIMER1 + M_TIMER_DELAY, FREQ_TO_PERIOD_IN_NS(TriggerFrequency) /2);
   MsysControl(MilConcordPoESystem, M_TIMER1 + M_TIMER_DURATION, FREQ_TO_PERIOD_IN_NS(TriggerFrequency) / 2);
   MsysControl(MilConcordPoESystem, M_TIMER1 + M_TIMER_TRIGGER_SOURCE, M_CONTINUOUS);

   if (UseActionCapableDevices)
      {
      SetupToEUsingActions(MilConcordPoESystem, Devices);
      }
   else
      {
      SetupToEUsingSoftware(MilConcordPoESystem, Devices);
      }

   for (size_t i = 0; i < Devices.size(); i++)
      {
      /* Queue the grab buffers. The processing function is called with every frame */
      /* grabbed.                                                                   */
      MdigProcess(Devices[i].MilDigitizerId, Devices[i].GrabBufList,
         BUFFERING_SIZE_MAX, M_START, M_DEFAULT, ProcessingFunction, &Devices[i]);
      }

   MosPrintf(MIL_TEXT("Press <Enter> to start transmission of ToE packets and frame capture.\n"));
   MosGetch();

   /* Start triggering ToE packets using M_TIMER1. */
   if (UseActionCapableDevices)
      MsysControl(MilConcordPoESystem, M_GC_ACTION0 + M_TRIGGER_STATE, M_ENABLE);
   else
      MsysControl(MilConcordPoESystem, M_GC_TRIGGER_SOFTWARE0 + M_TRIGGER_STATE, M_ENABLE);

   MsysControl(MilConcordPoESystem, M_TIMER1 + M_TIMER_STATE, M_ENABLE);

   /* Here the main() is free to perform other tasks while the processing is executing. */
   /* --------------------------------------------------------------------------------- */

   /* Print a message and wait for a key press after a minimum number of frames.        */
   MosPrintf(MIL_TEXT("Press <Enter> to stop the grab.\n\n"));
   MosGetch();

   /* Stop triggering ToE packets. */
   MsysControl(MilConcordPoESystem, M_TIMER1 + M_TIMER_STATE, M_DISABLE);
   if (UseActionCapableDevices)
      MsysControl(MilConcordPoESystem, M_GC_ACTION0 + M_TRIGGER_STATE, M_DISABLE);
   else
      MsysControl(MilConcordPoESystem, M_GC_TRIGGER_SOFTWARE0 + M_TRIGGER_STATE, M_DISABLE);
   MosSleep(250);

   for (size_t i = 0; i < Devices.size(); i++)
      {
      MIL_INT ProcessFrameCount = 0;
      MIL_DOUBLE ProcessFrameRate = 0;

      /* Stop the processing. */
      MdigProcess(Devices[i].MilDigitizerId, Devices[i].GrabBufList,
         BUFFERING_SIZE_MAX, M_STOP, M_DEFAULT, ProcessingFunction, &Devices[i]);

      /* Print statistics. */
      MdigInquire(Devices[i].MilDigitizerId, M_PROCESS_FRAME_COUNT, &ProcessFrameCount);
      MdigInquire(Devices[i].MilDigitizerId, M_PROCESS_FRAME_RATE, &ProcessFrameRate);
      MosPrintf(MIL_TEXT("%d frames grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
         (int)ProcessFrameCount, ProcessFrameRate, 1000.0 / ProcessFrameRate);
      }

   MosPrintf(MIL_TEXT("\nInter-frame jitter measurements (in nanoseconds):\n"));
   PrintInterfameJitter(Devices, TriggerFrequency);

   if (!UseActionCapableDevices)
      {
      MosPrintf(MIL_TEXT("\nNOTE: SOFTWARE TRIGGER PACKETS were used so inter-frame jitter can increase significantly.\n"));
      }

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free allocated MIL resources. */
   for (size_t i = 0; i < Devices.size(); i++)
      {
      Devices[i].Free();
      }

   MsysFree(MilConcordPoESystem);
   MsysFree(MilGigESystem);
   MappFree(MilApplication);
   return 0;
   }

void SetupToEUsingActions(MIL_ID MilConcordPoESystem, const std::vector<ToEDevice>& Devices)
   {
   MosPrintf(MIL_TEXT("\nSetting-up GigE Vision devices and the Matrox ConcordPoE board.\n\n"));

   /* Set-up action context in the ConcordPoE */
   MIL_INT64 DeviceKey = 0x56781234, GroupKey = 0x24, GroupMask = 0xFFFFFFFF;
   MsysControl(MilConcordPoESystem, M_GC_ACTION0 + M_GC_ACTION_DEVICE_KEY, DeviceKey);
   MsysControl(MilConcordPoESystem, M_GC_ACTION0 + M_GC_ACTION_GROUP_KEY, GroupKey);
   MsysControl(MilConcordPoESystem, M_GC_ACTION0 + M_GC_ACTION_GROUP_MASK, GroupMask);
   MsysControl(MilConcordPoESystem, M_GC_ACTION0 + M_TRIGGER_SOURCE, M_TIMER1);

   /* Set-up action context in the GigE Vision devices */
   for (size_t i = 0; i < Devices.size(); i++)
      {
      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("ActionSelector"), M_TYPE_INT64, &Devices[i].ActionNumber);
      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("ActionDeviceKey"), M_TYPE_INT64, &DeviceKey);
      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("ActionGroupKey"), M_TYPE_INT64, &GroupKey);
      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("ActionGroupMask"), M_TYPE_INT64, &GroupMask);

      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("FrameStart"));
      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("TriggerSource"), M_TYPE_STRING, Devices[i].TriggerSource);
      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("On"));

      MsysControl(MilConcordPoESystem, M_GC_ACTION0 + M_ADD_DESTINATION, Devices[i].MilDigitizerId);
      }
   }

void SetupToEUsingSoftware(MIL_ID MilConcordPoESystem, const std::vector<ToEDevice>& Devices)
   {
   MosPrintf(MIL_TEXT("\nSetting-up GigE Vision devices and the Matrox ConcordPoE board.\n\n"));

   /* Set-up action context in the ConcordPoE */
   MsysControl(MilConcordPoESystem, M_GC_TRIGGER_SOFTWARE0 + M_TRIGGER_SOURCE, M_TIMER1);
   MsysControl(MilConcordPoESystem, M_GC_TRIGGER_SOFTWARE0 + M_GC_TRIGGER_SELECTOR, MIL_TEXT("FrameStart"));

   /* Set-up action context in the GigE Vision devices */
   for (size_t i = 0; i < Devices.size(); i++)
      {
      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("FrameStart"));
      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("TriggerSource"), M_TYPE_STRING, MIL_TEXT("Software"));
      MdigControlFeature(Devices[i].MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("On"));

      MsysControl(MilConcordPoESystem, M_GC_TRIGGER_SOFTWARE0 + M_ADD_DESTINATION, Devices[i].MilDigitizerId);
      }
   }

/* User's processing function called every time a grab buffer is ready. */
/* -------------------------------------------------------------------- */

/* Local defines. */
#define STRING_LENGTH_MAX  20
#define STRING_POS_X       20
#define STRING_POS_Y       20

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   ToEDevice &Device = *(ToEDevice*)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_DOUBLE TimeStamp = 0;
   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX] = { MIL_TEXT('\0'), };

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);
   MdigGetHookInfo(HookId, M_GC_CAMERA_TIME_STAMP, &TimeStamp);

   /* Increment the frame counter. */
   Device.ProcessedImageCount++;

   /* Draw the frame count (remove to reduce CPU usage). */
   MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%d"),
      (int)Device.ProcessedImageCount);
   MgraText(M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, Text);

   /* Update the display. */
   MbufCopy(ModifiedBufferId, Device.MilImageDisp);

   /* Calculate inter-frame jitter. */
   if (Device.TimeStamp != 0)
      {
      MIL_DOUBLE Delta = TimeStamp - Device.TimeStamp;
      if (Delta > Device.DeltaMax)
         {
         Device.DeltaMax = Delta;
         }

      if (Delta < Device.DeltaMin)
         {
         Device.DeltaMin = Delta;
         }
      }

   Device.TimeStamp = TimeStamp;

   return 0;
   }
