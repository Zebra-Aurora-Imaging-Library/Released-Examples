/******************************************************************************/
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

#include "CDisplay.h"
/* -------------------------------------------------------------- */
/* Example description.                                           */
/* -------------------------------------------------------------- */
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("M3dmod\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to use the 3D model finder module \n"));
   MosPrintf(MIL_TEXT("to define surface models and search for them in 3D scenes.\n"));
   MosPrintf(MIL_TEXT("\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: 3D Model Finder, 3D Display, 3D Graphics, and 3D Image\n"
                      "Processing.\n\n"));
   }

/* Input scanned point cloud files. */
static const MIL_STRING SINGLE_MODEL   = M_IMAGE_PATH MIL_TEXT("SimpleModel.mbufc");
static const MIL_STRING SINGLE_SCENE   = M_IMAGE_PATH MIL_TEXT("SimpleScene.mbufc");
static const MIL_STRING COMPLEX_MODEL1 = M_IMAGE_PATH MIL_TEXT("ComplexModel1.ply");
static const MIL_STRING COMPLEX_MODEL2 = M_IMAGE_PATH MIL_TEXT("ComplexModel2.ply");
static const MIL_STRING COMPLEX_SCENE  = M_IMAGE_PATH MIL_TEXT("ComplexScene.ply");

/* Constants. */
static const MIL_INT    DISP_SIZE_X = 480;
static const MIL_INT    DISP_SIZE_Y = 420;

/* functions. */
void SimpleSceneSurfaceFinder (MIL_ID    MilSystem,
                               CDisplay& DisplayModel,
                               CDisplay& DisplayScene);
void ComplexSceneSurfaceFinder(MIL_ID    MilSystem,
                               CDisplay& DisplayModel,
                               CDisplay& DisplayScene);
void AddComponentNormalsIfMissing(MIL_ID MilContainer);
void ShowResults(MIL_ID MilResult, MIL_DOUBLE ComputationTime);
/* -------------------------------------------------------------- */

