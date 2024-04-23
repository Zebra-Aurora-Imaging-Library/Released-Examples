#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: Mthread.py 
#
# Synopsis:  This program shows how to use threads in a MIL application and
#            synchronize them with event. It creates 4 processing threads that
#            are used to work in 4 different regions of a display buffer.
#
#     Thread usage:
#      - The main thread starts a processing thread in each of the 4 different
#        quarters of a display buffer. The main thread then waits for a key to
#        be pressed to stop them.
#      - The top-left and bottom-left threads work in a loop, as follows: the
#        top-left thread adds a constant to its buffer, then sends an event to
#        the bottom-left thread. The bottom-left thread waits for the event
#        from the top-left thread, rotates the top-left buffer image, then sends an
#        event to the top-left thread to start a new loop.
#      - The top-right and bottom-right threads work the same way as the
#        top-left and bottom-left threads, except that the bottom-right thread
#        performs an edge detection operation, rather than a rotation.
#
#      Note : - Under MIL-Lite, the threads will do graphic annotations instead.
#             - Comment out the MdispSelect() if you wish to avoid benchmarking
#               the display update overhead on CPU usage and processing rate.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################


import mil as MIL

# Local defines.
IMAGE_FILE         = MIL.M_IMAGE_PATH + "Bird.mim"
IMAGE_WIDTH        = 256
IMAGE_HEIGHT       = 240
STRING_LENGTH_MAX  = 40
STRING_POS_X       = 10
STRING_POS_Y       = 220
DRAW_RADIUS_NUMBER = 5
DRAW_RADIUS_STEP   = 10
DRAW_CENTER_POSX   = 196
DRAW_CENTER_POSY   = 180

class ThreadParam:
   def __init__(self):
      self.Id = None
      self.System = None
      self.OrgImage = None
      self.SrcImage = None
      self.DstImage = None
      self.DispImage = None
      self.DispOffsetX = None
      self.DispOffsetY = None
      self.ReadyEvent = None
      self.DoneEvent = None
      self.NumberOfIteration = None
      self.Radius = None
      self.Exit = None
      self.LicenseModules = None
      self.SlaveThreadParam = None


