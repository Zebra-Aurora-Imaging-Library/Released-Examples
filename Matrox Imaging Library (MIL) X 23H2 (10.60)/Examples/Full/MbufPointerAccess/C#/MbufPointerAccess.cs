// ******************************************************************************
//
// File name: MIL.MbufPointerAccess.cs
//
// Synopsis:  This program shows how to use the pointer of a
//            MIL buffer in order to directly access its data.
//
//  Note: This program does not support Distributed MIL (DMIL).
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//


using System;
using System.Collections.Generic;
using System.Text;

using Matrox.MatroxImagingLibrary;

namespace MBufPointerAccess
{
    class Program
    {
        // Target image size.
        private const int IMAGE_SIZE_X = 512;
        private const int IMAGE_SIZE_Y = 512;

        static void Main(string[] args)
        {
            MIL_ID MilApplication = MIL.M_NULL;  // Application identifier.
            MIL_ID MilSystem = MIL.M_NULL;       // System identifier.
            MIL_ID MilDisplay = MIL.M_NULL;      // Display identifier.

            Console.WriteLine();
            Console.WriteLine("MIL buffer pointer access example.");
            Console.WriteLine("----------------------------------");
            Console.WriteLine();

            // Allocate defaults.
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, ref MilDisplay, MIL.M_NULL, MIL.M_NULL);

            // Run the custom function only if the target system's memory is local and accessible.
            if (MIL.MsysInquire(MilSystem, MIL.M_LOCATION, MIL.M_NULL) == MIL.M_LOCAL)
            {

                // Pointer access example for a monochrome buffer
                MonochromeBufferPointerAccessExample(MilSystem, MilDisplay);

                // Pointer access example for a color packed buffer.
                ColorPackedBufferPointerAccessExample(MilSystem, MilDisplay);

                // Pointer access example for a color planar buffer.
                ColorPlanarBufferPointerAccessExample(MilSystem, MilDisplay);
            }
            else
            {
                // Print that the example don't run remotely.
                Console.WriteLine("This example doesn't run with Distributed MIL.");
                // Wait for a key to terminate.
                Console.WriteLine("Press a key to terminate.");
                Console.WriteLine();
                Console.ReadKey();
            }

            // Free allocated objects.
            MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL);
        }

        // Pointer access example for a monochrome buffer.
        // ------------------------------------------------

        // Pixel value calculation parameters.
        private const double X_REF1 = -0.500;
        private const double Y_REF1 = +0.002;
        private const double DIM1 = +3.200;

        static void MonochromeBufferPointerAccessExample(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilImage = MIL.M_NULL;       // Image buffer identifier.
            MIL_INT MilImagePtr = MIL.M_NULL;   // Image pointer.
            MIL_INT MilImagePitch = MIL.M_NULL; // Image pitch.
            MIL_INT x, y;                       // Buffer access variables.
            MIL_UINT Value;                     // Value to write.

            Console.Write("- The data of a 8bits monochrome MIL buffer is modified\n");
            Console.Write("  using its pointer to directly access the memory.\n\n");

            // Allocate a monochrome buffer.
            MIL.MbufAlloc2d(MilSystem, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + MIL.M_UNSIGNED,
               MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP, ref MilImage);

            // Lock buffer for direct access.
            MIL.MbufControl(MilImage, MIL.M_LOCK, MIL.M_DEFAULT);

            // Retrieving buffer data pointer and pitch information.
            MIL.MbufInquire(MilImage, MIL.M_HOST_ADDRESS, ref MilImagePtr);
            MIL.MbufInquire(MilImage, MIL.M_PITCH, ref MilImagePitch);

            // Direct Access to the buffer's data.
            if (MilImagePtr != MIL.M_NULL)
            {
                unsafe // Unsafe code block to allow manipulating memory addresses
                {
                    // Convert the address inquired from MIL to a fixed size pointer.
                    // The MIL_INT type cannot be converted to a byte * so use the
                    // IntPtr portable type
                    //
                    IntPtr MilImagePtrIntPtr = MilImagePtr;
                    byte* MilImageAddr = (byte*)MilImagePtrIntPtr;

                    // For each row.
                    for (y = 0; y < IMAGE_SIZE_Y; y++)
                    {
                        // For each column.
                        for (x = 0; x < IMAGE_SIZE_X; x++)
                        {
                            // Calculate the pixel value.
                            Value = Mandelbrot(x, y, X_REF1, Y_REF1, DIM1);

                            // Write the pixel using its pointer.
                            MilImageAddr[x] = (byte)(Value);
                        }

                        // Move pointer to the next line taking into account the image's pitch.
                        MilImageAddr += MilImagePitch;
                    }


                    // Signals MIL that the buffer data has been updated.
                    MIL.MbufControl(MilImage, MIL.M_MODIFIED, MIL.M_DEFAULT);

                    // Unlock buffer.
                    MIL.MbufControl(MilImage, MIL.M_UNLOCK, MIL.M_DEFAULT);

                    // Select to display.
                    MIL.MdispSelect(MilDisplay, MilImage);
                }
            }
            else
            {
                Console.Write("The source buffer has no accessible memory\n");
                Console.Write("address on this specific system. Try changing\n");
                Console.Write("the system in the MIL Config utility.\n\n");
            }

            // Print a message and wait for a key.
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Free allocation.
            MIL.MbufFree(MilImage);
        }

        // Pointer access example for a color packed buffer.
        // --------------------------------------------------

        // Pixel value calculation parameters.
        private const double X_REF2 = -1.1355;
        private const double Y_REF2 = -0.2510;
        private const double DIM2 = +0.1500;

        // Utility to pack B,G,R values into 32 bits integer.
        static uint PackToBGR32(Byte b, Byte g, Byte r)
        {
            return ((uint)b | (uint)(g << 8) | (uint)(r << 16));
        }

        static void ColorPackedBufferPointerAccessExample(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilImage = MIL.M_NULL;       // Image buffer identifier.
            MIL_INT MilImagePtr = MIL.M_NULL;   // Image pointer.
            MIL_INT MilImagePitch = MIL.M_NULL; // Image pitch.
            MIL_INT x, y, NbBand = 3;           // Buffer access variables.
            MIL_UINT Value;                     // Value to write.
            uint Value_BGR32;                   // Value to write.

            Console.Write("- The data of a 32bits color packed MIL buffer is modified\n");
            Console.Write("  using its pointer to directly access the memory.\n\n");

            // Allocate a monochrome buffer.
            MIL.MbufAllocColor(MilSystem, NbBand, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + MIL.M_UNSIGNED,
               MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP + MIL.M_BGR32 + MIL.M_PACKED, ref MilImage);

            // Lock buffer for direct access.
            MIL.MbufControl(MilImage, MIL.M_LOCK, MIL.M_DEFAULT);

            // Retrieving buffer data pointer and pitch information.
            MIL.MbufInquire(MilImage, MIL.M_HOST_ADDRESS, ref MilImagePtr);
            MIL.MbufInquire(MilImage, MIL.M_PITCH, ref MilImagePitch);

            // Custom modification of the buffer's data.
            if (MilImagePtr != MIL.M_NULL)
            {
                unsafe // Unsafe code block to allow manipulating memory addresses
                {
                    // Convert the address inquired from MIL to a fixed size pointer.
                    // The MIL_INT type cannot be converted to a byte * so use the
                    // IntPtr portable type
                    //
                    IntPtr MilImagePtrIntPtr = MilImagePtr;
                    uint* MilImageAddr = (uint*)MilImagePtrIntPtr;

                    // For each row.
                    for (y = 0; y < IMAGE_SIZE_Y; y++)
                    {
                        // For each column.
                        for (x = 0; x < IMAGE_SIZE_X; x++)
                        {
                            // Calculate the pixel value.
                            Value = Mandelbrot(x, y, X_REF2, Y_REF2, DIM2);

                            Value_BGR32 = PackToBGR32(
                               (byte)GetColorFromIndex(MIL.M_BLUE, (MIL_INT)Value, 255),
                               (byte)GetColorFromIndex(MIL.M_GREEN, (MIL_INT)Value, 255),
                               (byte)GetColorFromIndex(MIL.M_RED, (MIL_INT)Value, 255)
                               );

                            // Write the pixel using its pointer.
                            MilImageAddr[x] = (uint)Value_BGR32;
                        }

                        // Move pointer to the next line taking into account the image's pitch.
                        MilImageAddr += MilImagePitch;
                    }

                    // Signals MIL that the buffer data has been updated.
                    MIL.MbufControl(MilImage, MIL.M_MODIFIED, MIL.M_DEFAULT);

                    // Unlock buffer.
                    MIL.MbufControl(MilImage, MIL.M_UNLOCK, MIL.M_DEFAULT);

                    // Select to display.
                    MIL.MdispSelect(MilDisplay, MilImage);
                }
            }
            else
            {
                Console.Write("The source buffer has no accessible memory\n");
                Console.Write("address on this specific system. Try changing\n");
                Console.Write("the system in the MIL Config utility.\n\n");
            }

            // Print a message and wait for a key.
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Free allocation.
            MIL.MbufFree(MilImage);
        }

        // Pointer access example for a color planar buffer.
        // ------------------------------------------------

        // Pixel value calculation parameters.
        private const double X_REF3 = -0.7453;
        private const double Y_REF3 = +0.1127;
        private const double DIM3 = +0.0060;

        static void ColorPlanarBufferPointerAccessExample(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilImage = MIL.M_NULL;         // Image buffer identifier.
            MIL_ID MilImageBand = MIL.M_NULL;     // Image band identifier.
            MIL_INT MilImageBandPtr = MIL.M_NULL; // Image pointer.
            MIL_INT MilImagePitch = MIL.M_NULL;   // Image pitch.
            MIL_INT x, y, i, NbBand = 3;          // Buffer access variables.
            MIL_UINT Value;                       // Value to write.

            MIL_INT[] ColorBand = new MIL_INT[] { MIL.M_RED, MIL.M_GREEN, MIL.M_BLUE };

            Console.Write("- The data of a 24bits color planar MIL buffer is modified using\n");
            Console.Write("  each color band pointer's to directly access the memory.\n\n");

            // Allocate a monochrome buffer.
            MIL.MbufAllocColor(MilSystem, NbBand, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + MIL.M_UNSIGNED,
               MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP + MIL.M_PLANAR, ref MilImage);

            // Retrieving buffer pitch information.
            MIL.MbufInquire(MilImage, MIL.M_PITCH, ref MilImagePitch);

            // Lock buffer for direct access.
            MIL.MbufControl(MilImage, MIL.M_LOCK, MIL.M_DEFAULT);

            // Verifying the buffer has a host address.
            MIL.MbufChildColor(MilImage, MIL.M_RED, ref MilImageBand);
            MIL.MbufInquire(MilImageBand, MIL.M_HOST_ADDRESS, ref MilImageBandPtr);
            MIL.MbufFree(MilImageBand);

            if (MilImageBandPtr != MIL.M_NULL)
            {
                unsafe // Unsafe code block to allow manipulating memory addresses
                {
                    // For each color band.
                    for (i = 0; i < NbBand; i++)
                    {
                        // Retrieving buffer color band pointer.
                        MIL.MbufChildColor(MilImage, (MIL_INT)ColorBand[i], ref MilImageBand);
                        MIL.MbufInquire(MilImageBand, MIL.M_HOST_ADDRESS, ref MilImageBandPtr);

                        // Convert the address inquired from MIL to a fixed size pointer.
                        // The MIL_INT type cannot be converted to a byte * so use the
                        // IntPtr portable type
                        //
                        IntPtr MilImageBandPtrIntPtr = MilImageBandPtr;
                        byte* MilImageBandAddr = (byte*)MilImageBandPtrIntPtr;

                        // For each row.
                        for (y = 0; y < IMAGE_SIZE_Y; y++)
                        {
                            // For each column.
                            for (x = 0; x < IMAGE_SIZE_X; x++)
                            {
                                // Calculate the pixel value.
                                Value = Mandelbrot(x, y, X_REF3, Y_REF3, DIM3);

                                // Write the pixel using its pointer.
                                MilImageBandAddr[x] = (byte)GetColorFromIndex(ColorBand[i], (MIL_INT)Value, 255);
                            }

                            // Move pointer to the next line taking into account the image's pitch.
                            MilImageBandAddr += MilImagePitch;
                        }

                        MIL.MbufFree(MilImageBand);
                    }
                }

                // Signals MIL that the buffer data has been updated.
                MIL.MbufControl(MilImage, MIL.M_MODIFIED, MIL.M_DEFAULT);

                // Unlock buffer.
                MIL.MbufControl(MilImage, MIL.M_UNLOCK, MIL.M_DEFAULT);

                // Select to display.
                MIL.MdispSelect(MilDisplay, MilImage);
            }
            else
            {
                Console.Write("The source buffer has no accessible memory\n");
                Console.Write("address on this specific system. Try changing\n");
                Console.Write("the system in the MIL Config utility.\n\n");
            }

            // Print a message and wait for a key.
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Free allocation.
            MIL.MbufFree(MilImage);
        }

        // Mandelbrot fractal utility functions.
        static double Remap(double pos, double size, double min, double max)
        {
            return ((((max) - (min)) / (size)) * (pos) + (min));
        }

        static MIL_UINT Mandelbrot(MIL_INT PosX, MIL_INT PosY, double RefX,
           double RefY, double Dim)
        {
            const int maxIter = 256;
            double xMin = RefX - 0.5 * Dim;
            double xMax = RefX + 0.5 * Dim;
            double yMin = RefY - 0.5 * Dim;
            double yMax = RefY + 0.5 * Dim;
            double x0 = Remap((double)PosX, (double)IMAGE_SIZE_X, xMin, xMax);
            double y0 = Remap((double)PosY, (double)IMAGE_SIZE_Y, yMin, yMax);
            double x = 0.0;
            double y = 0.0;
            MIL_UINT Iter = 0;

            while ((x * x + y * y < 4) && (Iter < maxIter))
            {
                double Temp = x * x - y * y + x0;
                y = 2 * x * y + y0;
                x = Temp;
                Iter++;
            }

            return Math.Min(255, Iter);
        }

        // Calculate color from index.
        static MIL_INT GetColorFromIndex(MIL_INT Band, MIL_INT Index, MIL_INT MaxIndex)
        {
            MIL_INT[] Segments = { };
            MIL_INT[] SegmentsR = { 0, 0, 0, 255, 255, 128 };
            MIL_INT[] SegmentsG = { 0, 0, 255, 255, 0, 0 };
            MIL_INT[] SegmentsB = { 128, 255, 255, 0, 0, 0 };

            switch ((int)Band)
            {
                case MIL.M_RED:
                    Segments = SegmentsR;
                    break;
                case MIL.M_GREEN:
                    Segments = SegmentsG;
                    break;
                case MIL.M_BLUE:
                    Segments = SegmentsB;
                    break;
            }

            double RemapedIndex = Index * MaxIndex / 256.0;
            MIL_INT SegmentIndex = (MIL_INT)(RemapedIndex * 5.0 / 256.0);
            double Slope = (Segments[SegmentIndex + 1] - Segments[SegmentIndex]) / (256.0 / 5.0);
            double Offset = (Segments[SegmentIndex] - Slope * SegmentIndex * 256.0 / 5.0);
            MIL_INT Value = (MIL_INT)(Slope * RemapedIndex + Offset + 0.5);

            return Value;
        }
    }
}
