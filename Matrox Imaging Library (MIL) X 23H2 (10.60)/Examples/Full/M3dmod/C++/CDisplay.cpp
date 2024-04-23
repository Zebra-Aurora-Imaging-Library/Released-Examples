/*****************************************************************************/
/*
/* File name: CDisplay.cpp
/*
/* Synopsis:  Class that manages the 2D/3D mil displays for 3D  
/*            examples.
/*
/* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
/* All Rights Reserved
/******************************************************************************/
#include "CDisplay.h"

CDisplay::CDisplay(MIL_ID MilSystem)
   :m_MilSystem(MilSystem)
   {}
/* -------------------------------------------------------------- */
/* Allocates a 3D display and returns its MIL identifier.         */
/* -------------------------------------------------------------- */
void CDisplay::Alloc3dDisplayId()
   {
   // Try to allocate a 3d display.
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   m_MilDisplay = M3ddispAlloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"),
                               M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(m_MilDisplay == M_NULL)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("A 2D display will be used instead.\n"));

      // Allocate a 2d display instead.
      m_MilDisplay = MdispAlloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"),
                                M_DEFAULT, M_NULL);
      m_Lut = MbufAllocColor(m_MilSystem, 3, 256, 1, M_UNSIGNED + 8, M_LUT, M_NULL);
      MgenLutFunction(m_Lut, M_COLORMAP_TURBO + M_FLIP, M_DEFAULT, M_DEFAULT, M_DEFAULT,
                      M_DEFAULT, M_DEFAULT, M_DEFAULT);
      }
   MobjInquire(m_MilDisplay, M_OBJECT_TYPE, &m_DisplayType);
   GetGraphicListId();
   }
/* ----------------------------------------------- */
/* Sets the window size.                           */
/* ----------------------------------------------- */
void  CDisplay::Size(MIL_INT SizeX, MIL_INT SizeY)
   {
   if(m_DisplayType == M_3D_DISPLAY)
      {
      M3ddispControl(m_MilDisplay, M_SIZE_X, SizeX);
      M3ddispControl(m_MilDisplay, M_SIZE_Y, SizeY);
      }
   else
      {
      m_MilDepthMap  = MbufAlloc2d   (m_MilSystem,    SizeX, SizeY, M_UNSIGNED + 8,
                                      M_IMAGE | M_PROC | M_DISP, M_NULL);
      m_IntensityMap = MbufAllocColor(m_MilSystem, 3, SizeX, SizeY, M_UNSIGNED + 8,
                                      M_IMAGE | M_PROC | M_DISP, M_NULL);
      }
   }
/* ----------------------------------------------- */
/* Sets the window position x.                     */
/* ----------------------------------------------- */
void  CDisplay::PositionX(MIL_INT PositionX)
   {
   if(m_DisplayType == M_3D_DISPLAY)
      {
      M3ddispControl(m_MilDisplay, M_WINDOW_INITIAL_POSITION_X, PositionX);
      }
   else
      {
      MdispControl(m_MilDisplay, M_WINDOW_INITIAL_POSITION_X, PositionX);
      }
   }
