﻿// *****************************************************************************
// 
// File name: M3dblob.cs
//
// Synopsis: This program demonstrates how to use the 3d blob module to
//           identify objects in a scene and separate them into categories.
//           See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
using System;
using Matrox.MatroxImagingLibrary;

namespace M3dblob
{
    class Program
    {
        // Source file specification.
        private static readonly string  CONNECTORS_AND_WASHERS_FILE = MIL.M_IMAGE_PATH + "ConnectorsAndWashers.mbufc";
        private static readonly string  CONNECTORS_AND_WASHERS_ILLUSTRATION_FILE = MIL.M_IMAGE_PATH + "ConnectorsAndWashers.png";

        private static readonly string  TWISTY_PUZZLES_FILE = MIL.M_IMAGE_PATH + "TwistyPuzzles.mbufc";

        // Segmentation thresholds.
        private static readonly MIL_INT LOCAL_SEGMENTATION_MIN_NB_POINTS = 100;
        private static readonly MIL_INT LOCAL_SEGMENTATION_MAX_NB_POINTS = 10000;
        private static readonly double  LOCAL_SEGMENTATION_DISTANCE_THRESHOLD = 0.75; // in mm

        private static readonly MIL_INT PLANAR_SEGMENTATION_MIN_NB_POINTS = 5000;
        private static readonly double  PLANAR_SEGMENTATION_NORMAL_THRESHOLD = 20;    // in deg

        // --------------------------------------------------------------
        private static void Main(string[] args)
        {
            Console.WriteLine("[EXAMPLE NAME]");
            Console.WriteLine("M3dblob");
            Console.WriteLine();
            Console.WriteLine("[SYNOPSIS]");
            Console.WriteLine("This program demonstrates how to use the 3d blob analysis module to");
            Console.WriteLine("identify objects in a scene and separate them into categories.");
            Console.WriteLine();

            Console.WriteLine("[MODULES USED]");
            Console.WriteLine("Modules used: 3D Blob Analysis, 3D Image Processing,");
            Console.WriteLine("3D Display, Display, Buffer, and 3D Graphics.");
            Console.WriteLine();

            // Allocate defaults.
            MIL_ID MilApplication = MIL.M_NULL;
            MIL_ID MilSystem = MIL.M_NULL;
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL);

            // Allocate the displays.
            MIL_ID SceneDisplay = Alloc3dDisplayId(MilSystem);
            if (SceneDisplay == MIL.M_NULL)
                {
                MIL.MappFreeDefault(MilApplication, MilSystem, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL);
                return;
                }
            MIL.M3ddispControl(SceneDisplay, MIL.M_TITLE, "Scene");

            MIL_ID IllustrationDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_WINDOWED, MIL.M_NULL);
            MIL_INT IllustrationOffsetX = MIL.M3ddispInquire(SceneDisplay, MIL.M_SIZE_X);
            MIL.MdispControl(IllustrationDisplay, MIL.M_TITLE, "Objects to inspect");
            MIL.MdispControl(IllustrationDisplay, MIL.M_WINDOW_INITIAL_POSITION_X, IllustrationOffsetX);

            // Run the examples.
            IdentificationAndSortingExample(SceneDisplay, IllustrationDisplay);
            PlanarSegmentationExample(SceneDisplay, IllustrationDisplay);

            MIL.MdispFree(IllustrationDisplay);
            MIL.M3ddispFree(SceneDisplay);

            MIL.MappFreeDefault(MilApplication, MilSystem, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL);

