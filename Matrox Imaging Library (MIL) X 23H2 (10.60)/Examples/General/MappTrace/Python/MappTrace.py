#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#########################################################################################
#
#
# File name: MappTrace.py
#
# Synopsis:  This example shows how to explicitly control and generate a trace for 
#            MIL functions and how to visualize it using the Matrox Profiler utility. 
#            To generate a trace, you must open Matrox Profiler (accessible from the 
#            MIL Control Center) and select 'Generate New Trace' from the 'File' menu 
#            before to run your MIL application.
# 
# Note:      By default, all MIL applications are traceable without code modifications.
#            You can try this using Matrox Profiler with any MIL example (Ex: MappStart).
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
#########################################################################################

import mil as MIL

# Trace related defines.
TRACE_TAG_HOOK_START         = 1
TRACE_TAG_PROCESSING         = 2
TRACE_TAG_PREPROCESSING      = 3

# General defines
COLOR_BROWN                  = MIL.M_RGB888(100, 65, 50)
BUFFERING_SIZE_MAX           = 3
NUMBER_OF_FRAMES_TO_PROCESS  = 10

class HookDataStruct():
   def __init__(self):
      self.MilImageDisp = 0
      self.MilImageTemp1 = 0
      self.MilImageTemp2 = 0
      self.ProcessedImageCount = 0
      self.DoneEvent = 0

