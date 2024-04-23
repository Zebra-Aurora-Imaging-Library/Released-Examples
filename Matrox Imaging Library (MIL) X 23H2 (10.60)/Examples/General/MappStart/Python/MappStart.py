#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MAppStart.py 
#
# Synopsis:  This program allocates a MIL application and system, then displays 
#            a welcoming message using graphics functions. It also shows how 
#            to check for errors.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
 
import mil as MIL

def MappStartExample():
   # Allocate a default MIL application, system, display and image.
   MilApplication, MilSystem, MilDisplay, MilImage = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL)

   # If no allocation errors.
   if not MIL.MappGetError(MIL.M_DEFAULT, MIL.M_GLOBAL, MIL.M_NULL):
      # Perform graphic operations in the display image. 
      MIL.MgraColor(MIL.M_DEFAULT, 0xF0)
      MIL.MgraFont(MIL.M_DEFAULT, MIL.M_FONT_DEFAULT_LARGE)
      MIL.MgraText(MIL.M_DEFAULT, MilImage, 160, 230, " Welcome to MIL !!! ")
      MIL.MgraColor(MIL.M_DEFAULT, 0xC0)
      MIL.MgraRect(MIL.M_DEFAULT, MilImage, 100, 150, 530, 340)
      MIL.MgraRect(MIL.M_DEFAULT, MilImage, 120, 170, 510, 320)
      MIL.MgraRect(MIL.M_DEFAULT, MilImage, 140, 190, 490, 300)

      # Print a message. 
      print("\nSYSTEM ALLOCATION:")
      print("------------------\n")
      print("System allocation successful.\n")
      print("     \"Welcome to MIL !!!\"\n")
   else:
      print("System allocation error !\n")

   # Wait for a key press.
   print("Press <Enter> to end.")
   MIL.MosGetch()

   # Free defaults.
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MilImage)
   
   return 0


if __name__ == "__main__":
   MappStartExample()
