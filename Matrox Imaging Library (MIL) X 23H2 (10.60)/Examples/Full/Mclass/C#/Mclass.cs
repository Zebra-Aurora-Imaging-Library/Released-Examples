﻿//*****************************************************************************
// 
// File name: Mclass.cs
//
// Synopsis:  This example identifies the type of pastas using a 
// pre-trained classification module. 
//
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*****************************************************************************
using System;
using System.Runtime.InteropServices;
using Matrox.MatroxImagingLibrary;

namespace Mclass
{
    // Function declarations.
    class ClassStruct
    {
        public MIL_INT NbCategories;
        public MIL_INT NbOfFrames;
        public MIL_INT SourceSizeX;
        public MIL_INT SourceSizeY;

        public MIL_ID ClassCtx;
        public MIL_ID ClassRes;
        public MIL_ID MilDisplay;
        public MIL_ID MilDispChild;
        public MIL_ID MilOverlayImage;
    }

    public class Program
    {
        // Path definitions.
        private const string EXAMPLE_IMAGE_DIR_PATH = MIL.M_IMAGE_PATH + "/Classification/Pasta/";
        private const string EXAMPLE_CLASS_CTX_PATH = EXAMPLE_IMAGE_DIR_PATH + "MatroxNet_PastaEx.mclass";
        private const string TARGET_IMAGE_DIR_PATH = EXAMPLE_IMAGE_DIR_PATH + "Products";

        private const string DIG_IMAGE_FOLDER = TARGET_IMAGE_DIR_PATH;
        private const string DIG_REMOTE_IMAGE_FOLDER = "remote:///" + TARGET_IMAGE_DIR_PATH;

        // Util constant.
        private const int BUFFERING_SIZE_MAX = 10;

