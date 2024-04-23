﻿// ***************************************************************************
//
// File name: MImLocatePeak1d.cs
//
// Synopsis:  This program finds the peak in each column of an input sequence
//            and reconstruct the height of a 3D object using it.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
// ***************************************************************************

using System;
using System.Text;
using System.Threading;

using Matrox.MatroxImagingLibrary;

namespace MimLocatePeak1d
{
    class Program
    {
        // Input sequence specifications.
        private const string SEQUENCE_FILE = MIL.M_IMAGE_PATH + "HandWithLaser.avi";

        //     ^            +
        //     |        +       +
        //     |      + <-Width-> + <------------
        //     |     +             +             | Min contrast
        //     | ++++               ++++++++ <---
        //     |
        //     |
        //     ------------------------------>
        //        Peak intensity profile

        // Peak detection parameters.
        private const int LINE_WIDTH_AVERAGE = 20;
        private const int LINE_WIDTH_DELTA = 20;
        private const double MIN_CONTRAST = 100.0;
        private const int NB_FIXED_POINT = 4;

        // M3D display parameters
        private const double M3D_MESH_SCALING_X = 1.0;
        private const double M3D_MESH_SCALING_Y = 4.0;
        private const double M3D_MESH_SCALING_Z = -0.13;

        static int Main(string[] args)
        {
            MIL_ID MilApplication = MIL.M_NULL;          // Application identifier.
            MIL_ID MilSystem = MIL.M_NULL;               // System identifier.
            MIL_ID MilDisplay = MIL.M_NULL;              // Display identifier.
            MIL_ID MilDisplayImage = MIL.M_NULL;         // Image buffer identifier.
            MIL_ID MilGraList = MIL.M_NULL;              // Graphic list identifier.
            MIL_ID MilImage = MIL.M_NULL;                // Image buffer identifier.
            MIL_ID MilPosYImage = MIL.M_NULL;            // Image buffer identifier.
            MIL_ID MilValImage = MIL.M_NULL;             // Image buffer identifier.
            MIL_ID MilContext = MIL.M_NULL;              // Processing context identifier.
            MIL_ID MilLocatePeak = MIL.M_NULL;           // Processing result identifier.
            MIL_ID MilStatContext = MIL.M_NULL;          // Statistics context identifier.
            MIL_ID MilExtreme = MIL.M_NULL;              // Result buffer identifier.

            MIL_INT SizeX = 0;
            MIL_INT SizeY = 0;
            MIL_INT NumberOfImages = 0;
            double FrameRate = 0.0;
            MIL_INT n = 0;
            double PreviousTime = 0.0;
            double StartTime = 0.0;
            double EndTime = 0.0;
            double TotalProcessTime = 0.0;
            double WaitTime = 0.0;
            MIL_INT[] ExtremePosY = new MIL_INT[] { 0, 0 };

            // Allocate defaults.
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, ref MilDisplay, MIL.M_NULL, MIL.M_NULL);

            // Inquire characteristics of the input sequence.
            MIL.MbufDiskInquire(SEQUENCE_FILE, MIL.M_SIZE_X, ref SizeX);
            MIL.MbufDiskInquire(SEQUENCE_FILE, MIL.M_SIZE_Y, ref SizeY);
            MIL.MbufDiskInquire(SEQUENCE_FILE, MIL.M_NUMBER_OF_IMAGES, ref NumberOfImages);
            MIL.MbufDiskInquire(SEQUENCE_FILE, MIL.M_FRAME_RATE, ref FrameRate);

