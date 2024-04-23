//**************************************************************************************
// File name: MPat.cs
//
// Synopsis:  This program contains 4 examples of the pattern matching module:
//
//            The first example defines a model and then searches for it in a shifted
//            version of the image (without rotation).
//
//            The second example defines a regular model and then searches for it in a
//            rotated version of the image using search angle range.
//
//            The third example defines a rotated model at certain angle and then
//            searches for it in a rotated version of the image.
//
//            The fourth example automatically allocates a model in a wafer image and 
//            finds its horizontal and vertical displacement.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//**************************************************************************************
using System;
using System.Collections.Generic;
using System.Text;

using Matrox.MatroxImagingLibrary;

namespace MPat
{
    class Program
    {
        static void Main(string[] args)
        {
            MIL_ID MilApplication = MIL.M_NULL;     // Application identifier.
            MIL_ID MilSystem = MIL.M_NULL;          // System identifier.
            MIL_ID MilDisplay = MIL.M_NULL;         // Display identifier.

            Console.Write("\nGRAYSCALE PATTERN MATCHING:\n");
            Console.Write("---------------------------\n\n");

            // Allocate defaults.
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, ref MilDisplay, MIL.M_NULL, MIL.M_NULL);

            // Run the search at 0 degree example.
            SearchModelExample(MilSystem, MilDisplay);

            // Run the search over 360 degrees example.
            SearchModelAngleRangeExample(MilSystem, MilDisplay);

            // Run the search rotated model example.
            SearchModelAtAngleExample(MilSystem, MilDisplay);

            // Run the automatic model allocation example.
            AutoAllocationModelExample(MilSystem, MilDisplay);

