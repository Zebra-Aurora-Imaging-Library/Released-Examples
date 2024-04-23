/***************************************************************************************/
/*
* File name: MappTrace.cpp
*
* Synopsis:  This example shows how to explicitly control and generate a trace for 
*            MIL functions and how to visualize it using the Matrox Profiler utility. 
*            To generate a trace, you must open Matrox Profiler (accessible from the 
*            MIL Control Center) and select 'Generate New Trace' from the 'File' menu 
*            before to run your MIL application.
* 
* Note:      By default, all MIL applications are traceable without code modifications.
*            You can try this using Matrox Profiler with any MIL example (Ex: MappStart).
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>

/* Trace related defines. */
#define TRACE_TAG_HOOK_START                       1
#define TRACE_TAG_PROCESSING                       2
#define TRACE_TAG_PREPROCESSING                    3

/* General defines. */
#define COLOR_BROWN                                M_RGB888(100,65,50)
#define BUFFERING_SIZE_MAX                         3
#define NUMBER_OF_FRAMES_TO_PROCESS                10

/* Function prototype. */
MIL_INT MFTYPE HookFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);
typedef struct
{
   MIL_ID  MilImageDisp;
   MIL_ID  MilImageTemp1;
   MIL_ID  MilImageTemp2;
   MIL_INT ProcessedImageCount;
   MIL_ID  DoneEvent;
} HookDataStruct;

