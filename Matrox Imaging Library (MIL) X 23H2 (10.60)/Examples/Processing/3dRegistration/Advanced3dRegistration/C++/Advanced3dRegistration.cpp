/*****************************************************************************/
/*
* File name: Advanced3dRegistration.cpp
*
* Synopsis: This example demonstrates the use of advanced 3D registration
*           controls and subsampling modes to improve the robustness,
*           precision, and speed in various registration scenarios.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include "CRegDisplay.h"
#include "DisplayLinker.h"
#include <mil.h>
#include <map>

//*****************************************************************************
// Example description.
//*****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Advanced3dRegistration\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the use of advanced 3d registration controls \n"));
   MosPrintf(MIL_TEXT("and subsampling modes to improve the robustness, precision, and speed in\n"));
   MosPrintf(MIL_TEXT("various registration scenarios. \n"));
   MosPrintf(MIL_TEXT("\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));

   MosPrintf(MIL_TEXT("Modules used: 3D Registration, 3D Display, and 3D Graphics.\n\n"));
   }

/* Constants. */
static const MIL_INT NUM_SCANS = 2;
static const MIL_INT MAX_ITERATIONS = 100;
static const MIL_INT REFERENCE_INDEX = 0;
static const MIL_INT TARGET_INDEX = 1;
static const MIL_INT DISP_SX = 500;
static const MIL_INT DISP_SY = 500;

/* Defines. */
#define EXAMPLE_IMAGE_PATH    M_IMAGE_PATH MIL_TEXT("Advanced3dRegistration/")

/* Input scanned point cloud (PLY) files. */
static const MIL_STRING PIN_MODEL = EXAMPLE_IMAGE_PATH MIL_TEXT("ClothesPinModel.ply");
static const MIL_STRING PIN_SCENE = EXAMPLE_IMAGE_PATH MIL_TEXT("ClothesPinScene.ply");

static const MIL_STRING PLUG_MODEL = EXAMPLE_IMAGE_PATH MIL_TEXT("PlugModel1.ply");
static const MIL_STRING PLUG_SCENE = EXAMPLE_IMAGE_PATH MIL_TEXT("PlugScene1.ply");

static const MIL_STRING TOY_MODEL = EXAMPLE_IMAGE_PATH MIL_TEXT("PillarModel.ply");
static const MIL_STRING TOY_SCENE = EXAMPLE_IMAGE_PATH MIL_TEXT("PillarScene.ply");

static const MIL_STRING COAXIAL_JACK_MODEL = EXAMPLE_IMAGE_PATH MIL_TEXT("CoaxialJackModel.ply");
static const MIL_STRING COAXIAL_JACK_SCENE = EXAMPLE_IMAGE_PATH MIL_TEXT("CoaxialJackScene.ply");

static const MIL_STRING AERATEUR_MODEL = EXAMPLE_IMAGE_PATH MIL_TEXT("AeratorKeyModel.ply");
static const MIL_STRING AERATEUR_SCENE = EXAMPLE_IMAGE_PATH MIL_TEXT("AeratorKeyScene.ply");

/* The colors assigned to each point cloud. */
const MIL_INT Color[NUM_SCANS] =
   {
   M_RGB888(0,   159, 255),
   M_RGB888(154,  77,  66)
   };

/* Structures.*/
struct SRegistrationStats
   {
   MIL_DOUBLE ComputationTime;
   MIL_DOUBLE RMSError;
   MIL_INT    NbIteration;
   };

/* Functions. */
void ExecuteExample(MIL_ID MilSystem,
                    MIL_STRING BasicContextName,
                    MIL_STRING ImprovedContextName,
                    MIL_ID MilContextBasic,
                    MIL_ID MilContextImproved,
                    MIL_ID MilModelContainer,
                    MIL_ID MilSceneContainer,
                    CCameraOrientation InitialCameraOrientation,
                    bool IsFinalExample = false);
void ColorCloud(MIL_ID MilPointCloud,
                MIL_INT Col);
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);
MIL_INT64 DisplayContainer(MIL_ID MilDisplay,
                           MIL_ID MilContainer);
void SubsampleContainer(MIL_DOUBLE NeighborhoodDistance,
                        MIL_ID MilSystem,
                        MIL_ID  MilContainerId);
