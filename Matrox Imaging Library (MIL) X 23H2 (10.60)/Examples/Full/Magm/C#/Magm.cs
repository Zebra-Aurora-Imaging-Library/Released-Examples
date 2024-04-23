//*****************************************************************************
//
// File name: Magm.cs
//
// Synopsis: This program consists of 2 examples that use the AGM module
//           to define a model and search for model occurrences in target images.
//           The first example extracts a single-definition model from a source image,
//           then quickly finds occurrences in a cluttered target image.
//           The second example constructs a composite-definition model through training,
//           then finds occurrences with slight variations in appearance in different target images.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*****************************************************************************
using System;
using System.Collections.Generic;
using System.Net.NetworkInformation;
using System.Text;

using Matrox.MatroxImagingLibrary;

namespace MImProcessing
    {
    class Program
        {
        // Path definitions.
        private const string EXAMPLE_IMAGE_DIR_PATH = MIL.M_IMAGE_PATH + "/Magm/";
        private const string MODEL_IMAGE_PATH       = EXAMPLE_IMAGE_DIR_PATH + "CircuitPinsModel.mim";
        private const string TARGET_IMAGE_PATH      = EXAMPLE_IMAGE_DIR_PATH + "CircuitBoardTarget.mim";
        private const string TRAIN_IMAGES_PATH      = EXAMPLE_IMAGE_DIR_PATH + "LabeledTrainImages.mbufc";
        private const string TEST_IMAGES_DIR_PATH   = EXAMPLE_IMAGE_DIR_PATH + "Testset/";

        //****************************************************************************
        // Single-definition model example.
        //****************************************************************************
        static void SingleModelExample(MIL_ID MilApplication, MIL_ID MilSystem, MIL_ID MilDisplay)
            {
            Console.Write("This example shows that AGM is able ");
            Console.Write("to quickly find occurrences\n");
            Console.Write("in a large cluttered target image.\n");
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            MIL_ID MilGraphicList  = MIL.M_NULL;   // Graphic list identifier.
            MIL_ID MilFindContext  = MIL.M_NULL;   // Find AGM context identifier.
            MIL_ID MilSearchResult = MIL.M_NULL;   // Find AGM result buffer identifier.
            MIL_ID MilModelImage   = MIL.M_NULL;   // Image buffer identifier.
            MIL_ID MilTargetImage  = MIL.M_NULL;   // Image buffer identifier.

            // Allocate a graphic list to hold the subpixel annotations to draw.
            MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref MilGraphicList);

            // Associate the graphic list to the display for annotations.
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

            // Restore the model image.
            MIL.MbufRestore(MODEL_IMAGE_PATH, MilSystem, ref MilModelImage);

            // Make the display a little bigger since the image is small.
            MIL_INT WindowSizeX = MIL.MbufInquire(MilModelImage, MIL.M_SIZE_X, MIL.M_NULL) * 6;
            MIL_INT WindowSizeY = MIL.MbufInquire(MilModelImage, MIL.M_SIZE_Y, MIL.M_NULL) * 2;

            MIL.MdispControl(MilDisplay, MIL.M_WINDOW_INITIAL_SIZE_X, WindowSizeX);
            MIL.MdispControl(MilDisplay, MIL.M_WINDOW_INITIAL_SIZE_Y, WindowSizeY);

            // Display the model image.
            MIL.MdispSelect(MilDisplay, MilModelImage);

            // Put the display back to its default state.
            MIL.MdispControl(MilDisplay, MIL.M_WINDOW_INITIAL_SIZE_X, MIL.M_DEFAULT);
            MIL.MdispControl(MilDisplay, MIL.M_WINDOW_INITIAL_SIZE_Y, MIL.M_DEFAULT);

            // Allocate a find AGM context.
            MIL.MagmAlloc(MilSystem, MIL.M_GLOBAL_EDGE_BASED_FIND, MIL.M_DEFAULT, ref MilFindContext);

            // Allocate a find AGM result buffer.
            MIL.MagmAllocResult(MilSystem, MIL.M_GLOBAL_EDGE_BASED_FIND_RESULT, MIL.M_DEFAULT, ref MilSearchResult);

            // Define the single-definition model.
            MIL.MagmDefine(MilFindContext, MIL.M_ADD, MIL.M_DEFAULT, MIL.M_SINGLE, MilModelImage, MIL.M_DEFAULT);

            // Pause to show the model.
            Console.Write("A single-definition model was defined ");
            Console.Write("from the displayed image.\n");
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Set the minimum acceptable detection score.
            MIL.MagmControl(MilFindContext, MIL.M_AGM_MODEL_INDEX(0), MIL.M_ACCEPTANCE_DETECTION, 90);

            // Preprocess the find AGM context.
            MIL.MagmPreprocess(MilFindContext, MIL.M_DEFAULT);

            // Restore the target image.
            MIL.MbufRestore(TARGET_IMAGE_PATH, MilSystem, ref MilTargetImage);

            // Reset the time.
            double FindTime = 0.0;
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL);

            // Find the model.
            MIL.MagmFind(MilFindContext, MilTargetImage, MilSearchResult, MIL.M_DEFAULT);

            // Read the find time.
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref FindTime);

            // Get the number of occurrences found.
            MIL_INT NumOccurrences = 0;
            MIL.MagmGetResult(MilSearchResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT, ref NumOccurrences);
            if (NumOccurrences > 0)
                {
                double[] XPositions      = new double[NumOccurrences];
                double[] YPositions      = new double[NumOccurrences];
                double[] DetectionScores = new double[NumOccurrences];
                double[] FitScores       = new double[NumOccurrences];
                double[] CoverageScores  = new double[NumOccurrences];
                MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_POSITION_X     , XPositions);     
                MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_POSITION_Y     , YPositions);     
                MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_DETECTION, DetectionScores);
                MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_FIT      , FitScores);      
                MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_COVERAGE , CoverageScores); 

                // Print the results for each occurrence found.
                Console.Write("The model was found in the target image:\n\n");
                Console.Write("Result   X Position   Y Position   DetectionScore   FitScore   CoverageScore\n\n");
                for (MIL_INT i = 0; i < NumOccurrences; ++i)
                    {
                    Console.Write("{0,-9}{1,-13:N2}{2,-13:N2}{3,-17:N2}{4,-11:N2}{5,-11:N2}\n",
                        i, XPositions[i], YPositions[i], DetectionScores[i], FitScores[i], CoverageScores[i]);
                    }
                Console.Write("\nNumber of occurrences found in target image: {0}\n", NumOccurrences);
                Console.Write("Search time: {0} ms\n", FindTime * 1000.0);

                // Draw green edges and bounding boxes over the occurrences that were found.
                MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN);
                MIL.MagmDraw(MIL.M_DEFAULT, MilSearchResult, MilGraphicList, MIL.M_DRAW_EDGES + MIL.M_DRAW_BOX, MIL.M_ALL, MIL.M_DEFAULT);

                // Draw red positions over the occurrences that were found.
                MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED);
                MIL.MagmDraw(MIL.M_DEFAULT, MilSearchResult, MilGraphicList, MIL.M_DRAW_POSITION, MIL.M_ALL, MIL.M_DEFAULT);
                }
            else
                {
                Console.Write("The model was not found in target image.\n");
                }

            // Display the target image.
            MIL.MdispSelect(MilDisplay, MilTargetImage);
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Remove the display.
            MIL.MdispSelect(MilDisplay, MIL.M_NULL);

            // Free MIL objects.
            MIL.MgraFree(MilGraphicList);
            MIL.MagmFree(MilFindContext);
            MIL.MagmFree(MilSearchResult);
            MIL.MbufFree(MilModelImage);
            MIL.MbufFree(MilTargetImage);
            }

        //****************************************************************************
        // Composite-definition model example.
        //****************************************************************************
        static void CompositeModelExample(MIL_ID MilApplication, MIL_ID MilSystem, MIL_ID MilDisplay)
            {
            Console.Write("This example shows that AGM is able ");
            Console.Write("to confidently find occurrences with appearance\n");
            Console.Write("variation in a complex background ");
            Console.Write("after training a composite-definition model.\n");
            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();

            MIL_ID MilGraphicList       = MIL.M_NULL;  // Graphic list identifier.
            MIL_ID MilTrainContext      = MIL.M_NULL;  // Train AGM context identifier.
            MIL_ID MilTrainResult       = MIL.M_NULL;  // Train AGM result buffer identifier.
            MIL_ID MilFindContext       = MIL.M_NULL;  // Find context identifier.
            MIL_ID MilSearchResult      = MIL.M_NULL;  // Find AGM result buffer identifier.
            MIL_ID Regions              = MIL.M_NULL;  // Graphic list identifier.
            MIL_ID TrainImagesContainer = MIL.M_NULL;  // Container buffer identifier.

            // Restore the training images.
            MIL.MbufRestore(TRAIN_IMAGES_PATH, MilSystem, ref TrainImagesContainer);

            // Print message about training image labels.
            Console.Write("\n*******************************************************\n");
            Console.Write("LOADING LABELED TRAINING IMAGES...\n");
            Console.Write("*******************************************************\n");

            Console.Write("Training requires labeled images with positive and negative samples.\n");
            Console.Write("Positive samples are occurrences delimited by blue boxes and\n");
            Console.Write("negative samples are background parts delimited by red boxes.\n");
            Console.Write("Typically, when false positives are detected in training images,\n");
            Console.Write("they should be used as negative samples to improve the training.\n");
            Console.Write("To ease the labeling of images, use the example AgmLabelingTool.\n");

            // Wait for a key to be pressed.
            Console.Write("\nPress <Enter> to show the labeled images used in this training.\n");
            Console.ReadKey();

            // Get the components from the container.
            MIL_INT NumTrainImage = 0;
            MIL.MbufInquireContainer(TrainImagesContainer, MIL.M_CONTAINER, MIL.M_COMPONENT_LIST + MIL.M_NB_ELEMENTS, ref NumTrainImage);
            MIL_ID[] TrainImages = new MIL_ID[NumTrainImage];
            MIL.MbufInquireContainer(TrainImagesContainer, MIL.M_CONTAINER, MIL.M_COMPONENT_LIST, TrainImages);

            // Allocate a graphic list to hold the subpixel annotations to draw.
            MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref Regions);

            // Associate the graphic list to the display for annotations.
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, Regions);

            // Display each labeled training image.
            for (MIL_INT i = 0; i < NumTrainImage; ++i)
                {
                MIL.MgraClear(MIL.M_DEFAULT, Regions);
                MIL.MbufSetRegion(TrainImages[i], Regions, MIL.M_DEFAULT, MIL.M_EXTRACT, MIL.M_DEFAULT);
                MIL.MdispSelect(MilDisplay, TrainImages[i]);
                Console.Write("Training image {0}/{1}\n", i + 1, NumTrainImage);
                Console.Write("Press <Enter> to continue.\n");
                Console.ReadKey();
                }

            // Disassociate the graphic list from the display and stop displaying the training images.
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MIL.M_NULL);
            MIL.MdispSelect(MilDisplay, MIL.M_NULL);

            // Allocate a find AGM context.
            MIL.MagmAlloc(MilSystem, MIL.M_GLOBAL_EDGE_BASED_FIND, MIL.M_DEFAULT, ref MilFindContext);

            // Allocate a find AGM result buffer.
            MIL.MagmAllocResult(MilSystem, MIL.M_GLOBAL_EDGE_BASED_FIND_RESULT, MIL.M_DEFAULT, ref MilSearchResult);

            // Allocate a train AGM context.
            MIL.MagmAlloc(MilSystem, MIL.M_GLOBAL_EDGE_BASED_TRAIN, MIL.M_DEFAULT, ref MilTrainContext);

            // Allocate a train AGM result buffer.
            MIL.MagmAllocResult(MilSystem, MIL.M_GLOBAL_EDGE_BASED_TRAIN_RESULT, MIL.M_DEFAULT, ref MilTrainResult);

            // Define the composite-definition model.
            MIL.MagmDefine(MilTrainContext, MIL.M_ADD, MIL.M_DEFAULT, MIL.M_COMPOSITE, MIL.M_NULL, MIL.M_DEFAULT);

            // Preprocess the train AGM context.
            MIL.MagmPreprocess(MilTrainContext, MIL.M_DEFAULT);

            // Train the composite-definition model.
            Console.Write("\n*******************************************************\n");
            Console.Write("TRAINING... THIS WILL TAKE SOME TIME...\n");
            Console.Write("*******************************************************\n");
            MIL.MagmTrain(MilTrainContext, ref TrainImagesContainer, 1, MilTrainResult, MIL.M_DEFAULT);

            // Check that the training process completed successfully.
            MIL_INT TrainStatus = -1;
            MIL.MagmGetResult(MilTrainResult, MIL.M_DEFAULT, MIL.M_STATUS + MIL.M_TYPE_MIL_INT, ref TrainStatus);
            if (TrainStatus == MIL.M_COMPLETE)
                {
                Console.Write("Training complete!\n");

                // Ensure that the trained model is valid before copying to the find AGM context.
                MIL_INT TrainedModelStatus = -1;
                MIL.MagmGetResult(MilTrainResult, MIL.M_AGM_MODEL_INDEX(0), MIL.M_STATUS, ref TrainedModelStatus);
                if (TrainedModelStatus == MIL.M_STATUS_TRAIN_OK)
                    {
                    MIL.MagmCopyResult(MilTrainResult, MIL.M_DEFAULT, MilFindContext, MIL.M_DEFAULT, MIL.M_TRAINED_MODEL, MIL.M_DEFAULT);
                    }
                }

            // Preprocess find AGM context.
            MIL.MagmPreprocess(MilFindContext, MIL.M_DEFAULT);

            Console.Write("\n*******************************************************\n");
            Console.Write("FINDING WITH THE TRAINED MODEL...\n");
            Console.Write("*******************************************************\n");

            // Restore test images.
            MIL_INT NumberOfImages = 0;
            string FilesToSearch = TEST_IMAGES_DIR_PATH + "*.mim";
            MIL.MappFileOperation(MIL.M_DEFAULT, FilesToSearch, MIL.M_NULL, MIL.M_NULL, MIL.M_FILE_NAME_FIND_COUNT, MIL.M_DEFAULT, ref NumberOfImages);
            MIL_ID[] TestImages = new MIL_ID[NumberOfImages];
            for (MIL_INT i = 0; i < NumberOfImages; i++)
                {
                StringBuilder Filename = new StringBuilder();
                MIL.MappFileOperation(MIL.M_DEFAULT, FilesToSearch, MIL.M_NULL, MIL.M_NULL, MIL.M_FILE_NAME_FIND, i, Filename);
                string FilePath = TEST_IMAGES_DIR_PATH + Filename;
                MIL.MbufRestore(FilePath, MilSystem, ref TestImages[i]);
                }

            // Wait for a key to be pressed.
            Console.Write("\nPress <Enter> to search for the trained model in different test images.\n\n");
            Console.ReadKey();

            // Allocate a graphic list to hold the subpixel annotations to draw.
            MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref MilGraphicList);

            // Associate the graphic list to the display for annotations.
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

            // Assign the color to draw.
            MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN);
            for (MIL_INT i = 0; i < NumberOfImages; ++i)
                {
                // Find the model in the test image.
                MIL.MagmFind(MilFindContext, TestImages[i], MilSearchResult, MIL.M_DEFAULT);

                // Get the number of occurrences found.
                MIL_INT NumOccurrences = 0;
                MIL.MagmGetResult(MilSearchResult, MIL.M_DEFAULT, MIL.M_NUMBER, ref NumOccurrences);

                if (NumOccurrences > 0)
                    {
                    // Get the results of the search.
                    double[] XPositions      = new double[NumOccurrences];
                    double[] YPositions      = new double[NumOccurrences];
                    double[] DetectionScores = new double[NumOccurrences];
                    double[] FitScores       = new double[NumOccurrences];
                    double[] CoverageScores  = new double[NumOccurrences];
                    MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_POSITION_X     , XPositions);     
                    MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_POSITION_Y     , YPositions);     
                    MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_DETECTION, DetectionScores);
                    MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_FIT      , FitScores);      
                    MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_COVERAGE , CoverageScores); 

                    // Print the results for each occurrence foud.
                    Console.Write("The model was found in the target image:\n\n");
                    Console.Write("Result   X Position   Y Position   DetectionScore   FitScore   CoverageScore\n\n");
                    for (MIL_INT j = 0; j < NumOccurrences; ++j)
                        {
                        Console.Write("{0,-9}{1,-13:N2}{2,-13:N2}{3,-17:N2}{4,-11:N2}{5,-11:N2}\n",
                                      j, XPositions[j], YPositions[j], DetectionScores[j], FitScores[j], CoverageScores[j]);
                        }

                    // Empty the graphic list.
                    MIL.MgraClear(MIL.M_DEFAULT, MilGraphicList);

                    // Draw bounding box
                    MIL.MagmDraw(MIL.M_DEFAULT, MilSearchResult, MilGraphicList, MIL.M_DRAW_BOX, MIL.M_ALL, MIL.M_DEFAULT);
                    }
                else
                    {
                    Console.Write("The model was not found in target image.\n");
                    }

                // Display the test image.
                MIL.MdispSelect(MilDisplay, TestImages[i]);

                // Wait for a key to be pressed.
                Console.Write("Press <Enter> to continue.\n\n");
                Console.ReadKey();
                }

            // Remove the display.
            MIL.MdispSelect(MilDisplay, MIL.M_NULL);

            // Free MIL objects.
            for (MIL_INT i = 0; i < NumberOfImages; ++i)
                {
                MIL.MbufFree(TestImages[i]);
                }
            MIL.MgraFree(MilGraphicList);
            MIL.MgraFree(Regions);
            MIL.MagmFree(MilTrainContext);
            MIL.MagmFree(MilTrainResult);
            MIL.MagmFree(MilFindContext);
            MIL.MagmFree(MilSearchResult);
            MIL.MbufFree(TrainImagesContainer);
            }

        static void Main(string[] args)
            {
            MIL_ID MilApplication = MIL.M_NULL;   // Application identifier.
            MIL_ID MilSystem      = MIL.M_NULL;   // System identifier.
            MIL_ID MilDisplay     = MIL.M_NULL;   // Display identifier.

            // Allocate defaults.
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, ref MilDisplay, MIL.M_NULL, MIL.M_NULL);

            // Print the example synopsis.
            Console.Write("[EXAMPLE NAME]\n");
            Console.Write("Magm\n\n");
            Console.Write("[SYNOPSIS]\n");
            Console.Write("This program shows the use of the AGM module.\n");
            Console.Write("[MODULES USED]\n");
            Console.Write("Advanced Geometric Matcher, Buffer, Display, Graphics.\n\n");

            // Run single-definition model example.
            SingleModelExample(MilApplication, MilSystem, MilDisplay);

            // Run composite-definition model example.
            CompositeModelExample(MilApplication, MilSystem, MilDisplay);

            // Wait for a key to be pressed.
            Console.Write("Press <Enter> to end.\n");
            Console.ReadKey();

            MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL);
            }
        }
    }
