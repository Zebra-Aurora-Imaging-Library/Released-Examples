#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#***********************************************************************
#
# File name: enumfeatures.py
#
# Synopsis:  This example shows how to use MIL in order to enumerate
#            all the features in your GenICam(c) compliant device.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
#

import sys
import mil as MIL 

# Print everything that is Guru level or less. 
VISIBILITY_LEVEL = MIL.M_FEATURE_VISIBILITY_GURU

# List containing the feature names.
FeatureList = []

# Verifies if this example can run on the selected system. 
def SystemSupportsGenICam(MilSystem):

   GenICamSupport = MIL.MsysInquire(MilSystem, MIL.M_GENICAM_AVAILABLE)
   if GenICamSupport == MIL.M_NO:
      print("This example program can only be used with the Matrox Driver for GenICam.")
      print("Please ensure that the default system type is set accordingly in MIL Config.")
      print("-------------------------------------------------------------\n\n")
      print("Press <enter> to quit.")
      MIL.MosGetch()

   return GenICamSupport != MIL.M_NO

# Main function. 
def EnumProcessExample():

   # Allocate defaults. 
   MilApplication = MIL.MappAlloc("M_DEFAULT", MIL.M_DEFAULT)
   MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MilDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
   MilDigitizer = MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
   
   if not SystemSupportsGenICam(MilSystem):  
      MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MIL.M_NULL)
      return 1
      
   print("This example shows how to query various feature properties.\n")
   print("Finally the example concludes with how to enumerate all the features")
   print("present in your GenICam compliant device.\n")

   # If we have a Camera-Link frame grabber, we must enable CLProtocol.
   if MIL.MsysInquire(MilSystem, MIL.M_BOARD_TYPE, MIL.M_NULL) & MIL.M_CL:    
      print("When using a Camera-Link frame grabber, make sure you are using")
      print("a GenICam compliant camera and the camera vendor supplied a CLProtocol dll.\n")
      print("The CLProtocol device must be selected in MILConfig->Boards->Camera Link.\n\n")

      # Enable CLProtocol.
      MIL.MdigControl(MilDigitizer, MIL.M_GC_CLPROTOCOL_DEVICE_ID, "M_DEFAULT")
      MIL.MdigControl(MilDigitizer, MIL.M_GC_CLPROTOCOL, MIL.M_ENABLE)
      
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   print("Getting feature properties and value for the \"PixelFormat\" feature.\n")
   # Retrieve feature info related to the PixelFormat feature. 
   GetFeatureProperties(MilDigitizer, "PixelFormat")

   print("\n\nPress <Enter> to continue and enumerate all GenICam features.\n\n")
   MIL.MosGetch()
   
   # Enumerate all features under the root node. 
   EnumerateGenICamNodeTree(MilDigitizer, "Root", 0)

   print("\nFinished enumeration.\n")
   print("Note: due to console width constraints, some strings printed might have been\nclipped.\n")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   print("Starting the MIL Feature Browser... Please wait.")
   print("Press <Enter> to quit.\n")
   MIL.MdigControl(MilDigitizer, MIL.M_GC_FEATURE_BROWSER, MIL.M_OPEN+MIL.M_ASYNCHRONOUS)
   MIL.MosGetch()
   
   # Free all allocations.
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MIL.M_NULL)

   return 0
 

