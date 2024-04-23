/*******************************************************************************/
/* 
* File name: M3dmod.cpp
*
* Synopsis: This example demonstrates how to use the 3D model finder module
*           to define surface models and search for them in 3D scenes.
*           A simple single model search is presented first followed by a more
*           complex example of multiple occurrences in a complex scene.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
/*******************************************************************************/

using System;
using Matrox.MatroxImagingLibrary;

namespace M3dmod
{
    class Program
    {
        /* -------------------------------------------------------------- */
        /* Example description.                                           */
        /* -------------------------------------------------------------- */
        private static void PrintHeader()
        {
            Console.WriteLine("[EXAMPLE NAME]");
            Console.WriteLine("M3dmod");
            Console.WriteLine();
            Console.WriteLine("[SYNOPSIS]");
            Console.WriteLine("This example demonstrates how to use the 3D model finder module");
            Console.WriteLine("to define surface models and search for them in 3D scenes.");
            Console.WriteLine();

            Console.WriteLine("[MODULES USED]");
            Console.WriteLine("Modules used: 3D Model Finder, 3D Display, 3D Graphics, and 3D"+
                              " Image\nProcessing.");
            Console.WriteLine();
        }

        /* Input scanned point cloud files. */
        const string SINGLE_MODEL   = MIL.M_IMAGE_PATH + "SimpleModel.mbufc";
        const string SINGLE_SCENE   = MIL.M_IMAGE_PATH + "SimpleScene.mbufc";
        const string COMPLEX_MODEL1 = MIL.M_IMAGE_PATH + "ComplexModel1.ply";
        const string COMPLEX_MODEL2 = MIL.M_IMAGE_PATH + "ComplexModel2.ply";
        const string COMPLEX_SCENE  = MIL.M_IMAGE_PATH + "ComplexScene.ply";

        /* Constants. */
        private static readonly MIL_INT    DISP_SIZE_X = 480;
        private static readonly MIL_INT    DISP_SIZE_Y = 420;

        // --------------------------------------------------------------
        private static void Main(string[] args)
        {
            /* Print example information in console. */
            PrintHeader();

            MIL_ID MilApplication = MIL.M_NULL;    /* MIL application identifier  */
            MIL_ID MilSystem      = MIL.M_NULL;    /* MIL system identifier       */

            /* Allocate MIL objects. */
            MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT, ref MilApplication);
            MIL.MsysAlloc(MIL.M_DEFAULT, MIL.M_SYSTEM_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                          ref MilSystem);

            /* Allocate the display. */
            CDisplay DisplayModel = new CDisplay(MilSystem);
            DisplayModel.Alloc3dDisplayId();
            DisplayModel.Size(DISP_SIZE_X / 2, DISP_SIZE_Y / 2);
            DisplayModel.Title("Model Cloud");

            CDisplay DisplayScene = new CDisplay(MilSystem);
            DisplayScene.Alloc3dDisplayId();
            DisplayScene.Size(DISP_SIZE_X, DISP_SIZE_Y);
            DisplayScene.PositionX((MIL_INT)(1.04 * 0.5 * DISP_SIZE_X));
            DisplayScene.Title("Scene Cloud");

            SimpleSceneSurfaceFinder(MilSystem, ref DisplayModel,ref DisplayScene);

            ComplexSceneSurfaceFinder(MilSystem, ref DisplayModel, ref DisplayScene);

