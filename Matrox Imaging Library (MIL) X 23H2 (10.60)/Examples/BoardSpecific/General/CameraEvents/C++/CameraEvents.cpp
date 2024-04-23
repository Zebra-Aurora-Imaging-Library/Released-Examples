﻿/********************************************************************************/
/*
* File name: CameraEvents.cpp
*
* Synopsis:  This program demonstrates new MIL features for managing GenICam(c)
*            devices. This program focuses on hooking a MIL handler to asynchronous
*            camera events.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

/* Headers. */
#include <mil.h>
#include <vector>

bool SystemSupported(MIL_ID MilSystem);

/******************************************************************************/
/* Example description.                                                       */
/******************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n\n"));
   MosPrintf(MIL_TEXT("Camera Events\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n\n"));
   MosPrintf(
      MIL_TEXT("This program demonstrates registering for asynchronous\n")\
      MIL_TEXT("camera event notifications.\n\n")\
      MIL_TEXT("Press <Enter> to start.\n\n")\
      );
   }

struct HookDataStruct
   {
   MIL_ID  MilDigitizer;
   MIL_ID  MilImageDisp;
   MIL_INT ProcessedImageCount;
   std::vector<MIL_STRING> Events;
   MIL_INT EventCount;
   MIL_INT NbEventsReceived;
   MIL_DOUBLE TimeStamp;
   };

/* Gets and prints the events supported by the camera. */
void GetCameraEventControls(MIL_ID MilDigitizer, MIL_INT& SupportedEventCount,
                            std::vector<MIL_STRING>& Events, bool& StandardEventEnable);
/* Hooks a MIL function callback to camera events. */
void HookToEvent(MIL_ID MilDigitizer, const MIL_STRING& Event, void* HookDataPtr,
                 bool StandardEventEnable, MIL_INT Unhook = 0);
/* Does a lookup of the event name that triggered the callback. */
void EventTypeLookUp(MIL_ID MilDigitizer, MIL_ID HookId, HookDataStruct* HookDataPtr);
/* Starts image acquisition. */
void DoAcquisition(MIL_ID MilSystem, MIL_ID MilDigitizer, MIL_ID MilImageDisp);

/* User's processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType,
                                  MIL_ID HookId,
                                  void* HookDataPtr);

/* User's callback function prototype. Called when a camera event fires. */
MIL_INT MFTYPE CameraEventHandler(MIL_INT HookType,
                                  MIL_ID HookId,
                                  void* HookDataPtr);


/* Main function. */
int MosMain(void)
   {
   MIL_ID MilApplication,  /* Application identifier.  */
      MilSystem,       /* System identifier.       */
      MilDisplay,      /* Display identifier.      */
      MilDigitizer,    /* Digitizer identifier.    */
      MilImage;        /* Image buffer identifier. */
   MIL_INT EventCount, Done = 0;
   std::vector<MIL_STRING> Events;
   int Selection = 0;
   bool StandardEventEnable = false;
   HookDataStruct UserHookData;

   /* Print a message. */
   PrintHeader();
   MosGetch();

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, &MilDigitizer, M_NULL);

   MbufAllocColor(MilSystem,
                  MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL),
                  MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
                  MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
                  MdigInquire(MilDigitizer, M_TYPE, M_NULL),
                  M_IMAGE + M_DISP + M_GRAB,
                  &MilImage);

   MosPrintf(MIL_TEXT("------------------------------------------------------------\n\n"));

   if (!SystemSupported(MilSystem))
      {
      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage);
      return 1;
      }

   /* Disable error printing in case camera is not
      Standard Feature Naming Convention compliant. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);

   /* Get the list of events supported by the camera. */
   GetCameraEventControls(MilDigitizer, EventCount, Events, StandardEventEnable);

   /* Re-enable error printing. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   /* If camera supports events, ask the user to select an event to use. */
   if (EventCount)
      {
      char InputStream[64] = { '\0' };
      do
         {
         MosPrintf(MIL_TEXT("\nPlease select the event you wish to hook a function to: "));
         fgets(InputStream, sizeof(InputStream), stdin);
#if M_MIL_USE_WINDOWS
         int result = sscanf_s(InputStream, "%d", &Selection);
#else
         int result = sscanf(InputStream, "%d", &Selection);
#endif
         if ((result == 1) && (Selection >= 0) && (Selection < EventCount))
            Done = 1;
         else
            MosPrintf(MIL_TEXT("\nInvalid selection"));
         } while (!Done);

         UserHookData.MilDigitizer = MilDigitizer;
         UserHookData.MilImageDisp = MilImage;
         UserHookData.ProcessedImageCount = 0;
         UserHookData.Events = Events;
         UserHookData.EventCount = EventCount;
         UserHookData.NbEventsReceived = 0;
         UserHookData.TimeStamp = 0.0;

         /* Hook a callback to the camera's event. */
         HookToEvent(MilDigitizer, Events[Selection], &UserHookData, StandardEventEnable);

         MosPrintf(MIL_TEXT("\nAwaiting %s events.\n"), Events[Selection].c_str());
         MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
         MosGetch();

         /* Start a continuous acquisition. */
         MdispSelect(MilDisplay, MilImage);

         /* Some events such as ExposureStart, ExposureEnd, ..., require an acquisition
            in order to be generated, therefore start an acquisition. */
         MosPrintf(MIL_TEXT("\nPress <Enter> to quit.\n"));
         DoAcquisition(MilSystem, MilDigitizer, MilImage);
         MosGetch();
         MosPrintf(MIL_TEXT("\n"));

         /* Unhook MIL callback from event. */
         HookToEvent(MilDigitizer, Events[Selection], &UserHookData, StandardEventEnable, M_UNHOOK);
      }

   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage);

   return 0;
   }