# Routine used to retrieve various info related to a feature. 
def GetFeatureProperties(MilDigitizer, FeatureName):
    
   # Inquire the feature's name.
   Str = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_NAME, FeatureName, MIL.M_TYPE_STRING+MIL.M_FEATURE_USER_ARRAY_SIZE(0))
   print("FeatureName:       ", Str)

   # Inquire the feature's display name string.
   Str = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_DISPLAY_NAME, FeatureName, MIL.M_TYPE_STRING)
   print("FeatureDisplayName:", Str)

   # Inquire the feature's tool tip string.
   Str = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_TOOLTIP, FeatureName, MIL.M_TYPE_STRING+MIL.M_FEATURE_USER_ARRAY_SIZE(0))
   print("FeatureTooltip:    ", Str)

   # Inquire the feature's description string. 
   Str = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_DESCRIPTION, FeatureName, MIL.M_TYPE_STRING+MIL.M_FEATURE_USER_ARRAY_SIZE(0))
   print("FeatureDescription:", Str, "\n")

   RegLen = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_SIZE, FeatureName, MIL.M_TYPE_INT64)   
   import numpy
   RegLenList = numpy.zeros(RegLen, dtype=numpy.uint8)
   MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VALUE, FeatureName, MIL.M_TYPE_UINT8+MIL.M_FEATURE_USER_ARRAY_SIZE(RegLen), RegLenList)

   # Inquire the feature's native data type.
   FeatureType = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_TYPE, FeatureName, MIL.M_TYPE_INT64)
   # Inquire the feature's native data size in bytes. 
   FeatureSize = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_SIZE, FeatureName, MIL.M_TYPE_MIL_INT)
   # Inquire the feature's access mode. 
   FeatureAccessMode = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_ACCESS_MODE, FeatureName, MIL.M_TYPE_INT64)
   # Inquire the feature's visibility attribute. 
   FeatureVisibility = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VISIBILITY, FeatureName, MIL.M_TYPE_INT64)
   # Inquire the feature's caching mode attribute. 
   FeatureCachingMode = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_CACHING_MODE, FeatureName, MIL.M_TYPE_INT64)
   # Inquire the feature's streamable attribute. 
   FeatureStreamable = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_STREAMABLE, FeatureName, MIL.M_TYPE_INT64)
   # Inquire the feature's deprecated attribute. 
   FeatureDeprecated = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_DEPRECATED, FeatureName, MIL.M_TYPE_INT64)

   print("Type:                       ", TypeToString(FeatureType))
   print("Size:                       ", str(FeatureSize), "bytes")
   print("Readable:                   ", str(MIL.M_FEATURE_IS_READABLE(FeatureAccessMode)).lower())
   print("Writable:                   ", str(MIL.M_FEATURE_IS_WRITABLE(FeatureAccessMode)).lower())
   print("Visibility:                 ", str(VisibilityToString(FeatureVisibility)))
   print("Available:                  ", str(MIL.M_FEATURE_IS_AVAILABLE(FeatureAccessMode)).lower())
   print("Implemented:                ", str(MIL.M_FEATURE_IS_IMPLEMENTED(FeatureAccessMode)).lower())
   print("Cachable:                   ", str(MIL.M_FEATURE_IS_CACHABLE(FeatureCachingMode)).lower())
   print("Streamable:                 ", str(bool(FeatureStreamable)).lower())
   print("Deprecated:                 ", str(bool(FeatureDeprecated)).lower())

   # Inquire the feature's value.
   if MIL.M_FEATURE_IS_READABLE(FeatureAccessMode):
      
      # For floating point type features inquire the feature's value range. 
      if FeatureType == MIL.M_TYPE_DOUBLE:     
         Min = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_MIN, FeatureName, MIL.M_TYPE_DOUBLE)
         Max = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_MAX, FeatureName, MIL.M_TYPE_DOUBLE)
         Representation = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_REPRESENTATION, FeatureName, MIL.M_TYPE_INT64)
         print("Min:                        ", Min)
         print("Max:                        ", Max)
         print("Representation:             ", Representation)
   
      # For integer type features inquire the feature's value range.
      elif FeatureType == MIL.M_TYPE_INT64:
         Min = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_MIN, FeatureName, MIL.M_TYPE_INT64)
         Max = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_MAX, FeatureName, MIL.M_TYPE_INT64)
         Inc = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_INCREMENT, FeatureName, MIL.M_TYPE_INT64)
         Representation = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_REPRESENTATION, FeatureName, MIL.M_TYPE_INT64)
         print("Min:                        ", Min)
         print("Max:                        ", Max)
         print("Inc:                        ", Inc)
         print("Representation:             ", RepresentationToString(Representation))
      
      # Inquire the feature's value as a sting.
      ValueAsString = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VALUE, FeatureName, MIL.M_TYPE_STRING)
      print("Value as string:            ", ValueAsString)
      
      # Inquire the feature's value using it's native data type. 
      if FeatureType == MIL.M_TYPE_INT64:
         IntVal = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VALUE, FeatureName, MIL.M_TYPE_INT64)
         print("Value:                       {} ({})".format(IntVal, hex(IntVal)))
      elif FeatureType == MIL.M_TYPE_DOUBLE:
         DoubleVal = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VALUE, FeatureName, MIL.M_TYPE_DOUBLE)
         print("Value:                     ", DoubleVal)
      elif FeatureType == MIL.M_TYPE_STRING:
         ValueAsString = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VALUE, FeatureName, MIL.M_TYPE_STRING+MIL.M_FEATURE_USER_ARRAY_SIZE(0))
         print("Value:                     ", ValueAsString)
      elif FeatureType == MIL.M_TYPE_BOOLEAN:
         BoolVal = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VALUE, FeatureName, MIL.M_TYPE_BOOLEAN)
         print("Value:                     ", BoolVal)
      elif FeatureType == MIL.M_TYPE_ENUMERATION:
         IntVal = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VALUE, FeatureName, MIL.M_TYPE_INT64)
         print("Value:                       {} ({})".format(IntVal, hex(IntVal)))

         Count = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_ENUM_ENTRY_COUNT, FeatureName, MIL.M_TYPE_MIL_INT)
         if Count:
   
            j = 0
            for i in range(0, Count):
               AccessMode = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_ENUM_ENTRY_ACCESS_MODE + i, FeatureName, MIL.M_TYPE_INT64)
               if MIL.M_FEATURE_IS_AVAILABLE(AccessMode):

                  Name = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_ENUM_ENTRY_NAME+i, FeatureName, MIL.M_TYPE_STRING+MIL.M_FEATURE_USER_ARRAY_SIZE(0))
                  if j == 0:
                     print("Enum Entry Names:           ", Name)
                  else:
                     print("                            ", Name)
                  j += 1
                  
            j = 0
            for i in range (0, Count):
               AccessMode = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_ENUM_ENTRY_ACCESS_MODE + i, FeatureName, MIL.M_TYPE_INT64)
               if MIL.M_FEATURE_IS_AVAILABLE(AccessMode):
                  Name = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_ENUM_ENTRY_DISPLAY_NAME+i, FeatureName, MIL.M_TYPE_STRING+MIL.M_FEATURE_USER_ARRAY_SIZE(0))
                  if j == 0:
                     print("Enum Entry Display Names:   ", Name)
                  else:
                     print("                            ", Name)
                  j += 1
                  
      elif FeatureType == MIL.M_TYPE_REGISTER:
         RegLen = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_SIZE, FeatureName, MIL.M_TYPE_INT64)
         RegVal = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VALUE, FeatureName, MIL.M_TYPE_UINT8+MIL.M_FEATURE_USER_ARRAY_SIZE(RegLen))
         print("Value: ", RegVal) # Print every value in RegVal
            
      elif FeatureType == MIL.M_DEFAULT or FeatureType == MIL.M_TYPE_CATEGORY:
         pass
      else:
         pass

   return 

   
