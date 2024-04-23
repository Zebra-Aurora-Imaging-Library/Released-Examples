//***************************************************************************************
//
// File name: MappTrace.cs
//
// Synopsis:  This example shows how to explicitly control and generate a trace for 
//            MIL functions and how to visualize it using the Matrox Profiler utility. 
//            To generate a trace, you must open Matrox Profiler (accessible from the 
//            MIL Control Center) and select 'Generate New Trace' from the 'File' menu 
//            before to run your MIL application.
// 
// Note:      By default, all MIL applications are traceable without code modifications.
//            You can try this using Matrox Profiler with any MIL example (Ex: MappStart).
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
using System;
using System.Runtime.InteropServices;
using Matrox.MatroxImagingLibrary;

namespace MappTrace
{
    class HookDataStruct
    {
        public MIL_ID MilImageDisp;
        public MIL_ID MilImageTemp1;
        public MIL_ID MilImageTemp2;
        public MIL_INT ProcessedImageCount;
        public MIL_ID DoneEvent;
    }

    class MappTrace
    {
        // Trace related constants
        private const int TRACE_TAG_HOOK_START = 1;
        private const int TRACE_TAG_PROCESSING = 2;
        private const int TRACE_TAG_PREPROCESSING = 3;

        // General constants.
        private static readonly int COLOR_BROWN = MIL.M_RGB888(100, 65, 50);

        private const int BUFFERING_SIZE_MAX = 3;
        private const int NUMBER_OF_FRAMES_TO_PROCESS = 10;

