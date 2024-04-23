// *****************************************************************************
// 
// File name: M3dreg.cs
//
// Synopsis: This example demonstrates how to use the 3D registration module
//           to stitch several partial point clouds of a 3D object together
//           into a single complete point cloud.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
using System;
using System.Threading;
using Matrox.MatroxImagingLibrary;

namespace M3dreg
{
    class Program
    {
        // ----------------------------------------------------------------------------
        // Example description.
        // ----------------------------------------------------------------------------
        private static void PrintHeader()
        {
            Console.WriteLine("[EXAMPLE NAME]");
            Console.WriteLine("M3dreg");
            Console.WriteLine();
            Console.WriteLine("[SYNOPSIS]");
            Console.WriteLine("This example demonstrates how to use the 3D Registration module ");
            Console.WriteLine("to stitch several partial point clouds of a 3D object together ");
            Console.WriteLine("into a single complete point cloud.");
            Console.WriteLine();

            Console.WriteLine("[MODULES USED]");
            Console.WriteLine("Modules used: 3D Registration, 3D Display, 3D Graphics, and 3D Image\nProcessing.");
            Console.WriteLine();
        }

        // Input scanned point cloud (PLY) files.
        private static readonly MIL_INT NUM_SCANS = 6;
        static readonly string[] FILE_POINT_CLOUD = new string[]
        {
               MIL.M_IMAGE_PATH + "Cloud1.ply",
               MIL.M_IMAGE_PATH + "Cloud2.ply",
               MIL.M_IMAGE_PATH + "Cloud3.ply",
               MIL.M_IMAGE_PATH + "Cloud4.ply",
               MIL.M_IMAGE_PATH + "Cloud5.ply",
               MIL.M_IMAGE_PATH + "Cloud6.ply",
        };

        // The colors assigned for each cloud.
        static readonly MIL_INT[] Color = new MIL_INT[]
        {
               MIL.M_RGB888(0,   159, 255),
               MIL.M_RGB888(154,  77,  66),
               MIL.M_RGB888(0,   255, 190),
               MIL.M_RGB888(120,  63, 193),
               MIL.M_RGB888(31,  150, 152),
               MIL.M_RGB888(255, 172, 253),
               MIL.M_RGB888(177, 204, 113)
        };

        // --------------------------------------------------------------
        private static void Main(string[] args)
        {
            // Print example information in console.
            PrintHeader();

            MIL_ID MilApplication = MIL.M_NULL;    // MIL application identifier
            MIL_ID MilSystem = MIL.M_NULL;         // MIL system identifier
            MIL_ID MilDisplay = MIL.M_NULL;        // MIL display identifier

            // Allocate MIL objects.
            MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT, ref MilApplication);
            MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilSystem);

            MIL_ID[] MilContainerIds = new MIL_ID[NUM_SCANS];
            Console.Write("Reading the PLY files of 6 partial point clouds");
            for (MIL_INT i = 0; i < NUM_SCANS; ++i)
            {
                Console.Write(".");
                MilContainerIds[i] = MIL.MbufImport(FILE_POINT_CLOUD[i], MIL.M_DEFAULT, MIL.M_RESTORE, MilSystem, MIL.M_NULL);
                ColorCloud(MilContainerIds[i], Color[i]);
            }
            Console.WriteLine();
            Console.WriteLine();

            MilDisplay = Alloc3dDisplayId(MilSystem);

            MIL_ID MilDisplayImage = MIL.M_NULL; // Used for 2D display if needed.
            MIL_ID MilDepthMap     = MIL.M_NULL; // Used for 2D display if needed.

            // Display the first point cloud container.
            DisplayContainer(MilSystem, MilDisplay, MilContainerIds[0], ref MilDepthMap, ref MilDisplayImage);
            AutoRotateDisplay(MilSystem, MilDisplay);

            Console.WriteLine("Showing the first partial point cloud of the object.");
            Console.WriteLine("Press <Enter> to start.");
            Console.WriteLine();
            Console.ReadKey();

            // Allocate context and result for 3D registration (stitching).
            MIL_ID MilContext = MIL.M3dregAlloc(MilSystem, MIL.M_PAIRWISE_REGISTRATION_CONTEXT, MIL.M_DEFAULT, MIL.M_NULL);
            MIL_ID MilResult  = MIL.M3dregAllocResult(MilSystem, MIL.M_PAIRWISE_REGISTRATION_RESULT, MIL.M_DEFAULT, MIL.M_NULL);