# Routine used to enumerate the device's GenICam(c) node tree. 
def EnumerateGenICamNodeTree(MilDigitizer, Node, RecurseCount):
      
   NodeCount = 0
   SubNodeCount = 0
   FeatureName = ""
   FeatureValue = ""
   
   if RecurseCount == 0:
      print("%-40s%-19s%20s" % ("Feature Name", "Feature Type", "Feature Value"))
      print(str(79*"-"))
      
   # Inquire the number of elements under this node. 
   NodeCount = MIL.MdigInquireFeature(MilDigitizer, MIL.M_SUBFEATURE_COUNT, Node, MIL.M_TYPE_MIL_INT)
   for i in range(0, NodeCount):

      # For each element under this node inquire it's string name. 
      FeatureName = MIL.MdigInquireFeature(MilDigitizer, MIL.M_SUBFEATURE_NAME+i, Node, MIL.M_TYPE_STRING+MIL.M_FEATURE_USER_ARRAY_SIZE(0))
      # For each element under this node inquire it's type. 
      Type = MIL.MdigInquireFeature(MilDigitizer, MIL.M_SUBFEATURE_TYPE+i, Node, MIL.M_TYPE_INT64)
      # For each element under this node inquire it's read property. 
      AccessMode = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_ACCESS_MODE, FeatureName, MIL.M_TYPE_INT64)
      # For each element under this node inquire it's visibility property. 
      Visibility = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VISIBILITY, FeatureName, MIL.M_TYPE_INT64)

      # Validate that the feature is actually implemented on this specific device. 
      if MIL.M_FEATURE_IS_IMPLEMENTED(AccessMode):
         
         # Read the feature's value only if it's a value feature and is readable. 
         if ((Type != MIL.M_DEFAULT and Type != MIL.M_TYPE_CATEGORY and Type != MIL.M_TYPE_REGISTER) and MIL.M_FEATURE_IS_READABLE(AccessMode)):
            FeatureValue = MIL.MdigInquireFeature(MilDigitizer, MIL.M_FEATURE_VALUE, FeatureName, MIL.M_TYPE_STRING+MIL.M_FEATURE_USER_ARRAY_SIZE(0))
            
         # Features under a selector will appear twice. Filter out the double. 
         if FeatureName not in FeatureList:
            
            # Feature not in FeatureList, insert it at the end. 
            FeatureList.append(FeatureName)

            # Inquire if this feature has child nodes. 
            SubNodeCount = MIL.MdigInquireFeature(MilDigitizer, MIL.M_SUBFEATURE_COUNT, FeatureName, MIL.M_TYPE_MIL_INT)

            # Print the feature name if visibility level ok. 
            if(Visibility <= VISIBILITY_LEVEL):
               str_print = ""
               FeatureNamePrint = (RecurseCount)*"   " + FeatureName
               if FeatureValue:
                  str_print = str(FeatureValue)
               print("%-40s%-19s%20.19s" % (FeatureNamePrint, TypeToString(Type), str_print, ))

            # If child nodes exist enumerate them. 
            if(SubNodeCount != 0):
               EnumerateGenICamNodeTree(MilDigitizer, FeatureName, RecurseCount+1)
            

