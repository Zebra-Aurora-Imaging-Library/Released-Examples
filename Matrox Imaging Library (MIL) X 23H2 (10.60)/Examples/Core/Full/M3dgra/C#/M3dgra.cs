//***************************************************************************************
// 
// File name: M3dgra.cpp  
//
// Synopsis: This program contains an example of how to use 3d graphics in MIL.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************

using System;
using Matrox.MatroxImagingLibrary;
using System.Threading;

namespace M3dgra
    {
    class Program
        {
        // ----------------------------------------------------------------------------
        // Example description.
        // ----------------------------------------------------------------------------
        private static void PrintHeader()
            {
            Console.WriteLine("[EXAMPLE NAME]");
            Console.WriteLine("M3dgra");
            Console.WriteLine();
            Console.WriteLine("[SYNOPSIS]");
            Console.WriteLine("This example demonstrates the usage of 3D graphics in MIL.");
            Console.WriteLine();

            Console.WriteLine("[MODULES USED]");
            Console.WriteLine("Modules used: application, system, buffer, 3D display, 3D graphics, 3D Geometry, 3D Image Processing.");
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();
            }
        //*****************************************************************************
        // Constants.
        //*****************************************************************************
        private static readonly string POINT_CLOUD_FILE = MIL.M_IMAGE_PATH + "M3dgra/MaskOrganized.mbufc";
        private static readonly string GLASSES_FILE = MIL.M_IMAGE_PATH + "M3dgra/Glasses.png";
        private static readonly string LOGO_FILE = MIL.M_IMAGE_PATH + "imaginglogo.mim";

        private static readonly int FADE_DELAY_MSEC      = 750;
        private static readonly int CONTROL_DELAY_MSEC   = 2000;
        private static readonly int CONTROL_GRANULARITY  = 20;

        // --------------------------------------------------------------
        private static void Main(string[] args)
            {
            // Print example information in console.
            PrintHeader();

            MIL_ID MilApplication = MIL.M_NULL;    // MIL application identifier
            MIL_ID MilSystem = MIL.M_NULL;         // MIL system identifier
            MIL_ID MilDisplay3d = MIL.M_NULL;      // MIL display identifier
            MIL_ID MilGraphicList3d = MIL.M_NULL;  // MIL 3d graphic list.
            MIL_ID MilContainerId = MIL.M_NULL;    // Point cloud to display.
            MIL_ID MilTextureId = MIL.M_NULL;      // Polygon texture.

            // Allocate the MIL application.
            MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT, ref MilApplication);
            
            // Allocate MIL objects.
            MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilSystem);

            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);
            MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, ref MilDisplay3d);
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);

            // Make sure we meet the minimum requirements for the 3d display.
            if (MilDisplay3d == MIL.M_NULL)
                {
                Console.WriteLine("The current system does not support the 3D display.");
                Console.WriteLine("Press any key to end.");
                Console.ReadKey();

                MIL.MsysFree(MilSystem);
                MIL.MappFree(MilApplication);
                return;
                }
            
            // Show the display.
            MIL.M3ddispInquire(MilDisplay3d, MIL.M_3D_GRAPHIC_LIST_ID, ref MilGraphicList3d);
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_VIEW_ORIENTATION, -2, -1.1, -1, MIL.M_DEFAULT);
            MIL.M3ddispSetView(MilDisplay3d, MIL.M_UP_VECTOR, 0, 0, 1, MIL.M_DEFAULT);
            MIL.M3ddispSelect(MilDisplay3d, MIL.M_NULL, MIL.M_OPEN, MIL.M_DEFAULT);


            // Draw an axis and a grid.
            Console.WriteLine("The 3d display can show many point clouds at the same time.");
            Console.WriteLine("It can also show no point cloud and only the contents of a 3D graphics list.");
            Console.WriteLine("Here, it shows an axis and a grid.");

            double AxisLength = 200;
            long AxisLabel = MIL.M3dgraAxis(MilGraphicList3d, MIL.M_ROOT_NODE, MIL.M_DEFAULT, AxisLength, "", MIL.M_DEFAULT, MIL.M_DEFAULT);

            MIL_ID Matrix = MIL.M3dgeoAlloc(MilSystem, MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dgeoMatrixSetTransform(Matrix, MIL.M_TRANSLATION, AxisLength * 0.4, AxisLength * 0.4, 0, MIL.M_DEFAULT, MIL.M_DEFAULT);
            long GridLabel = MIL.M3dgraGrid(MilGraphicList3d, AxisLabel, MIL.M_SIZE_AND_SPACING, Matrix, AxisLength * 0.8, AxisLength * 0.8, 16, 16, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, GridLabel, MIL.M_FILL_COLOR, MIL.M_COLOR_WHITE);
            MIL.M3dgraControl(MilGraphicList3d, GridLabel, MIL.M_COLOR, MIL.M_COLOR_BLACK);
            MIL.M3dgraControl(MilGraphicList3d, GridLabel, MIL.M_OPACITY, 30);

            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Restore and display the point cloud.
            MIL.MbufRestore(POINT_CLOUD_FILE, MilSystem, ref MilContainerId);
            long ContainerLabel = MIL.M3dgraAdd(MilGraphicList3d, AxisLabel, MilContainerId, MIL.M_DEFAULT);

            // Set various color modes.
            Console.WriteLine("A point cloud has been added to the display.");
            Console.WriteLine("By default, point clouds are colored using the reflectance or intensity component.");
            Console.WriteLine("However, you can use any band(s) of any component for the color, and optionally apply a LUT.");
            Console.WriteLine("Press <Enter> to view the coloring options.");
            Console.WriteLine();
            Console.ReadKey();

            Console.WriteLine("Range:                     The XYZ values are rescaled to RGB.");
            MIL.M3dgraControl(MilGraphicList3d, ContainerLabel, MIL.M_COLOR_COMPONENT, MIL.M_COMPONENT_RANGE);
            Console.Write("<Enter to continue>.\r");
            Console.ReadKey();

            Console.WriteLine("Range 3rd band with a LUT: Highlights elevation differences (Z).");
            MIL.M3dgraControl(MilGraphicList3d, ContainerLabel, MIL.M_COLOR_USE_LUT, MIL.M_TRUE);
            MIL.M3dgraControl(MilGraphicList3d, ContainerLabel, MIL.M_COLOR_COMPONENT_BAND, 2);
            Console.Write("<Enter to continue>.\r");
            Console.ReadKey();

            Console.WriteLine("Normals:                   Highlights details.");
            MIL.M3dimNormals(MIL.M_NORMALS_CONTEXT_ORGANIZED, MilContainerId, MilContainerId, MIL.M_DEFAULT);
            MIL.M3ddispControl(MilDisplay3d, MIL.M_UPDATE, MIL.M_DISABLE);
            MIL.M3dgraControl(MilGraphicList3d, ContainerLabel, MIL.M_COLOR_USE_LUT, MIL.M_FALSE);
            MIL.M3dgraControl(MilGraphicList3d, ContainerLabel, MIL.M_COLOR_COMPONENT_BAND, MIL.M_ALL_BANDS);
            MIL.M3dgraControl(MilGraphicList3d, ContainerLabel, MIL.M_COLOR_COMPONENT, MIL.M_COMPONENT_NORMALS_MIL);
            MIL.M3ddispControl(MilDisplay3d, MIL.M_UPDATE, MIL.M_ENABLE);
            Console.Write("<Enter to continue>.\r");
            Console.ReadKey();

            Console.WriteLine("Solid color:               Differentiates between multiple point clouds.");
            MIL.M3dgraControl(MilGraphicList3d, ContainerLabel, MIL.M_COLOR, MIL.M_COLOR_BLUE);
            MIL.M3dgraControl(MilGraphicList3d, ContainerLabel, MIL.M_COLOR_COMPONENT, MIL.M_NULL);
            Console.Write("<Enter to continue>.\r");
            Console.ReadKey();

            // Add a mesh.
            Console.WriteLine("Solid color (with a mesh): Similar to solid color while still showing details.");
            MIL.M3dimMesh(MIL.M_MESH_CONTEXT_ORGANIZED, MilContainerId, MilContainerId, MIL.M_DEFAULT);
            Console.WriteLine("<Enter to continue>.");
            Console.WriteLine();
            Console.ReadKey();

            // Restore the matrox logo and draw it on a polygon.
            Console.WriteLine("2D images can be displayed in the 3D graphics list via textured polygons.");

            MIL.M3dgraRemove(MilGraphicList3d, ContainerLabel, MIL.M_DEFAULT);
            MIL.MbufRestore(LOGO_FILE, MilSystem, ref MilTextureId);
            double PolygonCenterX = 100;
            double PolygonCenterY = 100;
            MIL_INT PolygonHalfSizeX = MIL.MbufInquire(MilTextureId, MIL.M_SIZE_X, MIL.M_NULL) / 5;
            MIL_INT PolygonHalfSizeY = MIL.MbufInquire(MilTextureId, MIL.M_SIZE_Y, MIL.M_NULL) / 5;
            double[] PolygonX = new double[] { PolygonCenterX - PolygonHalfSizeY, PolygonCenterX + PolygonHalfSizeY, PolygonCenterX + PolygonHalfSizeY, PolygonCenterX - PolygonHalfSizeY };
            double[] PolygonY = new double[] { PolygonCenterY - PolygonHalfSizeX, PolygonCenterY - PolygonHalfSizeX, PolygonCenterY + PolygonHalfSizeX, PolygonCenterY + PolygonHalfSizeX };
            double[] PolygonZ = new double[] { 30, 30, 30, 30 };
            long PolygonLabel = MIL.M3dgraPolygon(MilGraphicList3d, AxisLabel, MIL.M_DEFAULT, 4, PolygonX, PolygonY, PolygonZ, null, null, MilTextureId, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, PolygonLabel, MIL.M_SHADING, MIL.M_NONE);
            MIL.MbufFree(MilTextureId);

            // Draw other graphics.
            Console.WriteLine("Press <Enter> to show other graphic primitives.");
            Console.WriteLine();
            Console.ReadKey();
            MIL.M3dgraRemove(MilGraphicList3d, PolygonLabel, MIL.M_DEFAULT);
            MIL.M3dgeoMatrixSetTransform(Matrix, MIL.M_TRANSLATION, 100, 100, 0, MIL.M_DEFAULT, MIL.M_DEFAULT);
            long NodeLabel = MIL.M3dgraNode(MilGraphicList3d, AxisLabel, Matrix, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, MIL.M_DEFAULT_SETTINGS, MIL.M_OPACITY, 0);

            // Plane.
            long PlaneLabel = MIL.M3dgraPlane(MilGraphicList3d, NodeLabel, MIL.M_POINT_AND_NORMAL, 0, 0, 10, 0, 0, 1, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, 70, MIL.M_DEFAULT);
            FadeIn(MilGraphicList3d, PlaneLabel, FADE_DELAY_MSEC);

            // Box.
            long BoxLabel = MIL.M3dgraBox(MilGraphicList3d, NodeLabel, MIL.M_CENTER_AND_DIMENSION, 0, 0, 40, 40, 40, 60, MIL.M_DEFAULT, MIL.M_DEFAULT);
            FadeIn(MilGraphicList3d, BoxLabel, FADE_DELAY_MSEC);

            // Cylinder.
            long CylinderLabel = MIL.M3dgraCylinder(MilGraphicList3d, NodeLabel, MIL.M_TWO_POINTS, 0, 0, 70, 0, 0, 120, 20, MIL.M_DEFAULT, MIL.M_DEFAULT);
            FadeIn(MilGraphicList3d, CylinderLabel, FADE_DELAY_MSEC);

            // Sphere.
            long SphereLabel = MIL.M3dgraSphere(MilGraphicList3d, NodeLabel, 0, 0, 140, 20, MIL.M_DEFAULT);
            FadeIn(MilGraphicList3d, SphereLabel, FADE_DELAY_MSEC);

            // Line.
            long LineLabel = MIL.M3dgraLine(MilGraphicList3d, NodeLabel, MIL.M_TWO_POINTS, MIL.M_DEFAULT, 0, -20, 110, 0, -40, 70, MIL.M_DEFAULT, MIL.M_DEFAULT);
            FadeIn(MilGraphicList3d, LineLabel, FADE_DELAY_MSEC);

            // Arc.
            long ArcLabel = MIL.M3dgraArc(MilGraphicList3d, NodeLabel, MIL.M_THREE_POINTS, MIL.M_DEFAULT, 0, 20, 110, 0, 40, 130, 0, 40, 150, MIL.M_DEFAULT, MIL.M_DEFAULT);
            FadeIn(MilGraphicList3d, ArcLabel, FADE_DELAY_MSEC);

            // Dots.
            double[] DotsX = new double[] { 18, 18 };
            double[] DotsY = new double[] { -10, 10 };
            double[] DotsZ = new double[] { 145, 145 };
            long DotsLabel = MIL.M3dgraDots(MilGraphicList3d, NodeLabel, 2, DotsX, DotsY, DotsZ, null, null, null, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, DotsLabel, MIL.M_THICKNESS, 3);
            FadeIn(MilGraphicList3d, DotsLabel, FADE_DELAY_MSEC);

            // Polygon.
            MIL.MbufImport(GLASSES_FILE, MIL.M_PNG, MIL.M_RESTORE, MilSystem, ref MilTextureId);
            double[] GlassesX = new double[] { 20, 20, 20, 20 };
            double[] GlassesY = new double[] { -18, -18, 18, 18 };
            double[] GlassesZ = new double[] { 153, 141.5, 141.5, 153 };
            long GlassesLabel = MIL.M3dgraPolygon(MilGraphicList3d, NodeLabel, MIL.M_DEFAULT, 4, GlassesX, GlassesY, GlassesZ, null, null, MilTextureId, MIL.M_DEFAULT);
            MIL.MbufFree(MilTextureId);
            MIL.M3dgraControl(MilGraphicList3d, GlassesLabel, MIL.M_KEYING_COLOR, MIL.M_COLOR_WHITE);
            FadeIn(MilGraphicList3d, GlassesLabel, FADE_DELAY_MSEC);

            // Text.
            MIL.M3dgeoMatrixSetWithAxes(Matrix, MIL.M_XY_AXES, 0, 0, 165, 0, 1, 0, 0, 0, 1, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, MIL.M_DEFAULT_SETTINGS, MIL.M_FONT_SIZE, 15);
            MIL.M3dgraControl(MilGraphicList3d, MIL.M_DEFAULT_SETTINGS, MIL.M_TEXT_ALIGN_HORIZONTAL, MIL.M_CENTER);
            MIL.M3dgraControl(MilGraphicList3d, MIL.M_DEFAULT_SETTINGS, MIL.M_TEXT_ALIGN_VERTICAL, MIL.M_BOTTOM);
            long TextLabel = MIL.M3dgraText(MilGraphicList3d, NodeLabel, "Welcome to MIL!", Matrix, MIL.M_DEFAULT, MIL.M_DEFAULT);
            FadeIn(MilGraphicList3d, TextLabel, FADE_DELAY_MSEC);
            Console.WriteLine();

            MIL.M3dgraControl(MilGraphicList3d, MIL.M_DEFAULT_SETTINGS, MIL.M_OPACITY, 100);

            // Print the contents of the 3D graphics list.
            Console.WriteLine("The contents of the graphics list can be inquired either in a flat list or recursively.");
            Console.WriteLine("Press <Enter> to view the graphics in a flat list with their absolute position.");
            Console.WriteLine();
            Console.ReadKey();

            Console.WriteLine("Graphic type          Position X      Position Y      Position Z");
            Console.WriteLine("-----------------------------------------------------------------");
            PrintGraphicListFlat(MilGraphicList3d);

            Console.WriteLine();
            Console.WriteLine("Press <Enter> to view the graphics in a tree and their position relative to their parent.");
            Console.WriteLine();
            Console.ReadKey();

            Console.WriteLine("Graphic type          Position X      Position Y      Position Z");
            Console.WriteLine("-----------------------------------------------------------------");
            PrintGraphicListTree(MilGraphicList3d);

            // Perform various controls.
            Console.WriteLine();
            Console.WriteLine("The tree structure makes controlling groups of graphics easy.");
            Console.WriteLine("Here, the character's graphics are controlled all at once via their parent node.");
            Console.WriteLine("Press <Enter> see various controls.");
            Console.WriteLine();
            Console.ReadKey();

            // Color.
            Console.WriteLine("Color:      Doesn't affect textured polygons.");
            Console.Write("<Enter to continue>.\r");
            MIL_ID ColorLut = MIL.MbufAllocColor(MilSystem, 3, CONTROL_GRANULARITY, 1, MIL.M_UNSIGNED + 8, MIL.M_LUT, MIL.M_NULL);
            MIL.MgenLutFunction(ColorLut, MIL.M_COLORMAP_HUE, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
            for (int i = 0; !Console.KeyAvailable; i++)
                {
                byte[] Color = new byte[3];
                MIL.MbufGet1d(ColorLut, i % CONTROL_GRANULARITY, 1, Color);
                MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_COLOR + MIL.M_RECURSIVE, MIL.M_RGB888(Color[0], Color[1], Color[2]));
                Thread.Sleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
                }
            MIL.MbufFree(ColorLut);
            MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_COLOR + MIL.M_RECURSIVE, MIL.M_COLOR_WHITE);
            Console.ReadKey();

            // Opacity.
            Console.WriteLine("Opacity:    Graphics can be from fully opaque to fully transparent.");
            Console.Write("<Enter to continue>.\r");
            for (int i = 0; !Console.KeyAvailable; i++)
                {
                MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_OPACITY + MIL.M_RECURSIVE, 50 + 50 * Math.Sin(3.1415 * 2 * i / CONTROL_GRANULARITY));
                Thread.Sleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
                }
            MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_OPACITY + MIL.M_RECURSIVE, 100);
            Console.ReadKey();

            // Resolution.
            Console.WriteLine("Resolution: Controls how fine the mesh is for cylinders, spheres and arcs.");
            Console.Write("<Enter to continue>.\r");
            for (int i = 0; !Console.KeyAvailable; i++)
                {
                MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_GRAPHIC_RESOLUTION + MIL.M_RECURSIVE, 3 + i % CONTROL_GRANULARITY);
                Thread.Sleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
                }
            MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_GRAPHIC_RESOLUTION + MIL.M_RECURSIVE, 16);
            Console.ReadKey();

            // Shading.
            Console.WriteLine("Shading:    Graphics can choose between flat, Gouraud, Phong or no shading at all.");
            Console.Write("<Enter to continue>.\r");
            MIL_INT[] SHADINGS = { MIL.M_NONE, MIL.M_FLAT, MIL.M_GOURAUD, MIL.M_PHONG };
            for (int i = 0; !Console.KeyAvailable; i++)
                {
                MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_SHADING + MIL.M_RECURSIVE, SHADINGS[i % 4]);
                Thread.Sleep(CONTROL_DELAY_MSEC / 4);
                }
            MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_SHADING + MIL.M_RECURSIVE, MIL.M_GOURAUD);
            Console.ReadKey();

            // Thickness.
            Console.WriteLine("Thickness:  Controls how thick lines and points look.");
            Console.Write("<Enter to continue>.\r");
            for (int i = 0; !Console.KeyAvailable; i++)
                {
                MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_THICKNESS + MIL.M_RECURSIVE, 1 + i % CONTROL_GRANULARITY);
                Thread.Sleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
                }
            MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_THICKNESS + MIL.M_RECURSIVE, 1);
            Console.ReadKey();
            
            // Movement.
            Console.WriteLine("Movement:   Graphics can be moved with rigid transformations.");
            Console.Write("<Enter to continue>.\r");
            MIL.M3dgeoMatrixSetTransform(Matrix, MIL.M_TRANSLATION, -100, -100, 0, MIL.M_DEFAULT, MIL.M_DEFAULT);
            MIL.M3dgeoMatrixSetTransform(Matrix, MIL.M_ROTATION_Z, 90.0 / CONTROL_GRANULARITY, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_COMPOSE_WITH_CURRENT);
            MIL.M3dgeoMatrixSetTransform(Matrix, MIL.M_TRANSLATION, 100, 100, 0, MIL.M_DEFAULT, MIL.M_COMPOSE_WITH_CURRENT);

            while (!Console.KeyAvailable)
                {
                MIL.M3dgraCopy(Matrix, MIL.M_DEFAULT, MilGraphicList3d, NodeLabel, MIL.M_TRANSFORMATION_MATRIX + MIL.M_COMPOSE_WITH_CURRENT, MIL.M_DEFAULT);
                Thread.Sleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
                }
            MIL.M3dgeoMatrixSetTransform(Matrix, MIL.M_TRANSLATION, 100, 100, 0, MIL.M_DEFAULT, MIL.M_DEFAULT);
            MIL.M3dgraCopy(Matrix, MIL.M_DEFAULT, MilGraphicList3d, NodeLabel, MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT);
            Console.ReadKey();

            // Copy.
            Console.WriteLine("Copy:       Graphics can be copied across the same or different graphics lists.");
            Console.Write("<Enter to continue>.\r");
            long CopyLabel = MIL.M3dgraCopy(MilGraphicList3d, NodeLabel, MilGraphicList3d, AxisLabel, MIL.M_GRAPHIC + MIL.M_RECURSIVE, MIL.M_DEFAULT);
            MIL.M3dgeoMatrixSetTransform(Matrix, MIL.M_TRANSLATION, 100, 250, 0, MIL.M_DEFAULT, MIL.M_DEFAULT);
            MIL.M3dgraCopy(Matrix, MIL.M_DEFAULT, MilGraphicList3d, CopyLabel, MIL.M_TRANSFORMATION_MATRIX, MIL.M_DEFAULT);
            Console.ReadKey();

            // Visibility.
            Console.WriteLine("Visibility: Unneeded graphics can be hidden without deleting them.");
            Console.Write("<Enter to continue>.\r");
            for (int i = 0; !Console.KeyAvailable; i++)
                {
                MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_VISIBLE + MIL.M_RECURSIVE, ((i + 1) % 4 < 2) ? MIL.M_TRUE : MIL.M_FALSE);
                MIL.M3dgraControl(MilGraphicList3d, CopyLabel, MIL.M_VISIBLE + MIL.M_RECURSIVE, (i % 4 < 2) ? MIL.M_TRUE : MIL.M_FALSE);
                Thread.Sleep(CONTROL_DELAY_MSEC / 4);
                }
            MIL.M3dgraControl(MilGraphicList3d, NodeLabel, MIL.M_VISIBLE + MIL.M_RECURSIVE, MIL.M_TRUE);
            MIL.M3dgraControl(MilGraphicList3d, CopyLabel, MIL.M_VISIBLE + MIL.M_RECURSIVE, MIL.M_FALSE);
            Console.ReadKey();

            // Inquire and draw the bounding box.
            Console.WriteLine("                    ");
            Console.WriteLine("It may be useful to know the bounding box of the 3D graphics list.");

            MIL_ID MilBoxGeometry = MIL.M3dgeoAlloc(MilSystem, MIL.M_GEOMETRY, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dgraCopy(MilGraphicList3d, MIL.M_LIST, MilBoxGeometry, MIL.M_DEFAULT, MIL.M_BOUNDING_BOX, MIL.M_DEFAULT);
            long BoundingBoxLabel = MIL.M3dgeoDraw3d(MIL.M_DEFAULT, MilBoxGeometry, MilGraphicList3d, MIL.M_ROOT_NODE, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, BoundingBoxLabel, MIL.M_OPACITY, 30);
            MIL.M3dgraControl(MilGraphicList3d, BoundingBoxLabel, MIL.M_THICKNESS, 3);
            MIL.M3dgraControl(MilGraphicList3d, BoundingBoxLabel, MIL.M_FILL_COLOR, MIL.M_COLOR_WHITE);
            MIL.M3dgraControl(MilGraphicList3d, BoundingBoxLabel, MIL.M_COLOR, MIL.M_COLOR_BLACK);

            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Change the appearance of the bounding box.
            Console.WriteLine("Graphics can be displayed as either points, wireframe, or solid surfaces.");
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();

            MIL_INT[] Appearances = { MIL.M_POINTS, MIL.M_WIREFRAME, MIL.M_SOLID_WITH_WIREFRAME, MIL.M_SOLID };
            for (MIL_INT i = 0; !Console.KeyAvailable; i++)
                {
                MIL.M3dgraControl(MilGraphicList3d, BoundingBoxLabel, MIL.M_APPEARANCE, Appearances[i % 4]);
                Thread.Sleep(CONTROL_DELAY_MSEC / 4);
                }
            MIL.M3dgraControl(MilGraphicList3d, BoundingBoxLabel, MIL.M_APPEARANCE, MIL.M_SOLID_WITH_WIREFRAME);
            Console.ReadKey();

            // Clip a plane using the bounding box.
            Console.WriteLine("The bounding box is used to clip infinite geometries like planes, lines and cylinders.");
            Console.WriteLine("Press <Enter> to show plane clipping.");
            Console.WriteLine();
            Console.ReadKey();

            long InfinitePlaneLabel = MIL.M3dgraPlane(MilGraphicList3d, MIL.M_ROOT_NODE, MIL.M_POINT_AND_NORMAL, 100, 100, 30, 0.5, 0.4, 3, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_INFINITE, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, InfinitePlaneLabel, MIL.M_COLOR, MIL.M_COLOR_BLUE);
            MIL.M3dgraControl(MilGraphicList3d, InfinitePlaneLabel, MIL.M_OPACITY, 60);

            // Change the clipping box and clip a line.
            Console.WriteLine("The clipping box can also be set manually; it does not need to be the same as the bounding box.");
            Console.WriteLine("Press <Enter> to set a different clipping box.");
            Console.WriteLine();
            Console.ReadKey();

            // Remove plane to focus on the clipped line.
            MIL.M3dgraRemove(MilGraphicList3d, InfinitePlaneLabel, MIL.M_DEFAULT);

            MIL.M3dgeoBox(MilBoxGeometry, MIL.M_CENTER_AND_DIMENSION, 0, 0, 0, 350, 350, 350, MIL.M_DEFAULT);
            MIL.M3dgraCopy(MilBoxGeometry, MIL.M_DEFAULT, MilGraphicList3d, MIL.M_LIST, MIL.M_CLIPPING_BOX, MIL.M_DEFAULT);
            MIL.M3dgraRemove(MilGraphicList3d, BoundingBoxLabel, MIL.M_DEFAULT);
            long ClippingBoxLabel = MIL.M3dgeoDraw3d(MIL.M_DEFAULT, MilBoxGeometry, MilGraphicList3d, MIL.M_ROOT_NODE, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, ClippingBoxLabel, MIL.M_OPACITY, 30);
            MIL.M3dgraControl(MilGraphicList3d, ClippingBoxLabel, MIL.M_APPEARANCE, MIL.M_SOLID_WITH_WIREFRAME);
            MIL.M3dgraControl(MilGraphicList3d, ClippingBoxLabel, MIL.M_FILL_COLOR, MIL.M_COLOR_WHITE);
            MIL.M3dgraControl(MilGraphicList3d, ClippingBoxLabel, MIL.M_COLOR, MIL.M_COLOR_BLACK);
            MIL.M3dgraControl(MilGraphicList3d, ClippingBoxLabel, MIL.M_THICKNESS, 3);

            Console.WriteLine("Showing a clipped infinite line in cyan.");
            Console.WriteLine();

            long InfiniteLineLabel = MIL.M3dgraLine(MilGraphicList3d, MIL.M_ROOT_NODE, MIL.M_POINT_AND_VECTOR, MIL.M_DEFAULT, 140, 50, 0, 0, 5, 7, MIL.M_INFINITE, MIL.M_DEFAULT);
            MIL.M3dgraControl(MilGraphicList3d, InfiniteLineLabel, MIL.M_COLOR, MIL.M_COLOR_CYAN);
            MIL.M3dgraControl(MilGraphicList3d, InfiniteLineLabel, MIL.M_THICKNESS, 5);

            Console.WriteLine("Press <Enter> to end.");
            Console.WriteLine();
            Console.ReadKey();

            MIL.M3dgeoFree(MilBoxGeometry);
            MIL.M3dgeoFree(Matrix);
            MIL.MbufFree(MilContainerId);
            MIL.M3ddispFree(MilDisplay3d);
            MIL.MsysFree(MilSystem);
            MIL.MappFree(MilApplication);
            }

        private static string GetGraphicTypeString(MIL_ID MilGraphicList3d, long GraphicLabel)
            {
            string Info;
            if (GraphicLabel == MIL.M_ROOT_NODE)
                {
                Info = "Root        ";
                }
            else
                {
                MIL_INT Type = MIL.M_NULL;
                MIL.M3dgraInquire(MilGraphicList3d, GraphicLabel, MIL.M_GRAPHIC_TYPE, ref Type);
                switch ((int)Type)
                    {
                    case MIL.M_GRAPHIC_TYPE_ARC:            Info = "Arc         ";  break;
                    case MIL.M_GRAPHIC_TYPE_AXIS:           Info = "Axis        ";  break;
                    case MIL.M_GRAPHIC_TYPE_BOX:            Info = "Box         ";  break;
                    case MIL.M_GRAPHIC_TYPE_CYLINDER:       Info = "Cylinder    ";  break;
                    case MIL.M_GRAPHIC_TYPE_DOTS:           Info = "Dots        ";  break;
                    case MIL.M_GRAPHIC_TYPE_GRID:           Info = "Grid        ";  break;
                    case MIL.M_GRAPHIC_TYPE_LINE:           Info = "Line        ";  break;
                    case MIL.M_GRAPHIC_TYPE_NODE:           Info = "Node        ";  break;
                    case MIL.M_GRAPHIC_TYPE_PLANE:          Info = "Plane       ";  break;
                    case MIL.M_GRAPHIC_TYPE_POINT_CLOUD:    Info = "Point cloud ";  break;
                    case MIL.M_GRAPHIC_TYPE_POLYGON:        Info = "Polygon     ";  break;
                    case MIL.M_GRAPHIC_TYPE_SPHERE:         Info = "Sphere      ";  break;
                    case MIL.M_GRAPHIC_TYPE_TEXT:           Info = "Text        ";  break;
                    default:                                Info = "Unknown     ";  break;
                    }
                }

            return Info;
            }

        private static void PrintGraphicListFlat(MIL_ID MilGraphicList3d)
            {
            MIL_INT NbChildren = MIL.M_NULL;
            MIL.M3dgraInquire(MilGraphicList3d, MIL.M_ROOT_NODE, MIL.M_NUMBER_OF_CHILDREN + MIL.M_RECURSIVE, ref NbChildren);
            long[] GraphicLabels = new long[NbChildren + 1];
            MIL.M3dgraInquire(MilGraphicList3d, MIL.M_ROOT_NODE, MIL.M_CHILDREN + MIL.M_RECURSIVE, GraphicLabels);
            GraphicLabels[NbChildren] = MIL.M_ROOT_NODE;

            for (int i = 0; i <= NbChildren; i++)
                {
                string GraphicInfo = GetGraphicTypeString(MilGraphicList3d, GraphicLabels[i]);

                double PosX = 0;
                double PosY = 0;
                double PosZ = 0;
                MIL.M3dgraInquire(MilGraphicList3d, GraphicLabels[i], MIL.M_POSITION_X + MIL.M_RELATIVE_TO_ROOT, ref PosX);
                MIL.M3dgraInquire(MilGraphicList3d, GraphicLabels[i], MIL.M_POSITION_Y + MIL.M_RELATIVE_TO_ROOT, ref PosY);
                MIL.M3dgraInquire(MilGraphicList3d, GraphicLabels[i], MIL.M_POSITION_Z + MIL.M_RELATIVE_TO_ROOT, ref PosZ);

                Console.WriteLine("-{0:F2}\t\t{1:F2}\t\t{2:F2}\t\t{3:F2}", GraphicInfo, PosX, PosY, PosZ);
                }
            }
        
        private static void PrintGraphicListTree(MIL_ID MilGraphicList3d, long GraphicLabel = MIL.M_ROOT_NODE, string Prefix = "-")
            {
            string GraphicInfo = GetGraphicTypeString(MilGraphicList3d, GraphicLabel);
            Console.Write("{0}{1}", Prefix, GraphicInfo);

            int Padding = 24 - (Prefix.Length + GraphicInfo.Length);
            for(int i = 0; i < Padding; i++)
                { Console.Write(" "); }

            double PosX = 0;
            double PosY = 0;
            double PosZ = 0;
            MIL.M3dgraInquire(MilGraphicList3d, GraphicLabel, MIL.M_POSITION_X, ref PosX);
            MIL.M3dgraInquire(MilGraphicList3d, GraphicLabel, MIL.M_POSITION_Y, ref PosY);
            MIL.M3dgraInquire(MilGraphicList3d, GraphicLabel, MIL.M_POSITION_Z, ref PosZ);

            Console.WriteLine("{0:F2}\t\t{1:F2}\t\t{2:F2}", PosX, PosY, PosZ);

            MIL_INT NbChildren = MIL.M_NULL;
            MIL.M3dgraInquire(MilGraphicList3d, GraphicLabel, MIL.M_NUMBER_OF_CHILDREN, ref NbChildren);
            long[] ChildrenLabels = new long[NbChildren];
            MIL.M3dgraInquire(MilGraphicList3d, GraphicLabel, MIL.M_CHILDREN, ChildrenLabels);
            for (int i = 0; i < NbChildren; i++)
                {
                string ChildrenPrefix = Prefix.Replace('-', ' ').Replace('\'', ' ');
                if(i + 1 < NbChildren)
                    {
                    ChildrenPrefix += "|-";
                    }
                else
                    {
                    ChildrenPrefix += "'-";
                    }
                PrintGraphicListTree(MilGraphicList3d, ChildrenLabels[i], ChildrenPrefix);
                }
            }

        private static void FadeIn(MIL_ID MilGraphicList3d, long GraphicLabel, int Duration)
            {
            string GraphicTypeStr = GetGraphicTypeString(MilGraphicList3d, GraphicLabel);
            Console.WriteLine("Adding {0}.", GraphicTypeStr.TrimEnd(' ', '\n', '\t'));

            for (MIL_INT i = 0; (i < CONTROL_GRANULARITY) && !Console.KeyAvailable; i++)
                {
                MIL.M3dgraControl(MilGraphicList3d, GraphicLabel, MIL.M_OPACITY + MIL.M_RECURSIVE, 100.0 * i / CONTROL_GRANULARITY);
                Thread.Sleep(Duration / CONTROL_GRANULARITY);
                }

            MIL.M3dgraControl(MilGraphicList3d, GraphicLabel, MIL.M_OPACITY + MIL.M_RECURSIVE, 100.0);
            }
        }
    }