        static void Main(string[] args)
        {
            MIL_ID MilApplication = MIL.M_NULL;
            MIL_ID MilSystem = MIL.M_NULL;
            MIL_ID MilDisplay = MIL.M_NULL;
            MIL_ID MilDigitizer = MIL.M_NULL;
            MIL_ID[] MilGrabBuf = new MIL_ID[BUFFERING_SIZE_MAX];
            MIL_ID MilDummyBuffer = MIL.M_NULL;

            MIL_INT TracesActivated = MIL.M_NO;
            MIL_INT NbGrabBuf = 0;
            MIL_INT SizeX = 0, SizeY = 0;

            HookDataStruct UserHookData = new HookDataStruct();

            Console.WriteLine();
            Console.WriteLine("MIL PROGRAM TRACING AND PROFILING:");
            Console.WriteLine("----------------------------------");
            Console.WriteLine();
            Console.WriteLine("This example shows how to generate a trace for the execution");
            Console.WriteLine("of the MIL functions, and to visualize it using");
            Console.WriteLine("the Matrox Profiler utility.");
            Console.WriteLine();
            Console.WriteLine("ACTION REQUIRED:");
            Console.WriteLine();
            Console.WriteLine("Open 'Matrox Profiler' from the 'MIL Control Center' and");
            Console.WriteLine("select 'Generate New Trace' from the 'File' menu.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            //************** Untraceable code section ***************

            // The following code will not be visible in the trace.

            // MIL application allocation. 
            // At MIL Application allocation time, M_TRACE_LOG_DISABLE can be used to ensures that 
            // an application will not be traceable regardless of Matrox Profiler or MilConfig requests
            // unless traces are explicitly enabled in the program using an MappControl command.
            //

            MIL.MappAlloc("M_DEFAULT", MIL.M_TRACE_LOG_DISABLE, ref MilApplication);

            // Dummy MIL calls that will be invisible in the trace.


            MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_HOST, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilSystem);
            MIL.MbufAllocColor(MilSystem, 3, 128, 128, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE, ref MilDummyBuffer);
            MIL.MbufClear(MilDummyBuffer, 0L);
            MIL.MbufFree(MilDummyBuffer);
            MIL.MsysFree(MilSystem);

            //*******************************************************

            // Explicitly allow trace logging after a certain point if Matrox Profiler has
            // requested a trace. Note that M_TRACE = M_ENABLE can be used to force the log 
            // of a trace even if Profiler is not opened; M_TRACE = M_DISABLE can prevent 
            // logging of code section.
            //

            MIL.MappControl(MIL.M_DEFAULT, MIL.M_TRACE, MIL.M_DEFAULT);

            // Inquire if the traces are active (i.e. Profiler is open and waiting for a trace).

            MIL.MappInquire(MIL.M_DEFAULT, MIL.M_TRACE_ACTIVE, ref TracesActivated);

            if (TracesActivated == MIL.M_YES)
            {
                // Create custom trace markers: setting custom names and colors.

                // Initialize a custom Tag for the grab callback function with a unique color (blue).
                MIL.MappTrace(MIL.M_DEFAULT,
                          MIL.M_TRACE_SET_TAG_INFORMATION,
                          TRACE_TAG_HOOK_START,
                          MIL.M_COLOR_BLUE, "Grab Callback Marker");

                // Initialize the custom Tag for the processing section.
                MIL.MappTrace(MIL.M_DEFAULT,
                          MIL.M_TRACE_SET_TAG_INFORMATION,
                          TRACE_TAG_PROCESSING,
                          MIL.M_DEFAULT, "Processing Section");

                // Initialize the custom Tag for the preprocessing with a unique color (brown).
                MIL.MappTrace(MIL.M_DEFAULT,
                          MIL.M_TRACE_SET_TAG_INFORMATION,
                          TRACE_TAG_PREPROCESSING,
                          COLOR_BROWN, "Preprocessing Marker");
            }


            // Allocate MIL objects.
            MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_HOST, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilSystem);
            MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, ref MilDigitizer);
            MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, ref MilDisplay);

            SizeX = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X, MIL.M_NULL);
            SizeY = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y, MIL.M_NULL);

            MIL.MbufAllocColor(MilSystem, 3, SizeX, SizeY, 8 + MIL.M_UNSIGNED,
                           MIL.M_IMAGE + MIL.M_GRAB + MIL.M_PROC + MIL.M_DISP, ref UserHookData.MilImageDisp);

            MIL.MdispSelect(MilDisplay, UserHookData.MilImageDisp);

            // Allocate the processing temporary buffers.
            MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + MIL.M_UNSIGNED, MIL.M_PROC + MIL.M_IMAGE, ref UserHookData.MilImageTemp1);
            MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + MIL.M_UNSIGNED, MIL.M_PROC + MIL.M_IMAGE, ref UserHookData.MilImageTemp2);

            // Allocate the grab buffers.
            for (NbGrabBuf = 0; NbGrabBuf < BUFFERING_SIZE_MAX; NbGrabBuf++)
            {
                MIL.MbufAllocColor(MilSystem, 3, SizeX, SizeY, 8 + MIL.M_UNSIGNED,
                               MIL.M_IMAGE + MIL.M_GRAB + MIL.M_PROC, ref MilGrabBuf[NbGrabBuf]);
            }

            // Initialize the user's processing function data structure.
            UserHookData.ProcessedImageCount = 0;
            MIL.MthrAlloc(MilSystem, MIL.M_EVENT, MIL.M_NOT_SIGNALED + MIL.M_AUTO_RESET, MIL.M_NULL,
                      MIL.M_NULL, ref UserHookData.DoneEvent);

            GCHandle UserHookDataPtr = GCHandle.Alloc(UserHookData);
            MIL_DIG_HOOK_FUNCTION_PTR HookFunctionPtr = new MIL_DIG_HOOK_FUNCTION_PTR(HookFunction);

            // Start the processing. The processing function is called with every frame grabbed.
            MIL.MdigProcess(MilDigitizer, MilGrabBuf, BUFFERING_SIZE_MAX, MIL.M_START,
                        MIL.M_DEFAULT, HookFunctionPtr, GCHandle.ToIntPtr(UserHookDataPtr));

            // Stop the processing when the event is triggered.
            MIL.MthrWait(UserHookData.DoneEvent, MIL.M_EVENT_WAIT + MIL.M_EVENT_TIMEOUT(2000), MIL.M_NULL);

            // Stop the processing.
            MIL.MdigProcess(MilDigitizer, MilGrabBuf, BUFFERING_SIZE_MAX, MIL.M_STOP, MIL.M_DEFAULT,
                        HookFunctionPtr, GCHandle.ToIntPtr(UserHookDataPtr));
            UserHookDataPtr.Free();

            // Free the grab and temporary buffers.
            for (NbGrabBuf = 0; NbGrabBuf < BUFFERING_SIZE_MAX; NbGrabBuf++)
                MIL.MbufFree(MilGrabBuf[NbGrabBuf]);
            MIL.MbufFree(UserHookData.MilImageTemp1);
            MIL.MbufFree(UserHookData.MilImageTemp2);


            // Free defaults.
            MIL.MthrFree(UserHookData.DoneEvent);
            MIL.MappFreeDefault(MilApplication,
                            MilSystem,
                            MilDisplay,
                            MilDigitizer,
                            UserHookData.MilImageDisp);

            // If Matrox Profiler activated the traces, the trace file is now ready.
            if (TracesActivated == MIL.M_YES)
            {
                Console.WriteLine("A PROCESSING SEQUENCE WAS EXECUTED AND LOGGED A NEW TRACE:");
                Console.WriteLine();
                Console.WriteLine("The trace can now be loaded in Matrox Profiler by selecting the");
                Console.WriteLine("corresponding file listed in the 'Trace Generation' dialog.");
                Console.WriteLine();
                Console.WriteLine("Once loaded, Matrox Profiler's main window displays the 'Main'");
                Console.WriteLine("and the 'MdigProcess' threads of the application.");
                Console.WriteLine();
                Console.WriteLine("- This main window can now be used to select a section");
                Console.WriteLine("  of a thread and to zoom or pan in it.");
                Console.WriteLine();
                Console.WriteLine("- The right pane shows detailed statistics as well as a");
                Console.WriteLine("  'Quick Access' list displaying all MIL function calls.");
                Console.WriteLine();
                Console.WriteLine("- The 'User Markers' tab lists the markers and sections logged");
                Console.WriteLine("  during the execution. For example, selecting 'Tag:Processing'");
                Console.WriteLine("  allows double-clicking to refocus the display on the related");
                Console.WriteLine("  calls.");
                Console.WriteLine();
                Console.WriteLine("- By clicking a particular MIL function call, either in the");
                Console.WriteLine("  'main view' or in the 'Quick Access', additional details");
                Console.WriteLine("  are displayed, such as its parameters and execution time.");
                Console.WriteLine();
            }
            else
            {
                Console.WriteLine("ERROR: No active tracing detected in MIL Profiler!");
                Console.WriteLine();
            }
            Console.WriteLine("Press <Enter> to end.");
            Console.ReadKey();
        }

        static MIL_INT HookFunction(MIL_INT HookType, MIL_ID HookId, IntPtr HookDataPtr)
        {
            MIL_ID CurrentImage = MIL.M_NULL;

            if (HookDataPtr != IntPtr.Zero)
            {
                HookDataStruct UserDataPtr = GCHandle.FromIntPtr(HookDataPtr).Target as HookDataStruct;

                // Add a marker to indicate the reception of a new grabbed image.
                MIL.MappTrace(MIL.M_DEFAULT,
                          MIL.M_TRACE_MARKER,
                          TRACE_TAG_HOOK_START,
                          MIL.M_NULL,
                          "New Image grabbed");

                // Retrieve the MIL_ID of the grabbed buffer.
                MIL.MdigGetHookInfo(HookId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID, ref CurrentImage);

                // Start a Section to highlight the processing calls on the image.

                MIL.MappTrace(MIL.M_DEFAULT,
                          MIL.M_TRACE_SECTION_START,
                          TRACE_TAG_PROCESSING,
                          UserDataPtr.ProcessedImageCount,
                          "Processing Image");

                // Add a Marker to indicate the start of the preprocessing section.
                MIL.MappTrace(MIL.M_DEFAULT,
                          MIL.M_TRACE_MARKER,
                          TRACE_TAG_PREPROCESSING,
                          UserDataPtr.ProcessedImageCount,
                          "Start Preprocessing");

                // Do the preprocessing.
                MIL.MimConvert(CurrentImage, UserDataPtr.MilImageTemp1, MIL.M_RGB_TO_L);
                MIL.MimHistogramEqualize(UserDataPtr.MilImageTemp1, UserDataPtr.MilImageTemp1, MIL.M_UNIFORM, MIL.M_NULL, 55, 200);

                // Add a Marker to indicate the end of the preprocessing section.

                MIL.MappTrace(MIL.M_DEFAULT,
                          MIL.M_TRACE_MARKER,
                          TRACE_TAG_PREPROCESSING,
                          UserDataPtr.ProcessedImageCount,
                          "End Preprocessing");

                // Do the main processing.
                MIL.MimBinarize(UserDataPtr.MilImageTemp1, UserDataPtr.MilImageTemp2, MIL.M_IN_RANGE, 120, 140);
                MIL.MimBinarize(UserDataPtr.MilImageTemp1, UserDataPtr.MilImageTemp1, MIL.M_IN_RANGE, 220, 255);
                MIL.MimArith(UserDataPtr.MilImageTemp1, UserDataPtr.MilImageTemp2, UserDataPtr.MilImageDisp, MIL.M_OR);

                // End the Section that highlights the processing.
                MIL.MappTrace(MIL.M_DEFAULT,
                          MIL.M_TRACE_SECTION_END,
                          TRACE_TAG_PROCESSING,
                          UserDataPtr.ProcessedImageCount,
                          "Processing Image End");

                // Signal that we have done enough processing.
                if (++(UserDataPtr.ProcessedImageCount) >= NUMBER_OF_FRAMES_TO_PROCESS)
                    MIL.MthrControl(UserDataPtr.DoneEvent, MIL.M_EVENT_SET, MIL.M_SIGNALED);
            }

            return 0;
        }
    }
}