            /* Free mil objects. */
            DisplayModel.FreeDisplay();
            DisplayScene.FreeDisplay();
            MIL.MsysFree(MilSystem);
            MIL.MappFree(MilApplication);
            }
        /* -------------------------------------------------------------- */
        /* Simple scene with a single occurrence                          */
        /* -------------------------------------------------------------- */
        static void SimpleSceneSurfaceFinder(MIL_ID       MilSystem   ,
                                             ref CDisplay DisplayModel,
                                             ref CDisplay DisplayScene)
            {
            /* Allocate a surface Model Finder context. */
            MIL_ID MilContext = MIL.M3dmodAlloc(MilSystem, MIL.M_FIND_SURFACE_CONTEXT,
                                                MIL.M_DEFAULT, MIL.M_NULL);

            /* Allocate a surface Model Finder result. */
            MIL_ID MilResult = MIL.M3dmodAllocResult(MilSystem, MIL.M_FIND_SURFACE_RESULT,
                                                     MIL.M_DEFAULT, MIL.M_NULL);

            /* Restore the model container and display it */
            MIL_ID MilModelContainer = MIL.MbufRestore(SINGLE_MODEL, MilSystem, MIL.M_NULL);
            DisplayModel.SetView(MIL.M_AZIM_ELEV_ROLL, 45, -35, 180);
            DisplayModel.DisplayContainer(MilModelContainer, true);
            Console.WriteLine("The 3D point cloud of the model is restored from a file and "+
                               "displayed.");

            /* Load the single model scene point cloud. */
            MIL_ID MilSceneContainer = MIL.MbufRestore(SINGLE_SCENE, MilSystem, MIL.M_NULL);

            DisplayScene.SetView(MIL.M_AZIM_ELEV_ROLL, 202, -20.0, 182.0);
            DisplayScene.DisplayContainer(MilSceneContainer, true);

            Console.WriteLine("The 3D point cloud of the scene is restored from a file and"+
                              " displayed.\n");
            Console.WriteLine("Press <Enter> to start.\n");
            Console.ReadKey();


            /* Define the surface model. */
            MIL.M3dmodDefine(MilContext, MIL.M_ADD_FROM_POINT_CLOUD, MIL.M_SURFACE,
                             MilModelContainer, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                             MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
            Console.WriteLine("Define the model using the given model point cloud.\n");

            /* Set the search perseverance. */
            Console.WriteLine("Set the lowest perseverance to increase the search speed for" +
                              " a simple scene.\n");
            MIL.M3dmodControl(MilContext, MIL.M_DEFAULT, MIL.M_PERSEVERANCE, 0.0);

            Console.WriteLine("Set the scene complexity to low to increase the search speed " +
                              "for a simple scene.\n");
            MIL.M3dmodControl(MilContext, MIL.M_DEFAULT, MIL.M_SCENE_COMPLEXITY, MIL.M_LOW);

            /* Preprocess the search context. */
            MIL.M3dmodPreprocess(MilContext, MIL.M_DEFAULT);

            Console.WriteLine("M_COMPONENT_NORMALS_MIL is added to the point cloud if not " +
                              "present.\n");
            /* The surface finder requires the existence of M_COMPONENT_NORMALS_MIL in the */
            /* point cloud. */
            AddComponentNormalsIfMissing(MilSceneContainer);

            Console.WriteLine("3D surface finder is running..\n");

            /* Reset the timer. */
            double ComputationTime = 0.0;
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL);

            /* Find the model. */
            MIL.M3dmodFind(MilContext, MilSceneContainer, MilResult, MIL.M_DEFAULT);

            /* Read the find time. */
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS,
                          ref ComputationTime);

            ShowResults(MilResult, ComputationTime);
            DisplayScene.Draw(MilResult);

            Console.WriteLine("Press <Enter> to continue.\n");
            Console.ReadKey();