# Main function: #
# -------------- #
def MthreadExample():

   TParTopLeft = ThreadParam()
   TParTopRight = ThreadParam()
   TParBotLeft = ThreadParam()
   TParBotRight = ThreadParam()

   # Allocate defaults. 
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Allocate and display the main image buffer. 
   MilImage = MIL.MbufAlloc2d(MilSystem, IMAGE_WIDTH * 2, IMAGE_HEIGHT * 2, 8 + MIL.M_UNSIGNED,
                              MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP)

   MIL.MbufClear(MilImage, 0)
   MIL.MdispSelect(MilDisplay, MilImage)
   TParTopLeft.DispImage = MIL.MdispInquire(MilDisplay, MIL.M_SELECTED)

   # Allocate an image buffer to keep the original.
   MilOrgImage = MIL.MbufAlloc2d(MilSystem, IMAGE_WIDTH, IMAGE_HEIGHT, 8 + MIL.M_UNSIGNED,
                                 MIL.M_IMAGE + MIL.M_PROC)

   # Allocate a processing buffer for each thread.
   TParTopLeft.SrcImage = MIL.MbufAlloc2d(MilSystem, IMAGE_WIDTH, IMAGE_HEIGHT, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC)
   TParBotLeft.DstImage = MIL.MbufAlloc2d(MilSystem, IMAGE_WIDTH, IMAGE_HEIGHT, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC)
   TParTopRight.SrcImage = MIL.MbufAlloc2d(MilSystem, IMAGE_WIDTH, IMAGE_HEIGHT, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC)
   TParBotRight.DstImage = MIL.MbufAlloc2d(MilSystem, IMAGE_WIDTH, IMAGE_HEIGHT, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC)

   # Allocate synchronization events. 
   TParTopLeft.DoneEvent = MIL.MthrAlloc(MilSystem, MIL.M_EVENT, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL)
   TParBotLeft.DoneEvent = MIL.MthrAlloc(MilSystem, MIL.M_EVENT, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL)
   TParTopRight.DoneEvent = MIL.MthrAlloc(MilSystem, MIL.M_EVENT, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL)
   TParBotRight.DoneEvent = MIL.MthrAlloc(MilSystem, MIL.M_EVENT, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL)

   # Inquire MIL licenses. 
   MilRemoteApplication = MIL.MsysInquire(MilSystem, MIL.M_OWNER_APPLICATION)
   LicenseModules = MIL.MappInquire(MilRemoteApplication, MIL.M_LICENSE_MODULES)

   # Initialize remaining thread parameters. 
   TParTopLeft.System               = MilSystem
   TParTopLeft.OrgImage             = MilOrgImage
   TParTopLeft.DstImage             = TParTopLeft.SrcImage
   TParTopLeft.DispOffsetX          = 0
   TParTopLeft.DispOffsetY          = 0
   TParTopLeft.ReadyEvent           = TParBotLeft.DoneEvent
   TParTopLeft.NumberOfIteration    = 0
   TParTopLeft.Radius               = 0
   TParTopLeft.Exit                 = 0
   TParTopLeft.LicenseModules       = LicenseModules
   TParTopLeft.SlaveThreadParam     = TParBotLeft

   TParBotLeft.System               = MilSystem
   TParBotLeft.OrgImage             = 0
   TParBotLeft.SrcImage             = TParTopLeft.DstImage
   TParBotLeft.DispImage            = TParTopLeft.DispImage
   TParBotLeft.DispOffsetX          = 0
   TParBotLeft.DispOffsetY          = IMAGE_HEIGHT
   TParBotLeft.ReadyEvent           = TParTopLeft.DoneEvent
   TParBotLeft.NumberOfIteration    = 0
   TParBotLeft.Radius               = 0
   TParBotLeft.Exit                 = 0
   TParBotLeft.LicenseModules       = LicenseModules
   TParBotLeft.SlaveThreadParam     = 0

   TParTopRight.System              = MilSystem
   TParTopRight.OrgImage            = MilOrgImage
   TParTopRight.DstImage            = TParTopRight.SrcImage
   TParTopRight.DispImage           = TParTopLeft.DispImage
   TParTopRight.DispOffsetX         = IMAGE_WIDTH
   TParTopRight.DispOffsetY         = 0
   TParTopRight.ReadyEvent          = TParBotRight.DoneEvent
   TParTopRight.NumberOfIteration   = 0
   TParTopRight.Radius              = 0
   TParTopRight.Exit                = 0
   TParTopRight.LicenseModules      = LicenseModules
   TParTopRight.SlaveThreadParam    = TParBotRight

   TParBotRight.System              = MilSystem
   TParBotRight.OrgImage            = 0
   TParBotRight.SrcImage            = TParTopRight.DstImage
   TParBotRight.DispImage           = TParTopLeft.DispImage
   TParBotRight.DispOffsetX         = IMAGE_WIDTH
   TParBotRight.DispOffsetY         = IMAGE_HEIGHT
   TParBotRight.ReadyEvent          = TParTopRight.DoneEvent
   TParBotRight.NumberOfIteration   = 0
   TParBotRight.Radius              = 0
   TParBotRight.Exit                = 0
   TParBotRight.LicenseModules      = LicenseModules
   TParBotRight.SlaveThreadParam    = 0

   # Initialize the original image to process. 
   MIL.MbufLoad(IMAGE_FILE, MilOrgImage)

   # Start the 4 threads. 

   TopThreadPtr = MIL.MIL_THREAD_FUNCTION_PTR(TopThread)
   BotLeftThreadPtr = MIL.MIL_THREAD_FUNCTION_PTR(BotLeftThread)
   BotRightThreadPtr = MIL.MIL_THREAD_FUNCTION_PTR(BotRightThread)

   TParTopLeft.Id = MIL.MthrAlloc(MilSystem, MIL.M_THREAD, MIL.M_DEFAULT, TopThreadPtr, TParTopLeft)
   TParBotLeft.Id = MIL.MthrAlloc(MilSystem, MIL.M_THREAD, MIL.M_DEFAULT, BotLeftThreadPtr, TParBotLeft)
   TParTopRight.Id = MIL.MthrAlloc(MilSystem, MIL.M_THREAD, MIL.M_DEFAULT, TopThreadPtr, TParTopRight)
   TParBotRight.Id = MIL.MthrAlloc(MilSystem, MIL.M_THREAD, MIL.M_DEFAULT, BotRightThreadPtr, TParBotRight)

   # Start the timer.
   MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL)

   # Set events to start operation of top-left and top-right threads. 
   MIL.MthrControl(TParTopLeft.ReadyEvent,  MIL.M_EVENT_SET, MIL.M_SIGNALED)
   MIL.MthrControl(TParTopRight.ReadyEvent, MIL.M_EVENT_SET, MIL.M_SIGNALED)

   # Report that the threads are started and wait for a key press to stop them.
   print("\nMULTI-THREADING:")
   print("----------------\n")
   print("4 threads running...")
   print("Press <Enter> to stop.\n")
   MIL.MosGetch()

   # Signal the threads to exit.
   TParTopLeft.Exit  = 1
   TParTopRight.Exit = 1

   # Wait for all threads to terminate. 
   MIL.MthrWait(TParTopLeft.Id , MIL.M_THREAD_END_WAIT, MIL.M_NULL)
   MIL.MthrWait(TParBotLeft.Id , MIL.M_THREAD_END_WAIT, MIL.M_NULL)
   MIL.MthrWait(TParTopRight.Id, MIL.M_THREAD_END_WAIT, MIL.M_NULL)
   MIL.MthrWait(TParBotRight.Id, MIL.M_THREAD_END_WAIT, MIL.M_NULL)

   # Stop the timer and calculate the number of frames per second processed.
   Time = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)
   FramesPerSecond = (TParTopLeft.NumberOfIteration + TParBotLeft.NumberOfIteration +
                     TParTopRight.NumberOfIteration + TParBotRight.NumberOfIteration)/Time

   # Print statistics. 
   print("Top-left iterations done:     {NumIteration}.".format(NumIteration=TParTopLeft.NumberOfIteration))
   print("Bottom-left iterations done:  {NumIteration}.".format(NumIteration=TParBotLeft.NumberOfIteration))
   print("Top-right iterations done:    {NumIteration}.".format(NumIteration=TParTopRight.NumberOfIteration))
   print("Bottom-right iterations done: {NumIteration}.\n".format(NumIteration=TParBotRight.NumberOfIteration))
   print("Processing speed for the 4 threads: {FramesPerSecond} Images/Sec.\n".format(FramesPerSecond=round(FramesPerSecond)))
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free threads.
   MIL.MthrFree(TParTopLeft.Id)
   MIL.MthrFree(TParBotLeft.Id)
   MIL.MthrFree(TParTopRight.Id)
   MIL.MthrFree(TParBotRight.Id)

   # Free events. 
   MIL.MthrFree(TParTopLeft.DoneEvent)
   MIL.MthrFree(TParBotLeft.DoneEvent)
   MIL.MthrFree(TParTopRight.DoneEvent)
   MIL.MthrFree(TParBotRight.DoneEvent)

   # Free buffers. 
   MIL.MbufFree(TParTopLeft.SrcImage)
   MIL.MbufFree(TParTopRight.SrcImage)
   MIL.MbufFree(TParBotLeft.DstImage)
   MIL.MbufFree(TParBotRight.DstImage)
   MIL.MbufFree(MilOrgImage)
   MIL.MbufFree(MilImage)

   # Free defaults. 
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0

