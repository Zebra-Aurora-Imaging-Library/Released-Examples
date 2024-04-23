#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: M3dmod.cpp
#
# Synopsis: This example demonstrates how to use the 3D model finder module
#           to define surface models and search for them in 3D scenes.
#           A simple single model search is presented first followed by a more
#           complex example of multiple occurrences in a complex scene.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
##########################################################################

 
import mil as MIL

# --------------------------------------------------------------
# CDisplay 
# Class that manages the 2D/3D mil displays for 3D examples.
# --------------------------------------------------------------
class CDisplay:
   def __init__(self, MilSystem):
      self.m_MilSystem = MilSystem
      self.m_MilDisplay     = MIL.M_NULL
      self.m_MilGraphicList = MIL.M_NULL
      self.m_DisplayType    = MIL.M_NULL
      self.m_Lut            = MIL.M_NULL
      self.m_MilDepthMap    = MIL.M_NULL
      self.m_IntensityMap   = MIL.M_NULL

   # --------------------------------------------------------------
   # Allocates a 3D display and returns its MIL identifier.
   # --------------------------------------------------------------
   def Alloc3dDisplayId(self):
      # Try to allocate a 3d display.
      MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
      self.m_MilDisplay = MIL.M3ddispAlloc(self.m_MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
      MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)

      if self.m_MilDisplay == MIL.M_NULL:
         print()
         print("The current system does not support the 3D display.")
         print("A 2D display will be used instead.")
         
         # Allocate a 2d display instead.
         self.m_MilDisplay = MIL.MdispAlloc(self.m_MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT)
         self.m_Lut        = MIL.MbufAllocColor(self.m_MilSystem, 3, 256, 1, MIL.M_UNSIGNED + 8, MIL.M_LUT)
         MIL.MgenLutFunction(self.m_Lut, MIL.M_COLORMAP_TURBO + MIL.M_FLIP, MIL.M_DEFAULT,
                             MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                             MIL.M_DEFAULT)

      self.m_DisplayType = MIL.MobjInquire(self.m_MilDisplay, MIL.M_OBJECT_TYPE)
      self.GetGraphicListId()
      
   # --------------------------------------------------------------
   # Sets the window size.
   # --------------------------------------------------------------
   def Size(self, SizeX, SizeY):

      if (self.m_DisplayType == MIL.M_3D_DISPLAY):
         MIL.M3ddispControl(self.m_MilDisplay, MIL.M_SIZE_X, SizeX)
         MIL.M3ddispControl(self.m_MilDisplay, MIL.M_SIZE_Y, SizeY)

      else:
         self.m_MilDepthMap  = MIL.MbufAlloc2d(self.m_MilSystem, int(SizeX), int(SizeY), MIL.M_UNSIGNED + 8, MIL.M_IMAGE | MIL.M_PROC | MIL.M_DISP)
         self.m_IntensityMap = MIL.MbufAllocColor(self.m_MilSystem, 3, int(SizeX), int(SizeY), MIL.M_UNSIGNED + 8, MIL.M_IMAGE | MIL.M_PROC | MIL.M_DISP)

   # --------------------------------------------------------------
   # Sets the window position x.
   # --------------------------------------------------------------
   def PositionX(self, PositionX):
      if (self.m_DisplayType == MIL.M_3D_DISPLAY):
         MIL.M3ddispControl(self.m_MilDisplay, MIL.M_WINDOW_INITIAL_POSITION_X, PositionX)

      else:
         MIL.MdispControl(self.m_MilDisplay, MIL.M_WINDOW_INITIAL_POSITION_X, PositionX)

   # --------------------------------------------------------------
   # Displays the container in the 3D or 2D display.
   # --------------------------------------------------------------
   def DisplayContainer(self, MilContainer, UseLut):
      if (self.m_DisplayType == MIL.M_3D_DISPLAY):
         Label = MIL.M3ddispSelect(self.m_MilDisplay, MilContainer, MIL.M_DEFAULT, MIL.M_DEFAULT)
         if (UseLut):
            MIL.M3dgraCopy(MIL.M_COLORMAP_TURBO + MIL.M_FLIP, MIL.M_DEFAULT, self.m_MilGraphicList, Label, MIL.M_COLOR_LUT, MIL.M_DEFAULT)
            MIL.M3dgraControl(self.m_MilGraphicList, Label, MIL.M_COLOR_USE_LUT, MIL.M_TRUE)
            MIL.M3dgraControl(self.m_MilGraphicList, Label, MIL.M_COLOR_COMPONENT_BAND, 2)
            MIL.M3dgraControl(self.m_MilGraphicList, Label, MIL.M_COLOR_COMPONENT, MIL.M_COMPONENT_RANGE)
      else:
         # Project into a depthmap.
         MIL.M3dimCalibrateDepthMap(MilContainer, self.m_MilDepthMap, self.m_IntensityMap, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_CENTER)

         if (UseLut):
            # Associate a LUT.
            MIL.MbufControl(self.m_MilDepthMap, MIL.M_ASSOCIATED_LUT, self.m_Lut)
            MIL.M3dimProject(MilContainer, self.m_MilDepthMap, MIL.M_NULL, MIL.M_POINT_BASED, MIL.M_MAX_Z, MIL.M_DEFAULT, MIL.M_DEFAULT)
            MIL.MdispSelect(self.m_MilDisplay, self.m_MilDepthMap)

         else:
            HasReflectanceColor = MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_REFLECTANCE, MIL.M_COMPONENT_ID) != MIL.M_NULL
            HasIntensityColor = MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_INTENSITY, MIL.M_COMPONENT_ID)  != MIL.M_NULL

            if (HasReflectanceColor or HasIntensityColor):
               MIL.M3dimProject(MilContainer, self.m_MilDepthMap, self.m_IntensityMap, MIL.M_POINT_BASED, MIL.M_MAX_Z, MIL.M_DEFAULT, MIL.M_DEFAULT)
               MIL.MdispSelect(self.m_MilDisplay, self.m_IntensityMap)
            else:
               MIL.M3dimProject(MilContainer, self.m_MilDepthMap, MIL.M_NULL, MIL.M_POINT_BASED, MIL.M_MAX_Z, MIL.M_DEFAULT, MIL.M_DEFAULT)
               MIL.MdispSelect(self.m_MilDisplay, self.m_MilDepthMap)

   # --------------------------------------------------------------
   #  Set the title.
   # --------------------------------------------------------------
   def Title(self, Title):
      if (self.m_DisplayType == MIL.M_3D_DISPLAY):
         MIL.M3ddispControl(self.m_MilDisplay, MIL.M_TITLE, Title)
      else:
         MIL.MdispControl(self.m_MilDisplay, MIL.M_TITLE, Title)

   # --------------------------------------------------------------
   #  Set the 3D disply view.
   # --------------------------------------------------------------
   def SetView(self, Mode, Param1, Param2, Param3):
      if (self.m_DisplayType == MIL.M_3D_DISPLAY):
         MIL.M3ddispSetView(self.m_MilDisplay, Mode, Param1, Param2, Param3, MIL.M_DEFAULT)

   # --------------------------------------------------------------
   # Draw the 3d model occurrences found.
   # --------------------------------------------------------------
   def Draw(self, MilResult):
      if (self.m_DisplayType == MIL.M_3D_DISPLAY):
         return MIL.M3dmodDraw3d(MIL.M_DEFAULT, MilResult, MIL.M_ALL, self.m_MilGraphicList, MIL.M_DEFAULT, MIL.M_DEFAULT)
      
      else:
         Mil3dGraphicList  = MIL.M3dgraAlloc(self.m_MilSystem, MIL.M_DEFAULT)
         MIL.M3dmodDraw3d(MIL.M_DEFAULT, MilResult, MIL.M_ALL, Mil3dGraphicList, MIL.M_DEFAULT, MIL.M_DEFAULT)

         # Clear the graphic list.
         MIL.MgraControlList(self.m_MilGraphicList, MIL.M_ALL, MIL.M_DEFAULT, MIL.M_DELETE, MIL.M_DEFAULT)

         # Get all 3d graphics.
         Labels = MIL.M3dgraInquire(Mil3dGraphicList, MIL.M_ROOT_NODE, MIL.M_CHILDREN + MIL.M_RECURSIVE)

         Matrix = MIL.M3dgeoAlloc(self.m_MilSystem, MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT)

         MilContainer = MIL.MbufAllocContainer(self.m_MilSystem, MIL.M_PROC | MIL.M_DISP, MIL.M_DEFAULT)
         
         # Draw all 3d boxes and dots in the 2d display.
         for Label in Labels:
            GraphicType = MIL.M3dgraInquire(Mil3dGraphicList, Label, MIL.M_GRAPHIC_TYPE)

            if (GraphicType == MIL.M_GRAPHIC_TYPE_DOTS):
               Color = MIL.M3dgraInquire(Mil3dGraphicList, Label, MIL.M_COLOR)
               PointsX = MIL.M3dgraInquire(Mil3dGraphicList, Label, MIL.M_POINTS_X)
               PointsY = MIL.M3dgraInquire(Mil3dGraphicList, Label, MIL.M_POINTS_Y)

               MIL.MgraControl(MIL.M_DEFAULT, MIL.M_COLOR, Color)
               MIL.MgraControl(MIL.M_DEFAULT, MIL.M_INPUT_UNITS, MIL.M_WORLD)
               MIL.MgraDots(MIL.M_DEFAULT, self.m_MilGraphicList, MIL.M_DEFAULT, PointsX, PointsY, MIL.M_DEFAULT)

            elif (GraphicType == MIL.M_GRAPHIC_TYPE_BOX):
               CenterX = MIL.M3dgraInquire(Mil3dGraphicList, Label, MIL.M_CENTER_X)
               CenterY = MIL.M3dgraInquire(Mil3dGraphicList, Label, MIL.M_CENTER_Y)
               SizeX =  MIL.M3dgraInquire(Mil3dGraphicList, Label, MIL.M_SIZE_X)
               SizeY = MIL.M3dgraInquire(Mil3dGraphicList, Label, MIL.M_SIZE_Y)
               MIL.M3dgraCopy(Mil3dGraphicList, Label, Matrix, MIL.M_DEFAULT, MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT)
               (RotZ, RotX, RotY) = MIL.M3dgeoMatrixGetTransform(Matrix, MIL.M_ROTATION_ZXY, None, None, None, MIL.M_NULL, MIL.M_DEFAULT)

               MIL.MgraControl(MIL.M_DEFAULT, MIL.M_COLOR, MIL.M_COLOR_WHITE)
               MIL.MgraControl(MIL.M_DEFAULT, MIL.M_INPUT_UNITS, MIL.M_WORLD)
               MIL.MgraRectAngle(MIL.M_DEFAULT, self.m_MilGraphicList, CenterX, CenterY, SizeX, SizeY, -RotZ, MIL.M_CENTER_AND_DIMENSION)

         MIL.M3dgeoFree(Matrix)
         MIL.M3dgraFree(Mil3dGraphicList)
         MIL.MbufFree(MilContainer)
      return 0

   # --------------------------------------------------------------
   # Updates the displayed image.
   # --------------------------------------------------------------
   def UpdateDisplay(self, MilContainer, UseLut):
      if (self.m_DisplayType == MIL.M_3D_DISPLAY):
            return; # Containers are updated automatically in the 3D display
      else:
            self.DisplayContainer(MilContainer, UseLut)

   def Clear(self, Label):
      if (self.m_DisplayType == MIL.M_3D_DISPLAY):
            MIL.M3dgraRemove(self.m_MilGraphicList, Label, MIL.M_DEFAULT)
      else:
            MIL.MgraControlList(self.m_MilGraphicList, MIL.M_ALL, MIL.M_DEFAULT, MIL.M_DELETE, MIL.M_DEFAULT)

   # --------------------------------------------------------------
   # Free the display.
   # --------------------------------------------------------------
   def FreeDisplay(self):
      if (self.m_DisplayType == MIL.M_DISPLAY):
            MIL.MdispFree(self.m_MilDisplay)
            MIL.MbufFree(self.m_Lut)
            MIL.MbufFree(self.m_MilDepthMap)
            MIL.MbufFree(self.m_IntensityMap)
            MIL.MgraFree(self.m_MilGraphicList)
      else:
            MIL.M3ddispFree(self.m_MilDisplay)

   # --------------------------------------------------------------
   # Gets the display's graphic list, or allocates a standalone one.
   # --------------------------------------------------------------
   def GetGraphicListId(self):
      if (self.m_DisplayType == MIL.M_3D_DISPLAY):
         self.m_MilGraphicList = MIL.M3ddispInquire(self.m_MilDisplay, MIL.M_3D_GRAPHIC_LIST_ID)
      else:
         # Associate a graphic list.
         self.m_MilGraphicList = MIL.MgraAllocList(self.m_MilSystem, MIL.M_DEFAULT)
         MIL.MdispControl(self.m_MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, self.m_MilGraphicList)