/* Main function. */
int MosMain(void)
{
   MIL_ID   MilApplication = M_NULL;
   MIL_ID   MilSystem = M_NULL;
   MIL_ID   MilDisplay = M_NULL;
   MIL_ID   MilDigitizer = M_NULL;
   MIL_ID   MilGrabBuf[BUFFERING_SIZE_MAX] = { 0 };
   MIL_ID   MilDummyBuffer = M_NULL;

   MIL_INT TracesActivated = M_NO;
   MIL_INT NbGrabBuf = 0;
   MIL_INT SizeX = 0, SizeY = 0;

   HookDataStruct UserHookData;

   MosPrintf(MIL_TEXT("\nMIL PROGRAM TRACING AND PROFILING:\n"));
   MosPrintf(MIL_TEXT(  "----------------------------------\n\n"));

   MosPrintf(MIL_TEXT("This example shows how to generate a trace for the execution\n"));
   MosPrintf(MIL_TEXT("of the MIL functions, and to visualize it using\n"));
   MosPrintf(MIL_TEXT("the Matrox Profiler utility.\n\n"));
   MosPrintf(MIL_TEXT("ACTION REQUIRED:\n\n"));
#if M_MIL_USE_WINDOWS
   MosPrintf(MIL_TEXT("Open 'Matrox Profiler' from the 'MIL Control Center' and\n"));
   MosPrintf(MIL_TEXT("select 'Generate New Trace' from the 'File' menu.\n\n"));
#else
   MosPrintf(MIL_TEXT("Open 'MilConfig' from the 'MIL Control Center' and select the\n"));
   MosPrintf(MIL_TEXT("'MIL Profiler trace' page in 'Benchmarks and Utilities'.\n"));
#endif
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /*************** Untraceable code section ***************/

   /* The following code will not be visible in the trace. */

   /* MIL application allocation. 
      At MIL Application allocation time, M_TRACE_LOG_DISABLE can be used to ensures that 
      an application will not be traceable regardless of Matrox Profiler or MilConfig requests
      unless traces are explicitly enabled in the program using an MappControl command.
      */
   MappAlloc(MIL_TEXT("M_DEFAULT"), M_TRACE_LOG_DISABLE, &MilApplication);

   /* Dummy MIL calls that will be invisible in the trace. */
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MbufAllocColor(MilSystem, 3, 128, 128, 8 + M_UNSIGNED, M_IMAGE, &MilDummyBuffer);
   MbufClear(MilDummyBuffer, 0L);
   MbufFree(MilDummyBuffer);
   MsysFree(MilSystem);

   /********************************************************/

   /* Explicitly allow trace logging after a certain point if Matrox Profiler has
      requested a trace. Note that M_TRACE = M_ENABLE can be used to force the log 
      of a trace even if Profiler is not opened; M_TRACE = M_DISABLE can prevent 
      logging of code section.
      */
   MappControl(M_DEFAULT, M_TRACE, M_DEFAULT);

   /* Inquire if the traces are active (i.e. Profiler is open and waiting for a trace). */
   MappInquire(M_DEFAULT, M_TRACE_ACTIVE, &TracesActivated);

   if (TracesActivated == M_YES)
   {
      /* Create custom trace markers: setting custom names and colors. */

      /* Initialize a custom Tag for the grab callback function with a unique color (blue). */
      MappTrace(M_DEFAULT,
         M_TRACE_SET_TAG_INFORMATION,
         TRACE_TAG_HOOK_START,
         M_COLOR_BLUE, MIL_TEXT("Grab Callback Marker"));

      /* Initialize the custom Tag for the processing section. */
      MappTrace(M_DEFAULT,
         M_TRACE_SET_TAG_INFORMATION,
         TRACE_TAG_PROCESSING,
         M_DEFAULT, MIL_TEXT("Processing Section"));

      /* Initialize the custom Tag for the preprocessing with a unique color (brown). */
      MappTrace(M_DEFAULT,
         M_TRACE_SET_TAG_INFORMATION,
         TRACE_TAG_PREPROCESSING,
         COLOR_BROWN, MIL_TEXT("Preprocessing Marker"));
   }

   /* Allocate MIL objects. */
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdigAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDigitizer);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);

   SizeX = MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
   SizeY = MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);

   MbufAllocColor(MilSystem, 3, SizeX, SizeY, 8+M_UNSIGNED,
      M_IMAGE + M_GRAB+M_PROC+M_DISP, &UserHookData.MilImageDisp);
   MdispSelect(MilDisplay, UserHookData.MilImageDisp);

   /* Allocate the processing temporary buffers. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8+M_UNSIGNED, M_PROC+M_IMAGE, 
      &UserHookData.MilImageTemp1);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8+M_UNSIGNED, M_PROC+M_IMAGE,
      &UserHookData.MilImageTemp2);

   /* Allocate the grab buffers. */
   for (NbGrabBuf = 0; NbGrabBuf < BUFFERING_SIZE_MAX; NbGrabBuf++)
   {
      MbufAllocColor(MilSystem, 3, SizeX, SizeY, 8 + M_UNSIGNED,
         M_IMAGE + M_GRAB + M_PROC, &MilGrabBuf[NbGrabBuf]);
   }

   /* Initialize the user's processing function data structure. */
   UserHookData.ProcessedImageCount = 0;
   MthrAlloc(MilSystem, M_EVENT, M_NOT_SIGNALED + M_AUTO_RESET, M_NULL,
      M_NULL, &UserHookData.DoneEvent);

   /* Start the processing. The processing function is called with every frame grabbed.*/
   MdigProcess(MilDigitizer, MilGrabBuf, BUFFERING_SIZE_MAX, M_START,
      M_DEFAULT, HookFunction, &UserHookData);

   /* Stop the processing when the event is triggered. */
   MthrWait(UserHookData.DoneEvent, M_EVENT_WAIT + M_EVENT_TIMEOUT(2000), M_NULL);

   /* Stop the processing. */
   MdigProcess(MilDigitizer, MilGrabBuf, BUFFERING_SIZE_MAX, M_STOP, M_DEFAULT,
      HookFunction, &UserHookData);

   /* Free the grab and temporary buffers. */
   for (NbGrabBuf = 0; NbGrabBuf < BUFFERING_SIZE_MAX; NbGrabBuf++)
      MbufFree(MilGrabBuf[NbGrabBuf]);
   MbufFree(UserHookData.MilImageTemp1);
   MbufFree(UserHookData.MilImageTemp2);

   /* Free defaults. */
   MthrFree(UserHookData.DoneEvent);
   MappFreeDefault(MilApplication,
      MilSystem,
      MilDisplay,
      MilDigitizer,
      UserHookData.MilImageDisp);

   /* If Matrox Profiler activated the traces, the trace file is now ready. */
   if (TracesActivated == M_YES)
   {
      MosPrintf(MIL_TEXT("A PROCESSING SEQUENCE WAS EXECUTED AND LOGGED A NEW TRACE:\n\n"));
#if M_MIL_USE_WINDOWS      
      MosPrintf(MIL_TEXT("The trace can now be loaded in Matrox Profiler by selecting the\n"));
      MosPrintf(MIL_TEXT("corresponding file listed in the 'Trace Generation' dialog.\n\n"));

      MosPrintf(MIL_TEXT("Once loaded, Matrox Profiler's main window displays the 'Main'\n"));
      MosPrintf(MIL_TEXT("and the 'MdigProcess' threads of the application.\n\n"));
      
      MosPrintf(MIL_TEXT("- This main window can now be used to select a section\n"));
      MosPrintf(MIL_TEXT("  of a thread and to zoom or pan in it.\n\n"));

      MosPrintf(MIL_TEXT("- The right pane shows detailed statistics as well as a\n"));
      MosPrintf(MIL_TEXT("  'Quick Access' list displaying all MIL function calls.\n\n"));

      MosPrintf(MIL_TEXT("- The 'User Markers' tab lists the markers and sections logged\n"));
      MosPrintf(MIL_TEXT("  during the execution. For example, selecting 'Tag:Processing'\n"));
      MosPrintf(MIL_TEXT("  allows double-clicking to refocus the display on the related\n"));
      MosPrintf(MIL_TEXT("  calls.\n\n"));

      MosPrintf(MIL_TEXT("- By clicking a particular MIL function call, either in the\n"));
      MosPrintf(MIL_TEXT("  'main view' or in the 'Quick Access', additional details\n"));
      MosPrintf(MIL_TEXT("  are displayed, such as its parameters and execution time.\n\n"));
#else
      MosPrintf(MIL_TEXT("The trace is now available in 'MIL Profiler trace' page of MILConfig\n"));
      MosPrintf(MIL_TEXT("Copy the trace file to a Windows machine and open it with the MIL Profiler utility.\n\n"));
#endif
   }
   else
   {
#if M_MIL_USE_WINDOWS
      MosPrintf(MIL_TEXT("ERROR: No active tracing detected in MIL Profiler!\n\n"));
#else
      MosPrintf(MIL_TEXT("ERROR: No active tracing detected in 'MIL Profiler trace' page of MILConfig!\n\n"));
#endif
   }
   MosPrintf(MIL_TEXT("Press <Enter> to end."));
   MosGetch();

   return 0;
}

