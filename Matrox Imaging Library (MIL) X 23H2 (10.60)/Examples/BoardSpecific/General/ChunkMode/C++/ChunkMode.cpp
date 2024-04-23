/*************************************************************************************/
/*
 * File name: ChunkMode.cpp
 *
 * Synopsis:  This program shows the use of the MdigProcess() function to perform
 *            real-time acquisition. It also enables your GenICam(c) device in chunk
 *            mode (if supported).
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#include <vector>

 /******************************************************************************/
 /* Example description.                                                       */
 /******************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n\n"));
   MosPrintf(MIL_TEXT("ChunkMode\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n\n"));
   MosPrintf(
      MIL_TEXT("This program shows the use of the MdigProcess function to perform\n")\
      MIL_TEXT("real-time acquisition. It also enables GenICam(c) device chunk mode\n")\
      MIL_TEXT("if supported by your device.\n\n")\

      MIL_TEXT("Press <Enter> to start.\n\n")\
   );
   }

/* Number of images in the buffering grab queue.
   Generally, increasing this number gives better real-time grab.
   */
#define BUFFERING_SIZE_MAX 10

/* User's processing function hook data structure. */
struct HookDataStruct
   {
   MIL_ID  MilDigitizer;
   MIL_ID  MilImageDisp;
   MIL_INT ProcessedImageCount;
   MIL_STRING ChunkDataName;
   };

/* Verifies if this example can run on the selected system. */
bool SystemSupportsGenICam(MIL_ID MilSystem);

bool IsChunkModeAvailable(MIL_ID MilDigitizer);
MIL_STRING ChooseAndEnableChunkMode(MIL_ID MilDigitizer, HookDataStruct& UserHookData);
void EnableChunk(MIL_ID MilDigitizer, const MIL_STRING& Chunk, bool Enable);

/* User's processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId,
                                  void* HookDataPtr);

/* Main function. */
/* ---------------*/

