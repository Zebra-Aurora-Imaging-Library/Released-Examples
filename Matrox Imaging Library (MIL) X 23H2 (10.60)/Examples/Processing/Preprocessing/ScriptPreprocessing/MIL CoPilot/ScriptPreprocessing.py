#!/usr/bin/python
# -*- coding: utf-8 -*-
##########################################################################
#
# 
#  File name: ScriptPreprocessing.cpp  
#
#  Synopsis:  This example show how to use scripting to do some preprocessing.
#             
#
#  Copyright © Matrox Electronic Systems Ltd., 1992-2023.
#  All Rights Reserved

import ctypes
import mil as MIL
import time

STRUCTURING_ELEMENT_SIZE_X = 1
STRUCTURING_ELEMENT_SIZE_Y = 21
THRESHOLD_VALUE = 30
def PreprocessingFunction(funcId):

   MilFunc = MIL.MIL_ID(funcId)
   SourceImage = MIL.MIL_ID(0)
   DestImage = MIL.MIL_ID(0)
   TheReturnValue = ctypes.c_void_p(1)
   
   MIL.MfuncParamValue(MilFunc, 1, ctypes.byref(SourceImage))
   MIL.MfuncParamValue(MilFunc, 2, ctypes.byref(DestImage))
   MIL.MfuncParamValue(MilFunc, 3, ctypes.byref(TheReturnValue))

   MilSystem = MIL.MbufInquire(SourceImage, MIL.M_OWNER_SYSTEM, None)
   MilStructElem = MIL.MbufAlloc2d(MilSystem, STRUCTURING_ELEMENT_SIZE_X, STRUCTURING_ELEMENT_SIZE_Y, 32, MIL.M_STRUCT_ELEMENT, None)
   MIL.MbufClear(MilStructElem, 0)
   
   MIL.MimBinarize(SourceImage, DestImage, MIL.M_FIXED + MIL.M_GREATER, THRESHOLD_VALUE, 0)
   MIL.MimMorphic(DestImage, DestImage, MilStructElem, MIL.M_OPEN, 3, MIL.M_GRAYSCALE)
   
   MIL.MbufFree(MilStructElem)
   
   TheError = MIL.MappGetError(MIL.M_DEFAULT, MIL.M_GLOBAL)
   
   #Cast TheReturnValue as a pointer to set the error and return it to MIL
   TheReturnValueAsPtr = ctypes.cast(TheReturnValue, ctypes.POINTER(MIL.MIL_INT))   
   TheReturnValueAsPtr[0] = TheError
   return 0