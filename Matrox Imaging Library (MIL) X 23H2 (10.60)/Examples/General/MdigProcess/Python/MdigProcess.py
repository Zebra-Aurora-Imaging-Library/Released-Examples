#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# 
#  File name: MdigProcess.py  
#
#   Synopsis:  This program shows the use of the MdigProcess() function and its multiple
#              buffering acquisition to do robust real-time processing.           
#  
#              The user's processing code to execute is located in a callback function 
#              that will be called for each frame acquired (see ProcessingFunction()).
#    
#        Note: The average processing time must be shorter than the grab time or some
#              frames will be missed. Also, if the processing results are not displayed
#              and the frame count is not drawn or printed, the CPU usage is reduced 
#              significantly.
#
#  Copyright © Matrox Electronic Systems Ltd., 1992-2023.
#  All Rights Reserved

import mil as MIL

# User's processing function hook data structure.
class HookDataStruct():
   def __init__(self, MilDigitizer, MilImageDisp, ProcessedImageCount):
      self.MilDigitizer = MilDigitizer
      self.MilImageDisp = MilImageDisp
      self.ProcessedImageCount = ProcessedImageCount

# Number of images in the buffering grab queue.
# Generally, increasing this number gives a better real-time grab.
BUFFERING_SIZE_MAX = 20

# User's processing function called every time a grab buffer is ready.
# --------------------------------------------------------------------

# Local defines. 
STRING_POS_X       = 20 
STRING_POS_Y       = 20 

def ProcessingFunction(HookType, HookId, HookDataPtr):
   
   # Retrieve the MIL_ID of the grabbed buffer. 
   ModifiedBufferId = MIL.MdigGetHookInfo(HookId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID)
   
   # Extract the userdata structure
   UserData = HookDataPtr

   # Increment the frame counter.
   UserData.ProcessedImageCount += 1
   
   # Print and draw the frame count (remove to reduce CPU usage).
   print("Processing frame #{:d}.\r".format(UserData.ProcessedImageCount), end='')
   MIL.MgraText(MIL.M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, "{:d}".format(UserData.ProcessedImageCount))
   
   # Execute the processing and update the display.
   MIL.MimArith(float(ModifiedBufferId), 0.0, UserData.MilImageDisp, MIL.M_NOT)
   
   return 0
   
# Main function.
# ---------------

def MdigProcessExample():
   # Allocate defaults.
   MilApplication = MIL.MappAlloc("M_DEFAULT", MIL.M_DEFAULT)
   MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MilDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
   MilDigitizer = MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)

   SizeX = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X)
   SizeY = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y)

   MilImageDisp = MIL.MbufAlloc2d(MilSystem,
                                     SizeX,
                                     SizeY, 
                                     8 + MIL.M_UNSIGNED, 
                                     MIL.M_IMAGE + 
                                     MIL.M_PROC + MIL.M_DISP + MIL.M_GRAB)

   MIL.MbufClear(MilImageDisp, MIL.M_COLOR_BLACK)
   MIL.MdispSelect(MilDisplay, MilImageDisp)
     
   # Print a message.
   print("\nMULTIPLE BUFFERED PROCESSING.")
   print("-----------------------------\n")

   # Grab continuously on the display and wait for a key press
   MIL.MdigGrabContinuous(MilDigitizer, MilImageDisp)
   print("Press <Enter> to start processing.\n")
   MIL.MosGetch()

   # Halt continuous grab. 
   MIL.MdigHalt(MilDigitizer)

   # Allocate the grab buffers and clear them.
   MilGrabBufferList = []
   MilGrabBufferListSize = 0
   for n in range(0, BUFFERING_SIZE_MAX):
      if (n == 2):
         MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
      MilGrabBufferList.append(MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_GRAB + MIL.M_PROC))
      if (MilGrabBufferList[n] != MIL.M_NULL):
         MIL.MbufClear(MilGrabBufferList[n], 0xFF)
         MilGrabBufferListSize += 1
      else:
         break
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)

   # Initialize the user's processing function data structure.
   UserHookData = HookDataStruct(MilDigitizer, MilImageDisp, 0)
   
   # Start the processing. The processing function is called with every frame grabbed.
   ProcessingFunctionPtr = MIL.MIL_DIG_HOOK_FUNCTION_PTR(ProcessingFunction)
   MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_START, MIL.M_DEFAULT, ProcessingFunctionPtr, UserHookData)

   # Here the main() is free to perform other tasks while the processing is executing.
   # ---------------------------------------------------------------------------------

   # Print a message and wait for a key press after a minimum number of frames.
   print("Press <Enter> to stop.                    \n")
   MIL.MosGetch()

   # Stop the processing.
   MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_STOP, MIL.M_DEFAULT, ProcessingFunctionPtr, UserHookData)

   # Print statistics.
   ProcessFrameCount = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_COUNT)
   ProcessFrameRate = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_RATE)

   if (ProcessFrameRate > 0.0):
      print("\n\n{:d} frames grabbed at {:.1f} frames/sec ({:.1f} ms/frame).".format(ProcessFrameCount, ProcessFrameRate, 1000.0/ProcessFrameRate))
   else:
      print("\n\nNo data was grabbed.\n")

   print("Press <Enter> to end.\n")
   MIL.MosGetch()
      
   # Free the grab buffers.
   for id in range(0, MilGrabBufferListSize):
      MIL.MbufFree(MilGrabBufferList[id])
      
   # Release defaults.
   MIL.MbufFree(MilImageDisp)
   MIL.MdispFree(MilDisplay)
   MIL.MdigFree(MilDigitizer)
   MIL.MsysFree(MilSystem)
   MIL.MappFree(MilApplication)
   
   return
   
if __name__ == "__main__":
   MdigProcessExample()
