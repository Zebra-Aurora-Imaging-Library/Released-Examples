#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#########################################################################################
#
#  File name: MdigProcess3D.py
#
#  Synopsis: This program shows the use of the MdigProcess() function and its
#  multiple
#             buffering acquisition to do robust real-time 3D acquisition,
#             processing
#             and display.
#
#             The user's processing code to execute is located in a callback
#             function
#             that will be called for each frame acquired (see
#             ProcessingFunction()).
#
#       Note: The average processing time must be shorter than the grab time or
#       some
#             frames will be missed.  Also, if the processing results are not
#             displayed
#             the CPU usage is reduced significantly.
#
#  Copyright © Matrox Electronic Systems Ltd., 1992-2023.
#  All Rights Reserved
#

import sys
import mil as MIL

# User's processing function hook data structure.
class HookDataStruct():
   def __init__(self, MilDigitizer, MilContainerDisp, ProcessedImageCount):
      self.MilDigitizer = MilDigitizer
      self.MilContainerDisp = MilContainerDisp
      self.ProcessedImageCount = ProcessedImageCount

# Number of images in the buffering grab queue.
# Generally, increasing this number gives a better real-time grab.
BUFFERING_SIZE_MAX = 5

# User's processing function called every time a grab buffer is ready.
# --------------------------------------------------------------------

def ProcessingFunction(HookType, HookId, HookDataPtr):
   
   # Retrieve the MIL_ID of the grabbed buffer.
   ModifiedBufferId = MIL.MdigGetHookInfo(HookId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID)
   
   # Extract the userdata structure
   UserData = HookDataPtr
   
   # Increment the frame counter.
   UserData.ProcessedImageCount += 1
   
   # Print and draw the frame count (remove to reduce CPU usage).
   print("Processing frame #{:d}.\r".format(UserData.ProcessedImageCount), end='')
   
   # Execute the processing and update the display.
   MIL.MbufConvert3d(ModifiedBufferId, UserData.MilContainerDisp, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_COMPENSATE)
   
   return 0
   
# Main function.
# ---------------
def MdigProcessExample():
   # Allocate defaults.
   MilApplication = MIL.MappAlloc("M_DEFAULT", MIL.M_DEFAULT)
   MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   status, MilDisplay, MilContainerDisp = Alloc3dDisplayAndContainer(MilSystem)
   if status == False:
      MIL.MsysFree(MilSystem)
      MIL.MappFree(MilApplication)
      input()
      return

   MilDigitizer = MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
     
   # Print a message.
   print("\nMULTIPLE 3D CONTAINERS PROCESSING.")
   print("----------------------------------\n")

   # Open the feature browser to setup the camera before acquisition (if not
   # using the System Host simulator).
   if MIL.MsysInquire(MilSystem, MIL.M_GENICAM_AVAILABLE) != MIL.M_NO:
      MIL.MdigControl(MilDigitizer, MIL.M_GC_FEATURE_BROWSER, MIL.M_OPEN + MIL.M_ASYNCHRONOUS)
      print("Please setup your 3D camera using the feature browser.")
      print("Press <Enter> to start the acquisition.")
      input()

   # Do a first acquisition to determine what is included in the type camera
   # output.
   MIL.MdigGrab(MilDigitizer, MilContainerDisp)

   # Print the acquired MIL Container detailed informations.
   PrintContainerInfo(MilContainerDisp)

   # If the grabbed Container has 3D data and is Displayable and Processable.
   if (MIL.MbufInquireContainer(MilContainerDisp, MIL.M_CONTAINER, MIL.M_3D_DISPLAYABLE) != MIL.M_NOT_DISPLAYABLE) and (MIL.MbufInquireContainer(MilContainerDisp, MIL.M_CONTAINER, MIL.M_3D_CONVERTIBLE) != MIL.M_NOT_CONVERTIBLE):
      # Display the Container on the 3D display.
      MIL.M3ddispSelect(MilDisplay, MilContainerDisp, MIL.M_DEFAULT, MIL.M_DEFAULT)

      # Grab continuously on the display and wait for a key press
      MIL.MdigGrabContinuous(MilDigitizer, MilContainerDisp)
      print("Live 3D acquisition in progress...")
      input("Press <Enter> to start the processing.\n")

      # Halt continuous grab.
      MIL.MdigHalt(MilDigitizer)

      # Allocate the grab buffers and clear them.
      MilGrabBufferList = []
      MilGrabBufferListSize = 0
      for n in range(0, BUFFERING_SIZE_MAX):
         MilGrabBufferList.append(MIL.MbufAllocContainer(MilSystem, MIL.M_PROC + MIL.M_GRAB, MIL.M_DEFAULT))
         if (MilGrabBufferList[n] != MIL.M_NULL):
            MIL.MbufClear(MilGrabBufferList[n], 0xFF)
            MilGrabBufferListSize += 1
         else:
            break

      # Initialize the user's processing function data structure.
      UserHookData = HookDataStruct(MilDigitizer, MilContainerDisp, 0)

      # Start the processing.  The processing function is called with every
      # frame grabbed.
      ProcessingFunctionPtr = MIL.MIL_DIG_HOOK_FUNCTION_PTR(ProcessingFunction)
      MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_START, MIL.M_DEFAULT, ProcessingFunctionPtr, UserHookData)

      # Here the main() is free to perform other tasks while the processing is
      # executing.
      # ---------------------------------------------------------------------------------

      # Print a message and wait for a key press after a minimum number of
      # frames.
      print("Processing in progress...")
      input("Press <Enter> to stop.                    \n\n")

      # Print a message and wait for a key press after a minimum number of
      # frames.
      MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_STOP, MIL.M_DEFAULT, ProcessingFunctionPtr, UserHookData)

      # Print statistics.
      ProcessFrameCount = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_COUNT)
      ProcessFrameRate = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_RATE)
      print("\n{:d} 3D containers grabbed at {:.1f} frames/sec ({:.1f} ms/frame)".format(ProcessFrameCount, ProcessFrameRate, 1000.0 / ProcessFrameRate))
      input("Press <Enter> to end.\n")
      
      # Free the grab buffers.
      for id in range(0, MilGrabBufferListSize):
         MIL.MbufFree(MilGrabBufferList[id])
   else:
      print("ERROR: The camera provides no (or more than one) 3D Component(s) of type Range or Disparity.\nPress <Enter> to end.\n")
      input()

   # Release defaults.
   MIL.MbufFree(MilContainerDisp)
   MIL.M3ddispFree(MilDisplay)
   MIL.MdigFree(MilDigitizer)
   MIL.MsysFree(MilSystem)
   MIL.MappFree(MilApplication)
   
   return

