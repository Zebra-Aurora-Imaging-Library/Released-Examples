/*************************************************************************************/
/*
 * File name: MulticastMaster.cpp
 *
 * Synopsis:  This program shows how to perform IP Multicast with GigE Vision devices.
 *
 *            To do this you must have a network capable of delivering a Multicast
 *            service over IPv4. This requires the use of routers and LAN switches
 *            that support the Internet Group Management Protocol (IGMP). Some manual
 *            configuration of you LAN switches might be required. More information
 *            can be found in the IP Multicast section of Matrox GigE Vision
 *            Assistant's help file.
 *
 *      Note: This example must be used along with the MulticastSlave program
 *            connected to the same GigE Vision device and running on another PC.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#if M_MIL_USE_WINDOWS
#include <Windows.h>
#endif

 /* Number of images in the buffering grab queue.
    Generally, increasing this number gives better real-time grab.
  */
#define BUFFERING_SIZE_MAX 20

/* User's processing function hook data structure. */
typedef struct
   {
   MIL_ID MilImageDisp;
   MIL_INT ProcessedImageCount;
   MIL_INT CorruptImageCount;
   } HookDataStruct;

/* Function prototypes.                  */
void PrintCameraInfo(MIL_ID MilDigitizer);
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType,
                                  MIL_ID HookId,
                                  void* HookDataPtr);

/* Main function. */
/* ---------------*/

