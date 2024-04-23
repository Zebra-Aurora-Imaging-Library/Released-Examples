/******************************************************************************/
/*
 * File name: MultiCamera.cpp
 *
 * Synopsis:  This program detects the number of cameras attached
 *            to a GigE Vision, USB3 Vision or GevIQ system, prints
 *            camera vendor information and starts grabbing from all
 *            cameras found using MdigProcess. It also handles camera
 *            removal and addition.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */


#include <mil.h>
#include <cstddef>
#include <vector>
#include <array>
#include <algorithm>

#if M_MIL_USE_WINDOWS
#include <conio.h>
#include <stdlib.h>
#include <windows.h>
#endif

/* Globally customizable parameters. */
const std::size_t gBufferingSizeMax = 5;
const bool gUseFeatureBrowser = false;

/* Camera information structure used during device discovery. */
struct CameraInfo
   {
   CameraInfo() = default;
   ~CameraInfo() = default;

   MIL_INT DeviceNumber = 0;
   MIL_STRING Vendor;
   MIL_STRING Model;
   MIL_STRING UniqueID;
   MIL_STRING DeviceUserID;
   MIL_STRING InterfaceName;
   };

/* User's processing function and camera detect hook data structure. */
struct DigitizerInfo
   {
   DigitizerInfo() = default;
   ~DigitizerInfo() = default;

   MIL_ID MilDigitizer = 0;
   MIL_ID MilDisplay = 0;
   MIL_ID MilImageDisp = 0;
   std::vector<MIL_ID> MilGrabBufferList;
   MIL_INT DeviceNumber = 0;
   MIL_INT ProcessedImageCount = 0;
   MIL_DOUBLE FrameRate = 0;
   MIL_INT ResendRequests = 0;
   MIL_INT PacketSize = 0;
   MIL_INT CorruptImageCount = 0;
   MIL_INT GrabInProgress = 0;
   MIL_INT PayloadSize = 0;
   MIL_STRING CamVendor;
   MIL_STRING CamModel;
   MIL_STRING CamUniqueId;
   MIL_STRING DeviceUserID;
   MIL_STRING InterfaceName;
   bool IsConnected = false;
   MIL_INT SystemType = 0;
   };

/* User's system camera detect hook data structure. */
struct SystemInfo
   {
   SystemInfo() { DigInfoList.reserve(32); }
   ~SystemInfo() = default;

   MIL_ID MilSystem = 0;
   MIL_INT SystemType = 0;
   std::vector<DigitizerInfo> DigInfoList;
   std::vector<MIL_STRING> Interfaces;
   };

/* User's processing and camera detect hook functions. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID EventId, void* HookDataPtr);
MIL_INT MFTYPE CamPresentFunction(MIL_INT HookType, MIL_ID EventId, void* HookDataPtr);

/* Camera discovery routines. */
std::vector<CameraInfo> DiscoverDevices(const SystemInfo& SystemInfo);
std::vector<CameraInfo> GetDevices(const SystemInfo& SystemInfo);

/* Digitizer allocation and acquisition routines.
   Used to make the example more compact. */
DigitizerInfo DigAllocResources(SystemInfo& SysInfo, const CameraInfo& CamInfo);
void DigFreeResources(DigitizerInfo& DigInfo);
void DigStartAcquisition(DigitizerInfo& DigInfo);
void DigStopAcquisition(DigitizerInfo& DigInfo);

/* Utility functions. */
void PrintSynopsis();
void PrintCameraInfo(SystemInfo& SysInfo);
void AddAdapterToList(SystemInfo& SysInfo, const MIL_STRING& AdapterName);