            MIL.M3dregControl(MilContext, MIL.M_DEFAULT, MIL.M_NUMBER_OF_REGISTRATION_ELEMENTS, NUM_SCANS);
            MIL.M3dregControl(MilContext, MIL.M_DEFAULT, MIL.M_MAX_ITERATIONS, 40);

            // Pairwise registration context controls.
            // Use normal subsampling to preserve edges and yield faster registration.
            MIL_ID MilSubsampleContext = MIL.M_NULL;
            MIL.M3dregInquire(MilContext, MIL.M_DEFAULT, MIL.M_SUBSAMPLE_CONTEXT_ID, ref MilSubsampleContext);
            MIL.M3dregControl(MilContext, MIL.M_DEFAULT, MIL.M_SUBSAMPLE, MIL.M_ENABLE);

            // Keep edge points.
            MIL.M3dimControl(MilSubsampleContext, MIL.M_SUBSAMPLE_MODE, MIL.M_SUBSAMPLE_NORMAL);
            MIL.M3dimControl(MilSubsampleContext, MIL.M_NEIGHBORHOOD_DISTANCE, 10);

            // Chain of set location, i==0 is referencing to the GLOBAL.
            for (MIL_INT i = 1; i < NUM_SCANS; ++i)
            {
                MIL.M3dregSetLocation(MilContext, i, i - 1, MIL.M_IDENTITY_MATRIX, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
            }

            Console.WriteLine("The 3D stitching between partial point clouds has been performed with");
            Console.WriteLine("the help of the points within the expected common overlap regions.");
            Console.WriteLine();

            // Calculate the time to perform the registration.
            double ComputationTimeMS;
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET, MIL.M_NULL);

