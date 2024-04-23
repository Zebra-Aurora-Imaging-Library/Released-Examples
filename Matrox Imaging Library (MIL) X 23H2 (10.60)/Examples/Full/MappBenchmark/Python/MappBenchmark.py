#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MappBenchmark.py
#
# Synopsis:  This program uses the MappTimer function to demonstrate the 
#            benchmarking of MIL functions. It can be used as a template 
#            to benchmark any MIL or custom processing function accurately.
#
#            It takes into account different factors that can influence 
#            the timing such as dll load time and OS inaccuracy when 
#            measuring a very short time.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################
 
import mil as MIL

# Target MIL image specifications. 
IMAGE_FILE  = MIL.M_IMAGE_PATH + "LargeWafer.mim"
ROTATE_ANGLE = -15

# Timing loop iterations setting. 
MINIMUM_BENCHMARK_TIME = 2.0 # In seconds (1.0 and more recommended). 
ESTIMATION_NB_LOOP     =  10
DEFAULT_NB_LOOP        = 100

# Processing function parameters structure. 
class PROC_PARAM:
   def __init__(self):
      self.MilSourceImage      = None   # Image buffer identifier. 
      self.MilDestinationImage = None   # Image buffer identifier. 

def MappBenchmarkExample():
   ProcessingParam = PROC_PARAM()

   # Allocate defaults. 
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Get the system's owner application.
   MilSystemOwnerApplication = MIL.MsysInquire(MilSystem, MIL.M_OWNER_APPLICATION)

   # Get the system's current thread identifier. 
   MilSystemCurrentThreadId = MIL.MsysInquire(MilSystem, MIL.M_CURRENT_THREAD_ID)

   # Restore image into an automatically allocated image buffer. 
   MilDisplayImage = MIL.MbufRestore(IMAGE_FILE, MilSystem)

   # Display the image buffer. 
   MIL.MdispSelect(MilDisplay, MilDisplayImage)

   # Allocate the processing objects. 
   ProcessingInit(MilSystem, ProcessingParam)

   # Pause to show the original image. 
   print("\nPROCESSING FUNCTION BENCHMARKING:")
   print("---------------------------------\n")
   print("This program times a processing function under different conditions.")
   print("Press <Enter> to start.\n")
   MIL.MosGetch()
   print("PROCESSING TIME FOR {SizeX}x{SizeY}:".format(SizeX=MIL.MbufInquire(ProcessingParam.MilDestinationImage, MIL.M_SIZE_X), SizeY=MIL.MbufInquire(ProcessingParam.MilDestinationImage, MIL.M_SIZE_Y)))
   print("------------------------------\n")

   # Benchmark the processing function without multi-processing. 
   # ------------------------------------------------------------
   MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_MP_USE, MIL.M_DEFAULT, MIL.M_DISABLE, MIL.M_NULL)   
   TimeOneCore, FPSOneCore = Benchmark(ProcessingParam)

   # Show the resulting image and the timing results. 
   MIL.MbufCopy(ProcessingParam.MilDestinationImage, MilDisplayImage)
   print("Without multi-processing (  1 CPU core ): {TimeOneCore:.3f} ms ( {FPSOneCore:.1f} fps)\n".format(TimeOneCore=TimeOneCore, FPSOneCore=FPSOneCore))
   MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_MP_USE, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_NULL)   

   # Enable all performance level.
   MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_MP_USE_PERFORMANCE_LEVEL, MIL.M_ALL, MIL.M_ENABLE, MIL.M_NULL)

   # Inquire the number of performance level. This will return 1 on non-hybrid processor.
   NbPerformanceLevel   = MIL.MappInquireMp(MilSystemOwnerApplication, MIL.M_MP_NB_PERFORMANCE_LEVEL, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_NULL);

   for CurrentMaxPerfLevel in range(NbPerformanceLevel, 0, -1):

      if CurrentMaxPerfLevel > 1:
         print("Benchmark result with core performance level 1 to {CurrentMaxPerfLevel}.\n".format(CurrentMaxPerfLevel=CurrentMaxPerfLevel))
      else:
         print("Benchmark result with core performance level 1.\n");

      # Benchmark the processing function with multi-processing. 
      # ---------------------------------------------------------
      MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_MP_USE, MIL.M_DEFAULT, MIL.M_ENABLE, MIL.M_NULL)   
      MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_CORE_SHARING, MIL.M_DEFAULT, MIL.M_ENABLE, MIL.M_NULL) 
      NbCoresUsed = MIL.MthrInquireMp(MilSystemCurrentThreadId, MIL.M_CORE_NUM_EFFECTIVE, MIL.M_DEFAULT, MIL.M_DEFAULT)
      if NbCoresUsed > 1:
         TimeAllCores, FPSAllCores = Benchmark(ProcessingParam)

         # Show the resulting image and the timing results. 
         MIL.MbufCopy(ProcessingParam.MilDestinationImage, MilDisplayImage)
   
         print("Using multi-processing   (  {NbCoresUsed} CPU cores): {TimeAllCores:.3f} ms ( {FPSAllCores:.1f} fps)".format(NbCoresUsed=NbCoresUsed, TimeAllCores=TimeAllCores, FPSAllCores=FPSAllCores))
      MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_MP_USE, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_NULL)   
      MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_CORE_SHARING, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_NULL) 

      # Benchmark the processing function with multi-processing but no hyper-threading. 
      # --------------------------------------------------------------------------------

      # If Hyper-threading is available on the PC, test if the performance is bette
      #   with it disabled. Sometimes, too many cores sharing the same CPU resources that
      #   are already fully occupied can reduce the overall performance.
   
      MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_MP_USE, MIL.M_DEFAULT, MIL.M_ENABLE, MIL.M_NULL)   
      MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_CORE_SHARING, MIL.M_DEFAULT, MIL.M_DISABLE, MIL.M_NULL) 
      NbCoresUsedNoCS = MIL.MthrInquireMp(MilSystemCurrentThreadId, MIL.M_CORE_NUM_EFFECTIVE, MIL.M_DEFAULT, MIL.M_DEFAULT)
      if NbCoresUsedNoCS != NbCoresUsed:
         TimeAllCoresNoCS, FPSAllCoresNoCS = Benchmark(ProcessingParam)

         # Show the resulting image and the timing results. 
         MIL.MbufCopy(ProcessingParam.MilDestinationImage, MilDisplayImage)
         print("Using multi-processing   (  {NbCoresUsedNoCS} CPU cores): {TimeAllCoresNoCS:.3f} ms ( {FPSAllCoresNoCS:.1f} fps), no Hyper-Thread."
               .format(NbCoresUsedNoCS=NbCoresUsedNoCS,TimeAllCoresNoCS=TimeAllCoresNoCS,FPSAllCoresNoCS=FPSAllCoresNoCS)), 
      MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_MP_USE, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_NULL)   
      MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_CORE_SHARING, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_NULL)  

      # Disable the last performance level.
      MIL.MappControlMp(MilSystemOwnerApplication, MIL.M_MP_USE_PERFORMANCE_LEVEL, CurrentMaxPerfLevel, MIL.M_DISABLE, MIL.M_NULL);

      # Show a comparative summary of the timing results. 
      if NbCoresUsed > 1:
         print("Benchmark is {Time:.1f} times faster with multi-processing.".format(Time=TimeOneCore/TimeAllCores)) 
      
      if NbCoresUsedNoCS != NbCoresUsed:
         print("Benchmark is {Time:.1f} times faster with multi-processing and no Hyper-Thread.\n".format(Time=TimeOneCore/TimeAllCoresNoCS))

   # Wait for a key press. 
   print("Press <Enter> to end.")
   MIL.MosGetch()

   # Free all allocations. 
   ProcessingFree(ProcessingParam)
   MIL.MdispSelect(MilDisplay, MIL.M_NULL)
   MIL.MbufFree(MilDisplayImage)
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)   
   return 0

