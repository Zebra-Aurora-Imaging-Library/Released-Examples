﻿/***************************************************************************************/
/*
 * File name: MdigGrabDynamic.cpp
 *
 * Synopsis:  This program shows the use of grabbing with M_DYNAMIC buffers.
 *
 *            Dynamic buffers will adapt to what is being grabbed. They can be used
 *            with devices that change the size and/or format of the image output from
 *            one frame to the next. In this case the dynamic buffer's size and format
 *            will change from one grab to the next reflecting the format of the incoming
 *            stream of images sent by the camera.
 *
 *            Dynamic buffers can also be used to acquire a stream of images of unknown
 *            format to MIL. In this case the buffer will contain the raw image sent by
 *            the camera. The dynamic buffer's M_SIZE_X and M_SIZE_Y will reflect the
 *            actual size of the buffer grabbed. But since its internal format is unknown
 *            MIL will not be able to copy it or process it. The user must inquire the
 *            buffer's host address and access the memory directly.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#include <map>

/******************************************************************************/
/* Example description.                                                       */
/******************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n\n"));
   MosPrintf(MIL_TEXT("MdigGrabDynamic\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n\n"));
   MosPrintf(
      MIL_TEXT("This program allocates dynamic buffers for grabbing.\n\n")\
      MIL_TEXT("Dynamic buffers do not have a format defined at allocation time.\n")\
      MIL_TEXT("The format of a dynamic buffer becomes defined once you grab into it.\n")\
      MIL_TEXT("The buffer's rectangular size and format can be inquired using the\n")\
      MIL_TEXT("MbufInquire() function after a grab has been completed.\n\n")\

      MIL_TEXT("If the camera sends buffers of different formats from one frame\n")\
      MIL_TEXT("to the next then the dynamic buffer will change size and format accordingly.\n")\
      MIL_TEXT("The dynamic buffer's size in bytes must be large enough to accommodate the\n")\
      MIL_TEXT("largest image that can be sent by the camera.\n\n")\

      MIL_TEXT("Because of their nature dynamic buffers can be used to grab images\n")\
      MIL_TEXT("whose format is unknown to MIL. In this case the dynamic buffer's rectangular\n")\
      MIL_TEXT("size and PFNC format will reflect the raw data sent by the camera. Because the\n")\
      MIL_TEXT("format is unknown to MIL, MIL will not be able to copy or process this buffer.\n")\
      MIL_TEXT("The user must, in this case, inquire the buffer's host address and access\n")\
      MIL_TEXT("the memory directly.\n\n")\

      MIL_TEXT("Press <Enter> to start.\n\n")\
   );
   }

 /* Number of images in the buffering grab queue.
    Generally, increasing this number gives a better real-time grab.
  */
#define BUFFERING_SIZE_MAX 20

  /* User's processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

typedef struct
   {
   MIL_INT64 Value;
   MIL_STRING Name;
   MIL_STRING DisplayName;
   MIL_STRING Description;
   }PixelFormat_t;

typedef std::map<MIL_INT64, PixelFormat_t> PixelFormats_t;

/* Pixel format enumeration function prototype. */
PixelFormats_t EnumPixelFormats(MIL_ID MilDigitizer);

/* Pixel format selection function prototype. */
void SelectPixelFormat(MIL_ID MilDigitizer, MIL_ID MilDisplay, const PixelFormats_t& PixelFormats);

/* Find max payload size function prototype. */
MIL_INT GetMaxPayloadSize(MIL_ID MilDigitizer, const PixelFormats_t& PixelFormats);

/* User's processing function hook data structure. */
typedef struct
   {
   MIL_ID  MilSystem;
   MIL_ID  MilDigitizer;
   MIL_ID  MilDisplay;
   MIL_ID  MilImageDisp;
   MIL_INT ProcessedImageCount;
   PixelFormats_t PixelFormats;
   } HookDataStruct;

/* Main function. */
/* ---------------*/

