/*************************************************************************************/
/*
 * File name: MulticastSlave.cpp
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
 *      Note: This example must be used along with the MulticastMaster program
 *            connected to the same GigE Vision device and running on another PC.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#if M_MIL_USE_WINDOWS
#include <windows.h>
#endif

 /* Number of images in the buffering grab queue.
    Generally, increasing this number gives better real-time grab.
  */
#define BUFFERING_SIZE_MAX 20

/* User's processing function hook data structure. */
typedef struct
   {
   MIL_ID  MilDigitizer;
   MIL_ID  MilDisplay;
   MIL_ID  MilImageDisp;
   MIL_ID  MilGrabBufferList[BUFFERING_SIZE_MAX];
   MIL_INT MilGrabBufferListSize;
   MIL_INT ProcessedImageCount;
   MIL_INT CorruptImageCount;
   MIL_INT FrameSizeX;
   MIL_INT FrameSizeY;
   MIL_INT64 FramePixelFormat;
   MIL_INT FramePacketSize;
   bool DataFormatChanged;
   MIL_INT64 SourceDataFormat;
   MIL_ID Event;
   MIL_STRING DeviceVendor;
   MIL_STRING DeviceModel;
   } HookDataStruct;

/* Function prototypes.                  */
void AllocateGrabBuffers(MIL_INT MilSystem, HookDataStruct* HookDataPtr);
void FreeGrabBuffers(HookDataStruct* HookDataPtr);
void AdaptToMulticastMasterStatus(MIL_INT MilSystem, HookDataStruct* HookDataPtr);
void PrintCameraInfo(HookDataStruct* HookDataPtr);
void PrintMasterStatusInfo(MIL_INT IsConnected);
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType,
                                  MIL_ID HookId,
                                  void* HookDataPtr);

/* Main function. */
/* ---------------*/

int MosMain(void)
{
   MIL_ID MilApplication;
   MIL_ID MilSystem     ;
   MIL_INT ProcessFrameCount  = 0;
   MIL_INT DigProcessInProgress = M_FALSE;
   MIL_INT SystemType = 0;
   MIL_DOUBLE ProcessFrameRate= 0;
   HookDataStruct UserHookData;

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

   /* Allocate a slave Multicast digitizer. */
   MdigAlloc(MilSystem, M_DEFAULT, M_NULL, M_GC_MULTICAST_SLAVE, &UserHookData.MilDigitizer);

   /* Allocate synchronization event. */
   MthrAlloc(MilSystem, M_EVENT, M_NOT_SIGNALED+M_AUTO_RESET, M_NULL, M_NULL,
      &UserHookData.Event);

   /* Allocate a display and buffers. */
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &UserHookData.MilDisplay);
   AllocateGrabBuffers(MilSystem, &UserHookData);

   MosPrintf(MIL_TEXT("This example demonstrates the use of IP Multicast with GigE Vision"));
   MosPrintf(MIL_TEXT(" devices.\n"));
   MosPrintf(MIL_TEXT("It allocates a slave digitizer that can read and grab from a GigE"));
   MosPrintf(MIL_TEXT(" Vision\n"));
   MosPrintf(MIL_TEXT("device provided a Multicast master digitizer is allocated on the same"));
   MosPrintf(MIL_TEXT(" device.\n\n"));
   MosPrintf(MIL_TEXT("This example must be used along with MulticastMaster.cpp connected to"));
   MosPrintf(MIL_TEXT(" the same\n"));
   MosPrintf(MIL_TEXT("GigE Vision device and running on another PC.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue."));
   MosGetch();

   MdispSelect(UserHookData.MilDisplay, UserHookData.MilImageDisp);

   /* Initialize the User's processing function data structure. */
   UserHookData.ProcessedImageCount = 0;
   UserHookData.CorruptImageCount   = 0;
   UserHookData.FrameSizeX          = 0;
   UserHookData.FrameSizeY          = 0;
   UserHookData.FramePacketSize     = 0;
   UserHookData.FramePixelFormat    = 0;
   UserHookData.DataFormatChanged   = false;

   /* Print info related to the device we are connected to. */
   PrintCameraInfo(&UserHookData);

   /* Start the processing. The processing function is called for every frame grabbed. */
   MdigProcess(UserHookData.MilDigitizer, UserHookData.MilGrabBufferList,
      UserHookData.MilGrabBufferListSize, M_START, M_DEFAULT, ProcessingFunction,
      &UserHookData);

   /* NOTE: Now the main() is free to perform other tasks 
                                                    while the processing is executing. */
   /* -------------------------------------------------------------------------------- */

   /* Adjust the slave digitizer according to the master digitizer's status. */
   AdaptToMulticastMasterStatus(MilSystem, &UserHookData);

   MdigInquire(UserHookData.MilDigitizer, M_DIG_PROCESS_IN_PROGRESS, &DigProcessInProgress);
   if(DigProcessInProgress == M_TRUE)
      {
      /* Stop the processing. */
      MdigProcess(UserHookData.MilDigitizer, UserHookData.MilGrabBufferList,
         UserHookData.MilGrabBufferListSize, M_STOP, M_DEFAULT, ProcessingFunction,
         &UserHookData);
      }


   /* Print statistics. */
   MdigInquire(UserHookData.MilDigitizer, M_PROCESS_FRAME_COUNT,  &ProcessFrameCount);
   MdigInquire(UserHookData.MilDigitizer, M_PROCESS_FRAME_RATE,   &ProcessFrameRate);
   MosPrintf(MIL_TEXT("\n\n%lld frames grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
                          (long long)ProcessFrameCount, ProcessFrameRate, 1000.0/ProcessFrameRate);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   FreeGrabBuffers(&UserHookData);

   MdispFree(UserHookData.MilDisplay);
   MdigFree(UserHookData.MilDigitizer);

   MthrFree(UserHookData.Event);

   /* Release defaults. */
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
}

