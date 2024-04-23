﻿//*****************************************************************************
//
// File name: MimProcessing.cs
//
// Synopsis:  This program show the usage of image processing. Under MIL lite, 
//            it binarizes two images to isolate specific zones.
//            Under MIL full, it also uses different image processing primitives 
//            to determine the number of cell nuclei that are larger than a 
//            certain size and show them in pseudo-color.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*****************************************************************************
using System;
using System.Collections.Generic;
using System.Text;

using Matrox.MatroxImagingLibrary;

namespace MImProcessing
{
    class Program
    {
        // Target MIL image file specifications.
        private const string IMAGE_FILE = MIL.M_IMAGE_PATH + "Cell.mbufi";
        private const string IMAGE_CUP = MIL.M_IMAGE_PATH + "PlasticCup.mim";
        private const int IMAGE_SMALL_PARTICLE_RADIUS = 1;

        static void ExtractParticlesExample(MIL_ID MilApplication, MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilRemoteApplication = MIL.M_NULL;   // Remote Application identifier if running on a remote computer
            MIL_ID MilImage = MIL.M_NULL;               // Image buffer identifier.
            MIL_ID MilExtremeResult = 0;                // Extreme result buffer identifier.
            MIL_INT MaxLabelNumber = 0;                 // Highest label value.
            MIL_INT LicenseModules = 0;                 // List of available MIL modules.

            // Restore source image and display it.
            MIL.MbufRestore(IMAGE_FILE, MilSystem, ref MilImage);
            MIL.MdispSelect(MilDisplay, MilImage);

            // Pause to show the original image.
            Console.Write("\n1) Particles extraction:\n");
            Console.Write("-----------------\n\n");
            Console.Write("This first example extracts the dark particles in an image.\n");
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Binarize the image with an automatically calculated threshold so that 
            // particles are represented in white and the background removed.
            MIL.MimBinarize(MilImage, MilImage, MIL.M_BIMODAL + MIL.M_LESS_OR_EQUAL, MIL.M_NULL, MIL.M_NULL);

            // Print a message for the extracted particles.
            Console.Write("These particles were extracted from the original image.\n");

            // If MIL IM module is available, count and label the larger particles.
            MIL.MsysInquire(MilSystem, MIL.M_OWNER_APPLICATION, ref MilRemoteApplication);
            MIL.MappInquire(MilRemoteApplication, MIL.M_LICENSE_MODULES, ref LicenseModules);

            if ((LicenseModules & MIL.M_LICENSE_IM) != 0)
            {
                // Pause to show the extracted particles.
                Console.Write("Press <Enter> to continue.\n\n");
                Console.ReadKey();

                // Close small holes.
                MIL.MimClose(MilImage, MilImage, IMAGE_SMALL_PARTICLE_RADIUS, MIL.M_BINARY);

                // Remove small particles.
                MIL.MimOpen(MilImage, MilImage, IMAGE_SMALL_PARTICLE_RADIUS, MIL.M_BINARY);

                // Label the image.
                MIL.MimLabel(MilImage, MilImage, MIL.M_DEFAULT);

                // The largest label value corresponds to the number of particles in the image.
                MIL.MimAllocResult(MilSystem, 1, MIL.M_EXTREME_LIST, ref MilExtremeResult);
                MIL.MimFindExtreme(MilImage, MilExtremeResult, MIL.M_MAX_VALUE);
                MIL.MimGetResult(MilExtremeResult, MIL.M_VALUE, ref MaxLabelNumber);
                MIL.MimFree(MilExtremeResult);

                // Multiply the labeling result to augment the gray level of the particles.
                MIL.MimArith(MilImage, (int)(256 / (double)MaxLabelNumber), MilImage, MIL.M_MULT_CONST);

                // Display the resulting particles in pseudo-color.
                MIL.MdispLut(MilDisplay, MIL.M_PSEUDO);

                // Print results.
                Console.Write("There were {0} large particles in the original image.\n", MaxLabelNumber);
            }

            // Pause to show the result.
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            /* Reset MilDisplay to M_DEFAULT. */
            MIL.MdispLut(MilDisplay, MIL.M_DEFAULT);

            // Free all allocations.
            MIL.MbufFree(MilImage);
        }
        static void ExtractForegroundExample(MIL_ID MilApplication, MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilImage = MIL.M_NULL;               // Image buffer identifier.
            
            // Restore source image and display it.
            MIL.MbufRestore(IMAGE_CUP, MilSystem, ref MilImage);
            MIL.MdispSelect(MilDisplay, MilImage);

            // Pause to show the original image.
            Console.Write("\n2) Background removal:\n");
            Console.Write("-----------------\n\n");
            Console.Write("This second example separates a cup on a table from the background using MimBinarize() with an M_DOMINANT mode.\n");
            Console.Write("In this case, the dominant mode (black background) is separated from the rest. Note, using an M_BIMODAL mode\n");
            Console.Write("would give another result because the background and the cup would be considered as the same mode.\n");
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Binarize the image with an automatically calculated threshold so that 
            // particles are represented in white and the background removed.
            MIL.MimBinarize(MilImage, MilImage, MIL.M_DOMINANT + MIL.M_LESS_OR_EQUAL, MIL.M_NULL, MIL.M_NULL);

            // Print a message for the extracted cup and table.
            Console.Write("The cup and the table were separated from the background with M_DOMINANT mode of MimBinarize.\n");

            // Pause to show the result.
            Console.Write("Press <Enter> to end.\n\n");
            Console.ReadKey();

            // Free all allocations.
            MIL.MbufFree(MilImage);
        }

        static void Main(string[] args)
        {
            MIL_ID MilApplication = MIL.M_NULL;         // Application identifier.
            MIL_ID MilSystem = MIL.M_NULL;              // System identifier.
            MIL_ID MilDisplay = MIL.M_NULL;             // Display identifier.
            
            // Allocate defaults.
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, ref MilDisplay, MIL.M_NULL, MIL.M_NULL);

            // Show header
            Console.Write("\nIMAGE PROCESSING:\n");
            Console.Write("-----------------\n\n");
            Console.Write("This program shows two image processing examples.\n");

            // Example about extracting particles in an image 
            ExtractParticlesExample(MilApplication, MilSystem, MilDisplay);

            // Example about isolating objects from the background in an image
            ExtractForegroundExample(MilApplication, MilSystem, MilDisplay);

            MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL);
        }
    }
}
