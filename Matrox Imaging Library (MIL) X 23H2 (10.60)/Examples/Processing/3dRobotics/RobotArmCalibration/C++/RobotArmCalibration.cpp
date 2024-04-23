﻿//***************************************************************************************/
// 
// File name: RobotArmCalibration.cpp  
//
// Synopsis:  This program contains an example of 3d robotics calibration using the milcal module.
//            See the PrintHeader() function below for detailed description.
//
//  Credits:  The 3D data was simulated and generated thanks to the RoboDK robot
//            simulation environment.
//
// Printable calibration grids in PDF format can be found in your "Matrox Imaging/Images/"
// directory.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//**************************************************************************************/
#include <mil.h>

#include "math.h"

#define SAVE_PATH MIL_TEXT("")

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("RobotArmCalibration\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows how to calibrate using one of the following 3D robotic setups:\n")
             MIL_TEXT(" - A moving camera is mounted on a robot arm; the moving camera captures images of a\n")
             MIL_TEXT("   grid from different points of views.\n")
             MIL_TEXT(" - A stationary camera with a grid that is attached on the robot arm; the stationary\n")
             MIL_TEXT("   camera captures images of the grid as the robot arm is assigned different poses.\n\n")
 
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: Application, System, Display, Buffer, Calibration, 3D Display\n")
             MIL_TEXT("and 3D Graphics.\n\n")

             MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();
   }

//****************************************************************************
// Struct containing all necessary informations for a single calibration step:
// image file name of the calibration grid, corner hint and robot pose.
//****************************************************************************
struct SPoseData
   {
   MIL_CONST_TEXT_PTR m_ImageFile;
   MIL_DOUBLE m_GridCornerHintX;
   MIL_DOUBLE m_GridCornerHintY;
   MIL_DOUBLE m_ToolPositionX;
   MIL_DOUBLE m_ToolPositionY;
   MIL_DOUBLE m_ToolPositionZ;
   MIL_DOUBLE m_ToolRotationX;
   MIL_DOUBLE m_ToolRotationY;
   MIL_DOUBLE m_ToolRotationZ;
   };

//****************************************************************************
// Struct containing information about the grid
//****************************************************************************
class CGridInfo
   {
   public:
      CGridInfo(  MIL_INT    aRowNumber,
                  MIL_INT    aColumnNumber,
                  MIL_DOUBLE aRowSpacing,
                  MIL_DOUBLE aColumnSpacing,
                  bool       aUseFidicual,
                  MIL_INT    aGridType = M_CHESSBOARD_GRID
      ):
         RowNumber(aRowNumber),
         ColumnNumber(aColumnNumber),
         RowSpacing(aRowSpacing),
         ColumnSpacing(aColumnSpacing),
         UseFiducial(aUseFidicual),
         GridType(aGridType)
         {}

      const MIL_INT    RowNumber;
      const MIL_INT    ColumnNumber;
      const MIL_DOUBLE RowSpacing;           // in mm
      const MIL_DOUBLE ColumnSpacing;           // in mm
      const bool       UseFiducial;
      const MIL_INT    GridType;
   };

//****************************************************************************
// Interface class that allows to access calibration data
//****************************************************************************
class ICalibrationData
   {
   public:
      virtual ~ICalibrationData() = default;
      virtual const SPoseData& GetPoseData(const MIL_INT Index) const = 0;
      virtual MIL_INT GetNumPoses() const = 0;
      virtual const SPoseData& GetTestData() const = 0;
      virtual const CGridInfo& GetGridInfo() const = 0;
      virtual MIL_INT64 GetRobotSetup() const = 0;
      virtual void PrintDescription() const = 0;
      virtual void SetView(MIL_ID MilCalibration, MIL_ID MilDisplay3d) const = 0;
      virtual MIL_INT64 DrawGrid(MIL_ID MilCalibration, MIL_ID Mil3dGraphicList) const = 0;
   };