            return;
        }

        //*****************************************************************************
        // Allocates a 3D or 2D display depending on which one is supported.
        //*****************************************************************************
        private static MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
        {
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);
            MIL_ID MilDisplay = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, MIL.M_NULL);
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);

           if (MilDisplay == MIL.M_NULL)
                {
                Console.WriteLine("The current system does not support the 3D display.");
                Console.WriteLine("Press any key to exit.");
                Console.ReadKey();
                }

            return MilDisplay;
        }

        //*****************************************************************************
        // First example.
        //*****************************************************************************
        private static void IdentificationAndSortingExample(MIL_ID SceneDisplay, MIL_ID IllustrationDisplay)
            {
            MIL_ID MilSystem = (MIL_ID)MIL.MobjInquire(SceneDisplay, MIL.M_OWNER_SYSTEM, MIL.M_NULL);
            MIL_ID SceneGraphicList = MIL.M3ddispInquire(SceneDisplay, MIL.M_3D_GRAPHIC_LIST_ID, MIL.M_NULL);

            // Restore the point cloud and display it.
            MIL_ID MilPointCloud = MIL.MbufImport(CONNECTORS_AND_WASHERS_FILE, MIL.M_DEFAULT, MIL.M_RESTORE, MilSystem, MIL.M_NULL);

            MIL.M3dgraRemove(SceneGraphicList, MIL.M_ALL, MIL.M_DEFAULT);
            MIL.M3dgraControl(SceneGraphicList, MIL.M_DEFAULT_SETTINGS, MIL.M_THICKNESS, 3);

            MIL.M3ddispSelect(SceneDisplay, MilPointCloud, MIL.M_DEFAULT, MIL.M_DEFAULT);
            MIL.M3ddispSetView(SceneDisplay, MIL.M_AUTO, MIL.M_TOP_TILTED, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            // Show an illustration of the objects in the scene.
            MIL_ID IllustrationImage = MIL.MbufRestore(CONNECTORS_AND_WASHERS_ILLUSTRATION_FILE, MilSystem, MIL.M_NULL);
            MIL.MdispSelect(IllustrationDisplay, IllustrationImage);

            Console.WriteLine("A 3D point cloud consisting of wire connectors and washers");
            Console.WriteLine("is restored from a file and displayed.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to segment it into separate objects.");
            Console.WriteLine();
            Console.ReadKey();

            // Allocate the segmentation contexts.
            MIL_ID SegmentationContext = MIL.M3dblobAlloc(MilSystem, MIL.M_SEGMENTATION_CONTEXT, MIL.M_DEFAULT, MIL.M_NULL);
            MIL_ID CalculateContext = MIL.M3dblobAlloc(MilSystem, MIL.M_CALCULATE_CONTEXT, MIL.M_DEFAULT, MIL.M_NULL);
            MIL_ID Draw3dContext = MIL.M3dblobAlloc(MilSystem, MIL.M_DRAW_3D_CONTEXT, MIL.M_DEFAULT, MIL.M_NULL);

            // Allocate the segmentation results. One result is used for each category.
            MIL_ID AllBlobs = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT, MIL.M_NULL);
            MIL_ID Connectors = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT, MIL.M_NULL);
            MIL_ID Washers = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT, MIL.M_NULL);
            MIL_ID UnknownBlobs = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT, MIL.M_NULL);

            // Segment the point cloud into several blobs.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NEIGHBOR_SEARCH_MODE, MIL.M_ORGANIZED);                  // Take advantage of the 2d organization.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NEIGHBORHOOD_ORGANIZED_SIZE, 5);                         // Look for neighbors in a 5x5 square kernel.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NUMBER_OF_POINTS_MIN, LOCAL_SEGMENTATION_MIN_NB_POINTS); // Exclude small isolated clusters.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NUMBER_OF_POINTS_MAX, LOCAL_SEGMENTATION_MAX_NB_POINTS); // Exclude extremely large clusters which make up the background.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_MAX_DISTANCE, LOCAL_SEGMENTATION_DISTANCE_THRESHOLD);    // Set the distance between points to be blobbed together.

            MIL.M3dblobSegment(SegmentationContext, MilPointCloud, AllBlobs, MIL.M_DEFAULT);

            // Draw all blobs in the 3d display.
            MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_ACTIVE, MIL.M_ENABLE);
            MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_THICKNESS, 3);
            MIL_INT AllBlobsLabel = MIL.M3dblobDraw3d(Draw3dContext, MilPointCloud, AllBlobs, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT);

            Console.WriteLine("The point cloud is segmented based on the distance between points.");
            Console.WriteLine("Points belonging to the background plane or small isolated clusters");
            Console.WriteLine("are excluded.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            // Calculate features on the blobs and use them to determine the type of object they represent.
            MIL.M3dblobControl(CalculateContext, MIL.M_DEFAULT, MIL.M_PCA_BOX, MIL.M_ENABLE);
            MIL.M3dblobControl(CalculateContext, MIL.M_DEFAULT, MIL.M_LINEARITY, MIL.M_ENABLE);
            MIL.M3dblobControl(CalculateContext, MIL.M_DEFAULT, MIL.M_PLANARITY, MIL.M_ENABLE);

            MIL.M3dblobCalculate(CalculateContext, MilPointCloud, AllBlobs, MIL.M_ALL, MIL.M_DEFAULT);

            // Connectors are more elongated than other blobs.
            // Use the feature M_LINEARITY, which is a value from 0 (perfect sphere/plane) to 1 (perfect line).
            MIL.M3dblobSelect(AllBlobs, Connectors, MIL.M_LINEARITY, MIL.M_GREATER, 0.5, MIL.M_NULL, MIL.M_DEFAULT);

            // Washers are flat and circular.
            // Use the feature M_PLANARITY, which is a value from 0 (perfect sphere) to 1 (perfect plane).
            MIL.M3dblobSelect(AllBlobs, Washers, MIL.M_LINEARITY, MIL.M_LESS, 0.2, MIL.M_NULL, MIL.M_DEFAULT);
            MIL.M3dblobSelect(Washers, Washers, MIL.M_PLANARITY, MIL.M_GREATER, 0.8, MIL.M_NULL, MIL.M_DEFAULT);

            // Blobs that are neither connectors nor washers are unknown objects.
            // Use M3dblobCombine to subtract already identified blobs from AllBlobs.
            MIL.M3dblobCombine(AllBlobs, Connectors, UnknownBlobs, MIL.M_SUB, MIL.M_DEFAULT);
            MIL.M3dblobCombine(UnknownBlobs, Washers, UnknownBlobs, MIL.M_SUB, MIL.M_DEFAULT);

            // Print the number of blobs in each category.
            MIL_INT NbConnectors = 0, NbWashers = 0, NbUnknown = 0;
            MIL.M3dblobGetResult(Connectors,   MIL.M_DEFAULT, MIL.M_NUMBER, ref NbConnectors);
            MIL.M3dblobGetResult(Washers,      MIL.M_DEFAULT, MIL.M_NUMBER, ref NbWashers);
            MIL.M3dblobGetResult(UnknownBlobs, MIL.M_DEFAULT, MIL.M_NUMBER, ref NbUnknown);

            Console.WriteLine("Simple 3D features (planarity, linearity) are calculated on the");
            Console.WriteLine("blobs and used to identify them.");
            Console.WriteLine();
            Console.WriteLine("The relevant objects (connectors and washers) have their");
            Console.WriteLine("bounding box displayed.");
            Console.WriteLine("Connectors (in red):     {0}", NbConnectors);
            Console.WriteLine("Washers (in green) :     {0}", NbWashers);
            Console.WriteLine("Unknown (in yellow):     {0}", NbUnknown);
            Console.WriteLine();

            // Draw the blobs in the 3d display.
            MIL.M3dgraRemove(SceneGraphicList, AllBlobsLabel, MIL.M_DEFAULT);

            MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_COLOR, MIL.M_COLOR_YELLOW);
            MIL.M3dblobDraw3d(Draw3dContext, MilPointCloud, UnknownBlobs, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT);

            MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_PCA_BOX, MIL.M_ACTIVE, MIL.M_ENABLE);
            MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_COLOR, MIL.M_COLOR_RED);
            MIL.M3dblobDraw3d(Draw3dContext, MilPointCloud, Connectors, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT);

            MIL.M3dblobControlDraw(Draw3dContext, MIL.M_DRAW_BLOBS, MIL.M_COLOR, MIL.M_COLOR_GREEN);
            MIL.M3dblobDraw3d(Draw3dContext, MilPointCloud, Washers, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT);

            Console.WriteLine("Press <Enter> to continue.");
            Console.WriteLine();
            Console.ReadKey();

            MIL.M3dblobFree(UnknownBlobs);
            MIL.M3dblobFree(Washers);
            MIL.M3dblobFree(Connectors);
            MIL.M3dblobFree(AllBlobs);
            MIL.M3dblobFree(Draw3dContext);
            MIL.M3dblobFree(CalculateContext);
            MIL.M3dblobFree(SegmentationContext);

            MIL.MbufFree(IllustrationImage);
            MIL.MbufFree(MilPointCloud);

            }

        //*****************************************************************************
        // Second example.
        //*****************************************************************************
        private static void PlanarSegmentationExample(MIL_ID SceneDisplay, MIL_ID IllustrationDisplay)
            {
            MIL_ID MilSystem = (MIL_ID)MIL.MobjInquire(SceneDisplay, MIL.M_OWNER_SYSTEM, MIL.M_NULL);
            MIL_ID SceneGraphicList = MIL.M3ddispInquire(SceneDisplay, MIL.M_3D_GRAPHIC_LIST_ID, MIL.M_NULL);

            // Restore the point cloud and display it.
            MIL_ID MilPointCloud = MIL.MbufImport(TWISTY_PUZZLES_FILE, MIL.M_DEFAULT, MIL.M_RESTORE, MilSystem, MIL.M_NULL);

            MIL.M3dgraRemove(SceneGraphicList, MIL.M_ALL, MIL.M_DEFAULT);
            MIL.M3dgraControl(SceneGraphicList, MIL.M_DEFAULT_SETTINGS, MIL.M_THICKNESS, 1);

            MIL.M3ddispSelect(SceneDisplay, MilPointCloud, MIL.M_DEFAULT, MIL.M_DEFAULT);
            MIL.M3ddispSetView(SceneDisplay, MIL.M_AUTO, MIL.M_TOP_TILTED, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            Console.WriteLine("Another point cloud containing various twisty puzzles is restored.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to segment it into separate objects.");
            Console.WriteLine();
            Console.ReadKey();

            // Allocate the segmentation objects.
            MIL_ID SegmentationContext = MIL.M3dblobAlloc(MilSystem, MIL.M_SEGMENTATION_CONTEXT, MIL.M_DEFAULT, MIL.M_NULL);
            MIL_ID SegmentationResult = MIL.M3dblobAllocResult(MilSystem, MIL.M_SEGMENTATION_RESULT, MIL.M_DEFAULT, MIL.M_NULL);

            // First segment the point cloud with only local thresholds.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NEIGHBOR_SEARCH_MODE, MIL.M_ORGANIZED);                    // Take advantage of the 2d organization.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NEIGHBORHOOD_ORGANIZED_SIZE, 5);                           // Look for neighbors in a 5x5 square kernel.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NUMBER_OF_POINTS_MIN, PLANAR_SEGMENTATION_MIN_NB_POINTS);  // Exclude small isolated clusters.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_MAX_DISTANCE_MODE, MIL.M_AUTO);                            // Use an automatic local distance threshold.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NORMAL_DISTANCE_MAX_MODE, MIL.M_AUTO);                     // Use an automatic local normal threshold.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NORMAL_DISTANCE_MODE, MIL.M_ORIENTATION);                  // Consider flipped normals to be the same.

            MIL.M3dblobSegment(SegmentationContext, MilPointCloud, SegmentationResult, MIL.M_DEFAULT);

            MIL_INT AnnotationLabel = MIL.M3dblobDraw3d(MIL.M_DEFAULT, MilPointCloud, SegmentationResult, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT);

            Console.WriteLine("The point cloud is segmented based on local thresholds (distance, normals).");
            Console.WriteLine();
            Console.WriteLine("Local thresholds can separate distinct objects due to camera occlusions,");
            Console.WriteLine("but are often not enough to segment a single object into subparts.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to use global thresholds instead.");
            Console.WriteLine();
            Console.ReadKey();

            // Then segment again with global thresholds.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_NORMAL_DISTANCE_MAX_MODE, MIL.M_USER_DEFINED);                      // Remove the local normal threshold.
            MIL.M3dblobControl(SegmentationContext, MIL.M_DEFAULT, MIL.M_GLOBAL_NORMAL_DISTANCE_MAX, PLANAR_SEGMENTATION_NORMAL_THRESHOLD);  // Use a global normal threshold instead.

            MIL.M3dblobSegment(SegmentationContext, MilPointCloud, SegmentationResult, MIL.M_DEFAULT);

            MIL.M3dgraRemove(SceneGraphicList, AnnotationLabel, MIL.M_DEFAULT);
            AnnotationLabel = MIL.M3dblobDraw3d(MIL.M_DEFAULT, MilPointCloud, SegmentationResult, MIL.M_ALL, SceneGraphicList, MIL.M_ROOT_NODE, MIL.M_DEFAULT);

            Console.WriteLine("The puzzles' sides are now separated.");
            Console.WriteLine();
            Console.WriteLine("Press <Enter> to end.");
            Console.WriteLine();
            Console.ReadKey();

            MIL.M3dblobFree(SegmentationContext);
            MIL.M3dblobFree(SegmentationResult);

            MIL.MbufFree(MilPointCloud);
            }
        }
    }