int MosMain(void)
   {
   PrintSynopsis();

   MIL_ID MilApplication;
   SystemInfo SysInfo;

   /* Allocate default MIL application and system resources. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &SysInfo.MilSystem, M_NULL, M_NULL, M_NULL);
   SysInfo.SystemType = MsysInquire(SysInfo.MilSystem, M_SYSTEM_TYPE, M_NULL);

   /* Validate that we are running on a compatible MIL system. */
   if((SysInfo.SystemType != M_SYSTEM_GIGE_VISION_TYPE) &&
      (SysInfo.SystemType != M_SYSTEM_USB3_VISION_TYPE) &&
      (SysInfo.SystemType != M_SYSTEM_GEVIQ_TYPE))
      {
      MosPrintf(MIL_TEXT("This example requires a M_SYSTEM_GIGE_VISION, M_SYSTEM_USB3_VISION,\n"));
      MosPrintf(MIL_TEXT("or M_SYSTEM_GEVIQ system type.\n"));
      MosPrintf(MIL_TEXT("Please change system type in MILConfig.\n"));
      MappFreeDefault(MilApplication, SysInfo.MilSystem, M_NULL, M_NULL, M_NULL);
      MosGetch();
      return -1;
      }

   MosPrintf(MIL_TEXT("Discovering devices.\n\n"));

   /* Discover cameras attached to this system. */
   const auto Cameras = DiscoverDevices(SysInfo);

   MosPrintf(MIL_TEXT("Allocating devices.\n"));

   /* Allocate and start acquisition on all cameras found. */
   if (!Cameras.empty())
      {
      MIL_INT CamerasAllocated = M_FALSE;
      /* Allocate digitizers and other resources. */
      for (const auto& Camera : Cameras)
         {
         auto DigInfo = DigAllocResources(SysInfo, Camera);
         if (DigInfo.MilDigitizer)
            {
            CamerasAllocated = M_TRUE;
            SysInfo.DigInfoList.push_back(DigInfo);
            }
         }

      if (CamerasAllocated == M_TRUE)
         {
         /* Start acquisition. */
         for (auto& DigData : SysInfo.DigInfoList)
            DigStartAcquisition(DigData);
         }
      else
         {
         /* Cameras might already be allocated by another process on this or */
         /* another PC. Do a license check to determine if allocation        */
         /* failure was caused by lack of proper license.                    */

         MIL_ID MilRemoteApplication = M_NULL;
         MIL_INT LicenseModules = 0;
         MsysInquire(SysInfo.MilSystem, M_OWNER_APPLICATION, &MilRemoteApplication);

         MappInquire(MilRemoteApplication, M_LICENSE_MODULES, &LicenseModules);
         if (!(LicenseModules & (M_LICENSE_INTERFACE)))
            {
            MosPrintf(MIL_TEXT("Need a GigE Vision license to run this example.\n"));
            MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
            MosGetch();

            MappFreeDefault(MilApplication, SysInfo.MilSystem, M_NULL, M_NULL, M_NULL);
            return 0;
            }
         }
      }

   /* Register a hook function to the system's camera present event. */
   /* Used to:                                                        */
   /* 1- Allocate and start acquisition on a newly attached camera.   */
   /* 2- Stop acquisition on a camera that has been removed.          */
   /* 3- Resume acquisition on a camera that has been re-connected.   */
   MsysHookFunction(SysInfo.MilSystem, M_CAMERA_PRESENT, CamPresentFunction, &SysInfo);

   /* At this point digitizers have been allocated and acquisition started on all      */
   /* cameras found at MsysAlloc time (if any). The example now waits for the user to  */
   /* add / remove cameras to the system. The camera present hook will then get called */
   /* and everything will get handled from there.                                      */

   while (!MosKbhit())
      PrintCameraInfo(SysInfo);

   MosGetch();

   /* The user is stopping the example, stop acquisition and free everything. */
   for (auto& DigData : SysInfo.DigInfoList)
      DigStopAcquisition(DigData);

   PrintCameraInfo(SysInfo);

   MosPrintf(MIL_TEXT("\nFreeing everything.\n"));
   for (auto& DigData : SysInfo.DigInfoList)
      DigFreeResources(DigData);

   MsysHookFunction(SysInfo.MilSystem, M_CAMERA_PRESENT + M_UNHOOK, CamPresentFunction, &SysInfo);

   MsysFree(SysInfo.MilSystem);
   MappFree(MilApplication);

   return 0;
   }