#****************************************************************************
# Benchmark function. 
#****************************************************************************
def Benchmark(ProcParamPtr):
   EstimatedNbLoop = DEFAULT_NB_LOOP
   MinTime = 0

   # Wait for the completion of all functions in this thread. 
   MIL.MthrWait(MIL.M_DEFAULT, MIL.M_THREAD_WAIT, MIL.M_NULL)

   # Call the processing once before benchmarking for a more accurate time.
   # This compensates for Dll load time, etc.
   
   StartTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ)
   ProcessingExecute(ProcParamPtr)  
   MIL.MthrWait(MIL.M_DEFAULT, MIL.M_THREAD_WAIT, MIL.M_NULL)
   EndTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ)

   MinTime = EndTime - StartTime

   # Estimate the number of loops required to benchmark the processing for 
   # the specified minimum time.
   
   for n in range(ESTIMATION_NB_LOOP):
      StartTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ)
      ProcessingExecute(ProcParamPtr)
      MIL.MthrWait(MIL.M_DEFAULT, MIL.M_THREAD_WAIT, MIL.M_NULL)
      EndTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ)

      Time = EndTime-StartTime
      MinTime = Time if Time < MinTime else MinTime
   
   if MinTime > 0:
      EstimatedNbLoop = int(MINIMUM_BENCHMARK_TIME / MinTime) + 1

   # Benchmark the processing according to the estimated number of loops. 
   StartTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ)
   for n in range(EstimatedNbLoop):
      ProcessingExecute(ProcParamPtr)

   MIL.MthrWait(MIL.M_DEFAULT, MIL.M_THREAD_WAIT, MIL.M_NULL)
   EndTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ)

   Time = EndTime-StartTime

   FramesPerSecond = EstimatedNbLoop/Time
   Time = Time*1000/EstimatedNbLoop

   return Time, FramesPerSecond

#****************************************************************************
# Processing initialization function.
# ****************************************************************************
def ProcessingInit(MilSystem, ProcParamPtr):
   # Allocate a MIL source buffer. 
   ProcParamPtr.MilSourceImage = MIL.MbufAllocColor(MilSystem, 
                                 MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_BAND),
                                 MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_X),
                                 MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_Y),
                                 MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_BIT) + MIL.M_UNSIGNED,
                                 MIL.M_IMAGE + MIL.M_PROC)

   # Load the image into the source image.  
   MIL.MbufLoad(IMAGE_FILE, ProcParamPtr.MilSourceImage)

   # Allocate a MIL destination buffer. 
   ProcParamPtr.MilDestinationImage = MIL.MbufAllocColor(MilSystem, 
                                                         MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_BAND),
                                                         MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_X),
                                                         MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_Y),
                                                         MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_BIT) + MIL.M_UNSIGNED,
                                                         MIL.M_IMAGE + MIL.M_PROC)

#****************************************************************************
# Processing execution function.
# ***************************************************************************
def ProcessingExecute(ProcParamPtr):
   MIL.MimRotate(ProcParamPtr.MilSourceImage, ProcParamPtr.MilDestinationImage, ROTATE_ANGLE,
             MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_BILINEAR + MIL.M_OVERSCAN_CLEAR)

#****************************************************************************
# Processing free function.
# ***************************************************************************
def ProcessingFree(ProcParamPtr):
   # Free all processing allocations.  
   MIL.MbufFree(ProcParamPtr.MilSourceImage)
   MIL.MbufFree(ProcParamPtr.MilDestinationImage)

if __name__ == "__main__":
   MappBenchmarkExample()
