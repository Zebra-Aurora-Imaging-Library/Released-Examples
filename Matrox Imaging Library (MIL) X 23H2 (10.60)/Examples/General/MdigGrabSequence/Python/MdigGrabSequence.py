#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#########################################################################################
#
# File name: MDigGrabSequence.py
#
# Synopsis:  This example shows how to record a sequence and play it back in 
#            real time. the acquisition can be done in memory only or to 
#            an AVI file. 
#
# NOTE:      This example assumes that the hard disk is sufficiently fast 
#            to keep up with the grab. Also, removing the sequence display or 
#            the text annotation while grabbing will reduce the CPU usage and
#            might help if some frames are missed during acquisition. 
#            If the disk or system are not fast enough, choose the option to 
#            record uncompressed images to memory.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
#########################################################################################

import mil as MIL

# Sequence file name.
SEQUENCE_FILE = MIL.M_TEMP_DIR + "MilSequence.avi"

# Quantization factor to use during the compression.
# Valid values are 1 to 99 (higher to lower quality). 
COMPRESSION_Q_FACTOR = 50

# Annotation flag. Set to M_YES to draw the frame number in the saved image. 
FRAME_NUMBER_ANNOTATION = MIL.M_YES

# Maximum number of images for the multiple buffering grab. 
NB_GRAB_IMAGE_MAX = 20

# User's record function hook data structure. 
class HookDataStruct:
   def __init__(self, MilSystem, MilDisplay, MilImageDisp, MilCompressedImage, SaveSequenceToDisk, NbGrabbedFrames):
      self.MilSystem = MilSystem
      self.MilDisplay = MilDisplay
      self.MilImageDisp = MilImageDisp
      self.MilCompressedImage = MilCompressedImage
      self.NbGrabbedFrames = NbGrabbedFrames
      self.SaveSequenceToDisk = SaveSequenceToDisk

