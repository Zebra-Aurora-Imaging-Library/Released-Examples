/*************************************************************************/
/*
* File name: NetworkSegment.h
*
* Synopsis:  Utility class used to enumerate and control GigE Vision devices
*            that support specific capabilities grouped under the same
/            network segment.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#ifndef __NETWORK_SEGMENT_H__
#define __NETWORK_SEGMENT_H__

#include <vector>
#include <math.h>
#include "ActionDevice.h"

enum PrintMode { eHostController, eDevices, eAll };

/* Represents a network segment comprising the host Ethernet controller and all of its attached GigE Vision devices. */
class NetworkSegment
   {
   public:
      inline bool operator==(const NetworkSegment& Right)
         {
         return ((HostControllerName == Right.HostControllerName) && (HostControlerAddress == Right.HostControlerAddress));
         }

      inline void Free()
         {
         for (std::vector<ActionDevice>::size_type i = 0; i < ActionDevices.size(); i++)
            ActionDevices[i].Free();
         }

      void Print(PrintMode Mode = eAll)
         {
         MIL_STRING Format = MIL_TEXT("\t%s %s (M_DEV%d)\n");
         if (Mode == eAll)
            Format = MIL_TEXT("\t\t%s %s (M_DEV%d)\n");

         if (Mode == eHostController || Mode == eAll)
            MosPrintf(MIL_TEXT("\t%s\n"), HostControllerName.c_str());

         if (Mode == eDevices || Mode == eAll)
            {
            for (std::vector<ActionDevice>::size_type i = 0; i < ActionDevices.size(); i++)
               MosPrintf(Format.c_str(), ActionDevices[i].Vendor.c_str(), ActionDevices[i].Model.c_str(), ActionDevices[i].DigitizerNumber);
            }
         }

      MIL_STRING HostControllerName;
      MIL_STRING HostControlerAddress;
      std::vector<ActionDevice> ActionDevices;
   };

bool DeviceUsesLegacyPtpNames(MIL_ID MilDigitizer)
   {
   MIL_BOOL UseLegacyNames = M_FALSE;
   MdigInquireFeature(MilDigitizer, M_FEATURE_PRESENT, MIL_TEXT("GevIEEE1588"), M_TYPE_BOOLEAN, &UseLegacyNames);
   return UseLegacyNames;
   }

/* Routine used to enumerate compatible devices. */
void EnumNetworkSegments(MIL_ID MilSystem, MIL_ID CompatibilityBits, std::vector<NetworkSegment>& CompatibleNetworks)
   {
   MIL_INT DeviceCount = 0, DeviceCapability = 0, CompatibleCount = 0, IncompatibleCount = 0;
   MIL_ID MilDigitizer;
   std::vector<ActionDevice> Devices;

   MsysInquire(MilSystem, M_DIGITIZER_NUM, &DeviceCount);
   MosPrintf(MIL_TEXT("%d GigE Vision devices found.\n"), (int)DeviceCount);

   for (MIL_INT i = 0; i < DeviceCount; i++)
      {
      /* Allocate digitizer to inquire the device's capabilities. */
      MdigAlloc(MilSystem, M_DEV + i, MIL_TEXT("gigevision_currentstate_continuous.dcf"),
                M_DEV_NUMBER, &MilDigitizer);

      if (MilDigitizer != M_NULL)
         {
         MIL_INT64 ActionNumber = -1;
         MIL_STRING ActionName;
         /* Inquire GigE Vision device capabilities for action command. */
         MdigInquire(MilDigitizer, M_GC_CONTROL_PROTOCOL_CAPABILITY, &DeviceCapability);
         /* Inquire GigE Vision features for the required action XML features. */
         if ((DeviceCapability & CompatibilityBits) == CompatibilityBits)
            {
            /* This device supports the appropriate capabilities. */
            GetDeviceAction(MilDigitizer, ActionName, ActionNumber);
            if (!ActionName.empty())
               {
               /* Required features are present. Keep this device. */
               ActionDevice Device;
               Device.MilDigitizer = MilDigitizer;
               MdigInquire(MilDigitizer, M_CAMERA_VENDOR, Device.Vendor);
               MdigInquire(MilDigitizer, M_CAMERA_MODEL, Device.Model);
               MdigInquire(MilDigitizer, M_GC_REMOTE_IP_ADDRESS_STRING, Device.IP);
               Device.DigitizerNumber = M_DEV + i;
               Device.ActionName = ActionName;
               Device.ActionNumber = ActionNumber;

               if ((CompatibilityBits & (M_GC_SCHEDULED_ACTION_SUPPORT + M_GC_IEEE_1588_SUPPORT)) == (M_GC_SCHEDULED_ACTION_SUPPORT + M_GC_IEEE_1588_SUPPORT))
                  {
                  // Does this device use deprecated PTP names?
                  if (DeviceUsesLegacyPtpNames(MilDigitizer))
                     {
                     Device.PtpEnableName = MIL_TEXT("GevIEEE1588");
                     Device.PtpDataSetLatchName = MIL_TEXT("GevIEEE1588DataSetLatch");
                     Device.PtpStatusName = MIL_TEXT("GevIEEE1588Status");
                     Device.PtpClockAccuracyName = MIL_TEXT("GevIEEE1588ClockAccuracy");
                     }
                  }

               Devices.push_back(Device);
               CompatibleCount++;
               }
            else
               {
               /* This device does not support the required XML features. Free it. */
               MdigFree(MilDigitizer);
               IncompatibleCount++;
               }
            }
         else
            {
            /* This device does not support the required capabilities. Free it. */
            MdigFree(MilDigitizer);
            IncompatibleCount++;
            }

         MosPrintf(MIL_TEXT("Found %d compatible device(s) and %d incompatible device(s).\r"), (int)CompatibleCount, (int)IncompatibleCount);
         }
      }

   /* Sort each compatible device found according to it's owner host interface (NIC). */
   for (std::vector<ActionDevice>::size_type i = 0; i < Devices.size(); i++)
      {
         {
         NetworkSegment Segment;
         MdigInquire(Devices[i].MilDigitizer, M_GC_INTERFACE_NAME, Segment.HostControllerName);
         MdigInquire(Devices[i].MilDigitizer, M_GC_LOCAL_IP_ADDRESS_STRING, Segment.HostControlerAddress);

         /* Find if interface name has already been enumerated. */
         std::vector<NetworkSegment>::iterator it = find(CompatibleNetworks.begin(), CompatibleNetworks.end(), Segment);
         if (it == CompatibleNetworks.end())
            {
            /* Add device to network and add network to GigeNetworks. */
            Segment.ActionDevices.push_back(Devices[i]);
            CompatibleNetworks.push_back(Segment);
            }
         else
            {
            /* Network already in list, simply add device. */
            (*it).ActionDevices.push_back(Devices[i]);
            }
         }
      }

   MosPrintf(MIL_TEXT("\n"));
   }

/* Convert devices into a simple form (From std::vector of network segments containing devices to std::vector of devices. */
std::vector<ActionDevice> ToActionDeviceVector(std::vector<NetworkSegment>& Segments, MIL_INT SegmentNumber = -1)
   {
   std::vector<ActionDevice> Devices;

   if (SegmentNumber == -1)
      {
      for (std::vector<NetworkSegment>::size_type i = 0; i < Segments.size(); i++)
         {
         for (std::vector<ActionDevice>::size_type j = 0; j < Segments[i].ActionDevices.size(); j++)
            {
            Devices.push_back(Segments[i].ActionDevices[j]);
            Segments[i].ActionDevices[j].MilDigitizer = M_NULL;
            }
         }
      }
   else if (SegmentNumber >= 0 && SegmentNumber < (MIL_INT)Segments.size())
      {
      for (std::vector<ActionDevice>::size_type j = 0; j < Segments[SegmentNumber].ActionDevices.size(); j++)
         {
         Devices.push_back(Segments[SegmentNumber].ActionDevices[j]);
         Segments[SegmentNumber].ActionDevices[j].MilDigitizer = M_NULL;
         }
      }

   return Devices;
   }

#endif