//****************************************************************************
// Dataset that contains calibration data for a camera on robot arm robot
// setup
//****************************************************************************
class CMovingCameraDataset: public ICalibrationData
   {
   static const MIL_INT NB_CALIBRATION_IMAGES = 7;
   public:
      CMovingCameraDataset() {}

      // Forbid copy constructor and assignment operator
      CMovingCameraDataset(CMovingCameraDataset&) = delete;
      CMovingCameraDataset& operator=(const CMovingCameraDataset&) = delete;

      const SPoseData& GetPoseData(const MIL_INT Index) const override
         {
         return CalibrationData[Index];
         }

      MIL_INT GetNumPoses() const override
         {
         return NB_CALIBRATION_IMAGES;
         }

      const SPoseData& GetTestData() const override
         {
         return TestData;
         }

      const CGridInfo& GetGridInfo() const override
         {
         return GridInfo;
         }

      MIL_INT64 GetRobotSetup() const override
         {
         return M_MOVING_CAMERA;
         }

      void PrintDescription() const override
         {
         MosPrintf(MIL_TEXT("\n\n========================================\n")
                   MIL_TEXT("Moving camera robot setup.\n")
                   MIL_TEXT("========================================\n\n")
                   MIL_TEXT("In this setup, the camera is attached to the robotic arm.\n")
                   MIL_TEXT("The calibration module is used to:\n")
                   MIL_TEXT(" - Calibrate the camera.\n")
                   MIL_TEXT(" - Find the pose of the camera coordinate system with respect to the\n")
                   MIL_TEXT("   robot tool coordinate system.\n")
                   MIL_TEXT(" - Find the pose of the robot base coordinate system with respect to the\n")
                   MIL_TEXT("   absolute coordinate system.\n\n")

                   MIL_TEXT("Press <Enter> to start.\n\n"));
         MosGetch();
         }

      MIL_INT64 DrawGrid(MIL_ID MilCalibration, MIL_ID Mil3dGraphicList) const override
         {
         return M3dgraGrid(Mil3dGraphicList, M_ROOT_NODE, M_SIZE_AND_SPACING, M_DEFAULT, 1000, 1000, 50, 50, M_DEFAULT);
         }

      virtual void SetView(MIL_ID MilCalibration, MIL_ID MilDisplay3d) const override
         {
         M3ddispSetView(MilDisplay3d, M_AZIM_ELEV_ROLL, 120.0, 220.0, 0.0, M_DEFAULT);
         }



   private:
      static const SPoseData CalibrationData[NB_CALIBRATION_IMAGES];
      static const SPoseData TestData;
      static const CGridInfo GridInfo;
   };


