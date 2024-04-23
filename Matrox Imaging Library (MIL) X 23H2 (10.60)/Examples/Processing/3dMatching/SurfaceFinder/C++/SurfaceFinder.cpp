//***************************************************************************************/
// 
// File name: SurfaceFinder.cpp  
//
// Synopsis: This example demonstrates how to use surface 3D Model Finder to define point cloud models
//           and search for them in 3D point clouds using advanced search controls and
//           challenging scenes.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*************************************************************************************/
#include <mil.h>
#include "SurfaceFinder.h"

/************************************************************************************/
/* Constants.                                                                       */
/************************************************************************************/
static const MIL_STRING REFINE_MODEL       = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//RefineModel.ply        ");
static const MIL_STRING REFINE_SCENE       = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//RefineScene.ply        ");
static const MIL_STRING BACKGROUND_MODEL   = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//ModelBackground.ply    ");
static const MIL_STRING BACKGROUND_SCENE   = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//SceneWithBackground.ply");
static const MIL_STRING RESOLUTION_MODEL   = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//CADModel.ply           ");
static const MIL_STRING RESOLUTION_SCENE   = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//Scene.ply              ");
static const MIL_STRING CONSTRAINED_MODEL  = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//RefineModel.ply        ");
static const MIL_STRING CONSTRAINED_SCENE1 = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//ConstrainedScene1.ply  ");
static const MIL_STRING CONSTRAINED_SCENE2 = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//ConstrainedScene2.ply  ");
static const MIL_STRING SORTED_MODEL       = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//SortedModel.ply        ");
static const MIL_STRING SORTED_SCENE       = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//SortedScene.ply        ");
static const MIL_STRING COMPLEX_MODEL      = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//ModelBackground.ply    ");
static const MIL_STRING COMPLEX_SCENE      = M_IMAGE_PATH MIL_TEXT("SurfaceFinder//ComplexScene.ply       ");
static const MIL_INT    DISP_SIZE_X        = 480;
static const MIL_INT    DISP_SIZE_Y        = 420;

//*******************************************************************/
/* Function declarations.                                           */
//*******************************************************************/
void RefineRegistrationFinder(MIL_ID MilSystem, CSurfaceFinder& Finder);
void BackgroundRemovedlFinder(MIL_ID MilSystem, CSurfaceFinder& Finder);
void SceneComplexityFinder   (MIL_ID MilSystem, CSurfaceFinder& Finder);
void CADFinder               (MIL_ID MilSystem, CSurfaceFinder& Finder);
void ConstrainedFinder       (MIL_ID MilSystem, CSurfaceFinder& Finder);
void SortedFinder            (MIL_ID MilSystem, CSurfaceFinder& Finder);

void AllocAndDefineContext(MIL_ID               MilSystem        ,
                           MIL_ID               MilModelContainer,
                           MIL_UNIQUE_3DMOD_ID& MilContext       );
bool CheckForRequiredMILFile(MIL_STRING FileName);
void AddComponentNormalsIfMissing(MIL_ID MilContainer);
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);

//*****************************************************/
/* Example description.                               */
//*****************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Surface Finder\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to use surface 3D Model Finder to define point\n")
             MIL_TEXT("cloud models and search for them in 3D point clouds using advanced search\n")
             MIL_TEXT("controls and challenging scenes.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, 3D Model Finder, \n")
             MIL_TEXT("3D Image Processing, 3D Display, and 3D Graphics. \n\n"));
   }