            /* Free mil objects*/
            MIL.M3dmodFree(MilContext);
            MIL.M3dmodFree(MilResult);
            MIL.MbufFree(MilModelContainer);
            MIL.MbufFree(MilSceneContainer);
            }
        /* -------------------------------------------------------------- */
        /* Complex scene with multiple occurrences                        */
        /* -------------------------------------------------------------- */
        static void ComplexSceneSurfaceFinder(MIL_ID       MilSystem   ,
                                              ref CDisplay DisplayModel,
                                              ref CDisplay DisplayScene)
            {
            /* Allocate a surface 3D Model Finder context. */
            MIL_ID MilContext = MIL.M3dmodAlloc(MilSystem, MIL.M_FIND_SURFACE_CONTEXT,
                                                MIL.M_DEFAULT, MIL.M_NULL);

            /* Allocate a surface 3D Model Finder result. */
            MIL_ID MilResult = MIL.M3dmodAllocResult(MilSystem, MIL.M_FIND_SURFACE_RESULT,
                                                     MIL.M_DEFAULT, MIL.M_NULL);

            DisplayModel.Clear(MIL.M_ALL);
            DisplayScene.Clear(MIL.M_ALL);

            /* Restore the first model container and display it. */
            MIL_ID MilModelContainer = MIL.MbufRestore(COMPLEX_MODEL1, MilSystem,
                                                       MIL.M_NULL);
            DisplayModel.SetView(MIL.M_AZIM_ELEV_ROLL, 290, -67, 265);
            DisplayModel.DisplayContainer(MilModelContainer, false);
            DisplayModel.SetView(MIL.M_AUTO, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
            Console.WriteLine("The 3D point cloud of the first model is restored from a file" +
                " and displayed.");

            /* Load the complex scene point cloud. */
            MIL_ID MilSceneContainer = MIL.MbufRestore(COMPLEX_SCENE, MilSystem, MIL.M_NULL);

      
            DisplayScene.SetView(MIL.M_AZIM_ELEV_ROLL, 260, -72, 142);
            DisplayScene.DisplayContainer(MilSceneContainer, false);
            DisplayScene.SetView(MIL.M_AUTO, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
            DisplayScene.SetView(MIL.M_ZOOM, 1.2, MIL.M_DEFAULT, MIL.M_DEFAULT);

            Console.WriteLine("The 3D point cloud of the scene is restored from a file and" +
                              " displayed.\n");
            Console.WriteLine("Press <Enter> to start.\n");
            Console.ReadKey();

            Console.WriteLine("M_COMPONENT_NORMALS_MIL is added to the point cloud if not " +
                              "present.\n");
            /* The surface finder requires the existence of M_COMPONENT_NORMALS_MIL in the */
            /* point cloud. */
            AddComponentNormalsIfMissing(MilSceneContainer);

            /* Define the surface model. */
            MIL.M3dmodDefine(MilContext, MIL.M_ADD_FROM_POINT_CLOUD, MIL.M_SURFACE,
                             MilModelContainer, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                             MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            /* Find all ocurrences. */
            MIL.M3dmodControl(MilContext, 0, MIL.M_NUMBER, MIL.M_ALL);
            MIL.M3dmodControl(MilContext, 0, MIL.M_COVERAGE_MAX, 75);

            MIL.M3dmodPreprocess(MilContext, MIL.M_DEFAULT);
            Console.WriteLine("3D surface finder is running..\n");

            /* Reset the timer. */
            double ComputationTime = 0.0;
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL);

            /* Find the model. */
            MIL.M3dmodFind(MilContext, MilSceneContainer, MilResult, MIL.M_DEFAULT);

            /* Read the find time. */
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS,
                          ref ComputationTime);

            ShowResults(MilResult, ComputationTime);
            MIL_INT Label = DisplayScene.Draw(MilResult);

            Console.WriteLine("Press <Enter> to continue.\n");
            Console.ReadKey();

            DisplayScene.Clear(Label);

            MIL.MbufFree(MilModelContainer);
            MilModelContainer = MIL.MbufRestore(COMPLEX_MODEL2, MilSystem, MIL.M_NULL);
            DisplayModel.DisplayContainer(MilModelContainer, false);
            DisplayModel.SetView(MIL.M_AUTO, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            Console.WriteLine("The 3D point cloud of the second model is restored from file" +
                              " and displayed.\n");

            /* Delete the previous model. */
            MIL.M3dmodDefine(MilContext, MIL.M_DELETE, MIL.M_DEFAULT, 0, MIL.M_DEFAULT,
                             MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT,
                             MIL.M_DEFAULT);

            /* Define the surface model. */
            MIL.M3dmodDefine(MilContext, MIL.M_ADD_FROM_POINT_CLOUD, MIL.M_SURFACE,
                             MilModelContainer, MIL.M_DEFAULT, MIL.M_DEFAULT,
                             MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            /* Find all ocurrences. */
            MIL.M3dmodControl(MilContext, 0, MIL.M_NUMBER, MIL.M_ALL);
            MIL.M3dmodControl(MilContext, 0, MIL.M_COVERAGE_MAX, 95);

            MIL.M3dmodPreprocess(MilContext, MIL.M_DEFAULT);
            Console.WriteLine("3D surface finder is running..\n");

            /* Reset the timer. */
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL);

            /* Find the model. */
            MIL.M3dmodFind(MilContext, MilSceneContainer, MilResult, MIL.M_DEFAULT);

            /* Read the find time. */
            MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS,
                          ref ComputationTime);

            ShowResults(MilResult, ComputationTime);
            DisplayScene.Draw(MilResult);

            Console.WriteLine("Press <Enter> to end.\n");
            Console.ReadKey();

            /* Free mil objects*/
            MIL.M3dmodFree(MilContext);
            MIL.M3dmodFree(MilResult);
            MIL.MbufFree(MilModelContainer);
            MIL.MbufFree(MilSceneContainer);
            }
        /* -------------------------------------------------------------- */
        /* Adds the component M_COMPONENT_NORMALS_MIL if it's not found.  */
        /* -------------------------------------------------------------- */
        static void AddComponentNormalsIfMissing(MIL_ID MilContainer)
            {
            MIL_ID MilNormals =
            (MIL_ID)MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_NORMALS_MIL,
                                             MIL.M_COMPONENT_ID, MIL.M_NULL);

            if (MilNormals != MIL.M_NULL)
                return;
            MIL_INT SizeX = MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_RANGE,
                                                     MIL.M_SIZE_X, MIL.M_NULL);
            MIL_INT SizeY = MIL.MbufInquireContainer(MilContainer, MIL.M_COMPONENT_RANGE,
                                                     MIL.M_SIZE_Y, MIL.M_NULL);
            if (SizeX < 50 || SizeY < 50)
                MIL.M3dimNormals(MIL.M_NORMALS_CONTEXT_TREE, MilContainer, MilContainer,
                                 MIL.M_DEFAULT);
            else
                MIL.M3dimNormals(MIL.M_NORMALS_CONTEXT_ORGANIZED, MilContainer, MilContainer,
                                 MIL.M_DEFAULT);
            }
        /* --------------------------------------------------------- */
        /* Shows the surface finder results.                         */
        /* --------------------------------------------------------- */
        static void ShowResults(MIL_ID MilResult, double ComputationTime)
            {
            MIL_INT Status = 0 ;
            MIL.M3dmodGetResult(MilResult, MIL.M_DEFAULT, MIL.M_STATUS,  ref Status);
            if (Status != MIL.M_COMPLETE)
                {
                Console.WriteLine("The find process is not completed.");
                }

            MIL_INT NbOcc = 0;
            MIL.M3dmodGetResult(MilResult, MIL.M_DEFAULT, MIL.M_NUMBER,  ref NbOcc);
            Console.WriteLine("Found {0} occurrence(s) in "+String.Format("{0:F2}",
                              ComputationTime)+" s.\n", NbOcc);

            if (NbOcc == 0)
                return;

            Console.WriteLine("Index        Score        Score_Target");
            Console.WriteLine("------------------------------------------------------");

            double ScoreTarget = 0.0;
            double Score       = 0.0;
            for (MIL_INT i = 0; i < NbOcc; ++i)
                {
                MIL.M3dmodGetResult(MilResult, i, MIL.M_SCORE_TARGET, ref ScoreTarget);
                MIL.M3dmodGetResult(MilResult, i, MIL.M_SCORE       , ref Score);


                Console.WriteLine("  {0}          "+ String.Format("{0:F4}", Score) + "       "
                                  + String.Format("{0:F2}", ScoreTarget) +"          ", i);
                }
            Console.WriteLine();
            }
    }
}
