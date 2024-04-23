//***************************************************************************************
// 
// File name: MdigProcess3D.cpp
// 
// Synopsis:  This program shows the use of the MdigProcess() function and its multiple
//            buffering acquisition to do robust real-time 3D acquisition, processing
//            and display.
// 
//            The user's processing code to execute is located in a callback function
//            that will be called for each frame acquired (see ProcessingFunction()).
// 
//      Note: The average processing time must be shorter than the grab time or some
//            frames will be missed. Also, if the processing results are not displayed
//            the CPU usage is reduced significantly.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
// 
//***************************************************************************************
using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using Matrox.MatroxImagingLibrary;

namespace MDigProcess3D
    {
    class Program
        {
        // Number of images in the buffering grab queue.
        // Generally, increasing this number gives a better real-time grab.
        private const int BUFFERING_SIZE_MAX = 5;

        // User's processing function hook data object.
        public class HookDataStruct
            {
            public MIL_ID MilDigitizer;
            public MIL_ID MilContainerDisp;
            public int ProcessedImageCount;
            };

        // Main function.
        static void Main(string[] args)
            {
            MIL_ID MilApplication = MIL.M_NULL;
            MIL_ID MilSystem = MIL.M_NULL;
            MIL_ID MilDigitizer = MIL.M_NULL;
            MIL_ID MilDisplay = MIL.M_NULL;
            MIL_ID MilContainerDisp = MIL.M_NULL;
            MIL_ID[] MilGrabBufferList = new MIL_ID[BUFFERING_SIZE_MAX];
            int MilGrabBufferListSize = 0;
            MIL_INT ProcessFrameCount = 0;
            double ProcessFrameRate = 0;
            MIL_INT NbFrames = 0, n = 0;

            HookDataStruct UserHookData = new HookDataStruct();

            // Allocate defaults.
            MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT, ref MilApplication);
            MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilSystem);

            if (Alloc3dDisplayAndContainer(MilSystem, ref MilDisplay, ref MilContainerDisp) == false)
                {
                MIL.MsysFree(MilSystem);
                MIL.MappFree(MilApplication);
                Console.ReadKey();
                return;
                }

            MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, ref MilDigitizer);

            // Print a message.
            Console.WriteLine();
            Console.WriteLine("MULTIPLE 3D CONTAINERS PROCESSING.");
            Console.WriteLine("----------------------------------");
            Console.WriteLine();

            // Open the feature browser to setup the camera before acquisition (if not using the System Host simulator).
            if (MIL.MsysInquire(MilSystem, MIL.M_GENICAM_AVAILABLE, MIL.M_NULL) != MIL.M_NO)
                {
                MIL.MdigControl(MilDigitizer, MIL.M_GC_FEATURE_BROWSER, MIL.M_OPEN + MIL.M_ASYNCHRONOUS);
                Console.WriteLine("Please setup your 3D camera using the feature browser.");
                Console.WriteLine("Press <Enter> to start the acquisition.");
                Console.WriteLine();
                Console.ReadKey();
                }

            // Do a first acquisition to determine what is included in the type camera output.
            MIL.MdigGrab(MilDigitizer, MilContainerDisp);

            // Print the acquired MIL Container detailed informations.
            PrintContainerInfo(MilContainerDisp);

            /* If the grabbed Container has 3D data and is Displayable and Processable. */
            if ((MIL.MbufInquireContainer(MilContainerDisp, MIL.M_CONTAINER, MIL.M_3D_DISPLAYABLE, IntPtr.Zero) != MIL.M_NOT_DISPLAYABLE) &&
               (MIL.MbufInquireContainer(MilContainerDisp, MIL.M_CONTAINER, MIL.M_3D_CONVERTIBLE, IntPtr.Zero) != MIL.M_NOT_CONVERTIBLE))
                {
                // Display the Container on the 3D display.
                MIL.M3ddispSelect(MilDisplay, MilContainerDisp, MIL.M_DEFAULT, MIL.M_DEFAULT);

                /* Grab continuously on the 3D display and wait for a key press. */
                MIL.MdigGrabContinuous(MilDigitizer, MilContainerDisp);

                Console.WriteLine("Live 3D acquisition in progress...");
                Console.WriteLine("Press <Enter> to start the processing.");
                Console.ReadKey();

                /* Halt continuous grab. */
                MIL.MdigHalt(MilDigitizer);

                /* Allocate the grab Containers for processing. */
                for (MilGrabBufferListSize = 0; MilGrabBufferListSize < BUFFERING_SIZE_MAX; MilGrabBufferListSize++)
                    {
                    MIL.MbufAllocContainer(MilSystem, MIL.M_PROC | MIL.M_GRAB, MIL.M_DEFAULT, ref MilGrabBufferList[MilGrabBufferListSize]);
                    }

                /* Initialize the user's processing function data structure. */
                UserHookData.MilDigitizer = MilDigitizer;
                UserHookData.MilContainerDisp = MilContainerDisp;
                UserHookData.ProcessedImageCount = 0;

                // get a handle to the HookDataStruct object in the managed heap, we will use this 
                // handle to get the object back in the callback function
                GCHandle hUserData = GCHandle.Alloc(UserHookData);
                MIL_DIG_HOOK_FUNCTION_PTR ProcessingFunctionPtr = new MIL_DIG_HOOK_FUNCTION_PTR(ProcessingFunction);

                // Start the processing. The processing function is called with every frame grabbed.
                MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_START, MIL.M_DEFAULT, ProcessingFunctionPtr, GCHandle.ToIntPtr(hUserData));

                // Here the main() is free to perform other tasks while the processing is executing.
                // ---------------------------------------------------------------------------------

                // Print a message and wait for a key press after a minimum number of frames.
                Console.WriteLine();
                Console.WriteLine("Processing in progress...");
                Console.WriteLine("Press <Enter> to stop.                    ");
                Console.WriteLine();
                Console.ReadKey();

                // Stop the processing.
                MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_STOP, MIL.M_DEFAULT, ProcessingFunctionPtr, GCHandle.ToIntPtr(hUserData));

                // Free the GCHandle when no longer used
                hUserData.Free();

                // Print statistics.
                MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_COUNT, ref ProcessFrameCount);
                MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_RATE, ref ProcessFrameRate);
                Console.WriteLine();
                Console.WriteLine();
                Console.WriteLine("{0} 3D containers grabbed at {1:0.0} frames/sec ({2:0.0} ms/frame).", ProcessFrameCount, ProcessFrameRate, 1000.0 / ProcessFrameRate);
                Console.WriteLine("Press <Enter> to end.");
                Console.WriteLine();
                Console.ReadKey();

                // Free the grab buffers.
                while (MilGrabBufferListSize > 0)
                    {
                    MIL.MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);
                    }
                }
            else
                {
                Console.WriteLine("ERROR: The camera provides no (or more than one) 3D Component(s) of type Range or Disparity.");
                Console.WriteLine("Press <Enter> to end.");
                Console.ReadKey();
                }
            // Free display buffer.
            MIL.M3ddispFree(MilDisplay);
            MIL.MbufFree(MilContainerDisp);
            MIL.MdigFree(MilDigitizer);
            MIL.MsysFree(MilSystem);
            MIL.MappFree(MilApplication);
            }

        // User's processing function called every time a grab buffer is ready.
        // -----------------------------------------------------------------------

        // Local defines.
        private const int STRING_LENGTH_MAX = 20;
        static MIL_INT ProcessingFunction(MIL_INT HookType, MIL_ID HookId, IntPtr HookDataPtr)
            {
            MIL_ID ModifiedBufferId = MIL.M_NULL;

            // this is how to check if the user data is null, the IntPtr class
            // contains a member, Zero, which exists solely for this purpose
            if (!IntPtr.Zero.Equals(HookDataPtr))
                {
                // get the handle to the DigHookUserData object back from the IntPtr
                GCHandle hUserData = GCHandle.FromIntPtr(HookDataPtr);

                // get a reference to the DigHookUserData object
                HookDataStruct UserData = hUserData.Target as HookDataStruct;

                // Retrieve the MIL_ID of the grabbed buffer.
                MIL.MdigGetHookInfo(HookId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID, ref ModifiedBufferId);

                // Increment the frame counter.
                UserData.ProcessedImageCount++;

                // Print and draw the frame count (remove to reduce CPU usage).
                Console.Write("Processing frame #{0}.\r", UserData.ProcessedImageCount);

                // Execute the processing and update the display.
                MIL.MbufConvert3d(ModifiedBufferId, UserData.MilContainerDisp, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_COMPENSATE);
                }

            return 0;
            }
        // Utility function to print the MIL Container detailed informations.
        // ------------------------------------------------------------------
        private static void PrintContainerInfo(MIL_ID MilContainer)
            {
            MIL_INT ComponentCount = MIL.MbufInquire(MilContainer, MIL.M_COMPONENT_COUNT, MIL.M_NULL);
            Console.WriteLine("Container Information:");
            Console.WriteLine("----------------------");
            Console.WriteLine($"Container:    Component Count: {ComponentCount}");
            for (MIL_INT c = 0; c < ComponentCount; c++)
                {
                MIL_ID ComponentId = MIL.M_NULL;
                MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_BY_INDEX(c), MIL.M_COMPONENT_ID, ref ComponentId);
                StringBuilder ComponentName = new StringBuilder();
                MIL.MbufInquire(ComponentId, MIL.M_COMPONENT_TYPE_NAME, ComponentName);
                MIL_INT DataType = MIL.MbufInquire(ComponentId, MIL.M_DATA_TYPE, MIL.M_NULL);
                MIL_INT DataFormat = MIL.MbufInquire(ComponentId, MIL.M_DATA_FORMAT, MIL.M_NULL) & (MIL.M_PACKED | MIL.M_PLANAR);
                long GroupId = 0;
                long SourceId = 0;
                long RegionId = 0;
                MIL.MbufInquire(ComponentId, MIL.M_COMPONENT_GROUP_ID, ref GroupId);
                MIL.MbufInquire(ComponentId, MIL.M_COMPONENT_SOURCE_ID, ref SourceId);
                MIL.MbufInquire(ComponentId, MIL.M_COMPONENT_REGION_ID, ref RegionId);
                Console.WriteLine("Component[{0}]: {1,-11}[{2}:{3}:{4}] Band: {5,1}, Size X: {6,4}, Size Y: {7,4}, Type: {8,2}{9} ({10,6})",
                          c, ComponentName,
                          (long)GroupId,
                          (long)SourceId,
                          (long)RegionId,
                          MIL.MbufInquire(ComponentId, MIL.M_SIZE_BAND, MIL.M_NULL), MIL.MbufInquire(ComponentId, MIL.M_SIZE_X, MIL.M_NULL),
                          MIL.MbufInquire(ComponentId, MIL.M_SIZE_Y, MIL.M_NULL), MIL.MbufInquire(ComponentId, MIL.M_SIZE_BIT, MIL.M_NULL),
                          (DataType == MIL.M_UNSIGNED) ? "u" : (DataType == MIL.M_SIGNED) ? "s" : (DataType == MIL.M_FLOAT) ? "f" : "",
                          (MIL.MbufInquire(ComponentId, MIL.M_SIZE_BAND, MIL.M_NULL) == 1) ? "Mono" : (DataFormat == MIL.M_PLANAR) ? "Planar" : "Packed"
                );
                }
            Console.WriteLine();
            }
        // *****************************************************************************
        // Allocates a 3D display and and a container and returns their MIL identifier
        // *****************************************************************************
        private static bool Alloc3dDisplayAndContainer(MIL_ID MilSystem, ref MIL_ID MilDisplay, ref MIL_ID MilContainerDisp)
            {
            // First we check if the system is local
            if (MIL.MsysInquire(MilSystem, MIL.M_LOCATION, MIL.M_NULL) != MIL.M_LOCAL)
                {
                Console.WriteLine("This example requires a 3D display which is not supported on a remote system.");
                Console.WriteLine("Please select a local system as the default.");
                return false;
                }

            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);
            MilDisplay = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, MIL.M_NULL);
            MilContainerDisp = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC | MIL.M_GRAB | MIL.M_DISP, MIL.M_DEFAULT, MIL.M_NULL);
            if (MilContainerDisp == MIL.M_NULL || MilDisplay == MIL.M_NULL)
                {
                var ErrorMessage = new StringBuilder();
                var ErrorMessageSub1 = new StringBuilder();
                MIL.MappGetError(MIL.M_DEFAULT, MIL.M_GLOBAL + MIL.M_MESSAGE, ErrorMessage);
                MIL.MappGetError(MIL.M_DEFAULT, MIL.M_GLOBAL_SUB_1 + MIL.M_MESSAGE, ErrorMessageSub1);
                Console.WriteLine();
                Console.WriteLine("The current system does not support the 3D display.");
                Console.WriteLine("   " + ErrorMessage.ToString());
                Console.WriteLine("   " + ErrorMessageSub1.ToString());
                Console.WriteLine();
                if (MilDisplay != MIL.M_NULL)
                    {
                    MIL.M3ddispFree(MilDisplay);
                    }
                if (MilContainerDisp != MIL.M_NULL)
                    {
                    MIL.MbufFree(MilContainerDisp);
                    }
                return false;
                }
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);

            return true;
            }
        }
    }
