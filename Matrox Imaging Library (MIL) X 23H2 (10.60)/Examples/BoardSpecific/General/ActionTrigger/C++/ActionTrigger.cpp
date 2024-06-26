﻿/*************************************************************************/
/*
* File name: ActionTrigger.cpp
*
* Synopsis:  This example shows how to send a trigger to multiple cameras
*            at once using the GigE Vision® Action command.
*            The example then continues to show how to send a trigger to
*            multiple cameras at once using the GigE Vision® Scheduled
*            Action command. The Scheduled Action command relies on services
*            provided by the IEEE 1588 PTP (Precision Time Protocol).
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>
#if M_MIL_USE_WINDOWS
#include <windows.h>
#endif
#include <algorithm>
#include "ActionDevice.h"
#include "NetworkSegment.h"

#define BUFFERING_SIZE_MAX    10

/* Function prototypes */
void DoAction(MIL_ID MilSystem);
void DoScheduledAction(MIL_ID MilSystem);
void ProgramActionDevices(const std::vector<ActionDevice>& Devices, MIL_INT64 DeviceKey, MIL_INT64 GroupKey, MIL_INT64 GroupMask);
void ProgramMILActionContext(MIL_ID MilSystem, const std::vector<ActionDevice>& Devices, MIL_INT64 DeviceKey, MIL_INT64 GroupKey, MIL_INT64 GroupMask);
bool ControlPrecisionTimeProtocolClocks(MIL_ID MilSystem, std::vector<ActionDevice>& Devices, bool Enable);
void MFTYPE TriggerAction(TriggerFunctionArgument* Argument);
void ClearAction(MIL_ID MilSystem);
std::vector<MIL_DOUBLE> GetAcquisitionTimeMeasurments(MIL_INT BufferingSize, std::vector<ActionDevice>& Devices);

/* Main function. */
int MosMain(void)
   {
   MIL_ID   MilApplication;
   MIL_ID   MilSystem     ;
   MIL_INT  SystemType    ;

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, 
      M_NULL, M_NULL);

   /* Get information on the system we are using and print a welcome message to the console. */
   MsysInquire(MilSystem, M_SYSTEM_TYPE, &SystemType);

   if(SystemType != M_SYSTEM_GIGE_VISION_TYPE)
      {
      /* Print error message. */
      MosPrintf(MIL_TEXT("This example program can only be used with the Matrox Driver for ")
         MIL_TEXT("GigE Vision.\n"));
      MosPrintf(MIL_TEXT("Please ensure that the default system type is set accordingly in ")
         MIL_TEXT("MIL Config.\n"));
      MosPrintf(MIL_TEXT("-------------------------------------------------------------\n\n"));
      MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
      MosGetch();
      MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
      return 1;
      }

   /* Setup and trigger acquisition using the Action command. */
   DoAction(MilSystem);

#if M_MIL_USE_WINDOWS
   system("cls");
#endif

   /* Setup and trigger acquisition using the Scheduled Action command. */
   DoScheduledAction(MilSystem);

   /* Free defaults */
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

