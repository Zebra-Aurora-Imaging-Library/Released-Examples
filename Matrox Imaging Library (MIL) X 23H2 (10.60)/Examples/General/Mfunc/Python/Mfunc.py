#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MFunc.py 
#
# Synopsis:  This example shows the use of the MIL Function Developer Tool Kit and how 
#            MIL and custom code can be mixed to create a custom MIL function that 
#            accesses the data pointer of a MIL buffer directly in order to process it.
#
#            The example creates a Master MIL function that registers all the parameters 
#            to MIL and calls the Slave function. The Slave function retrieves all the 
#            parameters, gets the pointers to the MIL image buffers, uses them to access 
#            the data directly and adds a constant.
#             
#            Note: The images must be 8-bit unsigned and have a valid Host pointer.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################


import mil as MIL
import ctypes

# MIL function specifications. 
FUNCTION_NB_PARAM             = 4
FUNCTION_OPCODE_ADD_CONSTANT  = 1 
FUNCTION_PARAMETER_ERROR_CODE = 1
FUNCTION_SUPPORTED_IMAGE_TYPE = 8 + MIL.M_UNSIGNED

# Target image file name. 
IMAGE_FILE = MIL.M_IMAGE_PATH + "BoltsNutsWashers.mim"

# Master MIL Function definition. #
# ------------------------------- #
def AddConstant(SrcImageId, DstImageId, ConstantToAdd):
   Func = MIL.MIL_ID(0)
   SlaveReturnValue = MIL.MIL_INT(0)

   SlaveAddConstantPtr = MIL.MIL_FUNC_FUNCTION_PTR(SlaveAddConstant)

   # Allocate a MIL function context that will be used to call a target 
   #  Slave function locally on the Host to do the processing.
   MIL.MfuncAlloc("AddConstant", FUNCTION_NB_PARAM, 
                  SlaveAddConstantPtr, MIL.M_NULL, MIL.M_NULL,
                  MIL.M_USER_MODULE_1 + FUNCTION_OPCODE_ADD_CONSTANT, 
                  MIL.M_LOCAL + MIL.M_SYNCHRONOUS_FUNCTION,
                  ctypes.byref(Func))

   # Register the parameters.
   MIL.MfuncParamMilId(Func.value, 1, SrcImageId, MIL.M_IMAGE, MIL.M_IN)
   MIL.MfuncParamMilId(Func.value, 2, DstImageId, MIL.M_IMAGE, MIL.M_OUT)
   MIL.MfuncParamMilInt(Func.value, 3, ConstantToAdd)
   MIL.MfuncParamDataPointer(Func.value, 4, ctypes.byref(SlaveReturnValue), ctypes.sizeof(MIL.MIL_INT), MIL.M_OUT)

   # Call the target Slave function. 
   MIL.MfuncCall(Func.value)

   # Free the MIL function context. 
   MIL.MfuncFree(Func.value)
   
   # Return the value received from the Slave function. 
   return SlaveReturnValue.value