def MappTraceExample():
   TracesActivated = MIL.M_NO
   MilGrabBuf = []

   UserHookData = HookDataStruct()

   print("\nMIL PROGRAM TRACING AND PROFILING:")
   print(  "----------------------------------\n")

   print("This example shows how to generate a trace for the execution")
   print("of the MIL functions, and to visualize it using")
   print("the Matrox Profiler utility.\n")
   print("ACTION REQUIRED:\n")
   if MIL.M_MIL_USE_WINDOWS:
      print("Open 'Matrox Profiler' from the 'MIL Control Center' and")
      print("select 'Generate New Trace' from the 'File' menu.\n")
   else:
      print("Open 'MilConfig' from the 'MIL Control Center' and select the")
      print("'MIL Profiler trace' page in 'Benchmarks and Utilities'.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   ############### Untraceable code section ###############

   # The following code will not be visible in the trace. 

   # MIL application allocation. 
   # At MIL Application allocation time, M_TRACE_LOG_DISABLE can be used to ensures that 
   # an application will not be traceable regardless of Matrox Profiler or MilConfig requests
   # unless traces are explicitly enabled in the program using an MappControl command.
   
   MilApplication = MIL.MappAlloc("M_DEFAULT", MIL.M_TRACE_LOG_DISABLE)

   # Dummy MIL calls that will be invisible in the trace. 
   MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_HOST, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MilDummyBuffer = MIL.MbufAllocColor(MilSystem, 3, 128, 128, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE)
   MIL.MbufClear(MilDummyBuffer, 0)
   MIL.MbufFree(MilDummyBuffer)
   MIL.MsysFree(MilSystem)

   ########################################################

   # Explicitly allow trace logging after a certain point if Matrox Profiler has
   # requested a trace. Note that M_TRACE = M_ENABLE can be used to force the log 
   # of a trace even if Profiler is not opened; M_TRACE = M_DISABLE can prevent 
   # logging of code section.
   
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_TRACE, MIL.M_DEFAULT)

   # Inquire if the traces are active (i.e. Profiler is open and waiting for a trace).
   TracesActivated = MIL.MappInquire(MIL.M_DEFAULT, MIL.M_TRACE_ACTIVE)

   if TracesActivated == MIL.M_YES:
      # Create custom trace markers: setting custom names and colors. 

      # Initialize a custom Tag for the grab callback function with a unique color (blue). 
      MIL.MappTrace(MIL.M_DEFAULT,
                    MIL.M_TRACE_SET_TAG_INFORMATION,
                    TRACE_TAG_HOOK_START,
                    MIL.M_COLOR_BLUE, 
                    "Grab Callback Marker")

      # Initialize the custom Tag for the processing section. 
      MIL.MappTrace(MIL.M_DEFAULT,
                    MIL.M_TRACE_SET_TAG_INFORMATION,
                    TRACE_TAG_PROCESSING,
                    MIL.M_DEFAULT,
                    "Processing Section")

      # Initialize the custom Tag for the preprocessing with a unique color (brown). 
      MIL.MappTrace(MIL.M_DEFAULT,
                    MIL.M_TRACE_SET_TAG_INFORMATION,
                    TRACE_TAG_PREPROCESSING,
                    COLOR_BROWN, 
                    "Preprocessing Marker")

   # Allocate MIL objects.
   MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_HOST, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MilDigitizer = MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
   MilDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)

   SizeX = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X)
   SizeY = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y)

   UserHookData.MilImageDisp = MIL.MbufAllocColor(MilSystem, 3, SizeX, SizeY, 
      8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_GRAB + MIL.M_PROC + MIL.M_DISP)
   MIL.MdispSelect(MilDisplay, UserHookData.MilImageDisp)

   # Allocate the processing temporary buffers. 
   UserHookData.MilImageTemp1 = MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 
      8 + MIL.M_UNSIGNED, MIL.M_PROC + MIL.M_IMAGE)
   UserHookData.MilImageTemp2 = MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 
      8 + MIL.M_UNSIGNED, MIL.M_PROC + MIL.M_IMAGE)
   
   # Allocate grab buffers.
   for NbGrabBuf in range(0, BUFFERING_SIZE_MAX):
      MilGrabBuf.append(MIL.MbufAllocColor(MilSystem, 3, SizeX, SizeY, 8 + MIL.M_UNSIGNED,
         MIL.M_IMAGE + MIL.M_GRAB + MIL.M_PROC))
   
   # Initialize the user's processing function data structure.
   UserHookData.ProcessedImageCount = 0
   UserHookData.DoneEvent = MIL.MthrAlloc(MilSystem, MIL.M_EVENT, 
      MIL.M_NOT_SIGNALED + MIL.M_AUTO_RESET, MIL.M_NULL, MIL.M_NULL)

   # Start processing. The processing function is called with every frame grabbed.
   HookFunctionPtr = MIL.MIL_DIG_HOOK_FUNCTION_PTR(HookFunction)
   MIL.MdigProcess(MilDigitizer, MilGrabBuf, BUFFERING_SIZE_MAX, MIL.M_START,
      MIL.M_DEFAULT, HookFunctionPtr, UserHookData)

   # Stop the processing when the event is triggered.
   MIL.MthrWait(UserHookData.DoneEvent, MIL.M_EVENT_WAIT + MIL.M_EVENT_TIMEOUT(2000), MIL.M_NULL)

   # Stop the processing.
   MIL.MdigProcess(MilDigitizer, MilGrabBuf, BUFFERING_SIZE_MAX, MIL.M_STOP, MIL.M_DEFAULT,
      HookFunctionPtr, UserHookData)

   # Free the grab and temporary buffers.
   for NbGrabBuf in range(0, BUFFERING_SIZE_MAX):
      MIL.MbufFree(MilGrabBuf[NbGrabBuf])
   MIL.MbufFree(UserHookData.MilImageTemp1)
   MIL.MbufFree(UserHookData.MilImageTemp2)

   # Free defaults.
   MIL.MthrFree(UserHookData.DoneEvent)
   MIL.MappFreeDefault(MilApplication,
      MilSystem,
      MilDisplay,
      MilDigitizer,
      UserHookData.MilImageDisp)
   
   # If Matrox Profiler activated the traces, the trace file is now ready. 
   if TracesActivated == MIL.M_YES:
      print("A PROCESSING SEQUENCE WAS EXECUTED AND LOGGED A NEW TRACE:\n")
      
      print("The trace can now be loaded in Matrox Profiler by selecting the")
      print("corresponding file listed in the 'Trace Generation' dialog.\n")

      print("Once loaded, Matrox Profiler's main window displays the 'Main'")
      print("and the 'MdigProcess' threads of the application.\n")
      
      print("- This main window can now be used to select a section")
      print("  of a thread and to zoom or pan in it.\n")

      print("- The right pane shows detailed statistics as well as a")
      print("  'Quick Access' list displaying all MIL function calls.\n")

      print("- The 'User Markers' tab lists the markers and sections logged")
      print("  during the execution. For example, selecting 'Tag:Processing'")
      print("  allows double-clicking to refocus the display on the related")
      print("  calls.\n")

      print("- By clicking a particular MIL function call, either in the")
      print("  'main view' or in the 'Quick Access', additional details")
      print("  are displayed, such as its parameters and execution time.\n")
   
   else:
      print("ERROR: No active tracing detected in MIL Profiler!\n")

   print("Press <Enter> to end.")
   MIL.MosGetch()

   return 0