/****************************************************************************/
/* Main.                                                                    */
/****************************************************************************/
int MosMain()
   {
   /* Print Header. */
   PrintHeader();

   /* Allocate MIL objects.*/
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem      = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   /* Check for the required example files.*/
   if(!CheckForRequiredMILFile(REFINE_MODEL))
      {
      return -1;
      }

   CSurfaceFinder Finder(MilSystem);
   Finder.AllocateDisplays();
   Finder.AllocateResult();

   /* Shows the impact of refine registration. */
   RefineRegistrationFinder(MilSystem, Finder);

   /* Shows the impact of enabling background removal. */
   BackgroundRemovedlFinder(MilSystem, Finder);

   /* Shows how to constrain the resting plane of occurrences. */
   ConstrainedFinder(MilSystem, Finder);

   /* Shows the sampling compensation with a CAD model  */
   /* and the effect of enabling the occlusion handling.*/
   CADFinder(MilSystem, Finder);

   /* Shows the impact of changing the default sorting of results. */
   SortedFinder(MilSystem, Finder);

   /* Shows the impact of changing scene complexity. */
   SceneComplexityFinder(MilSystem, Finder);
   }

//*****************************************************************************/
/* Surface 3D Model Finder with and without refine registration.              */
//*****************************************************************************/
void RefineRegistrationFinder(MIL_ID MilSystem, CSurfaceFinder& Finder)
   {
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Surface 3D Model Finder with and without refine registration.           \n"));
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n\n"));

   /* Restore the model and scene containers and display them. */
   auto MilModelContainer = MbufRestore(REFINE_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(REFINE_SCENE, MilSystem, M_UNIQUE_ID);

   Finder.ShowContainers(MilModelContainer,  MilSceneContainer, M_BOTTOM_VIEW);

   MIL_UNIQUE_3DMOD_ID MilContext;
   AllocAndDefineContext(MilSystem, MilModelContainer,MilContext);
  
   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n\n"));

   /* The surface 3D Model Finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud. */
   AddComponentNormalsIfMissing(MilSceneContainer);

   MosPrintf(MIL_TEXT("Find without the refine registration.\n\n"));

   M3dmodControl(MilContext, 0, M_NUMBER, M_ALL);

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find without the refine registration. */
   Finder.Find(MilContext, MilSceneContainer);

   /* Show the find results. */
   auto Label = Finder.ShowResults();
   
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Remove the old drawing.*/
   Finder.ClearScene(Label);

   /* Enable the fast refine registration. */
   M3dmodControl(MilContext, M_DEFAULT, M_REFINE_REGISTRATION, M_FIND_SURFACE_REFINEMENT_FAST);   
   MosPrintf(MIL_TEXT("Find with the refine registration.\n\n"));

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find with the refine registration. */
   Finder.Find(MilContext, MilSceneContainer);

   /* Show the find results. */
   Finder.ShowResults();
 
   MosPrintf(MIL_TEXT("\nPress <Enter> for the next example.\n\n"));
   MosGetch();
   }

/******************************************************************************/
/* Surface 3D Model Finder with background in scene.                          */
/******************************************************************************/
void  BackgroundRemovedlFinder(MIL_ID MilSystem, CSurfaceFinder& Finder)
   {
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Surface 3D Model Finder with background removed.                        \n"));
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n\n"));

   MosPrintf(MIL_TEXT("The 3D point clouds are restored from files and displayed.\n\n"));

   /* Restore the point clouds. */
   auto MilModelContainer = MbufRestore(BACKGROUND_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(BACKGROUND_SCENE, MilSystem, M_UNIQUE_ID);

   /* Display the point clouds. */
   Finder.ShowContainers(MilModelContainer, MilSceneContainer, M_BOTTOM_VIEW);

   MIL_UNIQUE_3DMOD_ID MilContext;
   AllocAndDefineContext(MilSystem, MilModelContainer, MilContext);

   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n\n"));
   AddComponentNormalsIfMissing(MilSceneContainer);

   MosPrintf(MIL_TEXT("Enable the background removal in the scene.\n\n"));
   /* Enable the background removal. */ 
   M3dmodControl(MilContext, M_CONTEXT, M_REMOVE_BACKGROUND, M_ENABLE);

   /* Model may not be fully covered. */
   M3dmodControl(MilContext, 0, M_COVERAGE_MAX, 90);

   /* Multiple occurrences. */
   M3dmodControl(MilContext, 0, M_NUMBER, M_ALL);
   M3dmodControl(MilContext, 0, M_CERTAINTY, 80);

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find with background removal. */
   Finder.Find(MilContext, MilSceneContainer);

   MosPrintf(MIL_TEXT("The removed background points are shown in dark cyan.\n\n"));

   /* Show the find results. */
   Finder.ShowResults();

   /* Shows the removed background points in dark cyan. */
   auto MilDrawContext = M3dmodAlloc(MilSystem, M_DRAW_3D_SURFACE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dmodControlDraw(MilDrawContext, M_DRAW_BACKGROUND_POINTS, M_ACTIVE, M_ENABLE);
   M3dmodControlDraw(MilDrawContext, M_DRAW_BACKGROUND_POINTS, M_COLOR, M_COLOR_DARK_CYAN);
   Finder.DrawInScene(MilDrawContext);

   MosPrintf(MIL_TEXT("\nPress <Enter> for the next example.\n\n"));
   MosGetch();
   }

//*********************************************************************************/
/* Surface 3D Model Finder using a CAD model and varying scene complexity effects.*/
//*********************************************************************************/ 
void SceneComplexityFinder(MIL_ID MilSystem, CSurfaceFinder& Finder)
   {
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Surface 3D Model Finder with varying scene complexity.                  \n"));
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n\n"));

   /* Restore the model and scene point clouds. */
   auto MilModelContainer = MbufRestore(COMPLEX_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(COMPLEX_SCENE, MilSystem, M_UNIQUE_ID);

   /* Display the point clouds. */
   Finder.ShowContainers(MilModelContainer, MilSceneContainer, M_BOTTOM_VIEW);

   MIL_UNIQUE_3DMOD_ID MilContext;
   AllocAndDefineContext(MilSystem, MilModelContainer, MilContext);

   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n\n"));
   /* The 3D surface Model Finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud. */
   AddComponentNormalsIfMissing(MilSceneContainer);

   MosPrintf(MIL_TEXT("Lower scene complexity and/or lower perseverance increase the search speed.\n\n"));
   MosPrintf(MIL_TEXT("Higher scene complexity and/or higher perseverance increase the search\n")
             MIL_TEXT("capabilities.\n\n"));

   /* Multiple occurrences. */
   M3dmodControl(MilContext, 0, M_NUMBER, M_ALL);
   /* Model may not be fully covered. */
   M3dmodControl(MilContext, 0, M_COVERAGE_MAX, 85);

   /* Enable low scene complexity. */
   MosPrintf(MIL_TEXT("Set the scene complexity to low to increase the search speed.\n\n"));
   M3dmodControl(MilContext, M_DEFAULT, M_SCENE_COMPLEXITY, M_LOW);

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find with low scene complexity. */
   Finder.Find(MilContext, MilSceneContainer);

   /* Show the find results. */
   auto Label = Finder.ShowResults();

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Remove the old drawing.*/
   if(Label != 0)
      Finder.ClearScene(Label);

   MosPrintf(MIL_TEXT("Set the scene complexity to high to find more occurrences in a complex scene.\n\n"));
   M3dmodControl(MilContext, M_DEFAULT, M_SCENE_COMPLEXITY, M_HIGH);

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find with high scene complexity. */
   Finder.Find(MilContext, MilSceneContainer);

   /* Show the find results. */
   Finder.ShowResults();

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();
   }

//*****************************************************************************************/
/* Surface 3D Model Finder using a CAD model and occlusion handling.                      */
//*****************************************************************************************/
void CADFinder(MIL_ID MilSystem, CSurfaceFinder& Finder)
   {
   const MIL_INT INITIAL_COVERAGE = 35;
   const MIL_INT SCENE_PROJECTION_COVERAGE = 70;

   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Surface 3D Model Finder with a CAD model and occlusion handling.        \n"));
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n\n"));

   /* Restore the point clouds.*/
   auto MilModelContainer = MbufRestore(RESOLUTION_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(RESOLUTION_SCENE, MilSystem, M_UNIQUE_ID);

   /* Display the point clouds.*/
   Finder.ShowContainers(MilModelContainer, MilSceneContainer, M_BOTTOM_TILTED);

   MosPrintf(MIL_TEXT("3D point clouds are restored from files and displayed.\n\n"));

   MIL_UNIQUE_3DMOD_ID MilContext;
   AllocAndDefineContext(MilSystem, MilModelContainer, MilContext);

   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n\n"));
   /* The surface 3D Model Finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud. */
   AddComponentNormalsIfMissing(MilSceneContainer);

   /* Find the actual scene point resolution. */
   auto StatResult = M3dimAllocResult(M_DEFAULT_HOST, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_DISTANCE_TO_NEAREST_NEIGHBOR, MilSceneContainer, StatResult, M_DEFAULT);
   auto SceneResolution = M3dimGetResult(StatResult, M_DISTANCE_TO_NEAREST_NEIGHBOR_AVERAGE, M_NULL);

   MosPrintf(MIL_TEXT("The search point resolution is set to be similar to the scene resolution.\n\n"));
   /* Set the search point resolution to be similar to the scene resolution. */
   M3dmodControl(MilContext, 0, M_SEARCH_POINT_RESOLUTION, SceneResolution);

   /* The scanned object represents a small percentage of the full CAD model. */
   M3dmodControl(MilContext, 0, M_COVERAGE_MAX, INITIAL_COVERAGE);
   MosPrintf(MIL_TEXT("Coverage max is set to %d since the model is complete and the acquired scene\n")
             MIL_TEXT("occurrence is self-occluded.\n\n"), INITIAL_COVERAGE);

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find with a given search point resolution. */
   Finder.Find(MilContext, MilSceneContainer);

   /* Show the find results. */
   auto Label = Finder.ShowResults();

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   Finder.ClearScene(Label);
 
   MosPrintf(MIL_TEXT("\nSurface 3D Model Finder with occlusion handling.\n\n"));
 
   MosPrintf(MIL_TEXT("The scene projection is enabled with higher max coverage to find an occurrence\n")
             MIL_TEXT("for an unknown pose of the scan relative to the CAD.\n\n"));

   MosPrintf(MIL_TEXT("The coverage max is set to %d.\n\n"), SCENE_PROJECTION_COVERAGE);

   MosPrintf(MIL_TEXT("The projection plane depends on the direction of the z-axis in the scene scan.\n")
             MIL_TEXT("If the z-axis direction is up, set the projection plane as an xy plane\n")
             MIL_TEXT("with a positive z.\n")
             MIL_TEXT("If the z-axis direction is down, set the projection plane as an xy plane\n")
             MIL_TEXT("with a negative z.\n\n"));

   M3dmodControl(MilContext, 0, M_COVERAGE_MAX, SCENE_PROJECTION_COVERAGE);

   M3dmodControl(MilContext, M_CONTEXT, M_SCENE_PROJECTION, M_ENABLE);
   M3dmodControl(MilContext, M_CONTEXT, M_DIRECTION_REFERENCE_X, 0.0);
   M3dmodControl(MilContext, M_CONTEXT, M_DIRECTION_REFERENCE_Y, 0.0);

   bool ZDirectionUpwards = true;
   if(ZDirectionUpwards)
      {
      M3dmodControl(MilContext, M_CONTEXT, M_DIRECTION_REFERENCE_Z, 1.0);
      }
   else
      {
      M3dmodControl(MilContext, M_CONTEXT, M_DIRECTION_REFERENCE_Z, -1.0);
      }   

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find with the scene projection enabled. */
   Finder.Find(MilContext, MilSceneContainer);

   /* Show the find results. */
   Finder.ShowResults();

   MosPrintf(MIL_TEXT("\nPress <Enter> for the next example.\n\n"));
   MosGetch();
   }

//*****************************************************************************************/
/* Surface 3D Model Finder with a position constraint using a single resting plane.       */
//*****************************************************************************************/
void ConstrainedFinder(MIL_ID MilSystem, CSurfaceFinder& Finder)
   {
   const MIL_INT NB_OCCURENCES = 4;

   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Surface 3D Model Finder with resting plane constraint.                  \n"));
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n\n"));

   /* Restore the point clouds.*/
   auto MilModelContainer  = MbufRestore(CONSTRAINED_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer1 = MbufRestore(CONSTRAINED_SCENE1, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer2 = MbufRestore(CONSTRAINED_SCENE2, MilSystem, M_UNIQUE_ID);

   /* Display the point clouds. */
   Finder.ShowContainers(MilModelContainer, MilSceneContainer2, M_BOTTOM_TILTED);

   MosPrintf(MIL_TEXT("3D point clouds are restored from files and displayed.\n\n"));

   MIL_UNIQUE_3DMOD_ID MilContext;
   AllocAndDefineContext(MilSystem, MilModelContainer, MilContext);

   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n\n"));

   /* The surface 3D Model Finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud. */
   AddComponentNormalsIfMissing(MilSceneContainer1);
   AddComponentNormalsIfMissing(MilSceneContainer2);

   MosPrintf(MIL_TEXT("Find multiple occurrences without any constraints.\n\n"));

   /* Multiple occurrences. */
   M3dmodControl(MilContext, 0, M_NUMBER, NB_OCCURENCES);

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find without any constraints. */
   Finder.Find(MilContext, MilSceneContainer2);

   /* Show the find results. */
   auto Label = Finder.ShowResults();

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   Finder.ClearScene(Label);

   /* Display the point clouds.*/
   Finder.ShowContainers(MilModelContainer, MilSceneContainer1, M_BOTTOM_TILTED);

   /* First use a single occurrence scan to find the resting plane using M3dmet. */
   auto MilFitResult = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_UNIQUE_ID);
   auto MilPlane     = M3dgeoAlloc      (MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dmetFit(M_DEFAULT, MilSceneContainer1, M_PLANE, MilFitResult, M_AUTO_VALUE, M_DEFAULT);
   M3dmetCopyResult(MilFitResult, MilPlane, M_FITTED_GEOMETRY, M_DEFAULT);

   MosPrintf(MIL_TEXT("Use the background plane to define the floor.\n\n"));
   M3dmodCopy(MilPlane, M_DEFAULT, MilContext, M_DEFAULT, M_FLOOR, M_DEFAULT);

   /* Remove the floor plane from the scan for faster and more accurate 3d model finding. */
   MIL_ID MilConfidence = MbufInquireContainer(MilSceneContainer1, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   M3dmetCopyResult(MilFitResult, MilConfidence, M_OUTLIER_MASK, M_DEFAULT);

   MosPrintf(MIL_TEXT("Find a single occurrence without any constraints.\n\n"));
 
   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find without any constraints. */
   Finder.Find(MilContext, MilSceneContainer1);

   /* Show the find results. */
   Label = Finder.ShowResults();

   MosPrintf(MIL_TEXT("Use the single occurrence's result to define a resting plane constraint.\n\n"));
   /* Use the found occurrence's result as a constraint for the multiple occurrences. */
   auto MilResult = Finder.GetResult();
   M3dmodCopyResult(MilResult , 0, MilContext, 0, M_RESTING_PLANE, M_DEFAULT);
   M3dmodControl   (MilContext, 0, M_RESTING_PLANE_ANGLE_TOLERANCE, 5);

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();
   Finder.ClearScene(Label);

   /* Display the point clouds.*/
   Finder.ShowContainers(MilModelContainer, MilSceneContainer2, M_BOTTOM_TILTED);

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   MosPrintf(MIL_TEXT("Find multiple occurrences with the resting plane constraint.\n\n"));

   /* Find with a resting plane constraint. */
   Finder.Find(MilContext, MilSceneContainer2);

   /* Show the find results. */
   Finder.ShowResults();

   MosPrintf(MIL_TEXT("\nPress <Enter> for the next example.\n\n"));
   MosGetch();
   }

//*****************************************************************************************/
/* Surface 3D Model Finder with sorting of the output results.                            */
//*****************************************************************************************/
void SortedFinder(MIL_ID MilSystem, CSurfaceFinder& Finder)
   {
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("Surface 3D Mdoel Finder with sorting of the output results.             \n"));
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------\n\n"));

   /* Restore the point clouds.*/
   auto MilModelContainer = MbufRestore(SORTED_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(SORTED_SCENE, MilSystem, M_UNIQUE_ID);

   /* Display the point clouds. */
   Finder.ShowContainers(MilModelContainer, MilSceneContainer, M_BOTTOM_VIEW);

   MIL_UNIQUE_3DMOD_ID MilContext;
   AllocAndDefineContext(MilSystem, MilModelContainer, MilContext);

   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n\n"));

   /* The surface 3D Model Finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud. */
   AddComponentNormalsIfMissing(MilSceneContainer);

   /* Multiple occurrences. */
   M3dmodControl(MilContext, 0, M_NUMBER, M_ALL);

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   MosPrintf(MIL_TEXT("Find with default sorting, where occurrences are sorted by score.\n\n"));

   /* Find with the default sorting. */
   Finder.Find(MilContext, MilSceneContainer);

   /* Show the find results. */
   auto Label = Finder.ShowResults();

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   Finder.ClearScene(Label);

   MosPrintf(MIL_TEXT("Find with occurrences sorted based on their positions relative to the z-axis.\n\n"));

   /* Sort by position in the z-axis. */
   M3dmodControl(MilContext, M_DEFAULT, M_SORT_DIRECTION, M_SORT_UP);
   M3dmodControl(MilContext, M_DEFAULT, M_SORT          , M_MIN_Z  );

   /* Preprocess the model. */
   Finder.PreprocessModel(MilContext);

   /* Find with a sorting option. */
   Finder.Find(MilContext, MilSceneContainer);

   /* Show the find results. */
   Finder.ShowResults();

   MosPrintf(MIL_TEXT("\nPress <Enter> for the next example.\n\n"));
   MosGetch();
   }

/*********************************************************************************/
/* Allocates a surface 3D Model Finder context and defines the model.            */
/*********************************************************************************/
void AllocAndDefineContext(MIL_ID               MilSystem        ,
                           MIL_ID               MilModelContainer,
                           MIL_UNIQUE_3DMOD_ID& MilContext       )
   {
   /* Allocates a surface 3D Model Finder context. */
   MilContext = M3dmodAlloc(MilSystem, M_FIND_SURFACE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
  
   /* Define the surface model. */
   M3dmodDefine(MilContext, M_ADD_FROM_POINT_CLOUD, M_SURFACE, (MIL_DOUBLE)MilModelContainer,
                M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MosPrintf(MIL_TEXT("The model is defined using the given model point cloud.\n\n"));

   }

/*********************************************************************************/
/* Allocates a surface 3D Model Finder result.                                   */
/*********************************************************************************/
void CSurfaceFinder::AllocateResult()
   {
   m_MilResult = M3dmodAllocResult(m_MilSystem, M_FIND_SURFACE_RESULT, M_DEFAULT, M_UNIQUE_ID);
   }

/*********************************************************************************/
/* Displays the model and scene containers.                                      */
/*********************************************************************************/
void CSurfaceFinder::ShowContainers(MIL_ID MilModelContainer, MIL_ID MilSceneContainer, MIL_INT View)
   {
   /* Remove previous graphics. */
   MIL_ID MilGraphicsList = (MIL_ID)M3ddispInquire(m_MilDisplayModel, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraRemove(MilGraphicsList, M_ALL, M_DEFAULT);
   MilGraphicsList = (MIL_ID)M3ddispInquire(m_MilDisplayScene, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraRemove(MilGraphicsList, M_ALL, M_DEFAULT);

   M3ddispSelect(m_MilDisplayModel, MilModelContainer, M_ADD, M_DEFAULT);
   M3ddispSelect(m_MilDisplayScene, MilSceneContainer, M_ADD, M_DEFAULT);

   M3ddispSetView(m_MilDisplayModel, M_AUTO, View, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(m_MilDisplayScene, M_AUTO, View, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   M3ddispSelect(m_MilDisplayModel, M_NULL, M_OPEN, M_DEFAULT);
   M3ddispSelect(m_MilDisplayScene, M_NULL, M_OPEN, M_DEFAULT);

   m_View = View;
   M3dgraRemove(m_ProcessModelGraphicsList, M_ALL, M_DEFAULT);
   }

/*************************************************/
/* Find the occurrences in the scene.            */
/*************************************************/
void CSurfaceFinder::Find(MIL_ID MilContext, MIL_ID MilContainer)
   {
   MosPrintf(MIL_TEXT("Surface 3D Model Finder is running..\n\n"));

   m_ComputationTime = 0.0;

   /* Reset the timer. */
   MappTimer(M_TIMER_RESET, M_NULL);

   /* Find the model. */
   M3dmodFind(MilContext, MilContainer, m_MilResult, M_DEFAULT);

   /* Read the find time. */
   MappTimer(M_TIMER_READ, &m_ComputationTime);
   }

/******************************************************************/
/* Preprocess the model and show the preprocessed model.          */
/******************************************************************/
MIL_INT64 CSurfaceFinder::PreprocessModel(MIL_ID MilContext)
   {
   /* Preprocess the context. */
   M3dmodPreprocess(MilContext, M_DEFAULT);

   /* Show the preprocessed model. */
   M3dgraRemove(m_ProcessModelGraphicsList, M_ALL, M_DEFAULT);
   auto MilDrawContext = M3dmodAlloc(m_MilSystem, M_DRAW_3D_SURFACE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dmodControlDraw(MilDrawContext, M_DRAW_MODEL_PREPROCESSED, M_ACTIVE, M_ENABLE);
   auto Label =  M3dmodDraw3d(MilDrawContext, MilContext, M_DEFAULT, m_ProcessModelGraphicsList, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(m_MilDisplayProcessModel, M_AUTO, m_View, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   return Label;
   }

/*****************************************************************************/
/* Show the surface 3D Model Finder results.                                 */
/*****************************************************************************/
  MIL_INT64  CSurfaceFinder::ShowResults()
      {
      MIL_INT NbOcc = 0;

      MIL_INT Status;
      M3dmodGetResult(m_MilResult, M_DEFAULT, M_STATUS, &Status);

      switch(Status)
         {
         case M_NOT_INITIALIZED:
            MosPrintf(MIL_TEXT("Surface finding failed: the result is not initialized.\n\n"));
            break;
         case M_NOT_ENOUGH_MEMORY:
            MosPrintf(MIL_TEXT("Surface finding failed: not enough memory.\n\n"));
            break;
         case M_NOT_ENOUGH_VALID_DATA:
            MosPrintf(MIL_TEXT("Surface finding failed: not enough valid points in the point cloud.\n\n"));
            break;
         case M_MISSING_COMPONENT_NORMALS_MIL:
            MosPrintf(MIL_TEXT("Surface finding failed: M_COMPONENT_NORMALS_MIL is not found in\n")
                      MIL_TEXT("the point cloud.\n\n"));
            break;
         case M_COMPLETE: { M3dmodGetResult(m_MilResult, M_DEFAULT, M_NUMBER, &NbOcc);
            MosPrintf(MIL_TEXT("Found %i occurrence(s) in %.2f s.\n\n"), NbOcc, m_ComputationTime);
         }break;
         default:  break;
         }

      if(NbOcc == 0)
         return 0;

      MosPrintf(MIL_TEXT("Index        Score        Score_Target \n"));
      MosPrintf(MIL_TEXT("---------------------------------------\n"));

      for(MIL_INT i = 0; i < NbOcc; ++i)
         {
         MIL_DOUBLE ScoreTarget = M3dmodGetResult(m_MilResult, i, M_SCORE_TARGET, M_NULL);
         MIL_DOUBLE Score       = M3dmodGetResult(m_MilResult, i, M_SCORE, M_NULL);


         MosPrintf(MIL_TEXT("  %i          %.4f        %6.2f          \n"),
                   i, Score, ScoreTarget);
         }

      MosPrintf(MIL_TEXT("\n"));

      /* Draws all occurrences by the default draw3d context.*/
      return M3dmodDraw3d(M_DEFAULT, m_MilResult, M_ALL, m_SceneGraphicsList, M_DEFAULT, M_DEFAULT);
      }

/******************************************************************/
/* Draws a 3D Model Finder result in the scene.                   */
/******************************************************************/
  MIL_INT64 CSurfaceFinder::DrawInScene(MIL_ID MilDrawContext)
     {
     return M3dmodDraw3d(MilDrawContext, m_MilResult, M_DEFAULT, m_SceneGraphicsList, M_DEFAULT, M_DEFAULT);
     }

/******************************************************************/
/* Allocates the 3D displays.                                     */
/******************************************************************/
void CSurfaceFinder::AllocateDisplays()
   {
   m_MilDisplayModel = Alloc3dDisplayId(m_MilSystem);
   M3ddispControl(m_MilDisplayModel, M_SIZE_X, DISP_SIZE_X / 2);
   M3ddispControl(m_MilDisplayModel, M_SIZE_Y, DISP_SIZE_Y / 2);
   M3ddispControl(m_MilDisplayModel, M_TITLE, MIL_TEXT("Model Cloud"));

   m_MilDisplayProcessModel = Alloc3dDisplayId(m_MilSystem);
   M3ddispControl(m_MilDisplayProcessModel, M_SIZE_X, DISP_SIZE_X / 2);
   M3ddispControl(m_MilDisplayProcessModel, M_SIZE_Y, DISP_SIZE_Y / 2);
   M3ddispControl(m_MilDisplayProcessModel, M_TITLE, MIL_TEXT("Preprocessed model Cloud"));
   M3ddispControl(m_MilDisplayProcessModel, M_WINDOW_INITIAL_POSITION_Y, (MIL_INT)(1.2 * 0.5 * DISP_SIZE_Y));

   m_MilDisplayScene = Alloc3dDisplayId(m_MilSystem);
   M3ddispControl(m_MilDisplayScene, M_SIZE_X, DISP_SIZE_X);
   M3ddispControl(m_MilDisplayScene, M_SIZE_Y, DISP_SIZE_Y);
   M3ddispControl(m_MilDisplayScene, M_WINDOW_INITIAL_POSITION_X, (MIL_INT)(1.04 * 0.5 * DISP_SIZE_X));
   M3ddispControl(m_MilDisplayScene, M_TITLE, MIL_TEXT("Scene Cloud"));

   m_SceneGraphicsList        = M3ddispInquire(m_MilDisplayScene, M_3D_GRAPHIC_LIST_ID, M_NULL);
   m_ProcessModelGraphicsList = M3ddispInquire(m_MilDisplayProcessModel, M_3D_GRAPHIC_LIST_ID, M_NULL);

   M3ddispSelect(m_MilDisplayProcessModel, M_NULL, M_OPEN, M_DEFAULT);
   }

/******************************************************************/
/* Adds the component M_COMPONENT_NORMALS_MIL if it's not found.  */
/******************************************************************/
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

/*****************************************************************************/
/* Allocates a 3D display and returns its MIL identifier.                    */
/*****************************************************************************/
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto MilDisplay3D =
      M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to exit.\n"));
      MosGetch();
      exit(0);
      }
   return MilDisplay3D;
   }
/****************************************************************************/
/* Checks the required files exist.                                         */
/****************************************************************************/
bool CheckForRequiredMILFile(MIL_STRING FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);

   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }
