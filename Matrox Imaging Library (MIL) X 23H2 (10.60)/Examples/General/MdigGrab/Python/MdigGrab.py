#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#########################################################################################
#
# File name: MdigGrab.py
#
# Synopsis:  This program demonstrates how to grab from a camera in
#            continuous and monoshot mode.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
#########################################################################################

import mil as MIL

# Main function.
def MdigGrabExample():
   
   # Allocate defaults.
   MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage = MIL.MappAllocDefault(MIL.M_DEFAULT)

   # Grab continuously.
   MIL.MdigGrabContinuous(MilDigitizer, MilImage)

   # When a key is pressed, halt. 
   print("\nDIGITIZER ACQUISITION:")
   print("----------------------\n")
   print("Continuous image grab in progress.")
   print("Press <Enter> to stop.\n")
   MIL.MosGetch()

   # Stop continuous grab
   MIL.MdigHalt(MilDigitizer)

   # Pause to show the result. 
   print("Continuous grab stopped.\n")
   print("Press <Enter> to do a single image grab.\n")
   MIL.MosGetch()

   # Monoshot grab. 
   MIL.MdigGrab(MilDigitizer, MilImage)

   # Pause to show the result. 
   print("Displaying the grabbed image.")
   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free defaults.
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage)

   return 0

if __name__ == "__main__":
   MdigGrabExample()