# Input scanned point cloud files.
SINGLE_MODEL   = MIL.M_IMAGE_PATH + "SimpleModel.mbufc"
SINGLE_SCENE   = MIL.M_IMAGE_PATH + "SimpleScene.mbufc"
COMPLEX_MODEL1 = MIL.M_IMAGE_PATH + "ComplexModel1.ply"
COMPLEX_MODEL2 = MIL.M_IMAGE_PATH + "ComplexModel2.ply"
COMPLEX_SCENE  = MIL.M_IMAGE_PATH + "ComplexScene.ply"

# Constants.
DISP_SIZE_X = 480
DISP_SIZE_Y = 420

# --------------------------------------------------------------
# Simple scene with a single occurrence.
# --------------------------------------------------------------
def SimpleSceneSurfaceFinder(MilSystem, DisplayModel : CDisplay, DisplayScene : CDisplay):
   # Allocate a surface Model Finder context.
   MilContext = MIL.M3dmodAlloc(MilSystem, MIL.M_FIND_SURFACE_CONTEXT, MIL.M_DEFAULT)

   # Allocate a surface Model Finder result.
   MilResult = MIL.M3dmodAllocResult(MilSystem, MIL.M_FIND_SURFACE_RESULT, MIL.M_DEFAULT)

   # Restore the model container and display it.
   MilModelContainer = MIL.MbufRestore(SINGLE_MODEL, MilSystem)
   DisplayModel.SetView(MIL.M_AZIM_ELEV_ROLL, 45, -35, 180)
   DisplayModel.DisplayContainer(MilModelContainer, True)
   print("The 3D point cloud of the model is restored from a file and displayed.")

   # Load the single model scene point cloud.
   MilSceneContainer = MIL.MbufRestore(SINGLE_SCENE, MilSystem)

   DisplayScene.SetView(MIL.M_AZIM_ELEV_ROLL, 202, -20.0, 182.0)
   DisplayScene.DisplayContainer(MilSceneContainer, True)

   print("The 3D point cloud of the scene is restored from a file and displayed.\n")
   print("Press <Enter> to start.\n");
   MIL.MosGetch()

   # Define the surface model.
   MIL.M3dmodDefine(MilContext, MIL.M_ADD_FROM_POINT_CLOUD, MIL.M_SURFACE,
                     MilModelContainer, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                     MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
   print("Define the model using the given model point cloud.\n")

   # Set the search perseverance.
   print("Set the lowest perseverance to increase the search speed for a simple scene.\n")
   MIL.M3dmodControl(MilContext, MIL.M_DEFAULT, MIL.M_PERSEVERANCE, 0.0)

   print("Set the scene complexity to low to increase the search speed for a simple scene.\n")
   MIL.M3dmodControl(MilContext, MIL.M_DEFAULT, MIL.M_SCENE_COMPLEXITY, MIL.M_LOW)

   # Preprocess the search context.
   MIL.M3dmodPreprocess(MilContext, MIL.M_DEFAULT)

   print("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n")
   # The surface finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud.
   AddComponentNormalsIfMissing(MilSceneContainer)

   print("3D surface finder is running..\n")

   # Reset the timer.
   MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS)

   # Find the model.
   MIL.M3dmodFind(MilContext, MilSceneContainer, MilResult, MIL.M_DEFAULT)

   # Read the find time.
   ComputationTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)

   ShowResults(MilResult, ComputationTime)
   DisplayScene.Draw(MilResult)

   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Free mil objects.
   MIL.M3dmodFree(MilContext)
   MIL.M3dmodFree(MilResult)
   MIL.MbufFree(MilModelContainer)
   MIL.MbufFree(MilSceneContainer)