int MosMain(void)
   {
   MIL_ID MilApplication;
   MIL_ID MilSystem;
   MIL_ID MilDigitizer;
   MIL_ID MilDisplay;
   MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX] = { 0 };
   MIL_INT MilGrabBufferListSize;
   MIL_INT ProcessFrameCount = 0;
   MIL_DOUBLE ProcessFrameRate = 0;
   HookDataStruct UserHookData;

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay,
                    &MilDigitizer, M_NULL);

   /* Print a message. */
   PrintHeader();
   MosGetch();

   /* Enumerate the PixelFormats supported by the camera. */
   UserHookData.PixelFormats = EnumPixelFormats(MilDigitizer);

   /* Get the maximum size image in bytes that the camera can return. */
   MIL_INT MaxPayloadSize = GetMaxPayloadSize(MilDigitizer, UserHookData.PixelFormats);

   /* Allocate dynamic grab buffers and clear them. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for (MilGrabBufferListSize = 0; MilGrabBufferListSize < BUFFERING_SIZE_MAX;
        MilGrabBufferListSize++)
      {
      MbufAlloc1d(MilSystem,
                  MaxPayloadSize,
                  8 + M_UNSIGNED,
                  M_IMAGE + M_GRAB + M_DYNAMIC,
                  &MilGrabBufferList[MilGrabBufferListSize]);
      if (MilGrabBufferList[MilGrabBufferListSize])
         MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);
      else
         break;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if (MilGrabBufferList[0] == M_NULL)
      {
      MosPrintf(MIL_TEXT("This system type does not support dynamic buffers.\n")),
      MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
      MosGetch();
      MappFreeDefault(MilApplication, MilSystem, MilDisplay,
                       MilDigitizer, M_NULL);
      return 0;
      }

   /* Let the user choose which pixel format to use. */
   SelectPixelFormat(MilDigitizer, MilDisplay, UserHookData.PixelFormats);

   /* Initialize the user's processing function data structure. */
   UserHookData.MilSystem = MilSystem;
   UserHookData.MilDigitizer = MilDigitizer;
   UserHookData.MilDisplay = MilDisplay;
   UserHookData.MilImageDisp = M_NULL;
   UserHookData.ProcessedImageCount = 0;

   /* Start the processing. The processing function is called with every frame grabbed. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
               M_START, M_DEFAULT, ProcessingFunction, &UserHookData);


   /* Here the main() is free to perform other tasks while the processing is executing. */
   /* --------------------------------------------------------------------------------- */


   /* Print a message and wait for a key press after a minimum number of frames. */
   MosPrintf(MIL_TEXT("Press <Enter> to stop.                    \n\n"));
   MosGetch();

   /* Stop the processing. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
               M_STOP, M_DEFAULT, ProcessingFunction, &UserHookData);

   /* Print statistics. */
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT, &ProcessFrameCount);
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &ProcessFrameRate);
   MosPrintf(MIL_TEXT("\n\n%d frames grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
      (int)ProcessFrameCount, ProcessFrameRate, 1000.0 / ProcessFrameRate);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Free the grab buffers. */
   while (MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);

   /* Free display buffer. */
   if (UserHookData.MilImageDisp != M_NULL)
      {
      MbufFree(UserHookData.MilImageDisp);
      }

   /* Release defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, M_NULL);

   return 0;
   }

/* User's processing function called every time a grab buffer is ready. */
/* -------------------------------------------------------------------- */

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_INT SizeX = 0, SizeY = 0, PfncFormat = 0;
   MIL_STRING PixelFormatName;

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);

   /* Inquire the dynamic buffer's size and PFNC format.*/
   MbufInquire(ModifiedBufferId, M_SIZE_X, &SizeX);
   MbufInquire(ModifiedBufferId, M_SIZE_Y, &SizeY);
   MbufInquire(ModifiedBufferId, M_PFNC_VALUE, &PfncFormat);

   /* Look-up the name of the buffers format sent by the camera. */
   PixelFormats_t::iterator It = UserHookDataPtr->PixelFormats.find(PfncFormat);
   if (It != UserHookDataPtr->PixelFormats.end())
      {
      PixelFormatName = It->second.DisplayName;
      }

   /* Increment the frame counter. */
   UserHookDataPtr->ProcessedImageCount++;

   MosPrintf(MIL_TEXT("#%d frames grabbed. SizeX: %d SizeY: %d Format: %s\r"),
      (int)UserHookDataPtr->ProcessedImageCount, (int)SizeX, (int)SizeY, PixelFormatName.c_str());

   /* Inquire if MIL can handle this type of format. */
   MIL_INT IsMilSupported = M_NO;
   MbufInquire(ModifiedBufferId, M_PFNC_SUPPORT, &IsMilSupported);

   if ((IsMilSupported == M_YES) || (IsMilSupported == M_WITH_COMPENSATION))
      {
      if (UserHookDataPtr->ProcessedImageCount == 1)
         {
         /* Allocate a display buffer. */
         MbufAllocColor(UserHookDataPtr->MilSystem,
                        MbufInquire(ModifiedBufferId, M_SIZE_BAND, M_NULL),
                        MbufInquire(ModifiedBufferId, M_SIZE_X, M_NULL),
                        MbufInquire(ModifiedBufferId, M_SIZE_Y, M_NULL),
                        MbufInquire(ModifiedBufferId, M_TYPE, M_NULL),
                        M_IMAGE + M_DISP,
                        &UserHookDataPtr->MilImageDisp);
         MbufClear(UserHookDataPtr->MilImageDisp, M_COLOR_BLACK);
         MdispSelect(UserHookDataPtr->MilDisplay, UserHookDataPtr->MilImageDisp);
         }

      MbufCopy(ModifiedBufferId, UserHookDataPtr->MilImageDisp);
      }

   return 0;
   }

