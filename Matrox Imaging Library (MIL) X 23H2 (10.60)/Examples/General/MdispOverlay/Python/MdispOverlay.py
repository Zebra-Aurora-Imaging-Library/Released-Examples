#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MDispOverlay.py
#
# Synopsis:  This program shows how to display an image while creating
#            text and graphics annotations on top of it using MIL
#            graphic functions and windows GDI drawing under Windows.
#            If the target system supports grabbing, the annotations
#            are done on top of a continuous grab.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################
 
import mil as MIL

# Target image.
IMAGE_FILE   = MIL.M_IMAGE_PATH + "Board.mim"

# Title for the display window.
WINDOW_TITLE = "Custom Title"

def MdispOverlayExample():

   # Allocate defaults.
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)
   MilDigitizer = None
   # If the system has a digitizer, use it.
   if MIL.MsysInquire(MilSystem, MIL.M_DIGITIZER_NUM):
      MilDigitizer = MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
      MilImage = MIL.MbufAllocColor(MilSystem,
                                    MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_BAND),
                                    MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X),
                                    MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y),
                                    8 + MIL.M_UNSIGNED,
                                    MIL.M_IMAGE + MIL.M_DISP + MIL.M_PROC + MIL.M_GRAB)
      MIL.MbufClear(MilImage, 0)
   
   else:
      # Restore a static image.
      MilImage = MIL.MbufRestore(IMAGE_FILE, MilSystem)

   # Change display window title.
   MIL.MdispControl(MilDisplay, MIL.M_TITLE, WINDOW_TITLE)

   # Display the image buffer.
   MIL.MdispSelect(MilDisplay, MilImage)

   # Draw text and graphis annotations in the display overlay.
   OverlayDraw(MilDisplay)

   # If the system supports it, grab continuously in the displayed image. 
   if (MilDigitizer):
      MIL.MdigGrabContinuous(MilDigitizer, MilImage)

   # Pause to show the image.
   print("\nOVERLAY ANNOTATIONS:")
   print("--------------------\n")
   print("Displaying an image with overlay annotations.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Stop the continuous grab and free digitizer if needed.
   if (MilDigitizer):
      MIL.MdigHalt(MilDigitizer)
      MIL.MdigFree(MilDigitizer)

      # Pause to show the result. 
      print("Displaying the last grabbed image.")
      print("Press <Enter> to end.\n")
      MIL.MosGetch()

   # Free image.
   MIL.MbufFree(MilImage)   

   # Free default allocations.
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0

# --------------------------------------------------------------- 
# Name:      OverlayDraw
# Synopsis:  This function draws annotations in the display overlay.

def OverlayDraw(MilDisplay):

   # Prepare overlay buffer. #
   ###########################

   # Enable the display of the overlay annotations.
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE)

   # Inquire the overlay buffer associated with the display. 
   MilOverlayImage = MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID)

   # Clear the overlay to transparent. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT)

   # Disable the overlay display update to accelerate annotations. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_SHOW, MIL.M_DISABLE)

   # Inquire overlay size. 
   ImageWidth  = MIL.MbufInquire(MilOverlayImage, MIL.M_SIZE_X)
   ImageHeight = MIL.MbufInquire(MilOverlayImage, MIL.M_SIZE_Y)

   # Draw MIL overlay annotations. #
   #################################

   # Set the graphic text background to transparent. 
   MIL.MgraControl(MIL.M_DEFAULT, MIL.M_BACKGROUND_MODE, MIL.M_TRANSPARENT)

   # Print a white string in the overlay image buffer. 
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_WHITE)
   MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, ImageWidth/9, ImageHeight/5,    " -------------------- ")
   MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, ImageWidth/9, ImageHeight/5+25, " - MIL Overlay Text - ")
   MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, ImageWidth/9, ImageHeight/5+50, " -------------------- ")

   # Print a green string in the overlay image buffer. 
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN)
   MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, ImageWidth*11/18, ImageHeight/5,    " ---------------------")
   MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, ImageWidth*11/18, ImageHeight/5+25, " - MIL Overlay Text - ")
   MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, ImageWidth*11/18, ImageHeight/5+50, " ---------------------")

   # Re-enable the overlay display after all annotations are done. 
   MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_SHOW, MIL.M_ENABLE)

   try:
      import win32ui
      import win32api
      import win32con

      # Draw GDI color overlay annotation #
      #####################################

      # Disable error printing for the inquire might not be supported.
      MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)

      # Create a device context to draw in the overlay buffer with GDI. 
      MIL.MbufControl(MilOverlayImage, MIL.M_DC_ALLOC, MIL.M_DEFAULT)
      # Inquire the device context. 
      hCustomDC = win32ui.CreateDCFromHandle(MIL.MbufInquire(MilOverlayImage, MIL.M_DC_HANDLE))

      # Re-enable error printing. 
      MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)

      # Perform operation if GDI drawing is supported. 
      if hCustomDC:
         Hor = [(0, int(ImageHeight / 2)), (int(ImageWidth), int(ImageHeight / 2))]
         Ver = [(int(ImageWidth / 2), 0), (int(ImageWidth / 2), int(ImageHeight))]

         # Draw a blue cross.
         hpen = win32ui.CreatePen(win32con.PS_SOLID, 1, win32api.RGB(0, 0, 255))
         hpenOld = hCustomDC.SelectObject(hpen)

         hCustomDC.Polyline(Hor)
         hCustomDC.Polyline(Ver)

         hCustomDC.SelectObject(hpenOld)

         # Prepare transparent text annotations. 
         hCustomDC.SetBkMode(win32con.TRANSPARENT)
         chText = "GDI Overlay Text"
         (x, y) = hCustomDC.GetTextExtentPoint(chText)

         # Write red text.
         Txt = (int(ImageWidth*3/18), int(ImageHeight*17/24), int(ImageWidth*3/18) + x, int(ImageHeight*17/24) + y)
         hCustomDC.SetTextColor(win32api.RGB(255, 0, 0))
         hCustomDC.DrawText(chText, Txt, win32con.DT_RIGHT)

         # Write yellow text.
         Txt = (int(ImageWidth*12/18), int(ImageHeight*17/24), int(ImageWidth*12/18) + x, int(ImageHeight*17/24) + y)
         hCustomDC.SetTextColor(win32api.RGB(255, 255, 0))
         hCustomDC.DrawText(chText, Txt, win32con.DT_RIGHT)

         # Delete device context. 
         MIL.MbufControl(MilOverlayImage, MIL.M_DC_FREE, MIL.M_DEFAULT)

         # Signal MIL that the overlay buffer was modified. 
         MIL.MbufControl(MilOverlayImage, MIL.M_MODIFIED, MIL.M_DEFAULT)
   except:
      pass
      # GDI drawing is not supported

if __name__ == "__main__":
   MdispOverlayExample()
