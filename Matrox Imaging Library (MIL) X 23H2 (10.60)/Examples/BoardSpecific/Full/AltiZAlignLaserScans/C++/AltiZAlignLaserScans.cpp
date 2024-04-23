﻿//***************************************************************************************
// 
// File name: AltiZAlignLaserScans.cpp  
//
// Synopsis: This program contains an example showing how to align and fixture
//           laser scans coming from the Matrox AltiZ.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************
#include <mil.h>
#include <math.h>

//*****************************************************************************
// Constants.
//*****************************************************************************

static const MIL_INT DISPLAY_SIZE_Y = 600;

// Edit this to reflect your calibration disk specs.
static const MIL_DOUBLE DISK_DIAMETER   = 70.0;
static const MIL_DOUBLE DISK_HEIGHT     = 50.0;
static const MIL_INT    PITCH_SIGN      = M_ZERO; // M_POSITIVE, M_NEGATIVE, M_ZERO

static const bool       FIXTURE_TO_DISK = false;

// Motion direction threshold to avoid chopping data in Rectified_C.
static const MIL_DOUBLE MAX_RECTIFIED_C_YAW = 5;
static const MIL_DOUBLE MAX_RECTIFIED_C_PITCH = 5;

// Minimum firmware version.
static const MIL_INT MIN_FIRMWARE_MAJOR = 0;
static const MIL_INT MIN_FIRMWARE_MINOR = 6;
static const MIL_STRING SCAN_ILLUSTRATION_FILENAME =
                        M_IMAGE_PATH MIL_TEXT("AlignLaserScans/ScanDisk.png");

static const MIL_STRING CORR_ILLUSTRATION_FILENAME =
                        M_IMAGE_PATH MIL_TEXT("AlignLaserScans/ScanCorrections.png");

// Error messages.
static MIL_CONST_TEXT_PTR LINE_TRIGGER_MISSED_MESSAGE =
   MIL_TEXT("Some line triggers were missed.\n")
   MIL_TEXT("The true motion speed must be smaller than maximum speed feature.\n")
   MIL_TEXT("The true encoder resolution may be smaller than the encoder resolution feature.\n")
   MIL_TEXT("Lower the true motion speed to correctly learn the encoder resolution.");

// Output matrix name.
static MIL_CONST_TEXT_PTR OUTPUT_MATRIX_FILENAME = MIL_TEXT("TransformationMatrix.m3dgeo");

//****************************************************************************
// Utility structure.
//****************************************************************************

// Structure to hold, modify and reset the temporary Altiz feature for the example.
struct SExampleTempFeatures
   {
   SExampleTempFeatures() : FeaturesModified(false) {};
   void ModifyFeatureForExample(MIL_ID MilAltiZDig);
   void ResetFeatures(MIL_ID MilAltiZDig) const;
   void ResetEventNotification(MIL_ID MilAltiZDig) const;
   void ResetCoordinateSystem(MIL_ID MilAltiZDig) const;

   bool FeaturesModified;
   MIL_STRING OutputCS;
   MIL_DOUBLE MotionPitch;
   MIL_DOUBLE MotionYaw;
   MIL_STRING LineTriggerMissedNotification;
   MIL_STRING AcquisitionErrorNotification;
   };

//****************************************************************************
// Function Declaration.
//****************************************************************************

enum class TriggerMode
   {
   Continuous,
   Trigger,
   Encoder,
   Unknown
   };

enum class MotionInputType
   {
   Step,
   Speed,
   Unknown
   };

template <class PrintableChoice>
MIL_INT AskMakeChoice(MIL_CONST_TEXT_PTR ChoiceQuestion, const std::vector<PrintableChoice>& Choices, MIL_INT StartIndex);
bool AskYesNo(MIL_CONST_TEXT_PTR Question);
MIL_INT MFTYPE HookHandler(MIL_INT, MIL_ID, void*) { return 0; };

int Terminate(MIL_CONST_TEXT_PTR Message, MIL_ID MilAltiZDig, const SExampleTempFeatures& TempFeatures);

std::vector<MIL_STRING> GetUserSetNames(MIL_ID MilAltiZDig);

MIL_STRING GetString(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName);
MIL_DOUBLE GetDouble(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName);
MIL_INT64  GetInt(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName);
bool       IsAvailable(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName);

TriggerMode GetTriggerMode(MIL_ID Dig);
MotionInputType GetInputType(MIL_ID Dig);

void SetString(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName, MIL_CONST_TEXT_PTR FeatureValue);
void SetDouble(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName, MIL_DOUBLE FeatureValue);
void SetTransformParam(MIL_ID Dig, MIL_CONST_TEXT_PTR ParamName, MIL_DOUBLE ParamValue);

bool IsFirmwareSupported(MIL_ID Dig);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("AltiZAlignLaserScans\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to align the scans captured \n")
             MIL_TEXT("by a misaligned Matrox AltiZ. It also shows how to perform fixturing.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Digitizer, Display, Buffer \n")
             MIL_TEXT("3D Geometry, 3D Map, 3D Display, and 3D Graphics. \n\n"));
   }

