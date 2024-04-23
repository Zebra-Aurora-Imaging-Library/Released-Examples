/*************************************************************************************/
/*
 * File name: TriggerOverEthernet.h
 *
 * Synopsis:  This program shows the use of the Matrox ConcordPoE's trigger over Ethernet
 *            offload feature to trigger GigE Vision devices using the Action command.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include <mil.h>
#include <list>
#include <vector>
#include <cmath>
#define BUFFERING_SIZE_MAX 100

/* Contains resources required to grab and display images from a GigE Vision device. */
struct ToEDevice
   {
   /* Constructor */
   ToEDevice()
      {
      MilToESystemId = M_NULL;
      MilDigitizerId = M_NULL;
      MilDisplayId = M_NULL;
      MilImageDisp = M_NULL;
      MacAddress = 0;
      ProcessedImageCount = 0;
      TimeStamp = 0;
      DeltaMin = 1e9;
      DeltaMax = 0;

      for (size_t i = 0; i < BUFFERING_SIZE_MAX; i++)
         GrabBufList[i] = M_NULL;
      }

   /* Allocates a display and grab buffers. */
   void Allocate()
      {
      MIL_ID MilSystem = MdigInquire(MilDigitizerId, M_OWNER_SYSTEM, M_NULL);
      MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplayId);
      MbufAllocColor(MilSystem,
                     MdigInquire(MilDigitizerId, M_SIZE_BAND, M_NULL),
                     MdigInquire(MilDigitizerId, M_SIZE_X, M_NULL),
                     MdigInquire(MilDigitizerId, M_SIZE_Y, M_NULL),
                     8 + M_UNSIGNED,
                     M_IMAGE + M_DISP + M_GRAB + M_PROC,
                     &MilImageDisp);
      MbufClear(MilImageDisp, M_COLOR_BLACK);
      MdispSelect(MilDisplayId, MilImageDisp);

      for (size_t i = 0; i < BUFFERING_SIZE_MAX; i++)
         {
         MbufAllocColor(MilSystem,
                        MdigInquire(MilDigitizerId, M_SIZE_BAND, M_NULL),
                        MdigInquire(MilDigitizerId, M_SIZE_X, M_NULL),
                        MdigInquire(MilDigitizerId, M_SIZE_Y, M_NULL),
                        8 + M_UNSIGNED,
                        M_IMAGE + M_GRAB + M_PROC,
                        &GrabBufList[i]);
         }
      }

   /* Frees the display and the grab buffers. */
   void Free()
      {
      for (size_t i = 0; i < BUFFERING_SIZE_MAX; i++)
         {
         if (GrabBufList[i] != M_NULL)
            {
            MbufFree(GrabBufList[i]);
            }
         }

      if (MilImageDisp != M_NULL)
         {
         MbufFree(MilImageDisp);
         }

      if (MilDisplayId)
         {
         MdispFree(MilDisplayId);
         }

      if (MilDigitizerId)
         {
         MdigFree(MilDigitizerId);
         }
      }

   /* Finds the name of the TriggerSelector's Action value.       */
   /* To be used later when setting the device in triggered mode. */
   void GetActionSelectorInfo()
      {
      MdigControlFeature(MilDigitizerId, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("FrameStart"));

      /* Inquire the number of enumeration entries under the TriggerSource feature. */
      MIL_INT Count = 0;
      MdigInquireFeature(MilDigitizerId, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("TriggerSource"), M_TYPE_MIL_INT, &Count);
      for (MIL_INT i = 0; i < Count; i++)
         {
         /* Inquire the Nth enumeration entry name. */
         MIL_STRING SourceName;
         MdigInquireFeature(MilDigitizerId, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("TriggerSource"), M_TYPE_STRING, SourceName);
         /* If the enumeration entry is of the form ActionN, where N is a number, we are done. */
         if (SourceName.find(MIL_TEXT("Action")) != MIL_STRING::npos)
            {
            TriggerSource = SourceName;
            MdigInquireFeature(MilDigitizerId, M_FEATURE_MIN, MIL_TEXT("ActionSelector"), M_TYPE_INT64, &ActionNumber);
            break;
            }
         }
      }

   MIL_ID MilToESystemId;
   MIL_ID MilDigitizerId;
   MIL_ID MilDisplayId;
   MIL_ID MilImageDisp;
   MIL_ID GrabBufList[BUFFERING_SIZE_MAX];
   MIL_INT64 MacAddress;
   MIL_INT ProcessedImageCount;
   MIL_STRING Vendor;
   MIL_STRING Model;
   MIL_STRING TriggerSource;
   MIL_INT64 ActionNumber;
   MIL_DOUBLE TimeStamp;
   MIL_DOUBLE DeltaMin;
   MIL_DOUBLE DeltaMax;
   };