int MosMain()
   {
   /* Print example information in console. */
   PrintHeader();

   MIL_UNIQUE_APP_ID    MilApplication;    /* MIL application identifier  */
   MIL_UNIQUE_SYS_ID    MilSystem;         /* MIL system identifier       */

   /* Allocate MIL objects. */
   MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MilSystem      = MsysAlloc(M_DEFAULT, M_SYSTEM_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   /* Allocate the display. */ 
   CDisplay DisplayModel(MilSystem);
   DisplayModel.Alloc3dDisplayId();
   DisplayModel.Size(DISP_SIZE_X/2, DISP_SIZE_Y/2);
   DisplayModel.Title(MIL_TEXT("Model Cloud"));

   CDisplay DisplayScene(MilSystem);
   DisplayScene.Alloc3dDisplayId();
   DisplayScene.Size(DISP_SIZE_X, DISP_SIZE_Y);
   DisplayScene.PositionX((MIL_INT)(1.04 * 0.5*DISP_SIZE_X));
   DisplayScene.Title(MIL_TEXT("Scene Cloud"));

   SimpleSceneSurfaceFinder(MilSystem, DisplayModel, DisplayScene);

   ComplexSceneSurfaceFinder(MilSystem, DisplayModel, DisplayScene);

   DisplayModel.FreeDisplay();
   DisplayScene.FreeDisplay();
   return 0;
   }
/* -------------------------------------------------------------- */
/* Simple scene with a single occurrence.                         */
/* -------------------------------------------------------------- */
void SimpleSceneSurfaceFinder(MIL_ID MilSystem, CDisplay& DisplayModel, CDisplay& DisplayScene)
   {
   /* Allocate a surface Model Finder context. */
   auto MilContext = M3dmodAlloc(MilSystem, M_FIND_SURFACE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   /* Allocate a surface Model Finder result. */
   auto MilResult = M3dmodAllocResult(MilSystem, M_FIND_SURFACE_RESULT,
                                      M_DEFAULT, M_UNIQUE_ID);

   /* Restore the model container and display it */
   auto MilModelContainer = MbufRestore(SINGLE_MODEL, MilSystem, M_UNIQUE_ID);
   DisplayModel.SetView(M_AZIM_ELEV_ROLL, 45, -35, 180);
   DisplayModel.DisplayContainer(MilModelContainer, true);
   MosPrintf(MIL_TEXT("The 3D point cloud of the model is restored from a file and"
                      " displayed.\n"));

   /* Load the single model scene point cloud. */
   auto MilSceneContainer = MbufRestore(SINGLE_SCENE, MilSystem, M_UNIQUE_ID);

   DisplayScene.SetView(M_AZIM_ELEV_ROLL, 202, -20.0, 182.0);
   DisplayScene.DisplayContainer(MilSceneContainer, true);

   MosPrintf(MIL_TEXT("The 3D point cloud of the scene is restored from a file and"
                      " displayed."));
   MosPrintf(MIL_TEXT("\n\nPress <Enter> to start.\n\n"));
   MosGetch();

  
   /* Define the surface model. */
   M3dmodDefine(MilContext, M_ADD_FROM_POINT_CLOUD, M_SURFACE, (MIL_DOUBLE)MilModelContainer,
                M_DEFAULT, M_DEFAULT,M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MosPrintf(MIL_TEXT("Define the model using the given model point cloud.\n\n"));

   /* Set the search perseverance. */
   MosPrintf(MIL_TEXT("Set the lowest perseverance to increase the search speed for a simple"
                      " scene.\n\n"));
   M3dmodControl(MilContext, M_DEFAULT, M_PERSEVERANCE, 0.0);

   MosPrintf(MIL_TEXT("Set the scene complexity to low to increase the search speed for a "
                      "simple scene.\n\n"));
   M3dmodControl(MilContext, M_DEFAULT, M_SCENE_COMPLEXITY, M_LOW);
  
   /* Preprocess the search context. */
   M3dmodPreprocess(MilContext, M_DEFAULT);

   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if not"
                      " present.\n\n"));
   /* The surface finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud. */
   AddComponentNormalsIfMissing(MilSceneContainer);

   MosPrintf(MIL_TEXT("3D surface finder is running..\n\n"));

   /* Reset the timer. */
   MIL_DOUBLE ComputationTime = 0.0;
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the model. */
   M3dmodFind(MilContext, MilSceneContainer, MilResult, M_DEFAULT);

   /* Read the find time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &ComputationTime);

   ShowResults(MilResult, ComputationTime);
   DisplayScene.Draw(MilResult);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }
/* -------------------------------------------------------------- */
/* Complex scene with multiple occurrences.                       */
/* -------------------------------------------------------------- */
void ComplexSceneSurfaceFinder(MIL_ID    MilSystem,
                               CDisplay& DisplayModel,
                               CDisplay& DisplayScene)
   {
   /* Allocate a surface 3D Model Finder context. */
   auto MilContext = M3dmodAlloc(MilSystem, M_FIND_SURFACE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   /* Allocate a surface 3D Model Finder result. */
   auto MilResult = M3dmodAllocResult(MilSystem, M_FIND_SURFACE_RESULT,
                                      M_DEFAULT, M_UNIQUE_ID);

   DisplayModel.Clear(M_ALL);
   DisplayScene.Clear(M_ALL);

   /* Restore the first model container and display it. */
   auto MilModelContainer = MbufRestore(COMPLEX_MODEL1, MilSystem, M_UNIQUE_ID);
   DisplayModel.SetView(M_AZIM_ELEV_ROLL, 290, -67, 265);
   DisplayModel.DisplayContainer(MilModelContainer, false);
   DisplayModel.SetView(M_AUTO, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MosPrintf(MIL_TEXT("The 3D point cloud of the first model is restored from a file and"
                      " displayed.\n"));

   /* Load the complex scene point cloud. */
   auto MilSceneContainer = MbufRestore(COMPLEX_SCENE, MilSystem, M_UNIQUE_ID);

   DisplayScene.SetView(M_AZIM_ELEV_ROLL, 260, -72, 142);
   DisplayScene.DisplayContainer(MilSceneContainer, false);
   DisplayScene.SetView(M_AUTO, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   DisplayScene.SetView(M_ZOOM, 1.2, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The 3D point cloud of the scene is restored from a file and"
                      " displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if"
                      " not present.\n\n"));
   /* The surface finder requires the existence of M_COMPONENT_NORMALS_MIL*/
   /* in the point cloud.* /
   AddComponentNormalsIfMissing(MilSceneContainer);
                                                                              
   /* Define the surface model. */
   M3dmodDefine(MilContext, M_ADD_FROM_POINT_CLOUD, M_SURFACE, (MIL_DOUBLE)MilModelContainer,
                M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Find all ocurrences. */ 
   M3dmodControl(MilContext, 0, M_NUMBER, M_ALL);
   M3dmodControl(MilContext, 0, M_COVERAGE_MAX, 75);

   M3dmodPreprocess(MilContext, M_DEFAULT);
   MosPrintf(MIL_TEXT("3D surface finder is running..\n\n"));

   /* Reset the timer. */
   MIL_DOUBLE ComputationTime = 0.0;
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the model. */
   M3dmodFind(MilContext, MilSceneContainer, MilResult, M_DEFAULT);

   /* Read the find time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &ComputationTime);

   ShowResults(MilResult, ComputationTime);
   MIL_INT64 Label = DisplayScene.Draw(MilResult);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   DisplayScene.Clear(Label);
  
   MilModelContainer = MbufRestore(COMPLEX_MODEL2, MilSystem, M_UNIQUE_ID);
   DisplayModel.DisplayContainer(MilModelContainer, false);
   DisplayModel.SetView(M_AUTO, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The 3D point cloud of the second model is restored from file and"
                      " displayed.\n\n"));

   /* Delete the previous model. */
   M3dmodDefine(MilContext, M_DELETE, M_DEFAULT, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT,
                M_DEFAULT, M_DEFAULT);

   /* Define the surface model. */
   M3dmodDefine(MilContext, M_ADD_FROM_POINT_CLOUD, M_SURFACE, (MIL_DOUBLE)MilModelContainer,
                M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Find all ocurrences. */ 
   M3dmodControl(MilContext, 0, M_NUMBER, M_ALL);
   M3dmodControl(MilContext, 0, M_COVERAGE_MAX, 95);

   M3dmodPreprocess(MilContext, M_DEFAULT);
   MosPrintf(MIL_TEXT("3D surface finder is running..\n\n"));

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the model. */
   M3dmodFind(MilContext, MilSceneContainer, MilResult, M_DEFAULT);

   /* Read the find time. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &ComputationTime);

   ShowResults(MilResult, ComputationTime);
   DisplayScene.Draw(MilResult);

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();
   }
/* -------------------------------------------------------------- */
/* Adds the component M_COMPONENT_NORMALS_MIL if it's not found.  */
/* -------------------------------------------------------------- */
void AddComponentNormalsIfMissing(MIL_ID MilContainer)
   {
   MIL_ID MilNormals =
      MbufInquireContainer(MilContainer, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL);

   if(MilNormals != M_NULL)
      return;
   MIL_INT SizeX = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   if(SizeX < 50 || SizeY < 50)
      M3dimNormals(M_NORMALS_CONTEXT_TREE, MilContainer, MilContainer, M_DEFAULT);
   else
      M3dimNormals(M_NORMALS_CONTEXT_ORGANIZED, MilContainer, MilContainer, M_DEFAULT);
   }
/* --------------------------------------------------------- */
/* Shows the surface finder results.                         */
/* --------------------------------------------------------- */
void ShowResults(MIL_ID MilResult, MIL_DOUBLE ComputationTime)
   {
   MIL_INT Status;
   M3dmodGetResult(MilResult, M_DEFAULT, M_STATUS, &Status);

   if(Status != M_COMPLETE)
      {
      MosPrintf(MIL_TEXT("The find process is not completed.\n"));
      }

   MIL_INT NbOcc = 0;
   M3dmodGetResult(MilResult, M_DEFAULT, M_NUMBER, &NbOcc);
   MosPrintf(MIL_TEXT("Found %i occurrence(s) in %.2f s.\n\n"), NbOcc, ComputationTime);

   if(NbOcc == 0)
      return;

   MosPrintf(MIL_TEXT("Index        Score        Score_Target\n"));
   MosPrintf(MIL_TEXT("------------------------------------------------------\n"));

   for(MIL_INT i = 0; i < NbOcc; ++i)
      {
      MIL_DOUBLE ScoreTarget = M3dmodGetResult(MilResult, i, M_SCORE_TARGET, M_NULL);
      MIL_DOUBLE Score       = M3dmodGetResult(MilResult, i, M_SCORE       , M_NULL);
     

      MosPrintf(MIL_TEXT("  %i          %.4f      %6.2f          \n"),
                i, Score, ScoreTarget);
      }
   MosPrintf(MIL_TEXT("\n"));
   }

