/***************************************************************************************/
/*
 * File name: MdigProcess3D.cpp
 *
 * Synopsis:  This program shows the use of the MdigProcess() function and its multiple
 *            buffering acquisition to do robust real-time 3D acquisition, processing
 *            and display.
 *
 *            The user's processing code to execute is located in a callback function
 *            that will be called for each frame acquired (see ProcessingFunction()).
 *
 *      Note: The average processing time must be shorter than the grab time or some
 *            frames will be missed. Also, if the processing results are not displayed
 *            the CPU usage is reduced significantly.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#include <vector>

 /* Number of images in the buffering grab queue.
    Generally, increasing this number gives a better real-time grab.
 */
#define BUFFERING_SIZE_MAX 5

 /* User's processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

/* User's processing function hook data structure. */
typedef struct
   {
   MIL_ID  MilDigitizer;
   MIL_ID  MilContainerDisp;
   MIL_INT ProcessedImageCount;
   } HookDataStruct;

/* Utility function to print the MIL Container detailed informations. */
void PrintContainerInfo(MIL_ID MilContainer);
bool Alloc3dDisplayAndContainer(MIL_ID MilSystem, MIL_ID& MilDisplay, MIL_ID& MilContainerDisp);

/* Main function. */
/* ---------------*/

int MosMain(void)
   {
   MIL_ID MilApplication;
   MIL_ID MilSystem;
   MIL_ID MilDigitizer;
   MIL_ID MilDisplay;
   MIL_ID MilContainerDisp;
   MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX] = {0};
   MIL_INT MilGrabBufferListSize;
   MIL_INT ProcessFrameCount = 0;
   MIL_DOUBLE ProcessFrameRate = 0;
   MIL_INT NbFrames = 0, n = 0;
   HookDataStruct UserHookData;

   /* Allocate defaults. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_DEFAULT, M_DEFAULT, M_DEFAULT, &MilSystem);

   if(Alloc3dDisplayAndContainer(MilSystem, MilDisplay, MilContainerDisp) == false)
      {
      MsysFree(MilSystem);
      MappFree(MilApplication);
      MosGetch();
      return -1;
      }
   MdigAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDigitizer);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nMULTIPLE 3D CONTAINERS PROCESSING.\n"));
   MosPrintf(MIL_TEXT("----------------------------------\n\n"));

   /* Open the feature browser to setup the camera before acquisition (if not using the System Host simulator). */
   bool SkipFeatureBrowser = false;
#if M_MIL_USE_LINUX
   MIL_INT BoardType = 0;
   MsysInquire(MilSystem, M_BOARD_TYPE, &BoardType);
   SkipFeatureBrowser = BoardType & M_CL;
#endif
   if(MsysInquire(MilSystem, M_GENICAM_AVAILABLE, M_NULL) && !SkipFeatureBrowser)
      {
      MdigControl(MilDigitizer, M_GC_FEATURE_BROWSER, M_OPEN + M_ASYNCHRONOUS);
      MosPrintf(MIL_TEXT("Please setup your 3D camera using the feature browser.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to start the acquisition.\n\n"));
      MosGetch();
      }

   /* Do a first acquisition to determine what is included in the type camera output. */
   MdigGrab(MilDigitizer, MilContainerDisp);

   /* Print the acquired MIL Container detailed informations. */
   PrintContainerInfo(MilContainerDisp);

   /* If the grabbed Container has 3D data and is Displayable and Processable. */
   if((MbufInquireContainer(MilContainerDisp, M_CONTAINER, M_3D_DISPLAYABLE, M_NULL) != M_NOT_DISPLAYABLE) &&
      (MbufInquireContainer(MilContainerDisp, M_CONTAINER, M_3D_CONVERTIBLE, M_NULL) != M_NOT_CONVERTIBLE))
      {
      /* Display the Container on the 3D display. */
      M3ddispSelect(MilDisplay, MilContainerDisp, M_DEFAULT, M_DEFAULT);

      /* Grab continuously on the 3D display and wait for a key press. */
      MdigGrabContinuous(MilDigitizer, MilContainerDisp);

      MosPrintf(MIL_TEXT("Live 3D acquisition in progress...\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to start the processing.\n"));
      MosGetch();

      /* Halt continuous grab. */
      MdigHalt(MilDigitizer);

      /* Allocate the grab Containers for processing. */
      for(MilGrabBufferListSize = 0; MilGrabBufferListSize < BUFFERING_SIZE_MAX; MilGrabBufferListSize++)
         {
         MbufAllocContainer(MilSystem, M_PROC | M_GRAB, M_DEFAULT, &MilGrabBufferList[MilGrabBufferListSize]);
         }

      /* Initialize the user's processing function data structure. */
      UserHookData.MilDigitizer = MilDigitizer;
      UserHookData.MilContainerDisp = MilContainerDisp;
      UserHookData.ProcessedImageCount = 0;

      /* Start the processing. The processing function is called with every frame grabbed. */
      MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_START, M_DEFAULT, ProcessingFunction, &UserHookData);


      /* Here the main() is free to perform other tasks while the processing is executing. */
      /* --------------------------------------------------------------------------------- */


      /* Print a message and wait for a key press after a minimum number of frames. */
      MosPrintf(MIL_TEXT("\nProcessing in progress...\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to stop.                    \n\n"));
      MosGetch();

      /* Stop the processing. */
      MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_STOP, M_DEFAULT, ProcessingFunction, &UserHookData);

      /* Print statistics. */
      MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT, &ProcessFrameCount);
      MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &ProcessFrameRate);
      MosPrintf(MIL_TEXT("\n\n%d 3D containers grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
         (int)ProcessFrameCount, ProcessFrameRate, 1000.0 / ProcessFrameRate);
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();

      /* Free the grab buffers. */
      while(MilGrabBufferListSize > 0)
         MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);
      }
   else
      {
      MosPrintf(MIL_TEXT("ERROR: The camera provides no (or more than one) 3D Component(s) of type Range or Disparity.\nPress <Enter> to end.\n\n"));
      MosGetch();
      }

   /* Release. */
   MbufFree(MilContainerDisp);
   M3ddispFree(MilDisplay);
   MdigFree(MilDigitizer);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