bool SystemSupported(MIL_ID MilSystem)
   {
   MIL_INT GenICamSupport = M_FALSE;

   MsysInquire(MilSystem, M_GENICAM_AVAILABLE, &GenICamSupport);

   if (!GenICamSupport)
      {
      MosPrintf(MIL_TEXT("This example program can only be used with Matrox Drivers that ")
                  MIL_TEXT("support GenICam.\n"));
      MosPrintf(MIL_TEXT("Please ensure that the default system type is set accordingly in ")
                  MIL_TEXT("MIL Config.\n"));
      MosPrintf(MIL_TEXT("-------------------------------------------------------------\n\n"));
      MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
      MosGetch();
      }

   return GenICamSupport == M_TRUE;
   }

/* Gets and prints the events supported by the camera. */
void GetCameraEventControls(MIL_ID MilDigitizer, MIL_INT& SupportedEventCount,
                            std::vector<MIL_STRING>& Events, bool& StandardEventEnable)
   {
   MIL_STRING CameraVendor;
   MIL_STRING CameraModel;
   std::vector<MIL_STRING> EventNotification;
   MIL_INT EventCnt = 0, Cnt = 0;
   bool SupportsOn = false, SupportsOff = false;

   StandardEventEnable = false;

   /* Inquire general device information such as device vendor and name. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceVendorName"), M_TYPE_STRING, CameraVendor);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceModelName"), M_TYPE_STRING, CameraModel);

   /* Inquire supported events. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("EventSelector"), M_TYPE_MIL_INT, &EventCnt);
   for (MIL_INT i = 0; i < EventCnt; i++)
      {
      MIL_INT64 AccessMode = 0;
      MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_ACCESS_MODE + i, MIL_TEXT("EventSelector"), M_TYPE_INT64, &AccessMode);
      if (M_FEATURE_IS_AVAILABLE(AccessMode))
         {
         MIL_STRING EventName;
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("EventSelector"), M_TYPE_STRING, EventName);
         Events.push_back(EventName);
         }
      }

   /* Validate if device has an EventId node. */
   MIL_STRING EventId;
   bool SupportEventId = true;
   MIL_BOOL Present = M_FALSE;
   for (size_t i = 0; i < Events.size(); i++)
      {
      EventId = MIL_STRING(MIL_TEXT("Event")) + Events[i];
      MdigInquireFeature(MilDigitizer, M_FEATURE_PRESENT, EventId, M_TYPE_BOOLEAN, &Present);
      if (!Present)
         SupportEventId = false;
      else
         SupportEventId = true;
      }

   /*Validate if device supports standard event notification mechanism. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("EventNotification"), M_TYPE_MIL_INT, &Cnt);
   if (Cnt)
      {
      MIL_STRING Val;
      for (MIL_INT i = 0; i < Cnt; i++)
         {
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("EventNotification"), M_TYPE_STRING, Val);
         EventNotification.push_back(Val);

         if (EventNotification[i] == MIL_TEXT("On"))
            SupportsOn = true;
         else if (EventNotification[i] == MIL_TEXT("Off"))
            SupportsOff = true;
         }

      if (SupportsOn && SupportsOff && SupportEventId)
         StandardEventEnable = true;
      }

   /* Print data inquired above. */
   MosPrintf(MIL_TEXT("%20s %s %s\n"), MIL_TEXT("Connected to camera:"),
             (!CameraVendor.empty()) ? CameraVendor.c_str() : MIL_TEXT("N/A"), (!CameraModel.empty()) ? CameraModel.c_str() : MIL_TEXT("N/A"));
   MosPrintf(MIL_TEXT("%20s "), MIL_TEXT("Supported events:"));

   /* If camera supports events, ask the user to which event to hook to. */
   if (Events.size() == 0)
      {
      MosPrintf(MIL_TEXT("Your camera does not support events.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to quit.\n\n"));
      MosGetch();
      }
   else
      {
      MosPrintf(MIL_TEXT("(0) %s\n"), Events[0].c_str());
      for (size_t i = 1; i < Events.size(); i++)
         MosPrintf(MIL_TEXT("%20s (%d) %s\n"), MIL_TEXT(""), (int)i, Events[i].c_str());
      }

   SupportedEventCount = EventCnt;
   }

