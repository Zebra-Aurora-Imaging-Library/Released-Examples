﻿// *****************************************************************************
//
// File name: M3dmap.cs
//
// Synopsis: This program inspects a wood surface using
//           sheet-of-light profiling (laser) to find any depth defects.
//
// Printable calibration grids in PDF format can be found in your
// "Matrox Imaging/Images/" directory.
//
// When considering a laser-based 3D reconstruction system, the file "3D Setup Helper.xls"
// can be used to accelerate prototyping by choosing an adequate hardware configuration
// (angle, distance, lens, camera, ...). The file is located in your
// "Matrox Imaging/Tools/" directory.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
// *****************************************************************************
using System;
using System.Text;
using Matrox.MatroxImagingLibrary;

namespace M3dmap
{
    class Program
    {
        // *****************************************************************************
        // Main.
        // *****************************************************************************
        static int Main(string[] args)
        {
            MIL_ID MilApplication = MIL.M_NULL;     // Application identifier.
            MIL_ID MilSystem = MIL.M_NULL;          // System identifier.
            MIL_ID MilDisplay = MIL.M_NULL;         // Display identifier.

            // Allocate defaults.
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, ref MilDisplay, MIL.M_NULL, MIL.M_NULL);

            // Run the depth correction example.
            DepthCorrectionExample(MilSystem, MilDisplay);

            // Run the calibrated camera example.
            CalibratedCameraExample(MilSystem, MilDisplay);

            // Free defaults.
            MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL);