# --------------------------------------------------------------
# Complex scene with multiple occurrences.
# --------------------------------------------------------------
def ComplexSceneSurfaceFinder(MilSystem, DisplayModel : CDisplay, DisplayScene : CDisplay):
   # Allocate a surface 3D Model Finder context. 
   MilContext = MIL.M3dmodAlloc(MilSystem, MIL.M_FIND_SURFACE_CONTEXT, MIL.M_DEFAULT)

   # Allocate a surface 3D Model Finder result.
   MilResult = MIL.M3dmodAllocResult(MilSystem, MIL.M_FIND_SURFACE_RESULT, MIL.M_DEFAULT)

   DisplayModel.Clear(MIL.M_ALL)
   DisplayScene.Clear(MIL.M_ALL)

   # Restore the first model container and display it.
   MilModelContainer = MIL.MbufRestore(COMPLEX_MODEL1, MilSystem)
   DisplayModel.SetView(MIL.M_AZIM_ELEV_ROLL, 290, -67, 265)
   DisplayModel.DisplayContainer(MilModelContainer, False)
   DisplayModel.SetView(MIL.M_AUTO, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
   print("The 3D point cloud of the first model is restored from a file and displayed.")

   # Load the complex scene point cloud. 
   MilSceneContainer = MIL.MbufRestore(COMPLEX_SCENE, MilSystem)

   DisplayScene.SetView(MIL.M_AZIM_ELEV_ROLL, 260, -72, 142)
   DisplayScene.DisplayContainer(MilSceneContainer, False)
   DisplayScene.SetView(MIL.M_AUTO, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)
   DisplayScene.SetView(MIL.M_ZOOM, 1.2, MIL.M_DEFAULT, MIL.M_DEFAULT)

   print("The 3D point cloud of the scene is restored from a file and displayed.\n")
   print("Press <Enter> to start.\n")
   MIL.MosGetch()

   print("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n")

   # The surface finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud. 
   AddComponentNormalsIfMissing(MilSceneContainer)

   # Define the surface model.
   MIL.M3dmodDefine(MilContext, MIL.M_ADD_FROM_POINT_CLOUD, MIL.M_SURFACE,
                     MilModelContainer, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                     MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Find all ocurrences.
   MIL.M3dmodControl(MilContext, 0, MIL.M_NUMBER, MIL.M_ALL)
   MIL.M3dmodControl(MilContext, 0, MIL.M_COVERAGE_MAX, 75)

   MIL.M3dmodPreprocess(MilContext, MIL.M_DEFAULT)
   print("3D surface finder is running..\n")

   # Reset the timer. 
   MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS)

   # Find the model. 
   MIL.M3dmodFind(MilContext, MilSceneContainer, MilResult, MIL.M_DEFAULT)

   # Read the find time.
   ComputationTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)

   ShowResults(MilResult, ComputationTime)
   Label = DisplayScene.Draw(MilResult)

   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   DisplayScene.Clear(Label)

   MIL.MbufFree(MilModelContainer)
   MilModelContainer = MIL.MbufRestore(COMPLEX_MODEL2, MilSystem)
   DisplayModel.DisplayContainer(MilModelContainer, False)
   DisplayModel.SetView(MIL.M_AUTO, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   print("The 3D point cloud of the second model is restored from file and displayed.\n")

   # Delete the previous model. 
   MIL.M3dmodDefine(MilContext, MIL.M_DELETE, MIL.M_DEFAULT, 0, MIL.M_DEFAULT,
                     MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                     MIL.M_DEFAULT)

   # Define the surface model.
   MIL.M3dmodDefine(MilContext, MIL.M_ADD_FROM_POINT_CLOUD, MIL.M_SURFACE,
                     MilModelContainer, MIL.M_DEFAULT, MIL.M_DEFAULT,
                     MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Find all ocurrences.
   MIL.M3dmodControl(MilContext, 0, MIL.M_NUMBER, MIL.M_ALL)
   MIL.M3dmodControl(MilContext, 0, MIL.M_COVERAGE_MAX, 95)

   MIL.M3dmodPreprocess(MilContext, MIL.M_DEFAULT)
   print("3D surface finder is running..\n")

   # Reset the timer.
   MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS)

   # Find the model.
   MIL.M3dmodFind(MilContext, MilSceneContainer, MilResult, MIL.M_DEFAULT)

   # Read the find time. 
   ComputationTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)

   ShowResults(MilResult, ComputationTime)
   DisplayScene.Draw(MilResult)

   print("Press <Enter> to end.\n")
   MIL.MosGetch()

   # Free mil objects.
   MIL.M3dmodFree(MilContext)
   MIL.M3dmodFree(MilResult)
   MIL.MbufFree(MilModelContainer)
   MIL.MbufFree(MilSceneContainer)