MIL_INT MFTYPE HookFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
{
   MIL_INT64 PreprocReturnValue = -1;
   MIL_ID CurrentImage = M_NULL;

   HookDataStruct *UserDataPtr = (HookDataStruct *)HookDataPtr;

   /* Add a marker to indicate the reception of a new grabbed image. */
   MappTrace(M_DEFAULT,
      M_TRACE_MARKER,
      TRACE_TAG_HOOK_START,
      M_NULL,
      MIL_TEXT("New Image grabbed"));

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &CurrentImage);

   /* Start a Section to highlight the processing calls on the image. */
   MappTrace(M_DEFAULT,
      M_TRACE_SECTION_START,
      TRACE_TAG_PROCESSING,
      UserDataPtr->ProcessedImageCount,
      MIL_TEXT("Processing Image"));

   /* Add a Marker to indicate the start of the preprocessing section. */
   MappTrace(M_DEFAULT,
      M_TRACE_MARKER,
      TRACE_TAG_PREPROCESSING,
      UserDataPtr->ProcessedImageCount,
      MIL_TEXT("Start Preprocessing"));

   /* Do the preprocessing. */
   MimConvert(CurrentImage, UserDataPtr->MilImageTemp1, M_RGB_TO_L);
   MimHistogramEqualize(UserDataPtr->MilImageTemp1, UserDataPtr->MilImageTemp1, 
      M_UNIFORM, M_NULL, 55, 200);

   /* Add a Marker to indicate the end of the preprocessing section. */
   MappTrace(M_DEFAULT,
      M_TRACE_MARKER,
      TRACE_TAG_PREPROCESSING,
      UserDataPtr->ProcessedImageCount,
      MIL_TEXT("End Preprocessing"));

   /* Do the main processing. */
   MimBinarize(UserDataPtr->MilImageTemp1, UserDataPtr->MilImageTemp2,
      M_IN_RANGE, 120, 140);
   MimBinarize(UserDataPtr->MilImageTemp1, UserDataPtr->MilImageTemp1,
      M_IN_RANGE, 220, 255);
   MimArith(UserDataPtr->MilImageTemp1, UserDataPtr->MilImageTemp2, 
      UserDataPtr->MilImageDisp, M_OR);

   /* End the Section that highlights the processing. */
   MappTrace(M_DEFAULT,
      M_TRACE_SECTION_END,
      TRACE_TAG_PROCESSING,
      UserDataPtr->ProcessedImageCount,
      MIL_TEXT("Processing Image End"));

   /* Signal that processing has been completed. */
   if (++(UserDataPtr->ProcessedImageCount) >= NUMBER_OF_FRAMES_TO_PROCESS)
      MthrControl(UserDataPtr->DoneEvent, M_EVENT_SET, M_SIGNALED);
   return 0;
}
