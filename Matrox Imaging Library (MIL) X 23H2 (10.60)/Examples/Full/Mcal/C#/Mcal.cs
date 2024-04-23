﻿//*******************************************************************************
//
// File name: MCal.cs
//
// Synopsis:  This program uses the Calibration module to:
//              - Remove distortion and then take measurements in world units using a 2D 
//                calibration.
//              - Perform a 3D calibration to take measurements at several known elevations.
//              - Calibrate a scene using a partial calibration grid that has a 2D code 
//                fiducial.
//
// Printable calibration grids in PDF format can be found in your
// "Matrox Imaging/Images/" directory.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
//*******************************************************************************
using System;
using System.Collections.Generic;
using System.Text;

using Matrox.MatroxImagingLibrary;

namespace Mcal
{
    class Program
    {
        // Example selection.
        private const int RUN_LINEAR_CALIBRATION_EXAMPLE = MIL.M_YES;
        private const int RUN_TSAI_CALIBRATION_EXAMPLE = MIL.M_YES;
        private const int RUN_PARTIAL_GRID_CALIBRATION_EXAMPLE = MIL.M_YES;

        // Grid offset specifications.
        private const int GRID_OFFSET_X = 0;
        private const int GRID_OFFSET_Y = 0;
        private const int GRID_OFFSET_Z = 0;

        static void Main(string[] args)
        {
            MIL_ID MilApplication = MIL.M_NULL; // Application identifier.
            MIL_ID MilSystem = MIL.M_NULL;      // System Identifier.
            MIL_ID MilDisplay = MIL.M_NULL;     // Display identifier.

            // Allocate defaults.
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, ref MilDisplay, MIL.M_NULL, MIL.M_NULL);

            // Print module name.
            Console.WriteLine("CALIBRATION MODULE:");
            Console.WriteLine("-------------------");
            Console.WriteLine();

            if (RUN_LINEAR_CALIBRATION_EXAMPLE == MIL.M_YES)
            {
                LinearInterpolationCalibration(MilSystem, MilDisplay);
            }

            if (RUN_TSAI_CALIBRATION_EXAMPLE == MIL.M_YES)
            {
                TsaiCalibration(MilSystem, MilDisplay);
            }

            if (RUN_PARTIAL_GRID_CALIBRATION_EXAMPLE == MIL.M_YES)
            {
                PartialGridCalibration(MilSystem, MilDisplay);
            }

            // Free defaults.
            MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL);
        }

        //****************************************************************************
        // Linear interpolation example. 
        //****************************************************************************

        // Source image files specification.
        private const string GRID_IMAGE_FILE = MIL.M_IMAGE_PATH + "CalGrid.mim";
        private const string BOARD_IMAGE_FILE = MIL.M_IMAGE_PATH + "CalBoard.mim";

        // World description of the calibration grid.
        private const int GRID_ROW_SPACING = 1;
        private const int GRID_COLUMN_SPACING = 1;
        private const int GRID_ROW_NUMBER = 18;
        private const int GRID_COLUMN_NUMBER = 25;

        // Measurement boxes specification.
        private const int MEAS_BOX_POS_X1 = 55;
        private const int MEAS_BOX_POS_Y1 = 24;
        private const int MEAS_BOX_WIDTH1 = 7;
        private const int MEAS_BOX_HEIGHT1 = 425;

        private const int MEAS_BOX_POS_X2 = 225;
        private const int MEAS_BOX_POS_Y2 = 11;
        private const int MEAS_BOX_WIDTH2 = 7;
        private const int MEAS_BOX_HEIGHT2 = 450;

        // Specification of the stripes' constraints.
        private const int WIDTH_APPROXIMATION = 410;
        private const int WIDTH_VARIATION = 25;
        private const int MIN_EDGE_VALUE = 5;

        static void LinearInterpolationCalibration(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilImage = MIL.M_NULL;          // Image buffer identifier.
            MIL_ID MilOverlayImage = MIL.M_NULL;   // Overlay image.
            MIL_ID MilCalibration = MIL.M_NULL;    // Calibration identifier.
            MIL_ID MeasMarker1 = MIL.M_NULL;       // Measurement marker identifier.
            MIL_ID MeasMarker2 = MIL.M_NULL;       // Measurement marker identifier.
            double WorldDistance1 = 0.0;
            double WorldDistance2 = 0.0;
            double PixelDistance1 = 0.0;
            double PixelDistance2 = 0.0;
            double PosX1 = 0.0;
            double PosY1 = 0.0;
            double PosX2 = 0.0;
            double PosY2 = 0.0;
            double PosX3 = 0.0;
            double PosY3 = 0.0;
            double PosX4 = 0.0;
            double PosY4 = 0.0;
            MIL_INT CalibrationStatus = 0;

            // Clear the display.
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT);

            // Restore source image into an automatically allocated image buffer.
            MIL.MbufRestore(GRID_IMAGE_FILE, MilSystem, ref MilImage);

            // Display the image buffer.
            MIL.MdispSelect(MilDisplay, MilImage);

            // Prepare for overlay annotation.
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE);
            MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID, ref MilOverlayImage);

            // Pause to show the original image.
            Console.WriteLine();
            Console.WriteLine("LINEAR INTERPOLATION CALIBRATION:");
            Console.WriteLine("---------------------------------");
            Console.WriteLine();
            Console.WriteLine("The displayed grid has been grabbed with a high distortion");
            Console.WriteLine("camera and will be used to calibrate the camera.");
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Allocate a camera calibration context.
            MIL.McalAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilCalibration);

            // Calibrate the camera with the image of the grid and its world description.
            MIL.McalGrid(MilCalibration, MilImage, GRID_OFFSET_X, GRID_OFFSET_Y, GRID_OFFSET_Z, GRID_ROW_NUMBER, GRID_COLUMN_NUMBER, GRID_ROW_SPACING, GRID_COLUMN_SPACING, MIL.M_DEFAULT, MIL.M_DEFAULT);

            MIL.McalInquire(MilCalibration, MIL.M_CALIBRATION_STATUS + MIL.M_TYPE_MIL_INT, ref CalibrationStatus);
            if (CalibrationStatus == MIL.M_CALIBRATED)
            {
                // Perform a first image transformation with the calibration grid.
                MIL.McalTransformImage(MilImage, MilImage, MilCalibration, MIL.M_BILINEAR | MIL.M_OVERSCAN_CLEAR, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Pause to show the corrected image of the grid.
                Console.WriteLine("The camera has been calibrated and the image of the grid");
                Console.WriteLine("has been transformed to remove its distortions.");
                Console.WriteLine("Press <Enter> to continue.");
                Console.WriteLine();
                Console.ReadKey();

                // Read the image of the board and associate the calibration to the image.
                MIL.MbufLoad(BOARD_IMAGE_FILE, MilImage);
                MIL.McalAssociate(MilCalibration, MilImage, MIL.M_DEFAULT);

                // Allocate the measurement markers.
                MIL.MmeasAllocMarker(MilSystem, MIL.M_STRIPE, MIL.M_DEFAULT, ref MeasMarker1);
                MIL.MmeasAllocMarker(MilSystem, MIL.M_STRIPE, MIL.M_DEFAULT, ref MeasMarker2);

                // Set the markers' measurement search region.
                MIL.MmeasSetMarker(MeasMarker1, MIL.M_BOX_ORIGIN, MEAS_BOX_POS_X1, MEAS_BOX_POS_Y1);
                MIL.MmeasSetMarker(MeasMarker1, MIL.M_BOX_SIZE, MEAS_BOX_WIDTH1, MEAS_BOX_HEIGHT1);
                MIL.MmeasSetMarker(MeasMarker2, MIL.M_BOX_ORIGIN, MEAS_BOX_POS_X2, MEAS_BOX_POS_Y2);
                MIL.MmeasSetMarker(MeasMarker2, MIL.M_BOX_SIZE, MEAS_BOX_WIDTH2, MEAS_BOX_HEIGHT2);

                // Set markers' orientation.
                MIL.MmeasSetMarker(MeasMarker1, MIL.M_ORIENTATION, MIL.M_HORIZONTAL, MIL.M_NULL);
                MIL.MmeasSetMarker(MeasMarker2, MIL.M_ORIENTATION, MIL.M_HORIZONTAL, MIL.M_NULL);

                // Set markers' settings to locate the largest stripe within the range
                // [WIDTH_APPROXIMATION - WIDTH_VARIATION,
                //  WIDTH_APPROXIMATION + WIDTH_VARIATION],
                // and with an edge strength over MIN_EDGE_VALUE.
                MIL.MmeasSetMarker(MeasMarker1, MIL.M_EDGEVALUE_MIN, MIN_EDGE_VALUE, MIL.M_NULL);

                // Remove the default strength characteristic score mapping.
                MIL.MmeasSetScore(MeasMarker1, MIL.M_STRENGTH_SCORE,
                                           0.0,
                                           0.0,
                                           MIL.M_MAX_POSSIBLE_VALUE,
                                           MIL.M_MAX_POSSIBLE_VALUE,
                                           MIL.M_DEFAULT,
                                           MIL.M_DEFAULT,
                                           MIL.M_DEFAULT);

                // Add a width characteristic score mapping (increasing ramp)
                // to find the largest stripe within a given range.
                //
                // Width score mapping to find the largest stripe within a given
                // width range ]Wmin, Wmax]:
                //
                //    Score
                //       ^
                //       |         /|
                //       |       /  |
                //       |     /    |
                //       +---------------> Width
                //           Wmin  Wmax
                //
                MIL.MmeasSetScore(MeasMarker1, MIL.M_STRIPE_WIDTH_SCORE,
                                           WIDTH_APPROXIMATION - WIDTH_VARIATION,
                                           WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                           WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                           WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                           MIL.M_DEFAULT,
                                           MIL.M_PIXEL,
                                           MIL.M_DEFAULT);

                // Set the same settings for the second marker.
                MIL.MmeasSetMarker(MeasMarker2, MIL.M_EDGEVALUE_MIN, MIN_EDGE_VALUE, MIL.M_NULL);

                MIL.MmeasSetScore(MeasMarker2, MIL.M_STRENGTH_SCORE,
                                           0.0,
                                           0.0,
                                           MIL.M_MAX_POSSIBLE_VALUE,
                                           MIL.M_MAX_POSSIBLE_VALUE,
                                           MIL.M_DEFAULT,
                                           MIL.M_DEFAULT,
                                           MIL.M_DEFAULT);

                MIL.MmeasSetScore(MeasMarker2, MIL.M_STRIPE_WIDTH_SCORE,
                                           WIDTH_APPROXIMATION - WIDTH_VARIATION,
                                           WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                           WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                           WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                           MIL.M_DEFAULT,
                                           MIL.M_PIXEL,
                                           MIL.M_DEFAULT);

                // Find and measure the position and width of the board.
                MIL.MmeasFindMarker(MIL.M_DEFAULT, MilImage, MeasMarker1, MIL.M_STRIPE_WIDTH + MIL.M_POSITION);
                MIL.MmeasFindMarker(MIL.M_DEFAULT, MilImage, MeasMarker2, MIL.M_STRIPE_WIDTH + MIL.M_POSITION);

                // Get the world width of the two markers.
                MIL.MmeasGetResult(MeasMarker1, MIL.M_STRIPE_WIDTH, ref WorldDistance1);
                MIL.MmeasGetResult(MeasMarker2, MIL.M_STRIPE_WIDTH, ref WorldDistance2);

                // Get the pixel width of the two markers.
                MIL.MmeasSetMarker(MeasMarker1, MIL.M_RESULT_OUTPUT_UNITS, MIL.M_PIXEL, MIL.M_NULL);
                MIL.MmeasSetMarker(MeasMarker2, MIL.M_RESULT_OUTPUT_UNITS, MIL.M_PIXEL, MIL.M_NULL);
                MIL.MmeasGetResult(MeasMarker1, MIL.M_STRIPE_WIDTH, ref PixelDistance1);
                MIL.MmeasGetResult(MeasMarker2, MIL.M_STRIPE_WIDTH, ref PixelDistance2);

                // Get the edges position in pixel to draw the annotations.
                MIL.MmeasGetResult(MeasMarker1, MIL.M_POSITION + MIL.M_EDGE_FIRST, ref PosX1, ref PosY1);
                MIL.MmeasGetResult(MeasMarker1, MIL.M_POSITION + MIL.M_EDGE_SECOND, ref PosX2, ref PosY2);
                MIL.MmeasGetResult(MeasMarker2, MIL.M_POSITION + MIL.M_EDGE_FIRST, ref PosX3, ref PosY3);
                MIL.MmeasGetResult(MeasMarker2, MIL.M_POSITION + MIL.M_EDGE_SECOND, ref PosX4, ref PosY4);

                // Draw the measurement indicators on the image. 
                MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_YELLOW);
                MIL.MmeasDraw(MIL.M_DEFAULT, MeasMarker1, MilOverlayImage, MIL.M_DRAW_WIDTH, MIL.M_DEFAULT, MIL.M_RESULT);
                MIL.MmeasDraw(MIL.M_DEFAULT, MeasMarker2, MilOverlayImage, MIL.M_DRAW_WIDTH, MIL.M_DEFAULT, MIL.M_RESULT);

                MIL.MgraBackColor(MIL.M_DEFAULT, MIL.M_COLOR_BLACK);
                MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, (int)(PosX1 + 0.5 - 40), (int)((PosY1 + 0.5) + ((PosY2 - PosY1) / 2.0)), " Distance 1 ");
                MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, (int)(PosX3 + 0.5 - 40), (int)((PosY3 + 0.5) + ((PosY4 - PosY3) / 2.0)), " Distance 2 ");

                // Pause to show the original image and the measurement results.
                Console.WriteLine("A distorted image grabbed with the same camera was loaded and");
                Console.WriteLine("calibrated measurements were done to evaluate the board dimensions.");
                Console.WriteLine();
                Console.WriteLine("========================================================");
                Console.WriteLine("                      Distance 1          Distance 2 ");
                Console.WriteLine("--------------------------------------------------------");
                Console.WriteLine(" Calibrated unit:   {0,8:0.00} cm           {1,6:0.00} cm    ", WorldDistance1, WorldDistance2);
                Console.WriteLine(" Uncalibrated unit: {0,8:0.00} pixels       {1,6:0.00} pixels", PixelDistance1, PixelDistance2);
                Console.WriteLine("========================================================");
                Console.WriteLine();
                Console.WriteLine("Press <Enter> to continue.");
                Console.WriteLine();
                Console.ReadKey();

                // Clear the display overlay.
                MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT);

                // Read the image of the PCB.
                MIL.MbufLoad(BOARD_IMAGE_FILE, MilImage);

                // Transform the image of the board.
                MIL.McalTransformImage(MilImage, MilImage, MilCalibration, MIL.M_BILINEAR + MIL.M_OVERSCAN_CLEAR, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // show the transformed image of the board.
                Console.WriteLine("The image was corrected to remove its distortions.");

                // Free measurement markers.
                MIL.MmeasFree(MeasMarker1);
                MIL.MmeasFree(MeasMarker2);
            }
            else
            {
                Console.WriteLine("Calibration generated an exception.");
                Console.WriteLine("See User Guide to resolve the situation.");
                Console.WriteLine();
            }

            // Wait for a key to be pressed.
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Free all allocations.
            MIL.McalFree(MilCalibration);
            MIL.MbufFree(MilImage);
        }

        //****************************************************************************
        // Tsai example. 
        //****************************************************************************
        // Source image files specification.
        private const string GRID_ORIGINAL_IMAGE_FILE = MIL.M_IMAGE_PATH + "CalGridOriginal.mim";
        private const string OBJECT_ORIGINAL_IMAGE_FILE = MIL.M_IMAGE_PATH + "CalObjOriginal.mim";
        private const string OBJECT_MOVED_IMAGE_FILE = MIL.M_IMAGE_PATH + "CalObjMoved.mim";

        // World description of the calibration grid.
        private const double GRID_ORG_ROW_SPACING = 1.5;
        private const double GRID_ORG_COLUMN_SPACING = 1.5;
        private const int GRID_ORG_ROW_NUMBER = 12;
        private const int GRID_ORG_COLUMN_NUMBER = 13;
        private const int GRID_ORG_OFFSET_X = 0;
        private const int GRID_ORG_OFFSET_Y = 0;
        private const int GRID_ORG_OFFSET_Z = 0;

        // Region parameters for metrology
        private const int MEASURED_CIRCLE_LABEL = 1;
        private const double RING1_POS1_X = 2.3;
        private const double RING1_POS1_Y = 3.9;
        private const double RING2_POS1_X = 10.7;
        private const double RING2_POS1_Y = 11.1;

        private const double RING_START_RADIUS = 1.25;
        private const double RING_END_RADIUS = 2.3;

        // measured plane position
        private const double RING_THICKNESS = 0.175;
        private const double STEP_THICKNESS = 4.0;

        // Color definitions
        static readonly int ABSOLUTE_COLOR = MIL.M_RGB888(255, 0, 0);
        static readonly int RELATIVE_COLOR = MIL.M_RGB888(0, 255, 0);
        static readonly int REGION_COLOR = MIL.M_RGB888(0, 100, 255);
        static readonly int FEATURE_COLOR = MIL.M_RGB888(255, 0, 255);

        static void TsaiCalibration(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilImage = MIL.M_NULL;          // Image buffer identifier.
            MIL_ID MilOverlayImage = MIL.M_NULL;   // Overlay image buffer identifier.
            MIL_ID MilCalibration = MIL.M_NULL;    // Calibration identifier.

            MIL_INT CalibrationStatus = 0;

            // Restore source image into an automatically allocated image buffer.
            MIL.MbufRestore(GRID_ORIGINAL_IMAGE_FILE, MilSystem, ref MilImage);

            // Display the image buffer.
            MIL.MdispSelect(MilDisplay, MilImage);
            // Prepare for overlay annotation.
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE);
            MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID, ref MilOverlayImage);

            // Pause to show the original image.
            Console.WriteLine();
            Console.WriteLine("TSAI BASED CALIBRATION:");
            Console.WriteLine("-----------------------");
            Console.WriteLine();
            Console.WriteLine("The displayed grid has been grabbed with a high perspective");
            Console.WriteLine("camera and will be used to calibrate the camera.");
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Allocate a camera calibration context.
            MIL.McalAlloc(MilSystem, MIL.M_TSAI_BASED, MIL.M_DEFAULT, ref MilCalibration);

            // Calibrate the camera with the image of the grid and its world description.
            MIL.McalGrid(MilCalibration, MilImage, GRID_ORG_OFFSET_X, GRID_ORG_OFFSET_Y, GRID_ORG_OFFSET_Z, GRID_ORG_ROW_NUMBER, GRID_ORG_COLUMN_NUMBER, GRID_ORG_ROW_SPACING, GRID_ORG_COLUMN_SPACING, MIL.M_DEFAULT, MIL.M_DEFAULT);

            // Verify if the camera calibration was successful
            MIL.McalInquire(MilCalibration, MIL.M_CALIBRATION_STATUS + MIL.M_TYPE_MIL_INT, ref CalibrationStatus);
            if (CalibrationStatus == MIL.M_CALIBRATED)
            {
                // Display the world absolute coordinate system
                MIL.MgraColor(MIL.M_DEFAULT, ABSOLUTE_COLOR);
                MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilOverlayImage, MIL.M_DRAW_ABSOLUTE_COORDINATE_SYSTEM + MIL.M_DRAW_AXES, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Print camera information
                Console.WriteLine("The camera has been calibrated.");
                Console.WriteLine("The world absolute coordinate system is shown in red.");
                Console.WriteLine();
                ShowCameraInformation(MilCalibration);

                // Load source image into an image buffer.
                MIL.MbufLoad(OBJECT_ORIGINAL_IMAGE_FILE, MilImage);

                // Associate calibration context to the image
                MIL.McalAssociate(MilCalibration, MilImage, MIL.M_DEFAULT);

                // Set the offset of the camera calibration plane.
                // This moves the relative origin to the top of the first metallic part 
                MIL.McalSetCoordinateSystem(MilImage, MIL.M_RELATIVE_COORDINATE_SYSTEM, MIL.M_ABSOLUTE_COORDINATE_SYSTEM, MIL.M_TRANSLATION + MIL.M_ASSIGN, MIL.M_NULL, 0, 0, -RING_THICKNESS, MIL.M_DEFAULT);

                // Display the world relative coordinate system
                MIL.MgraColor(MIL.M_DEFAULT, RELATIVE_COLOR);
                MIL.McalDraw(MIL.M_DEFAULT, MilImage, MilOverlayImage, MIL.M_DRAW_RELATIVE_COORDINATE_SYSTEM + MIL.M_DRAW_FRAME, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Measure the first circle.
                Console.WriteLine("The relative coordinate system (shown in green) was translated by {0:f3} cm", -RING_THICKNESS);
                Console.WriteLine("in z to align it with the top of the first metallic part.");
                MeasureRing(MilSystem, MilOverlayImage, MilImage, RING1_POS1_X, RING1_POS1_Y);
                Console.WriteLine("Press <Enter> to continue.");
                Console.WriteLine();
                Console.ReadKey();

                // Modify the offset of the camera calibration plane.
                // This moves the relative origin to the top of the second metallic part 
                MIL.McalSetCoordinateSystem(MilImage, MIL.M_RELATIVE_COORDINATE_SYSTEM, MIL.M_ABSOLUTE_COORDINATE_SYSTEM, MIL.M_TRANSLATION + MIL.M_COMPOSE_WITH_CURRENT, MIL.M_NULL, 0, 0, -STEP_THICKNESS, MIL.M_DEFAULT);

                // Clear the overlay
                MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT);
                /* Display the world absolute coordinate system */
                MIL.MgraColor(MIL.M_DEFAULT, ABSOLUTE_COLOR);
                MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilOverlayImage, MIL.M_DRAW_ABSOLUTE_COORDINATE_SYSTEM + MIL.M_DRAW_AXES, MIL.M_DEFAULT, MIL.M_DEFAULT);
                // Display the world relative coordinate system
                MIL.MgraColor(MIL.M_DEFAULT, RELATIVE_COLOR);
                MIL.McalDraw(MIL.M_DEFAULT, MilImage, MilOverlayImage, MIL.M_DRAW_RELATIVE_COORDINATE_SYSTEM + MIL.M_DRAW_FRAME, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Measure the second circle.
                Console.WriteLine("The relative coordinate system was translated by another {0:f3} cm", -STEP_THICKNESS);
                Console.WriteLine("in z to align it with the top of the second metallic part.");
                MeasureRing(MilSystem, MilOverlayImage, MilImage, RING2_POS1_X, RING2_POS1_Y);
                Console.WriteLine("Press <Enter> to continue.");
                Console.WriteLine();
                Console.ReadKey();
            }
            else
            {
                Console.WriteLine("Calibration generated an exception.");
                Console.WriteLine("See User Guide to resolve the situation.");
                Console.WriteLine();
            }

            // Free all allocations.
            MIL.McalFree(MilCalibration);
            MIL.MbufFree(MilImage);
        }

        // Measuring function with MilMetrology module
        static void MeasureRing(MIL_ID MilSystem, MIL_ID MilOverlayImage, MIL_ID MilImage, double MeasureRingX, double MeasureRingY)
        {
            MIL_ID MilMetrolContext = MIL.M_NULL;  // Metrology Context.
            MIL_ID MilMetrolResult = MIL.M_NULL;   // Metrology Result.

            double Value = 0.0;

            // Allocate metrology context and result.
            MIL.MmetAlloc(MilSystem, MIL.M_DEFAULT, ref MilMetrolContext);
            MIL.MmetAllocResult(MilSystem, MIL.M_DEFAULT, ref MilMetrolResult);

            // Add a first measured segment feature to context and set its search region.
            MIL.MmetAddFeature(MilMetrolContext, MIL.M_MEASURED, MIL.M_CIRCLE, MEASURED_CIRCLE_LABEL, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, 0, MIL.M_DEFAULT);

            MIL.MmetSetRegion(MilMetrolContext, MIL.M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), MIL.M_DEFAULT, MIL.M_RING, MeasureRingX, MeasureRingY, RING_START_RADIUS, RING_END_RADIUS, MIL.M_NULL, MIL.M_NULL);

            // Calculate.
            MIL.MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, MIL.M_DEFAULT);

            // Draw region.
            MIL.MgraColor(MIL.M_DEFAULT, REGION_COLOR);
            MIL.MmetDraw(MIL.M_DEFAULT, MilMetrolResult, MilOverlayImage, MIL.M_DRAW_REGION, MIL.M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), MIL.M_DEFAULT);

            // Draw features.
            MIL.MgraColor(MIL.M_DEFAULT, FEATURE_COLOR);
            MIL.MmetDraw(MIL.M_DEFAULT, MilMetrolResult, MilOverlayImage, MIL.M_DRAW_FEATURE, MIL.M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), MIL.M_DEFAULT);

            MIL.MmetGetResult(MilMetrolResult, MIL.M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), MIL.M_RADIUS, ref Value);
            Console.WriteLine("The large circle's radius was measured: {0:0.000} cm.", Value);

            // Free all allocations.
            MIL.MmetFree(MilMetrolResult);
            MIL.MmetFree(MilMetrolContext);
        }

        // Print the current camera position and orientation 
        static void ShowCameraInformation(MIL_ID MilCalibration)
        {
            double CameraPosX = 0.0;
            double CameraPosY = 0.0;
            double CameraPosZ = 0.0;
            double CameraYaw = 0.0;
            double CameraPitch = 0.0;
            double CameraRoll = 0.0;

            MIL.McalGetCoordinateSystem(MilCalibration, MIL.M_CAMERA_COORDINATE_SYSTEM, MIL.M_ABSOLUTE_COORDINATE_SYSTEM, MIL.M_TRANSLATION, MIL.M_NULL, ref CameraPosX, ref CameraPosY, ref CameraPosZ, MIL.M_NULL);
            MIL.McalGetCoordinateSystem(MilCalibration, MIL.M_CAMERA_COORDINATE_SYSTEM, MIL.M_ABSOLUTE_COORDINATE_SYSTEM, MIL.M_ROTATION_YXZ, MIL.M_NULL, ref CameraYaw, ref CameraPitch, ref CameraRoll, MIL.M_NULL);

            // Pause to show the corrected image of the grid.
            Console.WriteLine("Camera position in cm:          (x, y, z)           ({0:0.00}, {1:0.00}, {2:0.00})", CameraPosX, CameraPosY, CameraPosZ);
            Console.WriteLine("Camera orientation in degrees:  (yaw, pitch, roll)  ({0:0.00}, {1:0.00}, {2:0.00})", CameraYaw, CameraPitch, CameraRoll);
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();
        }


        //****************************************************************************
        // Partial grid example. 
        // ***************************************************************************
        // Source image files specification.
        private const string PARTIAL_GRID_IMAGE_FILE = MIL.M_IMAGE_PATH + "PartialGrid.mim";

        // Definition of the region to correct.
        private const double CORRECTED_SIZE_X = 60.0;
        private const double CORRECTED_SIZE_Y = 50.0;
        private const double CORRECTED_OFFSET_X = -35.0;
        private const double CORRECTED_OFFSET_Y = -5.0;
        private const int CORRECTED_IMAGE_SIZE_X = 512;

        static void PartialGridCalibration(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilImage = MIL.M_NULL;               // Image buffer identifier.
            MIL_ID MilCorrectedImage = MIL.M_NULL;      // Corrected image identifier.
            MIL_ID MilGraList = MIL.M_NULL;             // Graphic list identifier.
            MIL_ID MilCalibration = MIL.M_NULL;         // Calibration identifier.

            MIL_INT CalibrationStatus = 0;
            MIL_INT ImageType = 0;
            MIL_INT CorrectedImageSizeY = 0;
            double RowSpacing = 0.0;
            double ColumnSpacing = 0.0;
            double CorrectedPixelSize = 0.0;
            StringBuilder UnitName = new StringBuilder();


            // Clear the display
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT);

            // Allocate a graphic list and associate it to the display.
            MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref MilGraList);
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

            // Restore source image into an automatically allocated image buffer.
            MIL.MbufRestore(PARTIAL_GRID_IMAGE_FILE, MilSystem, ref MilImage);
            MIL.MbufInquire(MilImage, MIL.M_TYPE, ref ImageType);

            // Display the image buffer.
            MIL.MdispSelect(MilDisplay, MilImage);

            // Pause to show the partial grid image.
            Console.WriteLine();
            Console.WriteLine("PARTIAL GRID CALIBRATION:");
            Console.WriteLine("-------------------------");
            Console.WriteLine();
            Console.WriteLine("A camera will be calibrated using a rectangular grid that");
            Console.WriteLine("is only partially visible in the camera's field of view.");
            Console.WriteLine("The 2D code in the center is used as a fiducial to retrieve");
            Console.WriteLine("the characteristics of the calibration grid.");
            Console.WriteLine("Press <Enter> to continue.");
            Console.ReadKey();

            // Allocate the calibration object.
            MIL.McalAlloc(MilSystem, MIL.M_TSAI_BASED, MIL.M_DEFAULT, ref MilCalibration);

            // Set the calibration to calibrate a partial grid with fiducial.
            MIL.McalControl(MilCalibration, MIL.M_GRID_PARTIAL, MIL.M_ENABLE);
            MIL.McalControl(MilCalibration, MIL.M_GRID_FIDUCIAL, MIL.M_DATAMATRIX);

            // Calibrate the camera with the partial grid with fiducial.
            MIL.McalGrid(MilCalibration, MilImage,
                     GRID_OFFSET_X, GRID_OFFSET_Y, GRID_OFFSET_Z,
                     MIL.M_UNKNOWN, MIL.M_UNKNOWN, MIL.M_FROM_FIDUCIAL, MIL.M_FROM_FIDUCIAL,
                     MIL.M_DEFAULT, MIL.M_CHESSBOARD_GRID);

            MIL.McalInquire(MilCalibration, MIL.M_CALIBRATION_STATUS + MIL.M_TYPE_MIL_INT, ref CalibrationStatus);
            if (CalibrationStatus == MIL.M_CALIBRATED)
            {
                // Draw the absolute coordinate system.
                MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED);
                MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilGraList, MIL.M_DRAW_ABSOLUTE_COORDINATE_SYSTEM,
                    MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Draw a box around the fiducial.
                MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_CYAN);
                MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilGraList, MIL.M_DRAW_FIDUCIAL_BOX,
                   MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Get the information of the grid read from the fiducial.
                MIL.McalInquire(MilCalibration, MIL.M_ROW_SPACING, ref RowSpacing);
                MIL.McalInquire(MilCalibration, MIL.M_COLUMN_SPACING, ref ColumnSpacing);
                MIL.McalInquire(MilCalibration, MIL.M_GRID_UNIT_SHORT_NAME, UnitName);

                // Draw the information of the grid read from the fiducial.
                MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED);
                MIL.MgraControl(MIL.M_DEFAULT, MIL.M_INPUT_UNITS, MIL.M_DISPLAY);
                DrawGridInfo(MilGraList, "Row spacing", RowSpacing, 0, UnitName.ToString());
                DrawGridInfo(MilGraList, "Col spacing", ColumnSpacing, 1, UnitName.ToString());

                // Pause to show the calibration result.
                Console.WriteLine();
                Console.WriteLine("The camera has been calibrated.");
                Console.WriteLine();
                Console.WriteLine("The grid information read is displayed.");
                Console.WriteLine("Press <Enter> to continue.");
                Console.WriteLine();
                Console.ReadKey();

                // Calculate the pixel size and size Y of the corrected image.
                CorrectedPixelSize = CORRECTED_SIZE_X / CORRECTED_IMAGE_SIZE_X;
                CorrectedImageSizeY = (MIL_INT)(CORRECTED_SIZE_Y / CorrectedPixelSize);

                // Allocate the corrected image.
                MIL.MbufAlloc2d(MilSystem, CORRECTED_IMAGE_SIZE_X, CorrectedImageSizeY, ImageType,
                   MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP, ref MilCorrectedImage);

                // Calibrate the corrected image.
                MIL.McalUniform(MilCorrectedImage, CORRECTED_OFFSET_X, CORRECTED_OFFSET_Y,
                   CorrectedPixelSize, CorrectedPixelSize, 0.0, MIL.M_DEFAULT);

                // Correct the calibrated image.
                MIL.McalTransformImage(MilImage, MilCorrectedImage, MilCalibration,
                   MIL.M_BILINEAR + MIL.M_OVERSCAN_CLEAR, MIL.M_DEFAULT,
                   MIL.M_WARP_IMAGE + MIL.M_USE_DESTINATION_CALIBRATION);

                // Select the corrected image on the display.
                MIL.MgraClear(MIL.M_DEFAULT, MilGraList);
                MIL.MdispSelect(MilDisplay, MilCorrectedImage);

                // Pause to show the corrected image.
                Console.WriteLine("A sub-region of the grid was selected and transformed");
                Console.WriteLine("to remove the distortions.");
                Console.WriteLine("The sub-region dimensions and position are:");
                Console.WriteLine("   Size X  : {0,3:g3} {1}", CORRECTED_SIZE_X, UnitName);
                Console.WriteLine("   Size Y  : {0,3:g3} {1}", CORRECTED_SIZE_Y, UnitName);
                Console.WriteLine("   Offset X: {0,3:g3} {1}", CORRECTED_OFFSET_X, UnitName);
                Console.WriteLine("   Offset Y: {0,3:g3} {1}", CORRECTED_OFFSET_Y, UnitName);

                // Wait for a key to be pressed.
                Console.WriteLine("Press <Enter> to quit.");
                Console.WriteLine();
                Console.ReadKey();

                MIL.MbufFree(MilCorrectedImage);
            }
            else
            {
                Console.WriteLine("Calibration generated an exception.");
                Console.WriteLine("See User Guide to resolve the situation.");
                Console.WriteLine();
                Console.WriteLine("Press <Enter> to quit.");
                Console.WriteLine();
                Console.ReadKey();
            }

            // Free all allocations.
            MIL.McalFree(MilCalibration);
            MIL.MbufFree(MilImage);
            MIL.MgraFree(MilGraList);
        }

        // Definition of the parameters for the drawing of the grid info
        private const int LINE_HEIGHT = 16;

        // Draw an information of the grid.
        static void DrawGridInfo(MIL_ID MilGraList, string InfoTag, double Value, MIL_INT LineOffsetY, string Units)
        {
            string Info = string.Format("{0}: {1:g3} {2}", InfoTag, Value, Units);
            MIL.MgraText(MIL.M_DEFAULT, MilGraList, 0, LineOffsetY * LINE_HEIGHT, Info);
        }
    }
}