/* This routine detects GigE Vision compatible devices. It uses the Action command */
/* to simultaneously trigger the devices at once.                                  */
void DoAction(MIL_ID MilSystem)
   {
   std::vector<NetworkSegment> NetworkSegments;
   std::vector<ActionDevice> ActionDevices;

   MosPrintf(MIL_TEXT("This example shows how to trigger an action signal across\n"));
   MosPrintf(MIL_TEXT("multiple GigE Vision devices\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Detecting connected GigE Vision devices that support the Action command.\n\n"));
   /* Enumerate compatible devices. */
   EnumNetworkSegments(MilSystem, M_GC_ACTION_SUPPORT, NetworkSegments);

   if (NetworkSegments.size() == 0)
      {
      MosPrintf(MIL_TEXT("\nNo compatible devices found\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
      MosGetch();
      return;
      }

   /* Print info related to the devices found. */
   MosPrintf(MIL_TEXT("\nCompatible GigE Vision devices found are:\n\n"));
   for (std::vector<NetworkSegment>::size_type i = 0; i < NetworkSegments.size(); i++)
      NetworkSegments[i].Print(eDevices);

   /* Inform user that the detected GigE Vision devices are physically connected to      */
   /* different network segments. The Action signal packet will get replicated on these  */
   /* segments and will therefore reach the intended destination at different times.     */
   if (NetworkSegments.size() > 1)
      {
      MosPrintf(MIL_TEXT("\nSome of the detected GigE Vision devices reside on different network\n"));
      MosPrintf(MIL_TEXT("segments namely:\n\n"));
      for (std::vector<NetworkSegment>::size_type i = 0; i < NetworkSegments.size(); i++)
         NetworkSegments[i].Print(eAll);

      MosPrintf(MIL_TEXT("\nThe action signal packet will be replicated on these segments.\n"));
      MosPrintf(MIL_TEXT("Because of this the replicated action signal packet will arrive\n"));
      MosPrintf(MIL_TEXT("at different times on different network segments."));
      }

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Reformat data structure... */
   ActionDevices = ToActionDeviceVector(NetworkSegments);

   MosPrintf(MIL_TEXT("Programming action keys into compatible devices."));

   /* All devices that we want to trigger with this action must use the same device key */
   MIL_INT64 DeviceKey = 0x56781234;
   /* Devices can be subdivided into groups; we will use the same group for all devices. */
   MIL_INT64 GroupKey = 0x24;
   /* A device mask can be used to disable some group keys; we will enable all keys */
   MIL_INT64 GroupMask = 0xFFFFFFFF;
   /* For an action to get triggered the following conditions must be met: */
   /* 1- The device must be allocated with MdigAlloc(). */
   /* 2- The DeviceKey programmed here must match the DeviceKey sent by TriggerAction(). */
   /* 3- The GroupKey programmed here must match the GroupKey sent by TriggerAction(). */
   /* 4- The logical AND-wise comparison of the GroupMask programmed here with the GroupMask sent by TriggerAction() must be non-zero. */
   ProgramActionDevices(ActionDevices, DeviceKey, GroupKey, GroupMask);
   ProgramMILActionContext(MilSystem, ActionDevices, DeviceKey, GroupKey, 0x1);

   /* Set an artificial trigger delay for demo purposes. We do this in case         */
   /* the connected device's frame rates are not the same. This will avoid          */
   /* over-triggering some devices.                                                 */
   TriggerFunctionArgument Argument;
   Argument.MilSystem = MilSystem;
   Argument.ActionDelay = 100;

   /* Allocate resources required by MdigProcess for queuing grabs. */
   for (std::vector<ActionDevice>::size_type i = 0; i < ActionDevices.size(); i++)
      {
      ActionDevices[i].Allocate(BUFFERING_SIZE_MAX);

      if (i == 0)
         ActionDevices[i].StartAcquisition(BUFFERING_SIZE_MAX, TriggerAction, &Argument);
      else
         ActionDevices[i].StartAcquisition(BUFFERING_SIZE_MAX);
      }

   MosPrintf(MIL_TEXT("\nAction keys programming complete.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to trigger actions.\n\n"));
   MosGetch();
   /* Trigger the action. */
   TriggerAction(&Argument);
   MosPrintf(MIL_TEXT("The initial action signal was sent.\n\n"));

   /* Wait for acquisition to complete. */
   for (std::vector<ActionDevice>::size_type i = 0; i < ActionDevices.size(); i++)
      {
      ActionDevices[i].StopAcquisition(M_WAIT);
      MosPrintf(MIL_TEXT("%lld frames completed on %s %s (M_DEV%d)\n\n"), (long long)ActionDevices[i].ProcessedImageCount,
                ActionDevices[i].Vendor.c_str(), ActionDevices[i].Model.c_str(), ActionDevices[i].DigitizerNumber);
      }

   /* Here inter-device jitter measurements is calculated using the GigE Vision driver's time stamp.*/
   /* The time stamp is read from the CPU for each device at the M_GRAB_FRAME_START event.          */
   /* This event is influenced by interrupt moderation of the host NIC (if enabled) and by the      */
   /* fact that multiple device streaming data to the same host NIC will have their streams         */
   /* serialized by devices such as Ethernet switches.                                              */
   /* Also different settings used across devices (e.g. exposure time) can affect the jitter        */
   /* measurements.                                                                                 */
   MosPrintf(MIL_TEXT("Inter-device jitter measurements:\n"));
   MosPrintf(MIL_TEXT("Note: the measurements can be affected by a multitude of factors such as:\n"));
   MosPrintf(MIL_TEXT("1- The use of different exposure times across devices.\n"));
   MosPrintf(MIL_TEXT("2- The use interrupt moderation on the host Ethernet controller(s).\n\n"));

   /* Use acquisition timestamps gathered during acquisition to calculate inter-frame delays between */
   /* devices.                                                                                        */
   std::vector<MIL_DOUBLE> TimeMeasurments = GetAcquisitionTimeMeasurments(BUFFERING_SIZE_MAX, ActionDevices);
   for (std::vector<MIL_DOUBLE>::size_type i = 0; i < TimeMeasurments.size(); i++)
      MosPrintf(MIL_TEXT("Frame: %-3d%-4.3f (usec).\n"), i + 1, TimeMeasurments[i] * 1e6);

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Free resources. */
   for (std::vector<ActionDevice>::size_type i = 0; i < ActionDevices.size(); i++)
      ActionDevices[i].Free();

   ClearAction(MilSystem);
   }

/* This routine detects GigE Vision compatible devices. It uses the Scheduled Action command */
/* to simultaneously trigger the devices at once.                                            */
void DoScheduledAction(MIL_ID MilSystem)
   {
   MIL_UINT32 Selection = 0;
   std::vector<NetworkSegment> NetworkSegments;
   std::vector<ActionDevice> ActionDevices;

   MosPrintf(MIL_TEXT("This example can also show how to trigger a scheduled action signal across\n"));
   MosPrintf(MIL_TEXT("multiple GigE Vision devices.\n\n"));

   MosPrintf(MIL_TEXT("The scheduled action signal allows for up to sub-microsecond synchronization\n"));
   MosPrintf(MIL_TEXT("between devices. It requires IEEE 1588 Precision Time Protocol (PTP) support\n"));
   MosPrintf(MIL_TEXT("from your GigE Vision device.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Detecting network segments with compatible GigE Vision devices.\n\n"));
   /* Enumerate compatible devices. */
   EnumNetworkSegments(MilSystem, M_GC_SCHEDULED_ACTION_SUPPORT + M_GC_IEEE_1588_SUPPORT, NetworkSegments);

   if (NetworkSegments.size() == 0)
      {
      MosPrintf(MIL_TEXT("\nNo compatible devices found\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to quit.\n"));
      MosGetch();
      return;
      }

   /* Print info relative to the devices found. */
   MosPrintf(MIL_TEXT("\nCompatible GigE Vision devices found are:\n\n"));
   for (std::vector<NetworkSegment>::size_type i = 0; i < NetworkSegments.size(); i++)
      NetworkSegments[i].Print(eDevices);

   /* Inform user that the detected GigE Vision devices are physically connected to */
   /* different network segments. With IEEE 1588 PTP devices must reside on the same */
   /* PTP domain otherwise they will not be able to synchronize their clocks.       */
   if (NetworkSegments.size() > 1)
      {
      MosPrintf(MIL_TEXT("\nSome of the detected GigE Vision devices reside on different network\n"));
      MosPrintf(MIL_TEXT("segments namely:\n\n"));
      for (std::vector<NetworkSegment>::size_type i = 0; i < NetworkSegments.size(); i++)
         NetworkSegments[i].Print(eAll);

      MosPrintf(MIL_TEXT("\nIEEE 1588 PTP requires that devices reside on the same PTP domain for\n"));
      MosPrintf(MIL_TEXT("clock synchronization to occur. You must also ensure that the Ethernet\n"));
      MosPrintf(MIL_TEXT("bandwidth of all devices on a network segment does not exceed 125 MB/s\n"));
      MosPrintf(MIL_TEXT("(1 Gbps).\n\n"));

      MosPrintf(MIL_TEXT("Devices residing on different PTP domains cannot be triggered using\n"));
      MosPrintf(MIL_TEXT("the same action signal because they do not share a common clock.\n"));
      }

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* If there is more than 1 network segment with IEEE 1588 PTP compatible devices ask   */
   /* the use to select a single segment to use. This is because multiple segments might  */
   /* not be synchronized using the same PTP master clock.                                */
   if (NetworkSegments.size() > 1)
      {
      MosPrintf(MIL_TEXT("Which network segment do you wish to use?\n\n"));
      for (std::vector<NetworkSegment>::size_type i = 0; i < NetworkSegments.size(); i++)
         {
         MosPrintf(MIL_TEXT("%3d"), i + 1);
         NetworkSegments[i].Print(eHostController);
         }
      MosPrintf(MIL_TEXT("\n"));

      do
         {
#if M_MIL_USE_WINDOWS
         scanf_s("%d", &Selection);
#else
         scanf("%d", (int *)&Selection);
#endif
         if (Selection < 1 || Selection > NetworkSegments.size())
            MosPrintf(MIL_TEXT("Invalid selection\n"));
         else
            break;

         } while (1);

         /* Adjust selection to index. */
         Selection--;

         MosPrintf(MIL_TEXT("%s segment selected.\n\n"), NetworkSegments[Selection].HostControllerName.c_str());
      }

   // Reformat devices to a simpler 
   ActionDevices = ToActionDeviceVector(NetworkSegments, Selection);

   /* Enable IEEE1588 PTP on the selected devices. */
   if (ControlPrecisionTimeProtocolClocks(MilSystem, ActionDevices, true))
      {
      MosPrintf(MIL_TEXT("\nError: Precision Time Protocol initialization failed.\n"));
      MosGetch();
      return;
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Programming Action keys into compatible devices."));
   /* All devices that we want to trigger with this action must use the same device key */
   MIL_INT64 DeviceKey = 0x56781234;
   /* Devices can be subdivided into groups; we will use the same group for all devices. */
   MIL_INT64 GroupKey = 0x24;
   /* A device mask can be used to disable some group keys; we will enable all keys */
   MIL_INT64 GroupMask = 0xFFFFFFFF;
   /* For an action to get triggered the following conditions must be met: */
   /* 1- The device must be allocated with MdigAlloc(). */
   /* 2- The DeviceKey programmed here must match the DeviceKey sent by TriggerAction(). */
   /* 3- The GroupKey programmed here must match the GroupKey sent by TriggerAction(). */
   /* 4- The logical AND-wise comparison of the GroupMask programmed here with the GroupMask sent by TriggerAction() must be non-zero. */
   ProgramActionDevices(ActionDevices, DeviceKey, GroupKey, GroupMask);
   ProgramMILActionContext(MilSystem, ActionDevices, DeviceKey, GroupKey, 0x1);

   // Setup so scheduled action gets triggered every 333 ms.
   TriggerFunctionArgument Argument;
   Argument.MilSystem = MilSystem;

   for (std::vector<ActionDevice>::size_type i = 0; i < ActionDevices.size(); i++)
      {
      ActionDevices[i].Allocate(BUFFERING_SIZE_MAX);

      if (i == 0)
         ActionDevices[i].StartAcquisition(BUFFERING_SIZE_MAX, TriggerAction, &Argument);
      else
         ActionDevices[i].StartAcquisition(BUFFERING_SIZE_MAX);
      }

   MosPrintf(MIL_TEXT("\nAction keys programming complete.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to trigger an action.\n\n"));
   MosGetch();
   
   /* Trigger the 1st action sometime in the future; say in 0.5 second from now. */
   /* The other action signals will get triggered from the MdigProcess hook callback of the first Device in the NetworkSegment. */
   Argument.PtpDueTime = ActionDevices[0].GetDeviceTicks() + 0.5;
   TriggerAction(&Argument);
   MosPrintf(MIL_TEXT("The action signal was sent.\n\n"));

   for (std::vector<ActionDevice>::size_type i = 0; i < ActionDevices.size(); i++)
      {
      ActionDevices[i].StopAcquisition(M_WAIT);
      MosPrintf(MIL_TEXT("%lld frames completed on %s %s (M_DEV%d)\n\n"), (long long)ActionDevices[i].ProcessedImageCount,
                ActionDevices[i].Vendor.c_str(), ActionDevices[i].Model.c_str(), ActionDevices[i].DigitizerNumber);
      }

   /* Here inter-device jitter measurements is calculated using the GigE Vision device time stamp.  */
   /* The time stamp generated by the GigE Vision device represents the time when the image was     */ 
   /* generated. With IEEE 1588 PTP enabled, the timestamps of all devices residing on the same PTP */
   /* domain are synchronized. The synchronization precision achievable is dependent upon multiple  */
   /* factors such as:                                                                              */
   /* 1- Network topology                                                                           */
   /* 2- The use of specialized devices (e.g. Ethernet switch) that uses special PTP clocks such as:*/
   /*    - Transparent clocks                                                                       */
   /*    - Boundary clocks                                                                          */
   /* Note that specialized devices are not required to use IEEE 1588 PTP                           */

   MosPrintf(MIL_TEXT("Inter-device jitter measurements:\n\n"));
   MosPrintf(MIL_TEXT("Note: the measurements can be affected by multiple factors such as:\n"));
   MosPrintf(MIL_TEXT("1- The use of different exposure times across devices.\n"));
   MosPrintf(MIL_TEXT("2- Network topology.\n"));
   MosPrintf(MIL_TEXT("3- The use (or lack of) IEEE1588 transparent clocks and/or IEEE1588 boundary clocks.\n\n"));
   std::vector<MIL_DOUBLE> TimeMeasurments = GetAcquisitionTimeMeasurments(BUFFERING_SIZE_MAX, ActionDevices);
   for (std::vector<MIL_DOUBLE>::size_type i = 0; i < TimeMeasurments.size(); i++)
      MosPrintf(MIL_TEXT("Frame: %-3d%-4.3f (usec).\n"), i + 1, TimeMeasurments[i] * 1e6);

   /* Disable IEEE1588 PTP on the selected devices. */
   ControlPrecisionTimeProtocolClocks(MilSystem, ActionDevices, false);

   MosPrintf(MIL_TEXT("Press <Enter> to Quit.\n"));
   MosGetch();

   for (std::vector<ActionDevice>::size_type i = 0; i < ActionDevices.size(); i++)
      ActionDevices[i].Free();

   for (std::vector<NetworkSegment>::size_type i = 0; i < NetworkSegments.size(); i++)
      {
      for (std::vector<ActionDevice>::size_type j = 0; j < NetworkSegments[i].ActionDevices.size(); j++)
         NetworkSegments[i].ActionDevices[j].Free();
      }

   ClearAction(MilSystem);
   }

/* This routine is used to enable or disable IEEE 1588 PTP on GigE Vision devices. */
bool ControlPrecisionTimeProtocolClocks(MIL_ID MilSystem, std::vector<ActionDevice>& Devices, bool Enable)
   {
   bool Error = false;
   MIL_BOOL EnablePtp = Enable ? M_TRUE : M_FALSE;
   MIL_INT ReadyCount = 0, Count = 0;
   std::vector<MIL_STRING> Status(Devices.size(), MIL_TEXT(""));

   if (Enable)
      {
      MosPrintf(MIL_TEXT("Enabling IEEE 1588 Precision Time Protocol on GigE Vision devices\n\n"));

      for (std::vector<ActionDevice>::size_type i = 0; i < Devices.size(); i++)
         MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, Devices[i].PtpEnableName, M_TYPE_BOOLEAN, &EnablePtp);

      MosPrintf(MIL_TEXT("Waiting for GigE Vision Precision Time Protocol enabled devices to report\n"));
      MosPrintf(MIL_TEXT("readiness.\n\n"));

      do
         {
         ReadyCount = 0;
         MosSleep(50);
         Count++;
         MosPrintf(MIL_TEXT("."));

         for (std::vector<ActionDevice>::size_type i = 0; i < Devices.size(); i++)
            {
            MIL_BOOL IsPresent = M_FALSE;
            MdigInquireFeature(Devices[i].MilDigitizer, M_FEATURE_PRESENT, Devices[i].PtpDataSetLatchName, M_TYPE_BOOLEAN, &IsPresent);
            if (IsPresent)
               MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_EXECUTE, Devices[i].PtpDataSetLatchName, M_DEFAULT, M_NULL);

            MdigInquireFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, Devices[i].PtpStatusName, M_TYPE_STRING, Status[i]);

            if (Status[i] == MIL_TEXT("Master") || Status[i] == MIL_TEXT("Slave"))
               {
               ReadyCount++;
               Devices[i].IEEE1588Enabled = true;
               }
            }

         if (Count > 500)
            break;

         } while (ReadyCount < (MIL_INT)Devices.size());

      // An additional delay is required for the clock synchronization process to converge to stable values.
      MIL_INT Counter = 200;
      while (Counter--)
         {
         MosPrintf(MIL_TEXT("."));
         MosSleep(50);
         }

      MosPrintf(MIL_TEXT("\n\nIEEE 1588 Precision Time Protocol report:\n\n"));
      if (ReadyCount < (MIL_INT)Devices.size())
         {
         Error = true;

         for (std::vector<ActionDevice>::size_type i = 0; i < Devices.size(); i++)
            {
            MosPrintf(MIL_TEXT("\t%s %s (M_DEV%d)\n\tStatus: %s\n\n"), Devices[i].Vendor.c_str(),
                     Devices[i].Model.c_str(), Devices[i].DigitizerNumber, Status[i].c_str());
            }

         MosPrintf(MIL_TEXT("Not all device's status report ready."));
         }
      else
         {
         MIL_STRING Accuracy = MIL_TEXT("Unknown");
         for (std::vector<ActionDevice>::size_type i = 0; i < Devices.size(); i++)
            {
            MIL_BOOL IsPresent = M_FALSE;
            MdigInquireFeature(Devices[i].MilDigitizer, M_FEATURE_PRESENT, Devices[i].PtpClockAccuracyName, M_TYPE_BOOLEAN, &IsPresent);
            if(IsPresent)
               MdigInquireFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, Devices[i].PtpClockAccuracyName, M_TYPE_STRING, Accuracy);

            MosPrintf(MIL_TEXT("\t%s %s (M_DEV%d)\n\tStatus: %-12s Clock Accuracy: %-12s\n\n"), Devices[i].Vendor.c_str(),
                        Devices[i].Model.c_str(), Devices[i].DigitizerNumber, Status[i].c_str(), Accuracy.c_str());
            }
         }
      }
   else
      {
      MosPrintf(MIL_TEXT("\nDisabling IEEE 1588 Precision Time Protocol on GigE Vision devices\n\n"));

      for (std::vector<ActionDevice>::size_type i = 0; i < Devices.size(); i++)
         MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, Devices[i].PtpEnableName, M_TYPE_BOOLEAN, &EnablePtp);
      }

   return Error;
   }

/* This routine is used to program Action Keys and Masks into GigE Vision devices. These keys and masks are used  */
/* by the devices when they receive the Action command.                                                           */
/* For an action to get triggered the following conditions must be met:                                           */
/* 1- The device must be allocated with MdigAlloc().                                                              */
/* 2- The DeviceKey programmed here must match the DeviceKey sent by TriggerAction().                             */
/* 3- The GroupKey programmed here must match the GroupKey sent by TriggerAction().                               */
/* 4- The logical AND-wise comparison of the GroupMask programmed here with the GroupMask sent by TriggerAction() */
/*    must be non-zero.                                                                                           */
void ProgramActionDevices(const std::vector<ActionDevice>& Devices, MIL_INT64 DeviceKey, MIL_INT64 GroupKey, MIL_INT64 GroupMask)
   {
   for (std::vector<ActionDevice>::size_type i = 0; i < Devices.size(); i++)
      {
      /* Setup action keys and masks on each device. */
      MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("ActionDeviceKey"), M_TYPE_INT64, &DeviceKey);
      MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("ActionSelector"), M_TYPE_INT64, &Devices[i].ActionNumber);
      MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("ActionGroupKey"), M_TYPE_INT64, &GroupKey);
      MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("ActionGroupMask"), M_TYPE_INT64, &GroupMask);
      
      /* Setup each device to trigger upon reception of an action signal. */
      MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("FrameStart"));
      MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("Off"));
      MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSource"), M_TYPE_STRING, Devices[i].ActionName);
      MdigControlFeature(Devices[i].MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerMode"), M_TYPE_STRING, MIL_TEXT("On"));
      }
   }

