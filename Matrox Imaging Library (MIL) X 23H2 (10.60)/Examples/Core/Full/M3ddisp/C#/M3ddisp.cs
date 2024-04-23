﻿//***************************************************************************************
// 
// File name: M3ddisp.cs 
//
// Synopsis: This program contains an example of how to use 3D displays in mil.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************

using System;
using System.Threading;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using Matrox.MatroxImagingLibrary;

namespace M3ddisp
    {
    class Program
        {

        //*****************************************************************************
        // Constants.
        //*****************************************************************************
        private const string BACKGROUND_IMAGE_FILE = MIL.M_IMAGE_PATH + "imaginglogo.mim";
        private const double PI = 3.14159265358979323846;

        //****************************************************************************
        // Example description.
        //****************************************************************************

        static void PrintHeader()
            {
            Console.Write("[EXAMPLE NAME]\n");
            Console.Write("M3ddisp\n\n");

            Console.Write("[SYNOPSIS]\n");
            Console.Write("This example demonstrates how to use MIL 3D displays.\n\n");

            Console.Write("[MODULES USED]\n");
            Console.Write("Modules used: application, system, buffer, 3D display, 3D graphics.\n");

            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();
            }


        //*****************************************************************************
        // Main.
        //*****************************************************************************
        static void Main(string[] args)
            {
            MIL_ID MilApplication, MilSystem, MilDisplay3d, MilContainerId1, MilContainerId2;
            MIL_ID MatrixTranslate1, MatrixTranslate2, InitialViewpointMatrix, MatrixGrid;
            long GridLabel, AxisLabel, ContainerLabel1, ContainerLabel2;
            double AxisLength;

            // Print Header. 
            PrintHeader();

            // Allocate the MIL application.
            MilApplication = MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT, MIL.M_NULL);

            // Allocate the MIL system.
            MilSystem = MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_HOST, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_NULL);

            // Allocate the MIL 3D Display.
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);
            MilDisplay3d = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, MIL.M_NULL);
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);

            // Make sure we meet the minimum requirements for the 3d display.
            if (MilDisplay3d == 0)
                {
                Console.Write("The current system does not support the 3D display.\n");
                Console.Write("Press <ENTER> to end.\n");
                Console.ReadKey();
                return;
                }

            // Show the display.
            MIL_ID MilGraphicList3d = MIL.M_NULL;
            MIL.M3ddispInquire(MilDisplay3d, MIL.M_3D_GRAPHIC_LIST_ID, ref MilGraphicList3d);

            //                                                  X,     Y,     Z
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEWPOINT, 100.0, 75.0, 75.0, MIL.M_DEFAULT);
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_UP_VECTOR, 0.0, 0.0, 1.0, MIL.M_DEFAULT);
            MIL.M3ddispSelect(MilDisplay3d,  MIL.M_NULL, MIL.M_OPEN, MIL.M_DEFAULT);

            // Draw an axis and a grid.
            Console.Write("MIL 3D displays can be used with 0, 1 or many point clouds.\n");
            Console.Write("This allows you to show only the content of the display's graphics list.\n");
            Console.Write("In this case, an axis and a grid are shown.\n\n");

            AxisLength = 15.0;
            AxisLabel = MIL.M3dgraAxis(MilGraphicList3d, MIL.M_ROOT_NODE, MIL.M_DEFAULT, AxisLength, "", MIL.M_DEFAULT, MIL.M_DEFAULT);

            // Draw a grid to be displayed.
            MatrixGrid = MIL.M3dgeoAlloc(MilSystem, MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dgeoMatrixSetTransform(MatrixGrid, MIL.M_TRANSLATION, AxisLength, AxisLength * 1.5, 0.0, MIL.M_DEFAULT, MIL.M_DEFAULT);
            GridLabel = MIL.M3dgraGrid(MilGraphicList3d, AxisLabel, MIL.M_SIZE_AND_SPACING, MatrixGrid, AxisLength * 2, AxisLength * 3, 5.0, 5.0, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, GridLabel, MIL.M_FILL_COLOR, MIL.M_COLOR_WHITE);
            MIL.M3dgraControl(MilGraphicList3d, GridLabel, MIL.M_COLOR, MIL.M_COLOR_BLACK);
            MIL.M3dgraControl(MilGraphicList3d, GridLabel, MIL.M_OPACITY, 20);

            // Translate both point clouds.
            MatrixTranslate1 = MIL.M3dgeoAlloc(MilSystem,  MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dgeoMatrixSetTransform(MatrixTranslate1, MIL.M_TRANSLATION, AxisLength, AxisLength * 2.2, 0.0,  MIL.M_DEFAULT, MIL.M_DEFAULT);
            MatrixTranslate2 = MIL.M3dgeoAlloc(MilSystem,  MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dgeoMatrixSetTransform(MatrixTranslate2, MIL.M_TRANSLATION, AxisLength, AxisLength * 0.75, 0.0, MIL.M_DEFAULT, MIL.M_DEFAULT);

            // Save viewpoint.
            InitialViewpointMatrix = MIL.M3dgeoAlloc(MilSystem, MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT, MIL.M_NULL);

            Console.Write("Use the mouse to set the 3D view in the display.\n");
            Console.Write("   - Left click and drag   : Orbits around the interest point.\n");
            Console.Write("   - Right click and drag  : Translates in the screen's plane.\n");
            Console.Write("   - Middle click and drag : Rolls.\n");
            Console.Write("   - Mouse wheel           : Zooms in, Zooms out.\n");
            Console.Write("\n");
            Console.Write("The resulting 3D view will be stored in a matrix using M3ddispCopy\n");
            Console.Write("and will be reused later.\n\n");
            Console.Write("Press <Enter> to copy the current 3D view and continue.\n");
            Console.ReadKey();
            MIL.M3ddispCopy(MilDisplay3d, InitialViewpointMatrix, MIL.M_VIEW_MATRIX, MIL.M_DEFAULT);

            Console.Write("Two point clouds have been added using M3ddispSelect.\n\n");

            MIL.M3ddispControl(MilDisplay3d, MIL.M_UPDATE, MIL.M_DISABLE);

            // Generate a first meshed point cloud container.
            MilContainerId1 = Generate3DContainer(MilSystem);

            // Clone the first to a second container.
            MilContainerId2 = MIL.MbufClone(MilContainerId1, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_COPY_SOURCE_DATA, MIL.M_NULL);

            // Select the containers to the 3D display and keep their corresponding labels in the graphics list of the 3D display.
            ContainerLabel1 = MIL.M3ddispSelect(MilDisplay3d, MilContainerId1, MIL.M_ADD, MIL.M_DEFAULT);
            ContainerLabel2 = MIL.M3ddispSelect(MilDisplay3d, MilContainerId2, MIL.M_ADD, MIL.M_DEFAULT);

            // Move the second container.
            MIL.M3dgraCopy(MatrixTranslate1, MIL.M_DEFAULT, MilGraphicList3d, ContainerLabel1, MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT);
            MIL.M3dgraCopy(MatrixTranslate2, MIL.M_DEFAULT, MilGraphicList3d, ContainerLabel2, MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT);

            MIL.M3ddispControl(MilDisplay3d, MIL.M_UPDATE, MIL.M_ENABLE);

            // Setting the viewpoint.
            Console.Write("Many options exist to change the display's viewpoint.\n");
            Console.Write("Press <Enter> to set the viewpoint, interest point and up vector.\n\n");
            Console.ReadKey();
            MIL.M3ddispControl(MilDisplay3d, MIL.M_UPDATE, MIL.M_DISABLE);
            //                                                            X,                Y,                 Z
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEWPOINT, AxisLength * 3.0, AxisLength, AxisLength * 10.0, MIL.M_DEFAULT);
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_INTEREST_POINT, AxisLength, AxisLength, 0.0, MIL.M_DEFAULT);
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_UP_VECTOR, -1.0, 0.0, 0.0, MIL.M_DEFAULT);

            MIL.M3ddispControl(MilDisplay3d, MIL.M_UPDATE, MIL.M_ENABLE);

            // Show the options to move the view.
            Console.Write("The view parameters can be either specific values or values composed\n");
            Console.Write("with the current 3D view.\n");
            Console.Write("Different options will be shown:\n");
            Console.Write(" -Move the viewpoint, relative to its current position, while keeping\n");
            Console.Write("  the interest point constant.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            
            for (int i = 0; (i < 100); i++)
                {
                Thread.Sleep(15);
                double Sign = (i < 50) ? 1.0 : -1.0;
                MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEWPOINT + MIL.M_COMPOSE_WITH_CURRENT, 0.0, Sign * 3.0, 0.0, MIL.M_DEFAULT);
                }

            // Show the effect of moving only interest point.
            Console.Write(" -Move the interest point while keeping the viewpoint constant.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            
            for (int i = 0; (i < 100); i++)
                {
                Thread.Sleep(15);
                double Sign = (i < 50) ? 1.0 : -1.0;
                MIL.M3ddispSetView(MilDisplay3d, MIL.M_INTEREST_POINT + MIL.M_COMPOSE_WITH_CURRENT, 0.0, Sign * 0.5, 0.0, MIL.M_DEFAULT);
                }

            // Reset the point of view.
            MIL.M3ddispControl(MilDisplay3d, MIL.M_UPDATE, MIL.M_DISABLE);
            //                                                            X,                Y,                 Z
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEWPOINT,      AxisLength * 3.0, AxisLength, AxisLength * 10.0, MIL.M_DEFAULT);
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_INTEREST_POINT, AxisLength,       AxisLength,               0.0, MIL.M_DEFAULT);
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_UP_VECTOR,            -1.0,              0.0,               0.0, MIL.M_DEFAULT);
            MIL.M3ddispControl(MilDisplay3d, MIL.M_UPDATE, MIL.M_ENABLE);

            // Rotate up vector.
            Console.Write(" -Modify the up vector (the same can be done by modifying the roll value).\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            
            for (int i = 0; (i <= 100); i++)
                {
                Thread.Sleep(15);
                MIL.M3ddispSetView(MilDisplay3d, MIL.M_UP_VECTOR, Math.Cos((2.0 * PI * i / 100.0) + PI), Math.Sin((2.0 * PI * i / 100.0)), 0.0, MIL.M_DEFAULT);
                }

            // Translate the viewpoint and interestpoint.
            Console.Write(" -Translate both the view and interest point.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            
            for (int i = 0; (i < 50); i++)
                {
                Thread.Sleep(15);
                MIL.M3ddispSetView(MilDisplay3d, MIL.M_TRANSLATE, 0.0, 1.0, 0.0, MIL.M_DEFAULT);
                }
            for (int i = 0; (i < 50); i++)
                {
                Thread.Sleep(15);
                MIL.M3ddispSetView(MilDisplay3d, MIL.M_TRANSLATE, 0.0, -0.95, 0.05, MIL.M_DEFAULT);
                }

            // Zoom.
            Console.Write(" -Zoom in and out.        \n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            
            for (int i = 0; (i < 100); i++)
                {
                Thread.Sleep(15);
                double Zoom = (i < 50) ? 1.01 : 0.99;
                MIL.M3ddispSetView(MilDisplay3d, MIL.M_ZOOM, Zoom, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
                }

            // Azimuth and Elevation.
            Console.Write(" -Modify the azimuth and the elevation.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            
            for (int i = 0; (i < 50); i++)
                {
                Thread.Sleep(15);
                MIL.M3ddispSetView(MilDisplay3d, MIL.M_ELEVATION + MIL.M_COMPOSE_WITH_CURRENT, Math.Cos((PI * i / 50.0) + PI), MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
                }
            for (int i = 0; (i < 50); i++)
                {
                Thread.Sleep(15);
                MIL.M3ddispSetView(MilDisplay3d, MIL.M_AZIMUTH + MIL.M_COMPOSE_WITH_CURRENT, Math.Cos((PI * i / 50.0) + PI), MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
                }

            // Specific node.
            Console.Write(" -Set the view to something that includes graphics of a specific node.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();

            MIL.M3ddispSetView(MilDisplay3d, MIL.M_AUTO, MIL.M_DEFAULT, ContainerLabel1, MIL.M_DEFAULT, MIL.M_DEFAULT);

            // Viewbox.
            Console.Write(" -Set the view to something that includes everything in the scene.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();

            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEW_BOX, MIL.M_WHOLE_SCENE, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            // Predefined orientations.
            Console.Write(" -Set the view to a view from the top.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEW_ORIENTATION, MIL.M_TOP_VIEW, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            Console.Write(" -Set the view to a view from below.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEW_ORIENTATION, MIL.M_BOTTOM_VIEW, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            Console.Write(" -Set the view to a view from the side.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEW_ORIENTATION, MIL.M_LEFT_VIEW, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            Console.Write(" -Set the view to an angled view from the top.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEW_ORIENTATION, MIL.M_TOP_TILTED, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            Console.Write(" -Set the view to an angled view from below.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEW_ORIENTATION, MIL.M_BOTTOM_TILTED, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            // Restore copied view.
            Console.Write("Press <Enter> to restore the previously copied view.\n\n");
            Console.ReadKey();
            MIL.M3ddispCopy(InitialViewpointMatrix, MilDisplay3d, MIL.M_VIEW_MATRIX, MIL.M_DEFAULT);

            Console.Write("The display's background color can be set to a solid color,\n");
            Console.Write("a gradient, or an image.\n");
            Console.Write("Press <Enter> to change the background color.\n\n");
            Console.ReadKey();

            MIL.M3ddispControl(MilDisplay3d, MIL.M_BACKGROUND_MODE, MIL.M_SINGLE_COLOR);
            MIL.M3ddispControl(MilDisplay3d, MIL.M_BACKGROUND_COLOR, MIL.M_RGB888(50, 150, 125));

            Console.Write("Press <Enter> to apply a gradient to the background.\n\n");
            Console.ReadKey();

            MIL.M3ddispControl(MilDisplay3d, MIL.M_BACKGROUND_MODE, MIL.M_GRADIENT_VERTICAL);

            MIL.M3ddispControl(MilDisplay3d, MIL.M_BACKGROUND_COLOR, MIL.M_COLOR_DARK_BLUE);
            MIL.M3ddispControl(MilDisplay3d, MIL.M_BACKGROUND_COLOR_GRADIENT, MIL.M_COLOR_DARK_YELLOW);

            Console.Write("Press <Enter> to use an image for the display's background.\n\n");
            Console.ReadKey();

            MIL_ID Image = MIL.M_NULL;
            MIL.MbufRestore(BACKGROUND_IMAGE_FILE, MilSystem, ref Image);

            // Make the image darker.
            MIL.MimShift(Image, Image, -1);
            MIL.M3ddispCopy(Image, MilDisplay3d, MIL.M_BACKGROUND_IMAGE, MIL.M_DEFAULT);
            MIL.M3ddispControl(MilDisplay3d, MIL.M_BACKGROUND_MODE, MIL.M_BACKGROUND_IMAGE);

            Console.Write("A gyroscope indicating interaction with the 3d display can be permanently\n");
            Console.Write("visible. Its appearance can also be modified. \n");
            Console.Write("Press <Enter> to make the gyroscope permanently visible.\n\n");
            Console.ReadKey();
            MIL.M3ddispControl(MilDisplay3d, MIL.M_ROTATION_INDICATOR, MIL.M_ENABLE);
            MIL.M3ddispControl(MilDisplay3d, MIL.M_BACKGROUND_MODE, MIL.M_SINGLE_COLOR);
            MIL.M3ddispControl(MilDisplay3d, MIL.M_BACKGROUND_COLOR, MIL.M_RGB888(50, 150, 125));

            Console.Write("Many keys are assigned to interactive actions.\n");
            Console.Write("   Arrows : Orbit around the interest point.\n");
            Console.Write("   Ctrl   : Speed modifier for the arrow keys.\n");
            Console.Write("   Alt    : Action modifier for the arrow keys. Press Alt and Up/Down arrow for\n");
            Console.Write("            zooming; press Alt and Left/Right arrow for rolling.\n");
            Console.Write("   Shift  : Modifies the arrows' function, moving the screen's plane instead.\n");
            Console.Write("   1 - 8  : Specify the predefined viewpoint. Press a number key.\n");

            Console.Write("Set focus to the 3D display window to use the keyboard.\n");

            Console.Write("Press <Enter> to end.\n\n");
            Console.ReadKey();


            MIL.MbufFree(Image);
            MIL.MbufFree(MilContainerId1);
            MIL.MbufFree(MilContainerId2);
            MIL.M3dgeoFree(MatrixGrid);
            MIL.M3dgeoFree(MatrixTranslate1);
            MIL.M3dgeoFree(MatrixTranslate2);
            MIL.M3dgeoFree(InitialViewpointMatrix);
            MIL.M3ddispFree(MilDisplay3d);
            MIL.MsysFree(MilSystem);
            MIL.MappFree(MilApplication);
            }


        //****************************************************************************
        // Generate a MIL 3D Container to display.
        //****************************************************************************
        public static MIL_ID Generate3DContainer(MIL_ID MilSystem)
            {
            // Use a SDCF to acquire a MIL container with 3D data.
            MIL_ID MilContainer3D = MIL.M_NULL;
            MIL_ID MilDigitizer = MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, "M_3D_SIMULATOR", MIL.M_DEFAULT, MIL.M_NULL);
            MIL.MbufAllocDefault(MilSystem, MilDigitizer, MIL.M_GRAB + MIL.M_DISP, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilContainer3D);
            MIL.MdigGrab(MilDigitizer, MilContainer3D);
            MIL.MdigFree(MilDigitizer);
            return MilContainer3D;
            }

        }



    }