def HookFunction(HookType, HookId, HookDataPtr):
   PreprocReturnValue = -1
   CurrentImage = MIL.M_NULL
   
   UserDataPtr = HookDataPtr

   # Add a marker to indicate the reception of a new grabbed image.
   MIL.MappTrace(MIL.M_DEFAULT,
                 MIL.M_TRACE_MARKER,
                 TRACE_TAG_HOOK_START,
                 MIL.M_NULL,
                 "New Image grabbed")

   # Retrieve the MIL_ID of the grabbed buffer
   CurrentImage = MIL.MdigGetHookInfo(HookId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID)

   # Start a Section to highlight the processing calls on the image. 
   MIL.MappTrace(MIL.M_DEFAULT,
                 MIL.M_TRACE_SECTION_START,
                 TRACE_TAG_PROCESSING,
                 UserDataPtr.ProcessedImageCount,
                 "Processing Image")

   # Add a Marker to indicate the start of the preprocessing section. 
   MIL.MappTrace(MIL.M_DEFAULT,
                 MIL.M_TRACE_MARKER,
                 TRACE_TAG_PREPROCESSING,
                 UserDataPtr.ProcessedImageCount,
                 "Start Preprocessing")

   # Do the preprocessing. 
   MIL.MimConvert(CurrentImage, UserDataPtr.MilImageTemp1, MIL.M_RGB_TO_L)
   MIL.MimHistogramEqualize(UserDataPtr.MilImageTemp1, UserDataPtr.MilImageTemp1, 
      MIL.M_UNIFORM, MIL.M_NULL, 55, 200)

   # Add a Marker to indicate the end of the preprocessing section. 
   MIL.MappTrace(MIL.M_DEFAULT,
                 MIL.M_TRACE_MARKER,
                 TRACE_TAG_PREPROCESSING,
                 UserDataPtr.ProcessedImageCount,
                 "End Preprocessing")

   # Do the main processing. 
   MIL.MimBinarize(UserDataPtr.MilImageTemp1, UserDataPtr.MilImageTemp2,
      MIL.M_IN_RANGE, 120, 140)
   MIL.MimBinarize(UserDataPtr.MilImageTemp1, UserDataPtr.MilImageTemp1,
      MIL.M_IN_RANGE, 220, 255)
   MIL.MimArith(UserDataPtr.MilImageTemp1, UserDataPtr.MilImageTemp2, 
      UserDataPtr.MilImageDisp, MIL.M_OR)
   
   # End the Section that highlights the processing. 
   MIL.MappTrace(MIL.M_DEFAULT,
                 MIL.M_TRACE_SECTION_END,
                 TRACE_TAG_PROCESSING,
                 UserDataPtr.ProcessedImageCount,
                 "Processing Image End")

   # Signal that processing has been completed. 
   UserDataPtr.ProcessedImageCount += 1
   if UserDataPtr.ProcessedImageCount >= NUMBER_OF_FRAMES_TO_PROCESS:
      MIL.MthrControl(UserDataPtr.DoneEvent, MIL.M_EVENT_SET, MIL.M_SIGNALED)
   
   return 0

if __name__ == "__main__":
   MappTraceExample()