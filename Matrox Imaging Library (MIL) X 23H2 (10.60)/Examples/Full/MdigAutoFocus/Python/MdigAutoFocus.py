#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MdigAutoFocus.py
#
# Synopsis:  This program performs an autofocus operation using the 
#            MdigFocus() function. Since the way to move a motorized
#            camera lens is device-specific, we will not include real
#            lens movement control and image grab but will simulate 
#            the lens focus with a smooth operation. 
#
#     Note : Under MIL-Lite, the out of focus lens simulation is not supported.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################

 
import mil as MIL

# Source MIL image file specification. 
IMAGE_FILE                    = MIL.M_IMAGE_PATH + "BaboonMono.mim"

# Lens mechanical characteristics.
FOCUS_MAX_NB_POSITIONS        = 100
FOCUS_MIN_POSITION            = 0
FOCUS_MAX_POSITION            = (FOCUS_MAX_NB_POSITIONS - 1)
FOCUS_START_POSITION          = 10

# Autofocus search properties.
FOCUS_MAX_POSITION_VARIATION  = MIL.M_DEFAULT
FOCUS_MODE                    = MIL.M_SMART_SCAN
FOCUS_SENSITIVITY             = 1

# User Data structure definition.
class DigHookUserData:
   def __init__(self):
      self.SourceImage = None
      self.FocusImage = None
      self.Display = None
      self.Iteration = None