//****************************************************************************
// Scanning guidlines
//****************************************************************************
void PrintScanningGuidelines(MIL_ID MilSystem)
   {
   MIL_UNIQUE_DISP_ID IllustrationDispId;
   MIL_UNIQUE_BUF_ID  IllustrationImageId;

   IllustrationDispId = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Scanning a calibration disk."));
   IllustrationImageId = MbufRestore(SCAN_ILLUSTRATION_FILENAME, MilSystem, M_UNIQUE_ID);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   MosPrintf(MIL_TEXT("Scanning guidelines.\n\n"));
   MosPrintf(MIL_TEXT("1 - The alignment disk must cover at least 50%% of the\n"));
   MosPrintf(MIL_TEXT("    scanned width (X direction).\n"));
   MosPrintf(MIL_TEXT("2 - The alignment disk edge must be fully visible in the scan.\n"));
   MosPrintf(MIL_TEXT("3 - The alignment disk must cover at least 30%% of the\n"));
   MosPrintf(MIL_TEXT("    scanned length (Y direction).\n"));
   MosPrintf(MIL_TEXT("4 - The alignment disk's holes must be at least \n"));
   MosPrintf(MIL_TEXT("    30 scan lines (Y-direction) and 30 points (X-direction).\n"));
   MosPrintf(MIL_TEXT("    The radii of the holes must be within 5 to 10%% of the disk's radius.\n"));
   MosPrintf(MIL_TEXT("    The depth of the holes must be at least 20%% of the total disk's height.\n"));
   MosPrintf(MIL_TEXT("5 - A floor (background plane) must be present in the scan.\n"));
   MosPrintf(MIL_TEXT("6 - Ensure the alignment disk surface is parallel to the motion plane.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
   {
   PrintHeader();

   MosPrintf(MIL_TEXT("Before starting this example, ensure that:\n"));
   MosPrintf(MIL_TEXT("- The GigE Vision driver is up-to-date\n"));
   MosPrintf(MIL_TEXT("- You have edited the AltiZAlignLaserScans.cpp constants to reflect\n"));
   MosPrintf(MIL_TEXT("  the calibration disk specs. Located in the example folder, the document\n"));
   MosPrintf(MIL_TEXT("  3DScannerAlignmentDisk.pdf contains the disk specs to align the scans of\n"));
   MosPrintf(MIL_TEXT("  the different Matrox AltiZ models.\n"));
   MosPrintf(MIL_TEXT("- You have recompiled the example.\n"));
   MosPrintf(MIL_TEXT("- You have access to a Matrox AltiZ on your network.\n"));
   MosPrintf(MIL_TEXT("- MILConfig is configured such that the default system is GigE Vision.\n"));
   MosPrintf(MIL_TEXT("- MILConfig is configured to connect to your Matrox AltiZ by default.\n"));
   MosPrintf(MIL_TEXT("- You configured the Matrox AltiZ, using Capture Works, such that:\n"));
   MosPrintf(MIL_TEXT("  - The AltiZ has appropriate settings to extract the laser line on your disk.\n"));
   MosPrintf(MIL_TEXT("  - The AltiZ is in full surface scan mode (Scan3dVolumeLengthWorld > 0).\n"));
   MosPrintf(MIL_TEXT("  - The AltiZ is using the appropriate trigger mode for your setup.\n"));
   MosPrintf(MIL_TEXT("  - (Recommended) The settings are saved in a user set for easy replay.\n\n"));
   
   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilHostSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   auto MilGigESystem = MsysAlloc(MilApplication, M_SYSTEM_GIGE_VISION, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   auto MilAltiZDig = MdigAlloc(MilGigESystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   auto MilDisplay = M3ddispAlloc(MilHostSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   // Allocate the example feature modification.
   SExampleTempFeatures TempFeatures;

   // Check that 3d display is available.
   if (!MilDisplay)
      return Terminate(MIL_TEXT("The current system does not support 3D display."), MilAltiZDig, TempFeatures);

   // Check that we connected to an AltiZ.
   if (GetString(MilAltiZDig, MIL_TEXT("DeviceModelName")) != MIL_TEXT("AltiZ"))
      return Terminate(MIL_TEXT("The default digitizer in MILConfig is not a Matrox AltiZ."), MilAltiZDig, TempFeatures);

   // Check if the AltiZ firmware version.
   if(!IsFirmwareSupported(MilAltiZDig))
      return Terminate(MIL_TEXT("The AltiZ firmware is not supported by this example. Please update your Altiz."), MilAltiZDig, TempFeatures);

   // Load a user set if required.
   std::vector<MIL_STRING> UserSetNames = GetUserSetNames(MilAltiZDig);
   std::vector<MIL_STRING> LoadChoices(1, MIL_TEXT("Use current settings"));
   LoadChoices.insert(LoadChoices.end(), UserSetNames.begin(), UserSetNames.end());
   auto UserSetIndex = AskMakeChoice(MIL_TEXT("Please select the initial configuration for the scan alignment"), LoadChoices, 0);
   if(UserSetIndex)
      {
      SetString(MilAltiZDig, MIL_TEXT("UserSetSelector"), LoadChoices[UserSetIndex].c_str());
      MdigControlFeature(MilAltiZDig, M_FEATURE_EXECUTE, MIL_TEXT("UserSetLoad"), M_DEFAULT, M_NULL);
      }
   MosPrintf(MIL_TEXT("\n"));

   // Ensure we are acquiring a full surface.
   if (GetString(MilAltiZDig, MIL_TEXT("Scan3dUsageMode")) != MIL_TEXT("Surface"))
      return Terminate(MIL_TEXT("You must set a non-zero length on the AltiZ to scan the calibration disk."), MilAltiZDig, TempFeatures);

   // Ensure we are using world volume definitions.
   if (GetString(MilAltiZDig, MIL_TEXT("Scan3dVolumeDefinitionMode")) != MIL_TEXT("World"))
      return Terminate(MIL_TEXT("This example only supports Scan3dVolumeDefinitionMode set to World."), MilAltiZDig, TempFeatures);

   // Detect trigger mode.
   TriggerMode TrigMode = GetTriggerMode(MilAltiZDig);
   if (TrigMode == TriggerMode::Unknown)
      return Terminate(MIL_TEXT("Unknown trigger mode."), MilAltiZDig, TempFeatures);

   // Detect motion input type.
   MotionInputType InputType = GetInputType(MilAltiZDig);
   if (InputType == MotionInputType::Unknown)
      return Terminate(MIL_TEXT("Unknown motion input type."), MilAltiZDig, TempFeatures);

   // Show the calibration scanning guidlines.
   PrintScanningGuidelines(MilGigESystem);

   if (TrigMode == TriggerMode::Continuous)
      {
      MosPrintf(MIL_TEXT("Continuous acquisition detected.\n"));
      MosPrintf(MIL_TEXT("When scanning, start the conveyor first, then quickly press <Enter>.\n\n"));
      }
   else
      {
      MosPrintf(MIL_TEXT("Hardware trigger detected.\n"));
      MosPrintf(MIL_TEXT("When scanning, first press <Enter>, then start the conveyor.\n\n"));
      }

   // Modify the necessary features for the example and record the original values in case
   // the acquisition or alignment fails.
   TempFeatures.ModifyFeatureForExample(MilAltiZDig);

   MosPrintf(MIL_TEXT("Make sure you are ready to scan the disk.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to scan.\n\n"));
   MosGetch();

   // Enable the genicam event notification of the digitizer.
   MdigHookFunction(MilAltiZDig, M_GC_EVENT, HookHandler, M_NULL);

   // Acquire the disk point cloud.
   auto MilDiskPointCloudRaw = MbufAllocContainer(MilGigESystem, M_GRAB, M_DEFAULT, M_UNIQUE_ID);
   MIL_ID MilDiskPointCloudRawId = MilDiskPointCloudRaw.get();
   MosPrintf(MIL_TEXT("Starting acquisition... "));
   MdigProcess(MilAltiZDig, &MilDiskPointCloudRawId, 1, M_SEQUENCE + M_COUNT(1), M_SYNCHRONOUS, HookHandler, M_NULL);
   MosPrintf(MIL_TEXT("done.\n\n"));

   // Verify if there was an acquisition error or a line trigger missed. We do not need
   // to check what is the event, just knowing that there was one is enough to know that
   // something happened.
   if (IsAvailable(MilAltiZDig, MIL_TEXT("EventLineTriggerMissed")))
      return Terminate(LINE_TRIGGER_MISSED_MESSAGE, MilAltiZDig, TempFeatures);
   if (IsAvailable(MilAltiZDig, MIL_TEXT("EventAcquisitionError")))
      return Terminate(MIL_TEXT("Error detected during the acquisition. Verify the AltiZ features."), MilAltiZDig, TempFeatures);
   
   auto MilDiskPointCloud = MbufAllocContainer(MilGigESystem, M_PROC+M_DISP, M_DEFAULT, M_UNIQUE_ID);
   MbufConvert3d(MilDiskPointCloudRaw, MilDiskPointCloud, M_NULL, M_DEFAULT, M_DEFAULT);
   M3ddispSelect(MilDisplay, MilDiskPointCloud, M_DEFAULT, M_DEFAULT);
   MIL_ID GraList = (MIL_ID)M3ddispInquire(MilDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraAxis(GraList, M_DEFAULT, M_DEFAULT, 50.0, MIL_TEXT("Anchor"), M_DEFAULT, M_DEFAULT);
   MosPrintf(MIL_TEXT("Displaying the calibration disk before correction.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to perform alignment.\n\n"));
   MosGetch();

   auto MilAlignContext = M3dmapAlloc(MilHostSystem, M_ALIGN_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto MilAlignResult = M3dmapAllocResult(MilHostSystem, M_ALIGN_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Set the controls based on the specifications of the calibration disk.
   M3dmapControl(MilAlignContext, M_DEFAULT, M_OBJECT_SHAPE, M_DISK);
   M3dmapControl(MilAlignContext, M_DEFAULT, M_DIAMETER, DISK_DIAMETER);
   M3dmapControl(MilAlignContext, M_DEFAULT, M_HEIGHT, DISK_HEIGHT);

   // M3dmapAlignScan algorithm with a calibration disk also requires
   // the sign of the rotation angle around X.
   M3dmapControl(MilAlignContext, M_DEFAULT, M_CAMERA_TILT_X, PITCH_SIGN);

   if (FIXTURE_TO_DISK)
      {
      M3dmapControl(MilAlignContext, M_DEFAULT, M_ALIGN_X_POSITION, M_OBJECT_CENTER);
      M3dmapControl(MilAlignContext, M_DEFAULT, M_ALIGN_Z_POSITION, M_OBJECT_BOTTOM); 
      M3dmapControl(MilAlignContext, M_DEFAULT, M_ALIGN_XY_DIRECTION, M_SAME_X);
      M3dmapControl(MilAlignContext, M_DEFAULT, M_ALIGN_Z_DIRECTION, M_Z_UP);
      }

   // Compute the alignement.
   M3dmapAlignScan(MilAlignContext, MilDiskPointCloud, MilAlignResult, M_DEFAULT);

   MIL_INT AlignStatus;
   M3dmapGetResult(MilAlignResult, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &AlignStatus);

   if (AlignStatus == M_COMPLETE)
      {
      MosPrintf(MIL_TEXT("Calibration disk found.\n"));

      MIL_INT HolesFound;
      M3dmapGetResult(MilAlignResult, M_DEFAULT, M_HOLES_FOUND+M_TYPE_MIL_INT, &HolesFound);
      if (HolesFound == M_TRUE)
         MosPrintf(MIL_TEXT("Holes detected.\n"));

      // Show corrections illustration.
      auto IllustrationDispId = MdispAlloc(MilHostSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);
      MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Scan corrections"));
      auto IllustrationImageId = MbufRestore(CORR_ILLUSTRATION_FILENAME, MilHostSystem, M_UNIQUE_ID);
      MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_Y, DISPLAY_SIZE_Y + 40);
      MdispSelect(IllustrationDispId, IllustrationImageId);

      MosPrintf(MIL_TEXT("The AltiZ features will now be modified to acquire aligned scans.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      MIL_DOUBLE Pitch, Yaw, NewStepLength;
      M3dmapGetResult(MilAlignResult, M_DEFAULT, M_SENSOR_PITCH, &Pitch);
      M3dmapGetResult(MilAlignResult, M_DEFAULT, M_SENSOR_YAW, &Yaw);
      M3dmapGetResult(MilAlignResult, M_DEFAULT, M_STEP_LENGTH, &NewStepLength);

      SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionPitch"), Pitch);
      SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionYaw"), Yaw);

      // Setting the motion step and direction depends on current settings.
      const MIL_DOUBLE CurStepLength = GetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionEffectiveStepWorld"));

      // Adjust the Altiz speed or step as well as encoder resolution if necessary.
      if (TrigMode == TriggerMode::Continuous)
         {
         // In this mode, movement speed is assumed constant, but unknown.
         // We need to adjust the step so that it becomes NewStepLength.
         if (InputType == MotionInputType::Step)
            {
            SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionStepWorld"), NewStepLength);
            }
         else if (InputType == MotionInputType::Speed)
            {
            // Compute a new speed, such that the effective step becomes NewStepLength.
            const MIL_DOUBLE CurSpeed = GetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionSpeedWorld"));
            const MIL_DOUBLE NewSpeed = CurSpeed * NewStepLength / CurStepLength;
            SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionSpeedWorld"), NewSpeed);
            }
         }
      else if (TrigMode == TriggerMode::Trigger)
         {
         // In this mode, the step is assumed to be constant, but unknown.
         // There is no speed mode.
         SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionStepWorld"), NewStepLength);
         }
      else if (TrigMode == TriggerMode::Encoder)
         {
         // In this mode, the step is assumed to be constant, and is a multiple
         // of the encoder resolution, according to the automatically-computed
         // encoder divider. We must adjust the step, without changing the divider.
         const MIL_DOUBLE EncoderDivider = GetDouble(MilAltiZDig, MIL_TEXT("EncoderDivider"));

         const MIL_DOUBLE RELATIVE_EPSILON = 0.00001;
         if (InputType == MotionInputType::Step)
            {
            const MIL_DOUBLE SafeStepLength = NewStepLength * (1.0 + RELATIVE_EPSILON); // To ensure floor() does not change divider
            SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionStepWorld"), SafeStepLength);
            }
         else if (InputType == MotionInputType::Speed)
            {
            // Compute a new speed, such that the effective step becomes NewStepLength.
            const MIL_DOUBLE CurSpeedMax = GetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionSpeedMaxWorld"));
            const MIL_DOUBLE SafeNewSpeed = (CurSpeedMax * NewStepLength / CurStepLength) * (1.0 - RELATIVE_EPSILON); // To ensure ceil() does not change divider
            SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionSpeedWorld"), SafeNewSpeed);
            }

         const MIL_DOUBLE NewEncoderResolution = NewStepLength / EncoderDivider;
         SetDouble(MilAltiZDig, MIL_TEXT("EncoderResolution"), NewEncoderResolution);

         const MIL_DOUBLE FinalEncoderDivider = GetDouble(MilAltiZDig, MIL_TEXT("EncoderDivider"));
         if(FinalEncoderDivider != EncoderDivider)
            return Terminate(MIL_TEXT("The encoder divider should have changed!\n")
                             MIL_TEXT("Check example Step, Speed and EncoderResolution computation."), MilAltiZDig, TempFeatures);

         MIL_DOUBLE STEP_EPSILON = NewEncoderResolution / 10;
         const MIL_DOUBLE FinalEffectiveStep = GetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionEffectiveStepWorld"));
         if(fabs(FinalEffectiveStep - NewStepLength) > STEP_EPSILON)
            return Terminate(MIL_TEXT("The effective new step length differs from the required step!\n")
                             MIL_TEXT("Check example Step, Speed and EncoderResolution computation."), MilAltiZDig, TempFeatures);
         }

      // Changing the step or speed also changes the number of acquired profiles.
      // Adjust the scan length so that the number of acquired profiles stays roughly the same.
      const MIL_DOUBLE CurScanLength = GetDouble(MilAltiZDig, MIL_TEXT("Scan3dVolumeEffectiveLengthWorld"));
      const MIL_DOUBLE NewScanLength = CurScanLength * NewStepLength / CurStepLength;
      SetDouble(MilAltiZDig, MIL_TEXT("Scan3dVolumeLengthWorld"), NewScanLength);

      // If holes have been detected on the calibration disk, use them to change
      // the motion direction, if needed.
      MIL_DOUBLE ScaleFactorY;
      M3dmapGetResult(MilAlignResult, M_DEFAULT, M_3D_SCALE_Y, &ScaleFactorY);
      if (ScaleFactorY < 0.0)
         {
         MosPrintf(MIL_TEXT("\nThe calibration disk is mirrored.\n"));
         MosPrintf(MIL_TEXT("Motion direction will be changed to correct it.\n"));
         if (GetString(MilAltiZDig, MIL_TEXT("Scan3dMotionDirection")) == MIL_TEXT("Same"))
            SetString(MilAltiZDig, MIL_TEXT("Scan3dMotionDirection"), MIL_TEXT("Reverse"));
         else
            SetString(MilAltiZDig, MIL_TEXT("Scan3dMotionDirection"), MIL_TEXT("Same"));
         }

      // Fixture to disk if required.
      if (FIXTURE_TO_DISK)
         {
         auto MilRigidMatrix = M3dgeoAlloc(MilHostSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
         M3dmapCopyResult(MilAlignResult, M_DEFAULT, MilRigidMatrix, M_RIGID_MATRIX, M_DEFAULT);

         // Extract rotation and translation in SFNC format.
         MIL_DOUBLE Rx, Ry, Rz, Tx, Ty, Tz;
         M3dgeoMatrixGetTransform(MilRigidMatrix, M_ROTATION_ZYX, &Rz, &Ry, &Rx, nullptr, M_DEFAULT);
         M3dgeoMatrixGetTransform(MilRigidMatrix, M_TRANSLATION, &Tx, &Ty, &Tz, nullptr, M_DEFAULT);

         // Set the Transformed parameters on the camera.
         SetTransformParam(MilAltiZDig, MIL_TEXT("RotationX"), Rx);
         SetTransformParam(MilAltiZDig, MIL_TEXT("RotationY"), Ry);
         SetTransformParam(MilAltiZDig, MIL_TEXT("RotationZ"), Rz);
         SetTransformParam(MilAltiZDig, MIL_TEXT("TranslationX"), Tx);
         SetTransformParam(MilAltiZDig, MIL_TEXT("TranslationY"), Ty);
         SetTransformParam(MilAltiZDig, MIL_TEXT("TranslationZ"), Tz);

         // Activate the Transformed coordinate system.
         SetString(MilAltiZDig, MIL_TEXT("Scan3dOutputMode"), MIL_TEXT("CalibratedABC_Grid"));
         SetString(MilAltiZDig, MIL_TEXT("Scan3dCoordinateSystemReference"), MIL_TEXT("Transformed"));
         }
      else
         {
         // Reset original output CS.
         TempFeatures.ResetCoordinateSystem(MilAltiZDig);
         }

      // Change the output mode to CalibratedABC_Grid if either the Pitch or Yaw is too large.
      auto CurrentOutputMode = GetString(MilAltiZDig, MIL_TEXT("Scan3dOutputMode"));
      if(CurrentOutputMode == MIL_TEXT("RectifiedC") && (Yaw > MAX_RECTIFIED_C_YAW || Pitch > MAX_RECTIFIED_C_PITCH))
         {
         MosPrintf(MIL_TEXT("\nThe Pitch or Yaw angle is significant.\n"));
         MosPrintf(MIL_TEXT("The Scan3dOutputMode will be set to CalibratedABC_Grid\n"));
         MosPrintf(MIL_TEXT("to make sure that the output scan contains all the data.\n"));
         SetString(MilAltiZDig, MIL_TEXT("Scan3dOutputMode"), MIL_TEXT("CalibratedABC_Grid"));
         }

      // Save the full alignment matrix.
      auto MilTransformationMatrix = M3dgeoAlloc(MilHostSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
      M3dmapCopyResult(MilAlignResult, M_DEFAULT, MilTransformationMatrix, M_TRANSFORMATION_MATRIX, M_DEFAULT);
      M3dgeoSave(OUTPUT_MATRIX_FILENAME, MilTransformationMatrix, M_DEFAULT);
      MosPrintf(MIL_TEXT("\nThe full transformation matrix (from unaligned anchor) was saved as\n"));
      MosPrintf(MIL_TEXT("\n   %s.\n\n"), OUTPUT_MATRIX_FILENAME);

      MosPrintf(MIL_TEXT("Press <Enter> to scan a new object.\n\n"));
      MosGetch();

      auto MilObjectPointCloudRaw = MbufAllocContainer(MilGigESystem, M_GRAB, M_DEFAULT, M_UNIQUE_ID);
      MIL_ID MilObjectPointCloudRawId = MilObjectPointCloudRaw.get();
      MosPrintf(MIL_TEXT("Starting acquisition... "));
      MdigProcess(MilAltiZDig, &MilObjectPointCloudRawId, 1, M_SEQUENCE + M_COUNT(1), M_SYNCHRONOUS, HookHandler, M_NULL);
      MosPrintf(MIL_TEXT("done.\n\n"));

      // Make sure that acquisition was successful.
      if (IsAvailable(MilAltiZDig, MIL_TEXT("EventLineTriggerMissed")) || IsAvailable(MilAltiZDig, MIL_TEXT("EventAcquisitionError")))
         return Terminate(MIL_TEXT("Unexpected event detected while acquiring a new object."), MilAltiZDig, TempFeatures);
         
      auto ObjectPointCloud = MbufAllocContainer(MilGigESystem, M_PROC+M_DISP, M_DEFAULT, M_UNIQUE_ID);
      MbufConvert3d(MilObjectPointCloudRaw, ObjectPointCloud, M_NULL, M_DEFAULT, M_DEFAULT);
      M3ddispSelect(MilDisplay, ObjectPointCloud, M_DEFAULT, M_DEFAULT);

      MosPrintf(MIL_TEXT("Displaying the object after correction.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Reset the event notification.
      TempFeatures.ResetEventNotification(MilAltiZDig);
      MosPrintf(MIL_TEXT("\n"));

      // Ask to save the current configuration to a custom user set.
      if (UserSetIndex)
         {
         std::vector<MIL_STRING> SaveChoices(1, MIL_TEXT("Do not save current settings"));
         SaveChoices.insert(SaveChoices.end(), UserSetNames.begin(), UserSetNames.begin() + (UserSetIndex-1));
         if(UserSetIndex != (MIL_INT)UserSetNames.size())
            SaveChoices.insert(SaveChoices.end(), UserSetNames.begin() + UserSetIndex, UserSetNames.end());
         UserSetIndex = AskMakeChoice(MIL_TEXT("Please select a user set to save the new configuration."), SaveChoices, 0);
         if (UserSetIndex)
            {
            SetString(MilAltiZDig, MIL_TEXT("UserSetSelector"), SaveChoices[UserSetIndex].c_str());
            SetString(MilAltiZDig, MIL_TEXT("UserSetDescription"), MIL_TEXT("Aligned"));
            MdigControlFeature(MilAltiZDig, M_FEATURE_EXECUTE, MIL_TEXT("UserSetSave"), M_DEFAULT, M_NULL);
            }
         }
      }
   else
      {
      return Terminate(MIL_TEXT("Could not detect the calibration disk."), MilAltiZDig, TempFeatures);
      }

   return 0;
   }

//*****************************************************************************
// Records the value of some features and replace them with values needed
// by the example.
//*****************************************************************************
void SExampleTempFeatures::ModifyFeatureForExample(MIL_ID MilAltiZDig)
   {
   MosPrintf(MIL_TEXT("The Scan3dCoordinateSystemReference will be changed to anchor since the\n"));
   MosPrintf(MIL_TEXT("aligned Pitch and Yaw are expressed in that coordinate system.\n"));
   MosPrintf(MIL_TEXT("The Pitch and Yaw will be reset to 0, i.e. unaligned state.\n"));
   MosPrintf(MIL_TEXT("The events notifications will be activated to ensure a proper acquisition.\n"));

   // Remember reference coordinate system, then force it to Anchor during alignment.
   OutputCS = GetString(MilAltiZDig, MIL_TEXT("Scan3dCoordinateSystemReference"));
   SetString(MilAltiZDig, MIL_TEXT("Scan3dCoordinateSystemReference"), MIL_TEXT("Anchor"));

   // Remember prior alignement result, then remove any prior alignment results.
   MotionPitch = GetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionPitch"));
   MotionYaw = GetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionYaw"));
   SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionPitch"), 0.0);
   SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionYaw"), 0.0);

   // Remember event notification, then enable it.
   SetString(MilAltiZDig, MIL_TEXT("EventSelector"), MIL_TEXT("LineTriggerMissed"));
   LineTriggerMissedNotification = GetString(MilAltiZDig, MIL_TEXT("EventNotification"));
   SetString(MilAltiZDig, MIL_TEXT("EventNotification"), MIL_TEXT("On"));
   SetString(MilAltiZDig, MIL_TEXT("EventSelector"), MIL_TEXT("AcquisitionError"));
   AcquisitionErrorNotification = GetString(MilAltiZDig, MIL_TEXT("EventNotification"));
   SetString(MilAltiZDig, MIL_TEXT("EventNotification"), MIL_TEXT("On"));

   FeaturesModified = true;
   }

void SExampleTempFeatures::ResetFeatures(MIL_ID MilAltiZDig) const
   {
   ResetCoordinateSystem(MilAltiZDig);
   ResetEventNotification(MilAltiZDig);
   if (FeaturesModified)
      {
      SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionPitch"), MotionPitch);
      SetDouble(MilAltiZDig, MIL_TEXT("Scan3dMotionYaw"), MotionYaw);
      }
   }

void SExampleTempFeatures::ResetEventNotification(MIL_ID MilAltiZDig) const
   {
   if (FeaturesModified)
      {
      SetString(MilAltiZDig, MIL_TEXT("EventSelector"), MIL_TEXT("LineTriggerMissed"));
      SetString(MilAltiZDig, MIL_TEXT("EventNotification"), LineTriggerMissedNotification.c_str());
      SetString(MilAltiZDig, MIL_TEXT("EventSelector"), MIL_TEXT("AcquisitionError"));
      SetString(MilAltiZDig, MIL_TEXT("EventNotification"), AcquisitionErrorNotification.c_str());
      }
   }

void SExampleTempFeatures::ResetCoordinateSystem(MIL_ID MilAltiZDig) const
   {
   if (FeaturesModified)
      SetString(MilAltiZDig, MIL_TEXT("Scan3dCoordinateSystemReference"), OutputCS.c_str());
   }

//*****************************************************************************
// Function to call when terminating because of an error. Resets if necessary
// the features that were modified to temporarily to align the Altiz.
//*****************************************************************************
int Terminate(MIL_CONST_TEXT_PTR Message, MIL_ID MilAltiZDig, const SExampleTempFeatures& TempFeatures)
   {
   TempFeatures.ResetFeatures(MilAltiZDig);
   MosPrintf(MIL_TEXT("\n%s\nPress <Enter> to end.\n"), Message);
   MosGetch();
   return 0;
   }

//*****************************************************************************
// Gets the features of the Altiz.
//*****************************************************************************
MIL_STRING GetString(MIL_ID MilDig, MIL_CONST_TEXT_PTR FeatureName)
   {
   MIL_STRING FeatureValue;
   MdigInquireFeature(MilDig, M_FEATURE_VALUE, FeatureName, M_TYPE_STRING, FeatureValue);
   return FeatureValue;
   }

MIL_DOUBLE GetDouble(MIL_ID MilDig, MIL_CONST_TEXT_PTR FeatureName)
   {
   MIL_DOUBLE FeatureValue;
   MdigInquireFeature(MilDig, M_FEATURE_VALUE, FeatureName, M_TYPE_DOUBLE, &FeatureValue);
   return FeatureValue;
   }

MIL_INT64 GetInt(MIL_ID MilDig, MIL_CONST_TEXT_PTR FeatureName)
   {
   MIL_INT64 FeatureValue;
   MdigInquireFeature(MilDig, M_FEATURE_VALUE, FeatureName, M_TYPE_INT64, &FeatureValue);
   return FeatureValue;
   }

bool IsAvailable(MIL_ID MilDig, MIL_CONST_TEXT_PTR FeatureName)
   {
   MIL_INT64 FeatureAccessMode;
   MdigInquireFeature(MilDig, M_FEATURE_ACCESS_MODE, FeatureName, M_TYPE_INT64, &FeatureAccessMode);
   return M_FEATURE_IS_AVAILABLE(FeatureAccessMode);
   }

//*****************************************************************************
// Gets the trigger mode of the Altiz.
//*****************************************************************************
TriggerMode GetTriggerMode(MIL_ID MilDig)
   {
   MIL_STRING TriggerModeName = GetString(MilDig, MIL_TEXT("Scan3dTriggerSourceMode"));
   if (TriggerModeName == MIL_TEXT("Continuous"))
      return TriggerMode::Continuous;
   if (TriggerModeName == MIL_TEXT("Trigger"))
      return TriggerMode::Trigger;
   if (TriggerModeName == MIL_TEXT("Encoder"))
      return TriggerMode::Encoder;
   return TriggerMode::Unknown;
   }

//*****************************************************************************
// Gets the motion input type of the Altiz.
//*****************************************************************************
MotionInputType GetInputType(MIL_ID MilDig)
   {
   MIL_STRING InputTypeName = GetString(MilDig, MIL_TEXT("Scan3dMotionInputType"));
   if (InputTypeName == MIL_TEXT("Step"))
      return MotionInputType::Step;
   if (InputTypeName == MIL_TEXT("Speed"))
      return MotionInputType::Speed;
   return MotionInputType::Unknown;
   }

//*****************************************************************************
// Sets the features of the Altiz.
//*****************************************************************************
void SetString(MIL_ID MilDig, MIL_CONST_TEXT_PTR FeatureName, MIL_CONST_TEXT_PTR FeatureValue)
   {
   MosPrintf(MIL_TEXT("   MdigControlFeature: %s is set to %s\n"), FeatureName, FeatureValue);
   MdigControlFeature(MilDig, M_FEATURE_VALUE, FeatureName, M_TYPE_STRING, FeatureValue);
   }

void SetDouble(MIL_ID MilDig, MIL_CONST_TEXT_PTR FeatureName, MIL_DOUBLE FeatureValue)
   {
   MosPrintf(MIL_TEXT("   MdigControlFeature: %s is set to %f\n"), FeatureName, FeatureValue);
   MdigControlFeature(MilDig, M_FEATURE_VALUE, FeatureName, M_TYPE_DOUBLE, &FeatureValue);
   }

void SetTransformParam(MIL_ID MilDig, MIL_CONST_TEXT_PTR ParamName, MIL_DOUBLE ParamValue)
   {
   SetString(MilDig, MIL_TEXT("Scan3dCoordinateTransformSelector"), ParamName);
   SetDouble(MilDig, MIL_TEXT("Scan3dTransformValue"), ParamValue);
   }

//****************************************************************************
// Check if firmware is supported.
//****************************************************************************
bool IsFirmwareSupported(MIL_ID MilDig)
   {
   auto Firmware = GetString(MilDig, MIL_TEXT("DeviceFirmwareVersion"));
   for (auto& c : Firmware)
      {
      if (c == MIL_TEXT('.'))
         c = MIL_TEXT(' ');
      }
   MIL_STRING_STREAM FirmwareStream(Firmware);
   MIL_INT Version;
   
   // Check the major version.
   FirmwareStream >> Version;
   if (MIN_FIRMWARE_MAJOR > Version)
      return false;
   else if (MIN_FIRMWARE_MAJOR < Version)
      return true;

   // Check the minor version if necessary.
   FirmwareStream >> Version;
   if (MIN_FIRMWARE_MINOR > Version)
      return false;
  
   return true;
   }

//****************************************************************************
// Gets the list of user sets names.
//****************************************************************************
std::vector<MIL_STRING> GetUserSetNames(MIL_ID MilAltiZDig)
   {
   MIL_INT UserSetCount = 0;
   MdigInquireFeature(MilAltiZDig, M_FEATURE_ENUM_ENTRY_COUNT, MIL_TEXT("UserSetSelector"), M_TYPE_MIL_INT, &UserSetCount);
   std::vector<MIL_STRING> UserSetNames;
   if (UserSetCount)
      {
      for (MIL_INT u = 0; u < UserSetCount; u++)
         {
         MIL_INT64 AccessMode;
         MdigInquireFeature(MilAltiZDig, M_FEATURE_ENUM_ENTRY_ACCESS_MODE + u, MIL_TEXT("UserSetSelector"), M_TYPE_INT64, &AccessMode);
         MIL_INT64 Visibility;
         MdigInquireFeature(MilAltiZDig, M_FEATURE_ENUM_ENTRY_VISIBILITY + u, MIL_TEXT("UserSetSelector"), M_TYPE_INT64, &Visibility);

         if (M_FEATURE_IS_READABLE(AccessMode) && Visibility != M_FEATURE_VISIBILITY_INVISIBLE)
            {
            MIL_STRING CurUserSetName;
            MdigInquireFeature(MilAltiZDig, M_FEATURE_ENUM_ENTRY_NAME + u, MIL_TEXT("UserSetSelector"), M_TYPE_STRING, CurUserSetName);
            if (CurUserSetName.find(MIL_TEXT("User")) != MIL_STRING::npos)
               UserSetNames.push_back(CurUserSetName);
            }
         }
      }
   return UserSetNames;
   }

//****************************************************************************
// Ask a question with a yes/no answer.
//****************************************************************************
bool AskYesNo(MIL_CONST_TEXT_PTR Question)
   {
   MosPrintf(MIL_TEXT("%s (y/n)?\n\n"), Question);
   while(1)
      {
      switch(MosGetch())
         {
         case 'Y':
         case 'y':
            return true;
         case 'N':
         case 'n':
            return false;
         default:
            break;
         }
      }
   }

//****************************************************************************
// Asks a question with a a list of choices.
//****************************************************************************
template <class PrintableChoice>
MIL_INT AskMakeChoice(MIL_CONST_TEXT_PTR ChoiceQuestion, const std::vector<PrintableChoice>& Choices, MIL_INT StartIndex)
   {
   MIL_INT Choice;
   do
      {
      // Print the choices.
      MosPrintf(MIL_TEXT("%s\n"), ChoiceQuestion);;
      for(MIL_INT c = 0; c < (MIL_INT)Choices.size(); c++)
         {
         MIL_STRING_STREAM ChoiceStream;
         ChoiceStream << c + StartIndex << ". ";
         ChoiceStream << Choices[c];
         MosPrintf(MIL_TEXT("%s\n"), ChoiceStream.str().c_str());
         }

      MosPrintf(MIL_TEXT("\n"));
      Choice = MosGetch();
      Choice -= ('0' + StartIndex);
      } while(Choice < 0 || Choice >= (MIL_INT)Choices.size());

      // Print the choice.
      MIL_STRING_STREAM ChoiceStream;
      ChoiceStream << Choice + StartIndex << ". ";
      ChoiceStream << Choices[Choice];
      MosPrintf(MIL_TEXT("%s\n"), ChoiceStream.str().c_str());

      return Choice;
   }