# MIL Slave function definition. #
# ------------------------------ #
def SlaveAddConstant(Func):
   SrcImageId = MIL.MIL_ID(0)
   DstImageId = MIL.MIL_ID(0)

   ConstantToAdd = MIL.MIL_INT(0)
   TempValue = MIL.MIL_INT(0)

   SlaveReturnValueMemAddr = MIL.MIL_INT(0)

   # Read the parameters. 
   MIL.MfuncParamValue(Func, 1, ctypes.byref(SrcImageId))
   MIL.MfuncParamValue(Func, 2, ctypes.byref(DstImageId))
   MIL.MfuncParamValue(Func, 3, ctypes.byref(ConstantToAdd))
   MIL.MfuncParamValue(Func, 4, ctypes.byref(SlaveReturnValueMemAddr))

   # Lock buffers for direct access.
   MIL.MbufControl(SrcImageId.value, MIL.M_LOCK, MIL.M_DEFAULT)
   MIL.MbufControl(DstImageId.value, MIL.M_LOCK, MIL.M_DEFAULT)

   # Read image information
   SrcImageMemAddr = MIL.MbufInquire(SrcImageId.value, MIL.M_HOST_ADDRESS)
   SrcImageSizeX = MIL.MbufInquire(SrcImageId.value, MIL.M_SIZE_X)
   SrcImageSizeY = MIL.MbufInquire(SrcImageId.value, MIL.M_SIZE_Y)
   SrcImageType = MIL.MbufInquire(SrcImageId.value, MIL.M_TYPE)
   SrcImagePitchByte = MIL.MbufInquire(SrcImageId.value, MIL.M_PITCH_BYTE)
   DstImageMemAddr = MIL.MbufInquire(DstImageId.value, MIL.M_HOST_ADDRESS)
   DstImageSizeX = MIL.MbufInquire(DstImageId.value, MIL.M_SIZE_X)
   DstImageSizeY = MIL.MbufInquire(DstImageId.value, MIL.M_SIZE_Y)
   DstImageType = MIL.MbufInquire(DstImageId.value, MIL.M_TYPE)
   DstImagePitchByte = MIL.MbufInquire(DstImageId.value, MIL.M_PITCH_BYTE)

   # Cast the addresses to a pointer of uint8
   SrcImageDataPtr = ctypes.cast(SrcImageMemAddr, ctypes.POINTER(ctypes.c_uint8))
   DstImageDataPtr = ctypes.cast(DstImageMemAddr, ctypes.POINTER(ctypes.c_uint8))

   # Reduce the destination area to process if necessary. 
   if SrcImageSizeX < DstImageSizeX:
      DstImageSizeX = SrcImageSizeX
   if SrcImageSizeY < DstImageSizeY:
      DstImageSizeY = SrcImageSizeY

   # If images have the proper type and a valid host pointer,
   # execute the operation using custom C code.
   if (SrcImageType == DstImageType and SrcImageType == FUNCTION_SUPPORTED_IMAGE_TYPE 
      and SrcImageDataPtr != MIL.M_NULL and DstImageDataPtr != MIL.M_NULL):
      for y in range(DstImageSizeY):
         for x in range(DstImageSizeX):
            # Calculate the value to write. 
            TempValue = SrcImageDataPtr[x] + ConstantToAdd.value

            # Write the value if no overflow, else saturate. 
            if (TempValue <= 0xff):
               DstImageDataPtr[x] = ctypes.c_uint8(TempValue).value
            else:
               DstImageDataPtr[x] = 0xff

         # Move pointer to the next line taking into account the image's pitch. 
         SrcImageMemAddr += SrcImagePitchByte
         DstImageMemAddr += DstImagePitchByte
         SrcImageDataPtr = ctypes.cast(SrcImageMemAddr, ctypes.POINTER(ctypes.c_uint8))
         DstImageDataPtr = ctypes.cast(DstImageMemAddr, ctypes.POINTER(ctypes.c_uint8))
      
      # Get the variable SlaveReturnValue at the memory address to modify it 
      SlaveReturnValuePtr = ctypes.cast(SlaveReturnValueMemAddr.value, ctypes.POINTER(MIL.MIL_INT))
      
      # Return a null error code to the Master function. 
      SlaveReturnValuePtr.contents.value = MIL.M_NULL
      

   else:
      # Buffer cannot be processed. Report an error. 
      MIL.MfuncErrorReport(Func,MIL.M_FUNC_ERROR + FUNCTION_PARAMETER_ERROR_CODE,
                           "Invalid parameter.",
                           "Image type not supported or invalid target system.",
                           "Image must be 8-bit and have a valid host address.",
                           MIL.M_NULL
                           )

      # Get the variable SlaveReturnValue at the memory address to modify it 
      SlaveReturnValuePtr = ctypes.cast(SlaveReturnValueMemAddr.value, ctypes.POINTER(MIL.MIL_INT))

      # Return an error code to the Master function. 
      SlaveReturnValuePtr.contents.value = MIL.M_FUNC_ERROR + FUNCTION_PARAMETER_ERROR_CODE

   # Unlock buffers.
   MIL.MbufControl(SrcImageId.value, MIL.M_UNLOCK, MIL.M_DEFAULT)
   MIL.MbufControl(DstImageId.value, MIL.M_UNLOCK, MIL.M_DEFAULT)

   # Signal to MIL that the destination buffer was modified. 
   MIL.MbufControl(DstImageId.value, MIL.M_MODIFIED, MIL.M_DEFAULT); 


# Main to test the custom function. #
# --------------------------------- #
def MfuncExample():
   # Allocate default application, system and display. 
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Load source image into a Host memory image buffer.
   MilImage = MIL.MbufAlloc2d(MilSystem, 
                              MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_X, MIL.M_NULL), 
                              MIL.MbufDiskInquire(IMAGE_FILE, MIL.M_SIZE_Y, MIL.M_NULL), 
                              8 + MIL.M_UNSIGNED, 
                              MIL.M_IMAGE + MIL.M_DISP + MIL.M_HOST_MEMORY,
                              )

   MIL.MbufLoad(IMAGE_FILE, MilImage)

   # Display the image.
   MIL.MdispSelect(MilDisplay, MilImage)

   # Pause.
   print("\nMIL FUNCTION DEVELOPER'S TOOLKIT:")
   print("---------------------------------\n")
   print("This example creates a custom MIL function that processes")
   print("an image using its data pointer directly.\n")
   print("Target image was loaded.")
   print("Press a key to continue.\n")
   MIL.MosGetch()

   # Run the custom function only if the target system's memory is local and accessible. 
   if MIL.MsysInquire(MilSystem, MIL.M_LOCATION, MIL.M_NULL) == MIL.M_LOCAL:
      # Process the image directly with the custom MIL function. 
      ReturnValue = AddConstant(MilImage, MilImage, 0x40)

      # Print a conclusion message. 
      if ReturnValue == MIL.M_NULL:
         print("The white level of the image was augmented.")
      else:
         print("An error was returned by the Slave function.")

   else:
      # Print that the example don't run remotely. 
      print("This example doesn't run with Distributed MIL.")

   # Wait for a key to terminate. 
   print("Press a key to terminate.\n")
   MIL.MosGetch()

   # Free all allocations.
   MIL.MbufFree(MilImage)
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)

   return 0 
   
if __name__ == "__main__":
   MfuncExample()