/* Finds GigE Vision devices that supports the action command and is connected to */
/* the Matrox Concord PoE board.                                                  */
/* The action command is a special packet sent to trigger an action in a GigE     */
/* Vision device.                                                                 */
std::vector<ToEDevice> FindToEDevices(MIL_ID MilConcordPoESystem, MIL_ID MilGigESystem, bool ActionSupportRequired)
   {
   std::vector<ToEDevice> Devices, TmpDevices;

   if (MilConcordPoESystem == M_NULL)
      return Devices;

   MappControl(M_ERROR, M_PRINT_DISABLE);

   /* Find all GigE Vision devices that supports the action command. */
   MIL_INT DeviceCount = 0;
   MsysInquire(MilGigESystem, M_DISCOVER_DEVICE_COUNT, &DeviceCount);
   for (MIL_INT DevNb = M_DEV0; DevNb < DeviceCount; DevNb++)
      {
      ToEDevice Device;
      MdigAlloc(MilGigESystem, DevNb, MIL_TEXT("gigevision_currentstate_continuous.dcf"), M_DEFAULT, &Device.MilDigitizerId);
      if (Device.MilDigitizerId != M_NULL)
         {
         /* Test for action support*/
         MIL_INT Capability = 0;
         MdigInquire(Device.MilDigitizerId, M_GC_CONTROL_PROTOCOL_CAPABILITY, &Capability);
         if (!ActionSupportRequired || (Capability & M_GC_ACTION_SUPPORT))
            {
            /* Inquire the host MAC address associated to the GigE Vision device. */
            MdigInquire(Device.MilDigitizerId, M_GC_LOCAL_MAC_ADDRESS, &Device.MacAddress);
            /* Inquire vendor and model names of the device. */
            MdigInquire(Device.MilDigitizerId, M_CAMERA_VENDOR, Device.Vendor);
            MdigInquire(Device.MilDigitizerId, M_CAMERA_MODEL, Device.Model);
            /* Make sure all devices use the same exposure time otherwise jitter measurements will be off.  */
            MdigControl(Device.MilDigitizerId, M_EXPOSURE_TIME, 1000000.0);  //1ms
            TmpDevices.push_back(Device);
            }
         else
            {
            /* Reject devices that do not support actions. */
            MdigFree(Device.MilDigitizerId);
            }
         }
      }

   /* Inquire the number of Ethernet ports on the Matrox Concord PoE. */
   MIL_INT PortCount = 0;
   MsysInquire(MilConcordPoESystem, M_GC_NIC_PORT_COUNT, &PortCount);
   for (MIL_INT j = 0; j < PortCount; j++)
      {
      /* For each port inquire its MAC address. */
      MIL_INT64 MacAddress = 0;
      MsysInquire(MilConcordPoESystem, M_GC_LOCAL_MAC_ADDRESS + j, &MacAddress);

      /* Find if a GigE Vision device is connected to this Ethernet port. */
      for(size_t i = 0; i < TmpDevices.size(); i++)
         {
         if (TmpDevices[i].MacAddress == MacAddress)
            {
            TmpDevices[i].MilToESystemId = MilConcordPoESystem;
            }
         }
      }

   MappControl(M_ERROR, M_PRINT_ENABLE);

   /* Remove all devices that are not connected to the Matrox Concord PoE. */
   for (size_t i = 0; i < TmpDevices.size(); i++)
      {
      if (TmpDevices[i].MilToESystemId != M_NULL)
         {
         Devices.push_back(TmpDevices[i]);
         }
      else
         {
         TmpDevices[i].Free();
         }
      }

   return Devices;
   }

/* Utility routine that computes the inter-frame jitter measurements on all devices. */
void PrintInterfameJitter(std::vector<ToEDevice>& Devices, MIL_DOUBLE ExpectedFrequency)
   {
   MIL_DOUBLE Period = 1.0 / ExpectedFrequency;
   for (size_t i = 0; i < Devices.size(); i++)
      {
      MIL_DOUBLE Val1 = fabs(Devices[i].DeltaMin - Period) * 1e9;
      MIL_DOUBLE Val2 = fabs(Devices[i].DeltaMax - Period) * 1e9;
      if (Val1 < Val2)
         {
         MosPrintf(MIL_TEXT("%.2d\t%s %s: Min: %.1f Max: %.1f\n"), (int)i, Devices[i].Vendor.c_str(), Devices[i].Model.c_str(), Val1, Val2);
         }
      else
         {
         MosPrintf(MIL_TEXT("%.2d\t%s %s: Min: %.1f Max: %.1f\n"), (int)i, Devices[i].Vendor.c_str(), Devices[i].Model.c_str(), Val2, Val1);
         }
      }
   }
