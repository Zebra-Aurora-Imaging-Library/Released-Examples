#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# 
#  File name: MedgeDispTk.py  
#
#   Synopsis:  This program shows how to embed a MIL display 
#              into a Tkinter frame using Python language.
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
   sys.exit()

try:
   import tkinter as tk
except:
   try:
      import Tkinter as tk
   except:
      print("Import Tkinter library failure.")
      print("An error occurred while trying to import tkinter. Please make sure the tkinter package is installed.\n")
      print("Press <Enter> to end.\n")
      input()
      sys.exit()
      
import ctypes

# Source image path 
image_name = "Seals.mim"
image_path = MIL.M_IMAGE_PATH + image_name
   
class MilEdgeDispTkinter:

   def __init__(self, master, mil_system, mil_display, mil_gralist, mil_image, mil_edgectx, mil_edgeres):
      
      self.milsystem = mil_system
      self.milgralist = mil_gralist
      self.milimage = mil_image
      self.miledgectx = mil_edgectx
      self.miledgeres = mil_edgeres
	
      # Retrieve source image sizes 
      self.sizex = MIL.MbufInquire(self.milimage, MIL.M_SIZE_X)
      self.sizey = MIL.MbufInquire(self.milimage, MIL.M_SIZE_Y)
      
      # Set master title and define main frames
      master.title("MedgeDispTk Example")
      frame = tk.Frame(master)
      frame.pack()
      
      # Set frames and widgets
      self.frametop = tk.Frame(frame)
      self.frametop.pack(side=tk.TOP, fill=tk.X)
      tk.Label(self.frametop, font="Arial 14", bg="darkgray", fg="white", text="Matrox Imaging Library Tkinter Example", padx=5, pady=5).pack(fill=tk.X)
      
      self.framecenter = tk.Frame(frame, padx=3, pady=3)
      self.framecenter.pack(side=tk.TOP)
      
      self.frameleft = tk.Frame(self.framecenter, padx=3)
      self.frameleft.pack(side=tk.RIGHT)
      
      # Frame to embed a MIL display
      self.framelefttop = tk.Frame(self.frameleft, width=self.sizex, height=self.sizey, bg="", colormap="new")
      self.framelefttop.pack(side=tk.TOP)

      # Associated the MIL display to the frame.
      MIL.MdispSelectWindow(mil_display, mil_image, self.framelefttop.winfo_id())
      
      self.frameright = tk.Frame(self.framecenter)
      self.frameright.pack(side=tk.LEFT)
      
      self.buttonproc = tk.Button(self.frameright, fg="darkgreen", font="Arial 12", text="Extract\nedges", height=2, width=10, command=self.Doprocess)
      self.buttonproc.pack(side=tk.TOP)
      
      self.framebottom = tk.Frame(self.frameleft, width=self.sizex, height=64, pady=10)
      self.framebottom.pack(side=tk.BOTTOM)
		
      self.textresult = tk.Text(self.framebottom, bg="white", height=3, width=50)
      self.textresult.pack()

   def Doprocess(self):
    
      # Extract and draw contours 
      MIL.MedgeCalculate(self.miledgectx, self.milimage, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, self.miledgeres, MIL.M_DEFAULT)      
      MIL.MedgeDraw(MIL.M_DEFAULT, self.miledgeres, self.milgralist, MIL.M_DRAW_EDGES, MIL.M_DEFAULT, MIL.M_DEFAULT)
      
      # Retrieve and outputs the number of found contours 

      number_of_edges = MIL.MedgeGetResult(self.miledgeres, MIL.M_DEFAULT, MIL.M_NUMBER_OF_CHAINS + MIL.M_TYPE_MIL_INT, None, MIL.M_NULL)
     
      number_of_edges = MIL.MIL_INT(0)
      MIL.MedgeGetResult(self.miledgeres, MIL.M_DEFAULT, MIL.M_NUMBER_OF_CHAINS + MIL.M_TYPE_MIL_INT, ctypes.byref(number_of_edges), MIL.M_NULL)

      self.textresult.configure(state='normal')
      self.textresult.delete(1.0, tk.END)
      self.textresult.insert(tk.END, "Image : " + image_name + " (" + str(self.sizex) + "x" + str(self.sizey) + ")\n")
      self.textresult.insert(tk.END, "Edges : " + str(number_of_edges.value) + " contours have been found.\n")
      self.textresult.configure(state='disabled')

      
def DoClean():
   # Free MIL objects 
   MIL.MedgeFree(MilEdgeCtx)
   MIL.MedgeFree(MilEdgeRes)   
   MIL.MdispFree(MilDisplay)
   MIL.MgraFree(MilGraList)
   MIL.MbufFree(MilImage)
   MIL.MsysFree(MilSystem)
   MIL.MappFree(MilApplication)
   root.destroy()

# MAIN 
def main():
   
   print("\n[SYNOPSIS]\n")
   print("This program shows how to embed a MIL display\ninto a Tkinter frame using Python language.\n")
   print("Close the displayed window to exit.")
   
   # Allocate MIL objects 
   global MilApplication
   global MilSystem
   global MilDisplay
   global MilImage
   global MilGraList
   global MilEdgeCtx
   global MilEdgeRes
   global root
   
   MilApplication = MIL.MappAlloc("M_DEFAULT", MIL.M_DEFAULT)
   MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
   MilDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_WINDOWED)
   MilImage = MIL.MbufRestore(image_path, MilSystem)
   MilGraList = MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT)
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList)
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED)
   MilEdgeCtx = MIL.MedgeAlloc(MilSystem, MIL.M_CONTOUR, MIL.M_DEFAULT)
   MilEdgeRes = MIL.MedgeAllocResult(MilSystem, MIL.M_DEFAULT)

   # Start Tk module 
   root = tk.Tk()
   try :
       root.iconbitmap('mil.ico')
   except:
       pass
   MilEdgeDispTkinter(root, MilSystem, MilDisplay, MilGraList, MilImage, MilEdgeCtx, MilEdgeRes)
   root.protocol('WM_DELETE_WINDOW', DoClean)	
   root.mainloop()   
   
# Main execution call 
if __name__ == "__main__":
    main()