# Top-left and top-right threads' function (Add an offset): #
# --------------------------------------------------------- #
def TopThread(ThreadParameters):
   TPar = ThreadParameters

   while not TPar.Exit:
      # Wait for bottom ready event before proceeding. 
      MIL.MthrWait(TPar.ReadyEvent, MIL.M_EVENT_WAIT, MIL.M_NULL)

      # For better visual effect, reset SrcImage to the original image regularly. 
      if (TPar.NumberOfIteration % 192) == 0:
         MIL.MbufCopy(TPar.OrgImage, TPar.SrcImage)

      if TPar.LicenseModules & MIL.M_LICENSE_IM:
         # Add a constant to the image. 
         MIL.MimArith(TPar.SrcImage, 1, TPar.DstImage, MIL.M_ADD_CONST + MIL.M_SATURATION)
      else:
         # Under MIL-Lite draw a variable size rectangle in the image. 
         TPar.Radius = TPar.SlaveThreadParam.Radius = (TPar.NumberOfIteration % DRAW_RADIUS_NUMBER) * DRAW_RADIUS_STEP
         MIL.MgraColor(MIL.M_DEFAULT, 0xff)
         MIL.MgraRectFill(MIL.M_DEFAULT, TPar.DstImage, 
                          DRAW_CENTER_POSX - TPar.Radius, DRAW_CENTER_POSY - TPar.Radius, 
                          DRAW_CENTER_POSX + TPar.Radius, DRAW_CENTER_POSY + TPar.Radius)

      # Increment iteraton count and draw text.
      TPar.NumberOfIteration += 1
      MIL.MgraColor(MIL.M_DEFAULT, 0xFF)
      MIL.MgraText(MIL.M_DEFAULT, TPar.DstImage, STRING_POS_X, STRING_POS_Y, str(TPar.NumberOfIteration))

      # Update the display. 
      if TPar.DispImage:
         MIL.MbufCopyColor2d(TPar.DstImage,
                             TPar.DispImage,
                             MIL.M_ALL_BANDS, 0, 0,
                             MIL.M_ALL_BANDS,
                             TPar.DispOffsetX,
                             TPar.DispOffsetY,
                             IMAGE_WIDTH,
                             IMAGE_HEIGHT)
      
      # Signal to the bottom thread that the first part of the processing is completed. s
      MIL.MthrControl(TPar.DoneEvent, MIL.M_EVENT_SET, MIL.M_SIGNALED)

   # Require the bottom thread to exit. 
   TPar.SlaveThreadParam.Exit = 1

   # Signal the bottom thread to wake up. 
   MIL.MthrControl(TPar.DoneEvent, MIL.M_EVENT_SET, MIL.M_SIGNALED)

   # Before exiting the thread, make sure that all the commands are executed. 
   MIL.MthrWait(TPar.System, MIL.M_THREAD_WAIT, MIL.M_NULL)
   return 1