            // Allocate buffers to hold images.
            MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC, ref MilImage);
            MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_DISP, ref MilDisplayImage);
            MIL.MbufAlloc2d(MilSystem, SizeX, NumberOfImages, 16 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC, ref MilPosYImage);
            MIL.MbufAlloc2d(MilSystem, SizeX, NumberOfImages, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC, ref MilValImage);

            // Allocate context for MIL.MimLocatePeak1D
            MIL.MimAlloc(MilSystem, MIL.M_LOCATE_PEAK_1D_CONTEXT, MIL.M_DEFAULT, ref MilContext);

            // Allocate result for MIL.MimLocatePeak1D
            MIL.MimAllocResult(MilSystem, MIL.M_DEFAULT, MIL.M_LOCATE_PEAK_1D_RESULT, ref MilLocatePeak);

            // Allocate graphic list.
            MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref MilGraList);

            // Select display.
            MIL.MdispSelect(MilDisplay, MilDisplayImage);
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

            // Print a message.
            Console.WriteLine();
            Console.WriteLine("EXTRACTING 3D IMAGE FROM A LASER LINE (SHEET-OF-LIGHT):");
            Console.WriteLine("--------------------------------------------------------");
            Console.WriteLine();
            Console.WriteLine("The position of a laser line is being extracted from an image");
            Console.WriteLine("to generate a depth image.");

            // Open the sequence file for reading.
            MIL.MbufImportSequence(SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_OPEN);

            // Preprocess the context.
            MIL.MimLocatePeak1d(MilContext, MilImage, MilLocatePeak, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_PREPROCESS, MIL.M_DEFAULT);

            // Read and process all images in the input sequence.
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref PreviousTime);
            TotalProcessTime = 0.0;

            for (n = 0; n < NumberOfImages; n++)
            {
                // Read image from sequence.
                MIL.MbufImportSequence(SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_LOAD, MIL.M_NULL, ref MilImage, MIL.M_DEFAULT, 1, MIL.M_READ);

                // Display the image.
                MIL.MbufCopy(MilImage, MilDisplayImage);

                // Locate the peak in each column of the image.
                MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref StartTime);

                MIL.MimLocatePeak1d(MilContext, MilImage, MilLocatePeak, LINE_WIDTH_AVERAGE, LINE_WIDTH_DELTA, MIN_CONTRAST, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Draw extracted peaks.
                MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED);
                MIL.MgraClear(MIL.M_DEFAULT, MilGraList);
                MIL.MimDraw(MIL.M_DEFAULT, MilLocatePeak, MIL.M_NULL, MilGraList, MIL.M_DRAW_PEAKS + MIL.M_CROSS, MIL.M_ALL, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Draw peak's data to depth map.
                MIL.MimDraw(MIL.M_DEFAULT, MilLocatePeak, MIL.M_NULL, MilPosYImage, MIL.M_DRAW_DEPTH_MAP_ROW, n, MIL.M_NULL, MIL.M_FIXED_POINT + NB_FIXED_POINT);
                MIL.MimDraw(MIL.M_DEFAULT, MilLocatePeak, MIL.M_NULL, MilValImage, MIL.M_DRAW_INTENSITY_MAP_ROW, n, MIL.M_NULL, MIL.M_FIXED_POINT + NB_FIXED_POINT);

                MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref EndTime);
                TotalProcessTime += EndTime - StartTime;

                // Wait to have a proper frame rate.
                WaitTime = (1.0 / FrameRate) - (EndTime - PreviousTime);
                if (WaitTime > 0)
                {
                    MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_WAIT, ref WaitTime);
                }
                MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref PreviousTime);
            }

            MIL.MgraClear(MIL.M_DEFAULT, MilGraList);

            // Close the sequence file.
            MIL.MbufImportSequence(SEQUENCE_FILE, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL, MIL.M_CLOSE);

            Console.WriteLine("{0} images processed in {1,7:f2} s ({2,7:f2} ms/image).",
               (int)NumberOfImages, TotalProcessTime,
               TotalProcessTime / NumberOfImages * 1000.0);

            // Pause to show the result.
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            Console.WriteLine("The reconstructed images are being displayed.");

            // Draw extracted peak position in each column of each image.
            int VisualizationDelayMsec = 10;
            for (n = 0; n < NumberOfImages; n++)
            {
                // Display the result image.
                MIL.MbufClear(MilImage, 0);
                MIL.MimDraw(MIL.M_DEFAULT, MilPosYImage, MilValImage, MilImage, MIL.M_DRAW_PEAKS + MIL.M_1D_COLUMNS + MIL.M_LINES, n, 1, MIL.M_FIXED_POINT + NB_FIXED_POINT);
                MIL.MbufCopy(MilImage, MilDisplayImage);

                Thread.Sleep(VisualizationDelayMsec);
            }

            // Pause to show the result.
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Try to allocate 3D display.
            MIL_ID MilDisplay3D = Alloc3dDisplayId(MilSystem);
            if (MilDisplay3D != MIL.M_NULL)
            {
                MIL.McalUniform(MilPosYImage, 0.0, 0.0, M3D_MESH_SCALING_X, M3D_MESH_SCALING_Y, 0.0, MIL.M_DEFAULT);
                MIL.McalControl(MilPosYImage, MIL.M_GRAY_LEVEL_SIZE_Z, M3D_MESH_SCALING_Z);
                MIL_ID ContainerId = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC | MIL.M_DISP, MIL.M_DEFAULT, MIL.M_NULL);
                MIL.MbufConvert3d(MilPosYImage, ContainerId, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT);
                MIL_ID Reflectance = MIL.MbufAllocComponent(ContainerId, 1, SizeX, NumberOfImages, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE, MIL.M_COMPONENT_REFLECTANCE, MIL.M_NULL);
                MIL.MbufCopy(MilValImage, Reflectance);
                Console.WriteLine("The depth buffer is displayed using 3D MIL.");
                Console.WriteLine("Press <R> on the display window to stop/start the rotation.");
                Console.WriteLine();

                // Hide Mil Display
                MIL.MdispControl(MilDisplay, MIL.M_WINDOW_SHOW, MIL.M_DISABLE);

                MIL.M3ddispSelect(MilDisplay3D, ContainerId, MIL.M_SELECT, MIL.M_DEFAULT);
                AutoRotate3dDisplay(MilDisplay3D);

                // Pause to show the result.
                Console.WriteLine("Press <Enter> to end.");
                Console.ReadKey();
                MIL.M3ddispFree(MilDisplay3D);
                MIL.MbufFree(ContainerId);
            }
            else
            {
                Console.WriteLine("The depth buffer is displayed using 2D MIL.");

                // Find the remapping for result buffers.
                MIL.MimAlloc(MilSystem, MIL.M_STATISTICS_CONTEXT, MIL.M_DEFAULT, ref MilStatContext);
                MIL.MimAllocResult(MilSystem, MIL.M_DEFAULT, MIL.M_STATISTICS_RESULT, ref MilExtreme);

                MIL.MimControl(MilStatContext, MIL.M_STAT_MIN, MIL.M_ENABLE);
                MIL.MimControl(MilStatContext, MIL.M_STAT_MAX, MIL.M_ENABLE);
                MIL.MimControl(MilStatContext, MIL.M_CONDITION, MIL.M_NOT_EQUAL);
                MIL.MimControl(MilStatContext, MIL.M_COND_LOW, 0xFFFF);


                MIL.MimStatCalculate(MilStatContext, MilPosYImage, MilExtreme, MIL.M_DEFAULT);
                MIL.MimGetResult(MilExtreme, MIL.M_STAT_MIN + MIL.M_TYPE_MIL_INT, ref ExtremePosY[0]);
                MIL.MimGetResult(MilExtreme, MIL.M_STAT_MAX + MIL.M_TYPE_MIL_INT, ref ExtremePosY[1]);

                MIL.MimFree(MilExtreme);
                MIL.MimFree(MilStatContext);

                // Free the display and reallocate a new one of the proper dimension for results.
                MIL.MbufFree(MilDisplayImage);
                MIL.MbufAlloc2d(MilSystem,
                   (MIL_INT)((double)SizeX * (M3D_MESH_SCALING_X > 0 ? M3D_MESH_SCALING_X : -M3D_MESH_SCALING_X)),
                   (MIL_INT)((double)NumberOfImages * M3D_MESH_SCALING_Y),
                   8 + MIL.M_UNSIGNED,
                   MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP,
                   ref MilDisplayImage);

                MIL.MdispSelect(MilDisplay, MilDisplayImage);

                // Display the height buffer.
                MIL.MimClip(MilPosYImage, MilPosYImage, MIL.M_GREATER, ExtremePosY[1], MIL.M_NULL, ExtremePosY[1], MIL.M_NULL);
                MIL.MimArith(MilPosYImage, ExtremePosY[0], MilPosYImage, MIL.M_SUB_CONST);
                MIL.MimArith(MilPosYImage, ((ExtremePosY[1] - ExtremePosY[0]) / 255.0) + 1, MilPosYImage, MIL.M_DIV_CONST);
                MIL.MimResize(MilPosYImage, MilDisplayImage, MIL.M_FILL_DESTINATION, MIL.M_FILL_DESTINATION, MIL.M_BILINEAR);

                // Pause to show the result.
                Console.WriteLine("Press <Enter> to end.");
                Console.ReadKey();
            }

            // Free all allocations.
            MIL.MimFree(MilLocatePeak);
            MIL.MimFree(MilContext);
            MIL.MbufFree(MilImage);
            MIL.MgraFree(MilGraList);
            MIL.MbufFree(MilDisplayImage);
            MIL.MbufFree(MilPosYImage);
            MIL.MbufFree(MilValImage);
            MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL);

            return 0;
        }

        // ****************************************************************************
        // Allocates a 3D display and returns its MIL identifier. 
        // ****************************************************************************
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

        // ***************************************************************************
        // Auto rotate the 3D object
        // ***************************************************************************
        private static void AutoRotate3dDisplay(MIL_ID MilDisplay)
        {
            MIL.M3ddispControl(MilDisplay, MIL.M_AUTO_ROTATE, MIL.M_ENABLE);
        }
    }
}