# Main function.
def MdigGrabSequenceExample():
   MilGrabImages = []
   MilCompressedImage = MIL.M_NULL
   TimeWait = 0.0

   # Allocate defaults.
   MilApplication, MilSystem, MilDisplay, MilDigitizer = MIL.MappAllocDefault(MIL.M_DEFAULT, ImageBufIdPtr=MIL.M_NULL)
   
   # Allocate an image and display it. 
   MilImageDisp = MIL.MbufAllocColor(MilSystem,
                                     MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_BAND),
                                     MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X),
                                     MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y),
                                     8 + MIL.M_UNSIGNED,
                                     MIL.M_IMAGE + MIL.M_GRAB + MIL.M_DISP)
   MIL.MbufClear(MilImageDisp, 0)
   MIL.MdispSelect(MilDisplay, MilImageDisp)

   # Grab continuously on display.
   MIL.MdigGrabContinuous(MilDigitizer, MilImageDisp)

   # Print a message
   print("\nSEQUENCE ACQUISITION:")
   print("--------------------\n")

   # Inquire MIL licenses.
   MilRemoteApplication = MIL.MsysInquire(MilSystem, MIL.M_OWNER_APPLICATION)
   LicenseModules = MIL.MappInquire(MilRemoteApplication, MIL.M_LICENSE_MODULES)

   # Ask for the sequence format. 
   print("Choose the sequence format:")
   print("1) Uncompressed images to memory (up to {NB_GRAB_IMAGE_MAX} frames).".format(NB_GRAB_IMAGE_MAX=NB_GRAB_IMAGE_MAX))
   print("2) Uncompressed images to an AVI file.")
   if LicenseModules & (MIL.M_LICENSE_JPEGSTD | MIL.M_LICENSE_JPEG2000):
      if LicenseModules & MIL.M_LICENSE_JPEGSTD:
         print("3) Compressed lossy JPEG images to an AVI file.")
      if LicenseModules & MIL.M_LICENSE_JPEG2000:
         print("4) Compressed lossy JPEG2000 images to an AVI file.")

   ValidSelection = False
   while (not ValidSelection):
      Selection = chr(MIL.MosGetch())
      ValidSelection = True
      # Set the buffer attribute.
      if Selection == "1" or Selection == "\r":
         print("\nUncompressed images to memory selected.")
         CompressAttribute = MIL.M_NULL
         SaveSequenceToDisk = MIL.M_NO

      elif Selection == "2":
         print("\nUncompressed images to file selected.")
         CompressAttribute = MIL.M_NULL
         SaveSequenceToDisk = MIL.M_YES
   
      elif Selection == "3":
         print("\nJPEG images to file selected.")
         CompressAttribute = MIL.M_COMPRESS + MIL.M_JPEG_LOSSY
         SaveSequenceToDisk = MIL.M_YES

      elif Selection == "4":
         print("\nJPEG 2000 images to file selected.")
         CompressAttribute = MIL.M_COMPRESS + MIL.M_JPEG2000_LOSSY
         SaveSequenceToDisk = MIL.M_YES

      else:
         print("\nInvalid selection !.")
         ValidSelection = False
   
   # Allocate a compressed buffer if required.
   if CompressAttribute:
      MilCompressedImage = MIL.MbufAllocColor(MilSystem,
                                              MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_BAND),
                                              MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X),
                                              MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y),
                                              8 + MIL.M_UNSIGNED, 
                                              MIL.M_IMAGE + CompressAttribute)
      MIL.MbufControl(MilCompressedImage, MIL.M_Q_FACTOR, COMPRESSION_Q_FACTOR)

   # Allocate the grab buffers to hold the sequence buffering.
   NbFrames = 0
   for n in range(0, NB_GRAB_IMAGE_MAX):
      if (n == 2):
         MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
      MilGrabImages.append(MIL.MbufAllocColor(MilSystem,
                                              MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_BAND),
                                              MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X),
                                              MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y),
                                              8 + MIL.M_UNSIGNED, 
                                              MIL.M_IMAGE + MIL.M_GRAB))
      if MilGrabImages[n]:
         NbFrames += 1
         MIL.MbufClear(MilGrabImages[n], 0xFF)
      else:
         break
   
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)

   # Halt continuous grab.
   MIL.MdigHalt(MilDigitizer)

   # If the sequence must be saved to disk.
   if SaveSequenceToDisk:
      # Open the AVI file if required. 
      print("\nSaving the sequence to an AVI file...")
      MIL.MbufExportSequence(SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_OPEN)
   else:
      print("\nSaving the sequence to memory...\n")
   
   # Initialize User's archiving function hook data structure. 
   UserHookData = HookDataStruct(MilSystem, MilDisplay, MilImageDisp, MilCompressedImage, SaveSequenceToDisk, 0)

   # Acquire the sequence. The processing hook function will
   # be called for each image grabbed to record and display it. 
   # If sequence is not saved to disk, stop after NbFrames.
   RecordFunctionPtr = MIL.MIL_DIG_HOOK_FUNCTION_PTR(RecordFunction) 
   MIL.MdigProcess(MilDigitizer, MilGrabImages, NbFrames, MIL.M_START if SaveSequenceToDisk else MIL.M_SEQUENCE, MIL.M_DEFAULT, RecordFunctionPtr, UserHookData)

   # Wait for a key press.
   if SaveSequenceToDisk:
      print("\nPress <Enter> to stop recording.\n")
      MIL.MosGetch()

   # Wait until we have at least 2 frames to avoid an invalid frame rate. 
   while True:
      FrameCount = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_COUNT)
      
      if FrameCount >= 2:
         break
   
   # Stop the sequence acquisition. 
   MIL.MdigProcess(MilDigitizer, MilGrabImages, NbFrames, MIL.M_STOP, MIL.M_DEFAULT, RecordFunctionPtr, UserHookData)
   
   # Read and print final statistics. 
   FrameCount = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_COUNT)
   FrameRate = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_RATE)
   FrameMissed = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_MISSED)
   print("\n\n{NbFrames} frames recorded ({FrameMissed} missed), at {FrameRate} frames/sec ({FrameRateMS} ms/frame).\n"
         .format(NbFrames=UserHookData.NbGrabbedFrames, FrameMissed=FrameMissed, FrameRate=round(FrameRate, 1), FrameRateMS=round(1000.0/FrameRate, 1)))

   # Sequence file closing if required.
   if SaveSequenceToDisk:
      MIL.MbufExportSequence(SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, FrameRate, MIL.M_CLOSE)

   # Wait for a key to playback. 
   print("Press <Enter> to start the sequence playback.")
   MIL.MosGetch()

   # Playback the sequence until a key is pressed.
   if UserHookData.NbGrabbedFrames > 0:
      KeyPressed = 0

      while True:
         # If sequence must be loaded. 
         if SaveSequenceToDisk:
            # Inquire information about the sequence. 
            print("\nPlaying sequence from the AVI file...")
            print("Press <Enter> to end playback.\n")
            FrameCount = MIL.MbufDiskInquire(SEQUENCE_FILE, MIL.M_NUMBER_OF_IMAGES)
            FrameRate = MIL.MbufDiskInquire(SEQUENCE_FILE, MIL.M_FRAME_RATE)
            CompressAttribute = MIL.MbufDiskInquire(SEQUENCE_FILE, MIL.M_COMPRESSION_TYPE)

            # Open the sequence file.
            MIL.MbufImportSequence(SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_OPEN)
         else:
            print("\nPlaying sequence from memory...\n")

         # Copy the images to the screen respecting the sequence frame rate.
         TotalReplay = 0.0
         NbFramesReplayed = 0
         for n in range(0, FrameCount):
            # Reset the time.
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET, MIL.M_NULL)

            # If image was saved to disk.
            if SaveSequenceToDisk:
               # Load image directly to the display.
               MIL.MbufImportSequence(SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_LOAD, MIL.M_NULL, MilImageDisp, n, 1, MIL.M_READ)
            else:
               # Copy the grabbed image to the display.
               MIL.MbufCopy(MilGrabImages[n], MilImageDisp)
            
            NbFramesReplayed += 1
            print("Frame #{NbFramesReplayed}             \r".format(NbFramesReplayed=NbFramesReplayed), end='')

            # Check for a pressed key to exit.
            if MIL.MosKbhit() and (n >= (NB_GRAB_IMAGE_MAX - 1)):
               MIL.MosGetch()
               break

            # Wait to have a proper frame rate.
            TimeWait = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ)
            TotalReplay += TimeWait
            TimeWait = (1/FrameRate) - TimeWait
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_WAIT, TimeWait)
            TotalReplay += TimeWait if (TimeWait > 0.0) else 0.0
         
         # Close the sequence file.
         if SaveSequenceToDisk:
            MIL.MbufImportSequence(SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_CLOSE)

         # Print statistics.
         print("\n\n{NbFramesReplayed} frames replayed, at a frame rate of {FrameRate} frames/sec ({FrameRateMS} ms/frame).\n"
               .format(NbFramesReplayed=NbFramesReplayed, FrameRate=round(n/TotalReplay, 1), FrameRateMS=round(1000.0*TotalReplay/n, 1)))
         print("Press <Enter> to end (or any other key to playback again).")
         
         KeyPressed = chr(MIL.MosGetch())

         if KeyPressed == '\r' or KeyPressed == '\n':
            break
   
   # Free all allocated buffers.
   MIL.MbufFree(MilImageDisp)
   for n in range(0, NbFrames):
      MIL.MbufFree(MilGrabImages[n])
   if MilCompressedImage:
      MIL.MbufFree(MilCompressedImage)
   
   # Free defaults.
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MIL.M_NULL)

   return 0