/* Setup a software context (M_GC_ACTION0 in this case) and store in it the action keys and masks that will be sent */
/* when MsysControl(MilSystem, M_GC_ACTION0 + M_GC_ACTION_EXECUTE, M_DEFAULT) is called.                            */
void ProgramMILActionContext(MIL_ID MilSystem, const std::vector<ActionDevice>& Devices, MIL_INT64 DeviceKey, MIL_INT64 GroupKey, MIL_INT64 GroupMask)
   {
   /* Use MIL M_GC_ACTION0 context to store the information */
   /* Other contexts can be used to store other key groups. */
   MsysControl(MilSystem, M_GC_ACTION0 + M_GC_ACTION_DEVICE_KEY, DeviceKey);
   MsysControl(MilSystem, M_GC_ACTION0 + M_GC_ACTION_GROUP_KEY, GroupKey);
   MsysControl(MilSystem, M_GC_ACTION0 + M_GC_ACTION_GROUP_MASK, GroupMask);
   
   for (std::vector<ActionDevice>::size_type i = 0; i < Devices.size(); i++)
      MsysControl(MilSystem, M_GC_ACTION0 + M_GC_ACTION_ADD_DEVICE, Devices[i].MilDigitizer);
   }

/* Sends an Action, or a Scheduled Action command. If M_GC_ACTION_TIME is non zero then a Scheduled Action is sent. */
void MFTYPE TriggerAction(TriggerFunctionArgument* Argument)
   {
   if (Argument->PtpDueTime != 0.0)
      MsysControl(Argument->MilSystem, M_GC_ACTION0 + M_GC_ACTION_TIME, Argument->PtpDueTime);

   MsysControl(Argument->MilSystem, M_GC_ACTION0 + M_GC_ACTION_EXECUTE, M_DEFAULT);
   }

/* Removes devices associated to this action context. */
void ClearAction(MIL_ID MilSystem)
   {
   MsysControl(MilSystem, M_GC_ACTION0 + M_GC_ACTION_CLEAR_DEVICES, M_DEFAULT);
   }

/* Utility routine that computes the maximum jitter measurements across all devices. */
std::vector<MIL_DOUBLE> GetAcquisitionTimeMeasurments(MIL_INT BufferingSize, std::vector<ActionDevice>& Devices)
   {
   std::vector<MIL_DOUBLE> Jitter(BufferingSize, 0.0);
   MIL_DOUBLE Diff = 0.0;

   for (std::vector<ActionDevice>::size_type i = 0; i < Devices.size(); i++)
      {
      for (std::vector<ActionDevice>::size_type j = i; j < Devices.size(); j++)
         {
         if (i == j)
            continue;

         for (MIL_INT k = 0; k < BufferingSize; k++)
            {
            Diff = fabs(Devices[i].DeviceTimeStamps[k] - Devices[j].DeviceTimeStamps[k]);

            if (Jitter[k] < Diff)
               Jitter[k] = Diff;
            }
         }
      }

   return Jitter;
   }