            // Free defaults.
            MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL);
        }

        //****************************************************************************
        // Find model in shifted version of the image example.

        // Source image file name.
        private const string FIND_IMAGE_FILE = MIL.M_IMAGE_PATH + "CircuitsBoard.mim";

        // Model position and size.
        private const int FIND_MODEL_X_POS = 153;
        private const int FIND_MODEL_Y_POS = 132;
        private const int FIND_MODEL_WIDTH = 128;
        private const int FIND_MODEL_HEIGHT = 128;
        private const double FIND_MODEL_X_CENTER = (FIND_MODEL_X_POS + (FIND_MODEL_WIDTH - 1) / 2.0);
        private const double FIND_MODEL_Y_CENTER = (FIND_MODEL_Y_POS + (FIND_MODEL_HEIGHT - 1) / 2.0);

        // Target image shifting values.
        private const double FIND_SHIFT_X = 4.5;
        private const double FIND_SHIFT_Y = 7.5;

        // Minimum match score to determine acceptability of model (default).
        private const double FIND_MODEL_MIN_MATCH_SCORE = 70.0;

        // Minimum accuracy for the search.
        private const double FIND_MODEL_MIN_ACCURACY = 0.1;

        static void SearchModelExample(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilImage = MIL.M_NULL;               // Image buffer identifier.
            MIL_ID GraphicList = MIL.M_NULL;            // Graphic list identifier.
            MIL_ID ContextId = MIL.M_NULL;              // ContextId identifier.
            MIL_ID Result = MIL.M_NULL;                 // Result identifier.
            MIL_INT NumResults = 0;                     // Number of results found.
            double XOrg = 0.0;                          // Original model position.
            double YOrg = 0.0;
            double x = 0.0;                             // Model position.
            double y = 0.0;
            double ErrX = 0.0;                          // Model error position.
            double ErrY = 0.0;
            double Score = 0.0;                         // Model correlation score.
            double Time = 0.0;                          // Model search time.
            double AnnotationColor = MIL.M_COLOR_GREEN; // Drawing color.

            // Restore source image into an automatically allocated image buffer.
            MIL.MbufRestore(FIND_IMAGE_FILE, MilSystem, ref MilImage);

            // Display the image buffer.
            MIL.MdispSelect(MilDisplay, MilImage);

            // Allocate a graphic list to hold the subpixel annotations to draw.
            MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref GraphicList);

            // Associate the graphic list to the display for annotations.
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

            // Allocate a normalized pattern matching context.
            MIL.MpatAlloc(MilSystem, MIL.M_NORMALIZED, MIL.M_DEFAULT, ref ContextId);

            // Define a regular model.
            MIL.MpatDefine(ContextId, MIL.M_REGULAR_MODEL, MilImage, FIND_MODEL_X_POS,
                       FIND_MODEL_Y_POS, FIND_MODEL_WIDTH, FIND_MODEL_HEIGHT, MIL.M_DEFAULT);

            // Set the search accuracy to high.
            MIL.MpatControl(ContextId, MIL.M_DEFAULT, MIL.M_ACCURACY, MIL.M_HIGH);

            // Set the search model speed to high.
            MIL.MpatControl(ContextId, MIL.M_DEFAULT, MIL.M_SPEED, MIL.M_HIGH);

            // Preprocess the model.
            MIL.MpatPreprocess(ContextId, MIL.M_DEFAULT, MilImage);

            // Draw a box around the model in the model image.
            MIL.MgraColor(MIL.M_DEFAULT, AnnotationColor);
            MIL.MpatDraw(MIL.M_DEFAULT, ContextId, GraphicList, MIL.M_DRAW_BOX + MIL.M_DRAW_POSITION,
                                                                           MIL.M_DEFAULT, MIL.M_ORIGINAL);

            // Pause to show the original image and model position.
            Console.Write("\nA {0}x{1} model was defined in the source image.\n", FIND_MODEL_WIDTH, FIND_MODEL_HEIGHT);
            Console.Write("It will be found in an image shifted by {0:0.00} in X and {1:0.00} in Y.\n", FIND_SHIFT_X, FIND_SHIFT_Y);
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Clear annotations.
            MIL.MgraClear(MIL.M_DEFAULT, GraphicList);

            // Translate the image on a subpixel level.
            MIL.MimTranslate(MilImage, MilImage, FIND_SHIFT_X, FIND_SHIFT_Y, MIL.M_DEFAULT);

            // Allocate result buffer.
            MIL.MpatAllocResult(MilSystem, MIL.M_DEFAULT, ref Result);

            // Dummy first call for bench measure purpose only (bench stabilization, cache effect, etc...). This first call is NOT required by the application.
            MIL.MpatFind(ContextId, MilImage, Result);
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL);

            // Find the model in the target buffer.
            MIL.MpatFind(ContextId, MilImage, Result);

            // Read the time spent in MpatFindModel.
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref Time);

            // If one model was found above the acceptance threshold.
            MIL.MpatGetResult(Result, MIL.M_GENERAL, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT, ref NumResults);
            if (NumResults == 1)
            {
                // Read results and draw a box around the model occurrence.
                MIL.MpatGetResult(Result, MIL.M_DEFAULT, MIL.M_POSITION_X, ref x);
                MIL.MpatGetResult(Result, MIL.M_DEFAULT, MIL.M_POSITION_Y, ref y);
                MIL.MpatGetResult(Result, MIL.M_DEFAULT, MIL.M_SCORE, ref Score);
                MIL.MgraColor(MIL.M_DEFAULT, AnnotationColor);
                MIL.MpatDraw(MIL.M_DEFAULT, Result, GraphicList, MIL.M_DRAW_BOX + MIL.M_DRAW_POSITION, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Calculate the position errors in X and Y and inquire original model position.
                ErrX = Math.Abs((FIND_MODEL_X_CENTER + FIND_SHIFT_X) - x);
                ErrY = Math.Abs((FIND_MODEL_Y_CENTER + FIND_SHIFT_Y) - y);
                MIL.MpatInquire(ContextId, MIL.M_DEFAULT, MIL.M_ORIGINAL_X, ref XOrg);
                MIL.MpatInquire(ContextId, MIL.M_DEFAULT, MIL.M_ORIGINAL_Y, ref YOrg);

                // Print out the search result of the model in the original image.
                Console.Write("Search results:\n");
                Console.Write("---------------------------------------------------\n");
                Console.Write("The model is found to be shifted by \tX:{0:0.00}, Y:{1:0.00}.\n", x - XOrg, y - YOrg);
                Console.Write("The model position error is \t\tX:{0:0.00}, Y:{1:0.00}\n", ErrX, ErrY);
                Console.Write("The model match score is \t\t{0:0.0}\n", Score);
                Console.Write("The search time is \t\t\t{0:0.000} ms\n\n", Time * 1000.0);

                // Verify the results.
                if ((Math.Abs((x - XOrg) - FIND_SHIFT_X) > FIND_MODEL_MIN_ACCURACY) ||
                    (Math.Abs((y - YOrg) - FIND_SHIFT_Y) > FIND_MODEL_MIN_ACCURACY) ||
                    (Score < FIND_MODEL_MIN_MATCH_SCORE))
                {
                    Console.Write("Results verification error !\n");
                }
            }
            else
            {
                Console.Write("Model not found !\n");
            }

            // Wait for a key to be pressed.
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Clear annotations.
            MIL.MgraClear(MIL.M_DEFAULT, GraphicList);

            // Free all allocations.
            MIL.MgraFree(GraphicList);
            MIL.MpatFree(Result);
            MIL.MpatFree(ContextId);
            MIL.MbufFree(MilImage);
        }

        //****************************************************************************
        // Find rotated model example.

        // Source image file name. 
        private const string ROTATED_FIND_IMAGE_FILE = MIL.M_IMAGE_PATH + "CircuitsBoard.mim";

        // Image rotation values.
        private const int ROTATED_FIND_ROTATION_DELTA_ANGLE = 10;
        private const int ROTATED_FIND_ROTATION_ANGLE_STEP = 1;
        private const double ROTATED_FIND_RAD_PER_DEG = 0.01745329251;

        // Model position and size.
        private const int ROTATED_FIND_MODEL_X_POS = 153;
        private const int ROTATED_FIND_MODEL_Y_POS = 132;
        private const int ROTATED_FIND_MODEL_WIDTH = 128;
        private const int ROTATED_FIND_MODEL_HEIGHT = 128;

        private const double ROTATED_FIND_MODEL_X_CENTER = ROTATED_FIND_MODEL_X_POS + (ROTATED_FIND_MODEL_WIDTH - 1) / 2.0;
        private const double ROTATED_FIND_MODEL_Y_CENTER = ROTATED_FIND_MODEL_Y_POS + (ROTATED_FIND_MODEL_HEIGHT - 1) / 2.0;

        // Minimum accuracy for the search position.
        private const double ROTATED_FIND_MIN_POSITION_ACCURACY = 0.10;

        // Minimum accuracy for the search angle.
        private const double ROTATED_FIND_MIN_ANGLE_ACCURACY = 0.25;

        // Angle range to search.
        private const int ROTATED_FIND_ANGLE_DELTA_POS = ROTATED_FIND_ROTATION_DELTA_ANGLE;
        private const int ROTATED_FIND_ANGLE_DELTA_NEG = ROTATED_FIND_ROTATION_DELTA_ANGLE;

        static void SearchModelAngleRangeExample(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilSourceImage = MIL.M_NULL;         // Model  image buffer identifier.
            MIL_ID MilTargetImage = MIL.M_NULL;         // Target image buffer identifier.
            MIL_ID MilDisplayImage = MIL.M_NULL;        // Target image buffer identifier.
            MIL_ID GraphicList = MIL.M_NULL;            // Graphic list.
            MIL_ID MilContextId = MIL.M_NULL;           // Model identifier.
            MIL_ID MilResult = MIL.M_NULL;              // Result identifier.
            double RealX = 0.0;                         // Model real position in x.
            double RealY = 0.0;                         // Model real position in y.
            double RealAngle = 0.0;                     // Model real angle.
            double X = 0.0;                             // Model position in x found.
            double Y = 0.0;                             // Model position in y found.
            double Angle = 0.0;                         // Model angle found.
            double Score = 0.0;                         // Model correlation score.
            double Time = 0.0;                          // Model search time.
            double ErrX = 0.0;                          // Model error position in x.
            double ErrY = 0.0;                          // Model error position in y.
            double ErrAngle = 0.0;                      // Model error angle.
            double SumErrX = 0.0;                       // Model total error position in x.
            double SumErrY = 0.0;                       // Model total error position in y.
            double SumErrAngle = 0.0;                   // Model total error angle.
            double SumTime = 0.0;                       // Model total search time.
            MIL_INT NumResults = 0;                     // Number of results found.
            MIL_INT NbFound = 0;                        // Number of models found.
            double AnnotationColor = MIL.M_COLOR_GREEN; // Drawing color.

            // Load target image into image buffers and display it.
            MIL.MbufRestore(ROTATED_FIND_IMAGE_FILE, MilSystem, ref MilSourceImage);
            MIL.MbufRestore(ROTATED_FIND_IMAGE_FILE, MilSystem, ref MilTargetImage);
            MIL.MbufRestore(ROTATED_FIND_IMAGE_FILE, MilSystem, ref MilDisplayImage);
            MIL.MdispSelect(MilDisplay, MilDisplayImage);

            // Allocate a graphic list to hold the subpixel annotations to draw.
            MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref GraphicList);

            // Associate the graphic list to the display for annotations.
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

            // Allocate a normalized pattern matching context.
            MIL.MpatAlloc(MilSystem, MIL.M_NORMALIZED, MIL.M_DEFAULT, ref MilContextId);

            // Define a regular model.
            MIL.MpatDefine(MilContextId, MIL.M_REGULAR_MODEL + MIL.M_CIRCULAR_OVERSCAN, MilSourceImage,
                       ROTATED_FIND_MODEL_X_POS, ROTATED_FIND_MODEL_Y_POS,
                       ROTATED_FIND_MODEL_WIDTH, ROTATED_FIND_MODEL_HEIGHT, MIL.M_DEFAULT);

            // Set the search model speed.
            MIL.MpatControl(MilContextId, MIL.M_DEFAULT, MIL.M_SPEED, MIL.M_MEDIUM);

            // Set the position search accuracy.
            MIL.MpatControl(MilContextId, MIL.M_DEFAULT, MIL.M_ACCURACY, MIL.M_HIGH);

            // Activate the search model angle mode.
            MIL.MpatControl(MilContextId, MIL.M_DEFAULT, MIL.M_SEARCH_ANGLE_MODE, MIL.M_ENABLE);

            // Set the search model range angle.
            MIL.MpatControl(MilContextId, MIL.M_DEFAULT, MIL.M_SEARCH_ANGLE_DELTA_NEG, ROTATED_FIND_ANGLE_DELTA_NEG);
            MIL.MpatControl(MilContextId, MIL.M_DEFAULT, MIL.M_SEARCH_ANGLE_DELTA_POS, ROTATED_FIND_ANGLE_DELTA_POS);

            // Set the search model angle accuracy.
            MIL.MpatControl(MilContextId, MIL.M_DEFAULT, MIL.M_SEARCH_ANGLE_ACCURACY, ROTATED_FIND_MIN_ANGLE_ACCURACY);

            // Set the search model angle interpolation mode to bilinear.
            MIL.MpatControl(MilContextId, MIL.M_DEFAULT, MIL.M_SEARCH_ANGLE_INTERPOLATION_MODE, MIL.M_BILINEAR);

            // Preprocess the model.
            MIL.MpatPreprocess(MilContextId, MIL.M_DEFAULT, MilSourceImage);

            // Allocate a result buffer.
            MIL.MpatAllocResult(MilSystem, MIL.M_DEFAULT, ref MilResult);

            // Draw the original model position.
            MIL.MpatDraw(MIL.M_DEFAULT, MilContextId, GraphicList, MIL.M_DRAW_BOX + MIL.M_DRAW_POSITION,
                                                                   MIL.M_DEFAULT, MIL.M_ORIGINAL);

            // Pause to show the original image and model position.
            Console.Write("\nA {0}x{1} model was defined in the source image.\n", ROTATED_FIND_MODEL_WIDTH, ROTATED_FIND_MODEL_HEIGHT);
            Console.Write("It will be searched in images rotated from {0} degree to {1} degree.\n", -ROTATED_FIND_ROTATION_DELTA_ANGLE, ROTATED_FIND_ROTATION_DELTA_ANGLE);
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Dummy first call for bench measure purpose only (bench stabilization, 
            // cache effect, etc...). This first call is NOT required by the application.
            MIL.MpatFind(MilContextId, MilSourceImage, MilResult);

            // If the model was found above the acceptance threshold.
            MIL.MpatGetResult(MilResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT, ref NumResults);
            if (NumResults == 1)
            {
                // Search for the model in images at different angles.
                RealAngle = ROTATED_FIND_ROTATION_DELTA_ANGLE;
                while (RealAngle >= -ROTATED_FIND_ROTATION_DELTA_ANGLE)
                {
                    // Rotate the image from the model image to target image.
                    MIL.MimRotate(MilSourceImage, MilTargetImage, RealAngle, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_BILINEAR + MIL.M_OVERSCAN_CLEAR);

                    // Reset the timer.
                    MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL);

                    // Find the model in the target image.
                    MIL.MpatFind(MilContextId, MilTargetImage, MilResult);

                    // Read the time spent in MpatFindModel().
                    MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref Time);

                    // Clear the annotations.
                    MIL.MgraClear(MIL.M_DEFAULT, GraphicList);

                    // If one model was found above the acceptance threshold.
                    MIL.MpatGetResult(MilResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT, ref NumResults);
                    if (NumResults == 1)
                    {
                        // Read results and draw a box around model occurrence.
                        MIL.MpatGetResult(MilResult, MIL.M_DEFAULT, MIL.M_POSITION_X, ref X);
                        MIL.MpatGetResult(MilResult, MIL.M_DEFAULT, MIL.M_POSITION_Y, ref Y);
                        MIL.MpatGetResult(MilResult, MIL.M_DEFAULT, MIL.M_ANGLE, ref Angle);
                        MIL.MpatGetResult(MilResult, MIL.M_DEFAULT, MIL.M_SCORE, ref Score);

                        MIL.MgraColor(MIL.M_DEFAULT, AnnotationColor);
                        MIL.MpatDraw(MIL.M_DEFAULT, MilResult, GraphicList, MIL.M_DRAW_BOX + MIL.M_DRAW_POSITION, MIL.M_DEFAULT, MIL.M_DEFAULT);

                        MIL.MbufCopy(MilTargetImage, MilDisplayImage);

                        // Calculate the angle error and the position errors for statistics.
                        ErrAngle = CalculateAngleDist(Angle, RealAngle);

                        RotateModelCenter(MilSourceImage, ref RealX, ref RealY, RealAngle);
                        ErrX = Math.Abs(X - RealX);
                        ErrY = Math.Abs(Y - RealY);

                        SumErrAngle += ErrAngle;
                        SumErrX += ErrX;
                        SumErrY += ErrY;
                        SumTime += Time;
                        NbFound++;

                        // Verify the precision for the position and the angle.
                        if ((ErrX > ROTATED_FIND_MIN_POSITION_ACCURACY) || (ErrY > ROTATED_FIND_MIN_POSITION_ACCURACY) || (ErrAngle > ROTATED_FIND_MIN_ANGLE_ACCURACY))
                        {
                            Console.Write("Model accuracy error at angle {0:0.0} !\n\n", RealAngle);
                            Console.Write("Errors are X:{0:0.000}, Y:{1:0.000} and Angle:{2:0.00}\n\n", ErrX, ErrY, ErrAngle);
                            Console.Write("Press <Enter> to continue.\n\n");
                            Console.ReadKey();
                        }
                    }
                    else
                    {
                        Console.Write("Model was not found at angle {0:0.0} !\n\n", RealAngle);
                        Console.Write("Press <Enter> to continue.\n\n");
                        Console.ReadKey();
                    }

                    RealAngle -= ROTATED_FIND_ROTATION_ANGLE_STEP;
                }

                // Print out the search result statistics
                // of the models found in rotated images.
                Console.Write("\nSearch statistics for the model found in the rotated images.\n");
                Console.Write("------------------------------------------------------------\n");
                Console.Write("The average position error is \t\tX:{0:0.000}, Y:{1:0.000}\n", SumErrX / NbFound, SumErrY / NbFound);
                Console.Write("The average angle error is \t\t{0:0.000}\n", SumErrAngle / NbFound);
                Console.Write("The average search time is \t\t{0:0.000} ms\n\n", SumTime * 1000.0 / NbFound);
            }
            else
            {
                Console.Write("Model was not found!\n\n");
            }

            // Wait for a key to be pressed. 
            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Free all allocations.
            MIL.MgraFree(GraphicList);
            MIL.MpatFree(MilResult);
            MIL.MpatFree(MilContextId);
            MIL.MbufFree(MilTargetImage);
            MIL.MbufFree(MilSourceImage);
            MIL.MbufFree(MilDisplayImage);
        }


        // Calculate the rotated center of the model to compare the accuracy with
        // the center of the occurrence found during pattern matching.
        static void RotateModelCenter(MIL_ID Buffer, ref double X, ref double Y, double Angle)
        {
            MIL_INT BufSizeX = MIL.MbufInquire(Buffer, MIL.M_SIZE_X, MIL.M_NULL);
            MIL_INT BufSizeY = MIL.MbufInquire(Buffer, MIL.M_SIZE_Y, MIL.M_NULL);

            double RadAngle = Angle * ROTATED_FIND_RAD_PER_DEG;
            double CosAngle = Math.Cos(RadAngle);
            double SinAngle = Math.Sin(RadAngle);

            double OffSetX = (BufSizeX - 1) / 2.0;
            double OffSetY = (BufSizeY - 1) / 2.0;

            X = (ROTATED_FIND_MODEL_X_CENTER - OffSetX) * CosAngle + (ROTATED_FIND_MODEL_Y_CENTER - OffSetY) * SinAngle + OffSetX;
            Y = (ROTATED_FIND_MODEL_Y_CENTER - OffSetY) * CosAngle - (ROTATED_FIND_MODEL_X_CENTER - OffSetX) * SinAngle + OffSetY;
        }


        // Calculate the absolute difference between the real angle 
        // and the angle found.
        static double CalculateAngleDist(double Angle1, double Angle2)
        {
            double dist = Math.Abs(Angle1 - Angle2);

            while (dist >= 360.0)
            {
                dist -= 360.0;
            }

            if (dist > 180.0)
            {
                dist = 360.0 - dist;
            }

            return dist;
        }

        //****************************************************
        // Find the rotated model in a rotated image example. 
        //****************************************************
        static void SearchModelAtAngleExample(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilSourceImage = MIL.M_NULL;         // Model image buffer identifier.
            MIL_ID MilTargetImage = MIL.M_NULL;         // Target image buffer identifier.
            MIL_ID MilOverlayImage = MIL.M_NULL;        // Overlay image buffer identifier.
            MIL_ID ContextId = MIL.M_NULL;              // Context identifier.
            MIL_ID GraphicList = MIL.M_NULL;            // Graphic list.
            MIL_ID MilResult = MIL.M_NULL;              // Result identifier.
            double Time = 0.0;                          // Model search time.
            MIL_INT NbFound = 0;                        // Number of models found.
            double AnnotationColor = MIL.M_COLOR_GREEN; // Drawing color.

            // Load the source image and display it.
            MIL.MbufRestore(ROTATED_FIND_IMAGE_FILE, MilSystem, ref MilSourceImage);
            MIL.MdispSelect(MilDisplay, MilSourceImage);

            // Allocate the target image.
            MIL.MbufAlloc2d(MilSystem, MIL.MbufInquire(MilSourceImage, MIL.M_SIZE_X, MIL.M_NULL),
                MIL.MbufInquire(MilSourceImage, MIL.M_SIZE_Y, MIL.M_NULL),
                8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP, ref MilTargetImage);

            // Allocate a graphic list to hold the subpixel annotations to draw.
            MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref GraphicList);

            // Associate the graphic list to the display for annotations.
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

            // Allocate a normalized grayscale model.
            MIL.MpatAlloc(MilSystem, MIL.M_NORMALIZED, MIL.M_DEFAULT, ref ContextId);

            // Define a regular model.
            MIL.MpatDefine(ContextId, MIL.M_REGULAR_MODEL, MilSourceImage,
                ROTATED_FIND_MODEL_X_POS, ROTATED_FIND_MODEL_Y_POS,
                ROTATED_FIND_MODEL_WIDTH, ROTATED_FIND_MODEL_HEIGHT, MIL.M_DEFAULT);

            // Activate the search model angle mode.
            MIL.MpatControl(ContextId, MIL.M_DEFAULT, MIL.M_SEARCH_ANGLE_MODE, MIL.M_ENABLE);

            // Disable the search model range angle.
            MIL.MpatControl(ContextId, MIL.M_DEFAULT, MIL.M_SEARCH_ANGLE_DELTA_NEG, 0);
            MIL.MpatControl(ContextId, MIL.M_DEFAULT, MIL.M_SEARCH_ANGLE_DELTA_POS, 0);

            // Set a specific angle.
            MIL.MpatControl(ContextId, 0, MIL.M_SEARCH_ANGLE, ROTATED_FIND_ROTATION_DELTA_ANGLE);

            // Preprocess the model.
            MIL.MpatPreprocess(ContextId, MIL.M_DEFAULT, MilSourceImage);

            // Allocate a result buffer.
            MIL.MpatAllocResult(MilSystem, MIL.M_DEFAULT, ref MilResult);

            // Draw the original model position.
            MIL.MpatDraw(MIL.M_DEFAULT, ContextId, GraphicList, MIL.M_DRAW_BOX + MIL.M_DRAW_POSITION,
                     MIL.M_DEFAULT, MIL.M_ORIGINAL);

            // Pause to show the original image and model position.
            Console.Write("\nA {0}x{1} model was defined in the source image.\n",
                      ROTATED_FIND_MODEL_WIDTH, ROTATED_FIND_MODEL_HEIGHT);
            Console.Write("It will be searched in an image rotated at {0} degrees.\n",
                      -ROTATED_FIND_ROTATION_DELTA_ANGLE);

            Console.Write("Press <Enter> to continue.\n");
            Console.ReadKey();

            // Rotate the source image -10 degrees.
            MIL.MimRotate(MilSourceImage, MilTargetImage, ROTATED_FIND_ROTATION_DELTA_ANGLE, MIL.M_DEFAULT,
                        MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_BILINEAR + MIL.M_OVERSCAN_CLEAR);

            MIL.MdispSelect(MilDisplay, MilTargetImage);

            // Dummy first call for bench measure purpose only (bench stabilization, 
            // cache effect, etc...). This first call is NOT required by the application.
            MIL.MpatFind(ContextId, MilTargetImage, MilResult);

            // Reset the timer.
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL);

            MIL.MpatFind(ContextId, MilTargetImage, MilResult);

            // Read the time spent in MpatFind().
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS, ref Time);

            // Clear the annotations.
            MIL.MgraClear(MIL.M_DEFAULT, GraphicList);

            MIL.MpatGetResult(MilResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT, ref NbFound);
            if (NbFound == 1)
            {
                MIL.MgraColor(MIL.M_DEFAULT, AnnotationColor);
                MIL.MpatDraw(MIL.M_DEFAULT, MilResult, GraphicList, MIL.M_DRAW_BOX + MIL.M_DRAW_POSITION, MIL.M_DEFAULT, MIL.M_DEFAULT);
                Console.Write("A search model at a specific angle has been found in the rotated image.\n");
                Console.Write("The search time is {0:F3} ms.\n\n", Time * 1000.0);
            }
            else
            {
                Console.Write("Model was not found!\n\n");
            }

            Console.Write("Press <Enter> to continue.\n\n");
            Console.ReadKey();

            // Disable the overlay display.
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_SHOW, MIL.M_DISABLE);

            // Free all allocations.
            MIL.MpatFree(MilResult);
            MIL.MpatFree(ContextId);
            MIL.MgraFree(GraphicList);
            MIL.MbufFree(MilTargetImage);
            MIL.MbufFree(MilSourceImage);
        }

        //*****************************************************************************
        // Automatic model allocation example.

        // Source and target images file specifications.
        private const string AUTO_MODEL_IMAGE_FILE = MIL.M_IMAGE_PATH + "Wafer.mim";
        private const string AUTO_MODEL_TARGET_IMAGE_FILE = MIL.M_IMAGE_PATH + "WaferShifted.mim";

        // Model width and height
        private const int AUTO_MODEL_WIDTH = 64;
        private const int AUTO_MODEL_HEIGHT = 64;

        static void AutoAllocationModelExample(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            MIL_ID MilImage = MIL.M_NULL;                       // Image buffer identifier.
            MIL_ID MilSubImage = MIL.M_NULL;                    // Sub-image buffer identifier.
            MIL_ID GraphicList = MIL.M_NULL;                    // Graphic list.
            MIL_ID ContextId = MIL.M_NULL;                      // Model identifier.
            MIL_ID Result = MIL.M_NULL;                         // Result buffer identifier.
            MIL_INT AllocError = 0;                             // Allocation error variable.
            MIL_INT NumResults = 0;                             // Number of results found.
            MIL_INT ImageWidth = 0;                             // Target image dimensions 
            MIL_INT ImageHeight = 0;
            double OrgX = 0.0;                                  // Original center of model.
            double OrgY = 0.0;
            double x = 0.0;                                     // Result variables.
            double y = 0.0;
            double Score = 0.0;
            double AnnotationColor = MIL.M_COLOR_GREEN;         // Drawing color.

            // Load model image into an image buffer.
            MIL.MbufRestore(AUTO_MODEL_IMAGE_FILE, MilSystem, ref MilImage);

            // Display the image.
            MIL.MdispSelect(MilDisplay, MilImage);

            // Allocate a graphic list to hold the subpixel annotations to draw.
            MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref GraphicList);

            // Associate the graphic list to the display for annotations.
            MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

            // Restrict the region to be processed to the bottom right corner of the image.
            MIL.MbufInquire(MilImage, MIL.M_SIZE_X, ref ImageWidth);
            MIL.MbufInquire(MilImage, MIL.M_SIZE_Y, ref ImageHeight);
            MIL.MbufChild2d(MilImage, ImageWidth / 2, ImageHeight / 2, ImageWidth / 2, ImageHeight / 2, ref MilSubImage);

            // Add an offset to the drawings so they are aligned with the processed child image.
            MIL.MgraControl(MIL.M_DEFAULT, MIL.M_DRAW_OFFSET_X, (double)-(ImageWidth / 2));
            MIL.MgraControl(MIL.M_DEFAULT, MIL.M_DRAW_OFFSET_Y, (double)-(ImageHeight / 2));

            // Automatically allocate a normalized grayscale type pattern matching context.
            MIL.MpatAlloc(MilSystem, MIL.M_NORMALIZED, MIL.M_DEFAULT, ref ContextId);

            // Define a unique model
            MIL.MpatDefine(ContextId, MIL.M_AUTO_MODEL, MilSubImage, MIL.M_DEFAULT, MIL.M_DEFAULT,
                       AUTO_MODEL_WIDTH, AUTO_MODEL_HEIGHT, MIL.M_DEFAULT);

            // Set the search accuracy to high.
            MIL.MpatControl(ContextId, MIL.M_DEFAULT, MIL.M_ACCURACY, MIL.M_HIGH);

            // Check for that model allocation was successful.
            MIL.MappGetError(MIL.M_DEFAULT, MIL.M_CURRENT, ref AllocError);
            if (AllocError == 0)
            {
                // Draw a box around the model.
                MIL.MgraColor(MIL.M_DEFAULT, AnnotationColor);
                MIL.MpatDraw(MIL.M_DEFAULT, ContextId, GraphicList, MIL.M_DRAW_BOX + MIL.M_DRAW_POSITION, MIL.M_DEFAULT, MIL.M_ORIGINAL);
                Console.Write("A model was automatically defined in the image.\n");
                Console.Write("Press <Enter> to continue.\n\n");
                Console.ReadKey();

                // Clear the annotations.
                MIL.MgraClear(MIL.M_DEFAULT, GraphicList);

                // Load target image into an image buffer.
                MIL.MbufLoad(AUTO_MODEL_TARGET_IMAGE_FILE, MilImage);

                // Allocate result.
                MIL.MpatAllocResult(MilSystem, MIL.M_DEFAULT, ref Result);

                // Preprocess the model.
                MIL.MpatPreprocess(ContextId, MIL.M_DEFAULT, MilSubImage);

                // Find model.
                MIL.MpatFind(ContextId, MilSubImage, Result);

                // If one model was found above the acceptance threshold set.
                MIL.MpatGetResult(Result, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT, ref NumResults);
                if (NumResults == 1)
                {
                    // Get results.
                    MIL.MpatGetResult(Result, MIL.M_DEFAULT, MIL.M_POSITION_X, ref x);
                    MIL.MpatGetResult(Result, MIL.M_DEFAULT, MIL.M_POSITION_Y, ref y);
                    MIL.MpatGetResult(Result, MIL.M_DEFAULT, MIL.M_SCORE, ref Score);

                    // Draw a box around the occurrence.
                    MIL.MgraColor(MIL.M_DEFAULT, AnnotationColor);
                    MIL.MpatDraw(MIL.M_DEFAULT, Result, GraphicList, MIL.M_DRAW_BOX + MIL.M_DRAW_POSITION, MIL.M_DEFAULT, MIL.M_DEFAULT);

                    // Analyze and print results.
                    MIL.MpatInquire(ContextId, MIL.M_DEFAULT, MIL.M_ORIGINAL_X, ref OrgX);
                    MIL.MpatInquire(ContextId, MIL.M_DEFAULT, MIL.M_ORIGINAL_Y, ref OrgY);
                    Console.Write("An image misaligned by 50 pixels in X and 20 pixels in Y was loaded.\n\n");
                    Console.Write("The image is found to be shifted by {0:0.00} in X, and {1:0.00} in Y.\n", x - OrgX, y - OrgY);
                    Console.Write("Model match score is {0:0.0} percent.\n", Score);
                    Console.Write("Press <Enter> to end.\n\n");
                    Console.ReadKey();
                }
                else
                {
                    Console.Write("Error: Pattern not found properly.\n");
                    Console.Write("Press <Enter> to end.\n\n");
                    Console.ReadKey();
                }

                // Free result buffer and model.
                MIL.MpatFree(Result);
                MIL.MpatFree(ContextId);
            }
            else
            {
                Console.Write("Error: Automatic model definition failed.\n");
                Console.Write("Press <Enter> to end.\n\n");
                Console.ReadKey();
            }

            // Remove the drawing offset.
            MIL.MgraControl(MIL.M_DEFAULT, MIL.M_DRAW_OFFSET_X, 0.0);
            MIL.MgraControl(MIL.M_DEFAULT, MIL.M_DRAW_OFFSET_Y, 0.0);

            // Free the graphic list.
            MIL.MgraFree(GraphicList);

            // Free child buffer and defaults.
            MIL.MbufFree(MilSubImage);
            MIL.MbufFree(MilImage);
        }
    }
}
