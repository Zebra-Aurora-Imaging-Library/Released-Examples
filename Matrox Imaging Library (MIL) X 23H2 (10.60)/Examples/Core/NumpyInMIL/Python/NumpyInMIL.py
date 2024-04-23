#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# 
#  File name: NumpyInMIL.py  
#
#   Synopsis:  This example shows how to use MIL in an Python project using numpy.
#
#  Copyright © Matrox Electronic Systems Ltd., 1992-2023.
#  All Rights Reserved
##########################################################################

import sys

try:
   import mil as MIL
except:
   print("Import MIL library failure.")
   print("An error occurred while trying to import mil. Please make sure mil is under your python path.\n")
   print("Press <Enter> to end.\n")
   input()
   sys.exit(2)

try:
   #All subsequent calls to MIL that return an array will return a numpy array
   import numpy as np
except:
   print("Import numpy library failure.")
   print("An error occurred while trying to import numpy. Please make sure the numpy package is installed.\n")
   print("Press <Enter> to end.\n")
   input()
   sys.exit(2)

try:
   #Lets plot the line data as a profile
   import sys
   if sys.platform == "linux" :
       import matplotlib
       matplotlib.use('Qt5Agg')
   from matplotlib import pyplot as plt
except:
   print("Import matplotlib library failure.")
   print("An error occurred while trying to import matplotlib. Please make sure the matplotlib package is installed.\n")
   print("Press <Enter> to end.\n")
   input()
   sys.exit(2)


def LineProfile(MilDisplay, GraphicsList, BaboonRGB):
   '''Create a line profile and display it in a chart'''
   print("Create a line profile and display it in a chart.")
   # Dimensions of line profile
   x_start = 135
   y_start = 135
   x_end = 365
   y_end = 315

   # Draw the line in the display
   MIL.MgraControl(MIL.M_DEFAULT, MIL.M_COLOR, MIL.M_COLOR_YELLOW)
   MIL.MgraLine(MIL.M_DEFAULT, GraphicsList, x_start, y_start, x_end, y_end)
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicsList)
   MIL.MdispSelect(MilDisplay, BaboonRGB)

   #Create child bands for each color
   red_band = MIL.MbufChildColor(BaboonRGB, MIL.M_RED)
   nb_pixels, red_line_data = MIL.MbufGetLine(red_band, x_start, y_start, x_end, y_end, MIL.M_DEFAULT)
   MIL.MbufFree(red_band)

   green_band = MIL.MbufChildColor(BaboonRGB, MIL.M_GREEN)
   nb_pixels, green_line_data = MIL.MbufGetLine(green_band, x_start, y_start, x_end, y_end, MIL.M_DEFAULT)
   MIL.MbufFree(green_band)

   blue_band = MIL.MbufChildColor(BaboonRGB, MIL.M_BLUE)
   nb_pixels, blue_line_data = MIL.MbufGetLine(blue_band, x_start, y_start, x_end, y_end, MIL.M_DEFAULT)
   MIL.MbufFree(blue_band)

   # Plot the lines using pyplot
   plt.plot(red_line_data, color='r', label='red band')
   plt.plot(green_line_data, color='g', label='green band')
   plt.plot(blue_line_data, color='b', label='blue band')
   plt.legend()
   plt.xlabel("Pixel Index")
   plt.ylabel("Pixel Value")
   plt.title("Line Profile of {0} Pixels".format(nb_pixels))
   plt.grid(True)
   print("Showing the data obtained from MbufGetLine (Line Profile)")
   print("Close the plot window to continue")
   plt.show()

   #Clean up
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MIL.M_NULL)

def DisplayBuffer(BaboonRGB):
   '''Display a MIL buffer using matplotlib by getting a copy of the buffer into a numpy array'''
   print("Display a MIL buffer using matplotlib by getting a copy of the buffer into a numpy array")

   numpy_array = MIL.MbufGet(BaboonRGB) #Retrieves a planar buffer in (Band, SizeY, SizeX)
   
   #Rearrange the buffer to be packed to support imshow (SizeY, SizeX, Band)
   numpy_array = np.dstack((numpy_array[0], numpy_array[1], numpy_array[2]))
   plt.imshow(numpy_array)
   plt.xlabel("X")
   plt.ylabel("Y")
   plt.title("BaboonRGB {0}".format(numpy_array.shape))
   print("Showing the BaboonRBG buffer using pyplot")
   print("Close the plot window to continue")
   plt.show()

