#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MimProcessing.py 
#
# Synopsis:  This program show the usage of image processing. Under MIL lite, 
#            it binarizes two images to isolate specific zones.
#            Under MIL full, it also uses different image processing primitives 
#            to determine the number of cell nuclei that are larger than a 
#            certain size and show them in pseudo-color.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
 
import mil as MIL

# Target MIL image file specifications.
IMAGE_FILE = MIL.M_IMAGE_PATH + "Cell.mbufi"
IMAGE_CUP  = MIL.M_IMAGE_PATH + "PlasticCup.mim"
IMAGE_SMALL_PARTICLE_RADIUS = 1

def ExtractParticlesExample(MilApplication, MilSystem, MilDisplay):
   # Restore source image and display it.
   MilImage = MIL.MbufRestore(IMAGE_FILE, MilSystem)
   MIL.MdispSelect(MilDisplay, MilImage)
   
   # Pause to show the original image..
   print("\n1) Particles extraction:")
   print("-----------------\n")
   print("This first example extracts the dark particles in an image.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()
   
   # Binarize the image with an automatically calculated threshold so that 
   # particles are represented in white and the background removed.
   MIL.MimBinarize(MilImage, MilImage, MIL.M_BIMODAL + MIL.M_LESS_OR_EQUAL, MIL.M_NULL, MIL.M_NULL)
   
   # Print a message for the extracted particles.
   print("These particles were extracted from the original image.")
   
   # If MIL IM module is available, count and label the larger particles.
   MilRemoteApplication = MIL.MsysInquire(MilSystem, MIL.M_OWNER_APPLICATION)
   LicenseModules = MIL.MappInquire(MilRemoteApplication, MIL.M_LICENSE_MODULES)
   
   if (LicenseModules & MIL.M_LICENSE_IM) != 0:
      # Pause to show the extracted particles.
      print("Press <Enter> to continue.\n")
      MIL.MosGetch()
      
      # Close small holes
      MIL.MimClose(MilImage, MilImage, IMAGE_SMALL_PARTICLE_RADIUS, MIL.M_BINARY)
      
      # Remove small particles.
      MIL.MimOpen(MilImage, MilImage, IMAGE_SMALL_PARTICLE_RADIUS, MIL.M_BINARY)
      
      # Label the image.
      MIL.MimLabel(MilImage, MilImage, MIL.M_DEFAULT)
      
      # The largest label value corresponds to the number of particles in the image.
      MilExtremeResult = MIL.MimAllocResult(MilSystem, 1, MIL.M_EXTREME_LIST)
      MIL.MimFindExtreme(MilImage, MilExtremeResult, MIL.M_MAX_VALUE)

      MaxLabelNumber = MIL.MimGetResult(MilExtremeResult, MIL.M_VALUE)[0]

      MIL.MimFree(MilExtremeResult)
      
      # Multiply the labeling result to augment the gray level of the particles.
      MIL.MimArith(MilImage, int(256 / float(MaxLabelNumber)), MilImage, MIL.M_MULT_CONST)

      # Display the resulting particles in pseudo-color.
      MIL.MdispLut(MilDisplay, MIL.M_PSEUDO)
      
      # Print results.
      print("There were {MaxLabelNumber} large particles in the original image.".format(MaxLabelNumber = int(MaxLabelNumber)))
      
   
   # Pause to show the result
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()
   
   MIL.MdispLut(MilDisplay, MIL.M_DEFAULT)

   # Free all allocations.
   MIL.MbufFree(MilImage)
   
def ExtractForegroundExample(MilApplication, MilSystem, MilDisplay):
   # Restore source image and display it.
   MilImage = MIL.MbufRestore(IMAGE_CUP, MilSystem)
   MIL.MdispSelect(MilDisplay, MilImage)
   
   # Pause to show the original image..
   print("\n2) Background removal:")
   print("-----------------\n")
   print("This second example separates a cup on a table from the background using MimBinarize() with an M_DOMINANT mode.")
   print("In this case, the dominant mode (black background) is separated from the rest. Note, using an M_BIMODAL mode")
   print("would give another result because the background and the cup would be considered as the same mode.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()
   
   # Binarize the image with an automatically calculated threshold so that
   # cup and table are represented in white and the background removed
   MIL.MimBinarize(MilImage, MilImage, MIL.M_DOMINANT + MIL.M_LESS_OR_EQUAL, MIL.M_NULL, MIL.M_NULL)
   
   # Print a message for the extracted cup and table
   print("The cup and the table were separated from the background with M_DOMINANT mode of MimBinarize.")
   
   # Pause to show the result
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free all allocations.
   MIL.MbufFree(MilImage)

def MimProcessingExample():
   # Allocate a default MIL application, system, display and image.
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Show header
   print("\nIMAGE PROCESSING:")
   print("-----------------\n")
   print("This program shows two image processing examples.")
   
   # Example about extracting particles in an image
   ExtractParticlesExample(MilApplication, MilSystem, MilDisplay)
   
   # Example about isolating objects from the background in an image
   ExtractForegroundExample(MilApplication, MilSystem, MilDisplay)
   
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)
   
   return 0

if __name__ == "__main__":
   MimProcessingExample()