//****************************************************************************
// Dataset that contains calibration data for a stationary camera setup
//****************************************************************************
class CStationaryCameraDataset: public ICalibrationData
   {
   static const MIL_INT NB_CALIBRATION_IMAGES = 6;
   public:
      CStationaryCameraDataset() {}

      // Forbid copy constructor and assignment operator
      CStationaryCameraDataset(CStationaryCameraDataset&) = delete;
      CStationaryCameraDataset& operator=(const CStationaryCameraDataset&) = delete;

      const SPoseData& GetPoseData(const MIL_INT Index) const override
         {
         return CalibrationData[Index];
         }

      MIL_INT GetNumPoses() const override
         {
         return NB_CALIBRATION_IMAGES;
         }

      const SPoseData& GetTestData() const override
         {
         return TestData;
         }

      const CGridInfo& GetGridInfo() const override
         {
         return GridInfo;
         }

      MIL_INT64 GetRobotSetup() const override
         {
         return M_STATIONARY_CAMERA;
         }

      void PrintDescription() const override
         {
         MosPrintf(MIL_TEXT("\n\n========================================\n")
                   MIL_TEXT("Stationary camera robot setup.\n")
                   MIL_TEXT("========================================\n\n")
                   MIL_TEXT("In this setup, the grid is attached to the robotic arm and the camera captures\n")
                   MIL_TEXT("images of the grid without moving. The grid moves between captures as the\n")
                   MIL_TEXT("robot arm is assigned different poses.\n")
                   MIL_TEXT("The calibration module is used to:\n")
                   MIL_TEXT(" - Calibrate the camera.\n")
                   MIL_TEXT(" - Find the pose of the grid coordinate system with respect to the\n")
                   MIL_TEXT("   robot tool coordinate system.\n")
                   MIL_TEXT(" - Find the pose of the robot base coordinate system with respect to the\n")
                   MIL_TEXT("   absolute coordinate system.\n\n")

                   MIL_TEXT("Press <Enter> to start.\n\n"));
         MosGetch();
         }

      MIL_INT64 DrawGrid(MIL_ID MilCalibration, MIL_ID Mil3dGraphicList) const override
         {
         // Get the correct rotation.
         auto MilDrawGridMatrix = M3dgeoAlloc(M_DEFAULT_HOST, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
         McalGetCoordinateSystem(MilCalibration, M_ROBOT_BASE_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM,
                                 M_HOMOGENEOUS_MATRIX, MilDrawGridMatrix, M_NULL, M_NULL, M_NULL, M_NULL);

         // Position the center of the grid in the robot base plane between the camera
         // and the robot base.
         MIL_DOUBLE Tx;
         MIL_DOUBLE Ty;
         MIL_DOUBLE Tz;
         McalGetCoordinateSystem(MilCalibration, M_ABSOLUTE_COORDINATE_SYSTEM, M_ROBOT_BASE_COORDINATE_SYSTEM,
                                 M_TRANSLATION, M_NULL, &Tx, &Ty, &Tz, M_NULL);

         auto MilTMatrix = M3dgeoAlloc(M_DEFAULT_HOST, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
         M3dgeoMatrixSetTransform(MilTMatrix, M_TRANSLATION, 0.5 * Tx, 0.5 * Ty, 0, M_DEFAULT, M_ASSIGN);
         M3dgeoMatrixSetTransform(MilDrawGridMatrix, M_COMPOSE_TWO_MATRICES, MilDrawGridMatrix, MilTMatrix, M_DEFAULT, M_DEFAULT, M_ASSIGN);

         // Draw the grid.
         MIL_DOUBLE GridSize = 1.5*sqrt(Tx*Tx + Ty * Ty);
         return M3dgraGrid(Mil3dGraphicList, M_ROOT_NODE, M_SIZE_AND_SPACING, MilDrawGridMatrix, GridSize, GridSize, 50, 50, M_DEFAULT);
         }

      virtual void SetView(MIL_ID MilCalibration, MIL_ID MilDisplay3d) const override
         {
         MIL_DOUBLE Tx;
         MIL_DOUBLE Ty;
         MIL_DOUBLE Tz;
         McalGetCoordinateSystem(MilCalibration, M_ROBOT_BASE_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM,
                                 M_TRANSLATION, M_NULL, &Tx, &Ty, &Tz, M_NULL);
         MIL_DOUBLE VPoint[3] = {3431.0, 2692.0, -1727.0};
         MIL_DOUBLE UpVector[3] = {0.5, 0.0 ,0.86};
         M3ddispSetView(MilDisplay3d, M_VIEWPOINT, VPoint[0], VPoint[1], VPoint[2], M_DEFAULT);
         M3ddispSetView(MilDisplay3d, M_INTEREST_POINT, 0.5*Tx, 0.5*Ty, 0.5*Tz, M_DEFAULT);
         M3ddispSetView(MilDisplay3d, M_UP_VECTOR, UpVector[0], UpVector[1], UpVector[2], M_DEFAULT);
         }

   private:
      static const SPoseData CalibrationData[NB_CALIBRATION_IMAGES];
      static const SPoseData TestData;
      static const CGridInfo GridInfo;
   };

//****************************************************************************
// Constants.
//****************************************************************************

// Directory containing all images used in this example.
#define EXAMPLE_IMAGE_PATH    M_IMAGE_PATH MIL_TEXT("RobotArmCalibration/")

// File name of the 3d robotics calibration context saved at the end of the example.
static MIL_CONST_TEXT_PTR const OUTPUT_CALIBRATION_FILE = SAVE_PATH MIL_TEXT("MilRobotCalibration.mca");

// Data to use during calibration for moving camera setup
const SPoseData CMovingCameraDataset::CalibrationData[] =
   {
   //           File                                           HintX   HintY   TranslationX TranslationY TranslationZ RotationX   RotationY   RotationZ
   {EXAMPLE_IMAGE_PATH MIL_TEXT("MovingCamera/CalGrid0.mim"), M_NONE, M_NONE,  -29.999479,  700.000122,  510.000092,  174.405594, 28.591669,  91.206627},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("MovingCamera/CalGrid1.mim"), M_NONE, M_NONE,  -51.989830,  599.020020,  505.920288,  173.120300, 20.788210,  95.883133},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("MovingCamera/CalGrid2.mim"), M_NONE, M_NONE,  118.010101,  629.020020,  515.919983, -169.119003, 24.478680,  79.661667},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("MovingCamera/CalGrid3.mim"), M_NONE, M_NONE,  118.009903,  719.020020,  505.920105, -167.463898, 31.302469,  85.128510},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("MovingCamera/CalGrid4.mim"), M_NONE, M_NONE,  -11.990170,  519.020081,  415.920013,  179.393494, 16.471180,  91.697990},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("MovingCamera/CalGrid5.mim"), M_NONE, M_NONE,  -29.999969,  399.999786,  509.999786,  175.392303,  0.067751,  97.142853},
   {EXAMPLE_IMAGE_PATH MIL_TEXT("MovingCamera/CalGrid6.mim"), M_NONE, M_NONE, -130.000000,  399.999786,  510.000000,  164.944305,  7.392438, 115.798599}
   };