# Bottom-left thread function (Rotate): #
# ------------------------------------- #
def BotLeftThread(ThreadParameters):
   TPar = ThreadParameters
   Angle = 0.0
   AngleIncrement = 0.5

   while not TPar.Exit:
      # Wait for the event in top-left function to be ready before proceeding. 
      MIL.MthrWait(TPar.ReadyEvent, MIL.M_EVENT_WAIT, MIL.M_NULL)

      if TPar.LicenseModules & MIL.M_LICENSE_IM:
         # Rotate the image. 
         MIL.MimRotate(TPar.SrcImage, TPar.DstImage, Angle,
                       MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                       MIL.M_NEAREST_NEIGHBOR + MIL.M_OVERSCAN_CLEAR)

         Angle += AngleIncrement

         if Angle >= 360:
            Angle -= 360
      else:
         # Under MIL-Lite copy the top-left image and draw 
         # a variable size filled circle in the image. 
         MIL.MbufCopy(TPar.SrcImage, TPar.DstImage)
         MIL.MgraColor(MIL.M_DEFAULT, 0x80)
         MIL.MgraArcFill(MIL.M_DEFAULT, TPar.DstImage, DRAW_CENTER_POSX, DRAW_CENTER_POSY,
                                                       TPar.Radius, TPar.Radius, 0, 360)
      # Increment iteration count and draw text. 
      TPar.NumberOfIteration += 1
      MIL.MgraColor(MIL.M_DEFAULT, 0xFF)
      MIL.MgraText(MIL.M_DEFAULT, TPar.DstImage, STRING_POS_X, STRING_POS_Y, str(TPar.NumberOfIteration))

      # Update the display. 
      if TPar.DispImage:
         MIL.MbufCopyColor2d(TPar.DstImage,
                             TPar.DispImage,
                             MIL.M_ALL_BANDS, 0, 0,
                             MIL.M_ALL_BANDS,
                             TPar.DispOffsetX,
                             TPar.DispOffsetY,
                             IMAGE_WIDTH,
                             IMAGE_HEIGHT)
      
      # Signal to the top-left thread that the last part of the processing is completed. 
      MIL.MthrControl(TPar.DoneEvent, MIL.M_EVENT_SET, MIL.M_SIGNALED)
      
   # Before exiting the thread, make sure that all the commands are executed. 
   MIL.MthrWait(TPar.System, MIL.M_THREAD_WAIT, MIL.M_NULL)

   return 1