int MosMain(void)
{
   MIL_ID  MilApplication;
   MIL_ID  MilSystem     ;
   MIL_ID  MilDigitizer;
   MIL_ID  MilDisplay;
   MIL_ID  MilImageDisp;
   MIL_ID  MilGrabBufferList[BUFFERING_SIZE_MAX];
   MIL_INT MilGrabBufferListSize;
   MIL_INT ProcessFrameCount  = 0;
   MIL_INT DigProcessInProgress = M_FALSE;
   MIL_INT SystemType = 0;
   MIL_DOUBLE ProcessFrameRate = 0;
   MIL_INT64 SourceDataFormat;
   HookDataStruct UserHookData;
   MIL_INT n = 0;

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, M_NULL, M_NULL);

   /* This example only runs on a MIL GigE Vision system type. */
   MsysInquire(MilSystem, M_SYSTEM_TYPE, &SystemType);
   if(SystemType != M_SYSTEM_GIGE_VISION_TYPE)
      {
      MosPrintf(MIL_TEXT("This example requires a M_GIGE_VISION system type.\n"));
      MosPrintf(MIL_TEXT("Please change system type in milconfig.\n"));
      MosPrintf(MIL_TEXT("\nPress <Enter> to quit.\n"));
      MosGetch();
      MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
      return 0;
      }

   /* Allocate a master Multicast digitizer. */
   MdigAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_GC_MULTICAST_MASTER,
      &MilDigitizer);
   /* The default Multicast address can be changed if a conflict exists with:  */
   // MIL_STRING MulticastAddr = MIL_TEXT("239.255.16.16");
   // MdigControl(MilDigitizer, M_GC_STREAM_CHANNEL_MULTICAST_ADDRESS_STRING, MulticastAddr);
   // MdigControl(MilDigitizer, M_GC_UPDATE_MULTICAST_INFO, M_DEFAULT);

   /* Note that the above IP address 239.255.16.16 is specified for illustrative purposes
      only. */

   /* Allocate a display. */
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);
   /* Inquire the buffer format compatible with the camera's current pixel format. */
   MdigInquire(MilDigitizer, M_SOURCE_DATA_FORMAT, &SourceDataFormat);

   /* Allocate the display buffer clear it and associate it to the display. */
   MbufAllocColor(MilSystem,
      MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL),
      MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
      MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
      MdigInquire(MilDigitizer, M_TYPE, M_NULL),
      M_IMAGE+M_DISP+M_GRAB+M_PROC+SourceDataFormat,
      &MilImageDisp);
   MbufClear(MilImageDisp, M_COLOR_BLACK);
   MdispSelect(MilDisplay, MilImageDisp);

   /* Allocate the grab buffers and clear them. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for(MilGrabBufferListSize = 0; 
      MilGrabBufferListSize<BUFFERING_SIZE_MAX; MilGrabBufferListSize++)
      {
      MbufAllocColor(MilSystem,
         MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL),
         MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
         MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
         MdigInquire(MilDigitizer, M_TYPE, M_NULL),
         M_IMAGE+M_GRAB+M_PROC+SourceDataFormat,
         &MilGrabBufferList[MilGrabBufferListSize]);

      if (MilGrabBufferList[MilGrabBufferListSize])
         {
         MbufClear(MilGrabBufferList[MilGrabBufferListSize], M_COLOR_WHITE);
         }
      else
         break;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);


   MosPrintf(MIL_TEXT("This example demonstrates the use of IP Multicast with GigE Vision"));
   MosPrintf(MIL_TEXT(" devices.\n"));
   MosPrintf(MIL_TEXT("It allocates a Multicast master digitizer that can read, write and"));
   MosPrintf(MIL_TEXT(" grab from\n"));
   MosPrintf(MIL_TEXT("a GigE Vision device.\n\n"));
   MosPrintf(MIL_TEXT("This example must be used along with MulticastSlave.cpp connected to"));
   MosPrintf(MIL_TEXT(" the same\n"));
   MosPrintf(MIL_TEXT("GigE Vision device and running on another PC.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue."));
   MosGetch();

   /* Print info related to the device we are connected to. */
   PrintCameraInfo(MilDigitizer);

   /* Initialize the User's processing function data structure. */
   UserHookData.MilImageDisp        = MilImageDisp;
   UserHookData.ProcessedImageCount = 0;
   UserHookData.CorruptImageCount   = 0;

   /* Start the processing. The processing function is called for every frame grabbed. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
                             M_START, M_DEFAULT, ProcessingFunction, &UserHookData);

   /* NOTE: Now the main() is free to perform other tasks 
                                                    while the processing is executing. */
   /* -------------------------------------------------------------------------------- */
   MosPrintf(MIL_TEXT("If the MulticastSlave program is already running on the other PC, it"));
   MosPrintf(MIL_TEXT(" should\n"));
   MosPrintf(MIL_TEXT("have detected that this device is controlled by a multicast master"));
   MosPrintf(MIL_TEXT(" digitizer\n"));
   MosPrintf(MIL_TEXT("and have started image acquisition.\n\n"));
   MosPrintf(MIL_TEXT("If the MulticastSlave program is not yet started then it should be"));
   MosPrintf(MIL_TEXT(" started now.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to stop.\n"));
   MosGetch();

   /* Stop the processing. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
               M_STOP, M_DEFAULT, ProcessingFunction, &UserHookData);

   /* Print statistics. */
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT,  &ProcessFrameCount);
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE,   &ProcessFrameRate);
   MosPrintf(MIL_TEXT("\n\n%lld frames grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
                          (long long)ProcessFrameCount, ProcessFrameRate, 1000.0/ProcessFrameRate);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   while(MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);

   /* Release defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);

   return 0;
}

/* Prints information regarding the device this master digitizer is connected to. */
/* -----------------------------------------------------------------------       */
void PrintCameraInfo(MIL_ID MilDigitizer)
   {
   MIL_STRING DeviceVendor;
   MIL_STRING DeviceModel;
   MIL_STRING PixelFormat;
   MIL_INT64 Width = 0, Height = 0;
   MIL_INT Port = 0;
   MIL_STRING MulticastAddress;

#if M_MIL_USE_WINDOWS
   /* Clear console. */
   system("cls");
#endif

   /* Inquire camera vendor name. */
   MdigInquire(MilDigitizer, M_CAMERA_VENDOR, DeviceVendor);
   
   /* Inquire camera model name. */
   MdigInquire(MilDigitizer, M_CAMERA_MODEL, DeviceModel);

   /* Inquire camera pixel format. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PixelFormat"), M_TYPE_STRING, PixelFormat);

   /* Inquire camera width and height. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("Width"), M_TYPE_INT64, &Width);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("Height"), M_TYPE_INT64, &Height);

   /* Inquire the Multicast address used. */
   MdigInquire(MilDigitizer, M_GC_STREAM_CHANNEL_MULTICAST_ADDRESS_STRING, MulticastAddress);

   MdigInquire(MilDigitizer, M_GC_LOCAL_STREAM_PORT, &Port);

   /* Print camera info. */
   MosPrintf(MIL_TEXT("\n--------------------- Master digitizer connection status. "));
   MosPrintf(MIL_TEXT("---------------------\n\n"));
   MosPrintf(MIL_TEXT("Connected to             %s %s\n"), DeviceVendor.c_str(), DeviceModel.c_str());
   MosPrintf(MIL_TEXT("Device pixel format:     %s\n"), PixelFormat.c_str());
   MosPrintf(MIL_TEXT("Device AOI:              %lld x %lld\n"), (long long)Width, (long long)Height);
   MosPrintf(MIL_TEXT("IPv4 Multicast address:  %s\n"), MulticastAddress.c_str());
   MosPrintf(MIL_TEXT("Stream port:             %lld\n\n"), (long long)Port);
   }

/* User's processing function called every time a grab buffer is modified. */
/* -----------------------------------------------------------------------*/

/* Local defines. */
#define STRING_LENGTH_MAX  20
#define STRING_POS_X       20
#define STRING_POS_Y       20

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType,
                                  MIL_ID HookId,
                                  void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX]= {MIL_TEXT('\0'),};
   MIL_INT IsFrameCorrupt = M_FALSE;

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID, &ModifiedBufferId);
   MdigGetHookInfo(HookId, M_CORRUPTED_FRAME, &IsFrameCorrupt);

   /* Print and draw the frame count. */
   UserHookDataPtr->ProcessedImageCount++;
   if(IsFrameCorrupt)
      UserHookDataPtr->CorruptImageCount++;
   MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%lld"), 
                                       (long long)UserHookDataPtr->ProcessedImageCount);
   MgraText(M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, Text);

   /* Perform the processing and update the display. */
   MbufCopy(ModifiedBufferId, UserHookDataPtr->MilImageDisp);
   
   
   return 0;
   }