def CreateNumpyArrayFromMILBuffer(MilDisplay, MilSystem):
   '''Create a numpy array using the host address of a MIL buffer'''
   print("Create a numpy array using the host address of a MIL buffer")

   MIL.MsysControl(MilSystem, MIL.M_ALLOCATION_OVERSCAN, MIL.M_DISABLE) # Disable the overscan so we have a zero pitch
   
   # Create a numpy array using a monochrome buffer
   Image = MIL.MbufImport(MIL.M_IMAGE_PATH +"BaboonMono.mim", MIL.M_DEFAULT, MIL.M_RESTORE + MIL.M_NO_GRAB,  MilSystem)
   size_x = MIL.MbufInquire(Image, MIL.M_SIZE_X)
   size_y = MIL.MbufInquire(Image, MIL.M_SIZE_Y)
   pitch = MIL.MbufInquire(Image, MIL.M_PITCH)

   if pitch != size_x:
      print("There is padding on the buffer, which numpy does not support")
      sys.exit(2)

   import ctypes # We use ctypes to get the buffer address and provide it to numpy
   host_address = MIL.MbufInquire(Image, MIL.M_HOST_ADDRESS)
   host_address_ptr = ctypes.cast(host_address, ctypes.POINTER(ctypes.c_ubyte)) # Cast as a c_ubyte to match image format
   
   image_array = np.ctypeslib.as_array(host_address_ptr, (size_y, size_x)) # Create the numpy array from the MIL address
   
   plt.imshow(image_array, cmap='gray') # Show that the array contains the same content
   print("Showing the BaboonMono buffer using pyplot")
   print("Close the plot window to continue")
   plt.show()

   MIL.MdispSelect(MilDisplay, Image) # Show the buffer before modification in MIL
   print("The image before modifying the numpy array")
   print("Press Enter to continue")
   MIL.MosGetch()
   
   for x in range(0, image_array.shape[0]):
      for y in range(0, image_array.shape[1]):
         if image_array[x, y] > 128:
            image_array[x, y] = 128 # Saturate all pixel values to 128

   MIL.MbufControl(Image, MIL.M_MODIFIED, MIL.M_DEFAULT) # Indicate to MIL the buffer has changed to trigger a display update
   print("The image after saturating the pixel values to 128 and signaling to MIL the buffer has been modified")
   print("Press Enter to continue")
   MIL.MosGetch()
   
   MIL.MbufFree(Image) # Once MbufFree is called the numpy array is no longer valid
   del image_array # Manually clear the numpy array to ensure no one tries to access it

   # Create a numpy array using an RGB buffer
   Image = MIL.MbufImport(MIL.M_IMAGE_PATH +"BaboonRGB.mim", MIL.M_DEFAULT, MIL.M_RESTORE + MIL.M_NO_GRAB,  MilSystem)
   size_x = MIL.MbufInquire(Image, MIL.M_SIZE_X)
   size_y = MIL.MbufInquire(Image, MIL.M_SIZE_Y)
   size_band = MIL.MbufInquire(Image, MIL.M_SIZE_BAND)
   data_format = MIL.MbufInquire(Image, MIL.M_DATA_FORMAT)
   pitch = MIL.MbufInquire(Image, MIL.M_PITCH)

   if pitch != size_x:
      print("There is padding on the buffer, which numpy does not support")
      sys.exit(2)

   import ctypes # We use ctypes to get the buffer address and provide it to numpy
   host_address = MIL.MbufInquire(Image, MIL.M_HOST_ADDRESS)
   host_address_ptr = ctypes.cast(host_address, ctypes.POINTER(ctypes.c_ubyte)) # Cast as a c_ubyte to match image format
   
   # Create the correct shape based on packed versus planar buffers
   if data_format & MIL.M_PACKED == MIL.M_PACKED:
      shape = (size_y, size_x, size_band)
   else:
      shape = (size_band, size_y, size_x)
   image_array = np.ctypeslib.as_array(host_address_ptr, shape) # Create the numpy array from the MIL address in planar format

   MIL.MdispSelect(MilDisplay, Image) # Show the buffer before modification in MIL
   print("The image before modifying the numpy array")
   print("Press Enter to continue")
   MIL.MosGetch()
   
   for y in range(0, image_array.shape[0]):
      for x in range(0, image_array.shape[1]):
         if image_array[y, x, 0] > 128:
            image_array[y, x, 0] = 128 # Saturate all pixel values to 128 in the first band

   MIL.MbufControl(Image, MIL.M_MODIFIED, MIL.M_DEFAULT) # Indicate to MIL the buffer has changed to trigger a display update
   print("The image after saturating the first bands pixel values to 128 and signaling to MIL the buffer has been modified")
   print("Press Enter to continue")
   MIL.MosGetch()
   
   MIL.MbufFree(Image) # Once MbufFree is called the numpy array is no longer valid
   del image_array # Manually clear the numpy array to ensure no one tries to access it
   MIL.MsysControl(MilSystem, MIL.M_ALLOCATION_OVERSCAN, MIL.M_DEFAULT)

# MAIN 
def main():
   print("\n[SYNOPSIS]\n")
   print("This example shows how to use MIL in an Python project using numpy.")
   
   MilApplication = MIL.MappAlloc("M_DEFAULT", MIL.M_DEFAULT)
   MilSystem = MIL.MsysAlloc(MilApplication, "M_DEFAULT", MIL.M_DEFAULT, MIL.M_DEFAULT)

      # First we check if the system is local
   if (MIL.MsysInquire(MilSystem, MIL.M_LOCATION) != MIL.M_LOCAL):
      print("\n\nThis example accesses the buffer host address which is not supported on a remote system.")
      print("Please select a local system as the default.")
      print("Press <Enter> to end.\n")
      input()
   else:
      MilDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
      GraphicsList = MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT)
   
      BaboonRGB = MIL.MbufImport(MIL.M_IMAGE_PATH +"BaboonRGB.mim", MIL.M_DEFAULT, MIL.M_RESTORE + MIL.M_NO_GRAB,  MilSystem) #Restore an image
   
      '''Create a line profile and display it in a chart'''
      LineProfile(MilDisplay, GraphicsList, BaboonRGB)

      '''Display a MIL buffer using matplotlib by getting a copy of the buffer into a numpy array'''
      DisplayBuffer(BaboonRGB)

      '''Create a numpy array using the host address of a MIL buffer'''
      CreateNumpyArrayFromMILBuffer(MilDisplay, MilSystem)

      MIL.MbufFree(BaboonRGB)
      MIL.MdispFree(MilDisplay)
      MIL.MgraFree(GraphicsList)

   MIL.MsysFree(MilSystem)
   MIL.MappFree(MilApplication)

if __name__ == "__main__":
    main()