# Helper function used to convert a feature type into its string representation. 
def TypeToString(Type):
    
    sType = None
   
    if Type == MIL.M_TYPE_INT64:
        sType = "M_TYPE_INT64"
    elif Type == MIL.M_TYPE_DOUBLE:
        sType = "M_TYPE_DOUBLE" 
    elif Type == MIL.M_TYPE_BOOLEAN:
        sType = "M_TYPE_BOOLEAN"
    elif Type == MIL.M_TYPE_STRING:
        sType = "M_TYPE_STRING"
    elif Type == MIL.M_TYPE_ENUMERATION:
        sType = "M_TYPE_ENUMERATION"
    elif Type == MIL.M_TYPE_COMMAND:
        sType = "M_TYPE_COMMAND"
    elif Type == MIL.M_TYPE_REGISTER:
        sType = "M_TYPE_REGISTER"
    elif Type == MIL.M_TYPE_CATEGORY:
        sType = "M_TYPE_CATEGORY"
    else:
        sType = ("M_NULL")
      
    return sType
   

def VisibilityToString(Visibility):
    
    sVisibility = None
    
    if Visibility == MIL.M_FEATURE_VISIBILITY_BEGINNER:
        sVisibility = "Beginner" 
    elif Visibility == MIL.M_FEATURE_VISIBILITY_EXPERT:
        sVisibility = "Expert"
    elif Visibility == MIL.M_FEATURE_VISIBILITY_GURU:
        sVisibility = "Guru"
    elif Visibility == MIL.M_FEATURE_VISIBILITY_INVISIBLE:
        sVisibility = "Invisible"
    return sVisibility
   

def RepresentationToString(Representation):
    
    sRepresentation = None

    if Representation == MIL.M_FEATURE_REPRESENTATION_LINEAR:
        sRepresentation = "Linear"
    elif Representation == MIL.M_FEATURE_REPRESENTATION_LOGARITHMIC:
        sRepresentation = "Logarithmic"
    elif Representation == MIL.M_FEATURE_REPRESENTATION_BOOLEAN:
        sRepresentation = "Boolean"
    elif Representation == MIL.M_FEATURE_REPRESENTATION_PURE_NUMBER:
        sRepresentation = "Pure number"
    elif Representation == MIL.M_FEATURE_REPRESENTATION_HEX_NUMBER:
        sRepresentation = "Hex number"
    elif Representation == MIL.M_FEATURE_REPRESENTATION_IPV4_ADDRESS:
        sRepresentation = "IPv4 address"
    elif Representation == MIL.M_FEATURE_REPRESENTATION_MAC_ADDRESS:
        sRepresentation = "MAC address"

    return sRepresentation
   
if __name__ == "__main__":
   EnumProcessExample()