/* User's processing function called every time a grab container is ready. */
/* ----------------------------------------------------------------------- */

/* Local defines. */
#define STRING_LENGTH_MAX  20

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX] = {MIL_TEXT('\0'),};

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);

   /* Increment the frame counter. */
   UserHookDataPtr->ProcessedImageCount++;

   /* Print and draw the frame count (remove to reduce CPU usage). */
   MosPrintf(MIL_TEXT("Processing frame #%d.\r"), (int)UserHookDataPtr->ProcessedImageCount);
   MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%d"), (int)UserHookDataPtr->ProcessedImageCount);

   /* Execute the processing and update the display. */
   MbufConvert3d(ModifiedBufferId, UserHookDataPtr->MilContainerDisp, M_NULL, M_DEFAULT, M_COMPENSATE);

   return 0;
   }

/* Utility function to print the MIL Container detailed informations. */
/* ------------------------------------------------------------------ */
void PrintContainerInfo(MIL_ID MilContainer)
   {
   std::vector<MIL_ID> ComponentList;
   MbufInquireContainer(MilContainer, M_CONTAINER, M_COMPONENT_LIST, ComponentList);
   MosPrintf(MIL_TEXT("Container Information:\n"));
   MosPrintf(MIL_TEXT("----------------------\n"));
   MosPrintf(MIL_TEXT("Container:    Component Count: %d\n"), (int)ComponentList.size());
   for(std::size_t i = 0; i < ComponentList.size(); i++)
      {
      MIL_STRING ComponentName;
      MbufInquire(ComponentList[i], M_COMPONENT_TYPE_NAME, ComponentName);
      MIL_INT DataType = MbufInquire(ComponentList[i], M_DATA_TYPE, M_NULL);
      MIL_INT DataFormat = MbufInquire(ComponentList[i], M_DATA_FORMAT, M_NULL) & (M_PACKED | M_PLANAR);
      MIL_INT64 GroupId, SourceId, RegionId;
      MbufInquire(ComponentList[i], M_COMPONENT_GROUP_ID, &GroupId);
      MbufInquire(ComponentList[i], M_COMPONENT_SOURCE_ID, &SourceId);
      MbufInquire(ComponentList[i], M_COMPONENT_REGION_ID, &RegionId);
      MosPrintf(MIL_TEXT("Component[%ld]: %-11s[%ld:%ld:%ld] Band: %1ld, Size X: %4ld, Size Y: %4ld, Type: %2ld%s (%6s)\n"),
                i,
                ComponentName.c_str(),
                (long)GroupId,
                (long)SourceId,
                (long)RegionId,
                MbufInquire(ComponentList[i], M_SIZE_BAND, M_NULL),
                MbufInquire(ComponentList[i], M_SIZE_X, M_NULL),
                MbufInquire(ComponentList[i], M_SIZE_Y, M_NULL),
                MbufInquire(ComponentList[i], M_SIZE_BIT, M_NULL),
                (DataType == M_UNSIGNED) ? MIL_TEXT("u") : (DataType == M_SIGNED) ? MIL_TEXT("s") : (DataType == M_FLOAT) ? MIL_TEXT("f") : MIL_TEXT(""),
                (MbufInquire(ComponentList[i], M_SIZE_BAND, M_NULL) == 1) ? MIL_TEXT("Mono") : (DataFormat == M_PLANAR) ? MIL_TEXT("Planar") : MIL_TEXT("Packed"));
      }
   MosPrintf(MIL_TEXT("\n"));
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.  
//*****************************************************************************
bool Alloc3dDisplayAndContainer(MIL_ID MilSystem, MIL_ID& MilDisplay, MIL_ID& MilContainerDisp)
   {
   // First we check if the system is local
   if(MsysInquire(MilSystem, M_LOCATION, M_NULL) != M_LOCAL)
      {
      MosPrintf(MIL_TEXT("This example requires a 3D display which is not supported on a remote system.\n"));
      MosPrintf(MIL_TEXT("Please select a local system as the default.\n"));
      return false;
      }

   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MilDisplay = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MilContainerDisp = MbufAllocContainer(MilSystem, M_PROC | M_GRAB | M_DISP, M_DEFAULT, M_NULL);
   if(MilContainerDisp == M_NULL || MilDisplay == M_NULL)
      {
      MIL_STRING ErrorMessage;
      MIL_STRING ErrorMessageSub1;
      MappGetError(M_DEFAULT, M_GLOBAL + M_MESSAGE, ErrorMessage);
      MappGetError(M_DEFAULT, M_GLOBAL_SUB_1 + M_MESSAGE, ErrorMessageSub1);
      MosPrintf(MIL_TEXT("\nThe current system does not support the 3D display:\n"));
      MosPrintf(MIL_TEXT("   %s\n"), ErrorMessage.c_str());
      MosPrintf(MIL_TEXT("   %s\n"), ErrorMessageSub1.c_str());
      if(MilDisplay != M_NULL)
         {
         M3ddispFree(MilDisplay);
         }
      if(MilContainerDisp != M_NULL)
         {
         MbufFree(MilContainerDisp);
         }
      return false;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   return true;
   }