# Utility function to print the MIL Container detailed informations.
# ------------------------------------------------------------------
def  PrintContainerInfo(MilContainer):
   ComponentCount = MIL.MbufInquire(MilContainer, MIL.M_COMPONENT_COUNT)
   print("Container Information:")
   print("----------------------")
   print("Container:    Component Count: {:d}".format(ComponentCount))
   for  c in range(0,ComponentCount):
      ComponentId = MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_BY_INDEX(c), MIL.M_COMPONENT_ID)
      ComponentName = MIL.MbufInquire(ComponentId, MIL.M_COMPONENT_TYPE_NAME)
      DataType = MIL.MbufInquire(ComponentId, MIL.M_DATA_TYPE)
      
      if DataType == MIL.M_UNSIGNED :
         DataTypeStr = "u"
      elif DataType == MIL.M_SIGNED:
         DataTypeStr = "s"
      elif DataType == MIL.M_FLOAT:
         DataTypeStr = "f"
      else:
         DataTypeStr = ""

      DataFormat = MIL.MbufInquire(ComponentId, MIL.M_DATA_FORMAT) & (MIL.M_PACKED | MIL.M_PLANAR)
      if MIL.MbufInquire(ComponentId, MIL.M_SIZE_BAND) == 1:
         SizeBandStr = "  Mono"
      elif DataFormat == MIL.M_PLANAR:
         SizeBandStr = "Planar"
      else:
         SizeBandStr == "Packed"

      GroupId = MIL.MbufInquire(ComponentId, MIL.M_COMPONENT_GROUP_ID)
      SourceId = MIL.MbufInquire(ComponentId, MIL.M_COMPONENT_SOURCE_ID)
      RegionId = MIL.MbufInquire(ComponentId, MIL.M_COMPONENT_REGION_ID)
      print("Component[{:d}]: {:11s}[{:d}:{:d}:{:d}] Band: {:1d}, Size X: {:4d}, Size Y: {:4d}, Type: {:2d}{:s} ({:6s})".format(c, ComponentName,
                GroupId,
                SourceId,
                RegionId,
                MIL.MbufInquire(ComponentId, MIL.M_SIZE_BAND), MIL.MbufInquire(ComponentId, MIL.M_SIZE_X),
                MIL.MbufInquire(ComponentId, MIL.M_SIZE_Y), MIL.MbufInquire(ComponentId, MIL.M_SIZE_BIT),
                DataTypeStr,
                SizeBandStr))
   print("")

# *****************************************************************************
# Allocates a 3D display and returns its MIL identifier.
# *****************************************************************************
def Alloc3dDisplayAndContainer(MilSystem):
   # First we check if the system is local
   if (MIL.MsysInquire(MilSystem, MIL.M_LOCATION) != MIL.M_LOCAL):
      print("This example requires a 3D display which is not supported on a remote system.")
      print("Please select a local system as the default.")
      return (False, MIL.M_NULL, MIL.M_NULL)

   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
   MilDisplay = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
   MilContainerDisp = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC + MIL.M_DISP + MIL.M_GRAB, MIL.M_DEFAULT)
   if (MilContainerDisp == MIL.M_NULL) or (MilDisplay == MIL.M_NULL):
      ErrorMessage = MIL.MappGetError(MIL.M_DEFAULT, MIL.M_GLOBAL + MIL.M_MESSAGE)
      ErrorMessageSub1 = MIL.MappGetError(MIL.M_DEFAULT, MIL.M_GLOBAL_SUB_1 + MIL.M_MESSAGE)
      
      print()
      print("The current system does not support the 3D display.")
      print("   " + ErrorMessage)
      print("   " + ErrorMessageSub1)
      print()
      if MilDisplay != MIL.M_NULL:
         MIL.M3ddispFree(MilDisplay)
      if MilContainerDisp != MIL.M_NULL:
         MIL.MbufFree(MilContainerDisp)
      return (False, MIL.M_NULL, MIL.M_NULL)

   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)

   return (True, MilDisplay, MilContainerDisp)

if __name__ == "__main__":
   MdigProcessExample()