# --------------------------------------------------------------
# Adds the component M_COMPONENT_NORMALS_MIL if it's not found.
# --------------------------------------------------------------
def AddComponentNormalsIfMissing(MilContainer):
   MilNormals = MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_NORMALS_MIL, MIL.M_COMPONENT_ID)

   if (MilNormals != MIL.M_NULL):
         return
   SizeX = MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_RANGE, MIL.M_SIZE_X)
   SizeY = MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_RANGE, MIL.M_SIZE_Y)
   if (SizeX < 50 or SizeY < 50):
      MIL.M3dimNormals(MIL.M_NORMALS_CONTEXT_TREE, MilContainer, MilContainer, MIL.M_DEFAULT)
   else:
      MIL.M3dimNormals(MIL.M_NORMALS_CONTEXT_ORGANIZED, MilContainer, MilContainer, MIL.M_DEFAULT)

# --------------------------------------------------------------
# Shows the surface finder results.
# --------------------------------------------------------------
def ShowResults(MilResult, ComputationTime):
   Status = MIL.M3dmodGetResult(MilResult, MIL.M_DEFAULT, MIL.M_STATUS)
   if (Status != MIL.M_COMPLETE):
      print("The find process is not completed.")

   NbOcc = int(MIL.M3dmodGetResult(MilResult, MIL.M_DEFAULT, MIL.M_NUMBER))
   print("Found {0} occurrence(s) in {1:.2f} s.".format(NbOcc, ComputationTime))
   print()

   if (NbOcc == 0):
         return

   print("Index        Score        Score_Target");
   print("------------------------------------------------------");

   for i in range(NbOcc):
      ScoreTarget = MIL.M3dmodGetResult(MilResult, i, MIL.M_SCORE_TARGET)
      Score = MIL.M3dmodGetResult(MilResult, i, MIL.M_SCORE)
      print("  {0}          {1:.4f}      {2:6.2f}          ".format( i, Score, ScoreTarget))

   print()

