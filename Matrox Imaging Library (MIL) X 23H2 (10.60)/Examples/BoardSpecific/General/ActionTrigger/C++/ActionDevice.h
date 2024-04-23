/*************************************************************************/
/*
* File name: ActionDevice.h
*
* Synopsis:  Utility classes used to enumerate and control GigE Vision devices
*            that support specific action capabilities.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#ifndef __ACTION_DEVICE_H__
#define __ACTION_DEVICE_H__

#include <vector>
#include <mil.h>

/* Argument passed to trigger function. */
class TriggerFunctionArgument
   {
   public:
      TriggerFunctionArgument()
         {
         MilSystem = M_NULL;
         PtpDueTime = 0.0;
         ActionDelay = 0;
         }

      MIL_ID MilSystem;
      MIL_DOUBLE PtpDueTime;
      MIL_INT ActionDelay;
   };

/* Action Trigger function prototype. */
typedef void(MFTYPE* TRIGGER_FUNCTION_PTR)(TriggerFunctionArgument* Argument);

/* Represents a GigE Vision device that supports Action or Scheduled Action command. */
class ActionDevice
   {
   public:
      ActionDevice()
         : PtpEnableName(MIL_TEXT("PtpEnable"))
         , PtpDataSetLatchName(MIL_TEXT("PtpDataSetLatch"))
         , PtpStatusName(MIL_TEXT("PtpStatus"))
         , PtpClockAccuracyName(MIL_TEXT("PtpClockAccuracy"))
         {
         IEEE1588Enabled = false;
         TriggerFunction = NULL;
         TriggerArguments = NULL;
         ProcessedImageCount = 0;
         DigitizerNumber = 0;
         MilDigitizer = M_NULL;
         MilDisplay = M_NULL;
         MilImageDisp = M_NULL;
         MilImageListSize = 0;
         MilImages = NULL;
         }

      /* Free any resources associated to this device. */
      void Free()
         {
         if (MilDigitizer != M_NULL)
            MdigFree(MilDigitizer);

         if (MilDisplay != M_NULL)
            MdispFree(MilDisplay);

         if (MilImageDisp != M_NULL)
            MbufFree(MilImageDisp);

         for (MIL_INT i = 0; i < MilImageListSize; i++)
            {
            if (MilImages[i] != M_NULL)
               MbufFree(MilImages[i]);
            }

         delete[] MilImages;

         MilDigitizer = M_NULL;
         MilDisplay = M_NULL;
         MilImageDisp = M_NULL;
         MilImages = NULL;
         }

      /* Allocate resources required by this device to acquire images. */
      void Allocate(MIL_INT BufferingSize)
         {
         MIL_ID MilSystem = M_NULL;
         MIL_INT SizeBand = 0;
         MIL_INT BufType = 0;
         MIL_INT64 BufFormat = 0;
         MIL_STRING_STREAM Title;

         Title << Model << MIL_TEXT(" (M_DEV") << DigitizerNumber << MIL_TEXT(")");

         MdigInquire(MilDigitizer, M_OWNER_SYSTEM, &MilSystem);

         MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT,
                    &MilDisplay);
         MdispControl(MilDisplay, M_TITLE, Title.str());

         /* Allocate a buffer format that matches the camera's pixel format. */
         MdigInquire(MilDigitizer, M_SIZE_BAND, &SizeBand);
         MdigInquire(MilDigitizer, M_TYPE, &BufType);
         MdigInquire(MilDigitizer, M_SOURCE_DATA_FORMAT, &BufFormat);

         MbufAllocColor(MilSystem,
                        SizeBand,
                        MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
                        MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
                        BufType,
                        M_IMAGE + M_GRAB + M_DISP + BufFormat,
                        &MilImageDisp);
         if (BufType != 8 + M_UNSIGNED)
            {
            MdispControl(MilDisplay, M_VIEW_MODE, M_BIT_SHIFT);
            MdispControl(MilDisplay, M_VIEW_BIT_SHIFT, BufType - 8);
            }

         MbufClear(MilImageDisp, M_COLOR_BLACK);
         MdispSelect(MilDisplay, MilImageDisp);

         MilImages = new MIL_ID [BufferingSize];
         for (MIL_INT i = 0; i < BufferingSize; i++)
            {
            MbufAllocColor(MilSystem,
                           SizeBand,
                           MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
                           MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
                           BufType,
                           M_GRAB + M_IMAGE + BufFormat,
                           &MilImages[i]);
            if (MilImages[i])
               {
               MilImageListSize++;
               MbufClear(MilImages[i], 0);
               }
            }
         }

      /* Returns the current raw device time (in ticks). */
      MIL_DOUBLE GetDeviceTicks()
         {
         MIL_DOUBLE TimeStamp = 0;
         
         MdigInquire(MilDigitizer, M_GC_CAMERA_TIME_STAMP, &TimeStamp);

         return TimeStamp;
         }

      /* Queues grab buffers for acquisition. */
      void StartAcquisition(MIL_INT Count, TRIGGER_FUNCTION_PTR FunctionPtr = NULL, TriggerFunctionArgument* Argument = NULL)
         {
         ProcessedImageCount = 0;
         TriggerFunction = FunctionPtr;
         TriggerArguments = Argument;

         if (IEEE1588Enabled == false)
            MdigHookFunction(MilDigitizer, M_GRAB_FRAME_START, ActionDevice::GrabFrameStart, this);

         /* Make sure all devices use the same exposure time otherwise jitter measurements will be off.  */
         MdigControl(MilDigitizer, M_EXPOSURE_TIME, 1000000.0);  //1ms

         MdigProcess(MilDigitizer, MilImages,
                     MilImageListSize, M_SEQUENCE+M_COUNT(Count), M_ASYNCHRONOUS,
                     ActionDevice::ProcessingFunction, this);
         }

      /* Stops acquisition and optionally wait for any pending grabs to complete. */
      void StopAcquisition(MIL_INT WaitFlag = 0)
         {
         MdigProcess(MilDigitizer, MilImages,
                     MilImageListSize, M_STOP + WaitFlag, M_DEFAULT,
                     ActionDevice::ProcessingFunction, this);

         if (IEEE1588Enabled == false)
            MdigHookFunction(MilDigitizer, M_GRAB_FRAME_START+M_UNHOOK, ActionDevice::GrabFrameStart, this);
         }

      static MIL_INT MFTYPE GrabFrameStart(MIL_INT HookType,
                                           MIL_ID HookId,
                                           void* HookDataPtr)
         {
         ActionDevice *ActionDevicePtr = (ActionDevice *)HookDataPtr;
         MIL_DOUBLE TimeStamp = 0;

         // For non IEEE 1588 devices, we cannot use the camera's time-stamp because:
         // 1- They are not synchronized with each other
         // 2- They might not have a common clock frequency
         //
         // Because of this we use the system's time-stamp instead.
         MdigGetHookInfo(HookId, M_TIME_STAMP, &TimeStamp);
         ActionDevicePtr->DeviceTimeStamps.push_back(TimeStamp);

         return 0;
         }

      static MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType,
                                               MIL_ID HookId,
                                               void* HookDataPtr)
         {
         ActionDevice *ActionDevicePtr = (ActionDevice *)HookDataPtr;
         MIL_DOUBLE TimeStamp = 0;
         MIL_ID ModifiedBufferId = 0;
         const MIL_INT StringLengthMax = 20;
         const MIL_INT StringPosX = 20;
         const MIL_INT StringPosY = 20;
         MIL_TEXT_CHAR Text[StringLengthMax] = { MIL_TEXT('\0'), };

         /* Retrieve the MIL_ID of the grabbed buffer and camera statistics. */
         MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);
         ActionDevicePtr->ProcessedImageCount++;

         MosSprintf(Text, StringLengthMax, MIL_TEXT("%d"),
                    (int)ActionDevicePtr->ProcessedImageCount);
         MgraText(M_DEFAULT, ModifiedBufferId, StringPosX, StringPosY, Text);
         
         MbufCopy(ModifiedBufferId, ActionDevicePtr->MilImageDisp);

         if (ActionDevicePtr->IEEE1588Enabled)
            {
            // Read the camera's time-stamp generated when the image was exposed.
            MdigGetHookInfo(HookId, M_GC_CAMERA_TIME_STAMP, &TimeStamp);
            ActionDevicePtr->DeviceTimeStamps.push_back(TimeStamp);
            }

         // Trigger another action sometime in the future
         if (ActionDevicePtr->TriggerFunction)
            {
            // For non IEEE1588 Ptp devices, throttle trigger rate in case connected devices frame rates are not the same.
            // We want to avoid over-triggering cameras.
            if (ActionDevicePtr->TriggerArguments->ActionDelay != 0)
               MosSleep(ActionDevicePtr->TriggerArguments->ActionDelay);

            if (ActionDevicePtr->IEEE1588Enabled)
               ActionDevicePtr->TriggerArguments->PtpDueTime = TimeStamp + 1.0/10.0; // Scheduled Action Trigger to take effect in 0.333 seconds from now
            else
               ActionDevicePtr->TriggerArguments->PtpDueTime = 0;

            ActionDevicePtr->TriggerFunction(ActionDevicePtr->TriggerArguments);
            }

         return 0;
         }

      void Print()
         {
         MosPrintf(MIL_TEXT("\t%s %s (M_DEV%d)\n"), Vendor.c_str(), Model.c_str(), DigitizerNumber);
         }

      MIL_STRING Vendor;
      MIL_STRING Model;
      MIL_STRING IP;
      MIL_STRING ActionName;
      MIL_INT64 ActionNumber;
      MIL_INT DigitizerNumber;
      MIL_ID MilDigitizer;
      MIL_ID MilDisplay;
      MIL_ID MilImageDisp;
      MIL_ID* MilImages;
      MIL_INT MilImageListSize;
      MIL_INT ProcessedImageCount;
      bool IEEE1588Enabled;
      MIL_STRING PtpEnableName;
      MIL_STRING PtpDataSetLatchName;
      MIL_STRING PtpStatusName;
      MIL_STRING PtpClockAccuracyName;
      TRIGGER_FUNCTION_PTR TriggerFunction;
      TriggerFunctionArgument* TriggerArguments;
      std::vector<MIL_DOUBLE> DeviceTimeStamps;
   };

/* Utility function used to determine if Actions defined in the device's XML file are 0 or 1 based. */
void GetDeviceAction(MIL_ID MilDigitizer, MIL_STRING& ActionName, MIL_INT64& ActionNumber)
   {
   MIL_INT Count = 0;
   MIL_INT ErrorPrint = 0;
   MIL_INT64 Num = -1;
   bool Found = false;

   MappInquire(M_ERROR, &ErrorPrint);
   MappControl(M_ERROR, M_PRINT_DISABLE);

   MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("TriggerSelector"), M_TYPE_STRING, MIL_TEXT("FrameStart"));
   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("TriggerSource"), M_TYPE_MIL_INT, &Count);

   /* Try and find a TriggerSource in the form of Action0, Action1, ... */
   /* Here we assume the first entry in "TriggerSource" that begins with "Action" is the lowest action number. */
   for (int i = 0; i < Count && !Found; i++)
      {
      MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("TriggerSource"), M_TYPE_STRING, ActionName);
      if (ActionName.find(MIL_TEXT("Action")) != MIL_STRING::npos)
         Found = true;
      else
         ActionName.clear();
         }

   /* Inquire the lowest action selector number. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_MIN, MIL_TEXT("ActionSelector"), M_TYPE_INT64, &Num);
   if (MappGetError(M_CURRENT, M_NULL) == M_NULL_ERROR)
      ActionNumber = Num;
   else
      ActionNumber = -1;

   MappControl(M_ERROR, ErrorPrint);
   }

/* Routine used to enumerate compatible devices. */
void EnumActionDevices(MIL_ID MilSystem, MIL_ID CompatibilityBits, std::vector<ActionDevice>& ActionDevices)
   {
   MIL_INT DeviceCount = 0, DeviceCapability = 0, CompatibleCount = 0, IncompatibleCount = 0;
   MIL_ID MilDigitizer;

   MsysInquire(MilSystem, M_DIGITIZER_NUM, &DeviceCount);
   MosPrintf(MIL_TEXT("%d GigE Vision devices found.\n\n"), (int)DeviceCount);

   for (MIL_INT i = 0; i < DeviceCount; i++)
      {
      MdigAlloc(MilSystem, M_DEV + i, MIL_TEXT("gigevision_currentstate_continuous.dcf"),
                  M_DEV_NUMBER, &MilDigitizer);

      if (MilDigitizer != M_NULL)
         {
         MIL_INT64 ActionNumber = -1;
         MIL_STRING ActionName;
         /* Inquire GigE Vision device capabilities for action command. */
         MdigInquire(MilDigitizer, M_GC_CONTROL_PROTOCOL_CAPABILITY, &DeviceCapability);
         /* Inquire GigE Vision features for the required action XML features. */
         GetDeviceAction(MilDigitizer, ActionName, ActionNumber);
         if (((DeviceCapability & CompatibilityBits) == CompatibilityBits) && (!ActionName.empty()))
            {
            ActionDevice Device;
            Device.MilDigitizer = MilDigitizer;
            MdigInquire(MilDigitizer, M_CAMERA_VENDOR, Device.Vendor);
            MdigInquire(MilDigitizer, M_CAMERA_MODEL, Device.Model);
            Device.DigitizerNumber = M_DEV + i;
            MdigInquire(MilDigitizer, M_GC_REMOTE_IP_ADDRESS_STRING, Device.IP);
            Device.ActionName = ActionName;
            Device.ActionNumber = ActionNumber;

            /* This device supports the required capabilities. Save it. */
            ActionDevices.push_back(Device);
            CompatibleCount++;
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

   MosPrintf(MIL_TEXT("\n"));
   }

#endif
