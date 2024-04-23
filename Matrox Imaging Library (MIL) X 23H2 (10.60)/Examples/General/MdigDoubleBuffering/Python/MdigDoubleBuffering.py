#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#########################################################################################
#
#
# File name: MdigDoubleBuffering.py
#
# Synopsis:  This program performs a double-buffering image acquisition that alternates 
#            between 2 target buffers. This permits the processing of one buffer while 
#            acquiring the next. 
#
#            The example also uses a hook callback function to the start of frames in 
#            order to print the index of the current frame being acquired.
#
#     Note:  The double-buffering method is not recommended for real-time processing, 
#            especially when the CPU usage is high. For more robust real-time behavior,
#            use the MdigProcess() function. See MdigProcess.cpp for a complete example.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
#########################################################################################

import mil as MIL

# Grab data structure prototype. 
class UserDataStruct:
   def __init__(self):
      self.NbGrabStart = 0
   
STRING_LENGTH_MAX = 20

# Main function.
def MdigDoubleBufferingExample():
   MilImage =[]
   NbProc = 0
   UserStruct = UserDataStruct()

   # Allocate defaults.
   MilApplication, MilSystem, MilDisplay, MilDigitizer = MIL.MappAllocDefault(MIL.M_DEFAULT, ImageBufIdPtr=MIL.M_NULL)

   # Allocate a monochrome display buffer. 
   MilImageDisp = MIL.MbufAlloc2d(MilSystem,
      MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X),
      MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y),
      8 + MIL.M_UNSIGNED,
      MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP)
   MIL.MbufClear(MilImageDisp, MIL.M_COLOR_BLACK)

   # Display the image buffer
   MIL.MdispSelect(MilDisplay, MilImageDisp)

   # Allocate 2 grab buffers. 
   for n in range(0, 2):
      MilImage.append(MIL.MbufAlloc2d(MilSystem,
                                      MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X),
                                      MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y),
                                      8 + MIL.M_UNSIGNED,
                                      MIL.M_IMAGE + MIL.M_GRAB + MIL.M_PROC))

   # Hook a function to the start of each frame to print the current frame index.
   GrabStartPtr = MIL.MIL_DIG_HOOK_FUNCTION_PTR(GrabStart)
   MIL.MdigHookFunction(MilDigitizer, MIL.M_GRAB_START, GrabStartPtr, UserStruct)

   # Print a message.
   print("\nDOUBLE BUFFERING ACQUISITION AND PROCESSING:")
   print("--------------------------------------------\n")
   print("Press <Enter> to stop.\n")

   # Put the digitizer in asynchronous mode to be able to process while grabbing.
   MIL.MdigControl(MilDigitizer, MIL.M_GRAB_MODE, MIL.M_ASYNCHRONOUS)

   # Grab the first buffer. 
   MIL.MdigGrab(MilDigitizer, MilImage[0])

   # Process one buffer while grabbing the other.
   n = 0
   while True:
      # Grab the other buffer while processing the previous one. 
      MIL.MdigGrab(MilDigitizer, MilImage[1-n])

      # Synchronize and start the timer. 
      if (NbProc == 0):
         MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL)

      # Write the frame counter. 
      MIL.MgraText(MIL.M_DEFAULT, MilImage[n], 32, 32, str(NbProc+1))

      # Process the first buffer already grabbed.  
      MIL.MimArith(MilImage[n], MIL.M_NULL, MilImageDisp, MIL.M_NOT)

      # Count processed buffers. 
      NbProc += 1

      # Toggle grab buffers. 
      n = 1 - n

      if MIL.MosKbhit():
         break

   # Wait until the end of the last grab and stop the timer. 
   MIL.MdigGrabWait(MilDigitizer, MIL.M_GRAB_END)
   Time = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)
   MIL.MosGetch()

   # Print statistics.
   print("{NbProc} frames processed, at a frame rate of {frameRate} frames/sec ({frameRateMS} ms/frame).\n"
         .format(NbProc=NbProc, frameRate=round(NbProc/Time, 2), frameRateMS=round(1000.0*Time/NbProc, 2)), end='\r\r\r')
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Unhook the function at the start of each frame.
   MIL.MdigHookFunction(MilDigitizer, MIL.M_GRAB_START + MIL.M_UNHOOK, GrabStartPtr, UserStruct)

   # Free allocations. 
   for n in range(0, 2):
      MIL.MbufFree(MilImage[n])

   # Free display buffer. 
   MIL.MbufFree(MilImageDisp)

   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MIL.M_NULL)

   return 0

# Grab Start hook function:
# This function is called at the start of each frame captured.
def GrabStart(HookType, EventId, UserStructPtr):
   UserPtr = UserStructPtr

   # Increment grab start count and print it. 
   UserPtr.NbGrabStart += 1
   print("#{:d}\r".format(UserPtr.NbGrabStart), end='')
 
   return 0

if __name__ == "__main__":
   MdigDoubleBufferingExample()