# --------------------------------------------------------------
# Main.                                          
# --------------------------------------------------------------
def M3dmodExample():
   print("[EXAMPLE NAME]")
   print("M3dmod")
   print()
   print("[SYNOPSIS]")
   print("This example demonstrates how to use the 3D model finder module")
   print("to define surface models and search for them in 3D scenes.")
   print()

   print("[MODULES USED]")
   print("Modules used: 3D Model Finder, 3D Display, 3D Graphics, and 3D Image")
   print("Processing.")
   print()

   # Allocate MIL objects. 
   MilApplication = MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT)
   MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT)

   # Allocate the display.
   DisplayModel = CDisplay(MilSystem)
   DisplayModel.Alloc3dDisplayId()
   DisplayModel.Size(DISP_SIZE_X / 2, DISP_SIZE_Y / 2)
   DisplayModel.Title("Model Cloud")

   DisplayScene = CDisplay(MilSystem)
   DisplayScene.Alloc3dDisplayId()
   DisplayScene.Size(DISP_SIZE_X, DISP_SIZE_Y)
   DisplayScene.PositionX((1.04 * 0.5 * DISP_SIZE_X))
   DisplayScene.Title("Scene Cloud")

   SimpleSceneSurfaceFinder(MilSystem, DisplayModel, DisplayScene)

   ComplexSceneSurfaceFinder(MilSystem, DisplayModel, DisplayScene)

   # Free mil objects.
   DisplayModel.FreeDisplay()
   DisplayScene.FreeDisplay()
   MIL.MsysFree(MilSystem)
   MIL.MappFree(MilApplication)

   return 0

if __name__ == "__main__":
   M3dmodExample()
