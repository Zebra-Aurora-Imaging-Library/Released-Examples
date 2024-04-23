//***************************************************************************************/
// 
// File name: HandEyeCalibration.cpp  
//
// Synopsis:  This program contains an example of Hand Eye calibration using the milcal module.
//            See the PrintHeader() function below for detailed description.
//
//  Credits:  The 3D data was simulated and generated thanks to the RoboDK robot
//            simulation environment.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//**************************************************************************************/
#include <mil.h>

#include "SphereDetector.h"

//****************************************************************************
// Defines and constants
//****************************************************************************
#define EXAMPLE_IMAGE_PATH    M_IMAGE_PATH MIL_TEXT("HandEyeCalibration/")
#define ORIGINAL_MODEL_PATH   EXAMPLE_IMAGE_PATH

static const int NUM_SPHERES = 4;
static const MIL_DOUBLE SPHERE_RADIUS[NUM_SPHERES] = {24, 22, 20, 18};

static const MIL_DOUBLE RADIUS_TOLERANCE = 0.5;

// Expected values of the matrix that transforms the TCP Coordinate system
// into Camera coordinate system.
static const MIL_DOUBLE Expected_CMT[16] =
   {0.993159, -0.099594, 0.060961, 6.370077,
    0.104385, 0.991214, -0.081229, 73.924813,
   -0.052336, 0.087036, 0.994829, 31.678853,
    0.000000, 0.000000, 0.000000, 1.000000};

// Expected values of the matrix that transforms the Robot base Coordinate system
// into the Absolute coordinate system.
static const MIL_DOUBLE Expected_AMB[16] =
   {1.000000, 0.000000, 0.000000, 645.000000,
    0.000000, 1.000000, 0.000000, -202.414000,
    0.000000, 0.000000, 1.000000, 11.000000,
    0.000000, 0.000000, 0.000000, 1.000000};

static const int NB_CALIBRATION_POSES = 6;
// Calibration data 
static const SPoseData POSES_DATA[NB_CALIBRATION_POSES] =
   {
   //           Point cloud File                      TranslationX         TranslationY         TranslationZ         RotationX            RotationY            RotationZ
   {EXAMPLE_IMAGE_PATH MIL_TEXT("PointCloud0.ply"),  {569.045213113165,    352.35751518019094,  403.7991966448984,   -153.21522596637038, -39.244420235349956, -111.60959517272498}},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("PointCloud1.ply"),  {243.67367500350719,  -413.60549757017895, 558.097468101954,    -138.02213938852447, 0.5140672595017578,  -71.9962046995104}},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("PointCloud2.ply"),  {790.1549528154275,   -530.3220824298126,  775.9146104660626,    179.63563018446013, 26.782090197028545,  -70.90272464831553}},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("PointCloud3.ply"),  {366.34787168979193,  -287.35745583223655, 303.8000238027653,   -121.80225573873356, 17.35663810241358,   -73.13108323663172}},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("PointCloud4.ply"),  {560.0499098262017,   -668.559507646469,   289.275054880309,    -144.9851201804691,  50.100308233065434,  -62.469704296084345}},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("PointCloud5.ply"),  {393.560409486617,    79.6448210880581,    311.73974739870914,  -147.08292433542383, -44.740300656490156, -87.24160906014554}}
   };

static const SPoseData TestData =
   {EXAMPLE_IMAGE_PATH MIL_TEXT("PointCloud6.ply"),  {391.97817688255066, 386.4314875289665, 357.7001892146799, -116.36050421846146, -15.477880995415973, -134.42392259237334}};


//****************************************************************************
// Forward declaration.
//****************************************************************************
void PrintHeader();
void TestCalibration(MIL_ID MilSystem, MIL_ID A_Matrix, MIL_ID X_Matrix, MIL_ID Z_Matrix, MIL_ID CalibrationObject, const SPoseData& Data, MIL_UNIQUE_3DDISP_ID& Display);
void PrintCameraOnRobotArmDescription();
bool RunCameraOnRobotArmCalibration(MIL_ID MilSystem);


//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("HandEyeCalibration\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example uses a simulated 3D Camera that captures point clouds of a\n")
             MIL_TEXT("3D model from multiple poses. The data is then used by McalCalculateHandEye()\n")
             MIL_TEXT("to solve AX=ZB for X and Z.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: Application, System, Display, Buffer, Calibration, 3D Display\n")
             MIL_TEXT("and 3D Graphics.\n\n")

             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//****************************************************************************
// Camera on robot arm setup description.
//****************************************************************************
void PrintCameraOnRobotArmDescription()
   {
   MosPrintf(
             MIL_TEXT("In the following setup, a simulated 3D Camera is attached to a robotic arm.\n")
             MIL_TEXT("We solve AX=ZB where:\n")
             MIL_TEXT(" - A is the pose of the robot tool coordinate system with respect to the\n")
             MIL_TEXT("   robot base coordinate system. The pose is provided by the robot controller\n")
             MIL_TEXT("   and is an input of McalCalculateHandEye.\n")
             MIL_TEXT(" - X is the pose of the camera coordinate system with respect to the\n")
             MIL_TEXT("   robot tool coordinate system. The pose is an output of\n")
             MIL_TEXT("   McalCalculateHandEye.\n")
             MIL_TEXT(" - Z is the pose of the absolute coordinate system with respect to the\n")
             MIL_TEXT("   robot base coordinate system. The pose is an output of\n")
             MIL_TEXT("   McalCalculateHandEye.\n")
             MIL_TEXT(" - B is the pose of the camera coordinate system with respect to the\n")
             MIL_TEXT("   absolute coordinate system. The pose is determined by locating a known\n")
             MIL_TEXT("   object and is an input of McalCalculateHandEye.\n\n")

             MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();
   }

int MosMain()
   {
   // Allocate Application.
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Allocate MIL objects. 
   auto MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Display example description.
   PrintHeader();

   // Run calibration for setup where a 3D camera is attached on robot arm.
   bool Success = RunCameraOnRobotArmCalibration(MilSystem);

   return !Success;
   }


//*****************************************************************************
// SphereDetector::RunCameraOnRobotArmCalibration
// Desctiption:
//        Calibrate robot setup where a 3D camera is attached to robot arm.
//        Analyze the point cloud captured by the camera and use Robot arm
//        position to infer X and Z matrices.
// Input: MilSystem -  MilSystem ID
// Output: True if successful
//*****************************************************************************
bool RunCameraOnRobotArmCalibration(MIL_ID MilSystem)
   {
   // Print robot pose description.
   PrintCameraOnRobotArmDescription();

   // Read CAD calibration model.
   auto MilCADModel = MbufImport(ORIGINAL_MODEL_PATH MIL_TEXT("CalibrationModel.stl"), M_DEFAULT, M_RESTORE, MilSystem, M_UNIQUE_ID);

   // Allocate the sphere detector.
   SphereDetector Detector(NUM_SPHERES, &SPHERE_RADIUS[0], RADIUS_TOLERANCE);

   // Prepare the common display that shows point clouds taken at different poses.
   MIL_UNIQUE_3DDISP_ID  MilCommonDisplay = Alloc3dDisplayId(MilSystem);
   M3ddispSetView(MilCommonDisplay, M_INTEREST_POINT, 0, 0, 350, M_DEFAULT);
   M3ddispSetView(MilCommonDisplay, M_DISTANCE, 2000, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilCommonDisplay, M_AZIM_ELEV_ROLL, 120.0, -30, 0.0, M_DEFAULT);

   // Allocate display to show the spheres of the CAD model.
   auto MilSpheresDisplay = Alloc3dDisplayId(MilSystem);   

   // Detect spheres on the CAD model.
   std::vector<SphereStats> SourceSpheres = Detector.RetrieveModelSpheres(MilSystem, MilCADModel, MilSpheresDisplay, M_NULL);
   if(SourceSpheres.size() != NUM_SPHERES)
      {
      MosPrintf(MIL_TEXT("Error: Wrong number of spheres detected.\nExpected %i spheres, detected %i.\n"), NUM_SPHERES, (MIL_INT)SourceSpheres.size());
      return false;
      }
   else
      {
      MosPrintf(MIL_TEXT("The model object's spheres have been found with regards to the model origin\n")
                MIL_TEXT("and uniquely identified by their radius. A minimum of 3 spheres is required\n")
                MIL_TEXT("to determine the pose; %i are used here for increased robustness.\n\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"),
                NUM_SPHERES);
      MosGetch();
      } 

   // Create Stream containing the centers of the Cad model spheres.
   PointStream SourceStream(SourceSpheres);
   auto SourceData = SourceStream.CreateStreamBuffer(MilSystem);

   // Declare A and B Matrices.
   std::vector<MIL_UNIQUE_3DGEO_ID> A_Matrices;
   std::vector<MIL_UNIQUE_3DGEO_ID> B_Matrices;
   std::vector<MIL_INT> SuccessfulPoses;

   // Reallocate the display to show the spheres of the poses.
   MilSpheresDisplay = Alloc3dDisplayId(MilSystem);
   MIL_INT CommonWindowSizeX = 0, CommonWindowPosX = 0;
   M3ddispInquire(MilCommonDisplay, M_SIZE_X, &CommonWindowSizeX);
   M3ddispInquire(MilCommonDisplay, M_WINDOW_INITIAL_POSITION_X, &CommonWindowPosX);
   M3ddispControl(MilSpheresDisplay, M_WINDOW_INITIAL_POSITION_X, CommonWindowSizeX + CommonWindowPosX);

   // Calculate A and B Matrices for each pose.
   for(MIL_INT i = 0; i < NB_CALIBRATION_POSES; i++)
      {
      // Read point cloud that was captured by the 3D Camera.
      MosPrintf(MIL_TEXT("Processing pose #%i.\n\n"), (MIL_INT)i);
      auto MilCloud = MbufImport(POSES_DATA[i].m_PointCloudFile, M_DEFAULT, M_RESTORE, MilSystem, M_UNIQUE_ID);

      // Detect the spheres in the point cloud.
      std::vector<SphereStats> TargetSpheres = Detector.RetrieveModelSpheres(MilSystem, MilCloud.get(), MilSpheresDisplay, MilCommonDisplay);

      if(TargetSpheres.size() != NUM_SPHERES)
         {
         MosPrintf(MIL_TEXT("Error: Wrong number of spheres detected.\n")
                   MIL_TEXT("Expected %i spheres, detected %i.\n")
                   MIL_TEXT("This pose will be discarded.\n"),
                            NUM_SPHERES, (MIL_INT)TargetSpheres.size());
         }
      else
         {
         // Create stream containing the centers of the spheres that were detected in the point cloud.
         PointStream TargetStream(TargetSpheres);
         auto TargetData = TargetStream.CreateStreamBuffer(MilSystem);

         // Prepare and store the A Matrix, which is the pose of the TCP relatively to the robot base.
         MIL_UNIQUE_3DGEO_ID MilTCPMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
         M3dgeoMatrixSetTransform(MilTCPMatrix, M_ROTATION_ZYX, POSES_DATA[i].m_Tool.m_RotationZ, POSES_DATA[i].m_Tool.m_RotationY, POSES_DATA[i].m_Tool.m_RotationX, M_DEFAULT, M_ASSIGN);
         M3dgeoMatrixSetTransform(MilTCPMatrix, M_TRANSLATION, POSES_DATA[i].m_Tool.m_PositionX, POSES_DATA[i].m_Tool.m_PositionY, POSES_DATA[i].m_Tool.m_PositionZ, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
         A_Matrices.push_back(std::move(MilTCPMatrix));

         // Calculate and store the B Matrix. For that we find the transformation that transforms the CAD sphere center
         // points to the point cloud sphere center points.
         MIL_UNIQUE_3DGEO_ID MilCameraMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
         M3dimFindTransformation(M_FIND_TRANSFORMATION_CONTEXT_RIGID, SourceData, TargetData, MilCameraMatrix, M_DEFAULT);
         M3dgeoMatrixSetTransform(MilCameraMatrix, M_INVERSE, MilCameraMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         B_Matrices.push_back(std::move(MilCameraMatrix));

         // Record the pose as successful.
         SuccessfulPoses.push_back(i);
         }
      MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
      MosGetch();
      }

   // Close the spheres display.
   M3ddispSelect(MilSpheresDisplay, M_NULL, M_CLOSE, M_DEFAULT);

   // Solve AX=ZB system.
   auto MilHandEyeContext = McalAlloc(MilSystem, M_CALCULATE_HAND_EYE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto MilHandEyeResult = McalAllocResult(MilSystem, M_CALCULATE_HAND_EYE_RESULT, M_DEFAULT, M_UNIQUE_ID);
   McalCalculateHandEye(MilHandEyeContext, &A_Matrices[0], &B_Matrices[0], MilHandEyeResult, A_Matrices.size(), M_DEFAULT);

   MosPrintf(MIL_TEXT("Solved AX=ZB system.\n"));

   // Retrieve X Matrix.
   auto MilMatrixX = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   McalCopyResult(MilHandEyeResult, M_DEFAULT, MilMatrixX, M_MATRIX_X, M_DEFAULT);
   MosPrintf(MIL_TEXT("\nX Matrix:\n"));
   DisplayMatrix(MilMatrixX);

   // Calculate and display Matrix X error.
   MIL_DOUBLE DiscrepancyValueX = CalculateMatrixDiscrepancy(MilSystem, MilMatrixX, Expected_CMT);
   MosPrintf(MIL_TEXT("\nMatrix X Discrepancy:%4.1f\n"), DiscrepancyValueX);

   // Retrieve Z Matrix.
   auto MilMatrixZ = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   McalCopyResult(MilHandEyeResult, M_DEFAULT, MilMatrixZ, M_MATRIX_Z, M_DEFAULT);
   MosPrintf(MIL_TEXT("\nZ Matrix:\n"));
   DisplayMatrix(MilMatrixZ);

   // Calculate and display Matrix Z error.
   MIL_DOUBLE DiscrepancyValueZ = CalculateMatrixDiscrepancy(MilSystem, MilMatrixZ, Expected_AMB);
   MosPrintf(MIL_TEXT("\nMatrix Z Discrepancy:%4.1f\n\n"), DiscrepancyValueZ);

   MosPrintf(MIL_TEXT("The point clouds of the different poses will now be transformed\n")
             MIL_TEXT("according to the A, X, and Z matrices to verify the accuracy of\n")
             MIL_TEXT(" the hand eye calibration.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Adjust the view of the comparative 3D display.
   M3ddispSetView(MilCommonDisplay, M_INTEREST_POINT, 0, 0, 200, M_DEFAULT);
   M3ddispSetView(MilCommonDisplay, M_DISTANCE, 1800, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilCommonDisplay, M_AZIM_ELEV_ROLL, 120.0, 30, 0.0, M_DEFAULT);

   // For each pose, transform the CAD model to superpose it on the point cloud.
   for(MIL_INT i = 0; i < (MIL_INT)SuccessfulPoses.size(); i++)
      {
      auto PoseIdx = SuccessfulPoses[i];
      MosPrintf(MIL_TEXT("Superposing the transformed point cloud of pose #%i on the model.\n\n"), (MIL_INT)PoseIdx);
      TestCalibration(MilSystem, A_Matrices[i], MilMatrixX, MilMatrixZ, MilCADModel, POSES_DATA[PoseIdx], MilCommonDisplay);
      }

   // Test Matrices on a pose that was not used during calibration.

   // Retrieve A Matrix for test data.
   MIL_UNIQUE_3DGEO_ID MilTCPMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MilTCPMatrix, M_ROTATION_ZYX, TestData.m_Tool.m_RotationZ, TestData.m_Tool.m_RotationY, TestData.m_Tool.m_RotationX, M_DEFAULT, M_ASSIGN);
   M3dgeoMatrixSetTransform(MilTCPMatrix, M_TRANSLATION, TestData.m_Tool.m_PositionX, TestData.m_Tool.m_PositionY, TestData.m_Tool.m_PositionZ, M_DEFAULT, M_COMPOSE_WITH_CURRENT);

   // Test Data. Note that B Matrix is not used here.
   MosPrintf(MIL_TEXT("Applying A, X and Z matrix to infer the object position for a pose\n")
             MIL_TEXT("that was not used during calibration.\n\n"));
   TestCalibration(MilSystem, MilTCPMatrix, MilMatrixX, MilMatrixZ, MilCADModel, TestData, MilCommonDisplay);

   return true;
   }

//*****************************************************************************
// SphereDetector::TestCalibration
// Desctiption:
//        Use the X and Z matrix to infer the placement of the calibration object.
//        Transforms the Model point cloud to superpose it on the
//        point cloud captured by the 3D camera.
// Input: MilSystem                 - MilSystem ID
//        A_Matrix                  - A matrix
//        X_Matrix                  - X matrix
//        Z_Matrix                  - Z matrix
//        MilCalibrationObjectModel - ID of the container containing the CAD model
//        Data                      - Pose information
//        MilSceneDisplay           - ID of the display to use for drawing
//*****************************************************************************
void TestCalibration(MIL_ID MilSystem, MIL_ID A_Matrix, MIL_ID X_Matrix, MIL_ID Z_Matrix,
                     MIL_ID MilCalibrationObjectModel, const SPoseData& Data, MIL_UNIQUE_3DDISP_ID& MilSceneDisplay)
   {
   // Absolute to Robot Base transform
   auto MilAbsoluteToBase = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MilAbsoluteToBase, M_INVERSE, Z_Matrix, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_ASSIGN);

   // Absolute to TCP transform
   auto MilAbsoluteToTool = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MilAbsoluteToTool, M_COMPOSE_TWO_MATRICES, MilAbsoluteToBase, A_Matrix, M_DEFAULT, M_DEFAULT, M_ASSIGN);

   // Equivalent of Matrix B.
   MIL_UNIQUE_3DGEO_ID MilAbsoluteToCamera = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MilAbsoluteToCamera, M_COMPOSE_TWO_MATRICES, MilAbsoluteToTool, X_Matrix, M_DEFAULT, M_DEFAULT, M_ASSIGN);

   // Retrieve point cloud and superimpose it on the CAD model.
   auto MilPointCloud = MbufImport(Data.m_PointCloudFile, M_DEFAULT, M_RESTORE, MilSystem, M_UNIQUE_ID);
   M3dimMatrixTransform(MilPointCloud, MilPointCloud, MilAbsoluteToCamera, M_DEFAULT);

   // If display available, manage drawing.
   if(MilSceneDisplay)
      {
      // Get the graphic list's identifier.
      MIL_ID MilSceneGraphicList = M_NULL;      
      M3ddispInquire(MilSceneDisplay, M_3D_GRAPHIC_LIST_ID, &MilSceneGraphicList);

      // Disable drawing.
      M3ddispControl(MilSceneDisplay, M_UPDATE, M_DISABLE);

      // Clear display.
      M3dgraRemove(MilSceneGraphicList, M_ALL, M_DEFAULT);

      // Set font size
      M3dgraControl(MilSceneGraphicList, M_DEFAULT_SETTINGS, M_FONT_SIZE, 60);

      // Draw the camera coordinate system
      M3dgraAxis(MilSceneGraphicList, M_DEFAULT, MilAbsoluteToCamera, 100, MIL_TEXT("Camera"), M_DEFAULT, M_DEFAULT);

      // Draw the robot base coordinate system
      M3dgraAxis(MilSceneGraphicList, M_DEFAULT, MilAbsoluteToBase, 100, MIL_TEXT("Robot base"), M_DEFAULT, M_DEFAULT);

      // Draw the TCP coordinate system
      M3dgraAxis(MilSceneGraphicList, M_DEFAULT, MilAbsoluteToTool, 100, MIL_TEXT("TCP"), M_DEFAULT, M_DEFAULT);

      // Draw CAD model.
      MIL_INT64 MilContainerGraphics = M3ddispSelect(MilSceneDisplay, MilCalibrationObjectModel, M_SELECT, M_DEFAULT);
      M3dgraControl(MilSceneGraphicList, MilContainerGraphics, M_COLOR_USE_LUT, M_TRUE);
      M3dgraControl(MilSceneGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);

      // Draw point cloud.
      M3ddispSelect(MilSceneDisplay, MilPointCloud, M_ADD, M_DEFAULT);

      // Enable drawing.
      M3ddispControl(MilSceneDisplay, M_UPDATE, M_ENABLE);
      }
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

