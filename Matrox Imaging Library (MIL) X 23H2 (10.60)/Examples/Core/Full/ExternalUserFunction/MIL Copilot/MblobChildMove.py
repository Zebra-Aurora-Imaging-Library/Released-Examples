#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import time
import mil as MIL
import ctypes

# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
# This is the header for function MoveChild:

# MIL: Start of function prototype for MoveChild
def MoveChild(Func):
   BlobResult = MIL.MIL_ID(0)
   BlobIndex = ctypes.c_longlong(0)
   ChildBuffer = MIL.MIL_ID(0)

   # Read the parameters.
   MIL.MfuncParamValue(Func, 1, ctypes.byref(BlobResult))
   MIL.MfuncParamValue(Func, 2, ctypes.byref(BlobIndex))
   MIL.MfuncParamValue(Func, 3, ctypes.byref(ChildBuffer))
   # MIL: End of function prototype for MoveChild

   BoxXMin = MIL.MblobGetResult(BlobResult, MIL.M_BLOB_INDEX(BlobIndex.value), MIL.M_BOX_X_MIN)
   BoxXMax = MIL.MblobGetResult(BlobResult, MIL.M_BLOB_INDEX(BlobIndex.value), MIL.M_BOX_X_MAX)
   BoxYMin = MIL.MblobGetResult(BlobResult, MIL.M_BLOB_INDEX(BlobIndex.value), MIL.M_BOX_Y_MIN)
   BoxYMax = MIL.MblobGetResult(BlobResult, MIL.M_BLOB_INDEX(BlobIndex.value), MIL.M_BOX_Y_MAX)

   padding = 25
   sizeX = int(BoxXMax - BoxXMin) + padding
   sizeY = int(BoxYMax - BoxYMin) + padding
   BoxXMin = BoxXMin - int(padding/2)
   BoxYMin = BoxYMin - int(padding/2)
   
   MIL.MbufChildMove(ChildBuffer, int(BoxXMin), int(BoxYMin), sizeX, sizeY, MIL.M_DEFAULT)

   # This is the footer for function MoveChild:
   # MIL: Start of out parameter handling for MoveChild
   # MIL: End of out parameter handling for MoveChild

   return

# This is the main footer (Probably empty):

if __name__ == "__main__":
   MainFunction()