/* Hooks a MIL function callback to camera events. */
void HookToEvent(MIL_ID MilDigitizer, const MIL_STRING& Event, void* HookDataPtr,
                 bool StandardEventEnable, MIL_INT Unhook /*= 0*/)
   {
   MIL_INT MilHookType = 0;
   MIL_INT Error = M_NULL_ERROR;
   bool UnknownEventType = false;
   
   if (Unhook)
      Unhook = M_UNHOOK;

   if (StandardEventEnable)
      {
      /* If the camera supports standard event notification we can simply hook user MdigHookFunction with the event name. */
      if (Event == MIL_TEXT("AcquisitionTrigger"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_ACQUISITION_TRIGGER + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("AcquisitionStart"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_ACQUISITION_START + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("AcquisitionEnd"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_ACQUISITION_END + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("AcquisitionTransferStart"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_ACQUISITION_TRANSFER_START + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("AcquisitionTransferEnd"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_ACQUISITION_TRANSFER_END + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("AcquisitionError"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_ACQUISITION_ERROR + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("FrameTrigger"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_FRAME_TRIGGER + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("FrameStart"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_FRAME_START + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("FrameEnd"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_FRAME_END + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("FrameTransferStart"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_FRAME_TRANSFER_START + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("FrameTransferEnd"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_FRAME_TRANSFER_END + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("ExposureStart"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_EXPOSURE_START + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event == MIL_TEXT("ExposureEnd"))
         {
         MdigHookFunction(MilDigitizer, M_GC_EVENT + M_EXPOSURE_END + Unhook, CameraEventHandler, HookDataPtr);
         }
      else if (Event.compare(0, 7, MIL_TEXT("Counter")) == 0)
         {
         /* For counter events we must generate the string that targets the proper counter instance. */
         MIL_STRING String1;
         MIL_STRING String2;
         MIL_INT Count = 0;
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("CounterSelector"), M_TYPE_MIL_INT, &Count);
         for (int i = 0; i < Count && MilHookType == 0; i++)
            {
               {
               MIL_STRING_STREAM stream;
               stream << MIL_TEXT("Counter") << i << MIL_TEXT("Start");
               String1 = stream.str();
               }
               {
               MIL_STRING_STREAM stream;
               stream << MIL_TEXT("Counter") << i << MIL_TEXT("End");
               String2 = stream.str();
               }

            if (Event == String1)
               MilHookType = M_GC_EVENT + M_COUNTER_START + i;
            else if (Event == String2)
               MilHookType = M_GC_EVENT + M_COUNTER_END + i;
            }

         if (MilHookType)
            MdigHookFunction(MilDigitizer, MilHookType + Unhook, CameraEventHandler, HookDataPtr);
         else
            UnknownEventType = true;
         }
      else if (Event.compare(0, 5, MIL_TEXT("Timer")) == 0)
         {
         /* For timer events we must generate the string that targets the proper timer instance. */
         MIL_STRING String1;
         MIL_STRING String2;
         MIL_INT Count = 0;
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("TimerSelector"), M_TYPE_MIL_INT, &Count);
         for (int i = 0; i < Count && MilHookType == 0; i++)
            {
               {
               MIL_STRING_STREAM stream;
               stream << MIL_TEXT("Timer") << i << MIL_TEXT("Start");
               String1 = stream.str();
               }
               {
               MIL_STRING_STREAM stream;
               stream << MIL_TEXT("Timer") << i << MIL_TEXT("End");
               String2 = stream.str();
               }
            if (Event == String1)
               MilHookType = M_GC_EVENT + M_TIMER_START + i;
            else if (Event == String2)
               MilHookType = M_GC_EVENT + M_TIMER_END + i;
            }

         if (MilHookType)
            MdigHookFunction(MilDigitizer, MilHookType + Unhook, CameraEventHandler, HookDataPtr);
         else
            UnknownEventType = true;
         }
      else if (Event.compare(0, 4, MIL_TEXT("Line")) == 0)
         {
         /* For Line events we must generate the string that targets the proper Line instance. */
         MIL_STRING String1;
         MIL_STRING String2;
         MIL_STRING String3;
         MIL_INT Count = 0;
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("LineSelector"), M_TYPE_MIL_INT, &Count);
         for (int i = 0; i < Count && MilHookType == 0; i++)
            {
               {
               MIL_STRING_STREAM stream;
               stream << MIL_TEXT("Line") << i << MIL_TEXT("RisingEdge");
               String1 = stream.str();
               }
               {
               MIL_STRING_STREAM stream;
               stream << MIL_TEXT("Line") << i << MIL_TEXT("FallingEdge");
               String2 = stream.str();
               }
               {
               MIL_STRING_STREAM stream;
               stream << MIL_TEXT("Line") << i << MIL_TEXT("AnyEdge");
               String3 = stream.str();
               }

            if (Event == String1)
               MilHookType = M_GC_EVENT + M_LINE_RISING_EDGE + i;
            else if (Event == String2)
               MilHookType = M_GC_EVENT + M_LINE_FALLING_EDGE + i;
            else if (Event == String3)
               MilHookType = M_GC_EVENT + M_LINE_ANY_EDGE + i;
            }

         if (MilHookType)
            MdigHookFunction(MilDigitizer, MilHookType + Unhook, CameraEventHandler, HookDataPtr);
         else
            UnknownEventType = true;
         }
      else
         {
         /* Unknown hook types are handled as generic M_GC_EVENT indicated below. */
         UnknownEventType = true;
         }
      }

   if ((StandardEventEnable == false) || UnknownEventType)
      {
      /* Disable error printing for cameras that do not support the SNFC EventNotification feature. */
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);

      //Hook to a generic (unknown to MIL) event type.
      MdigHookFunction(MilDigitizer, M_GC_EVENT + Unhook, CameraEventHandler, HookDataPtr);

      //Try to enable the event assuming that the "EventNotification" feature is implemented in the camera.
      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("EventSelector"), M_TYPE_STRING, Event);
      if (!Unhook)
         {
         MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("EventNotification"), M_TYPE_STRING, MIL_TEXT("On"));

         MappGetError(M_DEFAULT, M_CURRENT , &Error);
         if (Error != M_NULL_ERROR)
            {
            /* Standard EventNotification support is missing. Print message to user about this. */
            MosPrintf(MIL_TEXT("\nThe %s feature as implemented by the camera manufacturer lacks\n"), Event.c_str());
            MosPrintf(MIL_TEXT("standard \"EventNotification\" support. Make sure the event is enabled using\n"));
            MosPrintf(MIL_TEXT("the feature browser before continuing.\n\n"));
            MosPrintf(MIL_TEXT("Some older camera models might require \"EventNotification\" to be set to\n"));
            MosPrintf(MIL_TEXT("\"GigEVisionEvent\" or \"GenICamEvent\" for event notification to occur.\n"));

            /* Pop up the feature browser, the user should find the feature that enables event notification and set it. */
            MdigControl(MilDigitizer, M_GC_FEATURE_BROWSER, M_OPEN + M_ASYNCHRONOUS);
            }
         }
      else
         MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("EventNotification"), M_TYPE_STRING, MIL_TEXT("Off"));

      MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
      }
   }

/* Does a lookup of the event name that triggered the callback. */
void EventTypeLookUp(MIL_ID MilDigitizer, MIL_ID HookId, HookDataStruct* HookDataPtr)
   {
   MIL_STRING String;
   bool Found = false;
   MIL_DOUBLE EventTimeStamp;
   MIL_INT EventType;
   MIL_INT64 LocalEventType = 0;

   /* Inquire the raw event type and the timestamp when the event occurred. */
   MdigGetHookInfo(HookId, M_GC_EVENT_TYPE, &EventType);
   MdigGetHookInfo(HookId, M_GC_CAMERA_TIME_STAMP, &EventTimeStamp);

   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for (MIL_INT i = 0; i < HookDataPtr->EventCount && !Found; i++)
      {
      /*Lookup the event name from the raw event type. */
      String = MIL_STRING(MIL_TEXT("Event")) + HookDataPtr->Events[i];
      MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, String, M_TYPE_INT64, &LocalEventType);
      if (LocalEventType == EventType)
         {
         MosPrintf(MIL_TEXT("Received %d %s events. Interval from last: %.6f sec.\r"),
                   HookDataPtr->NbEventsReceived, HookDataPtr->Events[i].c_str(), EventTimeStamp - HookDataPtr->TimeStamp);
         Found = true;
         }
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   /* Look-up failed, camera probably does not support EventExposureData features.
      Simply print the raw event type. */
   if (Found == false)
      MosPrintf(MIL_TEXT("%d Received Event #%lld received. Interval from last: %.6f sec.\r"),
         HookDataPtr->NbEventsReceived, (long long)EventType, EventTimeStamp - HookDataPtr->TimeStamp);

   HookDataPtr->TimeStamp = EventTimeStamp;
   }

/* User's processing function hook data structure. */
void DoAcquisition(MIL_ID MilSystem, MIL_ID MilDigitizer, MIL_ID MilImageDisp)
   {
   MIL_INT NbFrames = 10;
   MIL_INT MilGrabBufferListSize;
   MIL_INT Done = 0;
   MIL_INT Ch = 0;
   MIL_ID* MilGrabBufferList = NULL;
   HookDataStruct UserHookData;

   MilGrabBufferList = new MIL_INT[(NbFrames == M_INFINITE) ? 10 : NbFrames];

   /* Allocate the grab buffers and clear them. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for (MilGrabBufferListSize = 0;
        MilGrabBufferListSize < NbFrames; MilGrabBufferListSize++)
      {
      MbufAlloc2d(MilSystem,
                  MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
                  MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
                  8 + M_UNSIGNED,
                  M_IMAGE + M_GRAB + M_PROC,
                  &MilGrabBufferList[MilGrabBufferListSize]);

      if (MilGrabBufferList[MilGrabBufferListSize])
         {
         MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0);
         }
      else
         break;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   /* Initialize the User's processing function data structure. */
   UserHookData.MilDigitizer = MilDigitizer;
   UserHookData.MilImageDisp = MilImageDisp;
   UserHookData.ProcessedImageCount = 0;

   /* Start the processing. The processing function is called for every frame grabbed. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
               M_START, M_DEFAULT, ProcessingFunction, &UserHookData);

   /* NOTE: Now the main() is free to perform other tasks
                                                    while the processing is executing. */
   /* -------------------------------------------------------------------------------- */
   MosGetch();

   /* Stop the processing. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
               Done ? M_STOP : M_STOP + M_WAIT, M_DEFAULT, ProcessingFunction, &UserHookData);

   /* Free the grab buffers. */
   while (MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);

   delete[] MilGrabBufferList;
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
   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX] = { MIL_TEXT('\0'), };

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);

   /* Print and draw the frame count. */
   UserHookDataPtr->ProcessedImageCount++;
   MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%lld"),
              (long long)UserHookDataPtr->ProcessedImageCount);
   MgraText(M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, Text);

   /* Perform the processing and update the display. */
   MbufCopy(ModifiedBufferId, UserHookDataPtr->MilImageDisp);

   return 0;
   }

/* User's callback function prototype. Called when a camera event fires. */
MIL_INT MFTYPE CameraEventHandler(MIL_INT HookType,
                                  MIL_ID HookId,
                                  void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;

   UserHookDataPtr->NbEventsReceived++;
   /* Print info related to the camera event that was fired. */
   EventTypeLookUp(UserHookDataPtr->MilDigitizer, HookId, UserHookDataPtr);

   return 0;
   }