            // Perform the registration (stitching).
            MIL.M3dregCalculate(MilContext, MilContainerIds, NUM_SCANS, MilResult, MIL.M_DEFAULT);
            ComputationTimeMS = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ, MIL.M_NULL) * 1000.0;

            Console.Write("The registration of the 6 partial point clouds ");
            Console.WriteLine("succeeded in {0,0:f2} ms .", ComputationTimeMS);
            Console.WriteLine();

            // Merging overlapping point clouds will result into unneeded large number of points at
            // the overlapping regions.                                                            
            // During merging, subsampling is used to ensure that the density of points
            // is fairly dense, but without replications.
            double GridSize = 0.0;
            MIL_ID StatResultId = MIL.M3dimAllocResult(MilSystem, MIL.M_STATISTICS_RESULT, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dimStat(MIL.M_STAT_CONTEXT_DISTANCE_TO_NEAREST_NEIGHBOR, MilContainerIds[0], StatResultId, MIL.M_DEFAULT);

            // Nearest neighbour distances gives a measure of the point cloud density.
            MIL.M3dimGetResult(StatResultId, MIL.M_DISTANCE_TO_NEAREST_NEIGHBOR_MIN, ref GridSize);

            // Use the measured point cloud density as guide for the subsampling.
            MIL_ID MilMergeSubsampleContext = MIL.M3dimAlloc(MilSystem, MIL.M_SUBSAMPLE_CONTEXT, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dimControl(MilMergeSubsampleContext, MIL.M_SUBSAMPLE_MODE, MIL.M_SUBSAMPLE_GRID);
            MIL.M3dimControl(MilMergeSubsampleContext, MIL.M_GRID_SIZE_X, GridSize);
            MIL.M3dimControl(MilMergeSubsampleContext, MIL.M_GRID_SIZE_Y, GridSize);
            MIL.M3dimControl(MilMergeSubsampleContext, MIL.M_GRID_SIZE_Z, GridSize);

            // Allocate the point cloud for the final stitched clouds.
            MIL_ID MilStitchedId = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC + MIL.M_DISP, MIL.M_DEFAULT, MIL.M_NULL);

            Console.WriteLine("The merging of point clouds will be shown incrementally.");
            Console.WriteLine("Press <Enter> to show 2 merged point clouds of 6.");
            Console.WriteLine();
            Console.ReadKey();

            // Merge can merge all clouds at once, but here it is done incrementally for the display
            for (MIL_INT i = 2; i <= NUM_SCANS; ++i)
            {
                MIL.M3dregMerge(MilResult, MilContainerIds, i, MilStitchedId, MilMergeSubsampleContext, MIL.M_DEFAULT);

                if (i == 2)
                {
                    DisplayContainer(MilSystem, MilDisplay, MilStitchedId, ref MilDepthMap, ref MilDisplayImage);
                }
                else
                {
                    UpdateDisplay(MilSystem, MilStitchedId, MilDepthMap, MilDisplayImage);
                }

                if (i < NUM_SCANS)
                {
                    Console.WriteLine("Press <Enter> to show {0} merged point clouds of 6.", i + 1);
                    Console.WriteLine();
                }
                else
                {
                    Console.WriteLine("Press <Enter> to end.");
                    Console.WriteLine();
                }

                AutoRotateDisplay(MilSystem, MilDisplay);
                Console.ReadKey();
            }

            // Free Objects
            for (MIL_INT i = 0; i < NUM_SCANS; ++i)
            {
                MIL.MbufFree(MilContainerIds[i]);
            }

            MIL.MbufFree(MilStitchedId);
            MIL.M3dimFree(StatResultId);
            MIL.M3dimFree(MilMergeSubsampleContext);
            MIL.M3dregFree(MilContext);
            MIL.M3dregFree(MilResult);
            FreeDisplay(MilDisplay);

            if (MilDisplayImage != MIL.M_NULL)
            {
                MIL.MbufFree(MilDisplayImage);
            }

            if (MilDepthMap != MIL.M_NULL)
            {
                MIL.MbufFree(MilDepthMap);
            }

            MIL.MsysFree(MilSystem);
            MIL.MappFree(MilApplication);
        }

        // --------------------------------------------------------------
        // Color the container  
        // --------------------------------------------------------------
        private static void ColorCloud(MIL_ID MilPointCloud, MIL_INT Col)
        {
            MIL_INT SizeX = MIL.MbufInquireContainer(MilPointCloud, MIL.M_COMPONENT_RANGE, MIL.M_SIZE_X, MIL.M_NULL);
            MIL_INT SizeY = MIL.MbufInquireContainer(MilPointCloud, MIL.M_COMPONENT_RANGE, MIL.M_SIZE_Y, MIL.M_NULL);
            MIL_ID ReflectanceId = MIL.MbufAllocComponent(MilPointCloud, 3, SizeX, SizeY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE, MIL.M_COMPONENT_REFLECTANCE, MIL.M_NULL);
            MIL.MbufClear(ReflectanceId, (double)Col);
        }

        // --------------------------------------------------------------
        // Auto rotate the 3D object
        // --------------------------------------------------------------
        private static void AutoRotateDisplay(MIL_ID MilSystem, MIL_ID MilDisplay)
        {
            long DisplayType = 0;
            MIL.MobjInquire(MilDisplay, MIL.M_OBJECT_TYPE, ref DisplayType);

            // AutoRotate available only for the 3D display.
            if (DisplayType == MIL.M_DISPLAY)
            {
                return;
            }

            // By default the display rotates around the Z axis, but the robot is oriented along the Y axis. 
            MIL_ID Geometry = MIL.M3dgeoAlloc(MilSystem, MIL.M_GEOMETRY, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3ddispCopy(MilDisplay, Geometry, MIL.M_ROTATION_AXIS, MIL.M_DEFAULT);
            double CenterX = MIL.M3dgeoInquire(Geometry, MIL.M_START_POINT_X, MIL.M_NULL);
            double CenterY = MIL.M3dgeoInquire(Geometry, MIL.M_START_POINT_Y, MIL.M_NULL);
            double CenterZ = MIL.M3dgeoInquire(Geometry, MIL.M_START_POINT_Z, MIL.M_NULL);
            MIL.M3dgeoLine(Geometry, MIL.M_POINT_AND_VECTOR, MIL.M_UNCHANGED, MIL.M_UNCHANGED, MIL.M_UNCHANGED, 0, 1, 0, MIL.M_UNCHANGED, MIL.M_DEFAULT);
            MIL.M3ddispCopy(Geometry, MilDisplay, MIL.M_ROTATION_AXIS, MIL.M_DEFAULT);
            MIL.M3ddispControl(MilDisplay, MIL.M_AUTO_ROTATE, MIL.M_ENABLE);
            MIL.M3dgeoFree(Geometry);

        }

        // --------------------------------------------------------------
        // Allocates a 3D display and returns its MIL identifier.
        // --------------------------------------------------------------
        private static MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
        {
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);
            MIL_ID MilDisplay = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, MIL.M_NULL);
            MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);

            if (MilDisplay == MIL.M_NULL)
            {
                Console.WriteLine();

                Console.WriteLine("The current system does not support the 3D display.");
                Console.WriteLine("A 2D display will be used instead.");
                Console.WriteLine("Press any key to continue.");
                Console.ReadKey();

                // Allocate a 2D Display instead.
                MilDisplay = MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_WINDOWED, MIL.M_NULL);
            }
            else
            {
                // Adjust the viewpoint of the 3D display.
                MIL.M3ddispSetView(MilDisplay, MIL.M_AUTO, MIL.M_BOTTOM_VIEW, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
                Console.WriteLine("Press <R> on the display window to stop/start the rotation.");
                Console.WriteLine();
                }

            return MilDisplay;
        }

        // --------------------------------------------------------------
        // Display the received container.
        // --------------------------------------------------------------
        private static void DisplayContainer(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilContainer, ref MIL_ID MilDepthMap, ref MIL_ID MilIntensityMap)
        {
            long DisplayType = 0;
            MIL.MobjInquire(MilDisplay, MIL.M_OBJECT_TYPE, ref DisplayType);

            bool Use3D = (DisplayType == MIL.M_3D_DISPLAY);

            if (Use3D)
            {
                MIL.M3ddispSelect(MilDisplay, MilContainer, MIL.M_ADD, MIL.M_DEFAULT);
                MIL.M3ddispSelect(MilDisplay, MIL.M_NULL, MIL.M_OPEN, MIL.M_DEFAULT);
            }
            else
            {
                if (MilDepthMap == MIL.M_NULL)
                {
                    MIL_INT SizeX = 0;         // Image size X for 2d display
                    MIL_INT SizeY = 0;         // Image size Y for 2d display

                    MIL.M3dimCalculateMapSize(MIL.M_DEFAULT, MilContainer, MIL.M_NULL, MIL.M_DEFAULT, ref SizeX, ref SizeY);

                    MilIntensityMap = MIL.MbufAllocColor(MilSystem, 3, SizeX, SizeY, MIL.M_UNSIGNED + 8, MIL.M_IMAGE | MIL.M_PROC | MIL.M_DISP, MIL.M_NULL);
                    MilDepthMap = MIL.MbufAlloc2d(MilSystem, SizeX, SizeY, MIL.M_UNSIGNED + 8, MIL.M_IMAGE | MIL.M_PROC | MIL.M_DISP, MIL.M_NULL);

                    MIL.M3dimCalibrateDepthMap(MilContainer, MilDepthMap, MilIntensityMap, MIL.M_NULL, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_CENTER);
                }

                // Rotate the point cloud container to be in the xy plane before projecting.
                MIL_ID RotatedContainer = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC, MIL.M_DEFAULT, MIL.M_NULL);

                MIL.M3dimRotate(MilContainer, RotatedContainer, MIL.M_ROTATION_XYZ, 90, 60, 0, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
                MIL.M3dimProject(MilContainer, MilDepthMap, MilIntensityMap, MIL.M_DEFAULT, MIL.M_MIN_Z, MIL.M_DEFAULT, MIL.M_DEFAULT);

                // Display the projected point cloud container.
                MIL.MdispSelect(MilDisplay, MilIntensityMap);
                MIL.MbufFree(RotatedContainer);
            }
        }

        // --------------------------------------------------------------
        // Updated the displayed image.
        // --------------------------------------------------------------
        private static void UpdateDisplay(MIL_ID MilSystem, MIL_ID MilContainer, MIL_ID MilDepthMap, MIL_ID MilIntensityMap)
        {
            if (MilDepthMap == MIL.M_NULL)
            {
                return;
            }

            MIL_ID RotatedContainer = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC, MIL.M_DEFAULT, MIL.M_NULL);

            MIL.M3dimRotate(MilContainer, RotatedContainer, MIL.M_ROTATION_XYZ, 90, 70, 0, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
            MIL.M3dimProject(MilContainer, MilDepthMap, MilIntensityMap, MIL.M_DEFAULT, MIL.M_MIN_Z, MIL.M_DEFAULT, MIL.M_DEFAULT);

            MIL.MbufFree(RotatedContainer);
        }

        // --------------------------------------------------------------
        // Free the display.
        // --------------------------------------------------------------
        private static void FreeDisplay(MIL_ID MilDisplay)
        {
            long DisplayType = 0;
            MIL.MobjInquire(MilDisplay, MIL.M_OBJECT_TYPE, ref DisplayType);

            if (DisplayType == MIL.M_DISPLAY)
            {
                MIL.MdispFree(MilDisplay);
            }
            else
            {
                MIL.M3ddispFree(MilDisplay);
            }
        }
    }
}