// Data to test moving camera calibration
const SPoseData CMovingCameraDataset::TestData =
   {EXAMPLE_IMAGE_PATH MIL_TEXT("MovingCamera/TestGrid.mim"), M_NONE, M_NONE,   18.009741,  629.019775,  505.919891,  178.899307, 22.548679,  88.419952};

// Info about the grid used for moving camera calibration
const CGridInfo CMovingCameraDataset::GridInfo = CGridInfo(20,     //RowNumber
                                                           20,     //ColumnNumber
                                                           10.05,  //RowSpacing
                                                           10.00,  //ColumnSpacing
                                                           false   //UseFiducial
                                                           );
// Data to use during calibration for stationary camera setup
const SPoseData CStationaryCameraDataset::CalibrationData[] =
   {
   //           File                                                  HintX   HintY    TranslationX TranslationY   TranslationZ  RotationX      RotationY   RotationZ
   {EXAMPLE_IMAGE_PATH MIL_TEXT("StationaryCamera/CalGridStat0.png"), M_NONE, M_NONE,  794.345445,  -322.415222,   311.151057,   172.310621,    47.687820,  -165.484726 },
   {EXAMPLE_IMAGE_PATH MIL_TEXT("StationaryCamera/CalGridStat1.png"), M_NONE, M_NONE,  857.365222,  -310.088196,   286.481416,  -148.671958,    57.825880,  -161.815312 },
   {EXAMPLE_IMAGE_PATH MIL_TEXT("StationaryCamera/CalGridStat2.png"), M_NONE, M_NONE,  771.087695,  -297.931092,   377.489046,  -153.274399,    67.043497,  -132.610780 },
   {EXAMPLE_IMAGE_PATH MIL_TEXT("StationaryCamera/CalGridStat3.png"), M_NONE, M_NONE,  737.077068,  -377.614836,   334.157713,   144.516353,    50.882819,  -179.775457 },
   {EXAMPLE_IMAGE_PATH MIL_TEXT("StationaryCamera/CalGridStat4.png"), M_NONE, M_NONE,  685.083127,  -238.828275,   427.671873,   158.858753,    62.085755,   167.640586 },
   {EXAMPLE_IMAGE_PATH MIL_TEXT("StationaryCamera/CalGridStat5.png"), M_NONE, M_NONE,  826.135540,  -310.559264,   384.313293,   167.700768,    37.566783,   178.934412 },
   };

// Data to test moving camera calibration
const SPoseData CStationaryCameraDataset::TestData =
   {EXAMPLE_IMAGE_PATH MIL_TEXT("StationaryCamera/TestGridStat.png"), M_NONE, M_NONE,   770.266155,  -360.177764,   455.736060,  -153.635208,    37.566786,   178.934412};

// Info about the grid used for moving camera calibration
const CGridInfo CStationaryCameraDataset::GridInfo = CGridInfo(M_UNKNOWN,     //RowNumber
                                                               M_UNKNOWN,     //ColumnNumber
                                                               10.00,         //RowSpacing
                                                               10.00,         //ColumnSpacing
                                                               true           //UseFiducial
                                                               ); 

// Colors used to draw points in the overlay.
static const MIL_DOUBLE PIXEL_COLOR = M_COLOR_GREEN;
static const MIL_DOUBLE WORLD_COLOR = M_COLOR_RED;

// Position of the 3D display.
static const MIL_INT M3D_DISPLAY_POSITION_X = 600;

// String used to separate output sections.
static MIL_CONST_TEXT_PTR const SEPARATOR = MIL_TEXT("--------------------\n\n");

//****************************************************************************
// Functions declarations.
//****************************************************************************
void MoveRobotPose(MIL_ID MilCalibration, const SPoseData& rData);
void AddCalibrationGrid(MIL_ID  MilCalibration,
                        MIL_ID  MilDisplayImage,
                        MIL_ID  MilGraphicList,
                        ICalibrationData& Dataset,
                        MIL_INT ImageIndex);
void ShowCalibrationResults(MIL_ID MilCalibration,
                            MIL_ID MilDisplayImage,
                            MIL_ID MilGraphicList,
                            ICalibrationData& Dataset);
void TestCalibration(MIL_ID  MilCalibration,
                     MIL_ID  MilDisplayImage,
                     MIL_ID  MilGraphicList,
                     ICalibrationData& Dataset);
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);
bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName);

void ExecuteCalibration(MIL_UNIQUE_SYS_ID &MilSystem, ICalibrationData& Dataset);
MIL_INT ChooseSubExample();

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Allocate MIL objects. 
   auto MilSystem  = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // If '1' is pressed, run stationary camera calibration
   if(ChooseSubExample() == 1)
      {
      CStationaryCameraDataset Dataset;

      if(!CheckForRequiredMILFile(Dataset.GetTestData().m_ImageFile))
         {
         return -1;
         }

      ExecuteCalibration(MilSystem, Dataset);
      }
   else // run moving camera calibration
      {
      CMovingCameraDataset Dataset;

      if(!CheckForRequiredMILFile(Dataset.GetTestData().m_ImageFile))
         {
         return -1;
         }

      ExecuteCalibration(MilSystem, Dataset);
      }

   return 0;
   }

//*****************************************************************************
// Run calibration on the provided dataset
//*****************************************************************************
void ExecuteCalibration(MIL_UNIQUE_SYS_ID &MilSystem, ICalibrationData& Dataset)
   {
   // Print description about the robot setup
   Dataset.PrintDescription();

   MIL_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID MilGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Allocate calibration context for 3D robotics.
   MIL_ID MilCalibration = McalAlloc(MilSystem, M_3D_ROBOTICS, Dataset.GetRobotSetup(), M_NULL);

   if(Dataset.GetGridInfo().UseFiducial)
      {
      McalControl(MilCalibration, M_GRID_PARTIAL, M_ENABLE);
      McalControl(MilCalibration, M_GRID_FIDUCIAL, M_DATAMATRIX);
      }

   // Create an image buffer with the right settings and select it to display.
   MIL_INT SizeX = MbufDiskInquire(Dataset.GetTestData().m_ImageFile, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufDiskInquire(Dataset.GetTestData().m_ImageFile, M_SIZE_Y, M_NULL);
   MIL_ID MilDisplayImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MbufClear(MilDisplayImage, 0.0);
   MdispSelect(MilDisplay, MilDisplayImage);

   // Add informations for calibration.
   for(MIL_INT ImageIndex = 0; ImageIndex < Dataset.GetNumPoses(); ++ImageIndex)
      {
      MosPrintf(MIL_TEXT("The robot arm is at pose #%d.\n\n"), (int)ImageIndex);

      // Move robot and provide new pose to the calibration module.
      MoveRobotPose(MilCalibration, Dataset.GetPoseData(ImageIndex));

      // Provide a calibration grid to the calibration module and verify feature extraction.
      AddCalibrationGrid(MilCalibration, MilDisplayImage, MilGraphicList, Dataset, ImageIndex);

      MosPrintf(SEPARATOR);
      }

   MosPrintf(MIL_TEXT("The 3D robotics calibration is performed using all the accumulated data.\n"));

   // Calibrate 3d robotics calibration context using all accumulated informations.
   McalGrid(MilCalibration, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL, M_DEFAULT, M_DEFAULT);

   // Verify that calibration succeeded.
   MIL_INT CalibrationStatus = McalInquire(MilCalibration, M_CALIBRATION_STATUS, M_NULL);

   if(CalibrationStatus == M_CALIBRATED)
      {
      MosPrintf(MIL_TEXT("The calibration is successful.\n\n"));

      // Show each pose, using DirectX display if available.
      ShowCalibrationResults(MilCalibration, MilDisplayImage, MilGraphicList, Dataset);

      MosPrintf(SEPARATOR);

      // Use one last pose to test the accuracy of the calibration.
      TestCalibration(MilCalibration, MilDisplayImage, MilGraphicList, Dataset);
      }
   else
      {
      MosPrintf(MIL_TEXT("The calibration failed.\n\n")
                MIL_TEXT("Press <Enter> to exit.\n\n"));
      MosGetch();
      }

   // Free MIL objects.
   MbufFree(MilDisplayImage);
   McalFree(MilCalibration);
   MgraFree(MilGraphicList);
   MdispFree(MilDisplay);
   }

//*****************************************************************************
// Sets the tool coordinate system with respect to the robot.
//*****************************************************************************
void MoveRobotPose(MIL_ID MilCalibration, const SPoseData& rData)
   {
   // In a real application, this is where the robot should move to a new location.
   // After moving, the robot software should be used to query the location of its
   // tool (or hand). In this example, this information is hardcoded in the given
   // rData structure.

   // Provide the tool pose given by the robot software to the calibration module.
   // Note that the TransformTypes used (M_TRANSLATION, then M_ROTATION_ZYX in this
   // case) is dependent on the robot controller.
   McalSetCoordinateSystem(MilCalibration, 
                           M_TOOL_COORDINATE_SYSTEM, 
                           M_ROBOT_BASE_COORDINATE_SYSTEM,
                           M_TRANSLATION+M_ASSIGN,
                           M_NULL, 
                           rData.m_ToolPositionX,
                           rData.m_ToolPositionY,
                           rData.m_ToolPositionZ,
                           M_DEFAULT);

   McalSetCoordinateSystem(MilCalibration, 
                           M_TOOL_COORDINATE_SYSTEM, 
                           M_TOOL_COORDINATE_SYSTEM,
                           M_ROTATION_ZYX+M_COMPOSE_WITH_CURRENT, 
                           M_NULL, 
                           rData.m_ToolRotationZ,
                           rData.m_ToolRotationY,
                           rData.m_ToolRotationX,
                           M_DEFAULT);
   }

//*****************************************************************************
// Analyzes one more calibration grid, and display extracted features.
//*****************************************************************************
void AddCalibrationGrid(MIL_ID  MilCalibration,
                        MIL_ID  MilDisplayImage,
                        MIL_ID  MilGraphicList,
                        ICalibrationData& Dataset,
                        MIL_INT ImageIndex)
   {
   MosPrintf(MIL_TEXT("An image of the calibration grid is taken at that position and used for\n")
             MIL_TEXT("calibration.\n\n")

             MIL_TEXT("Calling McalGrid(): "));

   const SPoseData& rData = Dataset.GetPoseData(ImageIndex);

   // Load the image of the calibration grid to the display.
   MbufLoad(rData.m_ImageFile, MilDisplayImage);

   // Provide a hint of where is the top-left corner of the grid.
   McalControl(MilCalibration, M_GRID_HINT_PIXEL_X, rData.m_GridCornerHintX);
   McalControl(MilCalibration, M_GRID_HINT_PIXEL_Y, rData.m_GridCornerHintY);

   const CGridInfo& GridInfo = Dataset.GetGridInfo();

   // Add this grid (note the use of M_ACCUMULATE).
   McalGrid(MilCalibration, MilDisplayImage,
            0.0, 0.0, 0.0, // GridOffset
            GridInfo.RowNumber, GridInfo.ColumnNumber,
            GridInfo.RowSpacing, GridInfo.ColumnSpacing,
            M_ACCUMULATE, GridInfo.GridType);

   // Verify that the operation succeeded.
   MIL_INT CalibrationStatus = McalInquire(MilCalibration, M_CALIBRATION_STATUS, M_NULL);

   // When using M_ACCUMULATE, a succesful call to McalGrid() leaves the calibration context
   // in a partially calibrated state, thus its status should be M_CALIBRATING.
   if (CalibrationStatus == M_CALIBRATING)
      {
      // Draw the features extracted from the grid in the overlay.
      MgraColor(M_DEFAULT, PIXEL_COLOR);
      McalDraw(M_DEFAULT, MilCalibration, MilGraphicList, M_DRAW_IMAGE_POINTS, ImageIndex, M_DEFAULT);

      MosPrintf(MIL_TEXT("Extracted features are displayed in green.\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Clear the graphic list.
      MgraClear(M_DEFAULT, MilGraphicList);
      }
   else
      {
      MosPrintf(MIL_TEXT("The grid was not found.\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }
   }

//*****************************************************************************
// Shows each pose, using MIL 3D display if available, and print error informations.
//*****************************************************************************
void ShowCalibrationResults(MIL_ID MilCalibration, MIL_ID MilDisplayImage, MIL_ID MilGraphicList, ICalibrationData& Dataset)
   {
   // Print results on global errors.
   MIL_DOUBLE AveragePixelError, MaximumPixelError, AverageWorldError, MaximumWorldError;
   McalInquire( MilCalibration, M_GLOBAL_AVERAGE_PIXEL_ERROR, &AveragePixelError );   
   McalInquire( MilCalibration, M_GLOBAL_MAXIMUM_PIXEL_ERROR, &MaximumPixelError );
   McalInquire( MilCalibration, M_GLOBAL_AVERAGE_WORLD_ERROR, &AverageWorldError );   
   McalInquire( MilCalibration, M_GLOBAL_MAXIMUM_WORLD_ERROR, &MaximumWorldError );

   MosPrintf(MIL_TEXT("Global pixel error\n   Average: %.3g pixels\n   Maximum: %.3g pixels\n"),
             AveragePixelError, MaximumPixelError);
   MosPrintf(MIL_TEXT("Global world error\n   Average: %.3g mm\n   Maximum: %.3g mm\n\n"),
             AverageWorldError, MaximumWorldError);

   // Save the calibration context.
   McalSave(OUTPUT_CALIBRATION_FILE, MilCalibration, M_DEFAULT);

   MosPrintf(MIL_TEXT("The calibration context was saved as '"));
   MosPrintf(OUTPUT_CALIBRATION_FILE);
   MosPrintf(MIL_TEXT("'.\nPress <Enter> to verify the calibration accuracy for each pose.\n\n"));
   MosGetch();

   MosPrintf(SEPARATOR);

   // Allocate 3D display.
   MIL_ID  MilDisplay3d  = Alloc3dDisplayId(M_DEFAULT_HOST);
   MIL_INT64 DrawLabel   = 0;
   MIL_ID Mil3dGraphicList = M_NULL;
   MIL_UNIQUE_CAL_ID MilDrawContextId;

   if(MilDisplay3d)
      {
      M3ddispControl(MilDisplay3d,M_WINDOW_INITIAL_POSITION_X, M3D_DISPLAY_POSITION_X);
      Dataset.SetView(MilCalibration, MilDisplay3d);
      M3ddispInquire(MilDisplay3d, M_3D_GRAPHIC_LIST_ID, &Mil3dGraphicList);

      M3dgraControl(Mil3dGraphicList, M_DEFAULT_SETTINGS, M_FONT_SIZE, 18);

      MilDrawContextId = McalAlloc(M_DEFAULT_HOST, M_DRAW_3D_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
      McalControl(MilDrawContextId, M_DRAW_RELATIVE_XY_PLANE, M_ENABLE);
      McalControl(MilDrawContextId, M_DRAW_RELATIVE_XY_PLANE_COLOR_FILL, M_TEXTURE_IMAGE);
      McalControl(MilDrawContextId, M_DRAW_RELATIVE_XY_PLANE_COLOR_OUTLINE, M_COLOR_WHITE);
      McalControl(MilDrawContextId, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DISABLE);
      McalControl(MilDrawContextId, M_DRAW_TOOL_COORDINATE_SYSTEM, M_ENABLE);

      MIL_INT64 MilGrid = Dataset.DrawGrid(MilCalibration, Mil3dGraphicList);
      M3dgraControl(Mil3dGraphicList, MilGrid, M_OPACITY, 10);
      }

   // Show each grid (with both types of draw).
   for (MIL_INT ImageIndex = 0; ImageIndex < Dataset.GetNumPoses(); ++ImageIndex)
      {
      MosPrintf(MIL_TEXT("Pose #%d\n"), (int)ImageIndex);
      MosPrintf(MIL_TEXT("-------\n\n"));

      // Load the next image.
      MbufLoad(Dataset.GetPoseData(ImageIndex).m_ImageFile, MilDisplayImage);

      // Move the tool to the pose used for calibration.
      MoveRobotPose(MilCalibration, Dataset.GetPoseData(ImageIndex));

      // Set all coordinate systems of the image to those used during calibration.
      McalAssociate(MilCalibration, MilDisplayImage, M_DEFAULT);

      // Print results on errors for this pose only.
      McalInquireSingle( MilCalibration, ImageIndex, M_AVERAGE_PIXEL_ERROR, &AveragePixelError );   
      McalInquireSingle( MilCalibration, ImageIndex, M_MAXIMUM_PIXEL_ERROR, &MaximumPixelError );
      McalInquireSingle( MilCalibration, ImageIndex, M_AVERAGE_WORLD_ERROR, &AverageWorldError );   
      McalInquireSingle( MilCalibration, ImageIndex, M_MAXIMUM_WORLD_ERROR, &MaximumWorldError );

      MosPrintf(MIL_TEXT("Pixel error\n   Average: %.3g pixels\n   Maximum: %.3g pixels\n"),
                AveragePixelError, MaximumPixelError);
      MosPrintf(MIL_TEXT("World error\n   Average: %.3g mm\n   Maximum: %.3g mm\n\n"),
                AverageWorldError, MaximumWorldError);

      // Draw the features extracted from the grid AND the world points in the overlay.
      MgraColor(M_DEFAULT, PIXEL_COLOR);
      McalDraw(M_DEFAULT, MilCalibration, MilGraphicList, M_DRAW_IMAGE_POINTS, ImageIndex, M_DEFAULT);
      MgraColor(M_DEFAULT, WORLD_COLOR);
      McalDraw(M_DEFAULT, MilCalibration, MilGraphicList, M_DRAW_WORLD_POINTS, ImageIndex, M_DEFAULT);
      MgraColor(M_DEFAULT, WORLD_COLOR);
      McalDraw(M_DEFAULT, MilCalibration, MilGraphicList, M_DRAW_WORLD_POINTS + M_DRAW_CALIBRATION_ERROR, ImageIndex, M_DEFAULT);
      
      MosPrintf(MIL_TEXT("Green: Extracted features (pixels).\n")
                MIL_TEXT("Red:   World points converted to pixels using the calibration context.\n\n"));

      if(MilDisplay3d)
         {
         if(DrawLabel != 0)
            M3dgraRemove(Mil3dGraphicList, DrawLabel, M_DEFAULT);

         DrawLabel = McalDraw3d(MilDrawContextId, MilDisplayImage, M_DEFAULT, Mil3dGraphicList, M_DEFAULT, MilDisplayImage, M_DEFAULT);
         MosPrintf(MIL_TEXT("The 3D display shows the coordinate systems for calibration pose #%d.\n\n"), (int)ImageIndex);
         if(ImageIndex == 0)
            {
            M3ddispSelect(MilDisplay3d, M_NULL, M_OPEN, M_DEFAULT);
            M3ddispSetView(MilDisplay3d, M_ZOOM, 2, M_DEFAULT, M_DEFAULT, M_DEFAULT);
            } 
         }
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Clear the graphic list.
      MgraClear(M_DEFAULT, MilGraphicList);
      }

   // Free 3D display.
   if(MilDisplay3d)
      { M3ddispFree(MilDisplay3d); }
   }

//*****************************************************************************
// Use one last pose to test the accuracy of the calibration.
//*****************************************************************************
void TestCalibration(MIL_ID  MilCalibration,
                     MIL_ID  MilDisplayImage,
                     MIL_ID  MilGraphicList,
                     ICalibrationData& Dataset
                     )
   {
   // Move one last time.
   MoveRobotPose(MilCalibration, Dataset.GetTestData());

   // Load image at the new position. This is an image of the same calibration grid used
   // for the calibration, but this particular pose was not used.
   MbufLoad(Dataset.GetTestData().m_ImageFile, MilDisplayImage);

   // Draw on image and show that it goes on the grid not used for calibration

   // This draw operation will show, in the overlay, the position of the world points used
   // during calibration, but taking into account the calibration of the overlay (and thus
   // the current pose).
   MgraColor(M_DEFAULT, WORLD_COLOR);
   McalDraw(M_DEFAULT, MilCalibration, MilGraphicList, M_DRAW_WORLD_POINTS, M_DEFAULT, M_DEFAULT);

   // If the calibration is accurate, the marks should be on top of the grid.
   MosPrintf(MIL_TEXT("The robot arm is moved to a new position. McalSetCoordinateSystem() is used\n")
             MIL_TEXT("to provide the new tool pose to the calibration module, thus the system\n")
             MIL_TEXT("remains fully calibrated.\n\n")

             MIL_TEXT("The calibration grid is grabbed at the new position. This grabbed\n")
             MIL_TEXT("image was not used during calibration. The corners of the grid are not\n")
             MIL_TEXT("extracted from this image; no image processing is performed.\n\n")

             MIL_TEXT("Instead, the world points of the calibration grid are converted to pixels\n")
             MIL_TEXT("using the calibration module with the new tool pose. Since these points\n")
             MIL_TEXT("(displayed in red) coincide with the corners of the grabbed grid, the\n")
             MIL_TEXT("calibration is accurate.\n\n")

             MIL_TEXT("Press <Enter> to exit.\n\n"));
   MosGetch();

   // Clear the graphic list.
   MgraClear(M_DEFAULT, MilGraphicList);
   }

//*****************************************************************************
// Creates a 3D display and returns its MIL identifier.  
//*****************************************************************************
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\nThe current system does not support the 3D display.\n\n"));
      }
   return MilDisplay3D;
   }

//*****************************************************************************
// Check for required files for running the example.   
//*****************************************************************************
bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The files needed to run this example are missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n")
                MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }

//****************************************************************************
// Asks the user to choose the sub example.
//****************************************************************************
MIL_INT ChooseSubExample()
   {
   do
      {
      MosPrintf( MIL_TEXT("To run camera on robot arm calibration example, press 0.\n")
                 MIL_TEXT("To run stationary camera calibration example, press 1.\n"));
      switch(MosGetch())
         {
         case MIL_TEXT('0'): return 0;
         case MIL_TEXT('1'): return 1;
         default:            MosPrintf(MIL_TEXT("\n"));
            break;
         }
      } while(1);
   return 0;
   }