/* Utility function used to enumerate the camera's pixel formats. */
/* -------------------------------------------------------------------- */
PixelFormats_t EnumPixelFormats(MIL_ID MilDigitizer)
   {
   PixelFormats_t PixelFormats;
   MIL_INT Count;

   MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("PixelFormat"), M_TYPE_MIL_INT, &Count);
   for (MIL_INT i = 0; i < Count; i++)
      {
      MIL_INT64 AccessMode = 0;

      MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_ACCESS_MODE + i, MIL_TEXT("PixelFormat"), M_TYPE_INT64, &AccessMode);

      if (M_FEATURE_IS_AVAILABLE(AccessMode))
         {
         PixelFormat_t PixelFormat;
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, MIL_TEXT("PixelFormat"), M_TYPE_STRING, PixelFormat.Name);
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_DISPLAY_NAME + i, MIL_TEXT("PixelFormat"), M_TYPE_STRING, PixelFormat.DisplayName);
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_DESCRIPTION + i, MIL_TEXT("PixelFormat"), M_TYPE_STRING, PixelFormat.Description);
         MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_VALUE + i, MIL_TEXT("PixelFormat"), M_TYPE_MIL_INT, &PixelFormat.Value);
         PixelFormats[PixelFormat.Value] = PixelFormat;
         }
      }

   return PixelFormats;
   }

/* Pixel format selection function definition. */
void SelectPixelFormat(MIL_ID MilDigitizer, MIL_ID MilDisplay, const PixelFormats_t& PixelFormats)
   {
   size_t i = 0;
   std::vector<PixelFormat_t> Formats;
   MosPrintf(MIL_TEXT("Select a pixel format to use:\n"));
   MosPrintf(MIL_TEXT("-----------------------------\n"));

   PixelFormats_t::const_iterator It = PixelFormats.cbegin();
   for (i = 0; It != PixelFormats.cend(); ++It, i++)
      {
      Formats.push_back(It->second);
      MosPrintf(MIL_TEXT("%2d %-20.20s %-55.55s\n"), i + 1, It->second.DisplayName.c_str(), It->second.Description.c_str());
      }

   MosPrintf(MIL_TEXT("\n"));
   int Selection = 0;
   bool Done = false;
   do
      {
      char InputStream[64] = { '\0' };
      fgets(InputStream, sizeof(InputStream), stdin);
#if M_MIL_USE_WINDOWS
      int result = sscanf_s(InputStream, "%d", &Selection);
#else
      int result = sscanf(InputStream, "%d", &Selection);
#endif
      if ((result == 1) && (Selection >= 0) && (((size_t)Selection) <= i))
         Done = true;
      else
         MosPrintf(MIL_TEXT("\nInvalid selection\n"));
      }
   while (!Done);

   MosPrintf(MIL_TEXT("Using %s pixel format and starting grab.\n"), Formats[Selection - 1].DisplayName.c_str());
   MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PixelFormat"), M_TYPE_STRING, Formats[Selection - 1].Name);

   /* Disable errors while inquiring M_SIZE_BIT in case an MIL unknown PixelFormat was selected. */
   MappControl(M_ERROR + M_THREAD_CURRENT, M_PRINT_DISABLE);
   MIL_INT SizeBit = MdigInquire(MilDigitizer, M_SIZE_BIT, M_NULL);
   MIL_INT Error = MappGetError(M_CURRENT, M_NULL);
   MappControl(M_ERROR + M_THREAD_CURRENT, M_PRINT_ENABLE);

   if ((Error == M_NULL_ERROR) && (SizeBit > 8))
      {
      if (SizeBit <= 16)
         {
         MdispControl(MilDisplay, M_VIEW_MODE, M_BIT_SHIFT);
         MdispControl(MilDisplay, M_VIEW_BIT_SHIFT, SizeBit - 8);
         }
      else
         {
         MdispControl(MilDisplay, M_VIEW_MODE, M_AUTO_SCALE);
         }
      }
   }

/* Find max payload size function prototype. */
MIL_INT GetMaxPayloadSize(MIL_ID MilDigitizer, const PixelFormats_t& PixelFormats)
   {
   MIL_INT MaxPayloadSize = 0;
   MIL_STRING PixelFormatBackup;
   /* Back-up original pixel format. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PixelFormat"), M_TYPE_STRING, PixelFormatBackup);

   /* Cycle pixel formats to find largest payload size. */
   PixelFormats_t::const_iterator It = PixelFormats.cbegin();
   for (; It != PixelFormats.cend(); ++It)
      {
      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PixelFormat"), M_TYPE_STRING, It->second.Name);
      MIL_INT PayloadSize = MdigInquire(MilDigitizer, M_GC_PAYLOAD_SIZE, M_NULL);
      if (MaxPayloadSize < PayloadSize)
         MaxPayloadSize = PayloadSize;
      }

   /* Restore original pixel format. */
   MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("PixelFormat"), M_TYPE_STRING, PixelFormatBackup);
   return MaxPayloadSize;
   }