            return 0;
        }

        // *****************************************************************************
        // Depth correction example.
        // *****************************************************************************

        // Input sequence specifications.
        private const string REFERENCE_PLANES_SEQUENCE_FILE = MIL.M_IMAGE_PATH + "ReferencePlanes.avi";
        private const string OBJECT_SEQUENCE_FILE = MIL.M_IMAGE_PATH + "ScannedObject.avi";

        // Peak detection parameters.
        private const int PEAK_WIDTH_NOMINAL = 10;
        private const int PEAK_WIDTH_DELTA = 8;
        private const int MIN_CONTRAST = 140;

        // Calibration heights in mm.
        private static readonly double[] CORRECTED_DEPTHS = { 1.25, 2.50, 3.75, 5.00 };

        private const double SCALE_FACTOR = 10000.0; // (depth in world units) * SCALE_FACTOR gives gray levels

        // Annotation position.
        private const int CALIB_TEXT_POS_X = 400;
        private const int CALIB_TEXT_POS_Y = 15;

        private static void DepthCorrectionExample(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilOverlayImage = MIL.M_NULL;    // Overlay image buffer identifier.
            MIL_ID MilImage = MIL.M_NULL;           // Image buffer identifier (for processing).
            MIL_ID MilDepthMap = MIL.M_NULL;        // Image buffer identifier (for results).
            MIL_ID MilLaser = MIL.M_NULL;           // 3dmap laser profiling context identifier.
            MIL_ID MilCalibScan = MIL.M_NULL;       // 3dmap result buffer identifier for laser
                                                    // line calibration.
            MIL_ID MilScan = MIL.M_NULL;            // 3dmap result buffer identifier.

            MIL_INT SizeX = 0;                      // Width of grabbed images.
            MIL_INT SizeY = 0;                      // Height of grabbed images.
            MIL_INT NbReferencePlanes = 0;          // Number of reference planes of known heights.
            MIL_INT NbObjectImages = 0;             // Number of frames for scanned objects.
            int n = 0;                              // Counter.
            double FrameRate = 0.0;                 // Number of grabbed frames per second (in AVI).
            double StartTime = 0.0;                 // Time at the beginning of each iteration.
            double EndTime = 0.0;                   // Time after processing for each iteration.
            double WaitTime = 0.0;                  // Time to wait for next frame.

            // Inquire characteristics of the input sequences.
            MIL.MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_SIZE_X, ref SizeX);
            MIL.MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_SIZE_Y, ref SizeY);
            MIL.MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_NUMBER_OF_IMAGES, ref NbReferencePlanes);
            MIL.MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_FRAME_RATE, ref FrameRate);
            MIL.MbufDiskInquire(OBJECT_SEQUENCE_FILE, MIL.M_NUMBER_OF_IMAGES, ref NbObjectImages);

            // Allocate buffer to hold images.
            MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_DISP + MIL.M_PROC, ref MilImage);
            MIL.MbufClear(MilImage, 0.0);

            Console.WriteLine();
            Console.WriteLine("DEPTH ANALYSIS:");
            Console.WriteLine("---------------");
            Console.WriteLine();
            Console.WriteLine("This program performs a surface inspection to detect depth defects ");
            Console.WriteLine("on a wood surface using a laser (sheet-of-light) profiling system.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Select display.
            MIL.MdispSelect(MilDisplay, MilImage);

            // Prepare for overlay annotations.
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE);
            MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID, ref MilOverlayImage);
            MIL.MgraControl(MIL.M_DEFAULT, MIL.M_BACKGROUND_MODE, MIL.M_TRANSPARENT);
            MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_WHITE);

            // Allocate 3dmap objects.
            MIL.M3dmapAlloc(MilSystem, MIL.M_LASER, MIL.M_DEPTH_CORRECTION, ref MilLaser);
            MIL.M3dmapAllocResult(MilSystem, MIL.M_LASER_CALIBRATION_DATA, MIL.M_DEFAULT, ref MilCalibScan);

            // Set laser line extraction options.
            MIL_ID MilPeakLocator = MIL.M_NULL;
            MIL.M3dmapInquire(MilLaser, MIL.M_DEFAULT, MIL.M_LOCATE_PEAK_1D_CONTEXT_ID + MIL.M_TYPE_MIL_ID, ref MilPeakLocator);
            MIL.MimControl(MilPeakLocator, MIL.M_PEAK_WIDTH_NOMINAL, PEAK_WIDTH_NOMINAL);
            MIL.MimControl(MilPeakLocator, MIL.M_PEAK_WIDTH_DELTA, PEAK_WIDTH_DELTA);
            MIL.MimControl(MilPeakLocator, MIL.M_MINIMUM_CONTRAST, MIN_CONTRAST);

            // Open the calibration sequence file for reading.
            MIL.MbufImportSequence(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                            MIL.M_NULL, MIL.M_NULL, MIL.M_OPEN);

            // Read and process all images in the input sequence.
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref StartTime);

            for (n = 0; n < NbReferencePlanes; n++)
            {
                string CalibString;

                // Read image from sequence.
                MIL.MbufImportSequence(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_LOAD, MIL.M_NULL,
                                                                    ref MilImage, MIL.M_DEFAULT, 1, MIL.M_READ);

                // Annotate the image with the calibration height.
                MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT);
                CalibString = string.Format("Reference plane {0}: {1,0:f2} mm", (int)(n + 1), CORRECTED_DEPTHS[n]);
                MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, CALIB_TEXT_POS_X, CALIB_TEXT_POS_Y, CalibString);

                // Set desired corrected depth of next reference plane.
                MIL.M3dmapControl(MilLaser, MIL.M_DEFAULT, MIL.M_CORRECTED_DEPTH,
                                                                   CORRECTED_DEPTHS[n] * SCALE_FACTOR);

                // Analyze the image to extract laser line.
                MIL.M3dmapAddScan(MilLaser, MilCalibScan, MilImage, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Wait to have a proper frame rate, if necessary.
                MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref EndTime);
                WaitTime = (1.0 / FrameRate) - (EndTime - StartTime);
                if (WaitTime > 0)
                {
                    MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_WAIT, ref WaitTime);
                }

                MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref StartTime);
            }

            // Close the calibration sequence file.
            MIL.MbufImportSequence(REFERENCE_PLANES_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                           MIL.M_NULL, MIL.M_NULL, MIL.M_CLOSE);

            // Calibrate the laser profiling context using reference planes of known heights.
            MIL.M3dmapCalibrate(MilLaser, MilCalibScan, MIL.M_NULL, MIL.M_DEFAULT);

            Console.WriteLine("The laser profiling system has been calibrated using 4 reference");
            Console.WriteLine("planes of known heights.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            Console.WriteLine("The wood surface is being scanned.");
            Console.WriteLine();

            // Free the result buffer used for calibration because it will not be used anymore.
            MIL.M3dmapFree(MilCalibScan);
            MilCalibScan = MIL.M_NULL;

            // Allocate the result buffer for the scanned depth corrected data.
            MIL.M3dmapAllocResult(MilSystem, MIL.M_DEPTH_CORRECTED_DATA, MIL.M_DEFAULT, ref MilScan);

            // Open the object sequence file for reading.
            MIL.MbufDiskInquire(OBJECT_SEQUENCE_FILE, MIL.M_FRAME_RATE, ref FrameRate);
            MIL.MbufImportSequence(OBJECT_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                                    MIL.M_NULL, MIL.M_OPEN);

            // Read and process all images in the input sequence.
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref StartTime);
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT);

            for (n = 0; n < NbObjectImages; n++)
            {
                // Read image from sequence.
                MIL.MbufImportSequence(OBJECT_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_LOAD, MIL.M_NULL, ref MilImage,
                                                                               MIL.M_DEFAULT, 1, MIL.M_READ);

                // Analyze the image to extract laser line and correct its depth.
                MIL.M3dmapAddScan(MilLaser, MilScan, MilImage, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Wait to have a proper frame rate, if necessary.
                MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref EndTime);
                WaitTime = (1.0 / FrameRate) - (EndTime - StartTime);
                if (WaitTime > 0)
                {
                    MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_WAIT, ref WaitTime);
                }

                MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref StartTime);
            }

            // Close the object sequence file.
            MIL.MbufImportSequence(OBJECT_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                                   MIL.M_NULL, MIL.M_CLOSE);

            // Allocate the image for a partially corrected depth map.
            MIL.MbufAlloc2d(MilSystem, SizeX, NbObjectImages, 16 + MIL.M_UNSIGNED,
                                                                MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP, ref MilDepthMap);

            // Get partially corrected depth map from accumulated information in the result buffer.
            MIL.M3dmapCopyResult(MilScan, MIL.M_DEFAULT, MilDepthMap, MIL.M_PARTIALLY_CORRECTED_DEPTH_MAP, MIL.M_DEFAULT);

            // Disable display updates.
            MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_DISABLE);

            // Show partially corrected depth map and find defects.
            SetupColorDisplay(MilSystem, MilDisplay, MIL.MbufInquire(MilDepthMap, MIL.M_SIZE_BIT, MIL.M_NULL));

            // Display partially corrected depth map.
            MIL.MdispSelect(MilDisplay, MilDepthMap);
            MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_ENABLE);
            MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID, ref MilOverlayImage);

            Console.WriteLine("The pseudo-color depth map of the surface is displayed.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            PerformBlobAnalysis(MilSystem, MilDisplay, MilOverlayImage, MilDepthMap);

            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Disassociates display LUT and clear overlay.
            MIL.MdispSelect(MilDisplay, MIL.M_NULL);
            MIL.MdispLut(MilDisplay, MIL.M_DEFAULT);
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT);

            // Free all allocations.
            MIL.M3dmapFree(MilScan);
            MIL.M3dmapFree(MilLaser);
            MIL.MbufFree(MilDepthMap);
            MIL.MbufFree(MilImage);
        }

        // Values used for binarization.
        private const double EXPECTED_HEIGHT = 3.4;    // Inspected surface should be at this height (in mm)
        private const double DEFECT_THRESHOLD = 0.2;   // Max acceptable deviation from expected height (mm)
        private const double SATURATED_DEFECT = 1.0;   // Deviation at which defect will appear red (in mm)

        // Radius of the smallest particles to keep.
        private const int MIN_BLOB_RADIUS = 3;

        // Pixel offset for drawing text.
        private const int TEXT_H_OFFSET_1 = -50;
        private const int TEXT_V_OFFSET_1 = -6;
        private const int TEXT_H_OFFSET_2 = -30;
        private const int TEXT_V_OFFSET_2 = 6;

        // Find defects in corrected depth map, compute max deviation and draw contours.
        private static void PerformBlobAnalysis(MIL_ID MilSystem,
                                 MIL_ID MilDisplay,
                                 MIL_ID MilOverlayImage,
                                 MIL_ID MilDepthMap)
        {
            MIL_ID MilBinImage = MIL.M_NULL;        // Binary image buffer identifier.
            MIL_ID MilBlobContext = MIL.M_NULL;     // Blob context identifier.
            MIL_ID MilBlobResult = MIL.M_NULL;      // Blob result buffer identifier.
            MIL_INT SizeX = 0;                      // Width of depth map.
            MIL_INT SizeY = 0;                      // Height of depth map.
            MIL_INT TotalBlobs = 0;                 // Total number of blobs.
            MIL_INT n = 0;                          // Counter.
            MIL_INT[] MinPixels;                    // Maximum height of defects.
            double DefectThreshold = 0.0;           // A gray level below it is a defect.
            double[] CogX;                          // X coordinate of center of gravity.
            double[] CogY;                          // Y coordinate of center of gravity.

            // Get size of depth map.
            MIL.MbufInquire(MilDepthMap, MIL.M_SIZE_X, ref SizeX);
            MIL.MbufInquire(MilDepthMap, MIL.M_SIZE_Y, ref SizeY);

            // Allocate a binary image buffer for fast processing.
            MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC, ref MilBinImage);

            // Binarize image.
            DefectThreshold = (EXPECTED_HEIGHT - DEFECT_THRESHOLD) * SCALE_FACTOR;
            MIL.MimBinarize(MilDepthMap, MilBinImage, MIL.M_FIXED + MIL.M_LESS_OR_EQUAL, DefectThreshold, MIL.M_NULL);

            // Remove small particles.
            MIL.MimOpen(MilBinImage, MilBinImage, MIN_BLOB_RADIUS, MIL.M_BINARY);

            // Allocate a blob context.
            MIL.MblobAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilBlobContext);

            // Enable the Center Of Gravity and Min Pixel features calculation.
            MIL.MblobControl(MilBlobContext, MIL.M_CENTER_OF_GRAVITY + MIL.M_GRAYSCALE, MIL.M_ENABLE);
            MIL.MblobControl(MilBlobContext, MIL.M_MIN_PIXEL, MIL.M_ENABLE);

            // Allocate a blob result buffer.
            MIL.MblobAllocResult(MilSystem, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilBlobResult);

            // Calculate selected features for each blob.
            MIL.MblobCalculate(MilBlobContext, MilBinImage, MilDepthMap, MilBlobResult);

            // Get the total number of selected blobs.
            MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT, ref TotalBlobs);
            Console.WriteLine("Number of defects: {0}", TotalBlobs);

            // Read and print the blob characteristics.
            CogX = new double[TotalBlobs];
            CogY = new double[TotalBlobs];
            MinPixels = new MIL_INT[TotalBlobs];
            if (CogX != null && CogY != null && MinPixels != null)
            {
                // Get the results.
                MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_CENTER_OF_GRAVITY_X + MIL.M_GRAYSCALE, CogX);
                MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_CENTER_OF_GRAVITY_Y + MIL.M_GRAYSCALE, CogY);
                MIL.MblobGetResult(MilBlobResult, MIL.M_DEFAULT, MIL.M_MIN_PIXEL + MIL.M_TYPE_MIL_INT, MinPixels);

                // Draw the defects.
                MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED);
                MIL.MblobDraw(MIL.M_DEFAULT, MilBlobResult, MilOverlayImage,
                          MIL.M_DRAW_BLOBS, MIL.M_INCLUDED_BLOBS, MIL.M_DEFAULT);
                MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_WHITE);

                // Print the depth of each blob.
                for (n = 0; n < TotalBlobs; n++)
                {
                    double DepthOfDefect;
                    string DepthString;

                    // Write the depth of the defect in the overlay.
                    DepthOfDefect = EXPECTED_HEIGHT - (MinPixels[n] / SCALE_FACTOR);
                    DepthString = string.Format("{0,0:f2} mm", DepthOfDefect);

                    Console.WriteLine("Defect #{0}: depth ={1,5:f2} mm", n, DepthOfDefect);
                    Console.WriteLine();
                    MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, CogX[n] + TEXT_H_OFFSET_1,
                                                   CogY[n] + TEXT_V_OFFSET_1, "Defect depth");
                    MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, CogX[n] + TEXT_H_OFFSET_2,
                                                                CogY[n] + TEXT_V_OFFSET_2, DepthString);
                }

            }
            else
            {
                Console.WriteLine("Error: Not enough memory.");
                Console.WriteLine();
            }

            // Free all allocations.
            MIL.MblobFree(MilBlobResult);
            MIL.MblobFree(MilBlobContext);
            MIL.MbufFree(MilBinImage);
        }

        // Color constants for display LUT.
        private const double BLUE_HUE = 171.0;      // Expected depths will be blue.
        private const double RED_HUE = 0.0;         // Worst defects will be red.
        private const int FULL_SATURATION = 255;    // All colors are fully saturated.
        private const int HALF_LUMINANCE = 128;     // All colors have half luminance.

        // Creates a color display LUT to show defects in red.
        private static void SetupColorDisplay(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_INT SizeBit)
        {
            MIL_ID MilRampLut1Band = MIL.M_NULL;    // LUT containing hue values.
            MIL_ID MilRampLut3Band = MIL.M_NULL;    // RGB LUT used by display.
            MIL_ID MilColorImage = MIL.M_NULL;      // Image used for HSL to RGB conversion.
            MIL_INT DefectGrayLevel = 0;            // Gray level under which all is red.
            MIL_INT ExpectedGrayLevel = 0;          // Gray level over which all is blue.
            MIL_INT NbGrayLevels = 0;

            // Number of possible gray levels in corrected depth map.
            NbGrayLevels = (MIL_INT)(1 << (int)SizeBit);

            // Allocate 1-band LUT that will contain hue values.
            MIL.MbufAlloc1d(MilSystem, NbGrayLevels, 8 + MIL.M_UNSIGNED, MIL.M_LUT, ref MilRampLut1Band);

            // Compute limit gray values.
            DefectGrayLevel = (MIL_INT)((EXPECTED_HEIGHT - SATURATED_DEFECT) * SCALE_FACTOR);
            ExpectedGrayLevel = (MIL_INT)(EXPECTED_HEIGHT * SCALE_FACTOR);

            // Create hue values for each possible gray level.
            MIL.MgenLutRamp(MilRampLut1Band, 0, RED_HUE, DefectGrayLevel, RED_HUE);
            MIL.MgenLutRamp(MilRampLut1Band, DefectGrayLevel, RED_HUE, ExpectedGrayLevel, BLUE_HUE);
            MIL.MgenLutRamp(MilRampLut1Band, ExpectedGrayLevel, BLUE_HUE, NbGrayLevels - 1, BLUE_HUE);

            // Create a HSL image buffer.
            MIL.MbufAllocColor(MilSystem, 3, NbGrayLevels, 1, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE, ref MilColorImage);
            MIL.MbufClear(MilColorImage, MIL.M_RGB888(0, FULL_SATURATION, HALF_LUMINANCE));

            // Set its H band (hue) to the LUT contents and convert the image to RGB.
            MIL.MbufCopyColor2d(MilRampLut1Band, MilColorImage, 0, 0, 0, 0, 0, 0, NbGrayLevels, 1);
            MIL.MimConvert(MilColorImage, MilColorImage, MIL.M_HSL_TO_RGB);

            // Create RGB LUT to give to display and copy image contents.
            MIL.MbufAllocColor(MilSystem, 3, NbGrayLevels, 1, 8 + MIL.M_UNSIGNED, MIL.M_LUT, ref MilRampLut3Band);
            MIL.MbufCopy(MilColorImage, MilRampLut3Band);

            // Associates LUT to display.
            MIL.MdispLut(MilDisplay, MilRampLut3Band);

            // Free all allocations.
            MIL.MbufFree(MilRampLut1Band);
            MIL.MbufFree(MilRampLut3Band);
            MIL.MbufFree(MilColorImage);
        }

        // *****************************************************************************
        // Calibrated camera example.
        // *****************************************************************************

        // Input sequence specifications.
        private const string GRID_FILENAME = MIL.M_IMAGE_PATH + "GridForLaser.mim";
        private const string LASERLINE_FILENAME = MIL.M_IMAGE_PATH + "LaserLine.mim";
        private const string OBJECT2_SEQUENCE_FILE = MIL.M_IMAGE_PATH + "Cookie.avi";

        // Camera calibration grid parameters.
        private const int GRID_NB_ROWS = 13;
        private const int GRID_NB_COLS = 12;
        private const double GRID_ROW_SPACING = 5.0;        // in mm
        private const double GRID_COL_SPACING = 5.0;        // in mm

        // Laser device setup parameters.
        private const double CONVEYOR_SPEED = -0.2;         // in mm/frame

        // Fully corrected depth map generation parameters.
        private const int DEPTH_MAP_SIZE_X = 480;           // in pixels
        private const int DEPTH_MAP_SIZE_Y = 480;           // in pixels
        private const double GAP_DEPTH = 1.5;               // in mm

        // D3D display parameters
        private const int D3D_DISPLAY_SIZE_X = 640;
        private const int D3D_DISPLAY_SIZE_Y = 480;

        // Peak detection parameters.
        private const int PEAK_WIDTH_NOMINAL_2 = 9;
        private const int PEAK_WIDTH_DELTA_2 = 7;
        private const int MIN_CONTRAST_2 = 75;

        // Everything below this is considered as noise.
        private const double MIN_HEIGHT_THRESHOLD = 1.0;    // in mm

        private static void CalibratedCameraExample(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilOverlayImage = MIL.M_NULL;    // Overlay image buffer identifier.
            MIL_ID MilImage = MIL.M_NULL;           // Image buffer identifier (for processing).
            MIL_ID MilCalibration = MIL.M_NULL;     // Calibration context.
            MIL_ID MilDepthMap = MIL.M_NULL;        // Image buffer identifier (for results).
            MIL_ID MilLaser = MIL.M_NULL;           // 3dmap laser profiling context identifier.
            MIL_ID MilCalibScan = MIL.M_NULL;       // 3dmap result buffer identifier for laser
                                                    // line calibration.
            MIL_ID MilScan = MIL.M_NULL;            // 3map result buffer identifier.
            MIL_ID MilContainerId = MIL.M_NULL;     // Point cloud container identifier.
            MIL_ID FillGapsContext = MIL.M_NULL;    // Fill gaps context identifier.
            MIL_INT CalibrationStatus = 0;          // Used to ensure if MIL.McalGrid() worked.
            MIL_INT SizeX = 0;                      // Width of grabbed images.
            MIL_INT SizeY = 0;                      // Height of grabbed images.
            MIL_INT NumberOfImages = 0;             // Number of frames for scanned objects.
            MIL_INT n = 0;                          // Counter.
            double FrameRate = 0.0;                 // Number of grabbed frames per second (in AVI).
            double StartTime = 0.0;                 // Time at the beginning of each iteration.
            double EndTime = 0.0;                   // Time after processing for each iteration.
            double WaitTime = 0.0;                  // Time to wait for next frame.
            double Volume = 0.0;                    // Volume of scanned object.


            Console.WriteLine();
            Console.WriteLine("3D PROFILING AND VOLUME ANALYSIS:");
            Console.WriteLine("---------------------------------");
            Console.WriteLine();
            Console.WriteLine("This program generates fully corrected 3D data of a");
            Console.WriteLine("scanned cookie and computes its volume.");
            Console.WriteLine("The laser (sheet-of-light) profiling system uses a");
            Console.WriteLine("3d-calibrated camera.");
            Console.WriteLine();

            // Load grid image for camera calibration.
            MIL.MbufRestore(GRID_FILENAME, MilSystem, ref MilImage);

            // Select display.
            MIL.MdispSelect(MilDisplay, MilImage);

            Console.WriteLine("Calibrating the camera...");
            Console.WriteLine();

            MIL.MbufInquire(MilImage, MIL.M_SIZE_X, ref SizeX);
            MIL.MbufInquire(MilImage, MIL.M_SIZE_Y, ref SizeY);

            // Allocate calibration context in 3D mode.
            MIL.McalAlloc(MilSystem, MIL.M_TSAI_BASED, MIL.M_DEFAULT, ref MilCalibration);

            // Calibrate the camera.
            MIL.McalGrid(MilCalibration, MilImage, 0.0, 0.0, 0.0, GRID_NB_ROWS, GRID_NB_COLS,
                     GRID_ROW_SPACING, GRID_COL_SPACING, MIL.M_DEFAULT, MIL.M_CHESSBOARD_GRID);

            MIL.McalInquire(MilCalibration, MIL.M_CALIBRATION_STATUS + MIL.M_TYPE_MIL_INT, ref CalibrationStatus);
            if (CalibrationStatus != MIL.M_CALIBRATED)
            {
                MIL.McalFree(MilCalibration);
                MIL.MbufFree(MilImage);
                Console.WriteLine("Camera calibration failed.");
                Console.WriteLine("Press <Enter> to end.");
                Console.WriteLine();
                Console.ReadKey();
                return;
            }

            // Prepare for overlay annotations.
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE);
            MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID, ref MilOverlayImage);
            MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN);

            // Draw camera calibration points.
            MIL.McalDraw(MIL.M_DEFAULT, MilCalibration, MilOverlayImage, MIL.M_DRAW_IMAGE_POINTS,
                                                                              MIL.M_DEFAULT, MIL.M_DEFAULT);

            Console.WriteLine("The camera was calibrated using a chessboard grid.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Disable overlay.
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_DISABLE);

            // Load laser line image.
            MIL.MbufLoad(LASERLINE_FILENAME, MilImage);

            // Allocate 3dmap objects.
            MIL.M3dmapAlloc(MilSystem, MIL.M_LASER, MIL.M_CALIBRATED_CAMERA_LINEAR_MOTION, ref MilLaser);
            MIL.M3dmapAllocResult(MilSystem, MIL.M_LASER_CALIBRATION_DATA, MIL.M_DEFAULT, ref MilCalibScan);

            // Set laser line extraction options.
            MIL_ID MilPeakLocator = MIL.M_NULL;
            MIL.M3dmapInquire(MilLaser, MIL.M_DEFAULT, MIL.M_LOCATE_PEAK_1D_CONTEXT_ID + MIL.M_TYPE_MIL_ID, ref MilPeakLocator);
            MIL.MimControl(MilPeakLocator, MIL.M_PEAK_WIDTH_NOMINAL, PEAK_WIDTH_NOMINAL_2);
            MIL.MimControl(MilPeakLocator, MIL.M_PEAK_WIDTH_DELTA, PEAK_WIDTH_DELTA_2);
            MIL.MimControl(MilPeakLocator, MIL.M_MINIMUM_CONTRAST, MIN_CONTRAST_2);

            // Calibrate laser profiling context.
            MIL.M3dmapAddScan(MilLaser, MilCalibScan, MilImage, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT);
            MIL.M3dmapCalibrate(MilLaser, MilCalibScan, MilCalibration, MIL.M_DEFAULT);

            Console.WriteLine("The laser profiling system has been calibrated using the image");
            Console.WriteLine("of one laser line.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Free the result buffer use for calibration as it will not be used anymore.
            MIL.M3dmapFree(MilCalibScan);
            MilCalibScan = MIL.M_NULL;

            // Allocate the result buffer to hold the scanned 3D points.
            MIL.M3dmapAllocResult(MilSystem, MIL.M_POINT_CLOUD_RESULT, MIL.M_DEFAULT, ref MilScan);

            // Set speed of scanned object (speed in mm/frame is constant).
            MIL.M3dmapControl(MilLaser, MIL.M_DEFAULT, MIL.M_SCAN_SPEED, CONVEYOR_SPEED);

            // Inquire characteristics of the input sequence.
            MIL.MbufDiskInquire(OBJECT2_SEQUENCE_FILE, MIL.M_NUMBER_OF_IMAGES, ref NumberOfImages);
            MIL.MbufDiskInquire(OBJECT2_SEQUENCE_FILE, MIL.M_FRAME_RATE, ref FrameRate);

            // Open the object sequence file for reading.
            MIL.MbufImportSequence(OBJECT2_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                                    MIL.M_NULL, MIL.M_OPEN);

            Console.WriteLine("The cookie is being scanned to generate 3D data.");
            Console.WriteLine();

            // Read and process all images in the input sequence.
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref StartTime);

            for (n = 0; n < NumberOfImages; n++)
            {
                // Read image from sequence.
                MIL.MbufImportSequence(OBJECT2_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_LOAD, MIL.M_NULL, ref MilImage,
                                                                               MIL.M_DEFAULT, 1, MIL.M_READ);

                // Analyze the image to extract laser line and correct its depth.
                MIL.M3dmapAddScan(MilLaser, MilScan, MilImage, MIL.M_NULL, MIL.M_NULL, MIL.M_POINT_CLOUD_LABEL(1), MIL.M_DEFAULT);

                // Wait to have a proper frame rate, if necessary.
                MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref EndTime);
                WaitTime = (1.0 / FrameRate) - (EndTime - StartTime);
                if (WaitTime > 0)
                {
                    MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_WAIT, ref WaitTime);
                }

                MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref StartTime);
            }

            // Close the object sequence file.
            MIL.MbufImportSequence(OBJECT2_SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL,
                                                                                   MIL.M_NULL, MIL.M_CLOSE);



            // Convert to MIL.M_CONTAINER for 3D processing.
            MIL.MbufAllocContainer(MilSystem, MIL.M_PROC | MIL.M_DISP, MIL.M_DEFAULT, ref MilContainerId);
            MIL.M3dmapCopyResult(MilScan, MIL.M_ALL, MilContainerId, MIL.M_POINT_CLOUD_UNORGANIZED, MIL.M_DEFAULT);

            // The container's reflectance is 16bits, but only uses the bottom 8.Set the maximum value to display it properly.
            MIL.MbufControlContainer(MilContainerId, MIL.M_COMPONENT_REFLECTANCE, MIL.M_MAX, 255);

            // Allocate image for the fully corrected depth map.
            MIL.MbufAlloc2d(MilSystem, DEPTH_MAP_SIZE_X, DEPTH_MAP_SIZE_Y, 16 + MIL.M_UNSIGNED,
                        MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP, ref MilDepthMap);

            // Include all points during depth map generation.
            MIL.M3dimCalibrateDepthMap(MilContainerId, MilDepthMap, MIL.M_NULL, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_NEGATIVE, MIL.M_DEFAULT);

            // Remove noise in the container close to the Z = 0.
            MIL_ID MilPlane = MIL.M3dgeoAlloc(MilSystem, MIL.M_GEOMETRY, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dgeoPlane(MilPlane, MIL.M_COEFFICIENTS, 0.0, 0.0, 1.0, MIN_HEIGHT_THRESHOLD, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            // MIL.M_INVERSE remove what is above the plane.
            MIL.M3dimCrop(MilContainerId, MilContainerId, MilPlane, MIL.M_NULL, MIL.M_SAME, MIL.M_INVERSE);
            MIL.M3dgeoFree(MilPlane);

            Console.WriteLine("Fully corrected 3D data of the cookie is displayed.");
            Console.WriteLine();

            MIL_ID M3dDisplay = Alloc3dDisplayId(MilSystem);
            if (M3dDisplay != MIL.M_NULL)
            {
                Console.WriteLine("Press <R> on the display window to stop/start the rotation.");
                Console.WriteLine();
                MIL.M3ddispSelect(M3dDisplay, MilContainerId, MIL.M_SELECT, MIL.M_DEFAULT);
                MIL.M3ddispSetView(M3dDisplay, MIL.M_AUTO, MIL.M_BOTTOM_TILTED, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
                MIL.M3ddispControl(M3dDisplay, MIL.M_AUTO_ROTATE, MIL.M_ENABLE);
            }

            // Get fully corrected depth map from accumulated information in the result buffer.
            MIL.M3dimProject(MilContainerId, MilDepthMap, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_MIN_Z, MIL.M_DEFAULT, MIL.M_DEFAULT);

            // Set fill gaps parameters.
            MIL.M3dimAlloc(MilSystem, MIL.M_FILL_GAPS_CONTEXT, MIL.M_DEFAULT, ref FillGapsContext);
            MIL.M3dimControl(FillGapsContext, MIL.M_FILL_MODE, MIL.M_X_THEN_Y);
            MIL.M3dimControl(FillGapsContext, MIL.M_FILL_SHARP_ELEVATION, MIL.M_MIN);
            MIL.M3dimControl(FillGapsContext, MIL.M_FILL_SHARP_ELEVATION_DEPTH, GAP_DEPTH);
            MIL.M3dimControl(FillGapsContext, MIL.M_FILL_BORDER, MIL.M_DISABLE);

            MIL.M3dimFillGaps(FillGapsContext, MilDepthMap, MIL.M_NULL, MIL.M_DEFAULT);

            // Compute the volume of the depth map.
            MIL.M3dmetVolume(MilDepthMap, MIL.M_XY_PLANE, MIL.M_TOTAL, MIL.M_DEFAULT, ref Volume, MIL.M_NULL);

            Console.WriteLine("Volume of the cookie is {0,4:f1} cm^3.", Volume / 1000.0);
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to end.");
            Console.WriteLine();
            Console.ReadKey();

            if (M3dDisplay != MIL.M_NULL)
            {
                MIL.M3ddispFree(M3dDisplay);
            }

            // Free all allocations.
            MIL.M3dimFree(FillGapsContext);
            MIL.MbufFree(MilContainerId);
            MIL.M3dmapFree(MilScan);
            MIL.M3dmapFree(MilLaser);
            MIL.McalFree(MilCalibration);
            MIL.MbufFree(MilDepthMap);
            MIL.MbufFree(MilImage);
        }

        // *****************************************************************************
        // Allocates a 3D display and returns its MIL identifier.
        // *****************************************************************************
        private static MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
        {
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);
            MIL_ID MilDisplay3D = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, MIL.M_NULL);
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);

            if (MilDisplay3D == MIL.M_NULL)
            {
                Console.WriteLine();
                Console.WriteLine("The current system does not support the 3D display.");
                Console.WriteLine();
            }
            return MilDisplay3D;
        }
    }
}