/* -------------------------------------------------------------- */
/* Gets the display's graphic list, or allocates a standalone one.*/
/* -------------------------------------------------------------- */
void CDisplay::GetGraphicListId()
   {
   if(m_DisplayType == M_3D_DISPLAY)
      {
      m_MilGraphicList=(MIL_ID)M3ddispInquire(m_MilDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
      }
   else // M_DISPLAY
      {
      // Associate a graphic list.
      m_MilGraphicList = MgraAllocList(m_MilSystem, M_DEFAULT, M_NULL);
      MdispControl(m_MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, m_MilGraphicList);
      }
   }
/* -------------------------------------------------------------- */
/* Free the display.                                              */
/* -------------------------------------------------------------- */
void CDisplay::FreeDisplay()
   {
   if(m_DisplayType == M_DISPLAY)
      {
      MdispFree(m_MilDisplay);
      MbufFree(m_Lut);
      MbufFree(m_MilDepthMap);
      MbufFree(m_IntensityMap);
      MgraFree(m_MilGraphicList);
      }
   else
      {
      M3ddispFree(m_MilDisplay);
      }
   }
/* -------------------------------------------------------------- */
// Displays the container in the 3D or 2D display.
/* -------------------------------------------------------------- */
void CDisplay::DisplayContainer(MIL_ID MilContainer, bool UseLut)
   {

   if(m_DisplayType == M_3D_DISPLAY)
      {
      MIL_INT64 Label = M3ddispSelect(m_MilDisplay, MilContainer, M_DEFAULT, M_DEFAULT);
      if(UseLut)
         {
         M3dgraCopy(M_COLORMAP_TURBO + M_FLIP, M_DEFAULT, m_MilGraphicList, Label,
                    M_COLOR_LUT, M_DEFAULT);
         M3dgraControl(m_MilGraphicList, Label, M_COLOR_USE_LUT, M_TRUE);
         M3dgraControl(m_MilGraphicList, Label, M_COLOR_COMPONENT_BAND, 2);
         M3dgraControl(m_MilGraphicList, Label, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
         }
      }
   else // M_DISPLAY
      {
      // Project into a depthmap.
      M3dimCalibrateDepthMap(MilContainer, m_MilDepthMap, m_IntensityMap, M_NULL, M_DEFAULT,
                              M_DEFAULT, M_CENTER);
      
      if(UseLut)
         {
         // Associate a LUT.
         MbufControl(m_MilDepthMap, M_ASSOCIATED_LUT, m_Lut);
         M3dimProject(MilContainer, m_MilDepthMap, M_NULL, M_POINT_BASED, M_MAX_Z, M_DEFAULT,
                      M_DEFAULT);
         MdispSelect(m_MilDisplay, m_MilDepthMap);
         }
      else
         {
         bool HasColor = MbufInquireContainer(MilContainer, M_COMPONENT_REFLECTANCE,
                                              M_COMPONENT_ID, M_NULL) != M_NULL ||
                         MbufInquireContainer(MilContainer, M_COMPONENT_INTENSITY  ,
                                               M_COMPONENT_ID, M_NULL) !=M_NULL;

         if(HasColor)
            {
            M3dimProject(MilContainer, m_MilDepthMap, m_IntensityMap, M_POINT_BASED, M_MAX_Z,
                         M_DEFAULT, M_DEFAULT);
            MdispSelect(m_MilDisplay, m_IntensityMap);
            }
         else
            {
            M3dimProject(MilContainer, m_MilDepthMap, M_NULL, M_POINT_BASED, M_MAX_Z,
                         M_DEFAULT, M_DEFAULT);
            MdispSelect(m_MilDisplay, m_MilDepthMap);
            }
         }     
      }
   }
/* -------------------------------------------------------------- */
/* Updates the displayed image.                                   */
/* -------------------------------------------------------------- */
void CDisplay::UpdateDisplay(MIL_ID MilContainer, bool UseLut)
{
   if(m_DisplayType == M_3D_DISPLAY)
   {
      return; // Containers are updated automatically in the 3D display
   }
   else
   {
      DisplayContainer(MilContainer, UseLut);
   }
}
/*--------------------------------------------------------------- */
/*  Set the 3D disply view .                                      */
/* -------------------------------------------------------------- */
void CDisplay::SetView(MIL_INT64 Mode, MIL_DOUBLE Param1, MIL_DOUBLE Param2, MIL_DOUBLE Param3)
   {
   if(m_DisplayType == M_3D_DISPLAY)
      M3ddispSetView(m_MilDisplay, Mode, Param1, Param2, Param3, M_DEFAULT);
   }
void CDisplay::Title(MIL_STRING Title)
   {
   if(m_DisplayType == M_3D_DISPLAY)
      M3ddispControl(m_MilDisplay, M_TITLE, Title);
   else
      MdispControl(m_MilDisplay, M_TITLE, Title);
   }
/*-------------------------------------------------------------- */
/* Draw the 3d model occurrences found.                          */
/* ------------------------------------------------------------- */
MIL_INT64 CDisplay::Draw(MIL_ID MilResult)
   {
   if(m_DisplayType == M_3D_DISPLAY)
   {
     return M3dmodDraw3d(M_DEFAULT, MilResult, M_ALL,
                   m_MilGraphicList, M_DEFAULT, M_DEFAULT);
   }
   else      
   {
      auto Mil3dGraphicList  = M3dgraAlloc(m_MilSystem, M_DEFAULT, M_UNIQUE_ID);
      M3dmodDraw3d(M_DEFAULT, MilResult, M_ALL,
                   Mil3dGraphicList, M_DEFAULT, M_DEFAULT);
      // Clear the graphic list.
      MgraControlList(m_MilGraphicList, M_ALL, M_DEFAULT, M_DELETE, M_DEFAULT);

      // Get all 3d graphics.
      std::vector<MIL_DOUBLE> PointsX, PointsY;
      std::vector<MIL_INT64> Labels;
      M3dgraInquire(Mil3dGraphicList, M_ROOT_NODE, M_CHILDREN + M_RECURSIVE, Labels);

      MIL_ID Matrix = M3dgeoAlloc(m_MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_NULL);

      auto MilContainer = MbufAllocContainer(m_MilSystem, M_PROC|M_DISP, M_DEFAULT,
                                             M_UNIQUE_ID);
      // Draw all 3d boxes and dots in the 2d display.
      for(MIL_UINT i = 0; i < Labels.size(); i++)
      {
         MIL_INT64 GraphicType = M3dgraInquire(Mil3dGraphicList, Labels[i], M_GRAPHIC_TYPE,
                                               M_NULL);

         if(GraphicType == M_GRAPHIC_TYPE_DOTS)
         { // Dots.
            MIL_INT64 Color = M3dgraInquire(Mil3dGraphicList, Labels[i], M_COLOR, M_NULL);
            M3dgraInquire(Mil3dGraphicList, Labels[i], M_POINTS_X, PointsX);
            M3dgraInquire(Mil3dGraphicList, Labels[i], M_POINTS_Y, PointsY);

            MgraControl(M_DEFAULT, M_COLOR, (MIL_DOUBLE)Color);
            MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
            MgraDots(M_DEFAULT, m_MilGraphicList, M_DEFAULT, PointsX, PointsY, M_DEFAULT);
         }
         else if(GraphicType == M_GRAPHIC_TYPE_BOX)
         { // Boxes.
            MIL_DOUBLE RotX, RotY, RotZ, CenterX, CenterY, SizeX, SizeY;
            M3dgraInquire(Mil3dGraphicList, Labels[i], M_CENTER_X, &CenterX);
            M3dgraInquire(Mil3dGraphicList, Labels[i], M_CENTER_Y, &CenterY);
            M3dgraInquire(Mil3dGraphicList, Labels[i], M_SIZE_X, &SizeX);
            M3dgraInquire(Mil3dGraphicList, Labels[i], M_SIZE_Y, &SizeY);
            M3dgraCopy(Mil3dGraphicList, Labels[i], Matrix, M_DEFAULT, M_TRANSFORMATION_MATRIX,
                       M_DEFAULT);
            M3dgeoMatrixGetTransform(Matrix, M_ROTATION_ZXY, &RotZ, &RotY, &RotX, M_NULL,
                                     M_DEFAULT);

            MgraControl(M_DEFAULT, M_COLOR, M_COLOR_WHITE);
            MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
            MgraRectAngle(M_DEFAULT, m_MilGraphicList, CenterX, CenterY, SizeX, SizeY, -RotZ,
                          M_CENTER_AND_DIMENSION);
         }
      }

      M3dgeoFree(Matrix);
   }
   return 0;
   }
void CDisplay::Clear(MIL_INT64 Label)
   {
   if(m_DisplayType == M_3D_DISPLAY)
      M3dgraRemove(m_MilGraphicList, Label, M_DEFAULT);
   else
      {
      MgraControlList(m_MilGraphicList, M_ALL, M_DEFAULT, M_DELETE, M_DEFAULT);
      }
   }