# User's record function called each time a new buffer is grabbed. #
# ---------------------------------------------------------------- #

# Local defines for the annotations.
STRING_LENGTH_MAX = 20
STRING_POS_X      = 20
STRING_POS_Y      = 20

def RecordFunction(HookType, HookId, HookDataPtr):
   UserHookDataPtr = HookDataPtr
   ModifiedImage = 0
   n = 0
   
   # Retrieve the MIL_ID of the grabbed buffer.
   ModifiedImage = MIL.MdigGetHookInfo(HookId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID)

   # Increment the frame count.
   UserHookDataPtr.NbGrabbedFrames += 1
   print("Frame #{FrameNumber}               \r".format(FrameNumber=UserHookDataPtr.NbGrabbedFrames), end='')

   # Draw the frame count in the image if enabled.
   if FRAME_NUMBER_ANNOTATION == MIL.M_YES:
      MIL.MgraText(MIL.M_DEFAULT, ModifiedImage, STRING_POS_X, STRING_POS_Y, str(UserHookDataPtr.NbGrabbedFrames))

   # Copy the new grabbed image to the display.
   MIL.MbufCopy(ModifiedImage, UserHookDataPtr.MilImageDisp)
   
   # Compress the new image if required. 
   if UserHookDataPtr.MilCompressedImage != MIL.M_NULL:
      MIL.MbufCopy(ModifiedImage, UserHookDataPtr.MilCompressedImage)
   
   # Record the new image. 
   ImageToExport = UserHookDataPtr.MilCompressedImage if UserHookDataPtr.MilCompressedImage != MIL.M_NULL else ModifiedImage
   if UserHookDataPtr.SaveSequenceToDisk:
      MIL.MbufExportSequence(SEQUENCE_FILE, MIL.M_DEFAULT, [ImageToExport],
                             1, MIL.M_DEFAULT, MIL.M_WRITE)
   
   return 0
      

if __name__ == "__main__":
   MdigGrabSequenceExample()