int MosMain(void)
   {
   MIL_ID MilApplication;
   MIL_ID MilSystem;
   MIL_ID MilDigitizer;
   MIL_ID MilDisplay;
   MIL_ID MilImageDisp;
   MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX] = { 0 };
   MIL_INT MilGrabBufferListSize;
   MIL_INT ProcessFrameCount = 0;
   MIL_INT NbFrames = 0, n = 0;
   MIL_DOUBLE ProcessFrameRate = 0;
   MIL_STRING ChunkSelected;
   HookDataStruct UserHookData;

   /* Print a message. */
   PrintHeader();
   MosGetch();

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay,
                    &MilDigitizer, &MilImageDisp);
   MdigControl(MilDigitizer, M_GC_FEATURE_BROWSER, M_OPEN + M_ASYNCHRONOUS);
   if (!SystemSupportsGenICam(MilSystem))
      {
      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
      return 1;
      }

   /* Validate that Chunk Mode is supported by the GigE Vision device. */
   if (!IsChunkModeAvailable(MilDigitizer))
      {
      MosPrintf(MIL_TEXT("Your device does not support chunk mode.\n\n"));
      MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
      MosGetch();
      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
      return 1;
      }

   /* Allocate the grab buffers and clear them. */
   for (MilGrabBufferListSize = 0;
        MilGrabBufferListSize < BUFFERING_SIZE_MAX; MilGrabBufferListSize++)
      {
      MbufAlloc2d(MilSystem,
                  MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
                  MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
                  8 + M_UNSIGNED,
                  M_IMAGE + M_GRAB + M_PROC,
                  &MilGrabBufferList[MilGrabBufferListSize]);

      if (MilGrabBufferList[MilGrabBufferListSize])
         {
         MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);
         }
      else
         break;
      }

   /* Initialize the User's processing function data structure. */
   UserHookData.MilDigitizer = MilDigitizer;
   UserHookData.MilImageDisp = MilImageDisp;
   UserHookData.ProcessedImageCount = 0;

   ChunkSelected = ChooseAndEnableChunkMode(MilDigitizer, UserHookData);

   /* Start the processing. The processing function is called for every frame grabbed. */
   MdigProcess(MilDigitizer,
               MilGrabBufferList,
               MilGrabBufferListSize,
               M_START,
               M_DEFAULT,
               ProcessingFunction,
               &UserHookData);


   /* NOTE: Now the main() is free to perform other tasks
                                                    while the processing is executing. */
   /* -------------------------------------------------------------------------------- */


   /* Print a message and wait for a key press after a minimum number of frames. */
   MosPrintf(MIL_TEXT("Press <Enter> to stop.\n\n"));
   MosGetch();

   /* Stop the processing. */
   MdigProcess(MilDigitizer,
               MilGrabBufferList,
               MilGrabBufferListSize,
               M_STOP,
               M_DEFAULT,
               ProcessingFunction,
               &UserHookData);


   /* Print statistics. */
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT, &ProcessFrameCount);
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &ProcessFrameRate);
   MosPrintf(MIL_TEXT("\n\n%lld frames grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
             (long long)ProcessFrameCount,
             ProcessFrameRate,
             1000.0 / ProcessFrameRate);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   EnableChunk(MilDigitizer, ChunkSelected, false);

   /* Free the grab buffers. */
   while (MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);

   /* Release defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);

   return 0;
   }

/* Verifies if this example can run on the selected system. */
bool SystemSupportsGenICam(MIL_ID MilSystem)
   {
   MIL_INT GenICamSupport = M_FALSE;

   MsysInquire(MilSystem, M_GENICAM_AVAILABLE, &GenICamSupport);
   if (GenICamSupport == M_FALSE)
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

bool IsChunkModeAvailable(MIL_ID MilDigitizer)
   {
   MIL_BOOL ChunkModeActive = M_FALSE, ChunkSelector = M_FALSE;

   MdigInquireFeature(MilDigitizer,
                      M_FEATURE_PRESENT,
                      MIL_TEXT("ChunkModeActive"),
                      M_TYPE_BOOLEAN,
                      &ChunkModeActive);

   MdigInquireFeature(MilDigitizer,
                     M_FEATURE_PRESENT,
                     MIL_TEXT("ChunkSelector"),
                     M_TYPE_BOOLEAN,
                     &ChunkSelector);

   return ChunkModeActive && ChunkSelector;
   }

MIL_STRING ChooseAndEnableChunkMode(MIL_ID MilDigitizer, HookDataStruct& UserHookData)
   {
   MIL_INT ChunkCount = 0;

   /* Enable chunk mode. */
   MosPrintf(MIL_TEXT("\nEnabling chunk mode.\n"));
   MdigControlFeature(MilDigitizer,
                      M_FEATURE_VALUE,
                      MIL_TEXT("ChunkModeActive"),
                      M_TYPE_STRING,
                      MIL_TEXT("1"));

   /* Enumerate available chunks. */
   MosPrintf(MIL_TEXT("Please select a chunk data to enable.\n"));
   MdigInquireFeature(MilDigitizer,
                      M_FEATURE_ENUM_ENTRY_COUNT,
                      MIL_TEXT("ChunkSelector"),
                      M_TYPE_MIL_INT,
                      &ChunkCount);
   
   std::vector<MIL_STRING> Chunks;
   int Selection = 0;
   if (ChunkCount)
      {
      Chunks.reserve(ChunkCount);
      for (MIL_INT i = 0; i < ChunkCount; i++)
         {
         MIL_STRING Chunk;
         MdigInquireFeature(MilDigitizer,
                            M_FEATURE_ENUM_ENTRY_NAME + i,
                            MIL_TEXT("ChunkSelector"),
                            M_TYPE_STRING,
                            Chunk);

         MosPrintf(MIL_TEXT("%20s (%d) %s\n"), MIL_TEXT(""), (int)Chunks.size(), Chunk.c_str());
         Chunks.push_back(Chunk);
         }

      bool Done = false;
      do
         {
         char InputStream[64] = { '\0' };

         MosPrintf(MIL_TEXT("\nPlease select the event you wish to hook a function to: "));
         fgets(InputStream, sizeof(InputStream), stdin);
#if M_MIL_USE_WINDOWS
         int result = sscanf_s(InputStream, "%d", &Selection);
#else
         int result = sscanf(InputStream, "%d", &Selection);
#endif
         if((result == 1) && (Selection >= 0) && (((size_t)Selection) < Chunks.size()))
            Done = 1;
         else
            MosPrintf(MIL_TEXT("\nInvalid selection"));
         } while(!Done);

      EnableChunk(MilDigitizer, Chunks[Selection], true);

      MIL_BOOL IsData;
      MIL_STRING ChunkDataName = MIL_TEXT("Chunk") + Chunks[Selection];
      MdigInquireFeature(MilDigitizer, M_FEATURE_PRESENT, ChunkDataName, M_TYPE_BOOLEAN, &IsData);
      if(IsData)
         UserHookData.ChunkDataName = ChunkDataName;
      }

   return Chunks[Selection];
   }

void EnableChunk(MIL_ID MilDigitizer, const MIL_STRING& Chunk, bool Enable)
   {
   MIL_BOOL lEnable = Enable ? M_TRUE : M_FALSE;
   MIL_INT64 lAccessMode = 0;
   MdigControlFeature(MilDigitizer,
                      M_FEATURE_VALUE,
                      MIL_TEXT("ChunkSelector"),
                      M_TYPE_STRING,
                      Chunk);

   /* Some chunks are always enabled and cannot be written to. */
   /* Check chunk state and enable chunk only if required. */
   MdigInquireFeature(MilDigitizer,
                      M_FEATURE_ACCESS_MODE,
                      MIL_TEXT("ChunkEnable"),
                      M_TYPE_INT64,
                      &lAccessMode);
   MdigInquireFeature(MilDigitizer,
                      M_FEATURE_VALUE,
                      MIL_TEXT("ChunkEnable"),
                      M_TYPE_BOOLEAN,
                      &lEnable);

   if (M_FEATURE_IS_WRITABLE(lAccessMode))
      {
      if (Enable && !lEnable)
         {
         lEnable = M_TRUE;
         MdigControlFeature(MilDigitizer,
                            M_FEATURE_VALUE,
                            MIL_TEXT("ChunkEnable"),
                            M_TYPE_BOOLEAN,
                            &lEnable);
         }
      else if (!Enable && lEnable)
         {
         lEnable = M_FALSE;
         MdigControlFeature(MilDigitizer,
                            M_FEATURE_VALUE,
                            MIL_TEXT("ChunkEnable"),
                            M_TYPE_BOOLEAN,
                            &lEnable);
         }
      }

   if (!Enable)
      MdigControlFeature(MilDigitizer,
                         M_FEATURE_VALUE,
                         MIL_TEXT("ChunkModeActive"),
                         M_TYPE_STRING,
                         MIL_TEXT("0"));
   }


/* User's processing function called every time a grab buffer is modified. */
/* -----------------------------------------------------------------------*/

/* Local defines. */
#define STRING_LENGTH_MAX  20
#define STRING_POS_X       20
#define STRING_POS_Y       20

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId,
                                  void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX] = { MIL_TEXT('\0'), };

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);

   /* Read the Chunk Data */
   if(UserHookDataPtr->ChunkDataName.length())
      {
      MIL_STRING ChunkValueStr;
      MdigInquireFeature(UserHookDataPtr->MilDigitizer,
                           M_FEATURE_VALUE,
                           UserHookDataPtr->ChunkDataName,
                           M_TYPE_STRING,
                           ChunkValueStr);
      MosPrintf(MIL_TEXT("Received \"%s\": %s.\r"), UserHookDataPtr->ChunkDataName.c_str(), ChunkValueStr.c_str());
      }
   else
      {
      MosPrintf(MIL_TEXT("Unable to print chunk data, please refer to feature browser to view chunk data.\r"));
      }
   /* Print and draw the frame count. */
   UserHookDataPtr->ProcessedImageCount++;
   MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%lld"),
              (long long)UserHookDataPtr->ProcessedImageCount);
   MgraText(M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, Text);

   /* Perform the processing and update the display. */
   MbufCopy(ModifiedBufferId, UserHookDataPtr->MilImageDisp);

   return 0;
   }
