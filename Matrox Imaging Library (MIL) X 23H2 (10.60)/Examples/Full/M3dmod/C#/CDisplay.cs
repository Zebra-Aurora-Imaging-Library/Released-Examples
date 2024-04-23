/******************************************************************************/
/*
/* File name: CDisplay.cpp
/*
/* Synopsis:  Class that manages the 2D/3D mil displays for 3D  
/*            examples.
/*
/* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
/* All Rights Reserved
/******************************************************************************/
//
using System;
using System.Collections.Generic;
using System.Linq;
using Matrox.MatroxImagingLibrary;

namespace M3dmod
    {
    internal class CDisplay
        {
        public CDisplay(MIL_ID MilSystem)
            {
            m_MilSystem = MilSystem;
            }
        /* -------------------------------------------------------------- */
        /* Allocates a 3D display and returns its MIL identifier.         */
        /* -------------------------------------------------------------- */
        public void Alloc3dDisplayId()
            {
            // Try to allocate a 3d display.
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);
            m_MilDisplay = MIL.M3ddispAlloc(m_MilSystem, MIL.M_DEFAULT, "M_DEFAULT",
                           MIL.M_DEFAULT, MIL.M_NULL);
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);

            if (m_MilDisplay == MIL.M_NULL)
                {
                Console.WriteLine();
                Console.WriteLine("The current system does not support the 3D display.");
                Console.WriteLine("A 2D display will be used instead.");
                
                // Allocate a 2d display instead.
                m_MilDisplay = MIL.MdispAlloc(m_MilSystem, MIL.M_DEFAULT, "M_DEFAULT",
                                              MIL.M_DEFAULT, MIL.M_NULL);
                m_Lut        = MIL.MbufAllocColor(m_MilSystem, 3, 256, 1, MIL.M_UNSIGNED + 8,
                                                  MIL.M_LUT, MIL.M_NULL);
                MIL.MgenLutFunction(m_Lut, MIL.M_COLORMAP_TURBO + MIL.M_FLIP, MIL.M_DEFAULT,
                                    MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                                    MIL.M_DEFAULT);
                }
            MIL.MobjInquire(m_MilDisplay, MIL.M_OBJECT_TYPE,  ref m_DisplayType);
            GetGraphicListId();
            }
        /* -------------------------------------------------------------- */
        /* Sets the window size.                                          */
        /* -------------------------------------------------------------- */
        public void Size(MIL_INT SizeX, MIL_INT SizeY)
            {
            if (m_DisplayType == MIL.M_3D_DISPLAY)
                {
                MIL.M3ddispControl(m_MilDisplay, MIL.M_SIZE_X, SizeX);
                MIL.M3ddispControl(m_MilDisplay, MIL.M_SIZE_Y, SizeY);
                }
            else
                {
                m_MilDepthMap  = MIL.MbufAlloc2d(m_MilSystem, SizeX, SizeY, MIL.M_UNSIGNED + 8,
                                                 MIL.M_IMAGE | MIL.M_PROC | MIL.M_DISP,
                                                 MIL.M_NULL);
                m_IntensityMap = MIL.MbufAllocColor(m_MilSystem, 3, SizeX, SizeY,
                                                    MIL.M_UNSIGNED + 8, MIL.M_IMAGE |
                                                    MIL.M_PROC | MIL.M_DISP, MIL.M_NULL);
                }
            }
        /* ----------------------------------------------- */
        /* Sets the window position x.                     */
        /* ----------------------------------------------- */
        public void PositionX(MIL_INT PositionX)
            {
            if (m_DisplayType == MIL.M_3D_DISPLAY)
                {
                MIL.M3ddispControl(m_MilDisplay, MIL.M_WINDOW_INITIAL_POSITION_X, PositionX);
                }
            else
                {
                MIL.MdispControl(m_MilDisplay, MIL.M_WINDOW_INITIAL_POSITION_X, PositionX);
                }
            }
        /* -------------------------------------------------------------- */
        /* Displays the container in the 3D or 2D display.                */
        /* -------------------------------------------------------------- */
        public void DisplayContainer(MIL_ID MilContainer, bool UseLut)
            {

            if (m_DisplayType == MIL.M_3D_DISPLAY)
                {
                MIL_INT Label = MIL.M3ddispSelect(m_MilDisplay, MilContainer, MIL.M_DEFAULT,
                                                  MIL.M_DEFAULT);
                if (UseLut)
                    {
                    MIL.M3dgraCopy(MIL.M_COLORMAP_TURBO + MIL.M_FLIP, MIL.M_DEFAULT,
                                   m_MilGraphicList, Label, MIL.M_COLOR_LUT, MIL.M_DEFAULT);
                    MIL.M3dgraControl(m_MilGraphicList, Label, MIL.M_COLOR_USE_LUT,
                                      MIL.M_TRUE);
                    MIL.M3dgraControl(m_MilGraphicList, Label, MIL.M_COLOR_COMPONENT_BAND, 2);
                    MIL.M3dgraControl(m_MilGraphicList, Label, MIL.M_COLOR_COMPONENT,
                                      MIL.M_COMPONENT_RANGE);
                    }
                }
            else // M_DISPLAY
                {
                // Project into a depthmap.
                MIL.M3dimCalibrateDepthMap(MilContainer, m_MilDepthMap, m_IntensityMap,
                                           MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT,
                                           MIL.M_CENTER);

                if (UseLut)
                    {
                    // Associate a LUT.
                    MIL.MbufControl(m_MilDepthMap, MIL.M_ASSOCIATED_LUT, m_Lut);
                    MIL.M3dimProject(MilContainer, m_MilDepthMap, MIL.M_NULL,
                                     MIL.M_POINT_BASED, MIL.M_MAX_Z, MIL.M_DEFAULT,
                                     MIL.M_DEFAULT);
                    MIL.MdispSelect(m_MilDisplay, m_MilDepthMap);
                    }
                else
                    {
                    bool HasColor = MIL.MbufInquireContainer(MilContainer,
                                    MIL.M_COMPONENT_REFLECTANCE, MIL.M_COMPONENT_ID,
                                    MIL.M_NULL) != MIL.M_NULL                     ||
                                    MIL.MbufInquireContainer(MilContainer,
                                    MIL.M_COMPONENT_INTENSITY, MIL.M_COMPONENT_ID, MIL.M_NULL)
                                    != MIL.M_NULL;

                    if (HasColor)
                        {
                        MIL.M3dimProject(MilContainer, m_MilDepthMap, m_IntensityMap,
                                       MIL.M_POINT_BASED, MIL.M_MAX_Z, MIL.M_DEFAULT,
                                       MIL.M_DEFAULT);
                        MIL.MdispSelect(m_MilDisplay, m_IntensityMap);
                        }
                    else
                        {
                        MIL.M3dimProject(MilContainer, m_MilDepthMap, MIL.M_NULL,
                                       MIL.M_POINT_BASED, MIL.M_MAX_Z, MIL.M_DEFAULT,
                                       MIL.M_DEFAULT);
                        MIL.MdispSelect(m_MilDisplay, m_MilDepthMap);
                        }
                    }
                }
            }
        public void Title(string Title)
            {
            if (m_DisplayType == MIL.M_3D_DISPLAY)
                { MIL.M3ddispControl(m_MilDisplay, MIL.M_TITLE, Title); }
            else
                { MIL.MdispControl(m_MilDisplay, MIL.M_TITLE, Title); }
            }
        /*--------------------------------------------------------------- * /
        /*  Set the 3D disply view .                                      */
        /* -------------------------------------------------------------- */
        public void SetView(MIL_INT Mode, double Param1, double Param2, double Param3)
            {
            if (m_DisplayType == MIL.M_3D_DISPLAY)
                MIL.M3ddispSetView(m_MilDisplay, Mode, Param1, Param2, Param3, MIL.M_DEFAULT);
            }
        /*-------------------------------------------------------------- */
        /* Draw the 3d model occurrences found.                          */
        /* ------------------------------------------------------------- */
        public MIL_INT Draw(MIL_ID MilResult)
            {
            if (m_DisplayType == MIL.M_3D_DISPLAY)
                {
                return MIL.M3dmodDraw3d(MIL.M_DEFAULT, MilResult, MIL.M_ALL,
                              m_MilGraphicList, MIL.M_DEFAULT, MIL.M_DEFAULT);
                }
            else
                {
                MIL_ID Mil3dGraphicList  = MIL.M3dgraAlloc(m_MilSystem, MIL.M_DEFAULT,
                                                           MIL.M_NULL);
                MIL.M3dmodDraw3d(MIL.M_DEFAULT, MilResult, MIL.M_ALL,
                                 Mil3dGraphicList, MIL.M_DEFAULT, MIL.M_DEFAULT);
                // Clear the graphic list.
                MIL.MgraControlList(m_MilGraphicList, MIL.M_ALL, MIL.M_DEFAULT, MIL.M_DELETE,
                                    MIL.M_DEFAULT);

                // Get all 3d graphics.
                IList<double> PointsX, PointsY;
                IList<MIL_INT> Labels;
                MIL.M3dgraInquire(Mil3dGraphicList, MIL.M_ROOT_NODE, MIL.M_CHILDREN +
                                  MIL.M_RECURSIVE,out Labels);

                MIL_ID Matrix = MIL.M3dgeoAlloc(m_MilSystem, MIL.M_TRANSFORMATION_MATRIX,
                                                MIL.M_DEFAULT, MIL.M_NULL);

                MIL_ID  MilContainer = MIL.MbufAllocContainer(m_MilSystem, MIL.M_PROC|
                                                              MIL.M_DISP, MIL.M_DEFAULT,
                                                              MIL.M_NULL);
                // Draw all 3d boxes and dots in the 2d display.
                for (int i = 0; i < Labels.Count(); i++)
                    {
                    MIL_INT GraphicType = MIL.M3dgraInquire(Mil3dGraphicList, Labels[i],
                                                            MIL.M_GRAPHIC_TYPE, MIL.M_NULL);

                    if (GraphicType == MIL.M_GRAPHIC_TYPE_DOTS)
                        { // Dots.
                        MIL_INT Color = MIL.M3dgraInquire(Mil3dGraphicList, Labels[i],
                                                          MIL.M_COLOR, MIL.M_NULL);
                        MIL.M3dgraInquire(Mil3dGraphicList, Labels[i], MIL.M_POINTS_X,
                                          out PointsX);
                        MIL.M3dgraInquire(Mil3dGraphicList, Labels[i], MIL.M_POINTS_Y,
                                          out PointsY);

                        MIL.MgraControl(MIL.M_DEFAULT, MIL.M_COLOR, (double)Color);
                        MIL.MgraControl(MIL.M_DEFAULT, MIL.M_INPUT_UNITS, MIL.M_WORLD);
                        MIL.MgraDots(MIL.M_DEFAULT, m_MilGraphicList, MIL.M_DEFAULT, PointsX,
                                     PointsY, MIL.M_DEFAULT);
                        }
                    else if (GraphicType == MIL.M_GRAPHIC_TYPE_BOX)
                        { // Boxes.
                        double RotX = 0.0, RotY=0.0, RotZ=0.0, CenterX=0.0, CenterY=0.0,
                               SizeX=0.0, SizeY=0.0;
                        MIL.M3dgraInquire(Mil3dGraphicList, Labels[i], MIL.M_CENTER_X,
                                          ref CenterX);
                        MIL.M3dgraInquire(Mil3dGraphicList, Labels[i], MIL.M_CENTER_Y,
                                          ref CenterY);
                        MIL.M3dgraInquire(Mil3dGraphicList, Labels[i], MIL.M_SIZE_X  ,
                                          ref SizeX);
                        MIL.M3dgraInquire(Mil3dGraphicList, Labels[i], MIL.M_SIZE_Y  ,
                                          ref SizeY);
                        MIL.M3dgraCopy(Mil3dGraphicList, Labels[i], Matrix, MIL.M_DEFAULT,
                                       MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT);
                        MIL.M3dgeoMatrixGetTransform(Matrix, MIL.M_ROTATION_ZXY, ref RotZ,
                                                     ref RotY,  ref RotX, MIL.M_NULL,
                                                     MIL.M_DEFAULT);

                        MIL.MgraControl(MIL.M_DEFAULT, MIL.M_COLOR, MIL.M_COLOR_WHITE);
                        MIL.MgraControl(MIL.M_DEFAULT, MIL.M_INPUT_UNITS, MIL.M_WORLD);
                        MIL.MgraRectAngle(MIL.M_DEFAULT, m_MilGraphicList, CenterX, CenterY,
                                          SizeX, SizeY, -RotZ, MIL.M_CENTER_AND_DIMENSION);
                        }
                    }

                MIL.M3dgeoFree(Matrix);
                MIL.M3dgraFree(Mil3dGraphicList);
                MIL.MbufFree(MilContainer);
                }
            return 0;
            }
        /* -------------------------------------------------------------- */
        /* Updates the displayed image.                                   */
        /* -------------------------------------------------------------- */
        public void UpdateDisplay(MIL_ID MilContainer, bool UseLut)
            {
            if (m_DisplayType == MIL.M_3D_DISPLAY)
                {
                return; // Containers are updated automatically in the 3D display
                }
            else
                {
                DisplayContainer(MilContainer, UseLut);
                }
            }
        public void Clear(MIL_INT Label)
            {
            if (m_DisplayType == MIL.M_3D_DISPLAY)
                MIL.M3dgraRemove(m_MilGraphicList, Label, MIL.M_DEFAULT);
            else
                {
                MIL.MgraControlList(m_MilGraphicList, MIL.M_ALL, MIL.M_DEFAULT, MIL.M_DELETE,
                                    MIL.M_DEFAULT);
                }
            }
        /* -------------------------------------------------------------- */
        /* Free the display.                                              */
        /* -------------------------------------------------------------- */
        public void FreeDisplay()
            {
            if (m_DisplayType == MIL.M_DISPLAY)
                {
                MIL.MdispFree(m_MilDisplay);
                MIL.MbufFree(m_Lut);
                MIL.MbufFree(m_MilDepthMap);
                MIL.MbufFree(m_IntensityMap);
                MIL.MgraFree(m_MilGraphicList);
                }
            else
                {
                MIL.M3ddispFree(m_MilDisplay);
                }
            }

        private
      /* -------------------------------------------------------------- */
      /* Gets the display's graphic list, or allocates a standalone one.*/
      /* -------------------------------------------------------------- */
      void GetGraphicListId()
            {
            if (m_DisplayType == MIL.M_3D_DISPLAY)
                {
                m_MilGraphicList = (MIL_ID)MIL.M3ddispInquire(m_MilDisplay,
                                   MIL.M_3D_GRAPHIC_LIST_ID, MIL.M_NULL);
                }
            else // M_DISPLAY
                {
                // Associate a graphic list.
                m_MilGraphicList = MIL.MgraAllocList(m_MilSystem, MIL.M_DEFAULT, MIL.M_NULL);
                MIL.MdispControl(m_MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID,
                                 m_MilGraphicList);
                }
            }

        MIL_ID    m_MilSystem      = MIL.M_NULL;
        MIL_ID    m_MilDisplay     = MIL.M_NULL;
        MIL_ID    m_MilGraphicList = MIL.M_NULL;
        MIL_INT   m_DisplayType    = MIL.M_NULL;
        MIL_ID    m_Lut            = MIL.M_NULL;
        MIL_ID    m_MilDepthMap    = MIL.M_NULL;
        MIL_ID    m_IntensityMap   = MIL.M_NULL;

        }
    }