        ///****************************************************************************
        //    Main.
        ///****************************************************************************
        static int Main(string[] args)
        {
            MIL_ID MilApplication = MIL.M_NULL;    // MIL application identifier
            MIL_ID MilSystem = MIL.M_NULL;         // MIL system identifier
            MIL_ID MilDisplay = MIL.M_NULL;        // MIL display identifier
            MIL_ID MilOverlay = MIL.M_NULL;        // MIL overlay identifier
            MIL_ID MilDigitizer = MIL.M_NULL;      // MIL digitizer identifier
            MIL_ID MilDispImage = MIL.M_NULL;      // MIL image identifier
            MIL_ID MilDispChild = MIL.M_NULL;      // MIL image identifier
            MIL_ID ClassCtx = MIL.M_NULL;          // MIL classification Context
            MIL_ID ClassRes = MIL.M_NULL;          // MIL classification Result

            MIL_ID[] MilGrabBufferList = new MIL_ID[BUFFERING_SIZE_MAX];   // MIL image identifier
            MIL_ID[] MilChildBufferList = new MIL_ID[BUFFERING_SIZE_MAX];  // MIL child identifier

            MIL_INT NumberOfCategories = 0;
            MIL_INT BufIndex = 0;
            MIL_INT SourceSizeX = 0;
            MIL_INT SourceSizeY = 0;
            MIL_INT InputSizeX = 0;
            MIL_INT InputSizeY = 0;

            // Allocate MIL objects.
            MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT, ref MilApplication);
            MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilSystem);
            MIL_INT SystemType = 0;
            MIL.MsysInquire(MilSystem, MIL.M_SYSTEM_TYPE, ref SystemType);
            if (SystemType != MIL.M_SYSTEM_HOST_TYPE)
                {
                MIL.MsysFree(MilSystem);
                MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_HOST, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilSystem);
                }

            MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "MIL.M_DEFAULT", MIL.M_DEFAULT, ref MilDisplay);

            MIL_INT MilSystemLocation = MIL.MsysInquire(MilSystem, MIL.M_LOCATION, MIL.M_NULL);
            string DigImageFolder = (MilSystemLocation == MIL.M_REMOTE) ? DIG_REMOTE_IMAGE_FOLDER : DIG_IMAGE_FOLDER;
            MIL.MdigAlloc(MilSystem, MIL.M_DEFAULT, DigImageFolder, MIL.M_DEFAULT, ref MilDigitizer);

            // Print the example synopsis.
            Console.WriteLine("[EXAMPLE NAME]");
            Console.WriteLine("Mclass");
            Console.WriteLine();
            Console.WriteLine("[SYNOPSIS]");
            Console.WriteLine("This programs shows the use of a pre-trained classification");
            Console.WriteLine("tool to recognize product categories.");
            Console.WriteLine();
            Console.WriteLine("[MODULES USED]");
            Console.WriteLine("Classification, Buffer, Display, Graphics, Image Processing.");
            Console.WriteLine();

            // Wait for user.
            Console.WriteLine("Press <Enter> to continue.");
            Console.ReadKey();

            Console.Write("Restoring the classification context from file..");
            MIL.MclassRestore(EXAMPLE_CLASS_CTX_PATH, MilSystem, MIL.M_DEFAULT, ref ClassCtx);
            Console.Write(".");

            // Preprocess the context.
            MIL.MclassPreprocess(ClassCtx, MIL.M_DEFAULT);
            Console.WriteLine(".ready.");

            MIL.MclassInquire(ClassCtx, MIL.M_CONTEXT, MIL.M_NUMBER_OF_CLASSES + MIL.M_TYPE_MIL_INT, ref NumberOfCategories);
            MIL.MclassInquire(ClassCtx, MIL.M_DEFAULT_SOURCE_LAYER, MIL.M_SIZE_X + MIL.M_TYPE_MIL_INT, ref InputSizeX);
            MIL.MclassInquire(ClassCtx, MIL.M_DEFAULT_SOURCE_LAYER, MIL.M_SIZE_Y + MIL.M_TYPE_MIL_INT, ref InputSizeY);

            if (NumberOfCategories > 0)
            {
                // Inquire and print source layer information.
                Console.WriteLine(" - The classifier was trained to recognize {0} categories", NumberOfCategories);
                Console.WriteLine(" - The classifier was trained for {0}x{1} source images", InputSizeX, InputSizeY);
                Console.WriteLine();

                // Allocate a classification result buffer.
                MIL.MclassAllocResult(MilSystem, MIL.M_PREDICT_CNN_RESULT, MIL.M_DEFAULT, ref ClassRes);

                // Inquire the size of the source image.
                MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X, ref SourceSizeX);
                MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y, ref SourceSizeY);

                // Setup the example display.
                SetupDisplay(MilSystem,
                             MilDisplay,
                             SourceSizeX,
                             SourceSizeY,
                             ClassCtx,
                             out MilDispImage,
                             out MilDispChild,
                             out MilOverlay,
                             NumberOfCategories);

                // Retrieve the number of frame in the source directory.
                MIL_INT NumberOfFrames = 0;
                MIL.MdigInquire(MilDigitizer, MIL.M_SOURCE_NUMBER_OF_FRAMES, ref NumberOfFrames);

                // Prepare data for Hook Function.
                ClassStruct ClassificationData = new ClassStruct();
                ClassificationData.ClassCtx = ClassCtx;
                ClassificationData.ClassRes = ClassRes;
                ClassificationData.MilDisplay = MilDisplay;
                ClassificationData.MilDispChild = MilDispChild;
                ClassificationData.NbCategories = NumberOfCategories;
                ClassificationData.MilOverlayImage = MilOverlay;
                ClassificationData.SourceSizeX = SourceSizeX;
                ClassificationData.SourceSizeY = SourceSizeY;
                ClassificationData.NbOfFrames = NumberOfFrames;

                // Allocate the grab buffers.
                for (BufIndex = 0; BufIndex < BUFFERING_SIZE_MAX; BufIndex++)
                    {
                    MIL.MbufAlloc2d(MilSystem, SourceSizeX, SourceSizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_GRAB + MIL.M_PROC, ref MilGrabBufferList[BufIndex]);
                    MIL.MbufChild2d(MilGrabBufferList[BufIndex], (SourceSizeX - InputSizeX) / 2, (SourceSizeY - InputSizeY) / 2, InputSizeX, InputSizeY, ref MilChildBufferList[BufIndex]);
                    MIL.MobjControl(MilGrabBufferList[BufIndex], MIL.M_OBJECT_USER_DATA_PTR, MilChildBufferList[BufIndex]);
                    }


                // Start the grab.
                MIL_DIG_HOOK_FUNCTION_PTR ClassificationFuncHook = new MIL_DIG_HOOK_FUNCTION_PTR(ClassificationFunc);
                GCHandle ClassificationDataHandle = GCHandle.Alloc(ClassificationData);
                if (NumberOfFrames != MIL.M_INFINITE)
                    MIL.MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE_MAX, MIL.M_SEQUENCE + MIL.M_COUNT(NumberOfFrames), MIL.M_SYNCHRONOUS, ClassificationFuncHook, GCHandle.ToIntPtr(ClassificationDataHandle));
                else
                    MIL.MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE_MAX, MIL.M_START, MIL.M_DEFAULT, ClassificationFuncHook, GCHandle.ToIntPtr(ClassificationDataHandle));

                // Ready to exit.
                Console.WriteLine();
                Console.WriteLine("Press <Enter> to exit.");
                Console.ReadKey();

                // Stop the digitizer.
                MIL.MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE_MAX, MIL.M_STOP, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_NULL);

                ClassificationDataHandle.Free();
                GC.KeepAlive(ClassificationFuncHook);

                // Free the allocated resources.
                MIL.MbufFree(MilDispChild);
                MIL.MbufFree(MilDispImage);

                for (BufIndex = 0; BufIndex < BUFFERING_SIZE_MAX; BufIndex++)
                    {
                    MIL.MbufFree(MilChildBufferList[BufIndex]);
                    MIL.MbufFree(MilGrabBufferList[BufIndex]);
                    }

                MIL.MclassFree(ClassRes);
                MIL.MclassFree(ClassCtx);
            }

            // Free the allocated resources.
            MIL.MdigFree(MilDigitizer);

            MIL.MdispFree(MilDisplay);
            MIL.MsysFree(MilSystem);
            MIL.MappFree(MilApplication);

            return 0;
        }

        private static void SetupDisplay(MIL_ID MilSystem,
                  MIL_ID MilDisplay,
                  MIL_INT SourceSizeX,
                  MIL_INT SourceSizeY,
                  MIL_ID ClassCtx,
                  out MIL_ID MilDispImage,
                  out MIL_ID MilDispChild,
                  out MIL_ID MilOverlay,
                  MIL_INT NbCategories
                  )
        {
            MIL_ID MilImageLoader = MIL.M_NULL;  // MIL image identifier       
            MIL_ID MilChildSample = MIL.M_NULL;  // MIL child image identifier

            // Allocate a color buffer.
            MIL_INT IconSize = SourceSizeY / NbCategories;
            MilDispImage = MIL.MbufAllocColor(MilSystem, 3, SourceSizeX + IconSize, SourceSizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP, MIL.M_NULL);
            MIL.MbufClear(MilDispImage, MIL.M_COLOR_BLACK);
            MilDispChild = MIL.MbufChild2d(MilDispImage, 0, 0, SourceSizeX, SourceSizeY, MIL.M_NULL);

            // Set annotation color.
            MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED);

            // Setup the display.
            for (int iter = 0; iter < NbCategories; iter++)
            {
                // Allocate a child buffer per product categorie.   
                MIL.MbufChild2d(MilDispImage, SourceSizeX, iter * IconSize, IconSize, IconSize, ref MilChildSample);

                // Load the sample image.
                MIL.MclassInquire(ClassCtx, MIL.M_CLASS_INDEX(iter), MIL.M_CLASS_ICON_ID + MIL.M_TYPE_MIL_ID, ref MilImageLoader);

                if (MilImageLoader != MIL.M_NULL)
                { MIL.MimResize(MilImageLoader, MilChildSample, MIL.M_FILL_DESTINATION, MIL.M_FILL_DESTINATION, MIL.M_BICUBIC + MIL.M_OVERSCAN_FAST); }

                // Draw an initial red rectangle around the buffer.
                MIL.MgraRect(MIL.M_DEFAULT, MilChildSample, 0, 1, IconSize - 1, IconSize - 2);

                // Free the allocated buffers.
                MIL.MbufFree(MilChildSample);
            }

            // Display the window with black color.
            MIL.MdispSelect(MilDisplay, MilDispImage);

            // Prepare for overlay annotations.
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE);
            MilOverlay = (MIL_ID)MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID, MIL.M_NULL);
        }

        private static MIL_INT ClassificationFunc(MIL_INT HookType, MIL_ID EventId, IntPtr DataPtr)
        {
            MIL_ID MilImage = MIL.M_NULL;
            MIL_ID pMilInputImage = MIL.M_NULL;

            MIL.MdigGetHookInfo(EventId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID, ref MilImage);

            ClassStruct data = (ClassStruct)GCHandle.FromIntPtr(DataPtr).Target;
            MIL.MdispControl(data.MilDisplay, MIL.M_UPDATE, MIL.M_DISABLE);
            pMilInputImage = (MIL_ID)MIL.MobjInquire(MilImage, MIL.M_OBJECT_USER_DATA_PTR, MIL.M_NULL);

            // Display the new target image.
            MIL.MbufCopy(MilImage, data.MilDispChild);

            // Perform product recognition using the classification module.
            MIL.MclassPredict(data.ClassCtx, pMilInputImage, data.ClassRes, MIL.M_DEFAULT);

            // Retrieve best classification score and class index.
            double BestScore = 0;
            MIL.MclassGetResult(data.ClassRes, MIL.M_GENERAL, MIL.M_BEST_CLASS_SCORE + MIL.M_TYPE_MIL_DOUBLE, ref BestScore);

            MIL_INT BestIndex = 0;
            MIL.MclassGetResult(data.ClassRes, MIL.M_GENERAL, MIL.M_BEST_CLASS_INDEX + MIL.M_TYPE_MIL_INT, ref BestIndex);

            // Clear the overlay buffer.
            MIL.MdispControl(data.MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_TRANSPARENT_COLOR);

            // Draw a green rectangle around the winning sample.
            MIL_INT IconSize = data.SourceSizeY / data.NbCategories;
            MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN);
            MIL.MgraRect(MIL.M_DEFAULT, data.MilOverlayImage, data.SourceSizeX, (BestIndex * IconSize) + 1, data.SourceSizeX + IconSize - 1, (BestIndex + 1) * IconSize - 2);

            // Print the classification accuracy in the sample buffer.
            string Accuracy_text = string.Format("{0:N1}% score", BestScore);
            MIL.MgraControl(MIL.M_DEFAULT, MIL.M_BACKGROUND_MODE, MIL.M_TRANSPARENT);
            MIL.MgraFont(MIL.M_DEFAULT, MIL.M_FONT_DEFAULT_SMALL);
            MIL.MgraText(MIL.M_DEFAULT, data.MilOverlayImage, data.SourceSizeX + 2, BestIndex * IconSize + 4, Accuracy_text);

            // Update the display.
            MIL.MdispControl(data.MilDisplay, MIL.M_UPDATE, MIL.M_ENABLE);

            // Wait for the user.
            if (data.NbOfFrames != MIL.M_INFINITE)
            {
                Console.Write("Press <Enter> to continue.\r");
                Console.ReadKey();
            }

            return 0;
        }

    }
}