#*****************************************************************************#
#  Main application function.                                                 #
def MdigAutoFocusExample():
   UserData = DigHookUserData()

   # Allocate defaults. 
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT,  DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Load the source image. 
   MilSource = MIL.MbufRestore(IMAGE_FILE, MilSystem)
   MilCameraFocus = MIL.MbufRestore(IMAGE_FILE, MilSystem)
   MIL.MbufClear(MilCameraFocus, 0)

   # Select image on the display. 
   MIL.MdispSelect(MilDisplay, MilCameraFocus)

   # Simulate the first image grab. 
   SimulateGrabFromCamera(MilSource, MilCameraFocus, FOCUS_START_POSITION, MilDisplay)

   # Initialize user data needed within the hook function. 
   UserData.SourceImage = MilSource
   UserData.FocusImage  = MilCameraFocus
   UserData.Iteration   = 0
   UserData.Display     = MilDisplay

   # Pause to show the original image. 
   print("\nAUTOFOCUS:")
   print("----------\n")
   print("Automatic focusing operation will be done on this image.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()
   print("Autofocusing...\n")
  
   # Perform Autofocus.
   # Since lens movement is hardware specific, no digitizer is used here.
   # We simulate the lens movement with by smoothing the image data in 
   # the hook function instead.
   MoveLensHookFunctionPtr = MIL.MIL_DIG_HOOK_FUNCTION_PTR(MoveLensHookFunction)
   FocusPos = MIL.MdigFocus(MIL.M_NULL,
                            MilCameraFocus,
                            MIL.M_DEFAULT,
                            MoveLensHookFunctionPtr,
                            UserData,
                            FOCUS_MIN_POSITION,
                            FOCUS_START_POSITION,
                            FOCUS_MAX_POSITION,
                            FOCUS_MAX_POSITION_VARIATION,
                            FOCUS_MODE + FOCUS_SENSITIVITY) 

   # Print the best focus position and number of iterations. 
   print("The best focus position is {FocusPos}.".format(FocusPos=FocusPos))
   print("The best focus position found in {Iteration} iterations.\n".format(Iteration=UserData.Iteration))
   print("Press <Enter> to end.")
   MIL.MosGetch()

  # Free all allocations. 
   MIL.MbufFree(MilSource)
   MIL.MbufFree(MilCameraFocus)
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0


#********************************************************************************
# Autofocus hook function responsible to move the lens.                        
def MoveLensHookFunction(HookType,
                         Position,
                         UserDataHookPtr):
   UserData = UserDataHookPtr

   # Here, the lens position must be changed according to the Position parameter.
   #   In that case, we simulate the lens position change followed by a grab.

   if HookType == MIL.M_CHANGE or HookType == MIL.M_ON_FOCUS:
      SimulateGrabFromCamera( UserData.SourceImage, 
                              UserData.FocusImage, 
                              Position,
                              UserData.Display 
                              )
      UserData.Iteration += 1

   return 0

#*********************************************************************************
# Utility function to simulate a grab from a camera at different lens position    
# by smoothing the original image. It should be replaced with a true camera grab. 
#                                                                                 
# Note that this lens simulation will not work under MIL-lite because it uses     
# MimConvolve().                                                                  

# Lens simulation characteristics. 
FOCUS_BEST_POSITION            = (FOCUS_MAX_NB_POSITIONS/2)

def SimulateGrabFromCamera(SourceImage,FocusImage, 
                           Iteration,  AnnotationDisplay):
   # Throw an error under MIL-lite since lens simulation cannot be used. 
   if  MIL.M_MIL_LITE:
      raise Exception("Replace the SimulateGrabFromCamera()function with a true image grab.")

   # Compute number of smooths needed to simulate focus. 
   NbSmoothNeeded = int(abs(Iteration - FOCUS_BEST_POSITION))

   # Buffer inquires. 
   BufType        = MIL.MbufInquire(FocusImage, MIL.M_TYPE)
   BufSizeX       = MIL.MbufInquire(FocusImage, MIL.M_SIZE_X)
   BufSizeY       = MIL.MbufInquire(FocusImage, MIL.M_SIZE_Y)

   if(NbSmoothNeeded == 0):
      # Directly copy image source to destination. 
      MIL.MbufCopy(SourceImage, FocusImage)

   elif(NbSmoothNeeded == 1):
      # Directly convolve image from source to destination. 
      MIL.MimConvolve(SourceImage, FocusImage, MIL.M_SMOOTH)
   
   else:
      SourceOwnerSystem = MIL.MbufInquire(SourceImage, MIL.M_OWNER_SYSTEM)

      # Allocate temporary buffer. */ 
      TempBuffer = MIL.MbufAlloc2d(SourceOwnerSystem, BufSizeX, BufSizeY, 
                  BufType, MIL.M_IMAGE + MIL.M_PROC)
   
      # Perform first smooth. 
      MIL.MimConvolve(SourceImage, TempBuffer, MIL.M_SMOOTH)
         
      # Perform smooths. 
      for i in range(1, NbSmoothNeeded-1):
         MIL.MimConvolve(TempBuffer, TempBuffer, MIL.M_SMOOTH)

      # Perform last smooth. 
      MIL.MimConvolve(TempBuffer, FocusImage, MIL.M_SMOOTH)

      # Free temporary buffer. 
      MIL.MbufFree(TempBuffer)
      
   # Draw position cursor. 
   DrawCursor(AnnotationDisplay, Iteration)


#**************************************************************
# Draw position of the focus lens.                             

# Cursor specifications. 
CURSOR_SIZE                  =  14
CURSOR_COLOR                 =  MIL.M_COLOR_GREEN

def DrawCursor(AnnotationDisplay, Position):
   # Prepare for overlay annotations. 
   MIL.MdispControl(AnnotationDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)
   MIL.MdispControl(AnnotationDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)
   AnnotationImage = MIL.MdispInquire(AnnotationDisplay, MIL.M_OVERLAY_ID)
   BufSizeX = MIL.MbufInquire(AnnotationImage, MIL.M_SIZE_X)
   BufSizeY = MIL.MbufInquire(AnnotationImage, MIL.M_SIZE_Y)
   CURSOR_POSITION              =  ((BufSizeY*7)/8)
   CursorColor = CURSOR_COLOR
   MIL.MgraColor(MIL.M_DEFAULT, CursorColor)

   # Write annotations. 
   n = (BufSizeX / FOCUS_MAX_NB_POSITIONS)
   MIL.MgraLine(MIL.M_DEFAULT, AnnotationImage, 0.0, CURSOR_POSITION + CURSOR_SIZE, 
                                        BufSizeX - 1, CURSOR_POSITION + CURSOR_SIZE)
   MIL.MgraLine(MIL.M_DEFAULT, AnnotationImage, Position * n, CURSOR_POSITION + CURSOR_SIZE,
                                        Position * n - CURSOR_SIZE, CURSOR_POSITION)
   MIL.MgraLine(MIL.M_DEFAULT, AnnotationImage, Position * n, CURSOR_POSITION + CURSOR_SIZE, 
                                        Position * n + CURSOR_SIZE, CURSOR_POSITION)
   MIL.MgraLine(MIL.M_DEFAULT, AnnotationImage, Position * n - CURSOR_SIZE, CURSOR_POSITION, 
                                        Position * n + CURSOR_SIZE, CURSOR_POSITION)

   

if __name__ == "__main__":
   MdigAutoFocusExample()