/* Allocate acquisition and display buffers.                                 */
/* -----------------------------------------------------------------------   */
void AllocateGrabBuffers(MIL_INT MilSystem, HookDataStruct* HookDataPtr)
   {
   MIL_INT n = 0;
   MdigInquire(HookDataPtr->MilDigitizer, M_SOURCE_DATA_FORMAT,
      &HookDataPtr->SourceDataFormat);

   /* Allocate the display buffer and clear it. */
   MbufAllocColor(MilSystem,
                  MdigInquire(HookDataPtr->MilDigitizer, M_SIZE_BAND, M_NULL),
                  MdigInquire(HookDataPtr->MilDigitizer, M_SIZE_X, M_NULL),
                  MdigInquire(HookDataPtr->MilDigitizer, M_SIZE_Y, M_NULL),
                  MdigInquire(HookDataPtr->MilDigitizer, M_TYPE, M_NULL),
                  M_IMAGE+M_DISP+M_GRAB+M_PROC+HookDataPtr->SourceDataFormat,
                  &HookDataPtr->MilImageDisp);
   MbufClear(HookDataPtr->MilImageDisp, M_COLOR_BLACK);

   /* Allocate the grab buffers and clear them. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for(HookDataPtr->MilGrabBufferListSize = 0; 
      HookDataPtr->MilGrabBufferListSize<BUFFERING_SIZE_MAX;
      HookDataPtr->MilGrabBufferListSize++)
      {
      MbufAllocColor(MilSystem,
                     MdigInquire(HookDataPtr->MilDigitizer, M_SIZE_BAND, M_NULL),
                     MdigInquire(HookDataPtr->MilDigitizer, M_SIZE_X, M_NULL),
                     MdigInquire(HookDataPtr->MilDigitizer, M_SIZE_Y, M_NULL),
                     MdigInquire(HookDataPtr->MilDigitizer, M_TYPE, M_NULL),
                     M_IMAGE+M_GRAB+M_PROC+HookDataPtr->SourceDataFormat,
                     &HookDataPtr->MilGrabBufferList[HookDataPtr->MilGrabBufferListSize]);

      if (HookDataPtr->MilGrabBufferList[HookDataPtr->MilGrabBufferListSize])
         {
         MbufClear(HookDataPtr->MilGrabBufferList[HookDataPtr->MilGrabBufferListSize],
            M_COLOR_WHITE);
         }
      else
         break;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);


   }

/* Free MIL acquisition and display buffers.                                 */
/* -----------------------------------------------------------------------   */
void FreeGrabBuffers(HookDataStruct* HookDataPtr)
   {
   while(HookDataPtr->MilGrabBufferListSize > 0)
      MbufFree(HookDataPtr->MilGrabBufferList[--HookDataPtr->MilGrabBufferListSize]);

   MbufFree(HookDataPtr->MilImageDisp);
   }

/* This routine queries periodically to determine if a Multicast master      */
/* digitizer is connected to the device this slave digitizer connects to.    */
/* If the Multicast master digitizer connection status changes, then this    */
/* information is printed to the user. When the Multicast master digitizer   */
/* connects back to this device, the slave digitizer will update its         */
/* connection and resume image acquisition.                                  */
/* -----------------------------------------------------------------------   */
void AdaptToMulticastMasterStatus(MIL_INT MilSystem, HookDataStruct* HookDataPtr)
   {
   MIL_INT IsConnected = M_FALSE, IsConnectedOld = M_FALSE;
   MIL_INT ProcessedImageCount = 0;
   MIL_INT DigProcessInProgress = M_FALSE;
   bool Done = false, GrabStopped = false;;

   /* Inquire the connection status of a Multicast master that might be */
   /* connected to this GigE Vision device. */
   MdigInquire(HookDataPtr->MilDigitizer, M_GC_MULTICAST_MASTER_CONNECTED, &IsConnectedOld);
   IsConnected = IsConnectedOld;
   PrintMasterStatusInfo(IsConnectedOld);

   do
      {
      /* Save the current processing count. */
      ProcessedImageCount = HookDataPtr->ProcessedImageCount;
      
      /* Sleep. */
      MthrWait(HookDataPtr->Event, M_EVENT_WAIT+M_EVENT_TIMEOUT(1000), M_NULL);

      GrabStopped = (ProcessedImageCount == HookDataPtr->ProcessedImageCount);

      if(GrabStopped && HookDataPtr->DataFormatChanged == false)
         {
         /* We are not grabbing anymore and we have not detected a data format change.
            Check if master is still connected. */
         MdigInquire(HookDataPtr->MilDigitizer, M_GC_MULTICAST_MASTER_CONNECTED, &IsConnected);
         }

      /* Validate if processing count has changed. */
      if(((IsConnected == M_TRUE) && (IsConnectedOld != IsConnected)) ||
         HookDataPtr->DataFormatChanged)
         {
         // Reset variable
         HookDataPtr->DataFormatChanged = !HookDataPtr->DataFormatChanged;

         /* The Multicast master is present, we must:             */
         /* 1- Stop any grabs that had previously been started.   */
         /* 2- Update the Multicast slave's network sockets       */
         /* 3- Restart the grab.                                  */

         MdigInquire(HookDataPtr->MilDigitizer, M_DIG_PROCESS_IN_PROGRESS,
            &DigProcessInProgress);
         if(DigProcessInProgress)
            {
            MdigProcess(HookDataPtr->MilDigitizer, HookDataPtr->MilGrabBufferList,
               HookDataPtr->MilGrabBufferListSize, M_STOP, M_DEFAULT, ProcessingFunction,
               HookDataPtr);
            }

         MdigControl(HookDataPtr->MilDigitizer, M_GC_UPDATE_MULTICAST_INFO, M_DEFAULT);

         PrintCameraInfo(HookDataPtr);
         FreeGrabBuffers(HookDataPtr);
         AllocateGrabBuffers(MilSystem, HookDataPtr);
         MdispSelect(HookDataPtr->MilDisplay, HookDataPtr->MilImageDisp);

         /* We are now ready to start grabbing. */
         MdigProcess(HookDataPtr->MilDigitizer, HookDataPtr->MilGrabBufferList,
            HookDataPtr->MilGrabBufferListSize, M_START, M_DEFAULT, ProcessingFunction,
            HookDataPtr);
         }

      /* Back-up the current Multicast master connection status. */
      IsConnectedOld = IsConnected;

      /* Tell the user what is happening. */
      PrintMasterStatusInfo(IsConnected);

      /* Must we quit? */
      if(MosKbhit())
         {
         MosGetch();
         Done = true;
         }
      }
   while(!Done);
   }

/* Prints information regarding the device this slave digitizer is connected to. */
/* -----------------------------------------------------------------------       */
void PrintCameraInfo(HookDataStruct* HookDataPtr)
   {
   MIL_STRING PixelFormat;
   MIL_STRING MulticastAddress;
#if M_MIL_USE_WINDOWS
   /* Clear console. */
   system("cls");
#endif

   if(HookDataPtr->DeviceVendor.empty() && HookDataPtr->DeviceModel.empty())
      {
      /* Inquire camera vendor name. */
      MdigInquire(HookDataPtr->MilDigitizer, M_CAMERA_VENDOR, HookDataPtr->DeviceVendor);
      
      /* Inquire camera model name. */
      MdigInquire(HookDataPtr->MilDigitizer, M_CAMERA_MODEL, HookDataPtr->DeviceModel);
      }

   if(HookDataPtr->FrameSizeX == 0 && HookDataPtr->FrameSizeY == 0)
      {
      MdigInquire(HookDataPtr->MilDigitizer, M_SIZE_X, &HookDataPtr->FrameSizeX);
      MdigInquire(HookDataPtr->MilDigitizer, M_SIZE_Y, &HookDataPtr->FrameSizeY);
      }

   if(HookDataPtr->FramePixelFormat == 0)
      {
      MdigInquireFeature(HookDataPtr->MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PixelFormat"), M_TYPE_INT64, &HookDataPtr->FramePixelFormat);

      MdigInquireFeature(HookDataPtr->MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PixelFormat"), M_TYPE_STRING, PixelFormat);
      }

   /* Inquire the Multicast address used. */
   MdigInquire(HookDataPtr->MilDigitizer, M_GC_STREAM_CHANNEL_MULTICAST_ADDRESS_STRING, MulticastAddress);

   /* Print camera info. */
   MosPrintf(MIL_TEXT("\n--------------------- Slave digitizer connection status. "));
   MosPrintf(MIL_TEXT("---------------------\n\n"));
   MosPrintf(MIL_TEXT("Connected to             %s %s\n"), HookDataPtr->DeviceVendor.c_str(),
      HookDataPtr->DeviceModel.c_str());
   MosPrintf(MIL_TEXT("Device pixel format:     %s\n"), PixelFormat.c_str());
   MosPrintf(MIL_TEXT("Device AOI:              %lld x %lld\n"), (long long)HookDataPtr->FrameSizeX,
      (long long)HookDataPtr->FrameSizeY);
   MosPrintf(MIL_TEXT("Multicast address:       %s\n"), MulticastAddress.c_str());

   MosPrintf(MIL_TEXT("\nPress <Enter> to stop.\n\n"));
   }

/* Prints whether a Multicast master digitizer is connected.              */
/* -----------------------------------------------------------------------*/
void PrintMasterStatusInfo(MIL_INT IsConnected)
   {
   MosPrintf(MIL_TEXT("Master digitizer status: "));
   if(IsConnected)
      MosPrintf(MIL_TEXT("connected\r"));
   else
      MosPrintf(MIL_TEXT("not connected --- waiting...\r"));
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
   MIL_INT FrameSizeX = 0;
   MIL_INT FrameSizeY = 0;
   MIL_INT FramePixelFormat = 0;
   MIL_INT FramePacketSize = 0;

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID,   &ModifiedBufferId);
   MdigGetHookInfo(HookId, M_CORRUPTED_FRAME,               &IsFrameCorrupt);
   MdigGetHookInfo(HookId, M_GC_FRAME_SIZE_X,               &FrameSizeX);
   MdigGetHookInfo(HookId, M_GC_FRAME_SIZE_Y,               &FrameSizeY);
   MdigGetHookInfo(HookId, M_GC_FRAME_PIXEL_TYPE,           &FramePixelFormat);
   MdigGetHookInfo(HookId, M_GC_PACKET_SIZE,                &FramePacketSize);

   /* Print and draw the frame count. */
   UserHookDataPtr->ProcessedImageCount++;
   if(IsFrameCorrupt)
      UserHookDataPtr->CorruptImageCount++;

   if((FrameSizeX       != UserHookDataPtr->FrameSizeX) ||
      (FrameSizeY       != UserHookDataPtr->FrameSizeY) ||
      (FramePixelFormat != UserHookDataPtr->FramePixelFormat) ||
      (FramePacketSize  != UserHookDataPtr->FramePacketSize))
      {
      UserHookDataPtr->FrameSizeX = FrameSizeX;
      UserHookDataPtr->FrameSizeY = FrameSizeY;
      UserHookDataPtr->FramePixelFormat = FramePixelFormat;
      UserHookDataPtr->FramePacketSize = FramePacketSize;

      // Do not set on first grab, we must initialize data once first.
      if (UserHookDataPtr->ProcessedImageCount > 1)
         {
         UserHookDataPtr->DataFormatChanged = M_TRUE;
         // Wake up main thread to perform buffer re-allocation.
         MthrControl(UserHookDataPtr->Event, M_EVENT_SET, M_SIGNALED);
         }
      }

   MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%lld"), 
                                       (long long)UserHookDataPtr->ProcessedImageCount);
   MgraText(M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, Text);

   /* Perform the processing and update the display. */
   MbufCopy(ModifiedBufferId, UserHookDataPtr->MilImageDisp);
   
   
   return 0;
   }