SRegistrationStats PerformRegistration(MIL_ID MilSystem,
                                       MIL_ID MilReference,
                                       MIL_ID MilTarget,
                                       MIL_ID MilDisplay,
                                       MIL_ID MilDisplayContainer,
                                       MIL_ID MilContext,
                                       const CCameraParameters& CameraParameters);
void PrintRegistrationStats(const SRegistrationStats& BasicStats,
                            const SRegistrationStats& ImprovedStats);

void PairsCreationFromTargetExample(MIL_ID MilSystem);
void PairsRejectionExample(MIL_ID MilSystem);
void TargetPointLimitExample(MIL_ID MilSystem);
void GeometricSubsamplingExample(MIL_ID MilSystem);
void FullAutoExample(MIL_ID MilSystem);


int MosMain()
   {
   /* Print example information in console. */
   PrintHeader();

   /* Allocate MIL objects. */
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   /* Execute registrations. */
   PairsCreationFromTargetExample(MilSystem);
   PairsRejectionExample(MilSystem);
   TargetPointLimitExample(MilSystem);
   GeometricSubsamplingExample(MilSystem);
   FullAutoExample(MilSystem);

   return 0;
   }

//*****************************************************************************
// Print example information.
//*****************************************************************************
void PrintExampleInfo(MIL_STRING BasicAlgorithmName,
                      MIL_STRING BasicAlgorithmDescription,
                      MIL_STRING ImprovedAlgorithmName,
                      MIL_STRING ImprovedAlgorithmDescription)
   {
   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("%s Example\t\n"), ImprovedAlgorithmName.c_str());
   MosPrintf(MIL_TEXT("------------------------------------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("[Basic]\n"));
   MosPrintf(MIL_TEXT("%s\n"), BasicAlgorithmName.c_str());
   MosPrintf(MIL_TEXT("   %s\n\n"), BasicAlgorithmDescription.c_str());

   MosPrintf(MIL_TEXT("[Improved]\n"));
   MosPrintf(MIL_TEXT("%s\n"), ImprovedAlgorithmName.c_str());
   MosPrintf(MIL_TEXT("   %s\n\n"), ImprovedAlgorithmDescription.c_str());
   }

//*****************************************************************************
// Pairs creation from target example.
//*****************************************************************************
void PairsCreationFromTargetExample(MIL_ID MilSystem)
   {
   CCameraOrientation InitialCameraOrientation(-90, -90, 0);

   auto MilModelContainer = MbufRestore(PIN_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(PIN_SCENE, MilSystem, M_UNIQUE_ID);

   /* Subsample containers. */
   SubsampleContainer(0.2, MilSystem, MilModelContainer);
   SubsampleContainer(0.2, MilSystem, MilSceneContainer);

   /* Define basic algorithm parameters. */
   auto MilContextBasic = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   M3dregControl(MilContextBasic, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_PLANE);
   M3dregControl(MilContextBasic, M_DEFAULT, M_PREREGISTRATION_MODE, M_CENTROID);

   /* Define improved algorithm parameters. */
   auto MilContextImproved = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   /* Match basic algorithm settings. */
   M3dregControl(MilContextImproved, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_PLANE);
   M3dregControl(MilContextImproved, M_DEFAULT, M_PREREGISTRATION_MODE, M_CENTROID);
   /* Activate M_PAIRS_CREATION_FROM_TARGET. The algorithm will create pairs for each target point. if*/
   /* source and target are too far apart.*/
   M3dregControl(MilContextImproved, M_DEFAULT, M_PAIRS_CREATION_FROM_TARGET, M_AUTO);

   MIL_STRING BasicAlgorithmName = MIL_TEXT("Centered point-to-plane");
   MIL_STRING ImprovedAlgorithmName = MIL_TEXT("Pairs creation from target");

   PrintExampleInfo(BasicAlgorithmName,
                    MIL_TEXT("Default point-to-plane settings and centroid prealignment."),
                    ImprovedAlgorithmName,
                    MIL_TEXT("M_PAIRS_CREATION_FROM_TARGET option set to M_AUTO.\n")
                    MIL_TEXT("This option allows pairs to be created from target to reference, which is\n")
                    MIL_TEXT("useful when the target and reference point clouds have very dissimilar \n")
                    MIL_TEXT("initial positions and orientations."));

   ExecuteExample(MilSystem, BasicAlgorithmName, ImprovedAlgorithmName,
                  MilContextBasic, MilContextImproved, MilModelContainer, MilSceneContainer, InitialCameraOrientation);

   }

//*****************************************************************************
// Pairs rejection example.
//*****************************************************************************
void PairsRejectionExample(MIL_ID MilSystem)
   {
   CCameraOrientation InitialCameraOrientation(140, -120, 0);

   auto MilModelContainer = MbufRestore(PLUG_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(PLUG_SCENE, MilSystem, M_UNIQUE_ID);

   /* Subsample containers. */
   SubsampleContainer(0.2, MilSystem, MilModelContainer);
   SubsampleContainer(0.2, MilSystem, MilSceneContainer);

   /* Define basic algorithm parameters. */
   auto MilContextBasic = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   M3dregControl(MilContextBasic, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_POINT);
   M3dregControl(MilContextBasic, M_DEFAULT, M_PREREGISTRATION_MODE, M_CENTROID);
   M3dregControl(MilContextBasic, M_ALL, M_OVERLAP, 100);

   /* Define improved algorithm parameters. */
   auto MilContextImproved = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   /* Match basic algorithm settings. */
   M3dregControl(MilContextImproved, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_POINT);
   M3dregControl(MilContextImproved, M_DEFAULT, M_PREREGISTRATION_MODE, M_CENTROID);
   M3dregControl(MilContextImproved, M_ALL, M_OVERLAP, 100);

   /* Activate pairs rejection. */
   M3dregControl(MilContextImproved, M_ALL, M_PAIRS_REJECTION_MODE, M_ROBUST_STANDARD_DEVIATION);
   M3dregControl(MilContextImproved, M_ALL, M_PAIRS_REJECTION_FACTOR, 3);

   MIL_STRING BasicAlgorithmName = MIL_TEXT("Overlap 100 point-to-point");
   MIL_STRING ImprovedAlgorithmName = MIL_TEXT("Pairs Rejection");

   PrintExampleInfo(BasicAlgorithmName,
                    MIL_TEXT("Point-to-point registration with M_OVERLAP set to 100 and\n")
                    MIL_TEXT("centroid prealignment."),
                    ImprovedAlgorithmName,
                    MIL_TEXT("Robust pairs rejection settings used.\n")
                    MIL_TEXT("In high overlap case where occlusion amount is hard to predict,\n")
                    MIL_TEXT("pairs rejection helps to automatically reject false pairs and\n")
                    MIL_TEXT("give a close to optimal overlap at the end of convergence."));

   ExecuteExample(MilSystem, BasicAlgorithmName, ImprovedAlgorithmName,
                  MilContextBasic, MilContextImproved, MilModelContainer, MilSceneContainer, InitialCameraOrientation);
   }

//*****************************************************************************
// Pairs rejection example.
//*****************************************************************************
void TargetPointLimitExample(MIL_ID MilSystem)
   {
   CCameraOrientation InitialCameraOrientation(90, 180, 0);

   auto MilModelContainer = MbufRestore(TOY_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(TOY_SCENE, MilSystem, M_UNIQUE_ID);

   /* Subsample containers. */
   SubsampleContainer(0.2, MilSystem, MilModelContainer);
   SubsampleContainer(0.2, MilSystem, MilSceneContainer);

   /* Define basic algorithm parameters. */
   auto MilContextBasic = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   M3dregControl(MilContextBasic, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_POINT);

   /* Define improved algorithm parameters. */
   auto MilContextImproved = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   /* Match basic algorithm settings. */
   M3dregControl(MilContextImproved, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_POINT);

   /* Activate M_PAIRS_LIMIT_PER_TARGET_POINT_MODE. */
   M3dregControl(MilContextImproved, M_DEFAULT, M_PAIRS_LIMIT_PER_TARGET_POINT_MODE, M_SINGLE);

   MIL_STRING BasicAlgorithmName = MIL_TEXT("Default point-to-point");
   MIL_STRING ImprovedAlgorithmName = MIL_TEXT("Target point limit");

   PrintExampleInfo(BasicAlgorithmName,
                    MIL_TEXT("point-to-point registration."),
                    ImprovedAlgorithmName,
                    MIL_TEXT("M_PAIRS_LIMIT_PER_TARGET_POINT_MODE option set to M_SINGLE.\n")
                    MIL_TEXT("This option pairs each reference point with a single point in the target\n")
                    MIL_TEXT("point cloud, which is useful to fine-tune the registration while reducing\n")
                    MIL_TEXT("the effect of noise and outliers. Recommended when the target and reference\n")
                    MIL_TEXT("point clouds are initially close to each other."));

   ExecuteExample(MilSystem, BasicAlgorithmName, ImprovedAlgorithmName,
                  MilContextBasic, MilContextImproved, MilModelContainer, MilSceneContainer, InitialCameraOrientation);
   }

//*****************************************************************************
// Geometric subsampling example.
//*****************************************************************************
void GeometricSubsamplingExample(MIL_ID MilSystem)
   {
   CCameraOrientation InitialCameraOrientation(-90, -90, 0);

   auto MilModelContainer = MbufRestore(COAXIAL_JACK_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(COAXIAL_JACK_SCENE, MilSystem, M_UNIQUE_ID);

   /* Subsample containers. */
   SubsampleContainer(0.2, MilSystem, MilModelContainer);
   SubsampleContainer(0.2, MilSystem, MilSceneContainer);

   /* Define basic algorithm parameters. */
   auto MilContextBasic = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   M3dregControl(MilContextBasic, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_PLANE);
   M3dregControl(MilContextBasic, M_ALL, M_OVERLAP, 100);

   /* Define improved algorithm parameters. */
   auto MilContextImproved = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   /* Match basic algorithm settings. */
   M3dregControl(MilContextImproved, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_PLANE);
   M3dregControl(MilContextImproved, M_ALL, M_OVERLAP, 100);

   /* Activate geometric subsampling. */
   MIL_ID MilSubsampleContext = M_NULL;
   M3dregInquire(MilContextImproved, 1, M_SUBSAMPLE_REFERENCE_CONTEXT_ID, &MilSubsampleContext);
   M3dregControl(MilContextImproved, 1, M_SUBSAMPLE_REFERENCE, M_ENABLE);

   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GEOMETRIC);
   M3dimControl(MilSubsampleContext, M_FRACTION_OF_POINTS, 0.3);

   MIL_STRING BasicAlgorithmName = MIL_TEXT("Overlap 100 point-to-plane");
   MIL_STRING ImprovedAlgorithmName = MIL_TEXT("Geometric subsampling");

   PrintExampleInfo(BasicAlgorithmName,
                    MIL_TEXT("point-to-plane registration with M_OVERLAP set to 100."),
                    ImprovedAlgorithmName,
                    MIL_TEXT("Registration with geometric subsampling.\n")
                    MIL_TEXT("Geometric subsampling is performed on the reference point cloud before\n")
                    MIL_TEXT("registration. This removes featureless points that could lead to divergence\n")
                    MIL_TEXT("or a slower convergence."));

   ExecuteExample(MilSystem, BasicAlgorithmName, ImprovedAlgorithmName,
                  MilContextBasic, MilContextImproved, MilModelContainer, MilSceneContainer, InitialCameraOrientation);
   }

//*****************************************************************************
// Full auto example.
//*****************************************************************************
void FullAutoExample(MIL_ID MilSystem)
   {
   CCameraOrientation InitialCameraOrientation(90, -90, 0);

   auto MilModelContainer = MbufRestore(AERATEUR_MODEL, MilSystem, M_UNIQUE_ID);
   auto MilSceneContainer = MbufRestore(AERATEUR_SCENE, MilSystem, M_UNIQUE_ID);

   /* Subsample containers. */
   SubsampleContainer(0.2, MilSystem, MilModelContainer);
   SubsampleContainer(0.2, MilSystem, MilSceneContainer);

   /* Define basic algorithm parameters. */
   auto MilContextBasic = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dregControl(MilContextBasic, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_PLANE);
   M3dregControl(MilContextBasic, M_ALL, M_OVERLAP, 100);
   MIL_ID MilSubsampleContextBasic = M_NULL;
   M3dregInquire(MilContextBasic, 1, M_SUBSAMPLE_REFERENCE_CONTEXT_ID, &MilSubsampleContextBasic);
   M3dregControl(MilContextBasic, 1, M_SUBSAMPLE_REFERENCE, M_ENABLE);

   /* Keep edge points. */
   M3dimControl(MilSubsampleContextBasic, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GEOMETRIC);
   M3dimControl(MilSubsampleContextBasic, M_FRACTION_OF_POINTS, 0.3);

   /* Define improved algorithm parameters. */
   auto MilContextImproved = M3dregAlloc(MilSystem, M_PAIRWISE_REGISTRATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   /* Match basic algorithm settings. */
   M3dregControl(MilContextImproved, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_PLANE);
   M3dregControl(MilContextImproved, M_ALL, M_OVERLAP, 100);
   MIL_ID MilSubsampleContext = M_NULL;
   M3dregInquire(MilContextImproved, 1, M_SUBSAMPLE_REFERENCE_CONTEXT_ID, &MilSubsampleContext);
   M3dregControl(MilContextImproved, 1, M_SUBSAMPLE_REFERENCE, M_ENABLE);

   /* Keep edge points. */
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GEOMETRIC);
   M3dimControl(MilSubsampleContext, M_FRACTION_OF_POINTS, 0.3);

   /* Activate M_PAIRS_CREATION_PER_REFERENCE_POINT_MODE, M_PAIRS_LIMIT_PER_TARGET_POINT_MODE, */
   /* M_PAIRS_CREATION_FROM_TARGET, and M_PAIRS_REJECTION_MODE. */
   M3dregControl(MilContextImproved, M_DEFAULT, M_PAIRS_CREATION_PER_REFERENCE_POINT_MODE, M_AUTO);
   M3dregControl(MilContextImproved, M_DEFAULT, M_PAIRS_LIMIT_PER_TARGET_POINT_MODE, M_AUTO);
   M3dregControl(MilContextImproved, M_DEFAULT, M_PAIRS_CREATION_FROM_TARGET, M_AUTO);
   M3dregControl(MilContextImproved, M_ALL, M_PAIRS_REJECTION_MODE, M_ROBUST_STANDARD_DEVIATION);

   MIL_STRING BasicAlgorithmName = MIL_TEXT("Geometric subsampling");
   MIL_STRING ImprovedAlgorithmName = MIL_TEXT("Full auto");

   PrintExampleInfo(BasicAlgorithmName,
                    MIL_TEXT("Registration with geometric subsampling.\n")
                    MIL_TEXT("Geometric subsampling is performed on the reference point cloud before\n")
                    MIL_TEXT("registration. This removes featureless points that could lead to divergence\n")
                    MIL_TEXT("or a slower convergence."),
                    ImprovedAlgorithmName,
                    MIL_TEXT("This algorithm sets M_PAIRS_CREATION_PER_REFERENCE_POINT_MODE,\n")
                    MIL_TEXT("M_PAIRS_LIMIT_PER_TARGET_POINT_MODE, and M_PAIRS_CREATION_FROM_TARGET to M_AUTO.\n")
                    MIL_TEXT("The optimal value for each of these features is calculated internally at each\n")
                    MIL_TEXT("iteration. Pairs rejection and geometric subsampling are also used."));

   ExecuteExample(MilSystem, BasicAlgorithmName, ImprovedAlgorithmName,
                  MilContextBasic, MilContextImproved, MilModelContainer, MilSceneContainer, InitialCameraOrientation,
                  true);
   }

//*****************************************************************************
// Execute the registration using a basic and then an improved algorithm.
//*****************************************************************************
void ExecuteExample(MIL_ID MilSystem,
                    MIL_STRING BasicContextName,
                    MIL_STRING ImprovedContextName,
                    MIL_ID MilContextBasic,
                    MIL_ID MilContextImproved,
                    MIL_ID MilModelContainer,
                    MIL_ID MilSceneContainer,
                    CCameraOrientation InitialCameraOrientation,
                    bool IsFinalExample)
   {
   ColorCloud(MilModelContainer, Color[REFERENCE_INDEX]);
   ColorCloud(MilSceneContainer, Color[TARGET_INDEX]);

   /* Display reference and target. */
   auto MilDisplay = Alloc3dDisplayId(MilSystem);
   InitialCameraOrientation.ApplyToDisplay(MilDisplay);

   CWindowParameters RefAndTargetWindowParam(MIL_TEXT("Reference and Target"), 0, 0, DISP_SX, DISP_SY);
   RefAndTargetWindowParam.ApplyToDisplay(MilDisplay);

   DisplayContainer(MilDisplay, MilModelContainer);
   DisplayContainer(MilDisplay, MilSceneContainer);

   M3ddispSetView(MilDisplay, M_VIEW_BOX, M_WHOLE_SCENE, 1.0, M_DEFAULT, M_DEFAULT);

   CCameraParameters MainDisplayCameraParams(MilSystem, MilDisplay);

   MosPrintf(MIL_TEXT("The reference and target point clouds are displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to register.\n\n"));
   MosGetch();

   /* Display reference algorithm. */
   auto MilDisplayBasic = Alloc3dDisplayId(MilSystem);
   CWindowParameters BasicWinParam(MIL_TEXT("Basic-") + BasicContextName, DISP_SX, 0, DISP_SX, DISP_SY);
   BasicWinParam.ApplyToDisplay(MilDisplayBasic);

   MosPrintf(MIL_TEXT("Calculating basic registration... \n\n"));

   auto MilDisplayContainerBasic = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   auto BasicStats = PerformRegistration(MilSystem,
                                         MilModelContainer,
                                         MilSceneContainer,
                                         MilDisplayBasic,
                                         MilDisplayContainerBasic,
                                         MilContextBasic,
                                         MainDisplayCameraParams);

   BasicWinParam.m_PositionY = DISP_SY;
   CRegDisplay BasicRegDisplay(MilModelContainer,
                               MilSceneContainer,
                               MilContextBasic,
                               BasicWinParam,
                               MainDisplayCameraParams);

   /* Display algorithm to highlight. */
   auto MilDisplayImproved = Alloc3dDisplayId(MilSystem);
   CWindowParameters ImprovedWinParam(MIL_TEXT("Improved-") + ImprovedContextName, DISP_SX * 2, 0, DISP_SX, DISP_SY);
   ImprovedWinParam.ApplyToDisplay(MilDisplayImproved);

   MosPrintf(MIL_TEXT("Calculating improved registration... \n\n"));

   auto MilDisplayContainerImproved = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   auto ImprovedStats = PerformRegistration(MilSystem,
                                            MilModelContainer,
                                            MilSceneContainer,
                                            MilDisplayImproved,
                                            MilDisplayContainerImproved,
                                            MilContextImproved,
                                            MainDisplayCameraParams);

   ImprovedWinParam.m_PositionY = DISP_SY;
   CRegDisplay ImprovedRegDisplay(MilModelContainer,
                                  MilSceneContainer,
                                  MilContextImproved,
                                  ImprovedWinParam,
                                  MainDisplayCameraParams);

   MosPrintf(MIL_TEXT("The registration results are displayed.\n\n"));

   /* Print the registration statistics. */
   PrintRegistrationStats(BasicStats, ImprovedStats);

   /* Link the displays' views. */
   CDisplayLinker DisplayLinker({MilDisplay, MilDisplayBasic, MilDisplayImproved,
                                BasicRegDisplay.GetMilDisplayID(), ImprovedRegDisplay.GetMilDisplayID()});

   /* Start display control thread. */
   CDisplayController Controller;
   Controller.RegisterDisplay(&BasicRegDisplay);
   Controller.RegisterDisplay(&ImprovedRegDisplay);
   Controller.Start(IsFinalExample);

   DisplayLinker.StopLink();
   }

//*****************************************************************************
// Color the point cloud.
//*****************************************************************************
void ColorCloud(MIL_ID MilPointCloud, MIL_INT Col)
   {
   MIL_INT SizeX = MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   MIL_ID ReflectanceId = MbufInquireContainer(MilPointCloud, M_COMPONENT_REFLECTANCE, M_COMPONENT_ID, M_NULL);
   if(ReflectanceId == M_NULL)
      {
      ReflectanceId = MbufAllocComponent(MilPointCloud, 3, SizeX, SizeY,
                                         8 + M_UNSIGNED, M_IMAGE, M_COMPONENT_REFLECTANCE, M_NULL);
      }
   MbufClear(ReflectanceId, static_cast<MIL_DOUBLE>(Col));

   }

//*****************************************************************************
// Allocate a 3D display and return its MIL identifier. 
//*****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"),
                                                    M_DEFAULT, M_UNIQUE_ID);
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

//*****************************************************************************
// Display the received container. 
//*****************************************************************************
MIL_INT64 DisplayContainer(MIL_ID MilDisplay, MIL_ID MilContainer)
   {
   MIL_INT Label = M3ddispSelect(MilDisplay, MilContainer, M_ADD, M_DEFAULT);
   M3ddispSelect(MilDisplay, M_NULL, M_OPEN, M_DEFAULT);

   return Label;
   }

//*****************************************************************************
// Subsample container.    
//*****************************************************************************
void SubsampleContainer(MIL_DOUBLE NeighborhoodDistance, MIL_ID MilSystem, MIL_ID  MilContainerId)
   {
   if(NeighborhoodDistance > 0)
      {
      auto SubSamplingContext = M3dimAlloc(MilSystem, M_SUBSAMPLE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
      M3dimControl(SubSamplingContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_NORMAL);
      if(MbufInquireContainer(MilContainerId, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL) == M_NULL)
         M3dimNormals(M_NORMALS_CONTEXT_TREE, MilContainerId, MilContainerId, M_DEFAULT);
      M3dimControl(SubSamplingContext, M_NEIGHBORHOOD_DISTANCE, NeighborhoodDistance);
      M3dimSample(SubSamplingContext, MilContainerId, MilContainerId, M_DEFAULT);
      }
   }

//*****************************************************************************
// Perform registration.  
//*****************************************************************************
SRegistrationStats PerformRegistration(MIL_ID MilSystem,
                                       MIL_ID Reference,
                                       MIL_ID Target,
                                       MIL_ID MilDisplay,
                                       MIL_ID MilDisplayContainer,
                                       MIL_ID MilContext,
                                       const CCameraParameters& CameraParameters)
   {
   SRegistrationStats RegStats;
   MIL_ID MilContainerIds[NUM_SCANS];
   MilContainerIds[REFERENCE_INDEX] = Reference;
   MilContainerIds[TARGET_INDEX] = Target;

   /* Allocate a 3D registration result object. */
   auto MilResult = M3dregAllocResult(MilSystem, M_PAIRWISE_REGISTRATION_RESULT, M_DEFAULT, M_UNIQUE_ID);

   M3dregControl(MilContext, M_DEFAULT, M_NUMBER_OF_REGISTRATION_ELEMENTS, NUM_SCANS);
   M3dregControl(MilContext, M_DEFAULT, M_MAX_ITERATIONS, MAX_ITERATIONS);

   /* Calculate the time to perform the registration. */
   MappTimer(M_TIMER_RESET, M_NULL);

   /* Perform the registration. */
   M3dregCalculate(MilContext, MilContainerIds, NUM_SCANS, MilResult, M_DEFAULT);

   RegStats.ComputationTime = MappTimer(M_TIMER_READ, M_NULL) * 1000.0;
   M3dregGetResult(MilResult, 1, M_RMS_ERROR + M_TYPE_MIL_DOUBLE, &RegStats.RMSError);
   M3dregGetResult(MilResult, 1, M_NB_ITERATIONS, &RegStats.NbIteration);

   /* Display results. */
   M3dregMerge(MilResult, MilContainerIds, NUM_SCANS, MilDisplayContainer, M_NULL, M_DEFAULT);

   M3ddispSelect(MilDisplay, M_NULL, M_REMOVE, M_DEFAULT);

   CameraParameters.ApplyToDisplay(MilDisplay);
   DisplayContainer(MilDisplay, MilDisplayContainer);

   return RegStats;
   }

//*****************************************************************************
// Print the registration statistics.      
//*****************************************************************************
void PrintRegistrationStats(const SRegistrationStats& BasicStats,
                            const SRegistrationStats& ImprovedStats)
   {
   // Print the statistics header.
   MosPrintf(MIL_TEXT("%9s   %11s   %8s   %19s\n"),
             MIL_TEXT("Algorithm"), MIL_TEXT("NbIteration"),
             MIL_TEXT("RMSError"), MIL_TEXT("ComputationTime(ms)"));
   MosPrintf(MIL_TEXT("--------------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("%9s   %11d   %8.2f   %19.2f\n"),
             MIL_TEXT("Basic"), BasicStats.NbIteration, BasicStats.RMSError, BasicStats.ComputationTime);
   MosPrintf(MIL_TEXT("%9s   %11d   %8.2f   %19.2f\n\n"),
             MIL_TEXT("Improved"), ImprovedStats.NbIteration, ImprovedStats.RMSError, ImprovedStats.ComputationTime);
   }