/* Prints example synopsis message. */
void PrintSynopsis()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n\n"));
   MosPrintf(MIL_TEXT("MultiCamera\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n\n"));
   MosPrintf(MIL_TEXT("This program detects the number of cameras attached\n"));
   MosPrintf(MIL_TEXT("to a MIL GigE Vision, USB3 Vision or GevIQ system, prints\n"));
   MosPrintf(MIL_TEXT("camera vendor information and starts grabbing from all\n"));
   MosPrintf(MIL_TEXT("cameras found using MdigProcess. It also handles camera\n"));
   MosPrintf(MIL_TEXT("removal and addition.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   }

/* Issues a device discovery request and retrieves camera info. */
std::vector<CameraInfo> DiscoverDevices(const SystemInfo& SystemInfo)
   {
   MsysControl(SystemInfo.MilSystem, M_DISCOVER_DEVICE, M_DEFAULT);
   return GetDevices(SystemInfo);
   }

/* Retrieves information related to cameras accessible from this system. */
std::vector<CameraInfo> GetDevices(const SystemInfo& SystemInfo)
   {
   const MIL_INT Count = MsysInquire(SystemInfo.MilSystem, M_DISCOVER_DEVICE_COUNT, M_NULL);

   std::vector<CameraInfo> Cameras;
   Cameras.reserve(Count);
   for (MIL_INT i = 0; i < Count; ++i)
      {
      CameraInfo Camera;
      MsysInquire(SystemInfo.MilSystem, M_DISCOVER_DEVICE_DIGITIZER_NUMBER + i, &Camera.DeviceNumber);
      MsysInquire(SystemInfo.MilSystem, M_DISCOVER_DEVICE_MANUFACTURER_NAME + i, Camera.Vendor);
      MsysInquire(SystemInfo.MilSystem, M_DISCOVER_DEVICE_MODEL_NAME + i, Camera.Model);
      MsysInquire(SystemInfo.MilSystem, M_DISCOVER_DEVICE_UNIQUE_IDENTIFIER + i, Camera.UniqueID);
      MsysInquire(SystemInfo.MilSystem, M_DISCOVER_DEVICE_USER_NAME + i, Camera.DeviceUserID);
      MsysInquire(SystemInfo.MilSystem, M_DISCOVER_DEVICE_INTERFACE_NAME + i, Camera.InterfaceName);

      Cameras.push_back(Camera);
      }

   return Cameras;
   }

/* User's processing function called every time a grab buffer is modified. */
/* ----------------------------------------------------------------------- */

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   DigitizerInfo& DigInfo = *(DigitizerInfo*)HookDataPtr;
   MIL_ID ModifiedBufferId = 0;
   MIL_INT IsCorrupt = 0;

   /* Retrieve the MIL_ID of the grabbed buffer and camera statistics. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);
   MdigGetHookInfo(HookId, M_CORRUPTED_FRAME, &IsCorrupt);

   /* Get GigE Vision TL specific information. */
   if(DigInfo.SystemType == M_SYSTEM_GIGE_VISION_TYPE)
      {
      const MIL_INT ResendRequests = MdigGetHookInfo(HookId, M_GC_PACKETS_RESENDS_NUM, M_NULL);
      DigInfo.ResendRequests += ResendRequests;
      }

   /* Copy the grabbed frame to display. */
   if (IsCorrupt)
      DigInfo.CorruptImageCount++;
   else
      {
      DigInfo.ProcessedImageCount++;
      MbufCopy(ModifiedBufferId, DigInfo.MilImageDisp);
      }

   return 0;
   }

/* User's camera present function called every time a camera connection state */
/* changes.                                                                   */
/* -------------------------------------------------------------------------- */

MIL_INT MFTYPE CamPresentFunction(MIL_INT HookType, MIL_ID EventId, void* HookDataPtr)
   {
   SystemInfo& SysInfo = *(SystemInfo *)HookDataPtr;
   auto& DigInfoList = SysInfo.DigInfoList;
   MIL_INT IsCamPresent, Number;

   /* Get the updated list of cameras. */
   const auto Cameras = GetDevices(SysInfo);

   /* Inquire the camera present state (present or not present) associated to this event. */
   MsysGetHookInfo(SysInfo.MilSystem, EventId, M_CAMERA_PRESENT, &IsCamPresent);

   /* Inquire the camera device number that triggered this event. */
   MsysGetHookInfo(SysInfo.MilSystem, EventId, M_NUMBER, &Number);

   /* Verify if this device number is already associated to a digitizer. */
   auto DigInfoIt = std::find_if(DigInfoList.begin(), DigInfoList.end(),
      [Number](const DigitizerInfo& DigInfo) { return DigInfo.DeviceNumber == Number; });

   /* Search the list of camera and get the one that corresponds to this event. */
   auto CamInfoIt = std::find_if(Cameras.begin(), Cameras.end(),
      [Number](const CameraInfo& CamInfo) { return CamInfo.DeviceNumber == Number; });

   if (IsCamPresent)
      {
      /* Camera is present. */

      MIL_STRING UniqueId;
      /* Inquire the camera's Unique Id associated to this event. */
      MsysGetHookInfo(SysInfo.MilSystem, EventId, M_GC_UNIQUE_ID_STRING, UniqueId);

      if (DigInfoIt == DigInfoList.end())
         {
         /* Newly attached camera, never seen it before: allocate it. */
         auto DigInfo = DigAllocResources(SysInfo, *CamInfoIt);
         DigInfoList.push_back(DigInfo);
         DigInfoIt = std::prev(DigInfoList.end());
         }
      else if (DigInfoIt->CamUniqueId != UniqueId)
         {
         /* New camera added in place of another one, free old digitizer */
         DigFreeResources(*DigInfoIt);
         DigInfoList.erase(DigInfoIt);

         /* Find out if camera was previously associated to a different device number. */
         DigInfoIt = std::find_if(DigInfoList.begin(), DigInfoList.end(),
            [&UniqueId](const DigitizerInfo& DigInfo) { return DigInfo.CamUniqueId == UniqueId; });

         if (DigInfoIt != DigInfoList.end())
            {
            DigFreeResources(*DigInfoIt);
            DigInfoList.erase(DigInfoIt);
            }

         /* Allocate resources. */
         auto DigInfo = DigAllocResources(SysInfo, *CamInfoIt);
         DigInfoList.push_back(DigInfo);
         DigInfoIt = std::prev(DigInfoList.end());
         }

      DigInfoIt->IsConnected = true;
      /* Start acquisition. */
      DigStartAcquisition(*DigInfoIt);
      }
   else
      {
      /* Camera is not present. */
      if (DigInfoIt != DigInfoList.end())
         {
         /* Stop acquisition. */
         DigStopAcquisition(*DigInfoIt);
         DigInfoIt->IsConnected = false;
         }
      }

   return 0;
   }

/* Allocates digitizer plus all other resources needed for image acquisition and */
/* camera state changes.                                                         */
/* ------------------------------------------------------------------------------*/

DigitizerInfo DigAllocResources(SystemInfo& SysInfo, const CameraInfo& CamInfo)
   {
   DigitizerInfo DigInfo;

   MIL_INT SizeBand = 0, BufType = 0, SizeBit = 0;
   MIL_INT64 BufFormat = 0;

   /* Cameras can be allocated using a device number or a user programmable identifier string. */
   /* The camera's DeviceUserID feature must be programmed by the user to a unique string.     */
   if (CamInfo.DeviceUserID.empty())
      MdigAlloc(SysInfo.MilSystem, CamInfo.DeviceNumber, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &DigInfo.MilDigitizer);
   else
      MdigAlloc(SysInfo.MilSystem, M_GC_CAMERA_ID(CamInfo.DeviceUserID.c_str()), MIL_TEXT("M_DEFAULT"),
         M_GC_DEVICE_NAME, & DigInfo.MilDigitizer);

   if (DigInfo.MilDigitizer)
      {
      DigInfo.SystemType = SysInfo.SystemType;
      DigInfo.DeviceNumber = CamInfo.DeviceNumber;
      DigInfo.IsConnected = true;
      DigInfo.CamVendor = CamInfo.Vendor;
      DigInfo.CamModel = CamInfo.Model;
      DigInfo.CamUniqueId = CamInfo.UniqueID;
      DigInfo.DeviceUserID = CamInfo.DeviceUserID;
      DigInfo.InterfaceName = CamInfo.InterfaceName;

      /* Disable MdigProcess grab monitor since disconnecting a camera will result in an  */
      /* error message.                                                                   */
      MdigControl(DigInfo.MilDigitizer, M_PROCESS_GRAB_MONITOR, M_DISABLE);
      /* Disable corrupted frame errors as they are handled from the MdigProcess hook     */
      /* function.                                                                        */
      MdigControl(DigInfo.MilDigitizer, M_CORRUPTED_FRAME_ERROR, M_DISABLE);

      if(DigInfo.SystemType == M_SYSTEM_GIGE_VISION_TYPE)
         MdigInquire(DigInfo.MilDigitizer, M_GC_PACKET_SIZE, &DigInfo.PacketSize);

      /* Pop-up the MIL feature browser; exposes device features. */
      if (gUseFeatureBrowser)
         {
         MdigControl(DigInfo.MilDigitizer, M_GC_FEATURE_BROWSER, M_OPEN + M_ASYNCHRONOUS);
         }

      /* Allocate displays and buffers; everything necessary to run MdigProcess. */
      MdispAlloc(SysInfo.MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT,
         &DigInfo.MilDisplay);
      MdispControl(DigInfo.MilDisplay, M_TITLE, DigInfo.CamModel);

      /* Allocate a buffer format that matches the camera's pixel format. */
      MdigInquire(DigInfo.MilDigitizer, M_SIZE_BAND, &SizeBand);
      MdigInquire(DigInfo.MilDigitizer, M_TYPE, &BufType);
      MdigInquire(DigInfo.MilDigitizer, M_SOURCE_DATA_FORMAT, &BufFormat);
      MdigInquire(DigInfo.MilDigitizer, M_SIZE_BIT, &SizeBit);

      MbufAllocColor(SysInfo.MilSystem,
         SizeBand,
         MdigInquire(DigInfo.MilDigitizer, M_SIZE_X, M_NULL),
         MdigInquire(DigInfo.MilDigitizer, M_SIZE_Y, M_NULL),
         BufType,
         M_IMAGE + M_GRAB + M_DISP + BufFormat,
         &DigInfo.MilImageDisp);

      MbufClear(DigInfo.MilImageDisp, M_COLOR_BLACK);
      if(SizeBit > 8)
         {
         MdispControl(DigInfo.MilDisplay, M_VIEW_MODE, M_BIT_SHIFT);
         MdispControl(DigInfo.MilDisplay, M_VIEW_BIT_SHIFT, SizeBit - 8);
         }

      MdispSelect(DigInfo.MilDisplay, DigInfo.MilImageDisp);

      for (MIL_INT i = 0; i < gBufferingSizeMax; i++)
         {
         MIL_ID BufferId = M_NULL;
         MbufAllocColor(SysInfo.MilSystem,
            SizeBand,
            MdigInquire(DigInfo.MilDigitizer, M_SIZE_X, M_NULL),
            MdigInquire(DigInfo.MilDigitizer, M_SIZE_Y, M_NULL),
            BufType,
            M_GRAB + M_IMAGE + BufFormat,
            &BufferId);
         if (BufferId)
            {
            MbufClear(BufferId, 0);
            DigInfo.MilGrabBufferList.push_back(BufferId);
            }
         }

      if (SysInfo.SystemType == M_SYSTEM_GIGE_VISION_TYPE)
         {
         AddAdapterToList(SysInfo, DigInfo.InterfaceName);
         }
      }

   return DigInfo;
   }

/* Free digitizer and all other resources allocated.  */
/* -------------------------------------------------- */

void DigFreeResources(DigitizerInfo& DigInfo)
   {
   if (DigInfo.MilDigitizer)
      {
      for (const auto BufferId : DigInfo.MilGrabBufferList)
         MbufFree(BufferId);

      DigInfo.MilGrabBufferList.clear();

      MbufFree(DigInfo.MilImageDisp);
      MdispFree(DigInfo.MilDisplay);

      if (gUseFeatureBrowser)
         {
         /* Close the MIL feature browser. */
         MdigControl(DigInfo.MilDigitizer, M_GC_FEATURE_BROWSER, M_CLOSE);
         }

      MdigFree(DigInfo.MilDigitizer);
      }
   }

/* Starts MdigProcess. */
/* ------------------- */

void DigStartAcquisition(DigitizerInfo& DigInfo)
   {
   if (DigInfo.MilDigitizer)
      {
      DigInfo.GrabInProgress = M_TRUE;
      MdigProcess(DigInfo.MilDigitizer, DigInfo.MilGrabBufferList,
         M_DEFAULT, M_START, M_DEFAULT,
         ProcessingFunction, &DigInfo);

      MdigInquire(DigInfo.MilDigitizer, M_GC_PAYLOAD_SIZE, &DigInfo.PayloadSize);
      }
   }

/* Stops MdigProcess. */
/* ------------------ */

void DigStopAcquisition(DigitizerInfo& DigInfo)
   {
   if (DigInfo.GrabInProgress)
      {
      MdigProcess(DigInfo.MilDigitizer, DigInfo.MilGrabBufferList,
         M_DEFAULT, M_STOP, M_DEFAULT,
         ProcessingFunction, &DigInfo);
      DigInfo.GrabInProgress = M_FALSE;
      }
   }

/* Print camera state information. */
/* ------------------ */
void PrintCameraInfo(SystemInfo& SysInfo)
   {
   const int StatsPrintPeriod = 1000;

   if (SysInfo.DigInfoList.empty() == false)
      {
      const MIL_TEXT_CHAR Str[] = MIL_TEXT("                           ");
      MosSleep(StatsPrintPeriod);
#if M_MIL_USE_WINDOWS
      system("cls");
#endif
      MosPrintf(MIL_TEXT("This example shows how to handle camera connect / ")
         MIL_TEXT("disconnect events.\n\n"));
      MosPrintf(MIL_TEXT("%d camera%s detected.\n"), (int)SysInfo.DigInfoList.size(),
         SysInfo.DigInfoList.size() > 1 ? MIL_TEXT("s") : MIL_TEXT(""));
      MosPrintf(MIL_TEXT("You can proceed to add / remove cameras to your ")
         MIL_TEXT("system at anytime.\n\n"));
      MosPrintf(MIL_TEXT("%s----------------------------------------------------\n"), Str);
      MosPrintf(MIL_TEXT("%s                  Camera statistics                 \n"), Str);
      MosPrintf(MIL_TEXT("%s-------------------------------------+--------------\n"), Str);
      MosPrintf(MIL_TEXT("%s                Frame                |    Packet    \n"), Str);
      MosPrintf(MIL_TEXT("%s-------------------------------------+--------------\n"), Str);
      MosPrintf(MIL_TEXT("%-14s%-13s%9s%8s%11s%8s%8s%8s\n"), MIL_TEXT("Model"),
         MIL_TEXT("State"), MIL_TEXT("Grabbed"), MIL_TEXT("Rate"),
         MIL_TEXT("Bandwidth"), MIL_TEXT("Corrupt"), MIL_TEXT("|  Size"),
         MIL_TEXT("Resends"));
      MosPrintf(MIL_TEXT("----------------------------")
         MIL_TEXT("------------------------------------+--------------\n"));

      for (auto& DigData : SysInfo.DigInfoList)
         {
         if (DigData.MilDigitizer)
            {
            if (DigData.IsConnected)
               MdigInquire(DigData.MilDigitizer, M_PROCESS_FRAME_RATE, &DigData.FrameRate);
            else
               DigData.FrameRate = 0;

            MosPrintf(MIL_TEXT("%-14.13s%-13.12s%9d%8.1f%11.1f%8d%8d%8d\n"),
               DigData.CamModel.c_str(),
               DigData.IsConnected ? MIL_TEXT("Connected") : MIL_TEXT("Disconnected"),
               DigData.ProcessedImageCount,
               DigData.FrameRate,
               (DigData.PayloadSize*DigData.FrameRate / 1e6),
               DigData.CorruptImageCount,
               DigData.PacketSize,
               DigData.ResendRequests);
            }
         }

      MosPrintf(MIL_TEXT("----------------------------")
         MIL_TEXT("---------------------------------------------------\n\n"));

      if(SysInfo.SystemType == M_SYSTEM_GIGE_VISION_TYPE)
         {
         MosPrintf(MIL_TEXT("Network adapter statistics\n\n"));
         MIL_DOUBLE AdapterBandwidth = 0;
         for (auto& Interface : SysInfo.Interfaces)
            {
            AdapterBandwidth = 0;
            for (auto& DigData : SysInfo.DigInfoList)
               {
               if (DigData.MilDigitizer && DigData.InterfaceName == Interface)
                  {
                  AdapterBandwidth += (DigData.PayloadSize * DigData.FrameRate / 1e6);
                  }
               }

            MosPrintf(MIL_TEXT("\n%-50.49s%.1f (MB/s) connected to:\n"), Interface.c_str(),
               AdapterBandwidth);
            MosPrintf(MIL_TEXT("----------------------------")
               MIL_TEXT("---------------------------------------------------\n"));

            for (auto& DigData : SysInfo.DigInfoList)
               {
               if (DigData.MilDigitizer && DigData.InterfaceName == Interface)
                  {
                  if (DigData.DeviceUserID.empty())
                     MosPrintf(MIL_TEXT("%s %s\n"), DigData.CamVendor.c_str(), DigData.CamModel.c_str());
                  else
                     MosPrintf(MIL_TEXT("%s %s (%s)\n"), DigData.CamVendor.c_str(), DigData.CamModel.c_str(),
                        DigData.DeviceUserID.c_str());
                  }
               }
            }
         }
      }
   else
      {
#if M_MIL_USE_WINDOWS
      system("cls");
#endif
      MosPrintf(MIL_TEXT("This example shows how to handle camera connect / ")
         MIL_TEXT("disconnect events.\n\n"));
      MosPrintf(MIL_TEXT("%d camera detected.\n"), (int)SysInfo.DigInfoList.size());
      MosPrintf(MIL_TEXT("You can proceed to add / remove cameras to your system ")
         MIL_TEXT("anytime.\n\n"));
      MosPrintf(MIL_TEXT("\r|"));
      MosSleep(StatsPrintPeriod / 4);
      MosPrintf(MIL_TEXT("\r/"));
      MosSleep(StatsPrintPeriod / 4);
      MosPrintf(MIL_TEXT("\r-"));
      MosSleep(StatsPrintPeriod / 4);
      MosPrintf(MIL_TEXT("\r\\"));
      MosSleep(StatsPrintPeriod / 4);
      }
   }

void AddAdapterToList(SystemInfo& SysInfo, const MIL_STRING& InterfaceName)
   {
   auto It = std::find_if(SysInfo.Interfaces.begin(), SysInfo.Interfaces.end(),
      [&InterfaceName](const MIL_STRING& Name) { return Name == InterfaceName; });

   if (It == SysInfo.Interfaces.end())
      SysInfo.Interfaces.push_back(InterfaceName);
   }