# Bottom-right thread function (Edge Detect): #
# ------------------------------------------- #
def BotRightThread(ThreadParameters):
   TPar = ThreadParameters

   while not TPar.Exit:
      # Wait for the event in top-right function to be ready before proceeding. 
      MIL.MthrWait(TPar.ReadyEvent, MIL.M_EVENT_WAIT, MIL.M_NULL)

      if TPar.LicenseModules & MIL.M_LICENSE_IM:
         # Perform an edge detection operation on the image. 
         MIL.MimConvolve(TPar.SrcImage, TPar.DstImage, MIL.M_EDGE_DETECT_SOBEL_FAST)
      else:
         # Under MIL-Lite copy the top-right image and draw 
         # a variable size filled circle in the image. 
         MIL.MbufCopy(TPar.SrcImage, TPar.DstImage)
         MIL.MgraColor(MIL.M_DEFAULT, 0x40)
         MIL.MgraArcFill(MIL.M_DEFAULT, TPar.DstImage, DRAW_CENTER_POSX, DRAW_CENTER_POSY,
                         TPar.Radius / 2, TPar.Radius / 2, 0, 360)

      # Increment iteration count and draw text. 
      TPar.NumberOfIteration += 1
      MIL.MgraColor(MIL.M_DEFAULT, 0xFF)
      MIL.MgraText(MIL.M_DEFAULT, TPar.DstImage, STRING_POS_X, STRING_POS_Y, str(TPar.NumberOfIteration))

      # Update the display. 
      if TPar.DispImage:
         MIL.MbufCopyColor2d(TPar.DstImage,
                             TPar.DispImage,
                             MIL.M_ALL_BANDS, 0, 0,
                             MIL.M_ALL_BANDS,
                             TPar.DispOffsetX,
                             TPar.DispOffsetY,
                             IMAGE_WIDTH,
                             IMAGE_HEIGHT)
         

      # Signal to the top-right thread that the last part of the processing is completed. 
      MIL.MthrControl(TPar.DoneEvent, MIL.M_EVENT_SET, MIL.M_SIGNALED)
      
      
   # Before exiting the thread, make sure that all the commands are executed. 
   MIL.MthrWait(TPar.System, MIL.M_THREAD_WAIT, MIL.M_NULL)
   return 1
   
if __name__ == "__main__":
  MthreadExample()
